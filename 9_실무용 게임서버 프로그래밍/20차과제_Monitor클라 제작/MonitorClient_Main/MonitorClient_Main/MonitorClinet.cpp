#include "pch.h"
#include "MonitorClient.h"
#include "CrashDump\CrashDump.h"
#include "Log\Log.h"
#include "Parser\Parser_Class.h"

#include "Protocol_Set\CommonProtocol.h"

#include <strsafe.h>

namespace Library_Jingyu
{

	// ���� ID
#define SERVER_HARDWARE	2
#define SERVER_CHAT	3


	CCrashDump* g_MonitorClientDump = CCrashDump::GetInstance();
	CSystemLog* g_MonitorClientLog = CSystemLog::GetInstance();

	// -----------------------
	// �����ڿ� �Ҹ���
	// -----------------------
	CMonitorClient::CMonitorClient()
	{
		// ------------------- ���� �о����
		if (SetFile(&m_stConfig) == false)
			g_MonitorClientDump->Crash();

		// ------------------- �α� ������ ���� ����
		g_MonitorClientLog->SetDirectory(L"MonitorClient");
		g_MonitorClientLog->SetLogLeve((CSystemLog::en_LogLevel)m_stConfig.m_iLogLevel);

		// �ϰ� TLS �����Ҵ�.
		m_WorkPool = new CMemoryPoolTLS<st_WorkNode>(0, false);

		// �ϰ� ���� ť �����Ҵ�
		m_LFQueue = new CLF_Queue<st_WorkNode*>(0, false);

		// �α��� �� �ƴ����� ����
		m_ullSessionID = 0xffffffffffffffff;
		m_bLoginCheck = false;


		// �� ���� ������ ������ ������� ����No ����

		int i = 0;
		int End = dfMONITOR_DATA_TYPE_END - 1;
		while (i < End)
		{
			// �ϵ������ 
			if (i < dfMONITOR_DATA_TYPE_SERVER_NONPAGED_MEMORY)
			{
				// ���� No ����
				m_LastData[i].m_ServerNo = dfMONITOR_ETC;
			}

			// ��ġ����ŷ �������
			else if (i < dfMONITOR_DATA_TYPE_MATCH_MATCHSUCCESS)
			{
				// ���� No ����
				m_LastData[i].m_ServerNo = dfMONITOR_ETC;
			}

			// ������ �������
			else if (i < dfMONITOR_DATA_TYPE_MASTER_BATTLE_STANDBY_ROOM)
			{
				// ���� No ����
				m_LastData[i].m_ServerNo = dfMONITOR_ETC;
			}

			// ��Ʋ �������
			else if (i < dfMONITOR_DATA_TYPE_BATTLE_ROOM_PLAY)
			{
				// ���� No ����
				m_LastData[i].m_ServerNo = dfMONITOR_ETC;
			}

			// ä�� �������
			else
			{
				// ���� No ����
				m_LastData[i].m_ServerNo = dfMONITOR_CHATSERVER;
			}

			++i;

		}

	}

	CMonitorClient::~CMonitorClient()
	{
		// ���� ������ �������̸�, ���� ����
		if (GetClinetState() == true)
			ClientStop();

		// �ϰ� TLS ��������
		delete m_WorkPool;

		// �ϰ� ���� ť ��������
		delete m_LFQueue;	
	}


