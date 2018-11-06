#include "pch.h"
#include "MasterServer.h"

#include <strsafe.h>


// ----------------- ��ġ����ŷ�� ��� ����

// �α��� ��, ��ū ������ �� Ƚ��
LONG g_lMatch_TokenError;

// �α��� ��, �̹� �α��� �� ��Ī������ �� �α��� ��û��.
LONG g_lMatch_DuplicateLogin;

// �α��� ���� ���� ������ ��Ŷ�� ����.
LONG g_lMatch_NotLoginPacket;




// -----------------------
//
// ������ Lan����
// �ܺο����� �� Ŭ������ �����ϸ�, �� Ŭ������ �� Lan������ ����.
// 
// -----------------------
namespace Library_Jingyu
{
	// ������
	CCrashDump* g_MasterDump = CCrashDump::GetInstance();

	// �α� �����
	CSystemLog* g_MasterLog = CSystemLog::GetInstance();
	


	// -------------------------------------
	// �ܺο��� ��� ������ �Լ�
	// -------------------------------------

	// ������ ���� ����
	// ���� Lan������ �����Ѵ�.
	// 
	// Parameter : ����
	// return : ���� �� false ����
	bool CMasterServer::ServerStart()
	{
		// �� �������� MasterServer ����
		m_pMatchServer->SetParent(this);
		m_pBattleServer->SetParent(this);

		// ��Ī�� ������ ����
		if (m_pMatchServer->ServerStart() == false)
		{
			return false;
		}

		// ��Ʋ�� ������ ����
		if (m_pBattleServer->ServerStart() == false)
		{
			return false;
		}

		// ���� ���� �α� ���		
		g_MasterLog->LogSave(L"MasterServer", CSystemLog::en_LogLevel::LEVEL_SYSTEM, L"AllServerOpen...");

		return true;
	}

	// ������ ���� ����
	// ���� Lan������ ����
	// 
	// Parameter : ����
	// return : ����
	void CMasterServer::ServerStop()
	{
		// �� ������ ����
		// ��Ī ������ �۵������� üũ �� ����
		if (m_pMatchServer->GetServerState() == true)
			m_pMatchServer->ServerStop();

		// ��Ʋ ������ �۵������� üũ �� ����
		if (m_pBattleServer->GetServerState() == true)
			m_pBattleServer->ServerStop();

		// ���� ���� �α� ���		
		g_MasterLog->LogSave(L"MasterServer", CSystemLog::en_LogLevel::LEVEL_SYSTEM, L"AllServerClose...");
	}
	   

	// -------------------------------------
	// Player ������ �ڷᱸ�� �Լ�
	// -------------------------------------

	// Player ���� �ڷᱸ�� "2��"�� Isnert
	//
	// Parameter : ClinetKey, AccountNo
	// return : �ߺ��� �� false.
	bool CMasterServer::InsertPlayerFunc(UINT64 ClinetKey, UINT64 AccountNo)
	{
		// 1. ClinetKey�� uset�� �߰�		
		AcquireSRWLockExclusive(&m_srwl_ClientKey_Umap);		// ------- ClinetKey Exclusive ��

		auto ret_A = m_ClientKey_Uset.insert(ClinetKey);

		// 2. �ߺ��� Ű�� �� false ����.
		if (ret_A.second == false)
		{
			ReleaseSRWLockExclusive(&m_srwl_ClientKey_Umap);		// ------- ClinetKey Exclusive ���
			return false;
		}

		ReleaseSRWLockExclusive(&m_srwl_ClientKey_Umap);		// ------- ClinetKey Exclusive ���

		// 3. AccountNo�� uset�� �߰�
		AcquireSRWLockExclusive(&m_srwl_m_AccountNo_Umap);		// ------- AccountNo Exclusive ��

		auto ret_B = m_AccountNo_Uset.insert(AccountNo);

		ReleaseSRWLockExclusive(&m_srwl_m_AccountNo_Umap);		// ------- AccountNo Exclusive ���

		// 4. �ߺ��� Ű�� �� false ����.
		if (ret_B.second == false)
			return false;

		return true;
	}

