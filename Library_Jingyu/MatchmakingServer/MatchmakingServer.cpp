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

// ��Ʋ �� ���� ���� ��Ŷ�� �Ⱥ����� ���� Ŭ���̾�Ʈ�� ��
LONG g_lNot_BattleRoom_Enter;

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

		// Matchmaking DB�� Connect ���� �����ϴ� �����ο�
		if (Parser.GetValue_Int(_T("MatchDBConnectUserChange"), &m_iMatchDBConnectUserChange) == false)
			return false;



		////////////////////////////////////////////////////////
		// MatchClient�� Config �о����
		////////////////////////////////////////////////////////

		// ���� ���� -------------------------
		if (Parser.AreaCheck(_T("MATCHLANCLINET")) == false)
			return false;

		// MasterServerIP
		if (Parser.GetValue_String(_T("MasterServerIP"), pConfig->MasterServerIP) == false)
			return false;

		// MasterServerPort
		if (Parser.GetValue_Int(_T("MasterServerPort"), &pConfig->MasterServerPort) == false)
			return false;

		// ���� ��Ŀ ��
		if (Parser.GetValue_Int(_T("ClientCreateWorker"), &pConfig->ClientCreateWorker) == false)
			return false;

		// Ȱ��ȭ ��Ŀ ��
		if (Parser.GetValue_Int(_T("ClientActiveWorker"), &pConfig->ClientActiveWorker) == false)
			return false;

		// Nodelay
		if (Parser.GetValue_Int(_T("ClientNodelay"), &pConfig->ClientNodelay) == false)
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

		// Ŭ�� ����� ���� char������ ������� ������ ��ȯ�ؼ� ������ �־���Ѵ�.
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
					gMatchServerDump->Crash();
			}
			
			else
			{
				// �ߺ�Ű�� �ƴ� ������ �߻������� ũ����	
				gMatchServerDump->Crash();
			}
		}	
	}

	// ClientKey ����� �Լ�
	//
	// Parameter : ����
	// return : ClientKey(UINT64)
	UINT64 Matchmaking_Net_Server::CreateClientKey()
	{
		// ���� 4����Ʈ�� ServerNo. ���� 4����Ʈ�� m_ClientKeyAdd�� ���� ����.
		UINT64 TempKey = InterlockedIncrement(&m_ClientKeyAdd);
		return ((TempKey << 16) | m_iServerNo);
	}

	// ������ ��� �������� shutdown �ϴ� �Լ�
	//
	// Parameter : ����
	// return : ����
	void Matchmaking_Net_Server::AllShutdown()
	{
		AcquireSRWLockShared(&m_srwlPlayer);	// ----- m_umapPlayer�� Shared ��

		// ��� �������� Shutdown ����.

		auto itor_Now = m_umapPlayer.begin();
		auto itor_End = m_umapPlayer.end();

		while (1)
		{
			if (itor_Now == itor_End)
				break;
			
			Disconnect(itor_Now->second->m_ullSessionID);
			++itor_Now;
		}

		ReleaseSRWLockShared(&m_srwlPlayer);	// ----- m_umapPlayer�� Shared ���
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

	// Player ���� �ڷᱸ�� "2��"��, ���� �߰�
	// ���� umap���� ������
	// 
	// Parameter : SessionID, ClientKey, stPlayer*
	// return : �߰� ���� ��, true
	//		  : SessionID�� �ߺ��� ��(�̹� �������� ����) false
	bool Matchmaking_Net_Server::InsertPlayerFunc(ULONGLONG SessionID, UINT64 ClientKey, stPlayer* insertPlayer)
	{
		// 1. SessionID�� umap�� �߰�		
		AcquireSRWLockExclusive(&m_srwlPlayer);		// ------- Exclusive ��

		auto ret_A = m_umapPlayer.insert(make_pair(SessionID, insertPlayer));		

		// 2. �ߺ��� Ű�� �� false ����.
		if (ret_A.second == false)
		{
			ReleaseSRWLockExclusive(&m_srwlPlayer);		// ------- Exclusive ���
			return false;
		}
		
		// 3. ClientKey�� uamp�� �߰�
		auto ret_B = m_umapPlayer_ClientKey.insert(make_pair(ClientKey, insertPlayer));

		ReleaseSRWLockExclusive(&m_srwlPlayer);		// ------- Exclusive ���

		// 4. �ߺ��� Ű�� �� false ����.
		if (ret_B.second == false)
			return false;

		return true;
	}


	// Player ���� �ڷᱸ������, ���� �˻�
	// ���� map���� ������
	// !!SessionID!! �� �̿��� �˻�
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

	// Player ���� �ڷᱸ������, ���� �˻�
	// !!ClientKey!! �� �̿��� �˻�
	// ���� umap���� ������
	// 
	// Parameter : ClientKey
	// return : �˻� ���� ��, stPalyer*
	//		  : �˻� ���� �� nullptr
	Matchmaking_Net_Server::stPlayer* Matchmaking_Net_Server::FindPlayerFunc_ClientKey(UINT64 ClientKey)
	{
		// 1. umap���� �˻�
		AcquireSRWLockShared(&m_srwlPlayer);		// ------- Shared ��

		auto FindPlayer = m_umapPlayer_ClientKey.find(ClientKey);

		ReleaseSRWLockShared(&m_srwlPlayer);		// ------- Shared ���

		// 2. �˻� ���� �� nullptr ����
		if (FindPlayer == m_umapPlayer_ClientKey.end())
			return nullptr;

		// 3. �˻� ���� ��, ã�� stPlayer* ����
		return FindPlayer->second;
	}


	// Player ���� �ڷᱸ�� "2��"����, ���� ���� (�˻� �� ����)
	// ���� umap���� ������
	// 
	// Parameter : SessionID
	// return : ���� ��, ���ŵ� ���� stPalyer*
	//		  : �˻� ���� ��(���������� ���� ����) nullptr
	Matchmaking_Net_Server::stPlayer* Matchmaking_Net_Server::ErasePlayerFunc(ULONGLONG SessionID)
	{
		// 1. SessionID�� umap���� ���� �˻�
		AcquireSRWLockExclusive(&m_srwlPlayer);		// ------- Exclusive ��

		auto FindPlayer_A = m_umapPlayer.find(SessionID);
		if (FindPlayer_A == m_umapPlayer.end())
		{
			ReleaseSRWLockExclusive(&m_srwlPlayer);		// ------- Exclusive ���
			return nullptr;
		}

		// 2. ClientKey �� umap���� ���� �˻�
		auto FindPlayer_B = m_umapPlayer_ClientKey.find(FindPlayer_A->second->m_ui64ClientKey);
		if (FindPlayer_B == m_umapPlayer_ClientKey.end())
		{
			ReleaseSRWLockExclusive(&m_srwlPlayer);		// ------- Exclusive ���
			return nullptr;
		}
		
		// 3. �� �ٿ��� �����Ѵٸ�, ������ �� �޾Ƶα�
		stPlayer* ret = FindPlayer_B->second;

		// 3. SessionID�� uamp, ClinetKey�� umap���� ����
		m_umapPlayer.erase(FindPlayer_A);
		m_umapPlayer_ClientKey.erase(FindPlayer_B);

		ReleaseSRWLockExclusive(&m_srwlPlayer);		// ------- Exclusive ���

		return ret;
	}






	// -------------------------------------
	// ��Ŷ ó���� �Լ�
	// -------------------------------------

	// Ŭ���� �α��� ��û ����
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

		// 2. �÷��̾� �˻�.		
		stPlayer* NowPlayer = FindPlayerFunc(SessionID);
		if (NowPlayer == nullptr)
			gMatchServerDump->Crash();


		// 3. ���� �̹� Login �� �����ų�, ��Ʋ �濡 ����� ������� Crash
		if (NowPlayer->m_bLoginCheck == true ||
			NowPlayer->m_bBattleRoomEnterCheck == true)
			gMatchServerDump->Crash();


		// 4. AccountNo�� DB���� ã�ƿ���
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



		// 5. DB ��� üũ
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

			SendPacket(SessionID, SendData);
			return;
		}		


		// 6. ����� 1�̶��, ��ūŰ�� ���� üũ
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

			SendPacket(SessionID, SendData);
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

			SendPacket(SessionID, SendData);
			return;
		}




		// 7. ������� ������ �������� �÷��̾�. ���� ����
		// 1) AccountNo ����
		NowPlayer->m_i64AccountNo = AccountNo;	

		// 2) �α��� ���·� ����
		NowPlayer->m_bLoginCheck = true;

		// �α��� ���� �� ����
		InterlockedIncrement(&g_lLoginUser);


		// 8. ���� ��Ŷ ����
		WORD Type = en_PACKET_CS_MATCH_RES_LOGIN;
		BYTE Status = 1; // ����

		CProtocolBuff_Net* SendData = CProtocolBuff_Net::Alloc();

		SendData->PutData((char*)&Type, 2);
		SendData->PutData((char*)&Status, 1);

		SendPacket(SessionID, SendData);

		return;
	}
	
	// �� ���� ���� (Ŭ�� --> ��Ī Net����)
	// �����Ϳ��� �� ���� ���� ��Ŷ ����
	//
	// Parameter : SessionID, Payload
	// return : ����. ������ �����, ���ο��� throw ����
	void Matchmaking_Net_Server::Packet_Battle_EnterOK(ULONGLONG SessionID, CProtocolBuff_Net* Payload)
	{
		// 1. ������
		WORD BattleServerNo;
		int RoomNo;

		Payload->GetData((char*)&BattleServerNo, 2);
		Payload->GetData((char*)&RoomNo, 4);


		// 2. ���� �˻�
		stPlayer* NowPlayer = FindPlayerFunc(SessionID);
		if (NowPlayer == nullptr)
			gMatchServerDump->Crash();


		// 3. �ش� ������ �� ���� ���� Flag ����
		// ����, �̹� �� ���� ���� ��Ŷ�� ���� ������� Crash()
		if (NowPlayer->m_bBattleRoomEnterCheck == true)
			gMatchServerDump->Crash();
		
		NowPlayer->m_bBattleRoomEnterCheck = true;


		// 4. ������ �������� ���� ��Ŷ ���� (Lan ����ȭ ���� ���)
		CProtocolBuff_Lan* SendBuff = CProtocolBuff_Lan::Alloc();

		WORD Type = en_PACKET_MAT_MAS_REQ_ROOM_ENTER_SUCCESS;

		SendBuff->PutData((char*)&Type, 2);
		SendBuff->PutData((char*)&BattleServerNo, 2);
		SendBuff->PutData((char*)&RoomNo, 4);
		SendBuff->PutData((char*)&NowPlayer->m_ui64ClientKey, 8);

		// 5. �����Ϳ��� Send
		m_pLanClient->SendPacket(m_pLanClient->m_ullClientID, SendBuff);
	}


	// �� ���� ����
	// �����Ϳ��� �� ���� ���� ��Ŷ ����
	//
	// Parameter : ClinetKey
	// return : ����
	void Matchmaking_Net_Server::Packet_Battle_EnterFail(UINT64 ClientKey)
	{
		// 1. �����Ϳ��� ���� ��Ŷ ����
		WORD Type = en_PACKET_MAT_MAS_REQ_ROOM_ENTER_FAIL;

		CProtocolBuff_Lan* SendBuff = CProtocolBuff_Lan::Alloc();

		SendBuff->PutData((char*)&Type, 2);
		SendBuff->PutData((char*)&ClientKey, 8);

		// 2. �����ͷ� ��Ŷ ������
		m_pLanClient->SendPacket(m_pLanClient->m_ullClientID, SendBuff);
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

		

		//------------------- �����Ϳ� ����Ǵ� �� Ŭ�� ���� �� this ����
		m_pLanClient->SetParent(this);
		if (m_pLanClient->ClientStart() == false)
			return false;

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

		TokenError : 		- DB���� ã�ƿ� ��ū�� �α��� ��û�� ������ ����� ��ū�� �ٸ�
		AccountError : 		- Selecet.account.php�� ���� ���ȴµ�, -10 ������ ��(ȸ���������� ���� ����)
		TempError :			- Selecet.account.php�� ���� ���ȴµ�, -10 �ܿ� ��Ÿ ������ ��
		VerError :			- �α��� ��û�� ������ ����� VerCode�� ������ ����ִ� VerCode�� �ٸ�
		NotRoomEnter :		- ��Ʋ �� ���� ���� ��Ŷ�� �Ⱥ����� ���� ���� ��

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
			"VerError : %d\n"
			"NotRoomEnter : %d\n\n"

			"========================================================\n\n",

			// ------------ ��ġ����ŷ Net ������
			GetClientCount(), g_lAllocNodeCount,
			g_lstPlayer_AllocCount, m_umapPlayer.size(),
			g_ullAcceptTotal, AccpetTPS, SendTPS,
			CProtocolBuff_Net::GetChunkCount(), CProtocolBuff_Net::GetOutChunkCount(),
			m_PlayerPool->GetAllocChunkCount(), m_PlayerPool->GetOutChunkCount(),
			g_lTokenError, g_lAccountError, g_lTempError, g_lVerError, g_lNot_BattleRoom_Enter);

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

		// 2. stPlayer�� SessionID, ClientKey ����
		NowPlayer->m_ullSessionID = SessionID;	
		UINT64 TempCKey = CreateClientKey();
		NowPlayer->m_ui64ClientKey = TempCKey;

		// 3. Player ���� umap�� �߰�.
		if (InsertPlayerFunc(SessionID, TempCKey, NowPlayer) == false)
			gMatchServerDump->Crash();

		// 4. umap�� ���� �� üũ.
		// umap�� �������� m_ChangeConnectUser +m_iMatchDBConnectUserChange ���� ���ٸ�, ���� �� ���� 100�� �̻� ���°�.
		// ��ġ����ŷ DB�� ��Ʈ��Ʈ �Ѵ�.	
		// ī��Ʈ�� ��Ȯ�ϰ� �ϱ� ���� �� ���	
		size_t NowumapCount = (int)m_umapPlayer.size();
		
		AcquireSRWLockExclusive(&m_srwlChangeConnectUser);	// Exclusive �� -----------

		if (m_ChangeConnectUser + m_iMatchDBConnectUserChange <= NowumapCount)
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
			gMatchServerDump->Crash();

		// 2. umap ���� �� üũ.
		// umap�� �������� m_ChangeConnectUser - m_iMatchDBConnectUserChange ���� ���ٸ�, ���� �� ���� 100�� �̻� ������.
		// ��ġ����ŷ DB�� ��Ʈ��Ʈ �Ѵ�.	
		// ī��Ʈ�� ��Ȯ�ϰ� �ϱ� ���� �� ���	
		size_t NowumapCount = m_umapPlayer.size();

		AcquireSRWLockExclusive(&m_srwlChangeConnectUser);	// Exclusive �� -----------

		if (m_ChangeConnectUser - m_iMatchDBConnectUserChange >= NowumapCount)
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

		// 5. ��Ʋ �� ���� ��Ŷ�� �Ⱥ��� �������, ���� ��Ŷ�� �����Ϳ��� ����
		if (ErasePlayer->m_bBattleRoomEnterCheck == false)
		{
			InterlockedIncrement(&g_lNot_BattleRoom_Enter);
			Packet_Battle_EnterFail(ErasePlayer->m_ui64ClientKey);
		}

		// 6. ��Ʋ�� ���� �÷��� ����
		ErasePlayer->m_bBattleRoomEnterCheck = false;


		// 7. �÷��̾� ����ü ��ȯ
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
				// LanClinet�� �Լ� ȣ��
			case en_PACKET_CS_MATCH_REQ_GAME_ROOM:
				m_pLanClient->Packet_Battle_Info(SessionID);
				break;

				// ��Ʋ ������ �� ���� ���� �˸�
			case en_PACKET_CS_MATCH_REQ_GAME_ROOM_ENTER:
				Packet_Battle_EnterOK(SessionID, Payload);
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
		m_umapPlayer_ClientKey.reserve(m_stConfig.MaxJoinUser);

		// �����Ϳ� ����ϴ� LanŬ�� �����Ҵ�
		m_pLanClient = new Matchmaking_Lan_Client();

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

