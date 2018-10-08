#include "pch.h"
#include "CGameServer.h"
#include "Parser\Parser_Class.h"
#include "Protocol_Set\CommonProtocol.h"
#include "Log\Log.h"
#include "CPUUsage\CPUUsage.h"


extern ULONGLONG g_ullAcceptTotal_MMO;
extern LONG	  g_lAcceptTPS_MMO;
extern LONG	g_lSendPostTPS_MMO;
extern LONG	g_lRecvTPS_MMO;

extern LONG g_lAllocNodeCount;

extern LONG g_lAuthModeUserCount;
extern LONG g_lGameModeUserCount;

extern LONG g_lAuthFPS;
extern LONG g_lGameFPS;



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
		// �Ұ� ���� 
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
		// 1. ������
		INT64 AccountNo;
		char SessionKey[64];
		int Version;

		Packet->GetData((char*)&AccountNo, 8);
		Packet->GetData(SessionKey, 64);
		Packet->GetData((char*)&Version, 4);

		// 2. �����۾� -----------
		// ������ ����

		// 3. �� ����
		m_Int64AccountNo = AccountNo;

		// 4. �α��� ���� ��Ŷ ������
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

		// 5. AUTH ���� GAME���� ��� ����
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
		// �α��ο�û�� ���� ���� �������, ������ ���� ����
		if (AccountNo != m_Int64AccountNo)
		{
			throw CException(_T("Game_EchoTest() --> AccountNo Error!!"));
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
// CMMOServer�� ��ӹ޴� ���� ����
// ---------------
namespace Library_Jingyu
{
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
	}

	CGameServer::~CGameServer()
	{
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
			SetSession(&m_cGameSession[i], m_stConfig.MaxJoinUser, m_stConfig.HeadCode, m_stConfig.XORCode1, m_stConfig.XORCode2);			
			++i;
		}
		
		
		// 2. ���� ����
		if (Start(m_stConfig.BindIP, m_stConfig.Port, m_stConfig.CreateWorker, m_stConfig.ActiveWorker, m_stConfig.CreateAccept, 
			m_stConfig.Nodelay, m_stConfig.MaxJoinUser, m_stConfig.HeadCode, m_stConfig.XORCode1, m_stConfig.XORCode2) == false)
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
		// 1. ���� ����
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
		// �ش� ���μ����� ��뷮 üũ�� Ŭ����
		static CCpuUsage_Process ProcessUsage;

		// ȭ�� ����� �� ����
		/*
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

		----------------------------------------------------
		CPU usage [MMOServer:%.1f%% U:%.1f%% K:%.1f%%] - ���μ��� ��뷮.

		*/

		LONG AccpetTPS = g_lAcceptTPS_MMO;
		LONG SendTPS = g_lSendPostTPS_MMO;
		LONG RecvTPS = g_lRecvTPS_MMO;
		LONG AuthFPS = g_lAuthFPS;
		LONG GameFPS = g_lGameFPS;
		InterlockedExchange(&g_lAcceptTPS_MMO, 0);
		InterlockedExchange(&g_lSendPostTPS_MMO, 0);
		InterlockedExchange(&g_lRecvTPS_MMO, 0);
		InterlockedExchange(&g_lAuthFPS, 0);
		InterlockedExchange(&g_lGameFPS, 0);

		// ��� ����, ���μ��� ��뷮 ����
		ProcessUsage.UpdateCpuTime();

		printf("========================================================\n"
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

			"========================================================\n\n"
			"CPU usage [MMOServer:%.1f%% U:%.1f%% K:%.1f%%]\n",


			// ----------- ���� ������
			GetClientCount(), g_lAuthModeUserCount, g_lGameModeUserCount, g_lAuthModeUserCount+g_lGameModeUserCount,
			g_lAllocNodeCount, GetASQ_Count(),
			g_ullAcceptTotal_MMO, AccpetTPS, SendTPS, RecvTPS,
			AuthFPS, GameFPS,
			CProtocolBuff_Net::GetChunkCount(), CProtocolBuff_Net::GetOutChunkCount(),
			GetChunkCount(), GetOutChunkCount(),

			// ----------- ���μ��� ��뷮 
			ProcessUsage.ProcessTotal(), ProcessUsage.ProcessUser(), ProcessUsage.ProcessKernel()
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
		// ChatServer config �о����
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

		return true;
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