	// ClinetKey ������ �ڷᱸ������ Erase
	// umap���� ������
	//
	// Parameter : ClinetKey
	// return : ���� ������ �� false
	//		  : �ִ� ������ �� true
	bool CMasterServer::ErasePlayerFunc(UINT64 ClinetKey)
	{
		// 1. umap���� ���� �˻�
		AcquireSRWLockExclusive(&m_srwl_ClientKey_Umap);		// ------- Exclusive ��

		auto FindPlayer = m_ClientKey_Uset.find(ClinetKey);
		if (FindPlayer == m_ClientKey_Uset.end())
		{
			ReleaseSRWLockExclusive(&m_srwl_ClientKey_Umap);		// ------- Exclusive ���
			return false;
		}	

		// 2. �����Ѵٸ� umap���� ����
		m_ClientKey_Uset.erase(FindPlayer);

		ReleaseSRWLockExclusive(&m_srwl_ClientKey_Umap);		// ------- Exclusive ���

		return true;
	}



	// -------------------------------------
	// ���ο����� ����ϴ� �Լ�
	// -------------------------------------

	// ���Ͽ��� Config ���� �о����
	// 
	// Parameter : config ����ü
	// return : ���������� ���� �� true
	//		  : �� �ܿ��� false
	bool CMasterServer::SetFile(stConfigFile* pConfig)
	{
		Parser Parser;

		// ���� �ε�
		try
		{
			Parser.LoadFile(_T("MasterServer_Config.ini"));
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
		// ��ġ����ŷ �������� Config
		////////////////////////////////////////////////////////

		// ���� ���� -------------------------
		if (Parser.AreaCheck(_T("MATCH")) == false)
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

		// Nodelay
		if (Parser.GetValue_Int(_T("Nodelay"), &pConfig->Nodelay) == false)
			return false;

		// �ִ� ���� ���� ���� ��
		if (Parser.GetValue_Int(_T("MaxJoinUser"), &pConfig->MaxJoinUser) == false)
			return false;

		// �α� ����
		if (Parser.GetValue_Int(_T("LogLevel"), &pConfig->LogLevel) == false)
			return false;

		// ��ū
		TCHAR tcEnterToken[33];
		if (Parser.GetValue_String(_T("EnterToken"), tcEnterToken) == false)
			return false;

		// Ŭ�� ���� ���� char������ ������ ������ ��ȯ�ؼ� ������ �־���Ѵ�.
		int len = WideCharToMultiByte(CP_UTF8, 0, tcEnterToken, lstrlenW(tcEnterToken), NULL, 0, NULL, NULL);
		WideCharToMultiByte(CP_UTF8, 0, tcEnterToken, lstrlenW(tcEnterToken), pConfig->EnterToken, len, NULL, NULL);






		////////////////////////////////////////////////////////
		// ��Ʋ �������� Config
		////////////////////////////////////////////////////////

		// ���� ���� -------------------------
		if (Parser.AreaCheck(_T("BATTLE")) == false)
			return false;

		// BindIP
		if (Parser.GetValue_String(_T("BattleBindIP"), pConfig->BattleBindIP) == false)
			return false;

		// Port
		if (Parser.GetValue_Int(_T("BattlePort"), &pConfig->BattlePort) == false)
			return false;

		// ���� ��Ŀ ��
		if (Parser.GetValue_Int(_T("BattleCreateWorker"), &pConfig->BattleCreateWorker) == false)
			return false;

		// Ȱ��ȭ ��Ŀ ��
		if (Parser.GetValue_Int(_T("BattleActiveWorker"), &pConfig->BattleActiveWorker) == false)
			return false;

		// ���� ����Ʈ
		if (Parser.GetValue_Int(_T("BattleCreateAccept"), &pConfig->BattleCreateAccept) == false)
			return false;

		// Nodelay
		if (Parser.GetValue_Int(_T("BattleNodelay"), &pConfig->BattleNodelay) == false)
			return false;

		// �ִ� ���� ���� ���� ��
		if (Parser.GetValue_Int(_T("BattleMaxJoinUser"), &pConfig->BattleMaxJoinUser) == false)
			return false;

		// �α� ����
		if (Parser.GetValue_Int(_T("BattleLogLevel"), &pConfig->BattleLogLevel) == false)
			return false;

		// ��ū
		TCHAR tcBattleEnterToken[33];
		if (Parser.GetValue_String(_T("EnterToken"), tcBattleEnterToken) == false)
			return false;

		// Ŭ�� ���� ���� char������ ������ ������ ��ȯ�ؼ� ������ �־���Ѵ�.
		len = WideCharToMultiByte(CP_UTF8, 0, tcBattleEnterToken, lstrlenW(tcBattleEnterToken), NULL, 0, NULL, NULL);
		WideCharToMultiByte(CP_UTF8, 0, tcBattleEnterToken, lstrlenW(tcBattleEnterToken), pConfig->BattleEnterToken, len, NULL, NULL);
			   

		return true;
	}




	// -------------------------------------
	// �����ڿ� �Ҹ���
	// -------------------------------------

	// ������
	CMasterServer::CMasterServer()
	{
		// �� �� ���� �����Ҵ�
		m_pMatchServer = new CMatchServer_Lan;
		m_pBattleServer = new CBattleServer_Lan;

		//  Config���� ����		
		if (SetFile(&m_stConfig) == false)
			g_MasterDump->Crash();

		// �α� ������ ���� ����
		g_MasterLog->SetDirectory(L"MasterServer");
		g_MasterLog->SetLogLeve((CSystemLog::en_LogLevel)m_stConfig.LogLevel);


		// �� �ʱ�ȭ
		InitializeSRWLock(&m_srwl_BattleServer_Umap);
		InitializeSRWLock(&m_srwl_ClientKey_Umap);
		InitializeSRWLock(&m_srwl_m_AccountNo_Umap);
		InitializeSRWLock(&m_srwl_Room_Umap);
		InitializeSRWLock(&m_srwl_Room_pq);

		// TLS �����Ҵ�
		m_TLSPool_BattleServer = new CMemoryPoolTLS<CMasterServer::CBattleServer>(0, false);
		m_TLSPool_Room = new CMemoryPoolTLS<CRoom>(0, false);
		

		// �ڷᱸ�� ���� �̸� ��Ƶα�
		m_BattleServer_Umap.reserve(1000);
		m_ClientKey_Uset.reserve(5000);
		m_AccountNo_Uset.reserve(5000);
		m_Room_Umap.reserve(5000);
	}
	
	// �Ҹ���
	CMasterServer::~CMasterServer()
	{
		// ��Ī ������ �۵������� üũ �� ����
		if (m_pMatchServer->GetServerState() == true)
			m_pMatchServer->ServerStop();

		// ��Ʋ ������ �۵������� üũ �� ����
		if (m_pBattleServer->GetServerState() == true)
			m_pBattleServer->ServerStop();
		
		delete m_pMatchServer;
		delete m_pBattleServer;

		// TLS ���� ����
		delete m_TLSPool_Room;
		delete m_TLSPool_BattleServer;
	}
}




// -----------------------
//
// ��ġ����ŷ�� ����Ǵ� Lan ����
// 
// -----------------------
namespace Library_Jingyu
{	
	// -----------------
	// umap�� �ڷᱸ�� ���� �Լ�
	// -----------------

