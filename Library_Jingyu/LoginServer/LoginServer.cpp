#include "pch.h"
#include "LoginServer.h"
#include "Protocol_Set\CommonProtocol.h"
#include "Parser\Parser_Class.h"
#include "Log\Log.h"

#include <process.h>

// ��¿� �ڵ�
extern LONG g_lStruct_PlayerCount;
extern LONG	  g_lAcceptTPS;
extern LONG	g_lSendPostTPS;
extern ULONGLONG g_ullAcceptTotal;

extern LONG g_lAllocNodeCount;
extern LONG g_lAllocNodeCount_Lan;
LONG g_lStruct_PlayerCount;

extern LONG g_lSemCount;
extern LONG g_lDisconnecetCount;


// ----------------------------------
// LoginServer(NetServer)
// ----------------------------------
namespace Library_Jingyu
{
	CCrashDump* g_LoginDump = CCrashDump::GetInstance();

	// �α� ���� �������� �ϳ� �ޱ�.
	CSystemLog* cLoginLibLog = CSystemLog::GetInstance();

	// ����ȭ ���� 1���� ũ��
	// �� ������ ���� ������ �����ؾ� ��.
	LONG g_lNET_BUFF_SIZE = 512;
	

	/////////////////////////////
	// �ܺο��� ȣ�� ���� �Լ�
	/////////////////////////////

	// ��¿� �Լ�
	void CLogin_NetServer::ShowPrintf()
	{
		// ���μ��� ��뷮 üũ�� Ŭ����
		CCpuUsage_Processor ProcessorUsage;

		// �ش� ���μ����� ��뷮 üũ�� Ŭ����
		CCpuUsage_Process ProcessUsage;

		while (1)
		{
			Sleep(1000);

			// ȭ�� ����� �� ����
			/*
			SessionNum : 	- NetServer �� ���Ǽ�
			PacketPool_Net : 	- �ܺο��� ��� ���� Net ����ȭ ������ ��

			PlayerData_Pool :	- Player ����ü �Ҵ緮
			Player Count :		- Contents ��Ʈ Player�� �� (�ۿ��� ������� ��� �� ����)

			Accept Total :		- Accept ��ü ī��Ʈ (accept ���Ͻ� +1)
			Accept TPS :		- Accept ó�� Ƚ��
			Send TPS			- �ʴ� Send�Ϸ� Ƚ��. (�Ϸ��������� ����)

			Net_BuffChunkAlloc_Count : - Net ����ȭ ���� �� Alloc�� ûũ �� (�ۿ��� ������� ûũ ��)
			Player_ChunkAlloc_Count : - �÷��̾� �� Alloc�� ûũ �� (�ۿ��� ������� ûũ ��)

			DisconnecetCount : - �������� ������ ���� �˴ٿ� �� �� 1�� ����

			----------------------------------------------------
			SessionNum : 	- LanServer �� ���Ǽ�
			PacketPool_Lan : 	- �ܺο��� ��� ���� Lan ����ȭ ������ ��

			Lan_BuffChunkAlloc_Count : - Lan ����ȭ ���� �� Alloc�� ûũ �� (�ۿ��� ������� ûũ ��)

			----------------------------------------------------
			CPU usage [T:%.1f%% U:%.1f%% K:%.1f%%] [LoginServer:%.1f%% U:%.1f%% K:%.1f%%] - ���μ��� / ���μ��� ��뷮.
			*/

			LONG AccpetTPS = g_lAcceptTPS;
			LONG SendTPS = g_lSendPostTPS;
			InterlockedExchange(&g_lAcceptTPS, 0);
			InterlockedExchange(&g_lSendPostTPS, 0);

			// ��� ����, ���μ��� / ���μ��� ��뷮 ����
			ProcessUsage.UpdateCpuTime();
			ProcessorUsage.UpdateCpuTime();

			printf("========================================================\n"
				"SessionNum : %lld\n"
				"PacketPool_Net : %d\n\n"

				"PlayerData_Pool : %d\n"
				"Player Count : %lld\n\n"

				"Accept Total : %lld\n"
				"Accept TPS : %d\n"
				"Send TPS : %d\n\n"

				"Net_BuffChunkAlloc_Count : %d (Out : %d)\n"
				"Player_ChunkAlloc_Count : %d (Out : %d)\n\n"

				"DisconnectCount : %d\n\n"

				"------------------------------------------------\n"
				"SessionNum : %lld\n"
				"PacketPool_Lan : %d\n\n"

				"Lan_BuffChunkAlloc_Count : %d (Out : %d)\n\n"

				"========================================================\n\n"
				"CPU Usage [T:%.1f%%  U:%.1f%%  K:%.1f%%]  [LoginServer:%.1f%%  U:%.1f%%  K:%.1f%%]\n",

				// ----------- �α��� �� ������
				GetClientCount(), g_lAllocNodeCount,
				g_lStruct_PlayerCount, m_umapJoinUser.size(),
				g_ullAcceptTotal, AccpetTPS, SendTPS,
				CProtocolBuff_Net::GetChunkCount(), CProtocolBuff_Net::GetOutChunkCount(),
				m_MPlayerTLS->GetAllocChunkCount(), m_MPlayerTLS->GetOutChunkCount(),
				g_lDisconnecetCount,

				// ----------- �α��� �� ������
				m_cLanS->GetClientCount(), g_lAllocNodeCount_Lan,
				CProtocolBuff_Lan::GetChunkCount(), CProtocolBuff_Lan::GetOutChunkCount(),

				// ----------- ���μ��� / ���μ��� ��뷮 
				ProcessorUsage.ProcessorTotal(), ProcessorUsage.ProcessorUser(), ProcessorUsage.ProcessorKernel(),
				ProcessUsage.ProcessTotal(), ProcessUsage.ProcessUser(), ProcessUsage.ProcessKernel());
		}
		
	}