// ---------------------------------------------
// 
// ��ġ����ŷ LanClient(Master������ LanServer�� ���)
//
// ---------------------------------------------
namespace Library_Jingyu
{
	// -------------------------------------
	// Net ������ ȣ���ϴ� �Լ�
	// -------------------------------------

	// �� ���� ��û (Ŭ�� --> ��Ī Net����)
	// LanClient�� ����, �����Ϳ��� ��Ŷ ����
	//
	// Parameter : SessionID
	// return : ����. ������ �����, ���ο��� throw ����
	void Matchmaking_Lan_Client::Packet_Battle_Info(ULONGLONG SessionID)
	{
		// 1. ���� �˻�
		Matchmaking_Net_Server::stPlayer* NowPlayer = m_pParent->FindPlayerFunc(SessionID);
		if (NowPlayer == nullptr)
			gMatchServerDump->Crash();

		// 2. �����Ϳ��� ���� ��Ŷ ����
		// Lan ����ȭ ���� ���
		WORD Type = en_PACKET_MAT_MAS_REQ_GAME_ROOM;

		CProtocolBuff_Lan* SendBuff = CProtocolBuff_Lan::Alloc();

		SendBuff->PutData((char*)&Type, 2);
		SendBuff->PutData((char*)&NowPlayer->m_ui64ClientKey, 8);
		SendBuff->PutData((char*)&NowPlayer->m_i64AccountNo, 8);

		// 3. ������ Send
		SendPacket(m_ullClientID, SendBuff);
	}