	// ������ ���� �ڷᱸ���� Insert
	// umap���� ������
	//
	// Parameter : SessionID, CMatchingServer*
	// return : ���� �� false ����
	bool CMatchServer_Lan::InsertPlayerFunc(ULONGLONG SessionID, CMasterServer::CMatchingServer* insertServer)
	{
		// 1. umap�� �߰�		
		AcquireSRWLockExclusive(&m_srwl_MatchServer_Umap);		// ------- Exclusive ��

		auto ret = m_MatchServer_Umap.insert(make_pair(SessionID, insertServer));

		// 2. �ߺ��� Ű�� �� false ����.
		if (ret.second == false)
		{
			ReleaseSRWLockExclusive(&m_srwl_MatchServer_Umap);		// ------- Exclusive ���
			return false;
		}	
		return true;
	}

	// ������ ���� �ڷᱸ������, ���� �˻�
	// ���� umap���� ������
	// 
	// Parameter : SessionID
	// return : �˻� ���� ��, CMatchingServer*
	//		  : �˻� ���� �� nullptr
	CMasterServer::CMatchingServer* CMatchServer_Lan::FindPlayerFunc(ULONGLONG SessionID)
	{
		// 1. umap���� �˻�
		AcquireSRWLockShared(&m_srwl_MatchServer_Umap);		// ------- Shared ��

		auto FindPlayer = m_MatchServer_Umap.find(SessionID);

		ReleaseSRWLockShared(&m_srwl_MatchServer_Umap);		// ------- Shared ���

		// 2. �˻� ���� �� nullptr ����
		if (FindPlayer == m_MatchServer_Umap.end())
			return nullptr;

		// 3. �˻� ���� ��, ã�� stPlayer* ����
		return FindPlayer->second;
	}



