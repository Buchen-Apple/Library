#include "pch.h"
#include "ChatServer.h"

#include "CommonProtocol.h"

#include <strsafe.h>

#include <time.h>
#include <process.h>
#include <Log\Log.h>

#include <map>

using namespace std;


// ��Ŷ�� Ÿ��
#define TYPE_JOIN	0
#define TYPE_LEAVE	1
#define TYPE_PACKET	2

// ���� ���� ��, ���Ƿ� �����صδ� ���� X,Y ��
#define TEMP_SECTOR_POS	12345

extern ULONGLONG g_ullUpdateStructCount;
extern ULONGLONG g_ullUpdateStruct_PlayerCount;
extern LONG		 g_lUpdateTPS;

// �α� ���� �������� �ϳ� �ޱ�.
CSystemLog* cChatLibLog = CSystemLog::GetInstance();


// ------------- ���� �׽�Ʈ��
extern int m_SectorPosError;

extern int m_SectorNoError;
extern int m_ChatNoError;

extern int m_TypeError;

extern int m_HeadCodeError;

extern int m_ChackSumError;
extern int m_HeaderLenBig;


// -------------------------------------
// Ŭ���� ���ο��� ����ϴ� �Լ�
// -------------------------------------

// ���Ͽ��� Config ���� �о����
// 
// 
// Parameter : config ����ü
// return : ���������� ���� �� true
//		  : �� �ܿ��� false
bool CChatServer::SetFile(stConfigFile* pConfig)
{
	Parser Parser;

	// ���� �ε�
	try
	{
		Parser.LoadFile(_T("ChatServer_Config.ini"));
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
			printf("FileR ead Fail...\n");
			return false;
		}
	}

	// ���� ����
	if (Parser.AreaCheck(_T("CHATSERVER")) == false)
		return false;

	// ------------ �о����
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


// Player ���� �ڷᱸ����, ���� �߰�
// ���� map���� ������
// 
// Parameter : SessionID, stPlayer*
// return : �߰� ���� ��, true
//		  : SessionID�� �ߺ��� ��(�̹� �������� ����) false
bool CChatServer::InsertPlayerFunc(ULONGLONG SessionID, stPlayer* insertPlayer)
{
	// map�� �߰�
	auto ret = m_mapPlayer.insert(make_pair(SessionID, insertPlayer));

	// �ߺ��� Ű�� �� false ����.
	if (ret.second == false)
		return false;

	return true;
}

// Player ���� �ڷᱸ������, ���� �˻�
// ���� map���� ������
// 
// Parameter : SessionID
// return : �˻� ���� ��, stPalyer*
//		  : �˻� ���� �� nullptr
CChatServer::stPlayer* CChatServer::FindPlayerFunc(ULONGLONG SessionID)
{
	auto FindPlayer = m_mapPlayer.find(SessionID);
	if (FindPlayer == m_mapPlayer.end())
		return nullptr;	

	return FindPlayer->second;
}

// Player ���� �ڷᱸ������, ���� ���� (�˻� �� ����)
// ���� map���� ������
// 
// Parameter : SessionID
// return : ���� ��, ���ŵ� ���� stPalyer*
//		  : �˻� ���� ��(���������� ���� ����) nullptr
CChatServer::stPlayer* CChatServer::ErasePlayerFunc(ULONGLONG SessionID)
{
	// 1) map���� ���� �˻�
	auto FindPlayer = m_mapPlayer.find(SessionID);
	if (FindPlayer == m_mapPlayer.end())	
		return nullptr;	

	stPlayer* ret = FindPlayer->second;

	// 2) �ʿ��� ����
	m_mapPlayer.erase(FindPlayer);

	return ret;
}