	// -------------------------------------
	// �����Ϳ��� ���� ��Ŷ ó���� �Լ�
	// -------------------------------------

	// �� ���� ��û�� ���� ����
	// ���ο���, Net������ SendPacket()���� ȣ���Ѵ�.
	// 
	// Parameter : CProtocolBuff_Lan*
	// return : ����. ������ �����, ���ο��� throw ����
	void Matchmaking_Lan_Client::Response_Battle_Info(CProtocolBuff_Lan* payload)
	{
		// 1. ClientKey�� Status ������
		UINT64 ClinetKey;
		BYTE Status;

		payload->GetData((char*)&ClinetKey, 8);
		payload->GetData((char*)&Status, 1);

		// 2. ���� �˻�
		Matchmaking_Net_Server::stPlayer* NowPlayer = m_pParent->FindPlayerFunc_ClientKey(ClinetKey);
		if (NowPlayer == nullptr)
			gMatchServerDump->Crash();

		// 3. Status Ȯ��
		// 1�� �ƴϸ� ��� �߸��� ��.
		if (Status != 1)
		{
			// Status�� 0�̶�� �� ��� ������ ��.
			// Ŭ�󿡰� �� ���� ���� ��� ����
			if (Status == 0)
			{
				// Net ����ȭ ���� ���.
				CProtocolBuff_Net* SendBuff = CProtocolBuff_Net::Alloc();

				WORD Type = en_PACKET_CS_MATCH_RES_GAME_ROOM;
				BYTE SendStatus = 0;

				SendBuff->PutData((char*)&Type, 2);
				SendBuff->PutData((char*)&SendStatus, 1);

				// ���� ������ ����� ���� ����..
				m_pParent->SendPacket(NowPlayer->m_ullSessionID, SendBuff);

				return;
			}

			// ������� ���� 1�� �ƴϰ� 0�� �ƴѰ�. �Ծ࿡ ���� Status. Crash
			else
				gMatchServerDump->Crash();
		}

		// 4. ���°� 1�̶��, �� ������ �� ��. ������ ������
		WORD	BattleServerNo;		
		WCHAR	IP[16];
		WORD	Port;		
		int		RoomNo;
		char	ConnectToken[32];
		char	EnterToken[32];		

		WCHAR	ChatServerIP[16];
		WORD	ChatServerPort;

		payload->GetData((char*)&BattleServerNo, 2);
		payload->GetData((char*)&IP, 32);
		payload->GetData((char*)&Port, 2);
		payload->GetData((char*)&RoomNo, 4);
		payload->GetData(ConnectToken, 32);
		payload->GetData(EnterToken, 32);

		payload->GetData((char*)&ChatServerIP, 32);
		payload->GetData((char*)&ChatServerPort, 2);


		// 5. Ŭ�󿡰� ���� ���� ����
		CProtocolBuff_Net* SendBuff = CProtocolBuff_Net::Alloc();

		WORD Type = en_PACKET_CS_MATCH_RES_GAME_ROOM;
		BYTE SendStatus = 0;

		SendBuff->PutData((char*)&Type, 2);
		SendBuff->PutData((char*)&SendStatus, 1);

		SendBuff->PutData((char*)&BattleServerNo, 2);
		SendBuff->PutData((char*)&IP, 32);
		SendBuff->PutData((char*)&Port, 2);
		SendBuff->PutData((char*)&RoomNo, 4);
		SendBuff->PutData(ConnectToken, 32);
		SendBuff->PutData(EnterToken, 32);

		SendBuff->PutData((char*)&ChatServerIP, 32);
		SendBuff->PutData((char*)&ChatServerPort, 2);


		// 6. Ŭ�󿡰� ������ (Net���� ������)
		m_pParent->SendPacket(NowPlayer->m_ullSessionID, SendBuff);
	}