	// ������ ���� �ڷᱸ������ Erase
	// umap���� ������
	//
	// Parameter : SessionID
	// return : ���� ��, ���ŵ� CMatchingServer*
	//		  : �˻� ���� ��(���������� ���� ����) nullptr
	CMasterServer::CMatchingServer* CMatchServer_Lan::ErasePlayerFunc(ULONGLONG SessionID)
	{
		// 1. umap���� ���� �˻�
		AcquireSRWLockExclusive(&m_srwl_MatchServer_Umap);		// ------- Exclusive ��

		auto FindPlayer = m_MatchServer_Umap.find(SessionID);
		if (FindPlayer == m_MatchServer_Umap.end())
		{
			ReleaseSRWLockExclusive(&m_srwl_MatchServer_Umap);		// ------- Exclusive ���
			return nullptr;
		}	

		// 2. �� �ٿ��� �����Ѵٸ�, ������ �� �޾Ƶα�
		CMasterServer::CMatchingServer* ret = FindPlayer->second;

		// 3. umap���� ����
		m_MatchServer_Umap.erase(FindPlayer);

		ReleaseSRWLockExclusive(&m_srwl_MatchServer_Umap);		// ------- Exclusive ���

		return ret;
	}




	// -----------------
	// uset�� �ڷᱸ�� ���� �Լ�
	// -----------------

	// �α��� �� ���� ���� �ڷᱸ���� Insert
	// uset���� ������
	//
	// Parameter : ServerNo
	// return : ���� �� false ����
	bool CMatchServer_Lan::InsertLoginPlayerFunc(int ServerNo)
	{
		// 1. uset�� �߰�		
		AcquireSRWLockExclusive(&m_srwl_LoginMatServer_Umap);		// ------- Exclusive ��

		auto ret = m_LoginMatServer_Uset.insert(ServerNo);

		// 2. �ߺ��� Ű�� �� false ����.
		if (ret.second == false)
		{
			ReleaseSRWLockExclusive(&m_srwl_LoginMatServer_Umap);		// ------- Exclusive ���
			return false;
		}

		return true;
	}

	// �α��� �� ���� ���� �ڷᱸ������ Erase
	// uset���� ������
	//
	// Parameter : ServerNo
	// return : ���� �� false ����
	bool CMatchServer_Lan::EraseLoginPlayerFunc(int ServerNo)
	{
		// 1. uset���� �˻�
		AcquireSRWLockExclusive(&m_srwl_LoginMatServer_Umap);	// ------- Exclusive ��

		auto FindPlayer = m_LoginMatServer_Uset.find(ServerNo);
		if (FindPlayer == m_LoginMatServer_Uset.end())
		{
			// ��ã������ false ����
			ReleaseSRWLockExclusive(&m_srwl_LoginMatServer_Umap);	// ------- Exclusive ���

			return false;
		}

		// 2. �� ã������ erase
		m_LoginMatServer_Uset.erase(FindPlayer);

		ReleaseSRWLockExclusive(&m_srwl_LoginMatServer_Umap);	// ------- Exclusive ���

		// 3. true ����
		return true;
	}



	// -----------------
	// ���ο����� ����ϴ� �Լ�
	// -----------------

	// ������ ������ ���� ����
	//
	// Parameter : CMasterServer* 
	// return : ����
	void CMatchServer_Lan::SetParent(CMasterServer* Parent)
	{
		pMasterServer = Parent;
	}




	// -----------------
	// ��Ŷ ó���� �Լ�
	// -----------------

