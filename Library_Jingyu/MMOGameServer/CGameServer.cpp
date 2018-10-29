#include "pch.h"
#include "CGameServer.h"
#include "Parser\Parser_Class.h"
#include "Protocol_Set\CommonProtocol.h"
#include "Log\Log.h"
#include "CPUUsage\CPUUsage.h"
#include "PDHClass\PDHCheck.h"

#include <process.h>


extern ULONGLONG g_ullAcceptTotal_MMO;
extern LONG	  g_lAcceptTPS_MMO;
extern LONG	g_lSendPostTPS_MMO;
extern LONG	g_lRecvTPS_MMO;

extern LONG g_lAllocNodeCount;
extern LONG g_lAllocNodeCount_Lan;

extern LONG g_lAuthModeUserCount;
extern LONG g_lGameModeUserCount;

extern LONG g_lAuthFPS;
extern LONG g_lGameFPS;

LONG g_lShowAuthFPS;
LONG g_lShowGameFPS;

// GQCS���� �������� ���� �� 1 ����
extern LONG g_SemCount;


// ------------------
// CGameSession�� �Լ�
// (CGameServer�� �̳�Ŭ����)
// ------------------
namespace Library_Jingyu
{
	// ������
	CCrashDump* g_GameServerDump = CCrashDump::GetInstance();

	// �α׿�
	CSystemLog* g_GameServerLog = CSystemLog::GetInstance();


	// -----------------------
	// �����ڿ� �Ҹ���
	// -----------------------

	// ������
	CGameServer::CGameSession::CGameSession()
		:CMMOServer::cSession()
	{
		// �Ұ� ����
		m_Int64AccountNo = 0x0fffffffffffffff;
	}

	// �Ҹ���
	CGameServer::CGameSession::~CGameSession()
	{
		// �Ұ� ����
	}

	// -----------------
	// �����Լ�
	// -----------------
	
	// --------------- AUTH ���� �Լ�

	// ������ Auth ���� �����
	//
	// Parameter : ����
	// return : ����
	void CGameServer::CGameSession::OnAuth_ClientJoin()
	{
		// �Ұ� ����
	}

	// ������ Auth ��忡�� ����
	//
	// Parameter : Game���� ����Ȱ����� �˷��ִ� Flag. ����Ʈ false.
	// return : ����
	void CGameServer::CGameSession::OnAuth_ClientLeave(bool bGame)
	{
		
		// ������ ���, �α��� ���� ��Ŷ ����
		if (bGame == true)
		{
			// Alloc
			CProtocolBuff_Net* SendBuff = CProtocolBuff_Net::Alloc();

			// Ÿ��
			WORD Type = en_PACKET_CS_GAME_RES_LOGIN;
			SendBuff->PutData((char*)&Type, 2);

			// Status
			BYTE Status = 1;
			SendBuff->PutData((char*)&Status, 1);

			// AccountNo
			SendBuff->PutData((char*)&m_Int64AccountNo, 8);

			// SendPacket
			SendPacket(SendBuff);
		}
		
	}

	// Auth ����� �������� packet�� ��
	//
	// Parameter : ��Ŷ (CProtocolBuff_Net*)
	// return : ����
	void CGameServer::CGameSession::OnAuth_Packet(CProtocolBuff_Net* Packet)
	{
		// ��Ŷ ó��
		try
		{
			// 1. Ÿ�� ����
			WORD Type;
			Packet->GetData((char*)&Type, 2);

			// 2. Ÿ�Կ� ���� �б� ó��
			switch (Type)
			{
				// �α��� ��û
			case en_PACKET_CS_GAME_REQ_LOGIN:
				Auth_LoginPacket(Packet);
				break;

				// ��Ʈ��Ʈ
			case en_PACKET_CS_GAME_REQ_HEARTBEAT:
				break;

				// �� �ܿ��� ��������.
			default:				
				throw CException(_T("OnAuth_Packet() --> Type Error!!"));
				break;
			}
		}
		catch (CException& exc)
		{
			// �α� ��� (�α� ���� : ����)
			g_GameServerLog->LogSave(L"GameServer", CSystemLog::en_LogLevel::LEVEL_ERROR, L"%s",
				(TCHAR*)exc.GetExceptionText());	

			// ����
			g_GameServerDump->Crash();
		}

	}



	// --------------- GAME ���� �Լ�

	// ������ Game���� �����
	//
	// Parameter : ����
	// return : ����
	void CGameServer::CGameSession::OnGame_ClientJoin()
	{
		// �Ұ� ����
	}