	// �α��� ��û�� ���� ����
	// 
	// Parameter : CProtocolBuff_Lan*
	// return : ����. ������ �����, ���ο��� throw ����
	void Matchmaking_Lan_Client::Response_Login(CProtocolBuff_Lan* payload)
	{
		// �̹� �α��� �������� üũ
		if(m_bLoginCheck == true)
			gMatchServerDump->Crash();

		// 1. ������
		int iServerNo;
		payload->GetData((char*)&iServerNo, 4);

		// 2. ���� Ȯ��
		if(iServerNo != m_pParent->m_iServerNo)
			gMatchServerDump->Crash();

		// ������ ���������� �´ٸ� �� �� ����.
	}




	// -------------------------------------
	// �ܺο��� ��� ������ �Լ�
	// -------------------------------------

	// ��ġ����ŷ LanClient ���� �Լ�
	//
	// Parameter : ����
	// return : ���� �� false ����
	bool Matchmaking_Lan_Client::ClientStart()
	{
		// ClientID �� �ʱ�ȭ
		m_ullClientID = 0xffffffffffffffff;

		// �α��� üũ �ʱ�ȭ
		m_bLoginCheck = false;

		// LanClient�� Start() ȣ��
		// �������� LanServer�� ����
		//if (Start(m_pParent->m_stConfig.MasterServerIP, m_pParent->m_stConfig.MasterServerPort, m_pParent->m_stConfig.ClientCreateWorker, m_pParent->m_stConfig.ClientActiveWorker, m_pParent->m_stConfig.ClientNodelay) == false)
			//return false;

		return true;
	}