	// Login��Ŷ ó��
	//
	// Parameter : SessionID, Payload
	// return : ����
	void CMatchServer_Lan::Packet_Login(ULONGLONG SessionID, CProtocolBuff_Lan* Payload)
	{
		// 1. ������
		int ServerNo;
		char MasterToken[32];

		Payload->GetData((char*)&ServerNo, 4);
		Payload->GetData(MasterToken, 32);

		// 2. ���� ��ūŰ ��.		
		if (memcmp(MasterToken, pMasterServer->m_stConfig.BattleEnterToken, 32) != 0)
		{
			InterlockedIncrement(&g_lMatch_TokenError);

			// �ٸ� ���, �α� ���, ���� ���� ���´�.
			g_MasterLog->LogSave(L"MasterServer", CSystemLog::en_LogLevel::LEVEL_ERROR, 
				L"Packet_Login() --> EnterToken Error. SessionID : %lld, ServerNo : %d", SessionID, ServerNo);

			Disconnect(SessionID);

			return;
		}

		// 3. �α��� ���� �Ǵ��� ���� �ڷᱸ���� Insert
		if (InsertLoginPlayerFunc(ServerNo) == false)
		{
			InterlockedIncrement(&g_lMatch_DuplicateLogin);
			
			// false�� ���ϵǴ� ���� �̹� �������� ��Ī ����.
			// �α� ���, ���� ���� ���´�.
			g_MasterLog->LogSave(L"MasterServer", CSystemLog::en_LogLevel::LEVEL_ERROR,
				L"Packet_Login() --> Duplicate Login. ServerNo : %d", ServerNo);

			Disconnect(SessionID);

			return;
		}

		// 4. �α��� ���� �ƴ϶��, SessionID�� �̿��� �˻�
		CMasterServer::CMatchingServer* NowUser = FindPlayerFunc(SessionID);
		if (NowUser == nullptr)
			g_MasterDump->Crash();

		// 5. �� ������ �α��� ������ üũ
		// ���� �ȵǴ� ��Ȳ������ Ȥ�ø� ���� üũ
		if (NowUser->m_bLoginCheck == true)
			g_MasterDump->Crash();

		// 6. �α��� Flag ����
		NowUser->m_bLoginCheck = true;

		// 7. ���� ����
		NowUser->m_iServerNo = ServerNo;

		// 8. ������Ŷ ����
		CProtocolBuff_Lan* SendBuff = CProtocolBuff_Lan::Alloc();

		WORD Type = en_PACKET_MAT_MAS_RES_SERVER_ON;
		SendBuff->PutData((char*)&Type, 2);
		SendBuff->PutData((char*)&ServerNo, 4);

		SendPacket(SessionID, SendBuff);
	}


	// �� ���� ��û
	//
	// Parameter : SessionID, Payload
	// return : ����
	void CMatchServer_Lan::Packet_RoomInfo(ULONGLONG SessionID, CProtocolBuff_Lan* Payload)
	{
		// 1. ��Ī ���� �˻�
		CMasterServer::CMatchingServer* NowUser = FindPlayerFunc(SessionID);
		if (NowUser == nullptr)
			g_MasterDump->Crash();


		// 2. ��Ī ������ �α��� ���� Ȯ��
		if (NowUser->m_bLoginCheck == false)
		{
			InterlockedIncrement(&g_lMatch_NotLoginPacket);

			// �α� ����� ���� ����.
			g_MasterLog->LogSave(L"MasterServer", CSystemLog::en_LogLevel::LEVEL_ERROR,
				L"Packet_RoomInfo() --> Not Login Packet. ServerNo : %d", NowUser->m_iServerNo);

			Disconnect(SessionID);

			return;
		}

		// 3. ������
		UINT64 ClinetKey;
		UINT64 AccountNo;

		Payload->GetData((char*)&ClinetKey, 8);
		Payload->GetData((char*)&AccountNo, 8);

		CProtocolBuff_Lan* SendBuff = CProtocolBuff_Lan::Alloc();


		// ----------------- �� ���� ����
		
		AcquireSRWLockExclusive(&pMasterServer->m_srwl_Room_pq);	// ----- pq �� Exclusive ��

		// 1) �� ���� Ȯ��
		if (pMasterServer->m_Room_pq.empty() == true)
		{
			ReleaseSRWLockExclusive(&pMasterServer->m_srwl_Room_pq);	// ----- pq �� Exclusive ���

			// ���� �ϳ��� ������, �� ���� ��Ŷ ����
			WORD Type = en_PACKET_MAT_MAS_RES_GAME_ROOM;
			BYTE Status = 0;

			SendBuff->PutData((char*)&Type, 2);
			SendBuff->PutData((char*)&ClinetKey, 8);
			SendBuff->PutData((char*)&Status, 1);

			SendPacket(SessionID, SendBuff);

			return;
		}

		// 2) �� ���
		CMasterServer::CRoom* NowRoom = pMasterServer->m_Room_pq.top();

		NowRoom->RoomLOCK();		// ----- �� 1���� ���� ��

		// 3) �� ���� ���� ���� �� Ȯ��
		if (NowRoom->m_iEmptyCount == 0)
			g_MasterDump->Crash();

		// 4) ���� ���� �� ����
		NowRoom->m_iEmptyCount--;

		// 5) ���� �� ���� ���� ���� 0���̶��, pop
		if (NowRoom->m_iEmptyCount == 0)
			pMasterServer->m_Room_pq.pop();
		
		ReleaseSRWLockExclusive(&pMasterServer->m_srwl_Room_pq);	// ----- pq �� Exclusive ���

		// 6) Send�� ������ ���� --- 1��
		WORD Type = en_PACKET_MAT_MAS_RES_GAME_ROOM;
		BYTE Status = 1;

		SendBuff->PutData((char*)&Type, 2);
		SendBuff->PutData((char*)&ClinetKey, 8);
		SendBuff->PutData((char*)&Status, 1);

		// 8) �ش� ���� �ִ� ��Ʋ������ ���� �˾ƿ���
		AcquireSRWLockShared(&pMasterServer->m_srwl_BattleServer_Umap);	// ------------- ��Ʋ���� �ڷᱸ�� Shared ��
		
		auto FindBattle = pMasterServer->m_BattleServer_Umap.find(NowRoom->m_iBattleServerNo);

		// ��Ʋ������ ������ Crash
		if (FindBattle == pMasterServer->m_BattleServer_Umap.end())
			g_MasterDump->Crash();

		CMasterServer::CBattleServer* NowBattle = FindBattle->second;

		// 9) Send�� ������ ���� --- 2��
		SendBuff->PutData((char*)&NowBattle->m_iServerNo, 2);
		SendBuff->PutData((char*)NowBattle->m_tcBattleIP, 32);
		SendBuff->PutData((char*)&NowBattle->m_wBattlePort, 2);
		SendBuff->PutData((char*)&NowRoom->m_iRoomNo, 4);
		SendBuff->PutData(NowBattle->m_cConnectToken, 32);
		SendBuff->PutData(NowRoom->m_cEnterToken, 32);

		SendBuff->PutData((char*)NowBattle->m_tcChatIP, 32);
		SendBuff->PutData((char*)NowBattle->m_wChatPort, 2);

		ReleaseSRWLockShared(&pMasterServer->m_srwl_BattleServer_Umap);	// ------------- ��Ʋ���� �ڷᱸ�� Shared ���
					   		 	  	  
		// 10) Ŭ���̾�Ʈ��, Ŭ���̾�Ʈ �ڷᱸ���� �߰�
		if (pMasterServer->InsertPlayerFunc(ClinetKey, AccountNo) == false)
			g_MasterDump->Crash();

		NowRoom->RoomUNLOCK();		// ----- �� 1���� ���� ���

		// ----------------- �� ���� ��

		// 4. �� ���� Send�ϱ�
		SendPacket(SessionID, SendBuff);
	}