	// ������ Game��忡�� ����
	//
	// Parameter : ����
	// return : ����
	void CGameServer::CGameSession::OnGame_ClientLeave()
	{
		// �Ұ� ����
	}

	// Game ����� �������� packet�� ��
	//
	// Parameter : ��Ŷ (CProtocolBuff_Net*)
	// return : ����
	void CGameServer::CGameSession::OnGame_Packet(CProtocolBuff_Net* Packet)
	{
		// Ÿ�Կ� ���� ��Ŷ ó��
		try
		{
			// 1. Ÿ�� ����
			WORD Type;
			Packet->GetData((char*)&Type, 2);

			// 2. Ÿ�Կ� ���� �б⹮ ó��
			switch (Type)
			{
				// �׽�Ʈ�� ���� ��û
			case en_PACKET_CS_GAME_REQ_ECHO:
				Game_EchoTest(Packet);
				break;

				// ��Ʈ��Ʈ
			case en_PACKET_CS_GAME_REQ_HEARTBEAT:
				break;

				// �� �ܿ��� ���� ����
			default:
				throw CException(_T("OnGame_Packet() --> Type Error!!"));
				break;
			}

		}
		catch (CException& exc)
		{
			// �α� ��� (�α� ���� : ����)
			g_GameServerLog->LogSave(L"GameServer", CSystemLog::en_LogLevel::LEVEL_ERROR, L"%s",
				(TCHAR*)exc.GetExceptionText());

			// ����
			g_GameServerDump->Crash();
		}

	}



	// --------------- Release ���� �Լ�
	
	// Release�� ����.
	//
	// Parameter : ����
	// return : ����
	void CGameServer::CGameSession::OnGame_ClientRelease()
	{
		// �ڷᱸ������ ����
		//m_pParent->AccountNO_Erase(m_Int64AccountNo);

		// AccountNo �ʱ�ȭ
		m_Int64AccountNo = 0x0fffffffffffffff;
	}

	


	// -----------------
	// ��Ŷ ó�� �Լ�
	// -----------------

	// �α��� ��û 
	//
	// Parameter : CProtocolBuff_Net*
	// return : ����
	void CGameServer::CGameSession::Auth_LoginPacket(CProtocolBuff_Net* Packet)
	{
		// ������	----------------------
		INT64 AccountNo;
		char SessionKey[64];
		int Version;

		Packet->GetData((char*)&AccountNo, 8);
		Packet->GetData(SessionKey, 64);
		Packet->GetData((char*)&Version, 4);

		
		// AccountNo �ڷᱸ���� �߰�.	----------------------
		// �̹� ������(false ����) �ߺ� �α������� ó��
		/*
		if (m_pParent->AccountNO_Insert(AccountNo) == false)
		{
			// �α��� ���� ��Ŷ ������ ����.
			CProtocolBuff_Net* SendBuff = CProtocolBuff_Net::Alloc();

			// Ÿ��
			WORD Type = en_PACKET_CS_GAME_RES_LOGIN;
			SendBuff->PutData((char*)&Type, 2);

			// Status
			BYTE Status = 0;		// ����
			SendBuff->PutData((char*)&Status, 1);

			// AccountNo
			SendBuff->PutData((char*)&AccountNo, 8);

			InterlockedIncrement(&g_DuplicateCount);

			// SendPacket
			SendPacket(SendBuff, TRUE);

			return;
		}
		*/


		// 3. m_Int64AccountNo�� ���� 0x0fffffffffffffff�� �ƴϸ�, ���� �̻���.
		// �ڵ� �߸��̶�� �ۿ� �� ��.
		if(m_Int64AccountNo != 0x0fffffffffffffff)
		{
			throw CException(_T("Auth_LoginPacket() --> AccountNo Error!!"));
		}

		// 4. �� ����
		m_Int64AccountNo = AccountNo;

		/*
		// �α��� ���� ��Ŷ ������

		// Alloc
		CProtocolBuff_Net* SendBuff = CProtocolBuff_Net::Alloc();

		// Ÿ��
		WORD Type = en_PACKET_CS_GAME_RES_LOGIN;
		SendBuff->PutData((char*)&Type, 2);

		// Status
		BYTE Status = 1;
		SendBuff->PutData((char*)&Status, 1);

		// AccountNo
		SendBuff->PutData((char*)&AccountNo, 8);

		// SendPacket
		SendPacket(SendBuff);
		*/

		// 5. �����̸�, ��带 AUTH_TO_GAME���� ���� ��û.	
		SetMode_GAME();
	}
	   
