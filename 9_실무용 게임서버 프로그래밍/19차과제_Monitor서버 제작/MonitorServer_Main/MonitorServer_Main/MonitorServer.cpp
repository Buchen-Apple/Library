#include "pch.h"
#include "MonitorServer.h"
#include "Log\Log.h"
#include "DB_Connector\DB_Connector.h"
#include "Parser\Parser_Class.h"

#include <process.h>
#include <strsafe.h>
#include <time.h>


namespace Library_Jingyu
{
	CCrashDump* g_MonitorDump = CCrashDump::GetInstance();

	// �α� ���� �������� �ϳ� �ޱ�.
	CSystemLog* cMonitorLibLog = CSystemLog::GetInstance();

	// ������ ���� �� ���� No
	enum en_Monitor_Type
	{
		dfMONITOR_ETC = 2,		// ä�ü��� �� ��� ����.
		dfMONITOR_CHATSERVER = 3		// ä�� ����
	};



	// ������
	CLan_Monitor_Server::CLan_Monitor_Server()
	{
		// ----------------- DB ���� ���� ����ü�� Ÿ�԰� ���� �̸� �����صα�
		int i = 0;
		while (i < dfMONITOR_DATA_TYPE_END - 1)
		{
			m_stDBInfo[i].m_iType = i + 1;

			// �ϵ������ 
			if (i < dfMONITOR_DATA_TYPE_SERVER_NONPAGED_MEMORY)
			{
				// �̸��� ���� No ����
				strcpy_s(m_stDBInfo[i].m_cServerName, 64, "HardWare");
				m_stDBInfo[i].m_iServerNo = dfMONITOR_ETC;
			}

			// ��ġ����ŷ �������
			else if (i < dfMONITOR_DATA_TYPE_MATCH_MATCHSUCCESS)
			{
				// �̸��� ���� No ����
				strcpy_s(m_stDBInfo[i].m_cServerName, 64, "MatchServer");
				m_stDBInfo[i].m_iServerNo = dfMONITOR_ETC;
			}

			// ������ �������
			else if (i < dfMONITOR_DATA_TYPE_MASTER_BATTLE_STANDBY_ROOM)
			{
				// �̸��� ���� No ����
				strcpy_s(m_stDBInfo[i].m_cServerName, 64, "MasterServer");
				m_stDBInfo[i].m_iServerNo = dfMONITOR_ETC;
			}

			// ��Ʋ �������
			else if (i < dfMONITOR_DATA_TYPE_BATTLE_ROOM_PLAY)
			{
				// �̸��� ���� No ����
				strcpy_s(m_stDBInfo[i].m_cServerName, 64, "BattleServer");
				m_stDBInfo[i].m_iServerNo = dfMONITOR_ETC;
			}

			// ä�� �������
			else
			{
				// �̸��� ���� No ����
				strcpy_s(m_stDBInfo[i].m_cServerName, 64, "ChatServer");
				m_stDBInfo[i].m_iServerNo = dfMONITOR_CHATSERVER;
			}

			++i;
		}

		// ----------------- ���Ͽ��� ���� �о����
		if (SetFile(&m_stConfig) == false)
			g_MonitorDump->Crash();

		// �� �ʱ�ȭ
		InitializeSRWLock(&srwl);
		InitializeSRWLock(&DBInfoSrwl);

		// ------------------- �α� ������ ���� ����
		// Net������ ��ġ ���� �ʿ�
		cMonitorLibLog->SetDirectory(L"MonitorServer");
		cMonitorLibLog->SetLogLeve((CSystemLog::en_LogLevel)m_stConfig.LanLogLevel);

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

		// ���ø� ���̺� �̸�
		char TemplateName[30] = "monitorlog_template";

		while (1)
		{
			// 1�п� �ѹ� ����� ���� �Ѵ�.
			DWORD ret = WaitForSingleObject(hEvent, 60000);

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
				if (g_This->m_stDBInfo[i].m_iTotal == 0)
				{
					// ������ ������, DB�� ���� ���Ѵ�.
					// !! ������ ���ٴ°���, 1�е��� ������ �ϳ��� �ȿԴٴ� ���ε�, �����ȵ�. !!
					// !! ������ �����ִ� ��� ����� ��~~~�� ���� !! 
					++i;
					continue;
				}

				// 2. ������ �ִٸ�, ���������� �� ä���
				else
				{
					TempDBInfo[TempCount].m_iType = g_This->m_stDBInfo[i].m_iType;
					strcpy_s(TempDBInfo[TempCount].m_cServerName, 64, g_This->m_stDBInfo[i].m_cServerName);

					TempDBInfo[TempCount].m_iValue = g_This->m_stDBInfo[i].m_iValue;
					TempDBInfo[TempCount].m_iMin = g_This->m_stDBInfo[i].m_iMin;
					TempDBInfo[TempCount].m_iMax = g_This->m_stDBInfo[i].m_iMax;
					TempDBInfo[TempCount].m_iAvr = (float)(g_This->m_stDBInfo[i].m_iTotal / g_This->m_stDBInfo[i].m_iTotalCount);
					TempDBInfo[TempCount].m_iServerNo = g_This->m_stDBInfo[i].m_iServerNo;
				}

				// 3. �� �ʱ�ȭ
				g_This->m_stDBInfo[i].init();

				++TempCount;

				++i;
			}

			ReleaseSRWLockShared(&g_This->DBInfoSrwl);		// ������� ��� -----------


			// -----------------------------------------------------
			// DB �̸� �����. ��/�� ���� ����ȴ�.
			// -----------------------------------------------------	
			char DBTableName[30] = { 0, };

			// 1. ���� �ð��� �� ������ ���
			time_t timer = time(NULL);

			// 2. �� ������ �ð��� �и��Ͽ� ����ü�� �ֱ� 
			struct tm NowTime;
			if (localtime_s(&NowTime, &timer) != 0)
				g_MonitorDump->Crash();

			// 3. �̸� �����. [DBName_Year-Mon]�� ���·� ���������.
			StringCbPrintfA(DBTableName, 30, "monitorlog_%04d-%02d", NowTime.tm_year + 1900, NowTime.tm_mon + 1);


			// -----------------------------------------------------
			// DB�� ����
			// -----------------------------------------------------

			i = 0;
			while (i < TempCount)
			{
				// TempCount��ŭ ���鼭 DB�� Write�Ѵ�.

				// ���� �����. -----------
				char query[1024] = "INSERT INTO `";
				StringCbCatA(query, 1024, DBTableName);
				StringCbCatA(query, 1024, "` (`logtime`, `serverno`, `servername`, `type`, `value`, `min`, `max`, `avg`) VALUE (now(), %d, '%s', %d, %d, %d, %d, %.2f)");
								
				// DB�� ������ -----------
				// ���̺��� ���� ���, ���̺� �������� �ϴ� �Լ� ȣ��.
				cConnector.Query_Save(TemplateName, DBTableName, query, TempDBInfo[i].m_iServerNo, TempDBInfo[i].m_cServerName, TempDBInfo[i].m_iType, TempDBInfo[i].m_iValue,
					TempDBInfo[i].m_iMin, TempDBInfo[i].m_iMax, TempDBInfo[i].m_iAvr);

				i++;
			}

			printf("AAAA!!!\n");
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
		if (Start(m_stConfig.LanBindIP, m_stConfig.LanPort, m_stConfig.LanCreateWorker, m_stConfig.LanActiveWorker, m_stConfig.LanCreateAccept,
			m_stConfig.LanNodelay, m_stConfig.LanMaxJoinUser) == false)
			return false;

		// DBWrite ������ ����
		m_hDBWriteThread = (HANDLE)_beginthreadex(NULL, 0, DBWriteThread, this, 0, 0);

		return true;
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
		// Monitor LanServer Config �о����
		////////////////////////////////////////////////////////
		// ���� ���� -------------------------
		if (Parser.AreaCheck(_T("MONITORSERVERLAN")) == false)
			return false;

		// IP
		if (Parser.GetValue_String(_T("LanBindIP"), pConfig->LanBindIP) == false)
			return false;

		// Port
		if (Parser.GetValue_Int(_T("LanPort"), &pConfig->LanPort) == false)
			return false;

		// ���� ��Ŀ ��
		if (Parser.GetValue_Int(_T("LanCreateWorker"), &pConfig->LanCreateWorker) == false)
			return false;

		// Ȱ��ȭ ��Ŀ ��
		if (Parser.GetValue_Int(_T("LanActiveWorker"), &pConfig->LanActiveWorker) == false)
			return false;

		// ���� ����Ʈ
		if (Parser.GetValue_Int(_T("LanCreateAccept"), &pConfig->LanCreateAccept) == false)
			return false;

		// Nodelay
		if (Parser.GetValue_Int(_T("LanNodelay"), &pConfig->LanNodelay) == false)
			return false;

		// �ִ� ���� ���� ���� ��
		if (Parser.GetValue_Int(_T("LanMaxJoinUser"), &pConfig->LanMaxJoinUser) == false)
			return false;

		// �α� ����
		if (Parser.GetValue_Int(_T("LanLogLevel"), &pConfig->LanLogLevel) == false)
			return false;


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


		return true;
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
		Type = Type - 1;	// �迭�� ��� �����ϱ� ������ -1�� �ؾ��Ѵ�. �迭�� 0���� ����.

		int Value;
		Payload->GetData((char*)&Value, 4);

		int TimeStamp;
		Payload->GetData((char*)&TimeStamp, 4);


		// ���� ����
		AcquireSRWLockExclusive(&DBInfoSrwl);	// ------------- ��

		// Value�� 0�̸� ���� �������� ����.
		// ���� Send�� �Ѵ�.
		if (Value != 0)
		{
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
		}		

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