#include "pch.h"
#include "LoginServer.h"
#include "Protocol_Set\CommonProtocol.h"
#include "Parser\Parser_Class.h"
#include "Log\Log.h"

#include <strsafe.h>

extern LONG g_lStruct_PlayerCount;
ULONGLONG g_ullDisconnectTotal;

// ----------------------------------
// LoginServer(NetServer)
// ----------------------------------
namespace Library_Jingyu
{
	CCrashDump* g_LoginDump = CCrashDump::GetInstance();

	// �α� ���� �������� �ϳ� �ޱ�.
	CSystemLog* cLoginLibLog = CSystemLog::GetInstance();
	

	/////////////////////////////
	// �ܺο��� ȣ�� ���� �Լ�
	/////////////////////////////


	// !! �׽�Ʈ�� !!
	// ������ ������ ���
	ULONGLONG CLogin_NetServer::GetLanClientCount()
	{
		return m_cLanS->GetClientCount();
	}


	// ������
	CLogin_NetServer::CLogin_NetServer()
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

		// Player TLS �����Ҵ�
		m_MPlayerTLS = new CMemoryPoolTLS< stPlayer >(100, false);

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
		// ���� �߰�
		if (m_cLanS->Start(m_stConfig.LanBindIP, m_stConfig.LanPort, m_stConfig.LanCreateWorker, m_stConfig.LanActiveWorker,
			m_stConfig.LanCreateAccept, m_stConfig.LanNodelay, m_stConfig.LanMaxJoinUser) == false)
			return false;

		// ------------------- �ݼ��� ����
		if (Start(m_stConfig.BindIP, m_stConfig.Port, m_stConfig.CreateWorker, m_stConfig.ActiveWorker, m_stConfig.CreateAccept, m_stConfig.Nodelay, m_stConfig.MaxJoinUser,
			m_stConfig.HeadCode, m_stConfig.XORCode1, m_stConfig.XORCode2) == false)
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

		// 2. ���� ���� (AccountNo, ID, NickName)
		NowPlayer->second->m_i64AccountNo = AccountNo;

		int len = (int)strlen(UserID);
		MultiByteToWideChar(CP_UTF8, 0, UserID, (int)strlen(UserID), NowPlayer->second->m_wcID, len);

		len = (int)strlen(NickName);
		MultiByteToWideChar(CP_UTF8, 0, NickName, (int)strlen(NickName), NowPlayer->second->m_wcNickName, len);


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
		default:
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
		// -- account ���̺��� ������ �����ϴ°�
		// -- account ������ ���� ��ūŰ�� ��ġ�ϴ°�
		// -- status ���°� �α׾ƿ� ���ΰ�

		// ���� ������
		char cQurey[200] = "SELECT * FROM `v_account` WHERE `accountno` = %d\0";
		m_AcountDB_Connector->Query(cQurey, AccountNo);

		MYSQL_ROW rowData = m_AcountDB_Connector->FetchRow();

		// -- account ���̺��� ������ �����ϴ°�
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
		SendPacket(SessionID, SendBuff);
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
		SendBuff->PutData((char*)&Type, 2);

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
		SendPacket(SessionID, SendBuff);
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
		default:
			Disconnect(SessionID);
			break;
		}	


	}

	// ��Ŷ �۽� �Ϸ� �� ȣ��Ǵ� �Լ�
	//
	// parameter : ���� ����Ű, Send �� ������
	// return : ����
	void CLogin_NetServer::OnSend(ULONGLONG SessionID, DWORD SendSize)
	{
		// 1. ���� �˻�
		stPlayer* NowPlayer = FindPlayerFunc(SessionID);

		// ���°� ���̾ȵ�. ũ����
		if (NowPlayer == nullptr)
			g_LoginDump->Crash();

		// 2. ������ ����, �ش� ������ �α��� ���·� ����
		//WCHAR qurey[200] = L"UPDATE `status` SET `status` = 1 WHERE `accountno` = %d";
		//m_AcountDB_Connector->Query_Save(qurey, NowPlayer->m_i64AccountNo);

		// �����͸� ��������, �ش� ������ ���� ����
		InterlockedIncrement(&g_ullDisconnectTotal);
		Disconnect(SessionID);		
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
	{}





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
	{
		// �� �ʱ�ȭ
		InitializeSRWLock(&srwl);
	}

	// �Ҹ���
	CLogin_LanServer::~CLogin_LanServer()
	{
		// �ϴ°� ����
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

			// �� ���� ���� �߸��Ȱ�. ����
		default:
			g_LoginDump->Crash();
			break;
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