	// ��ġ����ŷ LanClient ���� �Լ�
	//
	// Parameter : ����
	// return : ����
	void Matchmaking_Lan_Client::ClientStop()
	{

	}




	// -------------------------------------
	// ��� ���迡���� ��� ������ ��� �Լ�
	// -------------------------------------

	// �� �θ� ä���ִ� �Լ�
	void Matchmaking_Lan_Client::SetParent(Matchmaking_Net_Server* NetServer)
	{
		m_pParent = NetServer;
	}






	// -----------------------
	// Lan Ŭ���� ���� �Լ�
	// -----------------------

	void Matchmaking_Lan_Client::OnConnect(ULONGLONG ClinetID)
	{
		// �� m_ullClientID�� 0xffffffffffffffff���� üũ
		if(m_ullClientID != 0xffffffffffffffff)
			gMatchServerDump->Crash();

		// 1. ClientID �޾Ƶд�.
		m_ullClientID = ClinetID;

		// 2. ������ ������ ���� �α��� ���� ����
		CProtocolBuff_Lan* SendBuff = CProtocolBuff_Lan::Alloc();

		WORD Type = en_PACKET_MAT_MAS_REQ_SERVER_ON;
		SendBuff->PutData((char*)&Type, 2);
		SendBuff->PutData((char*)&m_pParent->m_iServerNo, 4);
		SendBuff->PutData(m_pParent->MasterToken, 32);

		// 3. ������ ������ �α��� ��Ŷ ������
		SendPacket(ClinetID, SendBuff);
	}