	// �׽�Ʈ�� ���� ��û
	//
	// Parameter : CProtocolBuff_Net*
	// return : ����
	void CGameServer::CGameSession::Game_EchoTest(CProtocolBuff_Net* Packet)
	{
		// 1. ������
		INT64 AccountNo;
		LONGLONG SendTick;

		Packet->GetData((char*)&AccountNo, 8);
		Packet->GetData((char*)&SendTick, 8);

		// 2. AccountNo Ȯ���ϱ�
		// �ٸ��� �α� ����� ���� ����.
		if (AccountNo != m_Int64AccountNo)
		{
			// �α� ��� (�α� ���� : ����)
			g_GameServerLog->LogSave(L"GameServer", CSystemLog::en_LogLevel::LEVEL_ERROR, L"%s : AccountNo : %lld",
				L"Game_EchoTest --> AccountNo Error", AccountNo);

			Disconnect();	
			return;
		}

		// 2. �״�� �ٽ� ������
		WORD Type = en_PACKET_CS_GAME_RES_ECHO;

		CProtocolBuff_Net* SendBuff = CProtocolBuff_Net::Alloc();

		SendBuff->PutData((char*)&Type, 2);
		SendBuff->PutData((char*)&AccountNo, 8);
		SendBuff->PutData((char*)&SendTick, 8);

		SendPacket(SendBuff);
	}


}

// ---------------
// CGameServer
// CMMOServer�� ��ӹ޴� ���� ����  (��Ʋ����)
// ---------------
namespace Library_Jingyu
{

	// Net ����ȭ ���� 1���� ũ�� (Byte)
	LONG g_lNET_BUFF_SIZE = 200;

	// -----------------
	// �����ڿ� �Ҹ���
	// -----------------
	CGameServer::CGameServer()
		:CMMOServer()
	{
		// ------------------- Config���� ����		
		if (SetFile(&m_stConfig) == false)
			g_GameServerDump->Crash();

		// ------------------- �α� ������ ���� ����
		g_GameServerLog->SetDirectory(L"GameServer");
		g_GameServerLog->SetLogLeve((CSystemLog::en_LogLevel)m_stConfig.LogLevel);	

		// ����͸� ������ ����ϱ� ���� LanClient �����Ҵ�
		m_Monitor_LanClient = new CGame_MinitorClient;
		m_Monitor_LanClient->ParentSet(this);

		// U_set�� reserve ����.
		// �̸� ���� ��Ƶд�.
		m_setAccount.reserve(m_stConfig.MaxJoinUser);

		// U_set�� SRW�� �ʱ�ȭ
		InitializeSRWLock(&m_setSrwl);
	}

	CGameServer::~CGameServer()
	{
		// ����͸� ������ ����ϴ� LanClient ��������
		delete m_Monitor_LanClient;
	}

	// GameServerStart
	// ���������� CMMOServer�� Start, ���� ���ñ��� �Ѵ�.
	//
	// Parameter : ����
	// return : ���� �� false
	bool CGameServer::GameServerStart()
	{
		// 1. ���� ����
		m_cGameSession = new CGameSession[m_stConfig.MaxJoinUser];			

		int i = 0;
		while (i < m_stConfig.MaxJoinUser)
		{
			// GameServer�� ������ ����
			m_cGameSession[i].m_pParent  = this;

			// ������ ���� ����
			SetSession(&m_cGameSession[i], m_stConfig.MaxJoinUser);			
			++i;
		}		
		
		// 2. ���� ���� ����
		if (Start(m_stConfig.BindIP, m_stConfig.Port, m_stConfig.CreateWorker, m_stConfig.ActiveWorker, m_stConfig.CreateAccept, 
			m_stConfig.Nodelay, m_stConfig.MaxJoinUser, m_stConfig.HeadCode, m_stConfig.XORCode1, m_stConfig.XORCode2) == false)
			return false;	

		// 3. ����͸� ������ ����Ǵ�, �� Ŭ���̾�Ʈ ����
		if (m_Monitor_LanClient->ClientStart(m_stConfig.MonitorServerIP, m_stConfig.MonitorServerPort, m_stConfig.MonitorClientCreateWorker,
			m_stConfig.MonitorClientActiveWorker, m_stConfig.MonitorClientNodelay) == false)
			return false;

		// ���� ���� �α� ���		
		g_GameServerLog->LogSave(L"GameServer", CSystemLog::en_LogLevel::LEVEL_SYSTEM, L"ServerOpen...");

		return true;
	}

