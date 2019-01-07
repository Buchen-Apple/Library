#include "pch.h"
#include "MasterServer.h"
#include "Log/Log.h"
#include "CrashDump/CrashDump.h"

#include <strsafe.h>

// -----------------------
//
// ������ Match ����
// 
// -----------------------
namespace Library_Jingyu
{
	// ������
	CCrashDump* g_MasterDump = CCrashDump::GetInstance();

	// �α� �����
	CSystemLog* g_MasterLog = CSystemLog::GetInstance();
	

	// -------------------------------------
	// ���ο����� ����ϴ� �Լ�
	// -------------------------------------

	// ���Ͽ��� Config ���� �о����
	// 
	// Parameter : config ����ü
	// return : ���������� ���� �� true
	//		  : �� �ܿ��� false
	bool CMatchServer_Lan::SetFile(stConfigFile* pConfig)
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
		if (Parser.GetValue_String(_T("BattleEnterToken"), tcBattleEnterToken) == false)
			return false;

		// Ŭ�� ���� ���� char������ ������ ������ ��ȯ�ؼ� ������ �־���Ѵ�.
		len = WideCharToMultiByte(CP_UTF8, 0, tcBattleEnterToken, lstrlenW(tcBattleEnterToken), NULL, 0, NULL, NULL);
		WideCharToMultiByte(CP_UTF8, 0, tcBattleEnterToken, lstrlenW(tcBattleEnterToken), pConfig->BattleEnterToken, len, NULL, NULL);


		return true;

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
		m_lMatch_TokenError = 0;
		m_lMatch_DuplicateLogin = 0;
		m_lMatch_NotLoginPacket = 0;


		// ��Ī ������ ����
		if (Start(m_stConfig.BindIP, m_stConfig.Port, m_stConfig.CreateWorker, m_stConfig.ActiveWorker, 
			m_stConfig.CreateAccept, m_stConfig.Nodelay, m_stConfig.MaxJoinUser) == false)
		{
			return false;
		}

		// ��Ī ���� ���� �α� ���		
		g_MasterLog->LogSave(true, L"MasterServer", CSystemLog::en_LogLevel::LEVEL_SYSTEM, L"MatchServerOpen...");

		// ��Ʋ ������ ����
		if(pBattleServer->ServerStart() == false)
		{
			return false;
		}		