	// ������
	CLogin_NetServer::CLogin_NetServer()
		:CNetServer()
	{
		// ------------------- ���Ͽ��� �ʿ��� ���� �о����
		if (SetFile(&m_stConfig) == false)
			g_LoginDump->Crash();

		// ------------------- �α� ������ ���� ����
		cLoginLibLog->SetDirectory(L"LoginServer");
		cLoginLibLog->SetLogLeve((CSystemLog::en_LogLevel)m_stConfig.LogLevel);

		// �� �ʱ�ȭ
		InitializeSRWLock(&srwl);

		// LanServer �����Ҵ�
		m_cLanS = new CLogin_LanServer;		

		// ����͸� LanClient �����Ҵ�.
		m_LanMonitorC = new CMoniter_Clinet;

		// Player TLS �����Ҵ�
		m_MPlayerTLS = new CMemoryPoolTLS< stPlayer >(50, false);

		// DBConnector ����
		m_AcountDB_Connector = new CBConnectorTLS(m_stConfig.DB_IP, m_stConfig.DB_User, m_stConfig.DB_Password, m_stConfig.DB_Name, m_stConfig.DB_Port);
		

		// LanServer�� this ����
		m_cLanS->ParentSet(this);
	}

	// �Ҹ���
	CLogin_NetServer::~CLogin_NetServer()
	{
		// LanServer ��������
		delete m_cLanS;

		// ����͸� ������ ����� LanClient ��������
		delete m_LanMonitorC;

		// Player TLS ��������
		delete m_MPlayerTLS;

		// DBConnector ����
		delete m_AcountDB_Connector;
	}


	// �α��� ���� ���� �Լ�
	// ���������� NetServer�� Start ȣ��
	// 
	// return false : ���� �߻� ��. �����ڵ� ���� �� false ����
	// return true : ����
	bool CLogin_NetServer::ServerStart()
	{
		// ------------------- �� ���� ����
		if (m_cLanS->Start(m_stConfig.LanBindIP, m_stConfig.LanPort, m_stConfig.LanCreateWorker, m_stConfig.LanActiveWorker,
			m_stConfig.LanCreateAccept, m_stConfig.LanNodelay, m_stConfig.LanMaxJoinUser) == false)
			return false;

		// ------------------- �ݼ��� ����
		if (Start(m_stConfig.BindIP, m_stConfig.Port, m_stConfig.CreateWorker, m_stConfig.ActiveWorker, m_stConfig.CreateAccept, m_stConfig.Nodelay, m_stConfig.MaxJoinUser,
			m_stConfig.HeadCode, m_stConfig.XORCode1, m_stConfig.XORCode2) == false)
			return false;

		// ------------------- ����͸� ������ ����Ǵ�, ����͸� Ŭ���̾�Ʈ ����
		if (m_LanMonitorC->ClientStart(m_stConfig.MonitorServerIP, m_stConfig.MonitorServerPort, m_stConfig.ClientCreateWorker, 
			m_stConfig.ClientActiveWorker, m_stConfig.ClientNodelay) == false)
			return false;

		// ���� ���� �α� ���		
		cLoginLibLog->LogSave(L"LoginServer", CSystemLog::en_LogLevel::LEVEL_SYSTEM, L"ServerOpen...");

		return true;
	}

	// �α��� ���� ���� �Լ�
	//
	// Parameter : ����
	// return : ����
	void CLogin_NetServer::ServerStop()
	{
		// �ݼ��� ��ž (����Ʈ, ��Ŀ ����)
		Stop();
		
		// ------------------- �� ���� ��ž
		m_cLanS->Stop();

		// ------------------- ����͸� ������ ����Ǵ�, ����͸� Ŭ���̾�Ʈ ����
		m_LanMonitorC->ClientStop();

		// ------------- ��� ����
		// ���� ����.


		// ------------- ���ҽ� �ʱ�ȭ		

		// Playermap�� �ִ� ��� ���� ��ȯ
		// �ݼ����� ��ž�Ǹ鼭 �̹� ��� ������ Disconnect �Ǿ���
		auto itor = m_umapJoinUser.begin();

		while (itor != m_umapJoinUser.end())
		{
			// �޸�Ǯ�� ��ȯ
			m_MPlayerTLS->Free(itor->second);
			InterlockedAdd(&g_lStruct_PlayerCount, -1);
		}

		// umap �ʱ�ȭ
		m_umapJoinUser.clear();

		// ���� ���� �α� ���		
		cLoginLibLog->LogSave(L"LoginServer", CSystemLog::en_LogLevel::LEVEL_SYSTEM, L"ServerStop...");
	}







	/////////////////////////////
	// ��� ó�� �Լ�
	/////////////////////////////

