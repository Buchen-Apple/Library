#include "pch.h"
#include "MonitorServer.h"
#include "Log\Log.h"
#include "DB_Connector\DB_Connector.h"
#include "Parser\Parser_Class.h"

#include <process.h>
#include <strsafe.h>
#include <time.h>


// ----------------------
// ����͸� Net ����
// ----------------------
namespace Library_Jingyu
{
	CCrashDump* g_MonitorDump = CCrashDump::GetInstance();

	// �α� ���� �������� �ϳ� �ޱ�.
	CSystemLog* cMonitorLibLog = CSystemLog::GetInstance();

	// -----------------
	// ������
	// -----------------
	CNet_Monitor_Server::CNet_Monitor_Server()
	{
		// ----------------- ���Ͽ��� ���� �о����
		if (SetFile(&m_stConfig) == false)
			g_MonitorDump->Crash();

		// ------------------- �α� ������ ���� ����
		cMonitorLibLog->SetDirectory(L"MonitorServer");
		cMonitorLibLog->SetLogLeve((CSystemLog::en_LogLevel)m_stConfig.LanLogLevel);

		// ----------------- �÷��̾� ���� �ڷᱸ���� Capacity �Ҵ� (�ִ� �÷��̾� �� ��ŭ)
		m_vectorPlayer.reserve(1000);

		// �÷��̾� TLS �޸�Ǯ �����Ҵ�
		// �ϳ��� �ȸ����д�.
		m_PlayerTLS = new CMemoryPoolTLS<stPlayer>(0, false);

		// ����͸� �� ���� �����Ҵ�
		m_CLanServer = new CLan_Monitor_Server;
		m_CLanServer->ParentSet(this);

		// �� �ʱ�ȭ
		InitializeSRWLock(&m_vectorSrwl);
	}

	// -----------------
	// �Ҹ���
	// -----------------
	CNet_Monitor_Server::~CNet_Monitor_Server()
	{
		// ����͸� �� ���� ��������
		delete m_CLanServer;

		// �� ������ �����ִٸ� ���� ����.
		if (GetServerState() == true)
			ServerStop();

		// �÷��̾� TLS �޸�Ǯ ��������
		delete m_PlayerTLS;
	}




	// -----------------------
	// ���ο����� ��� ������ ����Լ�
	// -----------------------

	// ���Ͽ��� Config ���� �о����
	// 
	// Parameter : config ����ü
	// return : ���������� ���� �� true
	//		  : �� �ܿ��� false
	bool CNet_Monitor_Server::SetFile(stConfigFile* pConfig)
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
		// Monitor NetServer Config �о����
		////////////////////////////////////////////////////////
		// ���� ���� -------------------------
		if (Parser.AreaCheck(_T("MONITORSERVERNET")) == false)
			return false;

		// IP
		if (Parser.GetValue_String(_T("BindIP"), pConfig->BindIP) == false)
			return false;

		// Port
		if (Parser.GetValue_Int(_T("Port"), &pConfig->Port) == false)
			return false;

		// ���� ��Ŀ ��
		if (Parser.GetValue_Int(_T("CreateWorker"), &pConfig->CreateWorker) == false)
			return false;

		// Ȱ��ȭ ��Ŀ ��
		if (Parser.GetValue_Int(_T("ActiveWorker"), &pConfig->ActiveWorker) == false)
			return false;

		// ���� ����Ʈ
		if (Parser.GetValue_Int(_T("CreateAccept"), &pConfig->CreateAccept) == false)
			return false;

		// ��� �ڵ�
		if (Parser.GetValue_Int(_T("HeadCode"), &pConfig->HeadCode) == false)
			return false;

		// xorcode1
		if (Parser.GetValue_Int(_T("XorCode1"), &pConfig->XORCode1) == false)
			return false;

		// xorcode2
		if (Parser.GetValue_Int(_T("XorCode2"), &pConfig->XORCode2) == false)
			return false;

		// Nodelay
		if (Parser.GetValue_Int(_T("Nodelay"), &pConfig->Nodelay) == false)
			return false;

		// �ִ� ���� ���� ���� ��
		if (Parser.GetValue_Int(_T("MaxJoinUser"), &pConfig->MaxJoinUser) == false)
			return false;

		// �α� ����
		if (Parser.GetValue_Int(_T("LogLevel"), &pConfig->LogLevel) == false)
			return false;

