#include "pch.h"
#include "MatchmakingServer.h"
#include "Parser/Parser_Class.h"
#include "Log/Log.h"
#include "Protocol_Set/CommonProtocol_2.h"		// �ñ������δ� CommonProtocol.h�� �̸� ���� �ʿ�. ������ ä�ü������� �α��� ������ �̿��ϴ� ���������� �־ _2�� ����.


// ---------------------------------------------
// 
// ��ġ����ŷ Net����
//
// ---------------------------------------------
namespace Library_Jingyu
{
	// ��ġ����ŷ Net�������� ����� Net ����ȭ ���� ũ��.
	LONG g_lNET_BUFF_SIZE = 512;

	// ��ġ����ŷ ������
	CCrashDump* gMatchServerDump = CCrashDump::GetInstance();

	// ��ġ����ŷ �α׿�
	CSystemLog* cMatchServerLog = CSystemLog::GetInstance();



	// -------------------------------------
	// ���ο����� ����ϴ� �Լ�
	// -------------------------------------

	// ���Ͽ��� Config ���� �о����
	// 
	// Parameter : config ����ü
	// return : ���������� ���� �� true
	//		  : �� �ܿ��� false
	bool Matchmaking_Net_Server::SetFile(stConfigFile* pConfig)
	{
		Parser Parser;

		// ���� �ε�
		try
		{
			Parser.LoadFile(_T("MatchmakingServer_Config.ini"));
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
		// Matchmaking Net������ config �о����
		////////////////////////////////////////////////////////

		// ���� ���� -------------------------
		if (Parser.AreaCheck(_T("MATCHNETSERVER")) == false)
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


		////////////////////////////////////////////////////////
		// MatchDB�� Config �о����
		////////////////////////////////////////////////////////

		// ���� ���� -------------------------
		if (Parser.AreaCheck(_T("MATCHDB")) == false)
			return false;

		// DBIP
		if (Parser.GetValue_String(_T("DBIP"), pConfig->DB_IP) == false)
			return false;

		// DBUser
		if (Parser.GetValue_String(_T("DBUser"), pConfig->DB_User) == false)
			return false;

		// DBPassword
		if (Parser.GetValue_String(_T("DBPassword"), pConfig->DB_Password) == false)
			return false;

		// DBName
		if (Parser.GetValue_String(_T("DBName"), pConfig->DB_Name) == false)
			return false;

		// DBPort
		if (Parser.GetValue_Int(_T("DBPort"), &pConfig->DB_Port) == false)
			return false;

		// Matchmaking DB�� ��� ��Ʈ��Ʈ
		if (Parser.GetValue_Int(_T("MatchDBHeartbeat"), &pConfig->MatchDBHeartbeat) == false)
			return false;


		////////////////////////////////////////////////////////
		// Matchmaking Lan Ŭ���� ���� �о����
		////////////////////////////////////////////////////////

		// ���� ���� -------------------------
		if (Parser.AreaCheck(_T("MATCHLANCLIENT")) == false)
			return false;		

		// ServerNo
		if (Parser.GetValue_Int(_T("ServerNo"), &pConfig->ServerNo) == false)
			return false;

		// MasterToken 
		TCHAR tcToken[33];
		if (Parser.GetValue_String(_T("MasterToken"), tcToken) == false)
			return false;

		// Ŭ�� ���� ���� char������ ������ ������ ��ȯ�ؼ� ������ �־���Ѵ�.
		int len = WideCharToMultiByte(CP_UTF8, 0, tcToken, lstrlenW(tcToken), NULL, 0, NULL, NULL);
		WideCharToMultiByte(CP_UTF8, 0, tcToken, lstrlenW(tcToken), pConfig->MasterToken, len, NULL, NULL);

		return true;
	}
	   

	// -------------------------------------
	// ������
	// -------------------------------------

	// matchmakingDB�� ���� �ð����� ��Ʈ��Ʈ�� ��� ������.
	UINT WINAPI Matchmaking_Net_Server::DBHeartbeatThread(LPVOID lParam)
	{
		Matchmaking_Net_Server* g_This = (Matchmaking_Net_Server*)lParam;

		// ����� �̺�Ʈ ���÷� �޾Ƶα�
		HANDLE hExitEvent = g_This->hDB_HBThread_ExitEvent;

		// ��Ʈ��Ʈ �ð� �޾Ƶα�
		int iHeartbeatTime = g_This->m_stConfig.MatchDBHeartbeat;

		// DBConenct �޾Ƶα�
		CBConnectorTLS* pMatchDBcon = g_This->m_MatchDB_Connector;

		// ���� �ѹ� �޾Ƶα�
		int iServerNo = g_This->m_stConfig.ServerNo;

		while (1)
		{
			// ��ġ����ŷDB�� ��Ʈ��Ʈ ���� �ð���ŭ �ܴ�.
			DWORD Check = WaitForSingleObject(hExitEvent, iHeartbeatTime);

			// �̻��� ��ȣ���
			if (Check == WAIT_FAILED)
			{
				DWORD Error = GetLastError();
				cMatchServerLog->LogSave(L"MatchServer", CSystemLog::en_LogLevel::LEVEL_SYSTEM, L"DBHeartbeatThread Exit Error!!!(%d)", Error);
				break;
			}

			// ����, ���� ��ȣ�� �Դٸ� ������ ����.
			else if (Check == WAIT_OBJECT_0)
				break;


			// ������ �ƴ϶�� ���� �Ѵ�.

			// 1. ���� ������
			char cQurey[200] = "UPDATE `matchmaking_status`.`server` SET `heartbeat` = NOW() WHERE `serverno` = %d\0";
			pMatchDBcon->Query(cQurey, iServerNo);

			// 2. ���� ��� ��������
			MYSQL_ROW rowData = pMatchDBcon->FetchRow();

			// �����ߴٸ� Crash
			if (rowData == NULL)
			{
				// ���� �޽��� ��´�.
				cMatchServerLog->LogSave(L"MatchServer", CSystemLog::en_LogLevel::LEVEL_SYSTEM, 
					L"DBHeartbeatThread QueryError. Errno : %d, Msg : %s", pMatchDBcon->GetLastError(), pMatchDBcon->GetLastErrorMsg());

				gMatchServerDump->Crash();
			}

			// 3. ���� ����
			pMatchDBcon->FreeResult();

		}
	}








	// -------------------------------------
	// �ܺο��� ��� ������ �Լ�
	// -------------------------------------

	// ���� Start �Լ�
	//
	// Parameter : ����
	// return : ���� �� false ����
	bool Matchmaking_Net_Server::ServerStart()
	{
		// ------------------- �ݼ��� ����
		if (Start(m_stConfig.BindIP, m_stConfig.Port, m_stConfig.CreateWorker, m_stConfig.ActiveWorker, m_stConfig.CreateAccept, m_stConfig.Nodelay, m_stConfig.MaxJoinUser,
			m_stConfig.HeadCode, m_stConfig.XORCode1, m_stConfig.XORCode2) == false)
		{
			cMatchServerLog->LogSave(L"MatchServer", CSystemLog::en_LogLevel::LEVEL_SYSTEM, L"ServerOpen Fail...");
			return false;
		}

		// ���� ���� �α� ���		
		cMatchServerLog->LogSave(L"MatchServer", CSystemLog::en_LogLevel::LEVEL_SYSTEM, L"ServerOpen...");

		return true;
	}

	// ��ġ����ŷ ���� ���� �Լ�
	//
	// Parameter : ����
	// return : ����
	void Matchmaking_Net_Server::ServerStop()
	{
		// �ݼ��� ��ž (����Ʈ, ��Ŀ ����)
		Stop();		

		// ���� ���� �α� ���		
		cMatchServerLog->LogSave(L"ChatServer", CSystemLog::en_LogLevel::LEVEL_SYSTEM, L"ServerStop...");
	}


	// -------------------------------------
	// NetServer�� �����Լ�
	// -------------------------------------

	bool Matchmaking_Net_Server::OnConnectionRequest(TCHAR* IP, USHORT port)
	{
		return true;
	}

	void Matchmaking_Net_Server::OnClientJoin(ULONGLONG SessionID)
	{}

	void Matchmaking_Net_Server::OnClientLeave(ULONGLONG SessionID)
	{}

	void Matchmaking_Net_Server::OnRecv(ULONGLONG SessionID, CProtocolBuff_Net* Payload)
	{}

	void Matchmaking_Net_Server::OnSend(ULONGLONG SessionID, DWORD SendSize)
	{}

	void Matchmaking_Net_Server::OnWorkerThreadBegin()
	{}

	void Matchmaking_Net_Server::OnWorkerThreadEnd()
	{}

	void Matchmaking_Net_Server::OnError(int error, const TCHAR* errorStr)
	{}




	// -------------------------------------
	// �����ڿ� �Ҹ���
	// -------------------------------------

	// ������
	Matchmaking_Net_Server::Matchmaking_Net_Server()
		:CNetServer()
	{
		// ------------------- Config���� ����		
		if (SetFile(&m_stConfig) == false)
			gMatchServerDump->Crash();

		// DBThread ����� �̺�Ʈ ����
		// 
		// �ڵ� ���� Event 
		// ���� ���� �� non-signalled ����
		// �̸� ���� Event	
		hDB_HBThread_ExitEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

		// MatchmakingDB�� Connect
		m_MatchDB_Connector = new CBConnectorTLS(m_stConfig.DB_IP, m_stConfig.DB_User, m_stConfig.DB_Password, m_stConfig.DB_Name, m_stConfig.DB_Port);

		// ------------------- �α� ������ ���� ����
		cMatchServerLog->SetDirectory(L"MatchServer");
		cMatchServerLog->SetLogLeve((CSystemLog::en_LogLevel)m_stConfig.LogLevel);
	}

	// �Ҹ���
	Matchmaking_Net_Server::~Matchmaking_Net_Server()
	{
		// ������ ���� �������̶�� ���� ����
		// Net ���̺귯���� ������������ �˸� ��.
		if (GetServerState() == true)
			ServerStop();

		// MatchDB Ŀ���� �Ҹ�
		delete m_MatchDB_Connector;

		// DBThread ����� �̺�Ʈ ��ȯ
		CloseHandle(hDB_HBThread_ExitEvent);
	}
	
}