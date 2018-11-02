#include "pch.h"
#include "MatchmakingServer.h"
#include "Parser/Parser_Class.h"
#include "Log/Log.h"
#include "Protocol_Set/CommonProtocol_2.h"		// �ñ������δ� CommonProtocol.h�� �̸� ���� �ʿ�. ������ ä�ü������� �α��� ������ �̿��ϴ� ���������� �־ _2�� ����.

#include "rapidjson\document.h"
#include "rapidjson\writer.h"
#include "rapidjson\stringbuffer.h"

#include <process.h>
#include <strsafe.h>

#include <mmsystem.h>
#pragma comment(lib, "winmm.lib")

using namespace rapidjson;

// ��¿� ������ ------------------------

// �α��� ��Ŷ �޾��� ��, ����.
LONG g_lTokenError;	
LONG g_lAccountError;
LONG g_lTempError;
LONG g_lVerError;

// �α��ο� ������ ���� ��
LONG g_lLoginUser;

// �÷��̾� ����ü �Ҵ� ��
LONG g_lstPlayer_AllocCount;

// Net �������� ī��Ʈ ���� ��.
extern ULONGLONG g_ullAcceptTotal;
extern LONG	  g_lAcceptTPS;
extern LONG	g_lSendPostTPS;

// ��Ŷ ����ȭ ���� ��� �� (Net)
extern LONG g_lAllocNodeCount;




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

		// BindIP
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
		// �⺻ CONFIG �о����
		////////////////////////////////////////////////////////

		// ���� ���� -------------------------
		if (Parser.AreaCheck(_T("CONFIG")) == false)
			return false;		

		// ServerNo
		if (Parser.GetValue_Int(_T("ServerNo"), &m_iServerNo) == false)
			return false;

		// VerCode
		if (Parser.GetValue_Int(_T("VerCode"), &m_uiVer_Code) == false)
			return false;

		// ServerIP
		TCHAR tcServerIP[20];
		if (Parser.GetValue_String(_T("ServerIP"), tcServerIP) == false)
			return false;

		// ServerIP UTF-8�� ��ȯ
		int len = WideCharToMultiByte(CP_UTF8, 0, tcServerIP, lstrlenW(tcServerIP), NULL, 0, NULL, NULL);
		WideCharToMultiByte(CP_UTF8, 0, tcServerIP, lstrlenW(tcServerIP), m_cServerIP, len, NULL, NULL);

		// MasterToken 
		TCHAR tcToken[33];
		if (Parser.GetValue_String(_T("MasterToken"), tcToken) == false)
			return false;

		// Ŭ�� ���� ���� char������ ������ ������ ��ȯ�ؼ� ������ �־���Ѵ�.
		len = WideCharToMultiByte(CP_UTF8, 0, tcToken, lstrlenW(tcToken), NULL, 0, NULL, NULL);
		WideCharToMultiByte(CP_UTF8, 0, tcToken, lstrlenW(tcToken), MasterToken, len, NULL, NULL);

		return true;
	}
	   
	// ��ġ����ŷ DB��, �ʱ� ������ Insert�ϴ� �Լ�.
	// �̹�, �����Ͱ� �����ϴ� ���, ������ Update�Ѵ�.
	// 
	// Parameter : ����
	// return : ����
	void Matchmaking_Net_Server::ServerInfo_DBInsert()
	{		
		// 1. Insert ���� ������.	
		char cQurey[200] = "INSERT INTO `matchmaking_status`.`server` VALUES(%d, '%s', %d, 0, NOW())\0";
		m_MatchDBcon->Query_Save(cQurey, m_iServerNo, m_cServerIP, m_stConfig.Port);


		// 2. ���� Ȯ��
		int Error = m_MatchDBcon->GetLastError();

		// ������ 0�� �ƴ� ���, Insert�� ������ ��.
		if (Error != 0)
		{
			// ���� �ߺ� Ű �������, �̹� �����Ͱ� �����Ѵٴ� ��. Update ���� ����
			if (Error == 1062)
			{
				char cUpdateQuery[200] = "UPDATE `matchmaking_status`.`server` SET `heartbeat` = NOW(), `connectuser` = 10 WHERE `serverno` = %d\0";
				m_MatchDBcon->Query_Save(cUpdateQuery, m_iServerNo);

				// ���� üũ
				Error = m_MatchDBcon->GetLastError();				
				if (Error != 0)
				{
					// ������ �߻������� ���� ��� ũ����
					cMatchServerLog->LogSave(L"MatchServer", CSystemLog::en_LogLevel::LEVEL_SYSTEM,
						L"ServerInfo_DBInsert() --> Query Error. %s(%d)", m_MatchDBcon->GetLastErrorMsg(), m_MatchDBcon->GetLastError());

					gMatchServerDump->Crash();
				}
			}
			
			else
			{
				// �ߺ�Ű�� �ƴ� ������ �߻������� �α� ���� �� ũ����
				cMatchServerLog->LogSave(L"MatchServer", CSystemLog::en_LogLevel::LEVEL_SYSTEM,
					L"ServerInfo_DBInsert() --> Query Error. %s(%d)", m_MatchDBcon->GetLastErrorMsg(), m_MatchDBcon->GetLastError());

				gMatchServerDump->Crash();
			}
		}	
	}


	// -------------------------------------
	// ������
	// -------------------------------------

	// matchmakingDB�� ���� �ð����� ��Ʈ��Ʈ�� ��� ������.
	//
	// �ܺο���, �̺�Ʈ�� ���� ��Ű�⵵ �Ѵ�.
	UINT WINAPI Matchmaking_Net_Server::DBHeartbeatThread(LPVOID lParam)
	{
		Matchmaking_Net_Server* g_This = (Matchmaking_Net_Server*)lParam;

		// ����� �̺�Ʈ, ���ϱ�� �̺�Ʈ ���÷� �޾Ƶα�
		HANDLE hExitEvent[2] = { g_This->hDB_HBThread_ExitEvent, g_This->hDB_HBThread_WorkerEvent };

		// ��Ʈ��Ʈ ���� �ð� �޾Ƶα�
		int iHeartbeatTime = g_This->m_stConfig.MatchDBHeartbeat;

		// ���� �ѹ� �޾Ƶα�
		int iServerNo = g_This->m_iServerNo;

		// umapPlayer ������ �޾Ƶα�. connectuser ���� �뵵
		unordered_map<ULONGLONG, stPlayer*>* pUmapPlayer = &g_This->m_umapPlayer;

		// MatchmakingDB�� ����. ���÷� �޾Ƶ�
		CBConnectorTLS* pMatchDBcon = g_This->m_MatchDBcon;

		while (1)
		{
			// ��ġ����ŷDB�� ��Ʈ��Ʈ ���� �ð���ŭ �ܴ�.
			DWORD Check = WaitForMultipleObjects(2, hExitEvent, FALSE, iHeartbeatTime);

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

			// -------------------------------
			// ������ �ƴ϶�� ���� �Ѵ�. (�ð��� �Ǿ ����ų�, ������ ���� ���װų�)
			// -------------------------------

			// 1. ���� ������
			char cQurey[200] = "UPDATE `matchmaking_status`.`server` SET `heartbeat` = NOW(), `connectuser` = %lld WHERE `serverno` = %d\0";
			pMatchDBcon->Query_Save(cQurey, pUmapPlayer->size(), iServerNo);

			// 2. ���� Ȯ��
			int Error = pMatchDBcon->GetLastError();
			if (Error != 0)
			{
				// ������ �߻��ߴٸ� �α� ����� ũ����
				cMatchServerLog->LogSave(L"MatchServer", CSystemLog::en_LogLevel::LEVEL_SYSTEM,
					L"DBHeartbeatThread() --> Query Error. %s(%d)", pMatchDBcon->GetLastErrorMsg(), pMatchDBcon->GetLastError());

				gMatchServerDump->Crash();
			}
		}

		return 0;
	}




	// -------------------------------------
	// �ڷᱸ�� �߰�,����,�˻��� �Լ�
	// -------------------------------------

	// Player ���� �ڷᱸ����, ���� �߰�
	// ���� umap���� ������
	// 
	// Parameter : SessionID, stPlayer*
	// return : �߰� ���� ��, true
	//		  : SessionID�� �ߺ��� ��(�̹� �������� ����) false		
	bool Matchmaking_Net_Server::InsertPlayerFunc(ULONGLONG SessionID, stPlayer* insertPlayer)
	{
		// 1. umap�� �߰�		
		AcquireSRWLockExclusive(&m_srwlPlayer);		// ------- Exclusive ��

		auto ret = m_umapPlayer.insert(make_pair(SessionID, insertPlayer));

		ReleaseSRWLockExclusive(&m_srwlPlayer);		// ------- Exclusive ���

		// 2. �ߺ��� Ű�� �� false ����.
		if (ret.second == false)
			return false;

		// 3. �ƴϸ� true ����
		return true;
	}


	// Player ���� �ڷᱸ������, ���� �˻�
	// ���� map���� ������
	// 
	// Parameter : SessionID
	// return : �˻� ���� ��, stPalyer*
	//		  : �˻� ���� �� nullptr
	Matchmaking_Net_Server::stPlayer*  Matchmaking_Net_Server::FindPlayerFunc(ULONGLONG SessionID)
	{
		// 1. umap���� �˻�
		AcquireSRWLockShared(&m_srwlPlayer);		// ------- Shared ��

		auto FindPlayer = m_umapPlayer.find(SessionID);

		ReleaseSRWLockShared(&m_srwlPlayer);		// ------- Shared ���

		// 2. �˻� ���� �� nullptr ����
		if (FindPlayer == m_umapPlayer.end())
			return nullptr;

		// 3. �˻� ���� ��, ã�� stPlayer* ����
		return FindPlayer->second;
	}


	// Player ���� �ڷᱸ������, ���� ���� (�˻� �� ����)
	// ���� umap���� ������
	// 
	// Parameter : SessionID
	// return : ���� ��, ���ŵ� ���� stPalyer*
	//		  : �˻� ���� ��(���������� ���� ����) nullptr
	Matchmaking_Net_Server::stPlayer* Matchmaking_Net_Server::ErasePlayerFunc(ULONGLONG SessionID)
	{
		// 1. umap���� ���� �˻�
		AcquireSRWLockExclusive(&m_srwlPlayer);		// ------- Exclusive ��

		auto FindPlayer = m_umapPlayer.find(SessionID);
		if (FindPlayer == m_umapPlayer.end())
		{
			ReleaseSRWLockExclusive(&m_srwlPlayer);		// ------- Exclusive ���
			return nullptr;
		}
		
		// 2. �����Ѵٸ�, ������ �� �޾Ƶα�
		stPlayer* ret = FindPlayer->second;

		// 3. �ʿ��� ����
		m_umapPlayer.erase(FindPlayer);

		ReleaseSRWLockExclusive(&m_srwlPlayer);		// ------- Exclusive ���

		return ret;
	}





	// -------------------------------------
	// ��Ŷ ó���� �Լ�
	// -------------------------------------

	// ��ġ����ŷ ������ �α��� ��û
	//
	// Parameter : SessionID, Payload
	// return : ����. ������ �����, ���ο��� throw ����
	void Matchmaking_Net_Server::Packet_Match_Login(ULONGLONG SessionID, CProtocolBuff_Net* Payload)
	{
		// 1. ������
		INT64 AccountNo;
		char Token[64];
		UINT Ver_Code;

		Payload->GetData((char*)&AccountNo, 8);
		Payload->GetData(Token, 64);
		Payload->GetData((char*)&Ver_Code, 4);


		// 2. AccountNo�� DB���� ã�ƿ���
		TCHAR RequestBody[2000];
		TCHAR Body[1000];

		ZeroMemory(RequestBody, sizeof(RequestBody));
		ZeroMemory(Body, sizeof(Body));

		swprintf_s(Body, _Mycountof(Body), L"{\"accountno\" : %lld}", AccountNo);
		if (m_HTTP_Post->HTTP_ReqANDRes((TCHAR*)_T("Contents/Select_account.php"), Body, RequestBody) == false)
			gMatchServerDump->Crash();

		// Json������ �Ľ��ϱ� (UTF-16)
		GenericDocument<UTF16<>> Doc;
		Doc.Parse(RequestBody);		



		// 3. DB ��� üũ
		int iResult = Doc[_T("result")].GetInt();

		// ����� 1�� �ƴ϶��, 
		if (iResult != 1)
		{
			WORD Type = en_PACKET_CS_MATCH_RES_LOGIN;
			BYTE Status;

			// ����� -10�� ��� (ȸ������ ��ü�� �ȵǾ� ����)	-----------------
			if (iResult == -10)
			{
				Status = 3;
				InterlockedIncrement(&g_lAccountError);
			}

			// �� �� ��Ÿ ������ ���
			else
			{
				Status = 4;
				InterlockedIncrement(&g_lTempError);
			}

			CProtocolBuff_Net* SendData = CProtocolBuff_Net::Alloc();

			SendData->PutData((char*)&Type, 2);
			SendData->PutData((char*)&Status, 1);

			// ������ ����
			SendPacket(SessionID, SendData, TRUE);
			return;
		}		


		// 4. ����� 1�̶��, ��ūŰ�� ���� üũ
		// ��ūŰ �� --------------------
		const TCHAR* tDBToekn = Doc[_T("sessionkey")].GetString();

		char DBToken[64];
		int len = (int)_tcslen(tDBToekn);
		WideCharToMultiByte(CP_UTF8, 0, tDBToekn, (int)_tcslen(tDBToekn), DBToken, len, NULL, NULL);
		
		if (memcmp(DBToken, Token, 64) != 0)
		{
			// ��ū�� �ٸ���� status 2(��ū ����)�� ������.
			InterlockedIncrement(&g_lTokenError);

			WORD Type = en_PACKET_CS_MATCH_RES_LOGIN;
			BYTE Status = 2;			

			CProtocolBuff_Net* SendData = CProtocolBuff_Net::Alloc();

			SendData->PutData((char*)&Type, 2);
			SendData->PutData((char*)&Status, 1);

			// ������ ����
			SendPacket(SessionID, SendData, TRUE);
			return;
		}

		// ���� �� --------------------
		if (m_uiVer_Code != Ver_Code)
		{
			// ������ �ٸ���� status 5(���� ����)�� ������.
			InterlockedIncrement(&g_lVerError);

			WORD Type = en_PACKET_CS_MATCH_RES_LOGIN;
			BYTE Status = 5;

			CProtocolBuff_Net* SendData = CProtocolBuff_Net::Alloc();

			SendData->PutData((char*)&Type, 2);
			SendData->PutData((char*)&Status, 1);

			// ������ ����
			SendPacket(SessionID, SendData, TRUE);
			return;
		}




		// 5. ������� ������ �������� �÷��̾�. ���� ����
		// 1) �÷��̾� �˻�.
		stPlayer* NowPlayer = FindPlayerFunc(SessionID);
		if (NowPlayer == nullptr)
		{
			// �˻� ���� �� ���� ������ ����.
			cMatchServerLog->LogSave(L"MatchServer", CSystemLog::en_LogLevel::LEVEL_SYSTEM,
				L"Packet_Match_Login Player Not Find. SessionID : %lld, AccountNo : %lld, ", 
				SessionID, AccountNo);

			gMatchServerDump->Crash();
		}

		// 2) AccountNo ����
		NowPlayer->m_i64AccountNo = AccountNo;

		// 3) ClinetKey ����
		// ���� 4����Ʈ�� ServerNo. ���� 4����Ʈ�� m_ClientKeyAdd�� ���� ����.
		UINT64 TempKey = InterlockedIncrement(&m_ClientKeyAdd);
		NowPlayer->m_ui64ClientKey = ((TempKey << 16) | m_iServerNo);

		// 4) �α��� ���·� ����
		NowPlayer->m_bLoginCheck = true;

		// �α��� ���� �� ����
		InterlockedIncrement(&g_lLoginUser);


		// 6. ���� ��Ŷ ����
		WORD Type = en_PACKET_CS_MATCH_RES_LOGIN;
		BYTE Status = 1; // ����

		CProtocolBuff_Net* SendData = CProtocolBuff_Net::Alloc();

		SendData->PutData((char*)&Type, 2);
		SendData->PutData((char*)&Status, 1);

		SendPacket(SessionID, SendData);
		return;
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
		// ------------------- ��ġ����ŷ DB�� �ʱ� ������ ����
		ServerInfo_DBInsert();

		// ------------------- ��Ʈ��Ʈ ������ ����
		hDB_HBThread = (HANDLE)_beginthreadex(NULL, 0, DBHeartbeatThread, this, 0, 0);
		if (hDB_HBThread == 0)
		{
			cMatchServerLog->LogSave(L"MatchServer", CSystemLog::en_LogLevel::LEVEL_SYSTEM, 
				L"ServerStart() --> HeartBeatThread Create Fail...");

			return false;
		}

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

		// ��Ʈ��Ʈ ������ ���� ��ȣ
		SetEvent(hDB_HBThread_ExitEvent);

		// ��Ʈ��Ʈ ������ ���� ���
		if (WaitForSingleObject(hDB_HBThread, INFINITE) == WAIT_FAILED)
		{
			// ���ῡ �����Ѵٸ�, ���� ���
			DWORD Error = GetLastError();
			cMatchServerLog->LogSave(L"MatchServer", CSystemLog::en_LogLevel::LEVEL_SYSTEM, 
				L"ServerStop() --> DBHeartbeatThread Exit Error!!!(%d)", Error);

			gMatchServerDump->Crash();
		}

		// ���� ���� �α� ���		
		cMatchServerLog->LogSave(L"ChatServer", CSystemLog::en_LogLevel::LEVEL_SYSTEM, L"ServerStop...");
	}


	// ��¿� �Լ�
	//
	// Parameter : ����
	// return : ����
	void Matchmaking_Net_Server::ShowPrintf()
	{
		// ȭ�� ����� �� ����
		/*
		SessionNum : 	- NetServer �� ���Ǽ�
		PacketPool_Net : 	- �ܺο��� ��� ���� Net ����ȭ ������ ��

		PlayerData_Pool :	- Player ����ü �Ҵ緮
		Player Count : 		- Contents ��Ʈ Player ����

		Accept Total :		- Accept ��ü ī��Ʈ (accept ���Ͻ� +1)
		Accept TPS :		- Accept ó�� Ƚ��
		Send TPS			- �ʴ� Send�Ϸ� Ƚ��. (�Ϸ��������� ����)

		Net_BuffChunkAlloc_Count : - Net ����ȭ ���� �� Alloc�� ûũ �� (�ۿ��� ������� ûũ ��)
		Chat_MessageChunkAlloc_Count : - �÷��̾� �� Alloc�� ûũ �� (�ۿ��� ������� ûũ ��)

		TokenError : 		- DB���� ã�ƿ� ��ū�� �α��� ��û�� ������ ���� ��ū�� �ٸ�
		AccountError : 		- Selecet.account.php�� ���� ���ȴµ�, -10 ������ ��(ȸ���������� ���� ����)
		TempError :			- Selecet.account.php�� ���� ���ȴµ�, -10 �ܿ� ��Ÿ ������ ��
		VerError :			- �α��� ��û�� ������ ���� VerCode�� ������ ����ִ� VerCode�� �ٸ�

		*/

		LONG AccpetTPS = InterlockedExchange(&g_lAcceptTPS, 0);
		LONG SendTPS = InterlockedExchange(&g_lSendPostTPS, 0);

		printf("========================================================\n"
			"SessionNum : %lld\n"
			"PacketPool_Net : %d\n\n"

			"PlayerData_Pool : %d\n"
			"Player Count : %lld\n\n"

			"Accept Total : %lld\n"
			"Accept TPS : %d\n"
			"Send TPS : %d\n\n"

			"Net_BuffChunkAlloc_Count : %d (Out : %d)\n"
			"Chat_PlayerChunkAlloc_Count : %d (Out : %d)\n\n"

			"TokenError : %d\n"
			"AccountError : %d\n"
			"TempError : %d\n"
			"VerError : %d\n\n"

			"========================================================\n\n",

			// ------------ ��ġ����ŷ Net ������
			GetClientCount(), g_lAllocNodeCount,
			g_lstPlayer_AllocCount, m_umapPlayer.size(),
			g_ullAcceptTotal, AccpetTPS, SendTPS,
			CProtocolBuff_Net::GetChunkCount(), CProtocolBuff_Net::GetOutChunkCount(),
			m_PlayerPool->GetAllocChunkCount(), m_PlayerPool->GetOutChunkCount(),
			g_lTokenError, g_lAccountError, g_lTempError, g_lVerError );		

	}



	// -------------------------------------
	// NetServer�� �����Լ�
	// -------------------------------------

	bool Matchmaking_Net_Server::OnConnectionRequest(TCHAR* IP, USHORT port)
	{
		return true;
	}

	// ���� ����(�α����� �ȵ� ����)
	void Matchmaking_Net_Server::OnClientJoin(ULONGLONG SessionID)
	{
		// 1. stPlayer TLS���� ����ü �Ҵ����
		stPlayer* NowPlayer = m_PlayerPool->Alloc();
		InterlockedIncrement(&g_lstPlayer_AllocCount);

		// 2. stPlayer�� SessionID ����
		NowPlayer->m_ullSessionID = SessionID;	

		// 3. Player ���� umap�� �߰�.
		if (InsertPlayerFunc(SessionID, NowPlayer) == false)
		{
			cMatchServerLog->LogSave(L"MatchServer", CSystemLog::en_LogLevel::LEVEL_SYSTEM,
				L"OnClientJoin() --> duplication SessionID...(SessionID : %lld)", SessionID);

			gMatchServerDump->Crash();
		}

		// 4. umap�� ���� �� üũ.
		// umap�� �������� m_ChangeConnectUser +100 ���� ���ٸ�, ���� �� ���� 100�� �̻� ���°�.
		// ��ġ����ŷ DB�� ��Ʈ��Ʈ �Ѵ�.	
		// ī��Ʈ�� ��Ȯ�ϰ� �ϱ� ���� �� ���	
		size_t NowumapCount = (int)m_umapPlayer.size();
		
		AcquireSRWLockExclusive(&m_srwlChangeConnectUser);	// Exclusive �� -----------

		if (m_ChangeConnectUser + 100 <= NowumapCount)
		{
			m_ChangeConnectUser = NowumapCount;
			ReleaseSRWLockExclusive(&m_srwlChangeConnectUser);	// Exclusive ��� -----------

			SetEvent(hDB_HBThread_WorkerEvent);
		}
		
		else
			ReleaseSRWLockExclusive(&m_srwlChangeConnectUser);	// Exclusive ��� -----------
	}

	// ���� ����
	void Matchmaking_Net_Server::OnClientLeave(ULONGLONG SessionID)
	{
		// 1. umap���� ���� ����
		stPlayer* ErasePlayer = ErasePlayerFunc(SessionID);
		if (ErasePlayer == nullptr)
		{
			cMatchServerLog->LogSave(L"MatchServer", CSystemLog::en_LogLevel::LEVEL_SYSTEM,
				L"OnClientLeave() --> Erase Fail.. (SessionID : %lld)", SessionID);

			gMatchServerDump->Crash();
		}

		// 2. umap ���� �� üũ.
		// umap�� �������� m_ChangeConnectUser - 100 ���� ���ٸ�, ���� �� ���� 100�� �̻� ������.
		// ��ġ����ŷ DB�� ��Ʈ��Ʈ �Ѵ�.	
		// ī��Ʈ�� ��Ȯ�ϰ� �ϱ� ���� �� ���	
		size_t NowumapCount = m_umapPlayer.size();

		AcquireSRWLockExclusive(&m_srwlChangeConnectUser);	// Exclusive �� -----------

		if (m_ChangeConnectUser - 100 >= NowumapCount)
		{
			m_ChangeConnectUser = NowumapCount;
			ReleaseSRWLockExclusive(&m_srwlChangeConnectUser);	// Exclusive ��� -----------

			SetEvent(hDB_HBThread_WorkerEvent);
		}

		else
			ReleaseSRWLockExclusive(&m_srwlChangeConnectUser);	// Exclusive ��� -----------

		// 3. ����, �α��� ������ ������ �����ٸ� (�α��� ��Ŷ���� ���� ����) g_LoginUser--
		if (ErasePlayer->m_bLoginCheck == true)
			InterlockedDecrement(&g_lLoginUser);

		// 4. ���� �α��� ���¸� false�� ����.
		ErasePlayer->m_bLoginCheck = false;

		// 5. �÷��̾� ����ü ��ȯ
		m_PlayerPool->Free(ErasePlayer);
		InterlockedDecrement(&g_lstPlayer_AllocCount);		
	}

	// ���ο� ��Ŷ ����
	void Matchmaking_Net_Server::OnRecv(ULONGLONG SessionID, CProtocolBuff_Net* Payload)
	{
		// 1. ��Ŷ�� Type ����
		WORD Type;
		Payload->GetData((char*)&Type, 2);

		// 2. Type�� ���� �б⹮ ó��
		try
		{
			switch (Type)
			{
				// ��ġ����ŷ ������ �α��� ��û
			case en_PACKET_CS_MATCH_REQ_LOGIN:
				Packet_Match_Login(SessionID, Payload);
				break;

				// �� ���� ��û
			case en_PACKET_CS_MATCH_REQ_GAME_ROOM:
				break;

				// ��Ʋ ������ �濡 ���� ���� �˸�
			case en_PACKET_CS_MATCH_REQ_GAME_ROOM_ENTER:
				break;

				// �̻��� Ÿ���� ��Ŷ�� ���� ���´�.
			default:				
				TCHAR ErrStr[1024];
				StringCchPrintf(ErrStr, 1024, _T("OnRecv(). TypeError. Type : %d, SessionID : %lld"),
					Type, SessionID);

				throw CException(ErrStr);
			}

		}
		catch (CException& exc)
		{
			// �α� ��� (�α� ���� : ����)
			cMatchServerLog->LogSave(L"MatchServer", CSystemLog::en_LogLevel::LEVEL_ERROR, L"%s",
				(TCHAR*)exc.GetExceptionText());	

			// Crash
			gMatchServerDump->Crash();

			// ���� ���� ��û
			//Disconnect(SessionID);
		}
		

	}

	void Matchmaking_Net_Server::OnSend(ULONGLONG SessionID, DWORD SendSize)
	{}

	void Matchmaking_Net_Server::OnWorkerThreadBegin()
	{}

	void Matchmaking_Net_Server::OnWorkerThreadEnd()
	{}

	void Matchmaking_Net_Server::OnError(int error, const TCHAR* errorStr)
	{
		// �α� ��� (�α� ���� : ����)
		cMatchServerLog->LogSave(L"MatchServer", CSystemLog::en_LogLevel::LEVEL_ERROR, L"%s (ErrorCode : %d)",
			errorStr, error);		
	}




	// -------------------------------------
	// �����ڿ� �Ҹ���
	// -------------------------------------

	// ������
	Matchmaking_Net_Server::Matchmaking_Net_Server()
		:CNetServer()
	{
		// m_ClientKeyAdd ����
		m_ClientKeyAdd = 0;

		// m_ChangeConnectUser ����
		m_ChangeConnectUser = 0;

		// ------------------- Config���� ����		
		if (SetFile(&m_stConfig) == false)
			gMatchServerDump->Crash();

		// ------------------- �α� ������ ���� ����
		cMatchServerLog->SetDirectory(L"MatchServer");
		cMatchServerLog->SetLogLeve((CSystemLog::en_LogLevel)m_stConfig.LogLevel);

		// DBThread ����� �̺�Ʈ, �� ��Ű��� �̺�Ʈ ����
		// 
		// �ڵ� ���� Event 
		// ���� ���� �� non-signalled ����
		// �̸� ���� Event	
		hDB_HBThread_ExitEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
		hDB_HBThread_WorkerEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

		// SRWLOCK �ʱ�ȭ
		InitializeSRWLock(&m_srwlPlayer);
		InitializeSRWLock(&m_srwlChangeConnectUser);

		// stPlayer ����ü�� �ٷ�� TLS �����Ҵ�
		m_PlayerPool = new CMemoryPoolTLS<stPlayer>(0, false);

		// DB_Connector TLS ����
		// MatchmakingDB�� ����
		m_MatchDBcon = new CBConnectorTLS(m_stConfig.DB_IP, m_stConfig.DB_User, m_stConfig.DB_Password,	m_stConfig.DB_Name, m_stConfig.DB_Port);
		
		// HTTP_Exchange �����Ҵ�
		m_HTTP_Post = new HTTP_Exchange((TCHAR*)_T("127.0.0.1"), 80);

		// �÷��̾ �����ϴ� umap�� �뷮�� �Ҵ��صд�.
		m_umapPlayer.reserve(m_stConfig.MaxJoinUser);		

		// �ð�
		timeBeginPeriod(1);
	}

	// �Ҹ���
	Matchmaking_Net_Server::~Matchmaking_Net_Server()
	{
		// ������ ���� �������̶�� ���� ����
		// Net ���̺귯���� ������������ �˸� ��.
		if (GetServerState() == true)
			ServerStop();

		// ����ü �޽��� TLS �޸� Ǯ ��������
		delete m_PlayerPool;

		// DBConnector ��������
		delete m_MatchDBcon;

		// HTTP_Exchange ��������
		delete m_HTTP_Post;

		// DBThread ����� �̺�Ʈ ��ȯ
		CloseHandle(hDB_HBThread_ExitEvent);

		// DBThread �� ��Ű��� �̺�Ʈ ��ȯ
		CloseHandle(hDB_HBThread_WorkerEvent);

		// �ð�
		timeEndPeriod(1);
	}
	
}