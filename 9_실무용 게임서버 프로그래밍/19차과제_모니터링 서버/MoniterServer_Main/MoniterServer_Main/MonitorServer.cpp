#include "pch.h"
#include "MonitorServer.h"
#include "Log\Log.h"
#include "DB_Connector\DB_Connector.h"
#include "Parser\Parser_Class.h"

#include <process.h>
#include <strsafe.h>

namespace Library_Jingyu
{
	CCrashDump* g_MonitorDump = CCrashDump::GetInstance();

	// �α� ���� �������� �ϳ� �ޱ�.
	CSystemLog* cMonitorLibLog = CSystemLog::GetInstance();




	// ������
	CLan_Monitor_Server::CLan_Monitor_Server()
	{
		// ----------------- ���Ͽ��� ���� �о����
		if (SetFile(&m_stConfig) == false)
			g_MonitorDump->Crash();

		// �� �ʱ�ȭ
		InitializeSRWLock(&srwl);
		InitializeSRWLock(&DBInfoSrwl);

		// ------------------- �α� ������ ���� ����
		// Net������ ��ġ ���� �ʿ�
		cMonitorLibLog->SetDirectory(L"MonitorServer");
		//cMonitorLibLog->SetLogLeve((CSystemLog::en_LogLevel)m_stConfig.LogLevel);

		// DBWirteThread ����� �̺�Ʈ ����
		// 
		// �ڵ� ���� Event 
		// ���� ���� �� non-signalled ����
		// �̸� ���� Event	
		m_hDBWriteThreadExitEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

	}


	// �Ҹ���
	CLan_Monitor_Server::~CLan_Monitor_Server()
	{
		// DBWirteThread ����� �̺�Ʈ ��ȯ
		CloseHandle(m_hDBWriteThreadExitEvent);
	}






	// DB�� ���� ���� ������
	// 1�п� 1ȸ DB�� Insert
	UINT WINAPI CLan_Monitor_Server::DBWriteThread(LPVOID lParam)
	{
		CLan_Monitor_Server* g_This = (CLan_Monitor_Server*)lParam;

		// ���� �̺�Ʈ �޾Ƶα�
		HANDLE hEvent = g_This->m_hDBWriteThreadExitEvent;

		// DB�� Write�ϱ����� �ӽ÷� �����ص� ����
		stDBWriteInfo TempDBInfo[dfMONITOR_DATA_TYPE_END - 1];
		int TempCount;

		// �� �� ���� �迭�� ������� �ִ��� üũ��.
		// ��ü ��ȸ �� ���
		int Index = dfMONITOR_DATA_TYPE_END - 1;

		// DB�� ����
		CBConnectorTLS cConnector(g_This->m_stConfig.DB_IP, g_This->m_stConfig.DB_User,
			g_This->m_stConfig.DB_Password, g_This->m_stConfig.DB_Name, g_This->m_stConfig.DB_Port);

		while (1)
		{
			// 1�п� �ѹ� ����� ���� �Ѵ�.
			DWORD ret = WaitForSingleObject(hEvent, 10000);

			// �̻��� ��ȣ���
			if (ret == WAIT_FAILED)
			{
				DWORD Error = GetLastError();
				printf("DBWriteThread Exit Error!!! (%d) \n", Error);
				break;
			}

			// ����, ���� ��ȣ�� �Դٸ� ������ ����.
			else if (ret == WAIT_OBJECT_0)
				break;

			// -----------------------------------------------------
			// �װ� �ƴ϶�� 1���� �Ǿ� ��� ���̴� ���� �Ѵ�.
			// -----------------------------------------------------

			AcquireSRWLockShared(&g_This->DBInfoSrwl);		// ������� �� -----------

			int i = 0;
			TempCount = 0;
			while (i < Index)
			{
				// 1. �ش� �迭�� ������ �ִ� �迭���� Ȯ��
				if (g_This->m_stDBInfo[i].m_iValue == 0)
				{
					++i;
					continue;
				}

				// 2. Temp ����ü �迭��, ���� �ֱ�
				TempDBInfo->m_iValue = g_This->m_stDBInfo[i].m_iValue;
				TempDBInfo->m_iMax = g_This->m_stDBInfo[i].m_iMax;
				TempDBInfo->m_iMin = g_This->m_stDBInfo[i].m_iMin;
				TempDBInfo->m_iAvr = g_This->m_stDBInfo[i].m_iTotal / g_This->m_stDBInfo[i].m_iTotalCount;

				// ServerNo ����
				// ----- �ϵ���� ������ ���
				if (0 <= i && i <= dfMONITOR_DATA_TYPE_SERVER_NONPAGED_MEMORY - 1)
				{

				}

				// TempCount ����.
				TempCount++;

				// 3. ���� �迭�� �� �ʱ�ȭ
				g_This->m_stDBInfo[i].init();

				++i;
			}

			ReleaseSRWLockShared(&g_This->DBInfoSrwl);		// ������� ��� -----------


			// -----------------------------------------------------
			// DB�� ����
			// -----------------------------------------------------
			// TempCount��ŭ ���鼭 ���� ���� �� ������.

			i = 0;
			while (i < TempCount)
			{	

				// 1. ���� �����
				char query[1024] = "INSERT INTO `monitorlog` (`logtime`, `serverNo`, `servername`, `type`, `value`, `min`, `max`, `avg`) "
														"VALUE(now(), %d, `%s`, %d, %d, %d, %d, %d)";

				// 2. DB�� ������
				cConnector.Query_Save(query, );






				i++;
			}

		}

		return 0;
	}