		// �α��� Ű
		TCHAR tcLoginKey[33];
		if (Parser.GetValue_String(_T("LoginKey"), tcLoginKey) == false)
			return false;

		// Ŭ�� ���� ���� char������ ������ ������ ��ȯ�ؼ� ������ �־���Ѵ�.
		int len = WideCharToMultiByte(CP_UTF8, 0, tcLoginKey, lstrlenW(tcLoginKey), NULL, 0, NULL, NULL);
		WideCharToMultiByte(CP_UTF8, 0, tcLoginKey, lstrlenW(tcLoginKey), m_cLoginKey, len, NULL, NULL);



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
	   	 	
	// �÷��̾� �ڷᱸ���� �÷��̾� �߰�.
	//
	// Parameter : SessionID
	// return : ����
	void CNet_Monitor_Server::InsertPlayer(ULONGLONG SessionID)
	{
		// 1. �÷��̾� ����ü Alloc
		stPlayer* NowPlayer = m_PlayerTLS->Alloc();

		// 2. ���� ����
		NowPlayer->m_ullSessionID = SessionID;
		NowPlayer->m_bLoginCheck = false;

		// 3. ���� �߰�
		AcquireSRWLockExclusive(&m_vectorSrwl);		// �� ------------------		
		m_vectorPlayer.push_back(NowPlayer);
		ReleaseSRWLockExclusive(&m_vectorSrwl);		// ��� ------------------	
	}

	// �÷��̾� �ڷᱸ������ �÷��̾� ����
	//
	// Parameter : SessionID
	// return : ���� �÷��̾� �� �� false.
	bool CNet_Monitor_Server::ErasePlayer(ULONGLONG SessionID)
	{
		AcquireSRWLockExclusive(&m_vectorSrwl);		// �� ------------------

		// 1. �����ϰ��� �ϴ� �÷��̾� ã��
		int Size = (int)m_vectorPlayer.size();		

		// ����, �������� �÷��̾ 1���̰ų� ���� �ڿ� �ִٸ�
		if (Size == 1 || 
			m_vectorPlayer[Size - 1]->m_ullSessionID == SessionID)
		{			
			stPlayer* ErasePlayer = m_vectorPlayer[Size-1];

			// Pop ��, �̸� �޾Ƶ� Player�� Free �Ѵ�.
			m_vectorPlayer.pop_back();
			m_PlayerTLS->Free(ErasePlayer);

			ReleaseSRWLockExclusive(&m_vectorSrwl);		// �� �� ------------------

			return true;
		}		

		// �װ� �ƴ϶��, ��ȸ�ϸ鼭 ������ �÷��̾� ã��
		else
		{
			int i = 0;
			while (i < Size)
			{
				// �����ϰ��� �ϴ� �÷��̾ ã������
				if (m_vectorPlayer[i]->m_ullSessionID == SessionID)
				{
					// ���� �ڿ� �ִ� �÷��̾��� ������ ������ �÷��̾��� ��ġ�� ����
					m_vectorPlayer[i]->m_ullSessionID = m_vectorPlayer[Size - 1]->m_ullSessionID;
					m_vectorPlayer[i]->m_bLoginCheck = m_vectorPlayer[Size - 1]->m_bLoginCheck;
					
					stPlayer* ErasePlayer = m_vectorPlayer[Size - 1];

					// Pop ��, �̸� �޾Ƶ� Player�� Free �Ѵ�.
					m_vectorPlayer.pop_back();
					m_PlayerTLS->Free(ErasePlayer);
					
					ReleaseSRWLockExclusive(&m_vectorSrwl);		// �� �� ------------------

					return true;
				}

				++i;
			}
		}

		ReleaseSRWLockExclusive(&m_vectorSrwl);		// �� �� ------------------

		// ������� ������ ���ϴ� ������ ��ã����. false ����
		return false;
	}