	// ���Ͽ��� Config ���� �о����
	// 
	// 
	// Parameter : config ����ü
	// return : ���������� ���� �� true
	//		  : �� �ܿ��� false
	bool CLogin_NetServer::SetFile(stConfigFile* pConfig)
	{
		Parser Parser;

		// ���� �ε�
		try
		{
			Parser.LoadFile(_T("LoginServer_Config.ini"));
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
		// LoginServer config �о����
		////////////////////////////////////////////////////////

		// ���� ���� -------------------------
		if (Parser.AreaCheck(_T("LOGINSERVER")) == false)
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
		// ���� IP config �о����
		////////////////////////////////////////////////////////

		// ���� ���� -------------------------
		if (Parser.AreaCheck(_T("IPCONFIG")) == false)
			return false;

		/*
		// ���Ӽ��� IP
		if (Parser.GetValue_String(_T("GameServerIP"), pConfig->GameServerIP) == false)
			return false;

		// ���Ӽ��� Port
		if (Parser.GetValue_Int(_T("GameServerPort"), &pConfig->GameServerPort) == false)
			return false;
		*/

		// ä�ü��� IP
		if (Parser.GetValue_String(_T("ChatServerIP"), pConfig->ChatServerIP) == false)
			return false;

		// ä�ü��� Port
		if (Parser.GetValue_Int(_T("ChatServerPort"), &pConfig->ChatServerPort) == false)
			return false;

		
		////////////////////////////////////////////////////////
		// DB config �о����
		////////////////////////////////////////////////////////

		// ���� ���� -------------------------
		if (Parser.AreaCheck(_T("ACCOUNTDBCONFIG")) == false)
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

		

		////////////////////////////////////////////////////////
		// LanServer Config �о����
		////////////////////////////////////////////////////////

		// ���� ���� -------------------------
		if (Parser.AreaCheck(_T("LANSERVER")) == false)
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
		// ����͸� Ŭ���̾�Ʈ Config �о����
		////////////////////////////////////////////////////////

		// ���� ���� -------------------------
		if (Parser.AreaCheck(_T("MONITORCLIENT")) == false)
			return false;

		// IP
		if (Parser.GetValue_String(_T("MonitorServerIP"), pConfig->MonitorServerIP) == false)
			return false;

		// Port
		if (Parser.GetValue_Int(_T("MonitorServerPort"), &pConfig->MonitorServerPort) == false)
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


		return true;
	}


	   	  








	/////////////////////////////
	// �ڷᱸ�� ���� �Լ�
	/////////////////////////////

	// ���� ���� �ڷᱸ������ ���� �˻�
	// ���� umap���� ������
	// 
	// Parameter : SessionID
	// return : �˻� ���� �� stPlayer*
	//		  : ���� �� nullptr
	CLogin_NetServer::stPlayer* CLogin_NetServer::FindPlayerFunc(ULONGLONG SessionID)
	{
		// 1. ���� ã��
		AcquireSRWLockShared(&srwl);		// ----------- ��
		auto NowPlayer = m_umapJoinUser.find(SessionID);

		// 2. ���ϴ� ������ ��ã������, nullptr ����
		if (NowPlayer == m_umapJoinUser.end())
		{
			ReleaseSRWLockShared(&srwl);		// ----------- ���
			return nullptr;
		}

		// 3. ���ϴ� ������ ã������ �ش� ������ ����
		stPlayer* retPlayer = NowPlayer->second;

		ReleaseSRWLockShared(&srwl);		// ----------- ���

		return retPlayer;
	}
	   
	// ���� ���� �ڷᱸ����, ���� ������ ���� �߰�
	// ���� umap���� ������
	// 
	// Parameter : SessionID, stPlayer*
	// return : �߰� ���� ��, true
	//		  : SessionID�� �ߺ��� �� false
	bool CLogin_NetServer::InsertPlayerFunc(ULONGLONG SessionID, stPlayer* InsertPlayer)
	{
		AcquireSRWLockExclusive(&srwl);		// ----------- ��
		auto ret = m_umapJoinUser.insert(make_pair(SessionID, InsertPlayer));
		ReleaseSRWLockExclusive(&srwl);		// ----------- ���

		// �ߺ��� Ű�� �� false ����.
		if (ret.second == false)
			return false;

		return true;
	}
	
	// ���� ���� �ڷᱸ���� �ִ�, ������ ���� ����
	// ���� umap���� ������
	// 
	// Parameter : SessionID, AccountNo, UserID, NickName
	// return : ����
	void CLogin_NetServer::SetPlayerFunc(ULONGLONG SessionID, INT64 AccountNo, char* UserID, char* NickName)
	{
		// 1. ���� ã��
		AcquireSRWLockExclusive(&srwl);		// ----------- ��
		auto NowPlayer = m_umapJoinUser.find(SessionID);

		// ���ϴ� ������ ��ã������, ���� ���ӵ� ���� ����?�� �̻��� ��Ŷ�� ������.
		// �����Ѵ�
		if (NowPlayer == m_umapJoinUser.end())
		{
			ReleaseSRWLockExclusive(&srwl);		// ----------- ���
			return;
		}

		// 2. ���� ���� (AccountNo, ID, NickName, �α��� ����)
		NowPlayer->second->m_i64AccountNo = AccountNo;

		int len = (int)strlen(UserID);
		MultiByteToWideChar(CP_UTF8, 0, UserID, (int)strlen(UserID), NowPlayer->second->m_wcID, len);

		len = (int)strlen(NickName);
		MultiByteToWideChar(CP_UTF8, 0, NickName, (int)strlen(NickName), NowPlayer->second->m_wcNickName, len);

		NowPlayer->second->m_bLoginCheck = true;


		ReleaseSRWLockExclusive(&srwl);		// ----------- ���
	}

	// ���� ���� �ڷᱸ���� �ִ�, ���� ����
	// ���� umap���� ������
	// 
	// Parameter : SessionID
	// return : ���� ��, stPlayer*
	//		  : ���� ��, nullptr
	CLogin_NetServer::stPlayer* CLogin_NetServer::ErasePlayerFunc(ULONGLONG SessionID)
	{
		// 1. ���� ã��
		AcquireSRWLockExclusive(&srwl);		// ----------- ��
		auto FindPlayer = m_umapJoinUser.find(SessionID);

		// ���ϴ� ������ ��ã������, �̹� �����߰ų� ���� ����.
		// �����Ѵ�
		if (FindPlayer == m_umapJoinUser.end())
		{
			ReleaseSRWLockExclusive(&srwl);		// ----------- ���
			return nullptr;
		}

		// 2. ������ �� �޾Ƶд�.
		stPlayer* ret = FindPlayer->second;

		// 3. �ڷᱸ������ erase
		m_umapJoinUser.erase(FindPlayer);

		ReleaseSRWLockExclusive(&srwl);		// ----------- ���

		// 4. �� ����
		return ret;
	}




	/////////////////////////////
	// Lan�� ��Ŷ ó�� �Լ�
	/////////////////////////////	

	// LanClient���� ���� ��Ŷ ó�� �Լ� (����)
	//
	// Parameter : ��Ŷ Ÿ��, AccountNo, SessionID(LanClient���� ���´� Parameter)
	// return : ����
	void CLogin_NetServer::LanClientPacketFunc(WORD Type, INT64 AccountNo, ULONGLONG SessionID)
	{
		// Type�� ���� �б⹮ ó��

		switch (Type)
		{
			// ����,ä�ÿ��� ���� ���� �Ϸ� ��, �������� ������Ŷ Send
		case en_PACKET_SS_RES_NEW_CLIENT_LOGIN:
			Success_Packet(SessionID, AccountNo, dfLOGIN_STATUS_OK);
			break;

			// ���� ������ �� �� ��Ŷ ������ ���� ���� �ȵ�. 
			// ��������̱� ������ ������ �����ؾ���. ũ����
		default:
			g_LoginDump->Crash();
			break;
		}

	}




	/////////////////////////////
	// Net�� ��Ŷ ó�� �Լ�
	/////////////////////////////

	// �α��� ��û
	// 
	// Parameter : SessionID, Packet*
	// return : ����
	void CLogin_NetServer::LoginPacketFunc(ULONGLONG SessionID, CProtocolBuff_Net* Packet)
	{
		// 1. AccountNo, Token ������
		INT64 AccountNo;
		char Token[64];

		Packet->GetData((char*)&AccountNo, 8);
		Packet->GetData(Token, 64);

		// 2. DB�� ���� ����
		// ���� ����
		// -- account ���̺� ������ �����ϴ°�
		// -- account ������ ���� ��ūŰ�� ��ġ�ϴ°�
		// -- status ���°� �α׾ƿ� ���ΰ�

		// ���� ������
		char cQurey[200] = "SELECT * FROM `v_account` WHERE `accountno` = %d\0";
		m_AcountDB_Connector->Query(cQurey, AccountNo);

		MYSQL_ROW rowData = m_AcountDB_Connector->FetchRow();

		// -- account ���̺� ������ �����ϴ°�
		if (rowData == NULL)
		{
			// ���� ����
			m_AcountDB_Connector->FreeResult();

			// �������� �ʴ´ٸ�, "�������� ����" �޽����� Send�Ѵ�.
			Fail_Packet(SessionID, AccountNo, dfLOGIN_STATUS_ACCOUNT_MISS);

			return;
		}

		// -- account ������ ���� ��ūŰ�� ��ġ�ϴ°�
		// ���� NULL �̰ų� ��ġ���� ������ �޽��� ����
		if(rowData[3] == NULL ||
			memcmp(rowData[3], Token, 64) != 0)
		{
			/*
			// ��ġ���� �ʴ´ٸ� "��ū Ű ��ġ���� ����" �޽����� Send�Ѵ�.
			Fail_Packet(SessionID, AccountNo, dfLOGIN_STATUS_SESSION_MISS, rowData[1], rowData[2]);

			// ���� ����
			m_AcountDB_Connector->FreeResult();
			return;
			*/
		}

		// -- status ���°� �α׾ƿ� ���ΰ�
		if (*rowData[4] != '0')
		{
			// ���� ����
			m_AcountDB_Connector->FreeResult();

			// �α� �ƿ� ���� �ƴ϶��, "���� ������ ��������" �޽����� Send�Ѵ�.
			Fail_Packet(SessionID, AccountNo, dfLOGIN_STATUS_STATUS_MISS, rowData[1], rowData[2]);
			
			return;
		}		

		// 3. �÷��̾� ����
		SetPlayerFunc(SessionID, AccountNo, rowData[1], rowData[2]);

		// 4. LanServer�� ����, �� ������ ���� ����
		m_cLanS->UserJoinSend(AccountNo, Token, SessionID);

		// ���� ����
		m_AcountDB_Connector->FreeResult();

	}   	
	   	  
	// "����" ��Ŷ ����� ������
	//
	// Parameter : SessionID, AccountNo, Status, UserID, NickName
	// return : ����
	void CLogin_NetServer::Success_Packet(ULONGLONG SessionID, INT64 AccountNo, BYTE Status)
	{
		// 1. ���� ���翩�� üũ
		stPlayer* NowPlayer = FindPlayerFunc(SessionID);

		// ���� �������, �̹� �α׾ƿ� �� ����
		// ó�� ���� ����.
		if (NowPlayer == nullptr)
			return;

		// 2. AccountNo�� ������ üũ
		// �ٸ��ٸ�, ����. ������ ���� ���翩�θ� üũ�߱� ������ ���⼭ �ɸ��� �ȵ�.
		if (NowPlayer->m_i64AccountNo != AccountNo)
			g_LoginDump->Crash();

		// 3. ����ȭ���� Alloc
		CProtocolBuff_Net* SendBuff = CProtocolBuff_Net::Alloc();

		// 4. �� ����
		WORD Type = en_PACKET_CS_LOGIN_RES_LOGIN;
		SendBuff->PutData((char*)&Type, 2);
		SendBuff->PutData((char*)&AccountNo, 8);
		SendBuff->PutData((char*)&Status, 1);

		SendBuff->PutData((char*)&NowPlayer->m_wcID, 40);
		SendBuff->PutData((char*)&NowPlayer->m_wcNickName, 40);
		
		SendBuff->MoveWritePos(34);

		/*SendBuff->PutData((char*)&m_stConfig.GameServerIP, 32);
		SendBuff->PutData((char*)&m_stConfig.GameServerPort, 2);*/

		SendBuff->PutData((char*)&m_stConfig.ChatServerIP, 32);
		SendBuff->PutData((char*)&m_stConfig.ChatServerPort, 2);

		// 5. SendPacket()
		// ������ ����.
		SendPacket(SessionID, SendBuff, TRUE);
	}

	// "����" ��Ŷ ����� ������
	//
	// Parameter : SessionID, AccountNo, Status, UserID, NickName
	// return : ����
	void CLogin_NetServer::Fail_Packet(ULONGLONG SessionID, INT64 AccountNo, BYTE Status, char* UserID, char* NickName)
	{
		// 1. ����ȭ���� Alloc
		CProtocolBuff_Net* SendBuff = CProtocolBuff_Net::Alloc();

		// 2. �� ����
		WORD Type = en_PACKET_CS_LOGIN_RES_LOGIN;
		SendBuff->PutData((char*)&Type, 2);
		SendBuff->PutData((char*)&AccountNo, 8);
		SendBuff->PutData((char*)&Status, 1);

		// ID�� Password�� nullptr�̶��, ����ȭ������ rear�� �����δ�.
		// (AccountNo�� ������ �� �� ������ ID�� �г����� Ȯ�� �Ұ����� ��Ȳ.)
		if (UserID == nullptr)
		{
			SendBuff->MoveWritePos(80);
		}

		else
		{
			SendBuff->PutData((char*)&UserID, 40);
			SendBuff->PutData((char*)&NickName, 40);
		}

		// ������ ������ ���� ���ϱ� ������, IP�� �ʿ����.
		// �� ��ŭ�� Rear�� �����δ�.
		SendBuff->MoveWritePos(68);

		// 3. SendPacket()
		// ������ ����
		SendPacket(SessionID, SendBuff, TRUE);
	}









	// Accept ����, ȣ��ȴ�.
	//
	// parameter : ������ ������ IP, Port
	// return false : Ŭ���̾�Ʈ ���� �ź�
	// return true : ���� ���
	bool CLogin_NetServer::OnConnectionRequest(TCHAR* IP, USHORT port)
	{
		return true;
	}

	// ���� �� ȣ��Ǵ� �Լ� (AcceptThread���� Accept �� ȣ��)
	//
	// parameter : ������ �������� �Ҵ�� ����Ű
	// return : ����
	void CLogin_NetServer::OnClientJoin(ULONGLONG SessionID)
	{
		// 1. �÷��̾� TLS���� Alloc
		stPlayer* JoinPlayer = m_MPlayerTLS->Alloc();
		InterlockedAdd(&g_lStruct_PlayerCount, 1);

		// �α��� �� �ƴ� ����
		JoinPlayer->m_bLoginCheck = false;

		// 2. umap�� ���� �߰�
		if (InsertPlayerFunc(SessionID, JoinPlayer) == false)
		{
			printf("duplication SessionID!!\n");
			g_LoginDump->Crash();
		}

	}

	// ���� ���� �� ȣ��Ǵ� �Լ� (InDIsconnect �ȿ��� ȣ��)
	//
	// parameter : ���� ����Ű
	// return : ����
	void CLogin_NetServer::OnClientLeave(ULONGLONG SessionID)
	{
		// 1. ���� ����
		stPlayer* ErasePlayer = ErasePlayerFunc(SessionID);
		if (ErasePlayer == nullptr)
			g_LoginDump->Crash();

		// 2. stPlayer* ��ȯ
		m_MPlayerTLS->Free(ErasePlayer);
		InterlockedAdd(&g_lStruct_PlayerCount, -1);
	}

	// ��Ŷ ���� �Ϸ� �� ȣ��Ǵ� �Լ�.
	//
	// parameter : ���� ����Ű, ���� ��Ŷ
	// return : ����
	void CLogin_NetServer::OnRecv(ULONGLONG SessionID, CProtocolBuff_Net* Payload)
	{		
		try
		{
			// 1. Type�� ������
			WORD Type;
			Payload->GetData((char*)&Type, 2);

			// 2. Type�� ���� �б�ó��
			switch (Type)
			{
				// �α��� ��û
			case en_PACKET_CS_LOGIN_REQ_LOGIN:
				LoginPacketFunc(SessionID, Payload);
				break;

				// �α��� ��û�� �ƴϸ� ���� ����
				// ����� �α��� ��û �ܿ� Ÿ���� �� ������ ����
			default:
				throw CException(_T("OnRecv(). TypeError"));
				break;
			}

		}
		catch (CException& exc)
		{
			char* pExc = exc.GetExceptionText();		

			// �α� ��� (�α� ���� : ����)
			cLoginLibLog->LogSave(L"LoginServer", CSystemLog::en_LogLevel::LEVEL_ERROR, L"%s",
				(TCHAR*)pExc);	

			// ���� ���� ��û
			Disconnect(SessionID);
		}
	}

	// ��Ŷ �۽� �Ϸ� �� ȣ��Ǵ� �Լ�
	//
	// parameter : ���� ����Ű, Send �� ������
	// return : ����
	void CLogin_NetServer::OnSend(ULONGLONG SessionID, DWORD SendSize)
	{

	}

	// ��Ŀ �����尡 ��� �� ȣ��Ǵ� �Լ�.
	// GQCS �ٷ� �ϴܿ��� ȣ��
	// 
	// parameter : ����
	// return : ����
	void CLogin_NetServer::OnWorkerThreadBegin()
	{}

	// ��Ŀ �����尡 ���� �� ȣ��Ǵ� �Լ�
	// GQCS �ٷ� ������ ȣ��
	// 
	// parameter : ����
	// return : ����
	void CLogin_NetServer::OnWorkerThreadEnd()
	{}

	// ���� �߻� �� ȣ��Ǵ� �Լ�.
	//
	// parameter : ���� �ڵ�(���� ������ �����ڵ�� WinGetLastError() �Լ��� ��� ����. ���� ��� 0�� ���ϵ�)
	//			 : ���� �ڵ忡 ���� ��Ʈ��
	// return : ����
	void CLogin_NetServer::OnError(int error, const TCHAR* errorStr)
	{
		g_LoginDump->Crash();
	}





}


// ----------------------------------
// LoginServer(LanServer)
// ----------------------------------
namespace Library_Jingyu
{
	// �θ� ����
	//
	// Parameter : CLogin_NetServer*
	// return : ����
	void CLogin_LanServer::ParentSet(CLogin_NetServer* parent)
	{
		m_cParentS = parent;
	}