	void Matchmaking_Lan_Client::OnDisconnect(ULONGLONG ClinetID)
	{
		// Net ������ ����� ��� ���� ���� ����
		m_pParent->AllShutdown();

		// m_ullClientID �ʱ�ȭ
		m_ullClientID = 0xffffffffffffffff;

		// �α��� Flag �ʱ�ȭ
		m_bLoginCheck = false;
	}

	void Matchmaking_Lan_Client::OnRecv(ULONGLONG ClinetID, CProtocolBuff_Lan* Payload)
	{
		// �� ClientID�� ���ڷ� ���� ClientID�� �ٸ��� Crash
		if(m_ullClientID != ClinetID)
			gMatchServerDump->Crash();

		// Ÿ�� ������
		WORD Type;
		Payload->GetData((char*)&Type, 2);

		// Ÿ�Կ� ���� �б�ó��
		try
		{
			switch (Type)
			{
				// �� ��û�� ���� ����
			case en_PACKET_MAT_MAS_RES_GAME_ROOM:
				Response_Battle_Info(Payload);
				break;

				// �α��� ��û�� ���� ����
			case en_PACKET_MAT_MAS_RES_SERVER_ON:
				Response_Login(Payload);
				break;

				// �̻��� Ÿ���� ��Ŷ�� ���� ���´�.
			default:
				TCHAR ErrStr[1024];
				StringCchPrintf(ErrStr, 1024, _T("LanClient -> OnRecv(). TypeError. Type : %d"), Type);

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
		}
		
	}

	void Matchmaking_Lan_Client::OnSend(ULONGLONG ClinetID, DWORD SendSize)
	{}

	void Matchmaking_Lan_Client::OnWorkerThreadBegin()
	{}

	void Matchmaking_Lan_Client::OnWorkerThreadEnd()
	{}

	void Matchmaking_Lan_Client::OnError(int error, const TCHAR* errorStr)
	{}





	// -------------------------------------
	// �����ڿ� �Ҹ���
	// -------------------------------------

	// ������
	Matchmaking_Lan_Client::Matchmaking_Lan_Client()
		:CLanClient()
	{
		// Ư���� �� �� ����

	}

	// �Ҹ���
	Matchmaking_Lan_Client::~Matchmaking_Lan_Client()
	{
		// Ư���� �� �� ����
	}	
}