// ���ڷ� ���� 9�� ������ ��� ����(������ ��Ŷ�� ���� Ŭ�� ����)���� SendPacket ȣ��
//
// parameter : ���� ����, ���� 9��
// return : ����
void CChatServer::SendPacket_Sector(int SectorX, int SectorY, CProtocolBuff_Net* SendBuff)
{
	// 1. �ش� ���� �ֺ� 9�� ���� ����
	int iCurX, iCurY;

	// ���ڷ� ���� X����, [ �� | ���� | �� ]�� üũ�ؾ� �ϱ� ������, �º��� üũ�ϱ� ���� X�� 1 ����.
	SectorX--;

	// ���ڷ� ���� Y����,  
	// *************
	//		�� 
	//	   ����	
	//		��
	// *************
	// �� üũ�ؾ� �ϱ� ������, ����� üũ�ϱ� ���� Y�� 1 ����.
	SectorY--;	

	// Y ��ġ�� �������� ��,����,�� ������ ����.
	// ���� �޸� ������ ���� Y���� üũ.
	// ���� 2���� �迭��, [Y][X] ������ ������.
	DWORD dwSectorX, dwSectorY;

	for (iCurY = 0; iCurY < 3; iCurY++)
	{
		// SectorY��ġ�� ������ ��� ���, (0���� �۰ų�, SECTOR_Y_COUNT���� ũ�ų� ���� ���)
		// ���͸� ����� ������ continue
		if (SectorY + iCurY < 0 || SectorY + iCurY >= SECTOR_Y_COUNT)
			continue;

		for (iCurX = 0; iCurX < 3; iCurX++)
		{
			// SectorX��ġ�� ������ ��� ���, (0���� �۰ų�, SECTOR_X_COUNT���� ũ�ų� ���� ���)
			// ���͸� ����� ������ continue
			if (SectorX + iCurX < 0 || SectorX + iCurX >= SECTOR_X_COUNT)
				continue;	

			// 2. ���͸� ��������, �ش� ������ ��� �������� ������.
			dwSectorX = SectorX + iCurX;
			dwSectorY = SectorY + iCurY;			

			auto NowSector = m_listSecotr[dwSectorY][dwSectorX];

			auto itor = NowSector.begin();
			auto Enditor = NowSector.end();

			// !! for�� ���� ���� ī��Ʈ��, �ش� ������ ���� �� ��ŭ ���� !!
			// NetServer�ʿ���, �Ϸ� ������ ���� Free�� �ϱ� ������ Add�ؾ� �Ѵ�.
			SendBuff->Add((int)NowSector.size());

			// SendPacket
			while (itor != Enditor)
			{
				SendPacket((*itor)->m_ullSessionID, SendBuff);
				++itor;
			}
		}
	}	

	// 3. ��Ŷ Free
	// !! Net������ ���뿡�� Free ������, �ϳ��� ������Ű�鼭 ���±� ������ 1���� �� ������ ����. !!	
	CProtocolBuff_Net::Free(SendBuff);
}