	// �� ���� ����
	//
	// Parameter : SessionID, Payload
	// return : ����
	void CMatchServer_Lan::Packet_RoomEnter_OK(ULONGLONG SessionID, CProtocolBuff_Lan* Payload)
	{
		// 1. ��Ī ���� �˻�
		CMasterServer::CMatchingServer* NowUser = FindPlayerFunc(SessionID);
		if (NowUser == nullptr)
			g_MasterDump->Crash();

		// 2. ��Ī ������ �α��� ���� Ȯ��
		if (NowUser->m_bLoginCheck == false)
		{
			InterlockedIncrement(&g_lMatch_NotLoginPacket);

			// �α� ����� ���� ����.
			g_MasterLog->LogSave(L"MasterServer", CSystemLog::en_LogLevel::LEVEL_ERROR,
				L"Packet_RoomEnter_OK() --> Not Login Packet. ServerNo : %d", NowUser->m_iServerNo);

			Disconnect(SessionID);

			return;
		}

		// 3. ������
		WORD BattleServerNo;
		int RoomNo;
		UINT64 ClinetKey;

		Payload->GetData((char*)&BattleServerNo, 2);
		Payload->GetData((char*)&RoomNo, 4);
		Payload->GetData((char*)&ClinetKey, 8);

		// ----- Ȯ�� �� Erase		

		// 1) ClinetKey �ڷᱸ������ ����
		if(pMasterServer->ErasePlayerFunc(ClinetKey) == false)
			g_MasterDump->Crash();


		
	}






	// -----------------------
	// �ܺο��� ��� ������ �Լ�
	// -----------------------

	// ���� ����
	//
	// Parameter : ����
	// return : ���� �� false.
	bool CMatchServer_Lan::ServerStart()
	{
		// LanServer.Start �Լ� ȣ��
		if (Start(pMasterServer->m_stConfig.BindIP, pMasterServer->m_stConfig.Port, pMasterServer->m_stConfig.CreateWorker, 
			pMasterServer->m_stConfig.ActiveWorker, pMasterServer->m_stConfig.CreateAccept, pMasterServer->m_stConfig.Nodelay, pMasterServer->m_stConfig.MaxJoinUser) == false)
		{
			return false;
		}

		// ���� ���� �α� ���		
		g_MasterLog->LogSave(L"MasterServer", CSystemLog::en_LogLevel::LEVEL_SYSTEM, L"MatchServerOpen...");

		return true;		
	}