	// GameServer, ChatServer�� LanClient�� ���ο� ���� ���� �˸�
	// 
	// Parameter : AccountNo, Token(64Byte), ULONGLONG Parameter
	// return : ����
	void CLogin_LanServer::UserJoinSend(INT64 AccountNo, char* Token, ULONGLONG Parameter)
	{
		// !! ���ڷ� ���� Parameter�� NetServer�� SessionID !!

		// 1. SendBuff Alloc()
		CProtocolBuff_Lan* SendBuff = CProtocolBuff_Lan::Alloc();

		// 2. ������ ������ ��� ����(LanClient)���� ���� ������ ����
		WORD Type = en_PACKET_SS_REQ_NEW_CLIENT_LOGIN;

		SendBuff->PutData((char*)&Type, 2);
		SendBuff->PutData((char*)&AccountNo, 8);
		SendBuff->PutData(Token, 64);
		SendBuff->PutData((char*)&Parameter, 8);

		// 3. �迭 ���� ī��Ʈ�� 0�̶��, "���� ���� ������ ����" ��Ŷ ����
		
		AcquireSRWLockShared(&srwl);		// �� ------------------

		if (m_iArrayCount == 0)
		{
			ReleaseSRWLockShared(&srwl);	 // �� ���� ------------------
			m_cParentS->Fail_Packet(Parameter, AccountNo, dfLOGIN_STATUS_NOSERVER);
			return;
		}

		// 4. ������ Send�ϱ�
		// ������ ���� ������ ���� �� ��ŭ ���۷��� ī��Ʈ ����
		SendBuff->Add(m_iArrayCount);

		// ������ �� ��ŭ ���鼭 Send
		int index = 0;
		while (index < m_iArrayCount)
		{
			SendPacket(m_arrayJoinServer[index], SendBuff);
			index++;
		}	

		ReleaseSRWLockShared(&srwl);	 // �� ���� ------------------

		// 5. ��� Send������ Free
		CProtocolBuff_Lan::Free(SendBuff);
	}