	// ����� Ŭ���� ����.
	// �ܺ���, WM_CREATE���� ȣ��
	//
	// Parameter : �ν��Ͻ�, ������ �ڵ�
	// retrun : ����
	void CMonitorClient::SetMonitorClass(HINSTANCE hInst, HWND hWnd)
	{
		// -------------------------
		// ����͸� ���� �Ľ�
		// -------------------------
		stSetMonitorInfo FileInfo[dfMONITOR_DATA_TYPE_END - 1];

		int i = 0;
		int End = dfMONITOR_DATA_TYPE_END - 1;
		while (i < End)
		{
			// ���� �̸� �����
			TCHAR Count[3] = { 0, };
			_itot_s(i+1, Count, 3, 10);

			TCHAR AreaName[20] = _T("MONITOR");

			StringCchCat(AreaName, 20, Count);

			// ���� ã�ƺ���.
			if (SetMonitorInfo(&FileInfo[i], AreaName) == true)
			{
				// ã������, ��������� �ٲ۴�.
				FileInfo[i].m_bUseCheck = true;
			}
			else
			{
				// ��ã���� ���, ����� �ƴ����� �ٲٰ� �Ѿ��.
				FileInfo[i].m_bUseCheck = false;
			}			

			++i;
		}


		// -------------------------
		// ����͸� ��� ����
		// -------------------------
		i = 0;

		while (i < End)
		{
			if(FileInfo[i].m_bUseCheck == true)
			{
				// ������ ȣ��
				m_pMonitor[i] = new CMonitorGraphUnit(hInst, hWnd, RGB(FileInfo[i].m_iRedColor, FileInfo[i].m_iGreenColor, FileInfo[i].m_iBlueColor), (CMonitorGraphUnit::TYPE)FileInfo[i].m_iGraphType,
					FileInfo[i].m_iPosX, FileInfo[i].m_iPosY, FileInfo[i].m_iWidth, FileInfo[i].m_iHeight, FileInfo[i].m_tcCaptionText);

				// �߰� ���� ����
				// ������� [Max��, �˶� �︮�� ��ġ, ǥ���� ���� ����]. 
				// - Max ���� 0�̸�, ���� ť�� ���� ���� ū ������ ���
				// - �˶� ���� 0�̸� �˶� �︮�� ����.
				// - ǥ���� ���� ������ L"NULL" �̸�, ���� ǥ������ ����.
				m_pMonitor[i]->AddData(FileInfo[i].m_iGraphMaxValue, FileInfo[i].m_iAlarmValue, FileInfo[i].m_iMinAlarmValue, FileInfo[i].m_tcUnit);

				// �÷� ���� ���� ����
				m_pMonitor[i]->SetColumnInfo(FileInfo[i].m_iDataTypeCount, FileInfo[i].m_iServerNo,
					FileInfo[i].m_iDataType, FileInfo[i].m_tcColumText);
			}

			++i;
		}		
	}