	// ���� ����
	//
	// Parameter : ����
	// return : ����
	void CMatchServer_Lan::ServerStop()
	{
		// LanServer.Stop()�Լ� ȣ��
		Stop();

		// ���� ����	
		g_MasterLog->LogSave(L"MasterServer", CSystemLog::en_LogLevel::LEVEL_SYSTEM, L"MatchServerClose...");
	}





	// -----------------------
	// �����Լ�
	// -----------------------

	bool CMatchServer_Lan::OnConnectionRequest(TCHAR* IP, USHORT port)
	{
		return true;
	}

	void CMatchServer_Lan::OnClientJoin(ULONGLONG SessionID)
	{
		// 1. CMatchingServer* Alloc
		CMasterServer::CMatchingServer* NewJoin = m_TLSPool_MatchServer->Alloc();

		// 2. SessionID ����
		NewJoin->m_ullSessionID = SessionID;

		// 3. Insert
		if (InsertPlayerFunc(SessionID, NewJoin) == false)
			g_MasterDump->Crash();

	}

	void CMatchServer_Lan::OnClientLeave(ULONGLONG SessionID)
	{
		// 1. �ڷᱸ������ ���� ����
		CMasterServer::CMatchingServer* NowUser = ErasePlayerFunc(SessionID);
		if(NowUser == nullptr)
			g_MasterDump->Crash();

		// 2. �α��� ��Ŷ ����
		if (NowUser->m_bLoginCheck == true)
		{
			// �α��εǾ��� �������, uset������ ����
			if(EraseLoginPlayerFunc(NowUser->m_iServerNo) == false)
				g_MasterDump->Crash();
		}

		// 3. �ʱ�ȭ
		NowUser->m_bLoginCheck = false;

		// 4. ��ȯ
		m_TLSPool_MatchServer->Free(NowUser);
	}

	void CMatchServer_Lan::OnRecv(ULONGLONG SessionID, CProtocolBuff_Lan* Payload)
	{
		// Ÿ�� ������
		WORD Type;
		Payload->GetData((char*)&Type, 2);

		// Ÿ�Կ� ���� �б�ó��
		try
		{
			switch (Type)
			{
				// �� ���� ��û
			case en_PACKET_MAT_MAS_REQ_GAME_ROOM:
				Packet_RoomInfo(SessionID, Payload);
				break;

				// ���� �� ���� ����
			case en_PACKET_MAT_MAS_REQ_ROOM_ENTER_SUCCESS:
				break;

				// ���� �� ���� ����
			case en_PACKET_MAT_MAS_REQ_ROOM_ENTER_FAIL:
				break;

				// ��ġ����ŷ �α��� ��û
			case en_PACKET_MAT_MAS_REQ_SERVER_ON:
				Packet_Login(SessionID, Payload);
				break;				

				// �� �� ��Ŷ�� ���� ó��
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
			g_MasterLog->LogSave(L"MasterServer", CSystemLog::en_LogLevel::LEVEL_ERROR, L"%s",
				(TCHAR*)exc.GetExceptionText());

			// Crash
			g_MasterDump->Crash();

			// ���� ���� ��û
			//Disconnect(SessionID);
		}
	}

	void CMatchServer_Lan::OnSend(ULONGLONG SessionID, DWORD SendSize)
	{}

	void CMatchServer_Lan::OnWorkerThreadBegin()
	{}

	void CMatchServer_Lan::OnWorkerThreadEnd()
	{}

	void CMatchServer_Lan::OnError(int error, const TCHAR* errorStr)
	{}




	// -----------------------
	// �����ڿ� �Ҹ���
	// -----------------------

	// ������
	CMatchServer_Lan::CMatchServer_Lan()
		:CLanServer()
	{	
		// �ڷᱸ�� ���� �̸� ��Ƶα�
		m_MatchServer_Umap.reserve(1000);

		// �� �ʱ�ȭ		
		InitializeSRWLock(&m_srwl_MatchServer_Umap);
		InitializeSRWLock(&m_srwl_LoginMatServer_Umap);

		// TLS �����Ҵ�
		m_TLSPool_MatchServer = new CMemoryPoolTLS<CMasterServer::CMatchingServer>(0, false);

	}

	// �Ҹ���
	CMatchServer_Lan::~CMatchServer_Lan()
	{
		// TLS ���� ����
		delete m_TLSPool_MatchServer;
	}

}



// -----------------------
//
// ��Ʋ�� ����Ǵ� Lan ����
// 
// -----------------------
namespace Library_Jingyu
{
	// -----------------
	// ���ο����� ����ϴ� �Լ�
	// -----------------