// ������Ʈ ������
//
// static �Լ�.
// Parameter : ä�ü��� this
UINT WINAPI	CChatServer::UpdateThread(LPVOID lParam)
{
	CChatServer* g_this = (CChatServer*)lParam;

	// [���� ��ȣ, ���ϱ� ��ȣ] �������
	HANDLE hEvent[2] = { g_this->UpdateThreadEXITEvent , g_this->UpdateThreadEvent };

	// ������Ʈ ������
	while (1)
	{
		// ��ȣ�� ������ �����.
		DWORD Check = WaitForMultipleObjects(2, hEvent, FALSE, INFINITE);
		
		// �̻��� ��ȣ���
		if (Check == WAIT_FAILED)
		{
			DWORD Error = GetLastError();
			printf("UpdateThread Exit Error!!! (%d) \n", Error);
			break;
		}

		// ����, ���� ��ȣ�� �Դٸ������Ʈ ������ ����.
		else if (Check == WAIT_OBJECT_0)
			break;

		// ----------------- �ϰ��� �ִ��� Ȯ��. �ϰ� ������ ���Ѵ�.
		while (g_this->m_LFQueue->GetInNode() > 0)
		{
			// 1. ť���� �ϰ� 1�� ������	
			st_WorkNode* NowWork;

			if (g_this->m_LFQueue->Dequeue(NowWork) == -1)
				g_this->m_ChatDump->Crash();

			// 2. Type�� ���� ���� ó��
				// �б� ���� �������� ���̱� ����, ���� ���� ȣ��� �� �� �ͺ��� switch case�� ��ܿ� ��ġ.
			switch (NowWork->m_wType)
			{
				// ��Ŷ ó��
				// �ش� Ÿ�� ���ο���, ���� try ~ catch�� ó��.
			case TYPE_PACKET:
				g_this->Packet_Normal(NowWork->m_ullSessionID, NowWork->m_pPacket);

				// ��Ŷ Free
				CProtocolBuff_Net::Free(NowWork->m_pPacket);
				break;

				// ���� ó��
			case TYPE_JOIN:
				g_this->Packet_Join(NowWork->m_ullSessionID);
				break;

				// ���� ó��
			case TYPE_LEAVE:
				g_this->Packet_Leave(NowWork->m_ullSessionID);
				break;

			default:
				break;
			}

			InterlockedIncrement(&g_lUpdateTPS);  // �׽�Ʈ��!!

			// 3. �ϰ� Free
			g_this->m_MessagePool->Free(NowWork);
			InterlockedDecrement(&g_ullUpdateStructCount);

		}
		
	}

	printf("UpdateThread Exit!!\n");

	return 0;
}







// -------------------------------------
// Ŭ���� ���ο����� ����ϴ� ��Ŷ ó�� �Լ�
// -------------------------------------

// ���� ��Ŷó�� �Լ�
// OnClientJoin���� ȣ��
// 
// Parameter : SessionID
// return : ����
void CChatServer::Packet_Join(ULONGLONG SessionID)
{
	// 1) Player Alloc()
	stPlayer* JoinPlayer = m_PlayerPool->Alloc();

	InterlockedIncrement(&g_ullUpdateStruct_PlayerCount);

	// 2) SessionID ����
	JoinPlayer->m_ullSessionID = SessionID;	

	JoinPlayer->m_wSectorY = TEMP_SECTOR_POS;
	JoinPlayer->m_wSectorX = TEMP_SECTOR_POS;	

	// 3) Player ���� �ڷᱸ���� ���� �߰�
	if (InsertPlayerFunc(SessionID, JoinPlayer) == false)
	{
		printf("duplication SessionID!!\n");
		m_ChatDump->Crash();
	}	
}

// ���� ��Ŷó�� �Լ�
// OnClientLeave���� ȣ��
// 
// Parameter : SessionID
// return : ����
void CChatServer::Packet_Leave(ULONGLONG SessionID)
{
	ULONGLONG TempID = SessionID;	

	// 1) Player �ڷᱸ������ ����
	stPlayer* ErasePlayer = ErasePlayerFunc(SessionID);
	if (ErasePlayer == nullptr)
	{
		printf("Not Find Player!!\n");
		m_ChatDump->Crash();
	}	

	// 2) ���� �Ҵ��� �ƴ� ���, ���Ϳ��� ����
	if (ErasePlayer->m_wSectorY != TEMP_SECTOR_POS &&
		ErasePlayer->m_wSectorX != TEMP_SECTOR_POS)
	{
		m_listSecotr[ErasePlayer->m_wSectorY][ErasePlayer->m_wSectorX].remove(ErasePlayer);
	}

	// 3) �ʱ�ȭ ------------------
	// SectorPos�� �ʱ� ��ġ�� ����
	ErasePlayer->m_wSectorY = TEMP_SECTOR_POS;
	ErasePlayer->m_wSectorX = TEMP_SECTOR_POS;

	// 4) Player Free()
	m_PlayerPool->Free(ErasePlayer);

	InterlockedDecrement(&g_ullUpdateStruct_PlayerCount);
}