	// GameServerStop
	// ���������� Stop ����
	//
	// Parameter : ����
	// return : ����
	void CGameServer::GameServerStop()
	{
		// 1. ����͸� Ŭ�� ����
		if (m_Monitor_LanClient->GetClinetState() == true)
			m_Monitor_LanClient->ClientStop();

		// 2. ���� ����
		if (GetServerState() == true)
			Stop();

		// 2. ���� ����
		delete[] m_cGameSession;
	}

	// ��¿� �Լ�
	//
	// Parameter : ����
	// return : ����
	void CGameServer::ShowPrintf()
	{
		// ȭ�� ����� �� ����
		/*
		Monitor Connect :					- ����͸� ������ ���� ����. 1�̸� �����.
		Total SessionNum : 					- MMOServer �� ���Ǽ�
		AuthMode SessionNum :				- Auth ����� ���� ��
		GameMode SessionNum (Auth + Game) :	- Game ����� ���� �� (Auth + Game��� ���� ��)

		PacketPool_Net : 		- �ܺο��� ��� ���� Net ����ȭ ������ ��	
		Accept Socket Queue :	- Accept Socket Queue ���� �ϰ� ��

		Accept Total :		- Accept ��ü ī��Ʈ (accept ���Ͻ� +1)
		Accept TPS :		- �ʴ� Accept ó�� Ƚ��
		Auth FPS :			- �ʴ� Auth ������ ó�� Ƚ��
		Game FPS :			- �ʴ� Game ������ ó�� Ƚ��

		Send TPS			- �ʴ� Send�Ϸ� Ƚ��. (�Ϸ��������� ����)
		Recv TPS			- �ʴ� Recv�Ϸ� Ƚ��. (��Ŷ 1���� �ϼ��Ǿ��� �� ����. RecvProc���� ��Ŷ�� �ֱ� ���� 1�� ����)		

		Net_BuffChunkAlloc_Count : - Net ����ȭ ���� �� Alloc�� ûũ �� (�ۿ��� ������� ûũ ��)
		ASQPool_ChunkAlloc_Count : - Accept Socket Queue�� ���� �ϰ� �� Alloc�� ûũ �� (�ۿ��� ������� ûũ ��)

		DuplicateCount :	- �ߺ� �α������� ���� ������ ���⸦ �� �� 1�� ����

		----------------------------------------------------

		*/		

		LONG AccpetTPS = InterlockedExchange(&g_lAcceptTPS_MMO, 0);
		LONG SendTPS = InterlockedExchange(&g_lSendPostTPS_MMO, 0);
		LONG RecvTPS = InterlockedExchange(&g_lRecvTPS_MMO, 0);
		g_lShowAuthFPS = InterlockedExchange(&g_lAuthFPS, 0);
		g_lShowGameFPS = InterlockedExchange(&g_lGameFPS, 0);

		printf("========================================================\n"
				"Monitor Connect : %d\n"
				"Total SessionNum : %lld\n"
				"AuthMode SessionNum : %d\n"
				"GameMode SessionNum : %d (Auth + Game : %d)\n\n"

				"PacketPool_Net : %d\n"
				"Accept Socket Queue : %d\n\n"

				"Accept Total : %lld\n"
				"Accept TPS : %d\n"
				"Send TPS : %d\n"
				"Recv TPS : %d\n\n"

				"Auth FPS : %d\n"
				"Game FPS : %d\n\n"				

				"Net_BuffChunkAlloc_Count : %d (Out : %d)\n"
				"ASQPool_ChunkAlloc_Count : %d (Out : %d)\n\n"
				
				"SemCount : %d\n\n"

			"========================================================\n\n",

			// ----------- ���� ������
			m_Monitor_LanClient->GetClinetState(),
			GetClientCount(), g_lAuthModeUserCount, g_lGameModeUserCount, g_lAuthModeUserCount+g_lGameModeUserCount,
			g_lAllocNodeCount, GetASQ_Count(),
			g_ullAcceptTotal_MMO, AccpetTPS, SendTPS, RecvTPS,
			g_lShowAuthFPS, g_lShowGameFPS,
			CProtocolBuff_Net::GetChunkCount(), CProtocolBuff_Net::GetOutChunkCount(),
			GetChunkCount(), GetOutChunkCount(),
			g_SemCount

			);
	}






	// -----------------------
	// ���ο����� ����ϴ� �Լ�
	// -----------------------