	// -----------------------
	// �ܺο��� ���� ������ ��� �Լ�
	// -----------------------

	// ����͸� Lan ���� ����
	// ���������� CLanServer�� Start�Լ� ȣ��
	//
	// Parameter : ����
	// return :  ����
	bool CLan_Monitor_Server::ServerStart()
	{
		// ������ ����
		//Start();

		// DBWrite ������ ����
		m_hDBWriteThread = (HANDLE)_beginthreadex(NULL, 0, DBWriteThread, this, 0, 0);

	}

	// ����͸� Lan ���� ����
	// ���������� CLanServer�� stop�Լ� ȣ��
	//
	// Parameter : ����
	// return :  ����
	void CLan_Monitor_Server::ServerStop()
	{
		// 1. ������ ����
		Stop();

		// 2. DBWrite ������ ����
		SetEvent(m_hDBWriteThreadExitEvent);

		// ���� ���
		DWORD ret = WaitForSingleObject(m_hDBWriteThreadExitEvent, INFINITE);

		// �������ᰡ �ƴϸ� ����
		if (ret != WAIT_OBJECT_0)
		{
			g_MonitorDump->Crash();
		}

		// 4. DBWirte Thread �ڵ� ��ȯ
		CloseHandle(m_hDBWriteThread);
	}




	// -----------------------
	// ���� ��� �Լ�
	// -----------------------