// �Ϲ� ��Ŷó�� �Լ�
// 
// Parameter : SessionID, CProtocolBuff_Net*
// return : ����
void CChatServer::Packet_Normal(ULONGLONG SessionID, CProtocolBuff_Net* Packet)
{
	// 1. ��Ŷ Ÿ�� Ȯ��
	WORD Type;
	*Packet >> Type;
	
	// 2. Ÿ�Կ� ���� switch case
	// �б� ���� ������ ������ ���� ���� ���� �� ���� �ϰ��� ��ܿ� ��ġ (����� ä�� �޽���)
	try
	{
		switch (Type)
		{
			// ä�ü��� ä�ú����� ��û
		case en_PACKET_CS_CHAT_REQ_MESSAGE:
			Packet_Chat_Message(SessionID, Packet);

			break;

			// ä�ü��� ���� �̵� ��û
		case en_PACKET_CS_CHAT_REQ_SECTOR_MOVE:
			Packet_Sector_Move(SessionID, Packet);

			break;

			// ��Ʈ��Ʈ
		case en_PACKET_CS_CHAT_REQ_HEARTBEAT:
			// �ϴ°� ����
			break;

			// �α��� ��û
		case en_PACKET_CS_CHAT_REQ_LOGIN:
			Packet_Chat_Login(SessionID, Packet);
			break;

		default:
			// �̻��� Ÿ���� ��Ŷ�� ���� ���´�.
			m_TypeError++;

			throw CException(_T("Packet_Normal(). TypeError"));			
			break;
		}

	}
	catch (CException& exc)
	{	
		// char* pExc = exc.GetExceptionText();		

		//// �α� ��� (�α� ���� : ����)
		//cChatLibLog->LogSave(L"NetServer", CSystemLog::en_LogLevel::LEVEL_ERROR, L"%s",
		//	(TCHAR*)pExc);	

		// ���� ���� ��û
		Disconnect(SessionID);
	}
	
}





// -------------------------------------
// '�Ϲ� ��Ŷ ó�� �Լ�'���� ó���Ǵ� �Ϲ� ��Ŷ��
// -------------------------------------

// ���� �̵���û ��Ŷ ó��
//
// Parameter : SessionID, Packet
// return : ���� �� true
//		  : ���������� ���� �����ų� ������ ���� �̵� �� false
void CChatServer::Packet_Sector_Move(ULONGLONG SessionID, CProtocolBuff_Net* Packet)
{	
	// 1) map���� ���� �˻�
	stPlayer* FindPlayer = FindPlayerFunc(SessionID);
	if (FindPlayer == nullptr)
	{
		printf("Not Find Player!!\n");
		return;
	}

	// 2) ������
	INT64 AccountNo;
	*Packet >> AccountNo;

	WORD wSectorX, wSectorY;
	*Packet >> wSectorX;
	*Packet >> wSectorY;
	
	// ��Ŷ ���� -----------------------------
	// 3) �̵��ϰ��� �ϴ� ���Ͱ� �������� üũ
	if (wSectorX < 0 || wSectorX > SECTOR_X_COUNT ||
		wSectorY < 0 || wSectorY > SECTOR_Y_COUNT)
	{
		m_SectorPosError++;

		// �������̸� ������ ���� ������
		throw CException(_T("Packet_Sector_Move(). Sector pos Error"));
	}

	// 4) AccountNo üũ
	if (AccountNo != FindPlayer->m_i64AccountNo)
	{
		m_SectorNoError++;

		// �������̸� ������ ���� ������
		throw CException(_T("Packet_Sector_Move(). AccountNo Error"));
	}
	
	// 4) ������ ���� ���� ����
	// ���� ���� ��Ŷ�� �ƴ϶��, ���� ���Ϳ��� ����.
	// �Ȱ��� ���̱� ������, �ϳ��� üũ�ص� �ȴ�.
	if (FindPlayer->m_wSectorY != TEMP_SECTOR_POS &&
		FindPlayer->m_wSectorX != TEMP_SECTOR_POS)
	{
		// ���� ���Ϳ��� ���� ����
		m_listSecotr[FindPlayer->m_wSectorY][FindPlayer->m_wSectorX].remove(FindPlayer);
	}

	// ���� ����
	FindPlayer->m_wSectorX = wSectorX;
	FindPlayer->m_wSectorY = wSectorY;

	// ���ο� ���Ϳ� ���� �߰�
	m_listSecotr[wSectorY][wSectorX].push_back(FindPlayer);

	// 5) Ŭ���̾�Ʈ���� ���� ��Ŷ ���� (���� �̵� ���)
	CProtocolBuff_Net* SendBuff = CProtocolBuff_Net::Alloc();

	// Ÿ��, AccountNo, SecotrX, SecotrY
	WORD SendType = en_PACKET_CS_CHAT_RES_SECTOR_MOVE;
	*SendBuff << SendType;
	*SendBuff << AccountNo;
	*SendBuff << wSectorX;
	*SendBuff << wSectorY;

	// 6) Ŭ�󿡰� ��Ŷ ������(��Ȯ���� NetServer�� ������ۿ� �ֱ�)
	SendPacket(SessionID, SendBuff);
}