	// ������
	CLogin_LanServer::CLogin_LanServer()
		:CLanServer()
	{
		// �� �ʱ�ȭ
		InitializeSRWLock(&srwl);

		m_iArrayCount = 0;
	}

	// �Ҹ���
	CLogin_LanServer::~CLogin_LanServer()
	{
		// ����, ������ �۵����̸� �۵� ����
		if (GetServerState() == true)
			Stop();
	}




	// -----------------------------------
	// �����Լ�
	// -----------------------------------

	// Accept ����, ȣ��ȴ�.
	//
	// parameter : ������ ������ IP, Port
	// return false : Ŭ���̾�Ʈ ���� �ź�
	// return true : ���� ���
	bool CLogin_LanServer::OnConnectionRequest(TCHAR* IP, USHORT port)
	{
		return true;
	}

	// ���� �� ȣ��Ǵ� �Լ� (AcceptThread���� Accept �� ȣ��)
	//
	// parameter : ������ �������� �Ҵ�� ����Ű
	// return : ����
	void CLogin_LanServer::OnClientJoin(ULONGLONG SessionID)
	{
		// 1. �迭�� ���� �߰�
		AcquireSRWLockExclusive(&srwl);		// ------- ��

		m_arrayJoinServer[m_iArrayCount] = SessionID;
		m_iArrayCount++;

		ReleaseSRWLockExclusive(&srwl);		// ------- �� ����
	}