	// 1�ʿ� 1ȸ ȣ��Ǵ� ������Ʈ ����
	//
	// Parameter : ����
	// return : ����
	void CMonitorClient::Update()
	{
		st_WorkNode* NowNode;

		// 1. ť ��ŭ ���鼭 ó��
		while (m_LFQueue->GetInNode() != 0)
		{
			// 1. ť���� ������ �ϳ� ����
			if (m_LFQueue->Dequeue(NowNode) == -1)
				g_MonitorClientDump->Crash();

			try
			{
				// 2. ����Դٴ� ���� ������ �����Ͷ�°�.
				// ������ ó��
				Data_Packet(NowNode->SessionID, NowNode->m_pPacket, m_LastData);
			}
			catch (CException& exc)
			{
				g_MonitorClientDump->Crash();
			}		
			
			// 3. Packet�� ������ ���۷���ī��Ʈ 1 ����
			CProtocolBuff_Net::Free(NowNode->m_pPacket);

			// 4. �ϰ� Free
			m_WorkPool->Free(NowNode);		
		}

		// 2. ť�� �ִ°� �� ó��������, ���� ȭ�鿡 ���.
		int MonitorIndex = 0;
		int End = dfMONITOR_DATA_TYPE_END - 1;
		int DataIndex = 0;

		while (DataIndex < End)
		{			
			// ���ʷ�, �� ���̶� �����͸� ������ �ִ� ��쿡�� �Ʒ� ó��
			// ���� �� ������, ������ üũ�Ұ� ����.
			if (m_LastData[DataIndex].m_bFirstCheck == true)
			{
				int Value;

				// value�� -1�� �ƴ϶��, �����Ͱ� �°�. ������ ����
				if (m_LastData[DataIndex].m_Value != -1)
				{
					Value = m_LastData[DataIndex].m_Value;
				}

				// value�� -1�̶�� �����Ͱ� �ȿ°�.
				else
				{
					if(m_LastData[DataIndex].m_ZeroCount < 3)
						m_LastData[DataIndex].m_ZeroCount++;

					// ����, ������ �ȿ°�, ���� On/Off��� ZeroCount�� �÷��ش�.
					// �̰ɷ�, �� ���̳� �ȿԴ��� üũ �� ����ȭ�� ����⸦ �Ѵ�.
					if (DataIndex + 1 == dfMONITOR_DATA_TYPE_MATCH_SERVER_ON ||
						DataIndex + 1 == dfMONITOR_DATA_TYPE_MASTER_SERVER_ON ||
						DataIndex + 1 == dfMONITOR_DATA_TYPE_CHAT_SERVER_ON)
					{
						// ���� ��, ���� 3�� �Ǿ��ٸ�, 3������ On �޽����� �ȿ°�.
						// �� ���� ���� ������ �����ִٰ� �Ǵ��� ��, 0�� �ִ´�. (���� Off)
						if (m_LastData[DataIndex].m_ZeroCount >= 3)
							Value = 0;

						// ���� 3�������� �ȵƴٸ�, 1�� �ִ´�. (���� On ��)
						else
							Value = 1;
					}

					// ���� On/Off�� �ƴ϶��, �ٷ� Value�� 0�� �ִ´�.
					else
						Value = 0;

				}

				// ���� ������ �ֱ�. �����鼭 ��µ� ����.
				while (MonitorIndex < End)
				{
					if (m_pMonitor[MonitorIndex] != nullptr)
					{
						m_pMonitor[MonitorIndex]->InsertData(Value, m_LastData[DataIndex].m_ServerNo, DataIndex + 1);
					}
					++MonitorIndex;
				}

				// �ʱ�ȭ
				MonitorIndex = 0;
				m_LastData[DataIndex].m_Value = -1;	
			}

			// ������ �ε��� ++
			DataIndex++;
		}
	}
	
	// ����͸� Ŭ���̾�Ʈ ��ŸƮ
	// ���ο����� NetClient�� Start �Լ� ȣ��
	//
	// Parameter : ����
	// return : ���� �� false
	bool CMonitorClient::ClientStart()
	{
		if (Start(m_stConfig.m_tcConnectIP, m_stConfig.m_iPort, m_stConfig.m_iCreateWorker, m_stConfig.m_iActiveWorker, m_stConfig.m_iNodelay, 
			m_stConfig.m_bHeadCode, m_stConfig.m_bXORCode1, m_stConfig.m_bXORCode2) == false)
			return false;

		// ���� ���� �α� ���	
		g_MonitorClientLog->LogSave(L"MonitorClient", CSystemLog::en_LogLevel::LEVEL_SYSTEM, L"Connect Success...");

		return true;
	}

	// ����͸� Ŭ���̾�Ʈ ��ž
	// ���ο����� NetClient�� Stop �Լ� ȣ��
	//
	// Parameter : ����
	// return : ����
	void CMonitorClient::ClientStop()
	{
		Stop();

		// ���� ���� �α� ���
		g_MonitorClientLog->LogSave(L"MonitorClient", CSystemLog::en_LogLevel::LEVEL_SYSTEM, L"Connect Close...");
	}






	// -----------------------
	// ���ο����� ��� ������ ��� �Լ�
	// -----------------------