// ä�� ������ ��û
//
// Parameter : SessionID, Packet
// return : ���� �� true
//		  : ���������� ���� ������ �� false
void CChatServer::Packet_Chat_Message(ULONGLONG SessionID, CProtocolBuff_Net* Packet)
{
	// 1) map���� ���� �˻�
	stPlayer* FindPlayer = FindPlayerFunc(SessionID);
	if (FindPlayer == nullptr)
		m_ChatDump->Crash();	

	// 2) ������
	INT64 AccountNo;
	*Packet >> AccountNo;

	WORD MessageLen;
	*Packet >> MessageLen;

	WCHAR Message[512];
	Packet->GetData((char*)Message, MessageLen);

	// ------------------- ����
	// 4) AccountNo üũ
	if (AccountNo != FindPlayer->m_i64AccountNo)
	{
		m_ChatNoError++;

		// �������̸� ������ ���� ������
		throw CException(_T("Packet_Chat_Message(). AccountNo Error"));
	}
	
	// 3) Ŭ���̾�Ʈ���� ���� ��Ŷ ���� (ä�� ������ ����)
	CProtocolBuff_Net* SendBuff = CProtocolBuff_Net::Alloc();

	WORD Type = en_PACKET_CS_CHAT_RES_MESSAGE;
	*SendBuff << Type;
	*SendBuff << AccountNo;
	SendBuff->PutData((char*)FindPlayer->m_tLoginID, 40);
	SendBuff->PutData((char*)FindPlayer->m_tNickName, 40);
	*SendBuff << MessageLen;
	SendBuff->PutData((char*)Message, MessageLen);

	// 4) �ֺ� �������� ä�� �޽��� ����
	// ��� �������� ������ (ä���� ���� ���� ����)
	SendPacket_Sector(FindPlayer->m_wSectorX, FindPlayer->m_wSectorY, SendBuff);
}


// �α��� ��û
//
// Parameter : SessionID, CProtocolBuff_Net*
// return : ����
void CChatServer::Packet_Chat_Login(ULONGLONG SessionID, CProtocolBuff_Net* Packet)
{
	// 1) map���� ���� �˻�
	stPlayer* FindPlayer = FindPlayerFunc(SessionID);
	if (FindPlayer == nullptr)
		m_ChatDump->Crash();

	// 2) ������
	INT64	AccountNo;
	*Packet >> AccountNo;
	FindPlayer->m_i64AccountNo = AccountNo;

	Packet->GetData((char*)FindPlayer->m_tLoginID, 40);
	Packet->GetData((char*)FindPlayer->m_tNickName, 40);
	Packet->GetData(FindPlayer->m_cToken, 64);

	// 3) ��ū �˻�
	

	// 4) Ŭ���̾�Ʈ���� ���� ��Ŷ ���� (�α��� ��û ����)
	CProtocolBuff_Net* SendBuff = CProtocolBuff_Net::Alloc();

	WORD SendType = en_PACKET_CS_CHAT_RES_LOGIN;
	*SendBuff << SendType;

	BYTE Status = 1;
	*SendBuff << Status;

	*SendBuff << AccountNo;

	// 6) Ŭ�󿡰� ��Ŷ ������(��Ȯ���� NetServer�� ������ۿ� �ֱ�)
	SendPacket(SessionID, SendBuff);
}