	// ���Ͽ��� Config ���� �о����
	// 
	// Parameter : config ����ü
	// return : ���������� ���� �� true
	//		  : �� �ܿ��� false
	bool CGameServer::SetFile(stConfigFile* pConfig)
	{
		Parser Parser;

		// ���� �ε�
		try
		{
			Parser.LoadFile(_T("MMOGameServer_Config.ini"));
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
		// GameServer config �о����
		////////////////////////////////////////////////////////

		// ���� ���� -------------------------
		if (Parser.AreaCheck(_T("MMOGAMESERVER")) == false)
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
		// ����͸� LanClient config �о����
		////////////////////////////////////////////////////////

		// ���� ���� -------------------------
		if (Parser.AreaCheck(_T("MONITORLANCLIENT")) == false)
			return false;

		// IP
		if (Parser.GetValue_String(_T("MonitorServerIP"), pConfig->MonitorServerIP) == false)
			return false;

		// Port
		if (Parser.GetValue_Int(_T("MonitorServerPort"), &pConfig->MonitorServerPort) == false)
			return false;

		// ���� ��Ŀ ��
		if (Parser.GetValue_Int(_T("MonitorClientCreateWorker"), &pConfig->MonitorClientCreateWorker) == false)
			return false;

		// Ȱ��ȭ ��Ŀ ��
		if (Parser.GetValue_Int(_T("MonitorClientActiveWorker"), &pConfig->MonitorClientActiveWorker) == false)
			return false;


		// Nodelay
		if (Parser.GetValue_Int(_T("MonitorClientNodelay"), &pConfig->MonitorClientNodelay) == false)
			return false;



		return true;
	}

	// AccountNo �ڷᱸ�� "�˻���"
	//
	// Parameter : AccountNO(INT64)
	// return : ���� �� true, ���� �� false
	bool CGameServer::AccountNO_Find(INT64 AccountNo)
	{		
		AcquireSRWLockShared(&m_setSrwl);		// Shared �� ------------------

		// 1. �˻�
		auto Find = m_setAccount.find(AccountNo);

		// 2. ����, ���� AccountNo��� false ����
		if (Find == m_setAccount.end())
		{
			ReleaseSRWLockShared(&m_setSrwl);		// Shared ���  ------------------ 
			return false;
		}

		ReleaseSRWLockShared(&m_setSrwl);		// Shared ���  ------------------ 

		// 3. �ִٸ� true ����
		return true;
	}

	// AccountNo �ڷᱸ�� "�߰�"
	//
	// Parameter : AccountNO(INT64)
	// return : �̹� �ڷᱸ���� ������ �� false, 
	//			�����ϸ� ���������� �߰��� �� true
	bool CGameServer::AccountNO_Insert(INT64 AccountNo)
	{
		AcquireSRWLockExclusive(&m_setSrwl);		// Exclusive �� ------------------

		// 1. �߰�
		auto ret = m_setAccount.insert(AccountNo);

		// 2. ����, �̹� �ִٸ� �ߺ��� ����. return false.
		if (ret.second == false)
		{
			ReleaseSRWLockExclusive(&m_setSrwl);		// Exclusive ���  ------------------ 
			return false;
		}

		// 3. ���ٸ�, �� �߰��Ȱ�. true ����
		ReleaseSRWLockExclusive(&m_setSrwl);		// Exclusive ���  ------------------ 
		return true;
	}

	// AccountNo �ڷᱸ�� "����"
	//
	// Parameter : AccountNO(INT64)
	// return : ����.
	//			�ڷᱸ���� �������� ���� �� Crash
	void CGameServer::AccountNO_Erase(INT64 AccountNo)
	{
		AcquireSRWLockExclusive(&m_setSrwl);		// Exclusive �� ------------------

		// 1. �˻�
		auto Find = m_setAccount.find(AccountNo);

		// 2. ����, ���ٸ� ���̾ȵ�. 
		if (Find == m_setAccount.end())
		{
			ReleaseSRWLockExclusive(&m_setSrwl);	//  Exclusive \��� ------------------

			// ���� �ȵ�. ũ����
			g_GameServerDump->Crash();
		}

		// 3. �ִٸ� Set���� ����	
		m_setAccount.erase(Find);

		ReleaseSRWLockExclusive(&m_setSrwl);	// ---------------- Unlock
	}




	// -----------------------
	// �����Լ�
	// -----------------------

	// AuthThread���� 1Loop���� 1ȸ ȣ��.
	// 1�������� ���������� ó���ؾ� �ϴ� ���� �Ѵ�.
	// 
	// parameter : ����
	// return : ����
	void CGameServer::OnAuth_Update()
	{

	}

	// GameThread���� 1Loop���� 1ȸ ȣ��.
	// 1�������� ���������� ó���ؾ� �ϴ� ���� �Ѵ�.
	// 
	// parameter : ����
	// return : ����
	void CGameServer::OnGame_Update()
	{}

	// ���ο� ���� ���� ��, Auth���� ȣ��ȴ�.
	//
	// parameter : ������ ������ IP, Port
	// return false : Ŭ���̾�Ʈ ���� �ź�
	// return true : ���� ���
	bool CGameServer::OnConnectionRequest(TCHAR* IP, USHORT port)
	{
		return true;
	}

	// ��Ŀ �����尡 ��� �� ȣ��Ǵ� �Լ�.
	// GQCS �ٷ� �ϴܿ��� ȣ��
	// 
	// parameter : ����
	// return : ����
	void CGameServer::OnWorkerThreadBegin()
	{}

	// ��Ŀ �����尡 ���� �� ȣ��Ǵ� �Լ�
	// GQCS �ٷ� ������ ȣ��
	// 
	// parameter : ����
	// return : ����
	void CGameServer::OnWorkerThreadEnd()
	{}

	// ���� �߻� �� ȣ��Ǵ� �Լ�.
	//
	// parameter : ���� �ڵ�(���� ������ �����ڵ�� WinGetLastError() �Լ��� ��� ����. ���� ��� 0�� ���ϵ�)
	//			 : ���� �ڵ忡 ���� ��Ʈ��
	// return : ����
	void CGameServer::OnError(int error, const TCHAR* errorStr)
	{
		// �α� ��� (�α� ���� : ����)
		g_GameServerLog->LogSave(L"GameServer", CSystemLog::en_LogLevel::LEVEL_ERROR, L"%s (ErrorCode : %d)",
			errorStr, error);
	}



}


// ---------------
// CGame_MinitorClient
// CLanClienet�� ��ӹ޴� ����͸� Ŭ��
// ---------------
namespace Library_Jingyu
{
	// -----------------------
	// �����ڿ� �Ҹ���
	// -----------------------
	CGame_MinitorClient::CGame_MinitorClient()
		:CLanClient()
	{
		// ����͸� ���� �������� �����带 �����ų �̺�Ʈ ����
		//
		// �ڵ� ���� Event 
		// ���� ���� �� non-signalled ����
		// �̸� ���� Event	
		m_hMonitorThreadExitEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

	}