	// �α��� ��û�� ���� ���� ó��
	//
	// Parameter : SessionID, Packet(Net)
	// return : ����
	void CMonitorClient::Login_Packet(ULONGLONG SessionID, CProtocolBuff_Net* Packet)
	{
		// 1. �α��� ��� �˾ƿ���
		BYTE Status = 0;

		Packet->GetData((char*)&Status, 1);

		// 2. �α��� ����� 0�̸� ����
		if (Status == 0)
			g_MonitorClientDump->Crash();

		// 3. �α��� ����� 1�̸� �α��� ����.
		m_bLoginCheck = true;
	}

	// ������ ���� ��Ŷ �޾��� �� �ϴ� ��
	//
	// Parameter : SessionID, Packet(Net), stLastData*
	// return : ����
	void CMonitorClient::Data_Packet(ULONGLONG SessionID, CProtocolBuff_Net* Packet, stLastData* LastData)
	{
		// �α����� �� �������� üũ
		if (m_bLoginCheck == false)
			g_MonitorClientDump->Crash();

		// 1. ������
		BYTE ServerNo;
		BYTE DataType;
		int DataValue;
		int TimeStamp;

		Packet->GetData((char*)&ServerNo, 1);
		Packet->GetData((char*)&DataType, 1);
		Packet->GetData((char*)&DataValue, 4);
		Packet->GetData((char*)&TimeStamp, 4);

		DataType -= 1;

		// 2. �����͸� �޾�����, �����͸� ���ʷ� ���� ǥ�ø� true�� ����
		LastData[DataType].m_bFirstCheck = true;

		// 3. ������ ����
		// ����, �̹� 0���� ����� �����Ͱ� �ڴʰ� �Դٸ� �����Ѵ�.
		if (LastData[DataType].m_ZeroCount > 0)
		{
			// Ÿ����, ���� On/Off���, �޴� ����, ������ ZeroCount�� 0���� �����, ���� ���� �ٷ� �ݿ��Ѵ�.
			if (DataType + 1 == dfMONITOR_DATA_TYPE_MATCH_SERVER_ON ||
				DataType + 1 == dfMONITOR_DATA_TYPE_MASTER_SERVER_ON ||
				DataType + 1 == dfMONITOR_DATA_TYPE_CHAT_SERVER_ON)
			{
				LastData[DataType].m_ZeroCount = 0;
				LastData[DataType].m_ServerNo = ServerNo;
				LastData[DataType].m_Value = DataValue;
			}

			// ����, ZeroCount�� 3�̾��ٸ�, �������� �ȿ��ٰ� �°�. �ٷ� �ݿ��Ѵ�.
			else if (LastData[DataType].m_ZeroCount == 3)
			{
				LastData[DataType].m_ZeroCount = 0;
				LastData[DataType].m_ServerNo = ServerNo;
				LastData[DataType].m_Value = DataValue;
			}

			// �װ� �ƴ϶�� ī��Ʈ �ϳ� ���̰� ��.
			else
				LastData[DataType].m_ZeroCount--;			
		}

		// 0���� ����� �����Ͱ� ���ٸ�, ���� �̹��� ���� �����Ͱ� �� ��.
		else if (LastData[DataType].m_ZeroCount == 0)
		{
			LastData[DataType].m_ServerNo = ServerNo;
			LastData[DataType].m_Value = DataValue;			
		}
	}