	// ������ Ŭ��鿡�� ���� �Ѹ��� (��ε� ĳ����)
	//
	// Parameter : ����No, ������ Ÿ��, ������ ��, Ÿ�ӽ�����
	// return : ����
	void CNet_Monitor_Server::DataBroadCasting(BYTE ServerNo, BYTE DataType, int DataValue, int TimeStamp)
	{
		CProtocolBuff_Net* SendBuff = CProtocolBuff_Net::Alloc();

		// 1. ���� ��Ŷ �����
		WORD Type = en_PACKET_CS_MONITOR_TOOL_DATA_UPDATE;

		SendBuff->PutData((char*)&Type, 2);
		SendBuff->PutData((char*)&ServerNo, 1);
		SendBuff->PutData((char*)&DataType, 1);
		SendBuff->PutData((char*)&DataValue, 4);
		SendBuff->PutData((char*)&TimeStamp, 4);

		// 2. ��ε� ĳ����
		AcquireSRWLockShared(&m_vectorSrwl);	// ���� �� ----------------

		int Size = (int)m_vectorPlayer.size();

		// ������ ���� 1�� �̻� ������ ������
		if (Size > 0)
		{
			int i = 0;

			while (i < Size)
			{
				// �α��� ó���� �÷��̾��� ���� ��Ŷ ����
				if (m_vectorPlayer[i]->m_bLoginCheck == true)
				{
					// ref ī��Ʈ 1 ����
					SendBuff->Add();

					// Send
					SendPacket(m_vectorPlayer[i]->m_ullSessionID, SendBuff);
				}

				++i;
			}
		}

		ReleaseSRWLockShared(&m_vectorSrwl);	// ���� ��� ----------------

		// �� ������ Free
		CProtocolBuff_Net::Free(SendBuff);
	}
	
	// �α��� ��û ��Ŷ ó��
	//
	// Parameter : ����ID, Net ����ȭ����
	void CNet_Monitor_Server::LoginPakcet(ULONGLONG SessionID, CProtocolBuff_Net* Payload)
	{
		// 1. ������
		char LoginKey[32];		
		Payload->GetData((char*)LoginKey, 32);

		// 2. �α��� Ű ��
		// �α��� Ű�� ���ٸ�
		if (memcmp(m_cLoginKey, LoginKey, 32) == 0)
		{
			// 1. ���ɰ�, �α��� ���·� ����
			AcquireSRWLockExclusive(&m_vectorSrwl);		// �� -----------

			int Size = (int)m_vectorPlayer.size();

			int i = 0;
			bool bFlag = false;	// ���ϴ� ������ ã�Ƽ� ���¸� �����ߴ��� ����
			while (i < Size)
			{
				if (m_vectorPlayer[i]->m_ullSessionID == SessionID)
				{
					bFlag = true;
					m_vectorPlayer[i]->m_bLoginCheck = true;
					break;
				}
				++i;
			}

			ReleaseSRWLockExclusive(&m_vectorSrwl);		// ��� -----------

			// ������� �Դµ� Flag�� false���, ���ӵ� ���� ����?�� �迭�� ���� �ִ���?
			// ���� �̻���. Crash.
			if (bFlag == false)
				g_MonitorDump->Crash();

			// 2. ���� ��Ŷ ����
			CProtocolBuff_Net* SendBuff = CProtocolBuff_Net::Alloc();

			WORD Type = en_PACKET_CS_MONITOR_TOOL_RES_LOGIN;
			BYTE Status = dfMONITOR_TOOL_LOGIN_OK;

			SendBuff->PutData((char*)&Type, 2);
			SendBuff->PutData((char*)&Status, 1);

			SendPacket(SessionID, SendBuff);
		}

		// �α��� Ű�� �ٸ��ٸ� ������Ŷ ����
		else
		{
			CProtocolBuff_Net* SendBuff = CProtocolBuff_Net::Alloc();

			WORD Type = en_PACKET_CS_MONITOR_TOOL_RES_LOGIN;
			BYTE Status = dfMONITOR_TOOL_LOGIN_ERR_SESSIONKEY;

			SendBuff->PutData((char*)&Type, 2);
			SendBuff->PutData((char*)&Status, 1);

			SendPacket(SessionID, SendBuff);
		}

	}



	// -----------------------
	// �ܺο��� ��� ������ �Լ�
	// -----------------------

	// ����͸� Net���� ����
	// ���ο����� CNetServer�� Start �Լ� ȣ��
	//
	// Parameter : ����
	// return : ����
	bool CNet_Monitor_Server::ServerStart()
	{
		// ����͸� �� ���� ����
		if (m_CLanServer->LanServerStart() == false)
			g_MonitorDump->Crash();

		// ����͸� �� ���� ����
		if (Start(m_stConfig.BindIP, m_stConfig.Port, m_stConfig.CreateWorker, m_stConfig.ActiveWorker, m_stConfig.CreateAccept,
			m_stConfig.Nodelay, m_stConfig.MaxJoinUser, m_stConfig.HeadCode, m_stConfig.XORCode1, m_stConfig.XORCode2) == false)
		{
			g_MonitorDump->Crash();
		}		

		// ���� ���� �α� ���		
		cMonitorLibLog->LogSave(L"MonitorServer", CSystemLog::en_LogLevel::LEVEL_SYSTEM, L"ServerOpen...");

		return true;
	}