	CGame_MinitorClient::~CGame_MinitorClient()
	{
		// ���� ������ �Ǿ�������, ���� ����
		if (GetClinetState() == true)
			ClientStop();

		// �̺�Ʈ ����
		CloseHandle(m_hMonitorThreadExitEvent);
	}



	// -----------------------
	// �ܺο��� ��� ������ �Լ�
	// -----------------------

	// ���� �Լ�
	// ����������, ��ӹ��� CLanClient�� Startȣ��.
	//
	// Parameter : ������ ������ IP, ��Ʈ, ��Ŀ������ ��, Ȱ��ȭ��ų ��Ŀ������ ��, TCP_NODELAY ��� ����(true�� ���)
	// return : ���� �� true , ���� �� falsel 
	bool CGame_MinitorClient::ClientStart(TCHAR* ConnectIP, int Port, int CreateWorker, int ActiveWorker, int Nodelay)
	{
		// ����͸� ������ ����
		if (Start(ConnectIP, Port, CreateWorker, ActiveWorker, Nodelay) == false)
			return false;

		// ����͸� ������ ���� ������ ������ ����
		m_hMonitorThread = (HANDLE)_beginthreadex(NULL, 0, MonitorThread, this, 0, NULL);

		return true;
	}

	// ���� �Լ�
	// ����������, ��ӹ��� CLanClient�� Stopȣ��.
	// �߰���, ���ҽ� ���� ��
	//
	// Parameter : ����
	// return : ����
	void CGame_MinitorClient::ClientStop()
	{
		// 1. ����͸� ���� �������� ������ ����
		SetEvent(m_hMonitorThreadExitEvent);

		// ���� ���
		if (WaitForSingleObject(m_hMonitorThread, INFINITE) == WAIT_FAILED)
		{
			DWORD Error = GetLastError();
			printf("MonitorThread Exit Error!!! (%d) \n", Error);
		}

		// 2. ������ �ڵ� ��ȯ
		CloseHandle(m_hMonitorThread);

		// 3. ����͸� ������ ���� ����
		Stop();
	}
	
	// ���Ӽ����� this�� �Է¹޴� �Լ�
	// 
	// Parameter : ���� ������ this
	// return : ����
	void CGame_MinitorClient::ParentSet(CGameServer* ChatThis)
	{
		m_GameServer_this = ChatThis;
	}




	// -----------------------
	// ���ο����� ����ϴ� ��� �Լ�
	// -----------------------