	// �Ľ� �Լ�
	// 
	// Parameter : stInfo*
	// return : ���� �� false
	bool CMonitorClient::SetFile(stInfo* pConfig)
	{
		Parser Parser;

		// ���� �ε�
		try
		{
			Parser.LoadFile(_T("MonitorClient_Config.ini"));
		}
		catch (int expn)
		{
			if (expn == 1)
			{
				printf("File Open Fail...\n");
				return false;
			}
			else if (expn == 2)
			{
				printf("FileR Read Fail...\n");
				return false;
			}
		}

		////////////////////////////////////////////////////////
		// Monitor NetClinet Config �о����
		////////////////////////////////////////////////////////

		// ���� ���� -------------------------
		if (Parser.AreaCheck(_T("MONITORCLIENT")) == false)
			return false;

		// IP
		if (Parser.GetValue_String(_T("ConnectIP"), pConfig->m_tcConnectIP) == false)
			return false;

		// Port
		if (Parser.GetValue_Int(_T("Port"), &pConfig->m_iPort) == false)
			return false;

		// ���� ��Ŀ ��
		if (Parser.GetValue_Int(_T("CreateWorker"), &pConfig->m_iCreateWorker) == false)
			return false;

		// Ȱ��ȭ ��Ŀ ��
		if (Parser.GetValue_Int(_T("ActiveWorker"), &pConfig->m_iActiveWorker) == false)
			return false;		

		// ��� �ڵ�
		if (Parser.GetValue_Int(_T("HeadCode"), &pConfig->m_bHeadCode) == false)
			return false;

		// xorcode1
		if (Parser.GetValue_Int(_T("XorCode1"), &pConfig->m_bXORCode1) == false)
			return false;

		// xorcode2
		if (Parser.GetValue_Int(_T("XorCode2"), &pConfig->m_bXORCode2) == false)
			return false;

		// Nodelay
		if (Parser.GetValue_Int(_T("Nodelay"), &pConfig->m_iNodelay) == false)
			return false;		

		// �α� ����
		if (Parser.GetValue_Int(_T("LogLevel"), &pConfig->m_iLogLevel) == false)
			return false;

		// �α��� Ű
		TCHAR tcLoginKey[33];
		if (Parser.GetValue_String(_T("LoginKey"), tcLoginKey) == false)
			return false;

		// Ŭ�� ���� ���� char������ ������ ������ ��ȯ�ؼ� ������ �־���Ѵ�.
		int len = WideCharToMultiByte(CP_UTF8, 0, tcLoginKey, lstrlenW(tcLoginKey), NULL, 0, NULL, NULL);
		WideCharToMultiByte(CP_UTF8, 0, tcLoginKey, lstrlenW(tcLoginKey), m_cLoginKey, len, NULL, NULL);

		return true;

	}