	// ������ ������ ���� ����
	//
	// Parameter : CMasterServer* 
	// return : ����
	void CBattleServer_Lan::SetParent(CMasterServer* Parent)
	{
		pMasterServer = Parent;
	}


	// -------------------------------------
	// �ܺο��� ��� ������ �Լ�
	// -------------------------------------

	// ���� ����
	// ���� Lan������ �����Ѵ�.
	// 
	// Parameter : ����
	// return : ���� �� false ����
	bool CBattleServer_Lan::ServerStart()
	{
		// LanServer.Start �Լ� ȣ��
		if (Start(pMasterServer->m_stConfig.BattleBindIP, pMasterServer->m_stConfig.BattlePort, pMasterServer->m_stConfig.BattleCreateWorker,
			pMasterServer->m_stConfig.BattleActiveWorker, pMasterServer->m_stConfig.BattleCreateAccept, pMasterServer->m_stConfig.BattleNodelay, 
			pMasterServer->m_stConfig.BattleMaxJoinUser) == false)
		{
			return false;
		}

		// ���� ���� �α� ���		
		g_MasterLog->LogSave(L"MasterServer", CSystemLog::en_LogLevel::LEVEL_SYSTEM, L"BattServerOpen...");

		return true;
	}


	// ���� ����
	// ���� Lan������ ����
	// 
	// Parameter : ����
	// return : ����
	void CBattleServer_Lan::ServerStop()
	{
		// LanServer.Stop()�Լ� ȣ��
		Stop();

		// ���� ����	
		g_MasterLog->LogSave(L"MasterServer", CSystemLog::en_LogLevel::LEVEL_SYSTEM, L"BattServerClose...");
	}



	// -----------------------
	// �����Լ�
	// -----------------------

	bool CBattleServer_Lan::OnConnectionRequest(TCHAR* IP, USHORT port)
	{
		return true;
	}

	void CBattleServer_Lan::OnClientJoin(ULONGLONG SessionID)
	{}

	void CBattleServer_Lan::OnClientLeave(ULONGLONG SessionID)
	{}

	void CBattleServer_Lan::OnRecv(ULONGLONG SessionID, CProtocolBuff_Lan* Payload)
	{
		// Ÿ�� ������
		WORD Type;
		Payload->GetData((char*)&Type, 2);

		// Ÿ�Կ� ���� �б�ó��
		try
		{
			switch (Type)
			{
				// �ű� ���� ����
			case en_PACKET_BAT_MAS_REQ_CREATED_ROOM:
				break;

				// ���� 1���� �濡�� ����
			case en_PACKET_BAT_MAS_REQ_LEFT_USER:
				break;

				// �� ����
			case en_PACKET_BAT_MAS_REQ_CLOSED_ROOM:
				break;

				// ���� ��ū �����
			case en_PACKET_BAT_MAS_REQ_CONNECT_TOKEN:
				break;

				// ��Ʋ �α��� ��û
			case en_PACKET_BAT_MAS_REQ_SERVER_ON:
				break;

				// �� �� ��Ŷ�� ���� ó��
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
			g_MasterLog->LogSave(L"MasterServer", CSystemLog::en_LogLevel::LEVEL_ERROR, L"%s",
				(TCHAR*)exc.GetExceptionText());

			// Crash
			g_MasterDump->Crash();

			// ���� ���� ��û
			//Disconnect(SessionID);
		}
	}

	void CBattleServer_Lan::OnSend(ULONGLONG SessionID, DWORD SendSize)
	{}

	void CBattleServer_Lan::OnWorkerThreadBegin()
	{}

	void CBattleServer_Lan::OnWorkerThreadEnd()
	{}

	void CBattleServer_Lan::OnError(int error, const TCHAR* errorStr)
	{}







	// -------------------------
	// �����ڿ� �Ҹ���
	// -------------------------

	// ������
	CBattleServer_Lan::CBattleServer_Lan()
	{
		
	}

	// �Ҹ���
	CBattleServer_Lan::~CBattleServer_Lan()
	{
		
	}

}