	// ���� ���� �� ȣ��Ǵ� �Լ� (InDIsconnect �ȿ��� ȣ��)
	//
	// parameter : ���� ����Ű
	// return : ����
	void CLogin_LanServer::OnClientLeave(ULONGLONG SessionID)
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
			if (m_arrayJoinServer[Tempindex] == SessionID)
			{
				// ����, �ش� ������ ��ġ�� �������̶�� ī��Ʈ�� 1 ���̰� ��
				if (Tempindex == (m_iArrayCount - 1))
				{
					m_iArrayCount--;
					break;
				}

				// ������ ��ġ�� �ƴ϶��, ������ ��ġ�� ������ ��ġ�� ���� �� ī��Ʈ ����
				m_arrayJoinServer[Tempindex] = m_arrayJoinServer[m_iArrayCount - 1];
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
	void CLogin_LanServer::OnRecv(ULONGLONG SessionID, CProtocolBuff_Lan* Payload)
	{
		try
		{
			// 1. ��Ŷ ������
			WORD Type;
			Payload->GetData((char*)&Type, 2);

			// 2. Ÿ�Կ� ���� ó��
			switch (Type)
			{
				// �ٸ� ������ �α��� �������� �α���
				// ����� �Ұ� ����.
				// ���� ���Ӽ����� �߰��Ǹ�, ���Ӽ������� ����������?�� ���� �޶�����. �����ִ� ������ �޶�������?
			case en_PACKET_SS_LOGINSERVER_LOGIN:
				break;

				// ���� or ä�ÿ��� ������ �Դٸ�, �� ������ �߰�
			case en_PACKET_SS_RES_NEW_CLIENT_LOGIN:
			{
				INT64 AccountNo;
				ULONGLONG Parameter;

				Payload->GetData((char*)&AccountNo, 8);
				Payload->GetData((char*)&Parameter, 8);

				// 2. Net�� Lan ��Ŷó�� �Լ� ȣ��
				m_cParentS->LanClientPacketFunc(Type, AccountNo, Parameter);
			}
			break;

			// Ÿ�Կ��� �� throw ����
			default:
				throw CException(_T("OnRecv. Type Error."));				
				break;
			}

		}
		catch (const std::exception&)
		{
			// ����ſ��� �߸��� ���� ���°��� �׳� �ڵ忡���� �Ǵ�.
			// ũ����
			g_LoginDump->Crash();
		}
		
		
	}

	// ��Ŷ �۽� �Ϸ� �� ȣ��Ǵ� �Լ�
	//
	// parameter : ���� ����Ű, Send �� ������
	// return : ����
	void CLogin_LanServer::OnSend(ULONGLONG SessionID, DWORD SendSize)
	{

	}

	// ��Ŀ �����尡 ��� �� ȣ��Ǵ� �Լ�.
	// GQCS �ٷ� �ϴܿ��� ȣ��
	// 
	// parameter : ����
	// return : ����
	void CLogin_LanServer::OnWorkerThreadBegin()
	{

	}

	// ��Ŀ �����尡 ���� �� ȣ��Ǵ� �Լ�
	// GQCS �ٷ� ������ ȣ��
	// 
	// parameter : ����
	// return : ����
	void CLogin_LanServer::OnWorkerThreadEnd()
	{

	}

	// ���� �߻� �� ȣ��Ǵ� �Լ�.
	//
	// parameter : ���� �ڵ�(���� ������ �����ڵ�� WinGetLastError() �Լ��� ��� ����. ���� ��� 0�� ���ϵ�)
	//			 : ���� �ڵ忡 ���� ��Ʈ��
	// return : ����
	void CLogin_LanServer::OnError(int error, const TCHAR* errorStr)
	{

	}




}


// ----------------------------------
// ����͸� Ŭ���̾�Ʈ (LanClient)
// ----------------------------------
namespace Library_Jingyu
{
	// -----------------------
	// �����ڿ� �Ҹ���
	// -----------------------
	CMoniter_Clinet::CMoniter_Clinet()
		:CLanClient()
	{
		// ����͸� ���� �������� �����带 �����ų �̺�Ʈ ����
		//
		// �ڵ� ���� Event 
		// ���� ���� �� non-signalled ����
		// �̸� ���� Event	
		m_hMonitorThreadExitEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	}