	// ����͸� ������ �о���� �Լ�
	// 
	// Parameter : stSetMonitorInfo ����ü, ���� �̸�
	// return : ���� ���� ���� �� false (�� �� ���д� ��� Crash)
	bool CMonitorClient::SetMonitorInfo(stSetMonitorInfo* pConfig, TCHAR* AreaName)
	{
		Parser Parser;

		// ���� �ε�
		try
		{
			Parser.LoadFile(_T("MonitorClient_Config.ini"));
		}
		catch (int expn)
		{
			if (expn == 1)
			{
				printf("File Open Fail...\n");
				return false;
			}
			else if (expn == 2)
			{
				printf("FileR Read Fail...\n");
				return false;
			}
		}

		////////////////////////////////////////////////////////
		// Monitor NetClinet Config �о����
		////////////////////////////////////////////////////////

		// ���� ���� -------------------------
		if (Parser.AreaCheck(AreaName) == false)
			return false;

		// DataTypeCount
		if (Parser.GetValue_Int(_T("DataTypeCount"), &pConfig->m_iDataTypeCount) == false)
			g_MonitorClientDump->Crash();
		
		// DataType
		// DataTypeCount��ŭ ���鼭 �̾Ƴ���.
		int i = 0;
		while (i < pConfig->m_iDataTypeCount)
		{
			// �÷� �̸� �����
			TCHAR Count[3] = { 0, };
			_itot_s(i + 1, Count, 3, 10);

			TCHAR TypeName[20] = _T("DataType");

			StringCchCat(TypeName, 20, Count);

			if (Parser.GetValue_Int(TypeName, &pConfig->m_iDataType[i]) == false)
				g_MonitorClientDump->Crash();

			++i;
		}
		

		

		// GraphType
		if (Parser.GetValue_Int(_T("GraphType"), &pConfig->m_iGraphType) == false)
			g_MonitorClientDump->Crash();

		// PosX
		if (Parser.GetValue_Int(_T("PosX"), &pConfig->m_iPosX) == false)
			g_MonitorClientDump->Crash();

		// PosY
		if (Parser.GetValue_Int(_T("PosY"), &pConfig->m_iPosY) == false)
			g_MonitorClientDump->Crash();

		// Width
		if (Parser.GetValue_Int(_T("Width"), &pConfig->m_iWidth) == false)
			g_MonitorClientDump->Crash();

		// Height
		if (Parser.GetValue_Int(_T("Height"), &pConfig->m_iHeight) == false)
			g_MonitorClientDump->Crash();



		// ĸ�� �ؽ�Ʈ
		if (Parser.GetValue_String(_T("CaptionText"), pConfig->m_tcCaptionText) == false)
			g_MonitorClientDump->Crash();

		// Unit�ؽ�Ʈ
		if (Parser.GetValue_String(_T("Unit"), pConfig->m_tcUnit) == false)
			g_MonitorClientDump->Crash();




		// Red
		if (Parser.GetValue_Int(_T("Red"), &pConfig->m_iRedColor) == false)
			g_MonitorClientDump->Crash();

		// Green
		if (Parser.GetValue_Int(_T("Green"), &pConfig->m_iGreenColor) == false)
			g_MonitorClientDump->Crash();

		// Blue
		if (Parser.GetValue_Int(_T("Blue"), &pConfig->m_iBlueColor) == false)
			g_MonitorClientDump->Crash();




		// GraphMaxValue 
		if (Parser.GetValue_Int(_T("GraphMaxValue"), &pConfig->m_iGraphMaxValue) == false)
			g_MonitorClientDump->Crash();

		// AlarmValue 
		if (Parser.GetValue_Int(_T("AlarmValue"), &pConfig->m_iAlarmValue) == false)
			g_MonitorClientDump->Crash();

		// MinAlarmValue 
		if (Parser.GetValue_Int(_T("MinAlarmValue"), &pConfig->m_iMinAlarmValue) == false)
			g_MonitorClientDump->Crash();

			   		 

		

		// ServerNo 
		// DataTypeCount��ŭ ���鼭 �̾Ƴ���.
		i = 0;
		while (i < pConfig->m_iDataTypeCount)
		{
			// �÷� �̸� �����
			TCHAR Count[3] = { 0, };
			_itot_s(i + 1, Count, 3, 10);

			TCHAR ServerNoText[20] = _T("ServerNo");

			StringCchCat(ServerNoText, 20, Count);

			if (Parser.GetValue_Int(ServerNoText, &pConfig->m_iServerNo[i]) == false)
				g_MonitorClientDump->Crash();

			++i;
		}		

		// Colum �ؽ�Ʈ
		// DataTypeCount��ŭ ���鼭 �̾Ƴ���.
		i = 0;
		while (i < pConfig->m_iDataTypeCount)
		{
			// �÷� �̸� �����
			TCHAR Count[3] = { 0, };
			_itot_s(i + 1, Count, 3, 10);

			TCHAR ColumName[20] = _T("ColumText");

			StringCchCat(ColumName, 20, Count);

			if (Parser.GetValue_String(ColumName, pConfig->m_tcColumText[i]) == false)
				g_MonitorClientDump->Crash();

			++i;
		}
		

		return true;
	}


	// -----------------------
	// �����Լ�
	// -----------------------

	// ��ǥ ������ ���� ���� ��, ȣ��Ǵ� �Լ� (ConnectFunc���� ���� ���� �� ȣ��)
	//
	// parameter : ����Ű
	// return : ����
	void CMonitorClient::OnConnect(ULONGLONG ClinetID)
	{
		// ���� ID �����صα�
		m_ullSessionID = ClinetID;

		// �α��� ��Ŷ ������
		WORD Type = en_PACKET_CS_MONITOR_TOOL_REQ_LOGIN;
		char Key[] = "P09djiwl34jWJV%@oW@#o0d82jvk#cjz";

		CProtocolBuff_Net* SendBuff = CProtocolBuff_Net::Alloc();

		SendBuff->PutData((char*)&Type, 2);
		SendBuff->PutData(Key, 32);

		SendPacket(ClinetID, SendBuff);
	}