// ------------------------------------------------
// �ܺο��� ȣ���ϴ� �Լ�
// ------------------------------------------------

// ������
CChatServer::CChatServer()
	:CNetServer()
{
	// �Ұ� ����.
	// NetServer�� �����ڸ� ȣ��Ǹ� ��.
}

// �Ҹ���
CChatServer::~CChatServer()
{
	// ������ �������̶�� ��ž.
	if (GetServerState() == true)
		ServerStop();	
}

// ä�� ���� ���� �Լ�
// ���������� NetServer�� Start�� ���� ȣ��
//
// return false : ���� �߻� ��. �����ڵ� ���� �� false ����
// return true : ����
bool CChatServer::ServerStart()
{
	// ------------------- Config���� ����
	stConfigFile config;
	if (SetFile(&config) == false)
		return false;

	// ------------------- �α� ������ ���� ����
	cChatLibLog->SetDirectory(L"ChatServer");
	cChatLibLog->SetLogLeve((CSystemLog::en_LogLevel)config.LogLevel);


	// ------------------- ���� ���ҽ� �Ҵ�
	// �ϰ� TLS �޸�Ǯ �����Ҵ�
	m_MessagePool = new CMemoryPoolTLS<st_WorkNode>(0, false);

	// �÷��̾� ����ü TLS �޸�Ǯ �����Ҵ�	
	m_PlayerPool = new CMemoryPoolTLS<stPlayer>(0, false);

	// ������ ť �����Ҵ� (��Ʈ��ũ�� ���������� �ϰ� ������ ť)
	// ����� 0�� ������, UpdateThread���� ť�� ������� üũ�ϰ� ���� �����ϱ� ������.
	m_LFQueue = new CLF_Queue<st_WorkNode*>(0);

	// ������Ʈ ������ ����� �뵵 Event, ������Ʈ ������ ���� �뵵 Event
	// 
	// �ڵ� ���� Event 
	// ���� ���� �� non-signalled ����
	// �̸� ���� Event	
	UpdateThreadEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	UpdateThreadEXITEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

	// ------------------- ������Ʈ ������ ����
	hUpdateThraed = (HANDLE)_beginthreadex(NULL, 0, UpdateThread, this, 0, 0);
	   	

	// ------------------- �ݼ��� ����
	if (Start(config.BindIP, config.Port, config.CreateWorker, config.ActiveWorker, config.CreateAccept, config.Nodelay, config.MaxJoinUser,
		config.HeadCode, config.XORCode1, config.XORCode2) == false)
		return false;		

	// ���� ���� �α� ���		
	cChatLibLog->LogSave(L"ChatServer", CSystemLog::en_LogLevel::LEVEL_SYSTEM, L"ServerOpen...");

	return true;
}

