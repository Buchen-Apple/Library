#include "pch.h"
#include "MonitorClient.h"
#include "CrashDump\CrashDump.h"
#include "Log\Log.h"

#include "Protocol_Set\CommonProtocol.h"

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

		// ------------------- �α� ������ ���� ����
		g_MonitorClientLog->SetDirectory(L"MonitorClient");
		g_MonitorClientLog->SetLogLeve((CSystemLog::en_LogLevel)CSystemLog::en_LogLevel::LEVEL_DEBUG);

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
		// -------------------- 
		// �ϵ����
		// -------------------- 
		m_pMonitor[dfMONITOR_DATA_TYPE_SERVER_CPU_TOTAL-1] = new CMonitorGraphUnit(hInst, hWnd, RGB(62, 62, 62), CMonitorGraphUnit::LINE_SINGLE, 10, 10, 500, 130, L"���� CPU");
		m_pMonitor[dfMONITOR_DATA_TYPE_SERVER_AVAILABLE_MEMORY-1] = new CMonitorGraphUnit(hInst, hWnd, RGB(62, 62, 62), CMonitorGraphUnit::LINE_SINGLE, 520, 10, 400, 130, L"���� ��� ���� �޸�");
		m_pMonitor[dfMONITOR_DATA_TYPE_SERVER_NETWORK_RECV-1] = new CMonitorGraphUnit(hInst, hWnd, RGB(62, 62, 62), CMonitorGraphUnit::LINE_MULTI, 930, 10, 510, 130, L"Network");
		m_pMonitor[dfMONITOR_DATA_TYPE_SERVER_NONPAGED_MEMORY -1] = new CMonitorGraphUnit(hInst, hWnd, RGB(62, 62, 62), CMonitorGraphUnit::LINE_SINGLE, 1450, 10, 460, 130, L"Nonpaged Mem");

		// �߰� ���� ����
		// ������� [Max��, �˶� �︮�� ��ġ, ǥ���� ���� ����]. 
		// Max ���� 0�̸�, ���� ť�� ���� ���� ū ������ ���
		// �˶� ���� 0�̸� �˶� �︮�� ����.
		// ǥ���� ���� ������ L"NULL" �̸�, ���� ǥ������ ����.
		m_pMonitor[dfMONITOR_DATA_TYPE_SERVER_CPU_TOTAL - 1]->AddData(100, 90, L"%");
		m_pMonitor[dfMONITOR_DATA_TYPE_SERVER_AVAILABLE_MEMORY - 1]->AddData(0, 0, L"MB");
		m_pMonitor[dfMONITOR_DATA_TYPE_SERVER_NETWORK_RECV - 1]->AddData(0, 0, L"KByte");
		m_pMonitor[dfMONITOR_DATA_TYPE_SERVER_NONPAGED_MEMORY - 1]->AddData(0, 0, L"MB");

		// �÷� ���� ���� ����
		int p1_ColumnCount = 1;
		int p1_ServerID[1] = { SERVER_HARDWARE };
		int p1_DataType[1] = { dfMONITOR_DATA_TYPE_SERVER_CPU_TOTAL };
		TCHAR p1_DataName[1][20] = { L"CPU ��뷮" };

		int p2_ColumnCount = 1;
		int p2_ServerID[1] = { SERVER_HARDWARE };
		int p2_DataType[1] = { dfMONITOR_DATA_TYPE_SERVER_AVAILABLE_MEMORY };
		TCHAR p2_DataName[1][20] = { L"���� ��� ���� �޸�" };

		int p3_ColumnCount = 2;
		int p3_ServerID[2] = { SERVER_HARDWARE, SERVER_HARDWARE };
		int p3_DataType[2] = { dfMONITOR_DATA_TYPE_SERVER_NETWORK_RECV, dfMONITOR_DATA_TYPE_SERVER_NETWORK_SEND };
		TCHAR p3_DataName[2][20] = { L"Recv", L"Send" };

		int p4_ColumnCount = 1;
		int p4_ServerID[1] = { SERVER_HARDWARE };
		int p4_DataType[1] = { dfMONITOR_DATA_TYPE_SERVER_NONPAGED_MEMORY };
		TCHAR p4_DataName[1][20] = { L"Nonpaged Mem" };

		m_pMonitor[dfMONITOR_DATA_TYPE_SERVER_CPU_TOTAL - 1]->SetColumnInfo(1, p1_ServerID, p1_DataType, p1_DataName);
		m_pMonitor[dfMONITOR_DATA_TYPE_SERVER_AVAILABLE_MEMORY - 1]->SetColumnInfo(1, p2_ServerID, p2_DataType, p2_DataName);
		m_pMonitor[dfMONITOR_DATA_TYPE_SERVER_NETWORK_RECV - 1]->SetColumnInfo(2, p3_ServerID, p3_DataType, p3_DataName);
		m_pMonitor[dfMONITOR_DATA_TYPE_SERVER_NONPAGED_MEMORY - 1]->SetColumnInfo(1, p4_ServerID, p4_DataType, p4_DataName);


		// -------------------- 
		// ä�ü���
		// -------------------- 
		m_pMonitor[dfMONITOR_DATA_TYPE_CHAT_SERVER_ON - 1] = new CMonitorGraphUnit(hInst, hWnd, RGB(18, 52, 120), CMonitorGraphUnit::ONOFF, 10, 150, 150, 130, L"ä�� ���� On/Off");
		m_pMonitor[dfMONITOR_DATA_TYPE_CHAT_CPU - 1] = new CMonitorGraphUnit(hInst, hWnd, RGB(18, 52, 120), CMonitorGraphUnit::LINE_SINGLE, 170, 150, 200, 130, L"ä�ü��� CPU");
		m_pMonitor[dfMONITOR_DATA_TYPE_CHAT_MEMORY_COMMIT - 1] = new CMonitorGraphUnit(hInst, hWnd, RGB(18, 52, 120), CMonitorGraphUnit::LINE_SINGLE, 380, 150, 200, 130, L"ä�ü��� �޸�");
		m_pMonitor[dfMONITOR_DATA_TYPE_CHAT_PACKET_POOL - 1] = new CMonitorGraphUnit(hInst, hWnd, RGB(18, 52, 120), CMonitorGraphUnit::LINE_SINGLE, 590, 150, 200, 130, L"PacketPool");
		m_pMonitor[dfMONITOR_DATA_TYPE_CHAT_SESSION - 1] = new CMonitorGraphUnit(hInst, hWnd, RGB(18, 52, 120), CMonitorGraphUnit::LINE_SINGLE, 1110, 150, 200, 130, L"SessionAll");
		m_pMonitor[dfMONITOR_DATA_TYPE_CHAT_PLAYER - 1] = new CMonitorGraphUnit(hInst, hWnd, RGB(18, 52, 120), CMonitorGraphUnit::LINE_SINGLE, 1320, 150, 300, 130, L"Login Player");
		m_pMonitor[dfMONITOR_DATA_TYPE_CHAT_ROOM - 1] = new CMonitorGraphUnit(hInst, hWnd, RGB(18, 52, 120), CMonitorGraphUnit::LINE_SINGLE, 1630, 150, 280, 130, L"Room");
				
		// �߰� ���� ����		
		m_pMonitor[dfMONITOR_DATA_TYPE_CHAT_SERVER_ON - 1]->AddData(0, 0, L"NULL");
		m_pMonitor[dfMONITOR_DATA_TYPE_CHAT_CPU - 1]->AddData(100, 90, L"%");
		m_pMonitor[dfMONITOR_DATA_TYPE_CHAT_MEMORY_COMMIT - 1]->AddData(0, 0, L"MB");
		m_pMonitor[dfMONITOR_DATA_TYPE_CHAT_PACKET_POOL - 1]->AddData(0, 0, L"NULL");
		m_pMonitor[dfMONITOR_DATA_TYPE_CHAT_SESSION - 1]->AddData(0, 0, L"NULL");
		m_pMonitor[dfMONITOR_DATA_TYPE_CHAT_PLAYER - 1]->AddData(0, 0, L"NULL");
		m_pMonitor[dfMONITOR_DATA_TYPE_CHAT_ROOM - 1]->AddData(0, 0, L"NULL");

		// �÷� ���� ���� ����
		int p5_ColumnCount = 1;
		int p5_ServerID[1] = { SERVER_CHAT };
		int p5_DataType[1] = { dfMONITOR_DATA_TYPE_CHAT_SERVER_ON };
		TCHAR p5_DataName[1][20] = { L"ä�� ���� On/Off" };

		int p6_ColumnCount = 1;
		int p6_ServerID[1] = { SERVER_CHAT };
		int p6_DataType[1] = { dfMONITOR_DATA_TYPE_CHAT_CPU };
		TCHAR p6_DataName[1][20] = { L"ä�ü��� CPU" };

		int p7_ColumnCount = 1;
		int p7_ServerID[1] = { SERVER_CHAT };
		int p7_DataType[1] = { dfMONITOR_DATA_TYPE_CHAT_MEMORY_COMMIT };
		TCHAR p7_DataName[1][20] = { L"ä�ü��� �޸�" };

		int p8_ColumnCount = 1;
		int p8_ServerID[1] = { SERVER_CHAT };
		int p8_DataType[1] = { dfMONITOR_DATA_TYPE_CHAT_PACKET_POOL };
		TCHAR p8_DataName[1][20] = { L"PacketPool" };

		int p9_ColumnCount = 1;
		int p9_ServerID[1] = { SERVER_CHAT };
		int p9_DataType[1] = { dfMONITOR_DATA_TYPE_CHAT_SESSION };
		TCHAR p9_DataName[1][20] = { L"SessionAll" };

		int p10_ColumnCount = 1;
		int p10_ServerID[1] = { SERVER_CHAT };
		int p10_DataType[1] = { dfMONITOR_DATA_TYPE_CHAT_PLAYER };
		TCHAR p10_DataName[1][20] = { L"Login Player" };

		int p11_ColumnCount = 1;
		int p11_ServerID[1] = { SERVER_CHAT };
		int p11_DataType[1] = { dfMONITOR_DATA_TYPE_CHAT_ROOM };
		TCHAR p11_DataName[1][20] = { L"Room" };
		
		m_pMonitor[dfMONITOR_DATA_TYPE_CHAT_SERVER_ON - 1]->SetColumnInfo(1, p5_ServerID, p5_DataType, p5_DataName);
		m_pMonitor[dfMONITOR_DATA_TYPE_CHAT_CPU - 1]->SetColumnInfo(1, p6_ServerID, p6_DataType, p6_DataName);
		m_pMonitor[dfMONITOR_DATA_TYPE_CHAT_MEMORY_COMMIT - 1]->SetColumnInfo(1, p7_ServerID, p7_DataType, p7_DataName);
		m_pMonitor[dfMONITOR_DATA_TYPE_CHAT_PACKET_POOL - 1]->SetColumnInfo(1, p8_ServerID, p8_DataType, p8_DataName);
		m_pMonitor[dfMONITOR_DATA_TYPE_CHAT_SESSION - 1]->SetColumnInfo(1, p9_ServerID, p9_DataType, p9_DataName);
		m_pMonitor[dfMONITOR_DATA_TYPE_CHAT_PLAYER - 1]->SetColumnInfo(1, p10_ServerID, p10_DataType, p10_DataName);
		m_pMonitor[dfMONITOR_DATA_TYPE_CHAT_ROOM - 1]->SetColumnInfo(1, p11_ServerID, p11_DataType, p11_DataName);
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
			int Value = 0;

			// value�� -1�� �ƴ϶��, �����Ͱ� �°�. ������ ����
			if (m_LastData[DataIndex].m_Value != -1)
			{
				Value = m_LastData[DataIndex].m_Value;						
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

			// �ε��� ++
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
		TCHAR IP[20] = L"127.0.0.1";
		WORD Port = 18245;
		int CreateWorker = 1;
		int ActiveWorker = 1;
		int Nodelay = 0;
		BYTE HeadCode = 119;
		BYTE XORCode1 = 50;
		BYTE XORCode2 = 132;

		if (Start(IP, Port, CreateWorker, ActiveWorker, Nodelay, HeadCode, XORCode1, XORCode2) == false)
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

		// 2. ������ ����
		// ����, �̹� 0���� ����� �����Ͱ� �ڴʰ� �Դٸ� �����Ѵ�.
		if (LastData[DataType].m_ZeroCount > 0)
		{
			// ī��Ʈ �ϳ� ���̰� ��.
			LastData[DataType].m_ZeroCount--;
		}

		// 0���� ����� �����Ͱ� ���ٸ�, ���� �̹��� ���� �����Ͱ� �� ��.
		else if (LastData[DataType].m_ZeroCount == 0)
		{
			LastData[DataType].m_ServerNo = ServerNo;
			LastData[DataType].m_Value = DataValue;			
		}




		///////////////////////////////////////////////////////////
		//// ������ ������ ����
		///////////////////////////////////////////////////////////
		//int SendVal[12] = { 0, };

		//int a[12];

		//// �ϵ������ CPU ��뷮
		//a[0] = rand() % 5;

		//// �ϵ������ ��� ���� �޸�
		//a[1] = rand() % 80;

		//// �ϵ������ Network Recv
		//a[2] = rand() % 40;

		//// �ϵ������ Network Send
		//a[3] = rand() % 40;

		//// �ϵ������ Nonpaged_Mem
		//a[4] = rand() % 38;

		//// ä�ü��� On/Off
		//a[5] = rand() % 2;

		//// ä�ü��� CPU
		//a[6] = rand() % 5;

		//// ä�ü��� �޸�
		//a[7] = rand() % 10;

		//// PacketPool
		//a[8] = rand() % 100;

		//// SessionAll
		//a[9] = rand() % 100;

		//// Login Player
		//a[10] = rand() % 50;

		//// Room
		//a[11] = rand() % 3;


		///////////////////////////////////////////////////////////
		//// ������ ������ ����
		///////////////////////////////////////////////////////////
		//// ù ��° �׷����� %�� ǥ���ϱ� ������(�ӽ÷�) 100 �̻��� ���� �ʵ��� �����Ѵ�.
		//// ������ ����, +���� -���� �����Ѵ�.
		//int randCheck = rand() % 2;

		//if (randCheck == 0)	// 0�̸� �A��.
		//{
		//	if (0 > (SendVal[0] - a[0]))	// �����ϴµ� �A ���� ���� -��� ���Ѵ�.
		//		SendVal[0] += a[0];
		//	else
		//		SendVal[0] -= a[0];
		//}
		//else   // 1�̸� ���Ѵ�.
		//{
		//	if (100 < (SendVal[0] + a[0]))	// ���ؾ� �ϴµ�, ���� ���� ���� 100�� �Ѿ�� ����.
		//		SendVal[0] -= a[0];
		//	else
		//		SendVal[0] += a[0];
		//}

		//int MonitorIndex = 0;
		//int End = dfMONITOR_DATA_TYPE_END - 1;
		//int DataType = 1;

		//while (MonitorIndex < End)
		//{
		//	if (m_pMonitor[MonitorIndex] != nullptr)
		//	{
		//		m_pMonitor[MonitorIndex]->InsertData(SendVal[0], SERVER_1, DataType);
		//	}
		//	++MonitorIndex;
		//}

		//DataType++;

		//// 2~10�� ������ ó��
		//for (int i = 1; i < 12; ++i)
		//{
		//	// ������ ����, +���� -���� �����Ѵ�.
		//	randCheck = rand() % 2;

		//	if (randCheck == 0)	// 0�̸� �A��.
		//	{
		//		if (0 > (SendVal[i] - a[i]))	// �����ϴµ� �A ���� -��� ���Ѵ�.
		//			SendVal[i] += a[i];
		//		else
		//			SendVal[i] -= a[i];
		//	}
		//	else   // 1�̸� ���Ѵ�.
		//	{
		//		SendVal[i] += a[i];
		//	}

		//	MonitorIndex = 0;

		//	while (MonitorIndex < End)
		//	{
		//		if (m_pMonitor[MonitorIndex] != nullptr)
		//		{
		//			m_pMonitor[MonitorIndex]->InsertData(SendVal[i], SERVER_1, DataType);
		//		}
		//		++MonitorIndex;
		//	}

		//	DataType++;
		//	if (DataType == 6)
		//		DataType += 30;
		//}

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

			// �� �� Ÿ���� ũ����
		default:
			g_MonitorClientDump->Crash();
			break;
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
	{}
}