	// ��ǥ ������ ���� ���� �� ȣ��Ǵ� �Լ� (InDIsconnect �ȿ��� ȣ��)
	//
	// parameter : ����Ű
	// return : ����
	void CMonitorClient::OnDisconnect(ULONGLONG ClinetID)
	{
		m_ullSessionID = 0xffffffffffffffff;
		m_bLoginCheck = false;
	}


	// ��Ŷ ���� �Ϸ� �� ȣ��Ǵ� �Լ�.
	//
	// parameter : ���� ����Ű, ���� ��Ŷ
	// return : ����
	void CMonitorClient::OnRecv(ULONGLONG SessionID, CProtocolBuff_Net* Payload)
	{
		try
		{
			// 1. Type�� ������.
			WORD Type;
			Payload->GetData((char*)&Type, 2);

			switch (Type)
			{
				// type�� �α��� ��û�̶�� �α��� ��Ŷ ó��
			case en_PACKET_CS_MONITOR_TOOL_RES_LOGIN:
				Login_Packet(SessionID, Payload);
				break;

				// type�� ������ �����̶��
			case en_PACKET_CS_MONITOR_TOOL_DATA_UPDATE:
			{
				// �ϰ� TLS���� �ϰ� ����ü Alloc
				st_WorkNode* NowWork = m_WorkPool->Alloc();

				// Payload ���۷��� ī��Ʈ ���� �� �ϰ��� ����
				Payload->Add();
				NowWork->m_pPacket = Payload;
				NowWork->SessionID = SessionID;

				// �ϰ� ť�� �ֱ�
				m_LFQueue->Enqueue(NowWork);
			}
			break;

			// �� �� Ÿ���� ����
			default:
				throw CException(_T("OnRecv() --> Type Error!!"));
				
				break;
			}

		}
		catch (CException& exc)
		{
			g_MonitorClientLog->LogSave(L"MonitorClient", CSystemLog::en_LogLevel::LEVEL_ERROR, L"%s", 
				(TCHAR*)exc.GetExceptionText());

			g_MonitorClientDump->Crash();
		}
		
		
	}

	// ��Ŷ �۽� �Ϸ� �� ȣ��Ǵ� �Լ�
	//
	// parameter : ���� ����Ű, Send �� ������
	// return : ����
	void CMonitorClient::OnSend(ULONGLONG SessionID, DWORD SendSize)
	{}

	// ��Ŀ �����尡 ��� �� ȣ��Ǵ� �Լ�.
	// GQCS �ٷ� �ϴܿ��� ȣ��
	// 
	// parameter : ����
	// return : ����
	void CMonitorClient::OnWorkerThreadBegin()
	{}

	// ��Ŀ �����尡 ���� �� ȣ��Ǵ� �Լ�
	// GQCS �ٷ� ������ ȣ��
	// 
	// parameter : ����
	// return : ����
	void CMonitorClient::OnWorkerThreadEnd()
	{}

	// ���� �߻� �� ȣ��Ǵ� �Լ�.
	//
	// parameter : ���� �ڵ�(���� ������ �����ڵ�� WinGetLastError() �Լ��� ��� ����. ���� ��� 0�� ���ϵ�)
	//			 : ���� �ڵ忡 ���� ��Ʈ��
	// return : ����
	void CMonitorClient::OnError(int error, const TCHAR* errorStr)
	{
		g_MonitorClientLog->LogSave(L"MonitorClient", CSystemLog::en_LogLevel::LEVEL_ERROR, errorStr);
		g_MonitorClientDump->Crash();
	}
}