	CMoniter_Clinet::~CMoniter_Clinet()
	{
		// Ŭ�� ���� �������̶�� ��������
		if (GetClinetState() == true)
			ClientStop();

		// �̺�Ʈ ����
		CloseHandle(m_hMonitorThreadExitEvent);
	}


	// --------------------------------
	// �ܺο��� ȣ�� ������ ��� �Լ�
	// --------------------------------

	// ���� �Լ�
	// ����������, ��ӹ��� CLanClient�� Startȣ��.
	//
	// Parameter : ������ ������ IP, ��Ʈ, ��Ŀ������ ��, Ȱ��ȭ��ų ��Ŀ������ ��, TCP_NODELAY ��� ����(true�� ���)
	// return : ���� �� true , ���� �� falsel 
	bool CMoniter_Clinet::ClientStart(TCHAR* ConnectIP, int Port, int CreateWorker, int ActiveWorker, int Nodelay)
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
	void CMoniter_Clinet::ClientStop()
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




	// -----------------------
	// ���ο����� ����ϴ� ��� �Լ�
	// -----------------------

	// ���� �ð����� ����͸� ������ ������ �����ϴ� ������
	UINT	WINAPI CMoniter_Clinet::MonitorThread(LPVOID lParam)
	{
		// this �޾Ƶα�
		CMoniter_Clinet* g_This = (CMoniter_Clinet*)lParam;

		// ���� ��ȣ �̺�Ʈ �޾Ƶα�
		HANDLE hEvent = g_This->m_hMonitorThreadExitEvent;

		// CPU ����� üũ Ŭ���� (�ϵ����)
		CCpuUsage_Processor CProcessorCPU;

		// PDH�� Ŭ����
		CPDH	CPdh;

		while (1)
		{
			// 1�ʿ� �ѹ� ����� ������ ������.
			DWORD Check = WaitForSingleObject(hEvent, 1000);

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

			// ���μ��� CPU �����, PDH ���� ����
			CProcessorCPU.UpdateCpuTime();
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
		}

		return 0;
	}