	// ���� �ð����� ����͸� ������ ������ �����ϴ� ������
	UINT	WINAPI CGame_MinitorClient::MonitorThread(LPVOID lParam)
	{
		// this �޾Ƶα�
		CGame_MinitorClient* g_This = (CGame_MinitorClient*)lParam;

		// ���� ��ȣ �̺�Ʈ �޾Ƶα�
		HANDLE* hEvent = &g_This->m_hMonitorThreadExitEvent;

		// CPU ����� üũ Ŭ���� (ä�ü��� ����Ʈ����)
		CCpuUsage_Process CProcessCPU;

		// CPU ����� üũ Ŭ���� (�ϵ����)
		CCpuUsage_Processor CProcessorCPU;

		// PDH�� Ŭ����
		CPDH	CPdh;

		while (1)
		{
			// 1�ʿ� �ѹ� ����� ������ ������.
			DWORD Check = WaitForSingleObject(*hEvent, 1000);

			// �̻��� ��ȣ���
			if (Check == WAIT_FAILED)
			{
				DWORD Error = GetLastError();
				printf("MoniterThread Exit Error!!! (%d) \n", Error);
				break;
			}

			// ����, ���� ��ȣ�� �Դٸ� ������ ����.
			else if (Check == WAIT_OBJECT_0)
				break;


			// �װ� �ƴ϶��, ���� �Ѵ�.

			// ���μ���, ���μ��� CPU �����, PDH ���� ����
			CProcessorCPU.UpdateCpuTime();
			CProcessCPU.UpdateCpuTime();
			CPdh.SetInfo();

			// ----------------------------------
			// �ϵ���� ���� ������ (���μ���)
			// ----------------------------------
			int TimeStamp = (int)(time(NULL));

			// 1. �ϵ���� CPU ���� ��ü
			g_This->InfoSend(dfMONITOR_DATA_TYPE_SERVER_CPU_TOTAL, (int)CProcessorCPU.ProcessorTotal(), TimeStamp);

			// 2. �ϵ���� ��밡�� �޸� (MByte)
			g_This->InfoSend(dfMONITOR_DATA_TYPE_SERVER_AVAILABLE_MEMORY, (int)CPdh.Get_AVA_Mem(), TimeStamp);

			// 3. �ϵ���� �̴��� ���� ����Ʈ (KByte)
			int iData = (int)(CPdh.Get_Net_Recv() / 1024);
			g_This->InfoSend(dfMONITOR_DATA_TYPE_SERVER_NETWORK_RECV, iData, TimeStamp);

			// 4. �ϵ���� �̴��� �۽� ����Ʈ (KByte)
			iData = (int)(CPdh.Get_Net_Send() / 1024);
			g_This->InfoSend(dfMONITOR_DATA_TYPE_SERVER_NETWORK_SEND, iData, TimeStamp);

			// 5. �ϵ���� �������� �޸� ��뷮 (MByte)
			iData = (int)(CPdh.Get_NonPaged_Mem() / 1024 / 1024);
			g_This->InfoSend(dfMONITOR_DATA_TYPE_SERVER_NONPAGED_MEMORY, iData, TimeStamp);



			// ----------------------------------
			// ���Ӽ��� ���� ������
			// ----------------------------------

			// ���Ӽ����� On�� ���, ���Ӽ��� ���� ������.
			if (g_This->m_GameServer_this->GetServerState() == true)
			{			
				// 1. ���Ӽ��� ON		
				g_This->InfoSend(dfMONITOR_DATA_TYPE_BATTLE_SERVER_ON, TRUE, TimeStamp);

				// 2. ���Ӽ��� CPU ���� (Ŀ�� + ����)
				g_This->InfoSend(dfMONITOR_DATA_TYPE_BATTLE_CPU, (int)CProcessCPU.ProcessTotal(), TimeStamp);

				// 3. ���Ӽ��� �޸� ���� Ŀ�� ��뷮 (Private) MByte
				int Data = (int)(CPdh.Get_UserCommit() / 1024 / 1024);
				g_This->InfoSend(dfMONITOR_DATA_TYPE_BATTLE_MEMORY_COMMIT, Data, TimeStamp);

				// 4. ���Ӽ��� ��ŶǮ ��뷮
				g_This->InfoSend(dfMONITOR_DATA_TYPE_BATTLE_PACKET_POOL, g_lAllocNodeCount + g_lAllocNodeCount_Lan, TimeStamp);

				// 5. Auth ������ �ʴ� ���� ��
				g_This->InfoSend(dfMONITOR_DATA_TYPE_BATTLE_AUTH_FPS, g_lShowAuthFPS, TimeStamp);

				// 6. Game ������ �ʴ� ���� ��
				g_This->InfoSend(dfMONITOR_DATA_TYPE_BATTLE_GAME_FPS, g_lShowGameFPS, TimeStamp);

				// 7. ���Ӽ��� ���� ������ü
				g_This->InfoSend(dfMONITOR_DATA_TYPE_BATTLE_SESSION_ALL, (int)g_This->m_GameServer_this->GetClientCount(), TimeStamp);

				// 8. Auth ������ ��� �ο�
				g_This->InfoSend(dfMONITOR_DATA_TYPE_BATTLE_SESSION_AUTH, g_lAuthModeUserCount, TimeStamp);

				// 9. Game ������ ��� �ο�
				g_This->InfoSend(dfMONITOR_DATA_TYPE_BATTLE_SESSION_GAME, g_lGameModeUserCount, TimeStamp);

				// 10. ���� ��
				
				// 11. �÷��� �� ��
			}

		}

		return 0;
	}