// ä�� ���� ���� �Լ�
//
// Parameter : ����
// return : ����
void CChatServer::ServerStop()
{
	// �ݼ��� ��ž (����Ʈ, ��Ŀ ����)
	Stop();		

	// ������Ʈ ������ ���� ��ȣ
	SetEvent(UpdateThreadEXITEvent);

	// ������Ʈ ������ ���� ���
	if (WaitForSingleObject(hUpdateThraed, INFINITE) == WAIT_FAILED)
	{
		DWORD Error = GetLastError();
		printf("UpdateThread Exit Error!!! (%d) \n", Error);
	}

	// ------------- ��� ����
	// ���� ����.


	// ------------- ���ҽ� �ʱ�ȭ
	// ť ���� ��� ���, Free
	st_WorkNode* FreeNode;
	if (m_LFQueue->GetInNode() != 0)
	{
		if (m_LFQueue->Dequeue(FreeNode) == -1)
			m_ChatDump->Crash();

		m_MessagePool->Free(FreeNode);
	}


	// ���� list���� ��� ���� ����
	for (int y = 0; y < SECTOR_Y_COUNT; ++y)
	{
		for (int x = 0; x < SECTOR_X_COUNT; ++x)
		{
			m_listSecotr[y][x].clear();
		}
	}

	// Playermap���� ��� ���� ����
	auto itor = m_mapPlayer.begin();

	while (itor != m_mapPlayer.end())
	{
		// �޸�Ǯ�� ��ȯ
		m_PlayerPool->Free(itor->second);

		// map���� ����
		itor = m_mapPlayer.erase(itor);
	}

	// ť ��������.
	delete m_LFQueue;

	// ����ü �޽��� TLS �޸� Ǯ ��������
	delete m_MessagePool;

	// �÷��̾� ����ü TLS �޸�Ǯ ��������
	delete m_PlayerPool;

	// ������Ʈ ������ ������ �̺�Ʈ ����
	CloseHandle(UpdateThreadEvent);

	// ������Ʈ ������ ����� �̺�Ʈ ����
	CloseHandle(UpdateThreadEXITEvent);

	// ������Ʈ ������ �ڵ� ��ȯ
	CloseHandle(hUpdateThraed);
}


// ------------------------------------------------
// ��ӹ��� virtual �Լ�
// ------------------------------------------------

bool CChatServer::OnConnectionRequest(TCHAR* IP, USHORT port)
{
	// ����, IP�� Port�� �̻��� ��(������� �ؿ�?)�� false ����.
	// false�� ���ϵǸ� ������ �ȵȴ�.

	return true;
}


void CChatServer::OnClientJoin(ULONGLONG SessionID)
{
	// ȣ�� ���� : ������ ������ ���������� ���ӵǾ��� ��
	// ȣ�� ��ġ : NetServer�� ��Ŀ������
	// �ϴ� �ൿ : �������� ����ִ� ť��, ���� ���� �޽����� �ִ´�.

	// 1. �ϰ� Alloc
	st_WorkNode* NowMessage = m_MessagePool->Alloc();

	InterlockedIncrement(&g_ullUpdateStructCount);

	// 2. Ÿ�� �ֱ�
	NowMessage->m_wType = TYPE_JOIN;

	// 3.  ���� ID ä���
	NowMessage->m_ullSessionID = SessionID;	

	// 4. �޽����� ť�� �ִ´�.
	m_LFQueue->Enqueue(NowMessage);	

	// 5. �ڰ��ִ� Update�����带 �����.
	SetEvent(UpdateThreadEvent);
}


void CChatServer::OnClientLeave(ULONGLONG SessionID)
{
	// ȣ�� ���� : ������ �������� ���� ��
	// ȣ�� ��ġ : NetServer�� ��Ŀ������
	// �ϴ� �ൿ : �������� ����ִ� ť��, ���� ���� �޽����� �ִ´�.

	// 1. �ϰ� Alloc
	st_WorkNode* NowMessage = m_MessagePool->Alloc();

	InterlockedIncrement(&g_ullUpdateStructCount);

	// 2. Typeä���
	// ���� Ÿ���� [����, ����, ��Ŷ] �� 3 �� �� �ϳ��̴�.
	// ���⼱ ������ ����
	NowMessage->m_wType = TYPE_LEAVE;

	// 3. ���� ID ä���
	NowMessage->m_ullSessionID = SessionID;

	// 4. �޽����� ť�� �ִ´�.
	m_LFQueue->Enqueue(NowMessage);	

	// 5. �ڰ��ִ� Update�����带 �����.
	SetEvent(UpdateThreadEvent);
}