	// ����͸� Net���� ����
	// ���ο����� CNetServer�� Stop �Լ� ȣ��
	//
	// Parameter : ����
	// return : ����
	void CNet_Monitor_Server::ServerStop()
	{
		// ������ ����
		m_CLanServer->LanServerStop();			

		// �ݼ��� ����
		Stop();			

		// ���� ���� �α� ���		
		cMonitorLibLog->LogSave(L"MonitorServer", CSystemLog::en_LogLevel::LEVEL_SYSTEM, L"ServerStop...");
	}




	/////////////////////////////
	// �����Լ�
	/////////////////////////////

	// Accept ����, ȣ��ȴ�.
	//
	// parameter : ������ ������ IP, Port
	// return false : Ŭ���̾�Ʈ ���� �ź�
	// return true : ���� ���
	bool CNet_Monitor_Server::OnConnectionRequest(TCHAR* IP, USHORT port)
	{
		return true;
	}

	// ���� �� ȣ��Ǵ� �Լ� (AcceptThread���� Accept �� ȣ��)
	//
	// parameter : ������ �������� �Ҵ�� ����Ű
	// return : ����
	void CNet_Monitor_Server::OnClientJoin(ULONGLONG SessionID)
	{
		// �ڷᱸ���� �÷��̾� �߰�
		InsertPlayer(SessionID);
	}

	// ���� ���� �� ȣ��Ǵ� �Լ� (InDIsconnect �ȿ��� ȣ��)
	//
	// parameter : ���� ����Ű
	// return : ����
	void CNet_Monitor_Server::OnClientLeave(ULONGLONG SessionID)
	{
		// ���ϴ� ������ ��ã�Ҵٸ� Crash()
		if (ErasePlayer(SessionID) == false)
			g_MonitorDump->Crash();
	}

	// ��Ŷ ���� �Ϸ� �� ȣ��Ǵ� �Լ�.
	//
	// parameter : ���� ����Ű, ���� ��Ŷ
	// return : ����
	void CNet_Monitor_Server::OnRecv(ULONGLONG SessionID, CProtocolBuff_Net* Payload)
	{
		try
		{
			// 1. ������
			WORD Type;

			Payload->GetData((char*)&Type, 2);

			switch (Type)
			{
				// �α��� ��û
			case en_PACKET_CS_MONITOR_TOOL_REQ_LOGIN:
				LoginPakcet(SessionID, Payload);
				break;

				// �� �ܿ��� ���� ����
			default:
				throw CException(_T("CNet_Monitor_Server. OnRecv() --> Type Error!!!"));
			}


		}
		catch (CException& exc)
		{
			// �α� ��� (�α� ���� : ����)
			 cMonitorLibLog->LogSave(L"MonitorServer", CSystemLog::en_LogLevel::LEVEL_ERROR, L"%s",
				 (TCHAR*)exc.GetExceptionText());

			Disconnect(SessionID);
		}

	}

	// ��Ŷ �۽� �Ϸ� �� ȣ��Ǵ� �Լ�
	//
	// parameter : ���� ����Ű, Send �� ������
	// return : ����
	void CNet_Monitor_Server::OnSend(ULONGLONG SessionID, DWORD SendSize)
	{}

	// ��Ŀ �����尡 ��� �� ȣ��Ǵ� �Լ�.
	// GQCS �ٷ� �ϴܿ��� ȣ��
	// 
	// parameter : ����
	// return : ����
	void CNet_Monitor_Server::OnWorkerThreadBegin()
	{}

	// ��Ŀ �����尡 ���� �� ȣ��Ǵ� �Լ�
	// GQCS �ٷ� ������ ȣ��
	// 
	// parameter : ����
	// return : ����
	void CNet_Monitor_Server::OnWorkerThreadEnd()
	{}

	// ���� �߻� �� ȣ��Ǵ� �Լ�.
	//
	// parameter : ���� �ڵ�(���� ������ �����ڵ�� WinGetLastError() �Լ��� ��� ����. ���� ��� 0�� ���ϵ�)
	//			 : ���� �ڵ忡 ���� ��Ʈ��
	// return : ����
	void CNet_Monitor_Server::OnError(int error, const TCHAR* errorStr)
	{}
}