	// ����͸� ������ ������ ����
	//
	// Parameter : DataType(BYTE), DataValue(int), TimeStamp(int)
	// return : ����
	void CMoniter_Clinet::InfoSend(BYTE DataType, int DataValue, int TimeStamp)
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
	// �����Լ�
	// -----------------------

	// ��ǥ ������ ���� ���� ��, ȣ��Ǵ� �Լ� (ConnectFunc���� ���� ���� �� ȣ��)
	//
	// parameter : ����Ű
	// return : ����
	void CMoniter_Clinet::OnConnect(ULONGLONG SessionID)
	{
		// ���Ǿ��̵� ����صд�.
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
	void CMoniter_Clinet::OnDisconnect(ULONGLONG SessionID)
	{
		// ���� �ϴ°� ����. 
		// ������ �����, LanClient���� �̹� ���� �ٽ� �õ���.
	}

	// ��Ŷ ���� �Ϸ� �� ȣ��Ǵ� �Լ�.
	//
	// parameter : ���� ����Ű, ���� ��Ŷ
	// return : ����
	void CMoniter_Clinet::OnRecv(ULONGLONG SessionID, CProtocolBuff_Lan* Payload)
	{
		// ���� �Ұ� ����
	}

	// ��Ŷ �۽� �Ϸ� �� ȣ��Ǵ� �Լ�
	//
	// parameter : ���� ����Ű, Send �� ������
	// return : ����
	void CMoniter_Clinet::OnSend(ULONGLONG SessionID, DWORD SendSize)
	{
		// ���� �Ұ� ����
	}

	// ��Ŀ �����尡 ��� �� ȣ��Ǵ� �Լ�.
	// GQCS �ٷ� �ϴܿ��� ȣ��
	// 
	// parameter : ����
	// return : ����
	void CMoniter_Clinet::OnWorkerThreadBegin()
	{
		// ���� �Ұ� ����
	}

	// ��Ŀ �����尡 ���� �� ȣ��Ǵ� �Լ�
	// GQCS �ٷ� ������ ȣ��
	// 
	// parameter : ����
	// return : ����
	void CMoniter_Clinet::OnWorkerThreadEnd()
	{
		// ���� �Ұ� ����
	}

	// ���� �߻� �� ȣ��Ǵ� �Լ�.
	//
	// parameter : ���� �ڵ�(���� ������ �����ڵ�� WinGetLastError() �Լ��� ��� ����. ���� ��� 0�� ���ϵ�)
	//			 : ���� �ڵ忡 ���� ��Ʈ��
	// return : ����
	void CMoniter_Clinet::OnError(int error, const TCHAR* errorStr)
	{
		// ���� �Ұ� ����
	}




}