	// ����͸� ������ ������ ����
	//
	// Parameter : DataType(BYTE), DataValue(int), TimeStamp(int)
	// return : ����
	void CGame_MinitorClient::InfoSend(BYTE DataType, int DataValue, int TimeStamp)
	{
		WORD Type = en_PACKET_SS_MONITOR_DATA_UPDATE;

		CProtocolBuff_Lan* SendBuff = CProtocolBuff_Lan::Alloc();

		SendBuff->PutData((char*)&Type, 2);
		SendBuff->PutData((char*)&DataType, 1);
		SendBuff->PutData((char*)&DataValue, 4);
		SendBuff->PutData((char*)&TimeStamp, 4);

		SendPacket(m_ullSessionID, SendBuff);
	}


	// -----------------------
	// ���� �����Լ�
	// -----------------------

	// ��ǥ ������ ���� ���� ��, ȣ��Ǵ� �Լ� (ConnectFunc���� ���� ���� �� ȣ��)
	//
	// parameter : ����Ű
	// return : ����
	void CGame_MinitorClient::OnConnect(ULONGLONG SessionID)
	{
		m_ullSessionID = SessionID;

		// ����͸� ����(Lan)�� �α��� ��Ŷ ����
		CProtocolBuff_Lan* SendBuff = CProtocolBuff_Lan::Alloc();

		WORD Type = en_PACKET_SS_MONITOR_LOGIN;
		int ServerNo = dfSERVER_NO;

		SendBuff->PutData((char*)&Type, 2);
		SendBuff->PutData((char*)&ServerNo, 4);

		SendPacket(SessionID, SendBuff);
	}

	// ��ǥ ������ ���� ���� �� ȣ��Ǵ� �Լ� (InDIsconnect �ȿ��� ȣ��)
	//
	// parameter : ����Ű
	// return : ����
	void CGame_MinitorClient::OnDisconnect(ULONGLONG SessionID)
	{}

	// ��Ŷ ���� �Ϸ� �� ȣ��Ǵ� �Լ�.
	//
	// parameter : ���� ����Ű, CProtocolBuff_Lan*
	// return : ����
	void CGame_MinitorClient::OnRecv(ULONGLONG SessionID, CProtocolBuff_Lan* Payload)
	{}

	// ��Ŷ �۽� �Ϸ� �� ȣ��Ǵ� �Լ�
	//
	// parameter : ���� ����Ű, Send �� ������
	// return : ����
	void CGame_MinitorClient::OnSend(ULONGLONG SessionID, DWORD SendSize)
	{}

	// ��Ŀ �����尡 ��� �� ȣ��Ǵ� �Լ�.
	// GQCS �ٷ� �ϴܿ��� ȣ��
	// 
	// parameter : ����
	// return : ����
	void CGame_MinitorClient::OnWorkerThreadBegin()
	{}

	// ��Ŀ �����尡 ���� �� ȣ��Ǵ� �Լ�
	// GQCS �ٷ� ������ ȣ��
	// 
	// parameter : ����
	// return : ����
	void CGame_MinitorClient::OnWorkerThreadEnd()
	{}

	// ���� �߻� �� ȣ��Ǵ� �Լ�.
	//
	// parameter : ���� �ڵ�(���� ������ �����ڵ�� WinGetLastError() �Լ��� ��� ����. ���� ��� 0�� ���ϵ�)
	//			 : ���� �ڵ忡 ���� ��Ʈ��
	// return : ����
	void CGame_MinitorClient::OnError(int error, const TCHAR* errorStr)
	{}



}