// ----------------------
// ����͸� Lan ����
// ----------------------
namespace Library_Jingyu
{
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

		// �� �ʱ�ȭ
		InitializeSRWLock(&srwl);
		InitializeSRWLock(&DBInfoSrwl);

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
		// �� ������ �����ִٸ� ����
		if (GetServerState() == true)
			LanServerStop();

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
		CBConnectorTLS cConnector(g_This->m_ParentThis->m_stConfig.DB_IP, g_This->m_ParentThis->m_stConfig.DB_User,
			g_This->m_ParentThis->m_stConfig.DB_Password, g_This->m_ParentThis->m_stConfig.DB_Name, g_This->m_ParentThis->m_stConfig.DB_Port);

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
			StringCchPrintfA(DBTableName, 30, "monitorlog_%04d-%02d", NowTime.tm_year + 1900, NowTime.tm_mon + 1);


			// -----------------------------------------------------
			// DB�� ����
			// -----------------------------------------------------

			i = 0;
			while (i < TempCount)
			{
				// TempCount��ŭ ���鼭 DB�� Write�Ѵ�.

				// ���� �����. -----------
				char query[1024] = "INSERT INTO `";
				StringCchCatA(query, 1024, DBTableName);
				StringCchCatA(query, 1024, "` (`logtime`, `serverno`, `servername`, `type`, `value`, `min`, `max`, `avg`) VALUE (now(), %d, '%s', %d, %d, %d, %d, %.2f)");
								
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
	bool CLan_Monitor_Server::LanServerStart()
	{
		// ������ ����
		if (Start(m_ParentThis->m_stConfig.LanBindIP, m_ParentThis->m_stConfig.LanPort, m_ParentThis->m_stConfig.LanCreateWorker, m_ParentThis->m_stConfig.LanActiveWorker, 
			m_ParentThis->m_stConfig.LanCreateAccept, m_ParentThis->m_stConfig.LanNodelay, m_ParentThis->m_stConfig.LanMaxJoinUser) == false)
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
	void CLan_Monitor_Server::LanServerStop()
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
	   	  
	// �θ�(NetServer)�� This ���� �Լ�
	//
	// Parameter : ����͸� �ݼ����� This
	void CLan_Monitor_Server::ParentSet(CNet_Monitor_Server* NetServer)
	{
		m_ParentThis = NetServer;
	}



	// ����͸� Ŭ���̾�Ʈ�κ��� ���� ���� ����
	// ���� ���� ��, �� ���鿡�� ����
	//
	// Parameter : Lan����ȭ ����
	// return : ����
	void CLan_Monitor_Server::DataUpdatePacket(CProtocolBuff_Lan* Payload)
	{
		// 1. ������
		BYTE Type;
		Payload->GetData((char*)&Type, 1);
		Type = Type - 1;	// �迭�� ��� �����ϱ� ������ -1�� �ؾ��Ѵ�. �迭�� 0���� ����.

		int Value;
		Payload->GetData((char*)&Value, 4);

		int TimeStamp;
		Payload->GetData((char*)&TimeStamp, 4);

		

		// 2. ���� ����
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

		// 6. Send�� ���� No �޾Ƶα�
		BYTE ServerNo = m_stDBInfo[Type].m_iServerNo;

		ReleaseSRWLockExclusive(&DBInfoSrwl);	// ------------- ���	

		// 3. ���� ���� ������
		// ������ Type�� 1 ���ҽ��ױ� ������ �������Ѿ� Ÿ�� ��ȣ�� �ȴ�.
		m_ParentThis->DataBroadCasting(ServerNo, Type + 1, Value, TimeStamp);

	}


	// �α��� ��û ó��
	//
	// Parameter : Lan����ȭ ����
	// return : ����
	void CLan_Monitor_Server::LoginPacket(ULONGLONG SessionID, CProtocolBuff_Lan* Payload)
	{
		// ������
		int ServerNo;
		Payload->GetData((char*)&ServerNo, 4);

		int i = 0;

		// ���� ���� ����
		AcquireSRWLockExclusive(&srwl);		// ------- ��
				
		while (i < m_iArrayCount)
		{
			// ���� ID �˻�
			if (m_arrayJoinServer[i].m_ullSessionID == SessionID)
			{
				m_arrayJoinServer[i].m_bServerNo = ServerNo;
				break;
			}
			++i;
		}

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
				LoginPacket(SessionID, Payload);
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