		return true;
	}
	
	// ���� ����
	//
	// Parameter : ����
	// return : ����
	void CMatchServer_Lan::ServerStop()
	{
		// ��Ī ������ ����
		Stop();

		// ��Ī ���� ���� �α� ���		
		g_MasterLog->LogSave(true, L"MasterServer", CSystemLog::en_LogLevel::LEVEL_SYSTEM, L"MatchServerClose...");

		// ��Ʋ ������ ����
		if (pBattleServer->GetServerState() == true)
			pBattleServer->ServerStop();
	}

	// ��¿� �Լ�
	//
	// Parameter : ����
	// return : ����
	void CMatchServer_Lan::ShowPrintf()
	{
		// ȭ�� ����� �� ����
		/*
		------------------ Match -------------------
		SessionNum : 		- �������� ��ġ����ŷ �������� ������ ��Ī������ ��	

		Accept Total :		- Accept ��ü ī��Ʈ (accept ���Ͻ� +1)
		Accept TPS :		- �ʴ� Accept ó�� Ƚ��
		Send TPS:			- �ʴ� Send�Ϸ� Ƚ��. (�Ϸ��������� ����)
		Recv TPS:			- �ʴ� Recv�Ϸ� Ƚ��. (��Ŷ 1���� �ϼ��Ǿ��� �� ����. RecvProc���� ��Ŷ�� �ֱ� ���� 1�� ����)
				
		Login_TokenError :	- �α��� �� ��ū ����
		Login_Duplivate :	- �̹� �α��� �� ��Ī������ �� �α��� ��û
		Login_NotLogin :	- �α��� ��Ŷ�� ������ ���� ��Ī������ ��Ŷ�� ����.

		------------------ Battle -------------------
		SessionNum : 		- �������� ��Ʋ �������� ������ ��Ʋ������ ��

		Accept Total :		- Accept ��ü ī��Ʈ (accept ���Ͻ� +1)
		Accept TPS :		- �ʴ� Accept ó�� Ƚ��
		Send TPS:			- �ʴ� Send�Ϸ� Ƚ��. (�Ϸ��������� ����)
		Recv TPS:			- �ʴ� Recv�Ϸ� Ƚ��. (��Ŷ 1���� �ϼ��Ǿ��� �� ����. RecvProc���� ��Ŷ�� �ֱ� ���� 1�� ����)

		Login_TokenError :  - �α��� �� ��ū ����

		------------------ Room -------------------
		TotalRoom :			- �� �� ��
		TotalRoom_Pool :	- �� umap�� �ִ� ī��Ʈ.

		Room_ChunkAlloc_Count : - �Ҵ���� �� ûũ ��(�ۿ��� ������� ��)		

		-------------- ProtocolBuff -------------------
		PacketPool_Lan : 			- �ܺο��� ��� ���� Lan ����ȭ ������ ��

		Lan_BuffChunkAlloc_Count :	- ������� ����ȭ ������ ûũ ��(�ۿ��� ������� ��)

		*/

		printf("==================== Master Server =====================\n"
			"------------------ Match -------------------\n"
			"SessionNum : %lld\n\n"		

			"Accept Total : %lld\n"
			"Accept TPS : %d\n"
			"Send TPS : %d\n"
			"Recv TPS : %d\n\n"		

			"Login_TokenError : %d\n"
			"Login_Duplivate : %d\n"
			"Login_NotLogin : %d\n\n"

			"------------------ Battle -------------------\n"
			"SessionNum : %lld\n\n"

			"Accept Total : %lld\n"
			"Accept TPS : %d\n"
			"Send TPS : %d\n"
			"Recv TPS : %d\n\n"

			"Login_TokenError : %d\n\n"

			"------------------ Room -------------------\n"
			"TotalRoom : %d\n"
			"TotalRoom_Pool : %lld\n\n"

			"Room_ChunkAlloc_Count : %d (Out : %d)\n\n"			

			"---------------- ProtocolBuff -------------\n"
			"PacketPool_Lan : %d\n\n"

			"Lan_BuffChunkAlloc_Count : %d (Out : %d)\n\n"			

			"========================================================\n\n",

			// ----------- ��Ī ������			
			GetClientCount(),

			GetAcceptTotal(),
			GetAcceptTPS(),
			GetSendTPS(),
			GetRecvTPS(),

			m_lMatch_TokenError,
			m_lMatch_DuplicateLogin,
			m_lMatch_NotLoginPacket,

			// ----------- ��Ʋ ������
			pBattleServer->GetClientCount(),

			pBattleServer->GetAcceptTotal(),
			pBattleServer->GetAcceptTPS(),
			pBattleServer->GetSendTPS(),
			pBattleServer->GetRecvTPS(),

			pBattleServer->m_lBattle_TokenError,

			// ----------- ��
			pBattleServer->m_lTotalRoom,
			pBattleServer->m_Room_List.size() + pBattleServer->m_Room_Umap.size(),

			pBattleServer->m_TLSPool_Room->GetAllocChunkCount(), pBattleServer->m_TLSPool_Room->GetOutChunkCount(),

			// ----------- ����ȭ����_��
			CProtocolBuff_Lan::GetNodeCount(),

			CProtocolBuff_Lan::GetChunkCount(), CProtocolBuff_Lan::GetOutChunkCount()
		);
	}



	// -----------------
	// ������ ��Ī ���� ������ �ڷᱸ�� �Լ�
	// -----------------

	// ������ ���� �ڷᱸ���� Insert
	// umap���� ������
	//
	// Parameter : SessionID, stMatching*
	// return : ���� �� false ����
	bool CMatchServer_Lan::InsertMatchServerFunc(ULONGLONG SessionID, stMatching* insertServer)
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

		ReleaseSRWLockExclusive(&m_srwl_MatchServer_Umap);		// ------- Exclusive ���
		return true;
	}


	// ������ ���� �ڷᱸ������, ���� �˻�
	// ���� umap���� ������
	// 
	// Parameter : SessionID
	// return : �˻� ���� ��, stMatchingr*
	//		  : �˻� ���� �� nullptr
	CMatchServer_Lan::stMatching* CMatchServer_Lan::FindMatchServerFunc(ULONGLONG SessionID)
	{
		// 1. umap���� �˻�
		AcquireSRWLockShared(&m_srwl_MatchServer_Umap);		// ------- Shared ��

		auto FindPlayer = m_MatchServer_Umap.find(SessionID);		

		// 2. �˻� ���� �� nullptr ����
		if (FindPlayer == m_MatchServer_Umap.end())
		{
			ReleaseSRWLockShared(&m_srwl_MatchServer_Umap);		// ------- Shared ���
			return nullptr;
		}

		// 3. �˻� ���� ��, ã�� stPlayer* ����
		stMatching* ReturnData = FindPlayer->second;

		ReleaseSRWLockShared(&m_srwl_MatchServer_Umap);		// ------- Shared ���

		return ReturnData;
	}


	// ������ ���� �ڷᱸ������ Erase
	// umap���� ������
	//
	// Parameter : SessionID
	// return : ���� ��, ���ŵ� stMatching*
	//		  : �˻� ���� ��(���������� ���� ����) nullptr
	CMatchServer_Lan::stMatching* CMatchServer_Lan::EraseMatchServerFunc(ULONGLONG SessionID)
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
		stMatching* ret = FindPlayer->second;

		// 3. umap���� ����
		m_MatchServer_Umap.erase(FindPlayer);

		ReleaseSRWLockExclusive(&m_srwl_MatchServer_Umap);		// ------- Exclusive ���

		return ret;
	}





	// -----------------
	// ������ ��Ī ���� ������ �ڷᱸ�� �Լ�(�α��� �� ���� ����)
	// -----------------

	// �α��� �� ���� ���� �ڷᱸ���� Insert
	// uset���� ������
	//
	// Parameter : ServerNo
	// return : ���� �� false ����
	bool CMatchServer_Lan::InsertLoginMatchServerFunc(int ServerNo)
	{
		// 1. uset�� �߰�		
		AcquireSRWLockExclusive(&m_srwl_LoginMatServer_Uset);		// ------- Exclusive ��

		auto ret = m_LoginMatServer_Uset.insert(ServerNo);

		ReleaseSRWLockExclusive(&m_srwl_LoginMatServer_Uset);		// ------- Exclusive ���

		// 2. �ߺ��� Ű�� �� false ����.
		if (ret.second == false)		
			return false;

		return true;
	}

	// �α��� �� ���� ���� �ڷᱸ������ Erase
	// uset���� ������
	//
	// Parameter : ServerNo
	// return : ���� �� false ����
	bool CMatchServer_Lan::EraseLoginMatchServerFunc(int ServerNo)
	{
		// 1. uset���� �˻�
		AcquireSRWLockExclusive(&m_srwl_LoginMatServer_Uset);	// ------- Exclusive ��

		auto FindPlayer = m_LoginMatServer_Uset.find(ServerNo);
		if (FindPlayer == m_LoginMatServer_Uset.end())
		{
			// ��ã������ false ����
			ReleaseSRWLockExclusive(&m_srwl_LoginMatServer_Uset);	// ------- Exclusive ���

			return false;
		}

		// 2. �� ã������ erase
		m_LoginMatServer_Uset.erase(FindPlayer);

		ReleaseSRWLockExclusive(&m_srwl_LoginMatServer_Uset);	// ------- Exclusive ���

		// 3. true ����
		return true;

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
		if (memcmp(MasterToken, m_stConfig.EnterToken, 32) != 0)
		{
			InterlockedIncrement(&m_lMatch_TokenError);

			g_MasterDump->Crash();

			/*
			// �ٸ� ���, �α� ���, ���� ���� ���´�.
			g_MasterLog->LogSave(false, L"MasterServer", CSystemLog::en_LogLevel::LEVEL_ERROR,
				L"Packet_Login() --> EnterToken Error. SessionID : %lld, ServerNo : %d", SessionID, ServerNo);

			Disconnect(SessionID);

			return;
			*/
		}

		// 3. �ߺ��α��� üũ�� ���� �ڷᱸ���� Insert
		if (InsertLoginMatchServerFunc(ServerNo) == false)
		{
			InterlockedIncrement(&m_lMatch_DuplicateLogin);

			g_MasterDump->Crash();

			/*
			// false�� ���ϵǴ� ���� �̹� �������� ��Ī ����.
			// �α� ���, ���� ���� ���´�.
			g_MasterLog->LogSave(false, L"MasterServer", CSystemLog::en_LogLevel::LEVEL_ERROR,
				L"Packet_Login() --> Duplicate Login. ServerNo : %d", ServerNo);

			Disconnect(SessionID);

			return;
			*/
		}

		// 4. �α��� ���� �ƴ϶��, SessionID�� �̿��� �˻�
		stMatching* NowUser = FindMatchServerFunc(SessionID);
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
	// ���� ���� ��, Battle�� ����
	//
	// Parameter : SessionID, Payload
	// return : ����
	void CMatchServer_Lan::Relay_RoomInfo(ULONGLONG SessionID, CProtocolBuff_Lan* Payload)
	{
		// 1. ��Ī ���� �˻�
		stMatching* NowUser = FindMatchServerFunc(SessionID);
		if (NowUser == nullptr)
			g_MasterDump->Crash();


		// 2. ��Ī ������ �α��� ���� Ȯ��
		if (NowUser->m_bLoginCheck == false)
		{
			InterlockedIncrement(&m_lMatch_NotLoginPacket);

			// �α� ����� ���� ����.
			g_MasterLog->LogSave(false, L"MasterServer", CSystemLog::en_LogLevel::LEVEL_ERROR,
				L"Packet_RoomInfo() --> Not Login Packet. ServerNo : %d", NowUser->m_iServerNo);

			Disconnect(SessionID);

			return;
		}

		// 3. ������
		UINT64 ClinetKey;
		Payload->GetData((char*)&ClinetKey, 8);

		// 4. ��Ʋ�� �������� ������
		// ���� ó���� ��Ʋ�� �������� ���
		pBattleServer->Relay_Battle_Room_Info(SessionID, ClinetKey, NowUser->m_iServerNo);
	}
	
	// �� ���� ����
	//
	// Parameter : SessionID, Payload
	// return : ����
	void CMatchServer_Lan::Packet_RoomEnter_OK(ULONGLONG SessionID, CProtocolBuff_Lan* Payload)
	{
		// 1. ��Ī ���� �˻�
		stMatching* NowUser = FindMatchServerFunc(SessionID);
		if (NowUser == nullptr)
			g_MasterDump->Crash();

		// 2. ��Ī ������ �α��� ���� Ȯ��
		if (NowUser->m_bLoginCheck == false)
		{
			InterlockedIncrement(&m_lMatch_NotLoginPacket);

			// �α� ����� ���� ����.
			g_MasterLog->LogSave(false, L"MasterServer", CSystemLog::en_LogLevel::LEVEL_ERROR,
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
		
		// ------------ ��Ʋ �� ������ �ڷᱸ������ ����
		// 1. ��Ʋ�� �Լ� ȣ��. 0�� ���ϵǾ�� ����
		// ���ο��� stPlayer*�� Free���� �Ѵ�.
		if(pBattleServer->RoomEnter_OK_Func(ClinetKey, RoomNo, BattleServerNo) != 0)
			g_MasterDump->Crash();

	}
	
	// �� ���� ����
	//
	// Parameterr : SessionID, Payload
	// return : ����
	void CMatchServer_Lan::Packet_RoomEnter_Fail(ULONGLONG SessionID, CProtocolBuff_Lan* Payload)
	{
		// 1. ��Ī ���� �˻�
		stMatching* NowUser = FindMatchServerFunc(SessionID);
		if (NowUser == nullptr)
			g_MasterDump->Crash();

		// 2. ��Ī ������ �α��� ���� Ȯ��
		if (NowUser->m_bLoginCheck == false)
		{
			InterlockedIncrement(&m_lMatch_NotLoginPacket);

			// �α� ����� ���� ����.
			g_MasterLog->LogSave(false, L"MasterServer", CSystemLog::en_LogLevel::LEVEL_ERROR,
				L"Packet_RoomEnter_OK() --> Not Login Packet. ServerNo : %d", NowUser->m_iServerNo);

			Disconnect(SessionID);

			return;
		}

		// 3. ������
		UINT64 ClinetKey;
		Payload->GetData((char*)&ClinetKey, 8);

		// ------------ ��Ʋ �� ������ �ڷᱸ������ ����	
		pBattleServer->RoomEnter_Fail_Func(ClinetKey);

		/*
		// ------------ ��Ʋ �� ������ �ڷᱸ������ ����		
		// 1. ��Ʋ�� �Լ� ȣ��.
		// Room�� üũ�� ī��Ʈ ����ó��
		// ���ο��� stPlayer*�� Free���� �Ѵ�.
		int Ret = pBattleServer->RoomEnter_Fail_Func(ClinetKey);

		// 0 : ����
		// 1 : ���� ����
		// 2 : Room ����
		// 3 : Room �ȿ� �ش� ���� ����
		// �� ��, 0, 2, 3�� ����. 1�� ������
		if (Ret == 1)
			g_MasterDump->Crash();
		*/
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
		stMatching* NewJoin = m_TLSPool_MatchServer->Alloc();

		// 2. SessionID ����
		NewJoin->m_ullSessionID = SessionID;

		// 3. Insert
		if (InsertMatchServerFunc(SessionID, NewJoin) == false)
			g_MasterDump->Crash();

	}

	void CMatchServer_Lan::OnClientLeave(ULONGLONG SessionID)
	{
		// 1. �ڷᱸ������ ��Ī���� ����
		stMatching* NowUser = EraseMatchServerFunc(SessionID);
		if (NowUser == nullptr)
			g_MasterDump->Crash();

		// 2. ��Ʋ �� ������ stPlayer* ��, �ش� ��Ī������ �����Ҵ� �Ѱ� ���������� ��� �����ش�.
		pBattleServer->MatchLeave(NowUser->m_iServerNo);

		// 3. �α��� ��Ŷ ����
		if (NowUser->m_bLoginCheck == true)
		{
			// �α��εǾ��� ��Ī �������, uset������ ����
			if (EraseLoginMatchServerFunc(NowUser->m_iServerNo) == false)
				g_MasterDump->Crash();

			// �ʱ�ȭ
			NowUser->m_bLoginCheck = false;
		}			

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
			// ���� �� ���� ��Ŷ ������� switch ~ case ����.
			//
			// �� ���� ��û�� ��ġ����ŷ ������ �ϴ� ���� ū ��Ȱ�̸� �߿䵵 1��. ������ ���� �� ������ ����Ǿ� 1�������� ����
			// ������ �޾ư� ������ ���� �� �濡 ������ ���̱� ������ ���� �� ���� ������ 2������ ����
			// ���� �� ���� ���д� ���� �߻� ���Ѵٰ� ����(������ �� ���� ���� �� ��Ʋ�濡 ���� ���� ��Ȳ)
			// ��ġ����ŷ �����κ��� ���� �α��� ��û��, ������ ������ ���� ��, �� �� �ȿð��̱� ������ ���� �ļ���.
			switch (Type)
			{
				// �� ���� ��û
			case en_PACKET_MAT_MAS_REQ_GAME_ROOM:
				Relay_RoomInfo(SessionID, Payload);
				break;			

				// ���� �� ���� ����
			case en_PACKET_MAT_MAS_REQ_ROOM_ENTER_SUCCESS:
				Packet_RoomEnter_OK(SessionID, Payload);
				break;

				// ���� �� ���� ����
			case en_PACKET_MAT_MAS_REQ_ROOM_ENTER_FAIL:
				Packet_RoomEnter_Fail(SessionID, Payload);
				break;		

				// ��ġ����ŷ �α��� ��û
			case en_PACKET_MAT_MAS_REQ_SERVER_ON:
				Packet_Login(SessionID, Payload);
				break;

				// Ÿ�� ����
			default:
				TCHAR ErrStr[1024];
				StringCchPrintf(ErrStr, 1024, _T("MatchAread OnRecv(). TypeError. Type : %d, SessionID : %lld"),
					Type, SessionID);

				throw CException(ErrStr);
			}

		}
		catch (CException& exc)
		{
			// �α� ��� (�α� ���� : ����)
			g_MasterLog->LogSave(false, L"MasterServer", CSystemLog::en_LogLevel::LEVEL_ERROR, L"%s",
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
		//  Config���� ����		
		if (SetFile(&m_stConfig) == false)
			g_MasterDump->Crash();

		// �α� ������ ���� ����
		g_MasterLog->SetDirectory(L"MasterServer");
		g_MasterLog->SetLogLeve((CSystemLog::en_LogLevel)m_stConfig.LogLevel);
		
		// �ڷᱸ�� ���� �̸� ��Ƶα�
		m_MatchServer_Umap.reserve(100);
		m_LoginMatServer_Uset.reserve(100);

		// �� �ʱ�ȭ		
		InitializeSRWLock(&m_srwl_MatchServer_Umap);
		InitializeSRWLock(&m_srwl_LoginMatServer_Uset);

		// TLS �����Ҵ�
		m_TLSPool_MatchServer = new CMemoryPoolTLS<stMatching>(0, false);

		// ��Ʋ ���� �����Ҵ�
		pBattleServer = new CBattleServer_Lan;
		pBattleServer->SetParent(this);
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
// ������ Battle ����
// 
// -----------------------
namespace Library_Jingyu
{

	// ----------------------------------------
	// ������ ��Ʋ���� ������ �ڷᱸ�� �Լ�
	// ----------------------------------------

	// ��Ʋ���� ���� �ڷᱸ���� ��Ʋ���� Insert
	//
	// Parameter : SessionID, stBattle*
	// return : ���� �� true
	//		  : ���� �� false(�ߺ�Ű)
	bool CBattleServer_Lan::InsertBattleServerFunc(ULONGLONG SessionID, stBattle* InsertBattle)
	{
		AcquireSRWLockExclusive(&m_srwl_BattleServer_Umap);		// ----- Battle���� Umap Exclusive ��
		
		// 1. �ڷᱸ���� Insert
		auto ret = m_BattleServer_Umap.insert(make_pair(SessionID, InsertBattle));

		ReleaseSRWLockExclusive(&m_srwl_BattleServer_Umap);		// ----- Battle���� Umap Exclusive ���
	
		// 2. �ߺ� Ű��� false ����
		if (ret.second == false)
			return false;

		return true;	
	}

	// ��Ʋ ���� ���� �ڷᱸ������ ��Ʋ���� Find
	//
	// Parameter : SessionID
	// return : ���������� ã�� �� stBattle*
	//		  : ���� ��, nullptr
	CBattleServer_Lan::stBattle* CBattleServer_Lan::FindBattleServerFunc(ULONGLONG SessionID)
	{
		AcquireSRWLockShared(&m_srwl_BattleServer_Umap);		// ----- Battle���� Umap Shared ��

		// 1. �˻�
		auto Ret = m_BattleServer_Umap.find(SessionID);

		// 2. ������ nullptr
		if (Ret == m_BattleServer_Umap.end())
		{
			ReleaseSRWLockShared(&m_srwl_BattleServer_Umap);		// ----- Battle���� Umap Shared ���
			return nullptr;
		}

		// 2. ������ �ش� ������ ����
		stBattle* RetData = Ret->second;

		ReleaseSRWLockShared(&m_srwl_BattleServer_Umap);		// ----- Battle���� Umap Shared ���

		return RetData;
	}

	// ��Ʋ���� ���� �ڷᱸ������ ��Ʋ���� erase
	//
	// Parameter : SessionID
	// return : ���� �� stBattle*
	//		  : ���� �� nullptr
	CBattleServer_Lan::stBattle* CBattleServer_Lan::EraseBattleServerFunc(ULONGLONG SessionID)
	{
		AcquireSRWLockExclusive(&m_srwl_BattleServer_Umap);		// ----- Battle���� Umap Exclusive ��

		// 1. �ڷᱸ������ find
		auto FIndBattle = m_BattleServer_Umap.find(SessionID);

		// ��ã������ nullptr ����
		if (FIndBattle == m_BattleServer_Umap.end())
		{
			ReleaseSRWLockExclusive(&m_srwl_BattleServer_Umap);		// ----- Battle���� Umap Exclusive ���
			return nullptr;
		}

		// 2. ������ �� �޾Ƶα�
		stBattle* EraseBattle = FIndBattle->second;

		// 3. Erase
		m_BattleServer_Umap.erase(FIndBattle);

		ReleaseSRWLockExclusive(&m_srwl_BattleServer_Umap);		// ----- Battle���� Umap Exclusive ���

		return EraseBattle;
	}




	// -----------------
	// �÷��̾� ������ �ڷᱸ�� �Լ�
	// -----------------

	// �÷��̾� ���� �ڷᱸ���� �÷��̾� Insert
	//
	// Parameter : ClientKey, stPlayer*
	// return : ���������� Insert �� true
	//		  : ���� �� false
	bool CBattleServer_Lan::InsertPlayerFunc(UINT64 ClientKey, stPlayer* InsertPlayer)
	{
		// 1. ��Ʋ�� �÷��̾� �ڷᱸ���� �߰�		
		AcquireSRWLockExclusive(&m_srwl_Player_Umap);		// ------- Exclusive ��

		auto ret = m_Player_Umap.insert(make_pair(ClientKey, InsertPlayer));

		ReleaseSRWLockExclusive(&m_srwl_Player_Umap);		// ------- Exclusive ���

		// 2. �ߺ��� Ű�� �� false ����.
		if (ret.second == false)		
			return false;

		return true;
	}





	// -----------------------------
	// �������� ��Ī�ʿ��� ȣ���ϴ� �Լ�
	// -----------------------------

	// ��Ī���� �� ���� ���� ��Ŷ�� �� �� ȣ��Ǵ� �Լ�
	// 1. RoomNo, BattleServerNo�� ���� ��ġ�ϴ��� Ȯ��
	// 2. �ڷᱸ������ Erase
	// 3. stPlayer*�� �޸�Ǯ�� Free
	//
	// !! ��Ī �� �������� ȣ�� !!
	//
	// Parameter : ClinetKey, RoomNo, BattleServerNo
	//
	// return Code
	// 0 : ���������� ������
	// 1 : �������� �ʴ� ���� (ClinetKey�� ���� �˻� ����)
	// 2 : RoomNo ����ġ
	// 3 : BattleServerNo ����ġ
	int CBattleServer_Lan::RoomEnter_OK_Func(UINT64 ClinetKey, int RoomNo, int BattleServerNo)
	{
		AcquireSRWLockExclusive(&m_srwl_Player_Umap);		// ----- Battle Player �ڷᱸ�� ��

		// 1. ���� �˻�
		auto FIndPlayer = m_Player_Umap.find(ClinetKey);

		// �������� ������ return false
		if (FIndPlayer == m_Player_Umap.end())
		{
			ReleaseSRWLockExclusive(&m_srwl_Player_Umap);		// ----- Battle Player �ڷᱸ�� ���
			return 1;
		}

		stPlayer* ErasePlayer = FIndPlayer->second;


		// 2. RoomNo üũ
		if (ErasePlayer->m_iJoinRoomNo != RoomNo)
		{
			ReleaseSRWLockExclusive(&m_srwl_Player_Umap);		// ----- Battle Player �ڷᱸ�� ���
			return 2;
		}


		// 3. BattleServerNo üũ
		if (ErasePlayer->m_iBattleServerNo != BattleServerNo)
		{
			ReleaseSRWLockExclusive(&m_srwl_Player_Umap);		// ----- Battle Player �ڷᱸ�� ���
			return 3;
		}


		// 4. ��� ��ġ�ϸ� Erase
		m_Player_Umap.erase(FIndPlayer);

		ReleaseSRWLockExclusive(&m_srwl_Player_Umap);		// ----- Battle Player �ڷᱸ�� ���


		// 5. stPlayer* ��ȯ
		m_TLSPool_Player->Free(ErasePlayer);

		// 6. ���� �ڵ� ����
		return 0;
	}

	// ��Ī���� �� ���� ���а� �� �� ȣ��Ǵ� �Լ�
	// 1. ���� �˻�
	// 2. �ش� ClinetKey�� ������ �� ã��
	// 3. ã�� ���� Set���� �ش� ���� ����
	// 4. stPlayer*�� �޸�Ǯ�� Free
	//
	// !! ��Ī �� �������� ȣ�� !!
	//
	// Parameter : ClinetKey
	//
	// return Code
	// 0 : ���������� ������
	// 1 : �������� �ʴ� ���� (ClinetKey�� ���� �˻� ����)
	// 2 : Room �������� ����
	// 3 : Room ���� �ڷᱸ��(Set)���� ClientKey�� �˻� ����
	int CBattleServer_Lan::RoomEnter_Fail_Func(UINT64 ClinetKey)
	{
		// 1. ���� �˻�
		AcquireSRWLockExclusive(&m_srwl_Player_Umap);		// ----- Battle Player �ڷᱸ�� Exclusive ��
		auto FIndPlayer = m_Player_Umap.find(ClinetKey);

		// �������� ������ return 1
		if (FIndPlayer == m_Player_Umap.end())
		{
			ReleaseSRWLockExclusive(&m_srwl_Player_Umap);		// ----- Battle Player �ڷᱸ�� Exclusive ���
			return 1;
		}

		stPlayer * ErasePlayer = FIndPlayer->second;

		// �� Key�� �� No �޾Ƶα�
		UINT64 RoomKey = ErasePlayer->m_ullRoomKey;
		int RoomNo = ErasePlayer->m_iJoinRoomNo;

		// Player Erase
		m_Player_Umap.erase(FIndPlayer);

		ReleaseSRWLockExclusive(&m_srwl_Player_Umap);		// ----- Battle Player �ڷᱸ�� Exclusive ���

		// stPlayer* ��ȯ
		m_TLSPool_Player->Free(ErasePlayer);



		bool bFalg = false;		// ����Ʈ���� �� ã�Ҵ��� ����
		AcquireSRWLockExclusive(&m_srwl_Room_List);		// ----- Room list �ڷᱸ�� Exclusive ��

		// 2. list ���� �濡 ���� ���, list�� 0�� �ε����� �ش� ������ �����Ϸ��� �ߴ� ������ Ȯ��
		if (m_Room_List.size() > 0)
		{
			stRoom* NowRoom = *(m_Room_List.begin());			

			// �ش� ������ �����ߴ� ���� Room_list�� 0�� �ε����� ���� ���
			// Room ������ ���� �� �� set ����
			if (NowRoom->m_iRoomNo == RoomNo)
			{
				// 1) Set���� ���� �˻�
				auto TempUser = NowRoom->m_uset_JoinUser.find(ClinetKey);

				// �������� ������ return 3
				if (TempUser == NowRoom->m_uset_JoinUser.end())
				{					
					ReleaseSRWLockExclusive(&m_srwl_Room_List);		// ----- Room list �ڷᱸ�� Exclusive ���
					return 3;
				}

				// 2) �����ϸ� Set���� ����.
				NowRoom->m_uset_JoinUser.erase(TempUser);

				// 3) �� ���� �׿� ���� �� 1 ����	
				NowRoom->m_iEmptyCount++;

				// �����ߴµ� 1���� �۴ٸ� ���� �ȵ�
				if (NowRoom->m_iEmptyCount < 1)
					g_MasterDump->Crash();

				bFalg = true;
				
				ReleaseSRWLockExclusive(&m_srwl_Room_List);		// ----- Room list �ڷᱸ�� Exclusive ���	
			}				
		}


		// 3. list ���� ���� ���ų�, ����Ʈ�� �濡 �ִ� ������ �ƴ� ��� umap���� �� �˻�
		if(bFalg == false)
		{
			ReleaseSRWLockExclusive(&m_srwl_Room_List);		// ----- Room list �ڷᱸ�� Exclusive ���

			AcquireSRWLockShared(&m_srwl_Room_Umap);		// ----- Room umap �ڷᱸ�� Shared ��

			// 1) ���ϴ� �� �˻�
			auto FindRoom = m_Room_Umap.find(RoomKey);

			// ���ϴ� ���� ������ return 2 (�� �������� ����)
			// list, umap ��ο� �������� �ʴ´ٴ� ��.
			if (FindRoom == m_Room_Umap.end())
			{
				ReleaseSRWLockShared(&m_srwl_Room_Umap);		// ----- Room umap �ڷᱸ�� Shared ���
				return 2;
			}

			stRoom* NowRoom = FindRoom->second;

			NowRoom->RoomLOCK();			// ----- �� 1���� ���� �� (umap�� Shared�̱� ������ �� �� �ʿ�.)

			// 2) Set���� ���� �˻�
			auto TempUser = NowRoom->m_uset_JoinUser.find(ClinetKey);

			// �������� ������ return 3
			if (TempUser == NowRoom->m_uset_JoinUser.end())
			{
				ReleaseSRWLockShared(&m_srwl_Room_Umap);		// ----- Room umap �ڷᱸ�� Shared ���
				NowRoom->RoomUNLOCK();							// ----- �� 1���� ���� ���
				return 3;
			}

			// 3) �����ϸ� ����.
			NowRoom->m_uset_JoinUser.erase(TempUser);

			// 4) �볻�� �׿� ���� �� 1 ����	
			NowRoom->m_iEmptyCount++;

			// �����ߴµ� 1���� �۴ٸ� ���� �ȵ�
			if (NowRoom->m_iEmptyCount < 1)
				g_MasterDump->Crash();

			ReleaseSRWLockShared(&m_srwl_Room_Umap);		// ----- Room umap �ڷᱸ�� Shared ���
			NowRoom->RoomUNLOCK();							// ----- �� 1���� ���� ���			
		}

		// 4. ���� �ڵ� ����
		return 0;
	}
		
	// ��Ī������ ������ ���� ��, �ش� ��Ī������ ���� �Ҵ�� stPlayer*�� ��� ��ȯ�ϴ� �Լ�
	//
	// Parameter : ��Ī ���� No(int)
	// return : ����
	void CBattleServer_Lan::MatchLeave(int MatchServerNo)
	{
		AcquireSRWLockExclusive(&m_srwl_Player_Umap);	// ----- �÷��̾� uamp Exclusive ��

		// ��ȸ�ϸ鼭 ���ڷ� ���� MatchServerNo�� ������ �ִ� stPlayer*�� Free �Ѵ�
		auto itor_Now = m_Player_Umap.begin();
		auto itor_End = m_Player_Umap.end();

		while (1)
		{
			if (itor_Now == itor_End)
				break;

			stPlayer* NowPlayer = itor_Now->second;

			if (NowPlayer->m_iMatchServerNo == MatchServerNo)
			{
				itor_Now = m_Player_Umap.erase(itor_Now);
				m_TLSPool_Player->Free(NowPlayer);
			}

			else
				++itor_Now;
		}

		ReleaseSRWLockExclusive(&m_srwl_Player_Umap);	// ----- �÷��̾� uamp Exclusive ���
	}

	// �� ���� ��û ��Ŷ ó��
	// !! ��Ī �� �������� ȣ�� !!
	//
	// Parameter : SessionID(��Ī ���� SessionID), ClientKey, ��Ī������ No
	// return : ����
	void CBattleServer_Lan::Relay_Battle_Room_Info(ULONGLONG SessionID, UINT64 ClientKey, int MatchServerNo)
	{
		int iSendRoomNo;
		int iBattleServerNo;
		ULONGLONG ullBattleSessionID;
		char cSendEnterToken[32];
		bool bSearchFlag = false;	// umap �ڷᱸ������ �濡 ���� ���� ã���� �÷���. false�� ��ã��.


		// -------------------------- �� ���� �˾ƿ���

		AcquireSRWLockShared(&m_srwl_Room_Umap);	// ----- umap �� Shared ��

		// umap �� �ڷᱸ���� �� �ִ��� Ȯ��
		if (m_Room_Umap.size() > 0)
		{
			// �� ���
			auto itor_Now = m_Room_Umap.begin();
			auto itor_End = m_Room_Umap.end();

			while (1)
			{
				if (itor_Now == itor_End)
					break;

				// ���� ã���� ��� ����
				if (itor_Now->second->m_iEmptyCount > 0)
				{				
					stRoom* NowRoom = itor_Now->second;

					NowRoom->RoomLOCK();		// ----- �� 1���� ���� �� (umap ���� Shared�̱� ������ �ʿ�)

					// 1. �� ���� ���� ���� �� Ȯ��
					// 0���̸� �� �˻����� ���õǾ����� �ȵ�. ���� ������ ����.
					if (NowRoom->m_iEmptyCount == 0)
						g_MasterDump->Crash();


					// 2. ���� �׿� ���� �� ����, �� �� �ڷᱸ���� �߰�
					NowRoom->m_iEmptyCount--;

					auto ret = NowRoom->m_uset_JoinUser.insert(ClientKey);
					if (ret.second == false)
						g_MasterDump->Crash();


					// 3. Room���� �ʿ��� ������ �޾Ƶα�
					iSendRoomNo = NowRoom->m_iRoomNo;
					iBattleServerNo = NowRoom->m_iBattleServerNo;
					ullBattleSessionID = NowRoom->m_ullBattleSessionID;
					memcpy_s(cSendEnterToken, 32, NowRoom->m_cEnterToken, 32);

					NowRoom->RoomUNLOCK();						// ----- �� 1���� ���� ���
					ReleaseSRWLockShared(&m_srwl_Room_Umap);	// ----- umap �� Shared ���					

					// 4.�� ã���� Flag ����
					bSearchFlag = true;

					break;
				}

				++itor_Now;
			}			
		}

		// umap���� ������ �� �� �ִ� ���� ��ã�Ұų�, umap �� �ڷᱸ���� ���� �ϳ��� ���� ���
		// list���� Ȯ��
		if (bSearchFlag == false)
		{
			ReleaseSRWLockShared(&m_srwl_Room_Umap);	// ----- umap �� Shared ���

			AcquireSRWLockExclusive(&m_srwl_Room_List);	// ----- list �� Exclusive ��

			// 1. list�� ���� �ֳ� Ȯ��.
			// ���⼭�� ������ ������ ���� �Ҵ� ������ ���� �ϳ��� ���°�.
			if (m_Room_List.size() == 0)
			{
				ReleaseSRWLockExclusive(&m_srwl_Room_List);	// ----- list �� Exclusive ���

				// ���� �ϳ��� ������, �� ���� ��Ŷ ����
				CProtocolBuff_Lan* SendBuff = CProtocolBuff_Lan::Alloc();
				WORD Type = en_PACKET_MAT_MAS_RES_GAME_ROOM;
				BYTE Status = 0;

				SendBuff->PutData((char*)&Type, 2);
				SendBuff->PutData((char*)&ClientKey, 8);
				SendBuff->PutData((char*)&Status, 1);

				// ��Ī �������� �� ���� ��Ŷ ������
				pMatchServer->SendPacket(SessionID, SendBuff);

				return;
			}


			// 2. ���� ���� �� ���
			stRoom* NowRoom = *(m_Room_List.begin());

			// 3. �� ���� ���� ���� �� Ȯ��
			// 0���̸� ���� ������ �ȵ�.
			if (NowRoom->m_iEmptyCount == 0)
				g_MasterDump->Crash();


			// 4. Room���� �ʿ��� ������ �޾Ƶα�
			iSendRoomNo = NowRoom->m_iRoomNo;
			iBattleServerNo = NowRoom->m_iBattleServerNo;
			ullBattleSessionID = NowRoom->m_ullBattleSessionID;
			memcpy_s(cSendEnterToken, 32, NowRoom->m_cEnterToken, 32);



			// 5. ���� �׿� ���� �� ����, �� �� �ڷᱸ���� �߰�
			NowRoom->m_iEmptyCount--;

			auto ret = NowRoom->m_uset_JoinUser.insert(ClientKey);
			if (ret.second == false)
				g_MasterDump->Crash();



			// 6. ���� �� ���� ���� ���� 0���̶��, list���� �� ��, umap�� �߰�
			if (NowRoom->m_iEmptyCount == 0)
			{
				AcquireSRWLockExclusive(&m_srwl_Room_Umap);	// ----- umap �� Exclusive ��

				// list���� pop
				m_Room_List.pop_front();

				// umap�� Insert
				auto ret = m_Room_Umap.insert(make_pair(NowRoom->m_ui64RoomKey, NowRoom));
				if (ret.second == false)
					g_MasterDump->Crash();

				ReleaseSRWLockExclusive(&m_srwl_Room_List);	// ----- list �� Exclusive ���
				ReleaseSRWLockExclusive(&m_srwl_Room_Umap);	// ----- umap �� Exclusive ���
			}

			// ���� ���� ���� 0 �̻��̶��, �׳� ���� �����ϰ� ������.
			else
			{
				ReleaseSRWLockExclusive(&m_srwl_Room_List);		// ----- list �� Exclusive ���
			}
		}

		// ---------------------------- ��Ŷ �����ϱ�
		// ������� ������ ���� ã�Ҵٴ� ��.

		// 1. Send�� ������ ���� --- 1��
		CProtocolBuff_Lan* SendBuff = CProtocolBuff_Lan::Alloc();
		WORD Type = en_PACKET_MAT_MAS_RES_GAME_ROOM;

		SendBuff->PutData((char*)&Type, 2);
		SendBuff->PutData((char*)&ClientKey, 8);


		// 2. �ش� ���� �ִ� ��Ʋ������ ���� �˾ƿ���
		AcquireSRWLockShared(&m_srwl_BattleServer_Umap);	// ------------- ��Ʋ���� �ڷᱸ�� Shared ��

		auto FindBattle = m_BattleServer_Umap.find(ullBattleSessionID);

		// ���⼭ ��Ʋ������ ���� ���� ��Ʋ������ �׾��ٴ� �� �ۿ� �ȵ�.
		if (FindBattle == m_BattleServer_Umap.end())
		{
			ReleaseSRWLockShared(&m_srwl_BattleServer_Umap);	// ------------- ��Ʋ���� �ڷᱸ�� Shared ���

			//g_MasterDump->Crash();

			// ��Ʋ������ �׾��� ������, �� ���� ��Ŷ ����
			BYTE Status = 0;

			SendBuff->PutData((char*)&Status, 1);

			SendPacket(SessionID, SendBuff);

			return;
		}

		stBattle* NowBattle = FindBattle->second;


		// 3. Send�� ������ ���� --- 2��
		BYTE Status = 1;
		SendBuff->PutData((char*)&Status, 1);

		SendBuff->PutData((char*)&NowBattle->m_iServerNo, 2);
		SendBuff->PutData((char*)NowBattle->m_tcBattleIP, 32);
		SendBuff->PutData((char*)&NowBattle->m_wBattlePort, 2);
		SendBuff->PutData((char*)&iSendRoomNo, 4);
		SendBuff->PutData(NowBattle->m_cConnectToken, 32);
		SendBuff->PutData(cSendEnterToken, 32);

		SendBuff->PutData((char*)NowBattle->m_tcChatIP, 32);
		SendBuff->PutData((char*)&NowBattle->m_wChatPort, 2);

		ReleaseSRWLockShared(&m_srwl_BattleServer_Umap);	// ------------- ��Ʋ���� �ڷᱸ�� Shared ���


		// 4. ��Ʋ�� �÷��̾� ���� �ڷᱸ���� �߰�
		stPlayer* NewPlayer = m_TLSPool_Player->Alloc();

		NewPlayer->m_ui64ClinetKey = ClientKey;
		NewPlayer->m_iJoinRoomNo = iSendRoomNo;
		NewPlayer->m_iBattleServerNo = iBattleServerNo;
		NewPlayer->m_iMatchServerNo = MatchServerNo;
		NewPlayer->m_ullRoomKey = Create_RoomKey(iBattleServerNo, iSendRoomNo);


		// ��Ʋ�� �ڷᱸ���� �߰�
		if (InsertPlayerFunc(ClientKey, NewPlayer) == false)
			g_MasterDump->Crash();


		// 5. ��Ī Lan������ ����, �� ���� Send�ϱ�
		pMatchServer->SendPacket(SessionID, SendBuff);
	}

	


	// -------------------------------
	// ���ο����� ȣ���ϴ� �Լ�
	// -------------------------------

	// ��Ʋ������ ������ ���� ��, �ش� ��Ʋ������ ���� ��� �����Ѵ�.
	// �� �ڷᱸ�� ���(2��)���� �����Ѵ�.
	//
	// Parameter : BattleServerNo
	// return : ����
	void CBattleServer_Lan::BattleLeave(int BattleServerNo)
	{
		// -------------------------- list ó��

		AcquireSRWLockExclusive(&m_srwl_Room_List);	 // ----- list �� Exclusive ��	

		// list ���� ���� �ϳ� �̻� ���� ���, list���� �� ����
		if (m_Room_List.size() > 0)
		{
			// list�� ��ȸ�ϸ鼭, �ش� ��Ʋ����No�� ���� erase �Ѵ�
			auto itor_Now = m_Room_List.begin();
			auto itor_End = m_Room_List.end();

			while (1)
			{
				if (itor_Now == itor_End)
					break;

				stRoom* NowRoom = (*itor_Now);

				// 1. ��Ʋ���� No ��
				if (NowRoom->m_iBattleServerNo == BattleServerNo)
				{
					// ���ٸ�, Erase �� Free
					itor_Now = m_Room_List.erase(itor_Now);
					m_TLSPool_Room->Free(NowRoom);

					InterlockedDecrement(&m_lTotalRoom);
				}

				// 2. �ٸ��ٸ� itor_Now ++
				else
					++itor_Now;
			}

		}

		ReleaseSRWLockExclusive(&m_srwl_Room_List);	 // ----- list �� Exclusive ���	
			   

		// -------------------------- umap ó��

		AcquireSRWLockExclusive(&m_srwl_Room_Umap);	 // ----- umap �� Exclusive ��

		// umap �ȿ� ���� �ϳ��� ���� ���, umap���� �� ����
		if (m_Room_Umap.size() > 0)
		{
			// ��� ��ȸ�ϸ�, ServerNo�� ���� ��� erase �� Free
			auto itor_Now = m_Room_Umap.begin();
			auto itor_End = m_Room_Umap.end();

			while (1)
			{
				if (itor_Now == itor_End)
					break;

				stRoom* NowRoom = itor_Now->second;

				// 1. ��Ʋ���� No ��
				if (NowRoom->m_iBattleServerNo == BattleServerNo)
				{
					// ���ٸ�, Erase�� Free
					itor_Now = m_Room_Umap.erase(itor_Now);
					m_TLSPool_Room->Free(NowRoom);

					InterlockedDecrement(&m_lTotalRoom);
				}

				// 2. �ٸ��ٸ� itor_Now ++
				else
					++itor_Now;
			}
		}

		ReleaseSRWLockExclusive(&m_srwl_Room_Umap);	 // ----- umap �� Exclusive ���
	}

	// �� umap���� ����ϴ� RoomKey�� ������ �Լ�
	//
	// Parameter : BattleServerNo, RoomNo
	// return : RoomKey(UINT64)
	UINT64 CBattleServer_Lan::Create_RoomKey(int BattleServerNo, int RoomNo)
	{
		// RoomKey��
		// ���� 4����Ʈ : BattleServerNo, ���� 4����Ʈ : RoomNo
		// �� 2���� OR �����Ѵ�.
		UINT64 ReturnKey = BattleServerNo;

		return ((ReturnKey << 32) | RoomNo);
	}

	// ��Ī���� ����
	//
	// Parameter : CMatchServer_Lan*
	// return : ����
	void CBattleServer_Lan::SetParent(CMatchServer_Lan* Match)
	{
		pMatchServer = Match;
	}


	// -------------------------------
	// ��Ŷ ó�� �Լ�
	// -------------------------------

	// ��Ʋ���� �α��� ��Ŷ
	//
	// Parameter : SessionID, Payload
	// return : ����
	void CBattleServer_Lan::Packet_Login(ULONGLONG SessionID, CProtocolBuff_Lan* Payload)
	{
		// 1. ��Ʋ���� �˻�
		stBattle* NowBattle = FindBattleServerFunc(SessionID);

		if (NowBattle == nullptr)
			g_MasterDump->Crash();


		// 2. ������ �� ��Ʋ������ ���� 1��
		Payload->GetData((char*)NowBattle->m_tcBattleIP, 32);
		Payload->GetData((char*)&NowBattle->m_wBattlePort, 2);
		Payload->GetData(NowBattle->m_cConnectToken, 32);
		
		char MasterEnterToken[32];
		Payload->GetData(MasterEnterToken, 32);

		// 3. ���� ��ū ��
		if (memcmp(pMatchServer->m_stConfig.BattleEnterToken, MasterEnterToken, 32) != 0)
		{
			// �ٸ� ���
			InterlockedIncrement(&m_lBattle_TokenError);

			g_MasterDump->Crash();

			/*
			// �ٸ� ���, �α� ���, ���� ���� ���´�.
			g_MasterLog->LogSave(false, L"MasterServer", CSystemLog::en_LogLevel::LEVEL_ERROR,
				L"Packet_Battle_Login() --> EnterToken Error. SessionID : %lld", SessionID);

			Disconnect(SessionID);

			return;
			*/
		}

		// 4. ������ �� ��Ʋ������ ���� 2��
		Payload->GetData((char*)NowBattle->m_tcChatIP, 32);
		Payload->GetData((char*)&NowBattle->m_wChatPort, 2);

		// 5. ��Ʋ���� No �ο�. �α��� ��Ŷ ó�� �÷��� ����
		int ServerNo = InterlockedIncrement(&m_lBattleServerNo_Add);
		NowBattle->m_iServerNo = ServerNo;
		NowBattle->m_bLoginCheck = true;

		// 6. ������Ŷ ������
		CProtocolBuff_Lan* SendBuff = CProtocolBuff_Lan::Alloc();
		WORD Type = en_PACKET_BAT_MAS_RES_SERVER_ON;

		SendBuff->PutData((char*)&Type, 2);
		SendBuff->PutData((char*)&ServerNo, 4);

		SendPacket(SessionID, SendBuff);
	}

	// ��ū �����
	//
	// Parameter : SessionID, Payload
	// return : ����
	void CBattleServer_Lan::Packet_TokenChange(ULONGLONG SessionID, CProtocolBuff_Lan* Payload)
	{
		// 1. ��Ʋ���� �˻�
		stBattle* NowBattle = FindBattleServerFunc(SessionID);

		if (NowBattle == nullptr)
			g_MasterDump->Crash();

		// �α��� ���� Ȯ��
		if (NowBattle->m_bLoginCheck == false)
			g_MasterDump->Crash();

		// 2. ������ �� ���ο� ��ū ��Ʋ������ ����
		UINT ReqSequence;
		Payload->GetData(NowBattle->m_cConnectToken, 32);
		Payload->GetData((char*)&ReqSequence, 4);

		// 3. ���� ��Ŷ ������
		CProtocolBuff_Lan* SendBuff = CProtocolBuff_Lan::Alloc();

		WORD Type = en_PACKET_BAT_MAS_RES_CONNECT_TOKEN;
		SendBuff->PutData((char*)&Type, 2);
		SendBuff->PutData((char*)&ReqSequence, 4);

		SendPacket(SessionID, SendBuff);
	}

	// �ű� ���� ����
	//
	// Parameter : SessionID, Payload
	// return : ����
	void CBattleServer_Lan::Packet_NewRoomCreate(ULONGLONG SessionID, CProtocolBuff_Lan* Payload)
	{
		// 1. ��Ʋ���� �˻�
		stBattle* NowBattle = FindBattleServerFunc(SessionID);

		if (NowBattle == nullptr)
			g_MasterDump->Crash();

		// �α��� ���� Ȯ��
		if (NowBattle->m_bLoginCheck == false)
			g_MasterDump->Crash();


		// 2. �� Alloc
		stRoom* NewRoom = m_TLSPool_Room->Alloc();


		// 3. ������ �� �濡 ����(�� ���� ��ū��. memcpy���̱� ���� ��ū�� �濡 �ٷ� ����)
		int BattleServerNo;
		int RoomNo;
		int MaxUser;
		UINT ReqSequence;

		Payload->GetData((char*)&BattleServerNo, 4);
		Payload->GetData((char*)&RoomNo, 4);
		Payload->GetData((char*)&MaxUser, 4);
		Payload->GetData(NewRoom->m_cEnterToken, 32);

		Payload->GetData((char*)&ReqSequence, 4);

		// ��Ʋ ������ No�� ��Ŷ���� �� ������ No�� �ٸ��� Crash
		if(NowBattle->m_iServerNo != BattleServerNo)
			g_MasterDump->Crash();


		// 4. �� ����
		NewRoom->m_ui64RoomKey = Create_RoomKey(BattleServerNo, RoomNo);
		NewRoom->m_iRoomNo = RoomNo;
		NewRoom->m_iEmptyCount = MaxUser;
		NewRoom->m_iBattleServerNo = BattleServerNo;
		NewRoom->m_ullBattleSessionID = SessionID;
		NewRoom->m_uset_JoinUser.clear();	


		// 5. �� ���� ���� �ڷᱸ���� Insert
		AcquireSRWLockExclusive(&m_srwl_Room_List);		// ----- �� list Exclusive ��

		m_Room_List.push_back(NewRoom);

		ReleaseSRWLockExclusive(&m_srwl_Room_List);		// ----- �� list Exclusive ���

		InterlockedIncrement(&m_lTotalRoom);

		// 6. ���� ��Ŷ ������
		CProtocolBuff_Lan* SendBuff = CProtocolBuff_Lan::Alloc();

		WORD Type = en_PACKET_BAT_MAS_RES_CREATED_ROOM;
		SendBuff->PutData((char*)&Type, 2);
		SendBuff->PutData((char*)&RoomNo, 4);
		SendBuff->PutData((char*)&ReqSequence, 4);

		SendPacket(SessionID, SendBuff);
	}

	// �� ����
	//
	// Parameter : SessionID, payload
	// return : ����
	void CBattleServer_Lan::Packet_RoomClose(ULONGLONG SessionID, CProtocolBuff_Lan* Payload)
	{
		// 1. ��Ʋ���� �˻�
		stBattle* NowBattle = FindBattleServerFunc(SessionID);

		if (NowBattle == nullptr)
			g_MasterDump->Crash();

		// �α��� ���� Ȯ��
		if (NowBattle->m_bLoginCheck == false)
			g_MasterDump->Crash();


		// 2. ������
		int RoomNo;
		UINT ReqSequence;
		Payload->GetData((char*)&RoomNo, 4);
		Payload->GetData((char*)&ReqSequence, 4);

		// 3. �� Key �޾ƿ���
		UINT64 RoomKey = Create_RoomKey(NowBattle->m_iServerNo, RoomNo);

		int EraseFlag = false;

		// 4. umap���� ����
		AcquireSRWLockExclusive(&m_srwl_Room_Umap);		// ----- umap �� Exclusive ��

		// �˻�
		auto FindRoom = m_Room_Umap.find(RoomKey);

		// ���� ��� ����
		if (FindRoom != m_Room_Umap.end())
		{
			stRoom* EraseRoom = FindRoom->second;

			// Erase
			m_Room_Umap.erase(FindRoom);

			ReleaseSRWLockExclusive(&m_srwl_Room_Umap);		// ----- umap �� Exclusive ���			

			InterlockedDecrement(&m_lTotalRoom);

			// 5. stRoom* Free
			m_TLSPool_Room->Free(EraseRoom);

			EraseFlag = true;
		}

		// ������ ����Ʈ �ڷᱸ�� Ȯ��.
		// �� ���̶� Ǯ ���� �Ǿ��� ���� umap�� ������ ������ ���°� ���� �ȵ�����
		// ä�� ������ ���ڱ� ����Ǿ��� ���, Wait������ ���� ������ ���ɼ��� ����.
		// Wait������ ���� list �ڷᱸ������ �����Ǳ� ������ list�� Ȯ���ؾ� �Ѵ�.
		else
		{
			ReleaseSRWLockExclusive(&m_srwl_Room_Umap);		// ----- umap �� Exclusive ���

			AcquireSRWLockExclusive(&m_srwl_Room_List);		// ----- list �� Exclusive ��

			// ����Ʈ ��ȸ
			size_t Size = m_Room_List.size();

			// ����� ���� ��쿡�� ��ȸ
			if (Size > 0)
			{
				auto itor_Now = m_Room_List.begin();
				auto itor_End = m_Room_List.end();

				while (itor_Now != itor_End)
				{
					// ���� ã�Ҵٸ� Erase
					if ((*itor_Now)->m_iRoomNo == RoomNo)
					{
						stRoom* EraseRoom = (*itor_Now);

						m_Room_List.erase(itor_Now);
						
						InterlockedDecrement(&m_lTotalRoom);

						//stRoom* Free
						m_TLSPool_Room->Free(EraseRoom);

						EraseFlag = true;

						break;
					}

					++itor_Now;
				}				
			}		

			ReleaseSRWLockExclusive(&m_srwl_Room_List);		// ----- list �� Exclusive ���
		}		

		// �� ������ ���ߴٸ� Crash
		if(EraseFlag == false)
			g_MasterDump->Crash();



		// 6. ���� ��Ŷ ������
		CProtocolBuff_Lan* SendBuff = CProtocolBuff_Lan::Alloc();

		WORD Type = en_PACKET_BAT_MAS_RES_CLOSED_ROOM;
		SendBuff->PutData((char*)&Type, 2);
		SendBuff->PutData((char*)&RoomNo, 4);
		SendBuff->PutData((char*)&ReqSequence, 4);

		SendPacket(SessionID, SendBuff);
	}
 
	// ���� ����
	//
	// Parameter : SessionID, Payload
	// return : ����
	void CBattleServer_Lan::Packet_UserExit(ULONGLONG SessionID, CProtocolBuff_Lan* Payload)
	{
		// 1. ��Ʋ���� �˻�
		stBattle* NowBattle = FindBattleServerFunc(SessionID);

		if (NowBattle == nullptr)
			g_MasterDump->Crash();

		// �α��� ���� Ȯ��
		if (NowBattle->m_bLoginCheck == false)
			g_MasterDump->Crash();


		// 2. ������
		int RoomNo;
		UINT64 ClientKey;
		UINT ReqSequence;
		Payload->GetData((char*)&RoomNo, 4);
		Payload->GetData((char*)&ClientKey, 8);
		Payload->GetData((char*)&ReqSequence, 4);


		// 3. list���� �� �˻�
		int bFlag = false;	// ����Ʈ���� �� ã�Ҵ��� üũ�ϴ� �÷���
		AcquireSRWLockExclusive(&m_srwl_Room_List);		// ----- list �� Exclusive ��

		// list�� ���� �ϳ� �̻� ���� ���, front�� ���� �ȴ�.
		if (m_Room_List.size() > 0)
		{			
			stRoom* NowRoom = *(m_Room_List.begin());

			// ������ �ִ� ���� �� ���� �´��� Ȯ��
			if (NowRoom->m_iRoomNo == RoomNo)
			{
				// 1) Set���� Ȯ�� (�뿡 �ش� ������ �����ϴ���)
				auto ret = NowRoom->m_uset_JoinUser.find(ClientKey);

				// !! ��Ʋ������ �뿡 ���� ��, ��Ī���� �� ���� ���� ��Ŷ�� �Ⱥ����� �ٷ� ���� ��� !!
				// !! �� ���� ���� ��Ŷ�� �´�. ���� ��Ŷ������ �ش� �濡�� ������ ���ܽ�Ų�� !!
				// !! ������, ���⼭ find���� �� ������ ���� ���� �ִ�. !!
				// !! �� ����, �ο��� ���� �Ƚ�Ű�� �׳� ������.
				if (ret == NowRoom->m_uset_JoinUser.end())
				{
					ReleaseSRWLockExclusive(&m_srwl_Room_List);		// ----- list �� Exclusive ���
					return;
				}

				// 2) �����Ѵٸ� Erase
				NowRoom->m_uset_JoinUser.erase(ret);

				// 3) �׿� ���� �� ����
				NowRoom->m_iEmptyCount++;

				// �����ߴµ� 1���� �۴ٸ� ���� �ȵ�
				if (NowRoom->m_iEmptyCount < 1)
					g_MasterDump->Crash();

				ReleaseSRWLockExclusive(&m_srwl_Room_List);		// ----- list �� Exclusive ���

				// �� ã�� Flag ����
				bFlag = true;
			}
		}


		// 4. ����Ʈ�� ���� �ϳ��� ���ų�, ������ ���� ���� ����Ʈ�� �ִ� ���� �ƴ� ���, umap�� ����
		if (bFlag == false)
		{
			ReleaseSRWLockExclusive(&m_srwl_Room_List);		// ----- list �� Exclusive ���

			AcquireSRWLockShared(&m_srwl_Room_Umap);		// ----- umap �� Shared ��

			// 1) �� �˻�
			UINT64 RoomKey = Create_RoomKey(NowBattle->m_iServerNo, RoomNo);
			auto ret = m_Room_Umap.find(RoomKey);

			// ���� ������ Crash.
			// ���⿡ ������ list���� ���� umap���� ���� �濡�� ������ ������. ���� �ȵ�.
			// �� �ȿ� ������ ���� �� ������ ���� ���� ���� ����!			
			if (ret == m_Room_Umap.end())
				g_MasterDump->Crash();
		
			stRoom* NowRoom = ret->second;
			NowRoom->RoomLOCK();			// ----- �� 1���� ���� ��


			// 2) Set���� Ȯ�� (�뿡 �ش� ������ �����ϴ���)
			auto findUser = NowRoom->m_uset_JoinUser.find(ClientKey);

			// !! ����Ʈ ���� ����������, �� �ȿ� �ش� ������ ���� ���� ����. !!
			if (findUser == NowRoom->m_uset_JoinUser.end())
			{
				ReleaseSRWLockShared(&m_srwl_Room_Umap);		// ----- umap �� Shared ���
				NowRoom->RoomUNLOCK();							// ----- �� 1���� ���� ���
				return;
			}

			// 3) �����Ѵٸ� Erase
			NowRoom->m_uset_JoinUser.erase(findUser);

			// 4) �׿� ���� �� ����
			NowRoom->m_iEmptyCount++;

			// �����ߴµ� 1���� �۴ٸ� ���� �ȵ�
			if (NowRoom->m_iEmptyCount < 1)
				g_MasterDump->Crash();

			ReleaseSRWLockShared(&m_srwl_Room_Umap);		// ----- umap �� Shared ���
			NowRoom->RoomUNLOCK();							// ----- �� 1���� ���� ���
		}

	}



	// -------------------------------
	// �ܺο��� ȣ�� ������ �Լ�
	// -------------------------------

	// ���� ����
	//
	// Parameter : ����
	// return : ���� �� false.
	bool CBattleServer_Lan::ServerStart()
	{
		m_lTotalRoom = 0;
		m_lBattle_TokenError = 0;

		// ��Ʋ ������ ����
		if (Start(pMatchServer->m_stConfig.BattleBindIP, pMatchServer->m_stConfig.BattlePort, pMatchServer->m_stConfig.BattleCreateWorker,
			pMatchServer->m_stConfig.BattleActiveWorker, pMatchServer->m_stConfig.BattleCreateAccept, pMatchServer->m_stConfig.BattleNodelay, 
			pMatchServer->m_stConfig.BattleMaxJoinUser) == false)
		{
			return false;
		}

		// ��Ʋ �� ���� ���� �α� ���		
		g_MasterLog->LogSave(true, L"MasterServer", CSystemLog::en_LogLevel::LEVEL_SYSTEM, L"BattleServerOpen...");

		return true;
	}

	// ���� ����
	//
	// Parameter : ����
	// return : ����
	void CBattleServer_Lan::ServerStop()
	{
		Stop();
		
		// ��Ʋ �� ���� ���� �α� ���		
		g_MasterLog->LogSave(true, L"MasterServer", CSystemLog::en_LogLevel::LEVEL_SYSTEM, L"BattleServerClose...");
	}




	// -----------------------
	// �����Լ�
	// -----------------------

	bool CBattleServer_Lan::OnConnectionRequest(TCHAR* IP, USHORT port)
	{
		return true;
	}

	void CBattleServer_Lan::OnClientJoin(ULONGLONG SessionID)
	{
		// stBattle �Ҵ�ޱ�
		stBattle* NewBattleServer = m_TLSPool_BattleServer->Alloc();

		// ���� ����
		NewBattleServer->m_ullSessionID = SessionID;

		// �ڷᱸ���� Insert
		InsertBattleServerFunc(SessionID, NewBattleServer);
	}

	void CBattleServer_Lan::OnClientLeave(ULONGLONG SessionID)
	{
		// 1. ��Ʋ���� uamp���� Erase
		stBattle* EraseBattle =  EraseBattleServerFunc(SessionID);

		// 2. �ش� ��Ʋ������ ��� ��� ����
		BattleLeave(EraseBattle->m_iServerNo);		

		// 3. stBattle* ��ȯ
		m_TLSPool_BattleServer->Free(EraseBattle);
	}

	void CBattleServer_Lan::OnRecv(ULONGLONG SessionID, CProtocolBuff_Lan* Payload)
	{
		WORD Type;
		Payload->GetData((char*)&Type, 2);

		// Ÿ�Կ� ���� �б� ó��
		try
		{
			// ���� �� ���� ��Ŷ ������� Switch ~ Case ����.
			//
			// ������ �׼��� ������ �� ���� ������, ������ �� ���� '���� ����' �׼��� 1������ ����
			// ��Ʋ�������� ���� ������ ������ �����ϱ� ������, �� ������ ���ÿ� �ű� ���� ������ ����� �� ������ ���� 2������ ����
			// ���� �ð����� ��ū ������� �� �� �ֱ� ������ 3����
			// ��Ʋ �����κ��� ���� �α��� ��û��, ������ ������ ���� �� �� �� �ȹޱ� ������ ���� �ļ���		
			switch (Type)
			{
				// ���� ����
			case en_PACKET_BAT_MAS_REQ_LEFT_USER:
				Packet_UserExit(SessionID, Payload);
				break;

				// �ű� ���� ����
			case en_PACKET_BAT_MAS_REQ_CREATED_ROOM:
				Packet_NewRoomCreate(SessionID, Payload);
				break;

				// �� ����
			case en_PACKET_BAT_MAS_REQ_CLOSED_ROOM:
				Packet_RoomClose(SessionID, Payload);
				break;

				// ��ū �����
			case en_PACKET_BAT_MAS_REQ_CONNECT_TOKEN:
				Packet_TokenChange(SessionID, Payload);
				break;

				// ��Ʋ���� �α��� 
			case en_PACKET_BAT_MAS_REQ_SERVER_ON:
				Packet_Login(SessionID, Payload);
				break;

				// Type Error
			default:
				TCHAR ErrStr[1024];
				StringCchPrintf(ErrStr, 1024, _T("BattleArea OnRecv(). TypeError. Type : %d, SessionID : %lld"),
					Type, SessionID);

				throw CException(ErrStr);
			}

		}
		catch (CException& exc)
		{
			// �α� ��� (�α� ���� : ����)
			g_MasterLog->LogSave(false, L"MasterServer", CSystemLog::en_LogLevel::LEVEL_ERROR, L"%s",
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




	// -----------------------
	// �����ڿ� �Ҹ���
	// -----------------------

	// ������
	CBattleServer_Lan::CBattleServer_Lan()
	{
		// ���� ���ʰ�
		m_lBattleServerNo_Add = 0;

		// �� �ʱ�ȭ
		InitializeSRWLock(&m_srwl_BattleServer_Umap);
		InitializeSRWLock(&m_srwl_Room_List);
		InitializeSRWLock(&m_srwl_Room_Umap);	

		// TLS �����Ҵ�
		m_TLSPool_BattleServer = new CMemoryPoolTLS<stBattle>(0, false);
		m_TLSPool_Room = new CMemoryPoolTLS<stRoom>(0, false);
		m_TLSPool_Player = new CMemoryPoolTLS<stPlayer>(0, false);

		// �ڷᱸ�� ���� �̸� ��Ƶα�
		m_BattleServer_Umap.reserve(100);
		m_Room_Umap.reserve(10000);
	}

	// �Ҹ���
	CBattleServer_Lan::~CBattleServer_Lan()
	{
		// TLS ��������
		delete m_TLSPool_BattleServer;
		delete m_TLSPool_Room;
		delete m_TLSPool_Player;
	}
}