	// ���Ͽ��� Config ���� �о����
	// 
	// 
	// Parameter : config ����ü
	// return : ���������� ���� �� true
	//		  : �� �ܿ��� false
	bool CLan_Monitor_Server::SetFile(stConfigFile* pConfig)
	{
		Parser Parser;

		// ���� �ε�
		try
		{
			Parser.LoadFile(_T("MonitorServer_Config.ini"));
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
		// DB config �о����
		////////////////////////////////////////////////////////
		// ���� ���� -------------------------
		if (Parser.AreaCheck(_T("MONITORDBCONFIG")) == false)
			return false;


		// IP
		if (Parser.GetValue_String(_T("DBIP"), pConfig->DB_IP) == false)
			return false;

		// User
		if (Parser.GetValue_String(_T("DBUser"), pConfig->DB_User) == false)
			return false;

		// Password
		if (Parser.GetValue_String(_T("DBPassword"), pConfig->DB_Password) == false)
			return false;

		// Name
		if (Parser.GetValue_String(_T("DBName"), pConfig->DB_Name) == false)
			return false;

		// Port
		if (Parser.GetValue_Int(_T("DBPort"), &pConfig->DB_Port) == false)
			return false;
	}



	// ����͸� Ŭ���̾�Ʈ�κ��� ���� ���� ����
	// ���� ���� ��, �� ���鿡�� ����
	//
	// Parameter : Lan����ȭ ����
	// return : ����
	void CLan_Monitor_Server::DataUpdatePacket(CProtocolBuff_Lan* Payload)
	{
		// ������
		BYTE Type;
		Payload->GetData((char*)&Type, 1);

		int Value;
		Payload->GetData((char*)&Value, 4);

		int TimeStamp;
		Payload->GetData((char*)&TimeStamp, 4);


		// ���� ����
		AcquireSRWLockExclusive(&DBInfoSrwl);	// ------------- ��

		// 1. Value ����
		m_stDBInfo[Type].m_iValue = Value;

		// 2. Min ����
		if (m_stDBInfo[Type].m_iMin > Value)
			m_stDBInfo[Type].m_iMin = Value;

		// 3. Max ����
		if (m_stDBInfo[Type].m_iMax < Value)
			m_stDBInfo[Type].m_iMax = Value;

		// 4. Total�� �� �߰�
		m_stDBInfo[Type].m_iTotal += Value;

		// 5. ī��Ʈ ����
		m_stDBInfo[Type].m_iTotalCount++;

		ReleaseSRWLockExclusive(&DBInfoSrwl);	// ------------- ���


		// ��� ���� ������ �����ϱ�
		AcquireSRWLockShared(&srwl);			// ------------- ������� ��

		WORD SendType = en_PACKET_SS_MONITOR_DATA_UPDATE;

		int i = 0;
		while (i < m_iArrayCount)
		{
			CProtocolBuff_Lan* SendBuff = CProtocolBuff_Lan::Alloc();

			SendBuff->PutData((char*)&SendType, 2);

			BYTE ServerNo = m_arrayJoinServer[i].m_bServerNo;
			SendBuff->PutData((char*)&ServerNo, 1);
			SendBuff->PutData((char*)&Type, 1);
			SendBuff->PutData((char*)&Value, 4);
			SendBuff->PutData((char*)&TimeStamp, 4);

			SendPacket(m_arrayJoinServer[i].m_ullSessionID, SendBuff);

			++i;
		}

		ReleaseSRWLockShared(&srwl);			// ------------- ������� ���

	}


	// �α��� ��û ó��
	//
	// Parameter : Lan����ȭ ����
	// return : ����
	void CLan_Monitor_Server::LoginPacket(CProtocolBuff_Lan* Payload)
	{
		// ������
		int ServerNo;
		Payload->GetData((char*)&ServerNo, 4);

		// ���� ���� ����
		AcquireSRWLockExclusive(&srwl);		// ------- ��

		m_arrayJoinServer[m_iArrayCount].m_bServerNo = ServerNo;

		ReleaseSRWLockExclusive(&srwl);		// ------- �� ����
	}



	// -----------------------
	// �����Լ�
	// -----------------------

	// Accept ����, ȣ��ȴ�.
	//
	// parameter : ������ ������ IP, Port
	// return false : Ŭ���̾�Ʈ ���� �ź�
	// return true : ���� ���
	bool CLan_Monitor_Server::OnConnectionRequest(TCHAR* IP, USHORT port)
	{
		return true;
	}

	// ���� �� ȣ��Ǵ� �Լ� (AcceptThread���� Accept �� ȣ��)
	//
	// parameter : ������ �������� �Ҵ�� ����Ű
	// return : ����
	void CLan_Monitor_Server::OnClientJoin(ULONGLONG SessionID)
	{
		// 1. �迭�� ���� �߰�
		AcquireSRWLockExclusive(&srwl);		// ------- ��

		m_arrayJoinServer[m_iArrayCount].m_ullSessionID = SessionID;
		m_iArrayCount++;

		ReleaseSRWLockExclusive(&srwl);		// ------- �� ����
	}

	// ���� ���� �� ȣ��Ǵ� �Լ� (InDIsconnect �ȿ��� ȣ��)
	//
	// parameter : ���� ����Ű
	// return : ����
	void CLan_Monitor_Server::OnClientLeave(ULONGLONG SessionID)
	{
		// 1. �迭���� ���� ����		
		int Tempindex = 0;

		AcquireSRWLockExclusive(&srwl);		// ------- ��

		// ������ ������ 1���̶��, ī��Ʈ�� 1 ���̰� ��
		if (m_iArrayCount == 1)
		{
			m_iArrayCount--;
			ReleaseSRWLockExclusive(&srwl);		// ------- �� ����
			return;
		}

		// ������ 1�� �̻��̶�� ���� ó��
		while (Tempindex < m_iArrayCount)
		{
			// ���ϴ� ������ ã������
			if (m_arrayJoinServer[Tempindex].m_ullSessionID == SessionID)
			{
				// ����, �ش� ������ ��ġ�� �������̶�� ī��Ʈ�� 1 ���̰� ��
				if (Tempindex == (m_iArrayCount - 1))
				{
					m_iArrayCount--;
					break;
				}

				// ������ ��ġ�� �ƴ϶��, ������ ��ġ�� ������ ��ġ�� ���� �� ī��Ʈ ����
				m_arrayJoinServer[Tempindex].m_ullSessionID = m_arrayJoinServer[m_iArrayCount - 1].m_ullSessionID;
				m_arrayJoinServer[Tempindex].m_bServerNo = m_arrayJoinServer[m_iArrayCount - 1].m_bServerNo;

				m_iArrayCount--;
				break;
			}

			Tempindex++;
		}

		ReleaseSRWLockExclusive(&srwl);		// ------- �� ����
	}

	// ��Ŷ ���� �Ϸ� �� ȣ��Ǵ� �Լ�.
	//
	// parameter : ���� ����Ű, ���� ��Ŷ
	// return : ����
	void CLan_Monitor_Server::OnRecv(ULONGLONG SessionID, CProtocolBuff_Lan* Payload)
	{	
		try
		{
			// 1. ��Ŷ Type�˾ƿ���
			WORD Type;
			Payload->GetData((char*)&Type, 2);

			// 2. Type�� ���� ó��
			switch (Type)
			{
				// �α��� ��û ó��
			case en_PACKET_SS_MONITOR_LOGIN:
				LoginPacket(Payload);
				break;

				// ������ ���� ó��
			case en_PACKET_SS_MONITOR_DATA_UPDATE:
				DataUpdatePacket(Payload);
				break;

			default:
				throw CException(_T("OnRecv. Type Error."));
				break;
			}			

		}
		catch (CException& exc)
		{
			char* pExc = exc.GetExceptionText();

			// �α� ��� (�α� ���� : ����)
			cMonitorLibLog->LogSave(L"MonitorServer", CSystemLog::en_LogLevel::LEVEL_ERROR, L"%s",
				(TCHAR*)pExc);

			g_MonitorDump->Crash();
		}


	}

	// ��Ŷ �۽� �Ϸ� �� ȣ��Ǵ� �Լ�
	//
	// parameter : ���� ����Ű, Send �� ������
	// return : ����
	void CLan_Monitor_Server::OnSend(ULONGLONG SessionID, DWORD SendSize)
	{}

	// ��Ŀ �����尡 ��� �� ȣ��Ǵ� �Լ�.
	// GQCS �ٷ� �ϴܿ��� ȣ��
	// 
	// parameter : ����
	// return : ����
	void CLan_Monitor_Server::OnWorkerThreadBegin()
	{}

	// ��Ŀ �����尡 ���� �� ȣ��Ǵ� �Լ�
	// GQCS �ٷ� ������ ȣ��
	// 
	// parameter : ����
	// return : ����
	void CLan_Monitor_Server::OnWorkerThreadEnd()
	{}

	// ���� �߻� �� ȣ��Ǵ� �Լ�.
	//
	// parameter : ���� �ڵ�(���� ������ �����ڵ�� WinGetLastError() �Լ��� ��� ����. ���� ��� 0�� ���ϵ�)
	//			 : ���� �ڵ忡 ���� ��Ʈ��
	// return : ����
	void CLan_Monitor_Server::OnError(int error, const TCHAR* errorStr)
	{}
}