void CChatServer::OnRecv(ULONGLONG SessionID, CProtocolBuff_Net* Payload)
{
	// ȣ�� ���� : �������� ��Ŷ�� ���� ��
	// ȣ�� ��ġ : NetServer�� ��Ŀ������
	// �ϴ� �ൿ : �������� ����ִ� �޽��� ť��, �����͸� �ִ´�.	

	// 1. �ϰ� Alloc
	st_WorkNode* NowMessage = m_MessagePool->Alloc();

	InterlockedIncrement(&g_ullUpdateStructCount);

	// 2. Typeä���
	// ���� Ÿ���� [����, ����, ��Ŷ] �� 3 �� �� �ϳ��̴�.
	// ���⼱ ������ ��Ŷ.
	NowMessage->m_wType = TYPE_PACKET;

	// 3. ���� ID ä���
	NowMessage->m_ullSessionID = SessionID;

	// 4. ��Ŷ �ֱ�
	Payload->Add();
	NowMessage->m_pPacket = Payload;

	// 5. �ϰ��� ť�� �ִ´�.
	m_LFQueue->Enqueue(NowMessage);	

	// 6. �ڰ��ִ� Update�����带 �����.
	SetEvent(UpdateThreadEvent);
}


void CChatServer::OnSend(ULONGLONG SessionID, DWORD SendSize)
{}

void CChatServer::OnWorkerThreadBegin()
{}

void CChatServer::OnWorkerThreadEnd()
{}

void CChatServer::OnError(int error, const TCHAR* errorStr)
{	
	//// �α� ��� (�α� ���� : ����)
	//cChatLibLog->LogSave(L"ChatServer", CSystemLog::en_LogLevel::LEVEL_ERROR, L"%s (ErrorCode : %d)",
	//	errorStr, error);

	// ���� �ڵ忡 ���� ó��
	switch (error)
	{
	case (int)CNetServer::euError::NETWORK_LIB_ERROR__IOCP_ERROR:
		break;

	case (int)CNetServer::euError::NETWORK_LIB_ERROR__NOT_FIND_CLINET:
		break;

	case (int)CNetServer::euError::NETWORK_LIB_ERROR__SEND_QUEUE_SIZE_FULL:
		break;

	case (int)CNetServer::euError::NETWORK_LIB_ERROR__QUEUE_DEQUEUE_EMPTY:
		break;

	case (int)CNetServer::euError::NETWORK_LIB_ERROR__WSASEND_FAIL:
		break;

	case (int)CNetServer::euError::NETWORK_LIB_ERROR__A_THREAD_ABNORMAL_EXIT:
		break;

	case (int)CNetServer::euError::NETWORK_LIB_ERROR__A_THREAD_IOCPCONNECT_FAIL:
		break;

	case (int)CNetServer::euError::NETWORK_LIB_ERROR__W_THREAD_ABNORMAL_EXIT:
		break;

	case (int)CNetServer::euError::NETWORK_LIB_ERROR__WFSO_ERROR:
		break;

	case (int)CNetServer::euError::NETWORK_LIB_ERROR__IOCP_IO_FAIL:
		break;
		
	case (int)CNetServer::euError::NETWORK_LIB_ERROR__JOIN_USER_FULL:	
		cChatLibLog->LogSave(L"ChatServer", CSystemLog::en_LogLevel::LEVEL_DEBUG, L"%s (ErrorCode : %d)",
			errorStr, error);
		break;

		// (��Ʈ��ũ) ��� �ڵ� ����
	case (int)CNetServer::euError::NETWORK_LIB_ERROR__RECV_CODE_ERROR:
		m_HeadCodeError++;		
		break;

		// (��Ʈ��ũ) üũ�� ����
	case (int)CNetServer::euError::NETWORK_LIB_ERROR__RECV_CHECKSUM_ERROR:
		m_ChackSumError++;		
		break;

		// (��Ʈ��ũ) ����� Len����� ������������ ŭ.
	case (int)CNetServer::euError::NETWORK_LIB_ERROR__RECV_LENBIG_ERROR:	
		m_HeaderLenBig++;
		break;

	default:
		break;
	}		
}
