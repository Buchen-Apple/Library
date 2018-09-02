#include "pch.h"
#include "ChatServer.h"

#include "CommonProtocol.h"

#include <strsafe.h>


// ���� ������ �˷��ִ� �������� Type
#define JOIN_USER_PROTOCOL_TYPE		15000

// ���� ������ �˷��ִ� �������� Type
#define LEAVE_USER_PROTOCOL_TYPE	15001



// -------------------------------------
// Ŭ���� ���ο��� ����ϴ� �Լ�
// -------------------------------------

// Player ���� �ڷᱸ����, ���� �߰�
// ���� map���� ������
// 
// Parameter : SessionID, stPlayer*
// return : �߰� ���� ��, true
//		  : SessionID�� �ߺ��� ��(�̹� �������� ����) false
bool CChatServer::InsertPlayerFunc(ULONGLONG SessionID, stPlayer* insertPlayer)
{
	// map�� �߰�
	auto ret = m_mapPlayer.insert(pair<ULONGLONG, stPlayer*>(SessionID, insertPlayer));

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
		return false;

	// 2) �ʿ��� ����
	m_mapPlayer.erase(FindPlayer);

	return FindPlayer->second;
}

// ���� ���� 9���� ���ϱ�
// ���ڷ� ���� X,Y�� ��������, ���ڷ� ���� ����ü�� 9������ �־��ش�.
//
// Parameter : ���� SectorX, ���� SectorY, (out)&stSectorCheck
// return : ����
void CChatServer::GetSector(int SectorX, int SectorY, stSectorCheck* Sector)
{
	int iCurX, iCurY;

	SectorX--;
	SectorY--;

	Sector->m_dwCount = 0;

	for (iCurY = 0; iCurY < 3; iCurY++)
	{
		if (SectorY + iCurY < 0 || SectorY + iCurY >= SECTOR_Y_COUNT)
			continue;

		for (iCurX = 0; iCurX < 3; iCurX++)
		{
			if (SectorX + iCurX < 0 || SectorX + iCurX >= SECTOR_X_COUNT)
				continue;

			Sector->m_Sector[Sector->m_dwCount].x = SectorX + iCurX;
			Sector->m_Sector[Sector->m_dwCount].y = SectorY + iCurY;
			Sector->m_dwCount++;

		}
	}
}

// ���ڷ� ���� 9�� ������ ��� ����(������ ��Ŷ�� ���� Ŭ�� ����)���� SendPacket ȣ��
//
// parameter : SessionID, ���� ����, ���� 9��
// return : ����
void CChatServer::SendPacket_Sector(ULONGLONG SessionID, CProtocolBuff_Net* SendBuff, stSectorCheck* Sector)
{
	// 1. Sector���� �����鿡�� ���ڷ� ���� ��Ŷ�� �ִ´�.
	DWORD dwCount = Sector->m_dwCount;
	for (DWORD i = 0; i < dwCount; ++i)
	{
		// 2. itor�� "i" ��° ������ ����Ʈ�� ����Ų��.
		auto itor = m_listSecotr[Sector->m_Sector[i].y][Sector->m_Sector[i].x].begin();
		auto Enditor = m_listSecotr[Sector->m_Sector[i].y][Sector->m_Sector[i].x].end();

		for (; itor != Enditor; ++itor)
		{
			// SendPacket
			SendPacket(SessionID, SendBuff);
		}
	}
}







// -------------------------------------
// Ŭ���� ���ο����� ����ϴ� ��Ŷ ó�� �Լ�
// -------------------------------------

// ���� �̵���û ��Ŷ ó��
//
// Parameter : SessionID, Packet
// return : ���� �� true
//		  : ���������� ���� ������ �� false
bool CChatServer::Packet_Sector_Move(ULONGLONG SessionID, BINGNODE* Packet)
{
	// 1) ���� �ʿ��� ������ ����ȯ
	st_Protocol_CS_CHAT_REQ_SECTOR_MOVE* NowPacket = (st_Protocol_CS_CHAT_REQ_SECTOR_MOVE*)Packet;

	// 2) map���� ���� �˻�
	stPlayer* FindPlayer = FindPlayerFunc(SessionID);
	if (FindPlayer == nullptr)
	{
		printf("Not Find Player!!\n");
		return false;
	}

	// 3) ������ ���� ���� ����
	// ���� ���Ϳ��� ���� ����
	m_listSecotr[FindPlayer->m_wSectorY][FindPlayer->m_wSectorX].remove(FindPlayer);

	// ���� ����
	WORD wSectorX = NowPacket->SectorX;
	WORD wSectorY = NowPacket->SectorY;

	FindPlayer->m_wSectorX = wSectorX;
	FindPlayer->m_wSectorY = wSectorY;

	// ���ο� ���Ϳ� ���� �߰�
	m_listSecotr[wSectorY][wSectorX].push_back(FindPlayer);

	// 4) Ŭ���̾�Ʈ���� ���� ��Ŷ ���� (���� �̵� ���)
	st_Protocol_CS_CHAT_RES_SECTOR_MOVE stSend;

	// Ÿ��, AccountNo, SecotrX, SecotrY
	stSend.Type = en_PACKET_CS_CHAT_RES_SECTOR_MOVE;
	stSend.AccountNo = NowPacket->AccountNo;
	stSend.SectorX = wSectorX;
	stSend.SectorY = wSectorY;

	CProtocolBuff_Net* SendBuff = CProtocolBuff_Net::Alloc();

	SendBuff->PutData((const char*)&stSend, sizeof(st_Protocol_CS_CHAT_RES_SECTOR_MOVE));

	// 5) Ŭ�󿡰� ��Ŷ ������(��Ȯ���� NetServer�� ������ۿ� �ֱ�)
	SendPacket(SessionID, SendBuff);

	return true;

}

// ä�� ������ ��û
//
// Parameter : SessionID, Packet
// return : ���� �� true
//		  : ���������� ���� ������ �� false
bool CChatServer::Packet_Chat_Message(ULONGLONG SessionID, BINGNODE* Packet)
{
	// 1) ���� �ʿ��� ������ ����ȯ
	st_Protocol_CS_CHAT_REQ_MESSAGE* NowPacket = (st_Protocol_CS_CHAT_REQ_MESSAGE*)Packet;

	// 2) �޽��� �������� �ι��� �ֱ�.
	// NowPacket->Message[NowPacket->MessageLen] = L'\0';

	// 3) map���� ���� �˻�
	stPlayer* FindPlayer = FindPlayerFunc(SessionID);
	if (FindPlayer == nullptr)
	{
		printf("Not Find Player!!\n");
		return false;
	}

	// 4) Ŭ���̾�Ʈ���� ���� ��Ŷ ����. (ä�ú����� ����)
	BINGNODE stSend;
	CProtocolBuff_Net* SendBuff = CProtocolBuff_Net::Alloc();

	// Ÿ��, AccountNo, ID, Nickname, MessageLen, Message
	stSend.Type = en_PACKET_CS_CHAT_RES_MESSAGE;
	stSend.AccountNo = NowPacket->AccountNo;

	StringCbCopy(stSend.ID, 20, FindPlayer->m_tLoginID);
	StringCbCopy(stSend.Nickname, 20, FindPlayer->m_tNickName);

	stSend.MessageLen = NowPacket->MessageLen;
	StringCbCopy(stSend.Message, NowPacket->MessageLen, NowPacket->Message);

	SendBuff->PutData((const char*)&stSend, sizeof(BINGNODE));

	// 5) �ش� ������ �ֺ� 9�� ���� ����.
	stSectorCheck SecCheck;
	GetSector(FindPlayer->m_wSectorX, FindPlayer->m_wSectorY, &SecCheck);

	// 6) �ֺ� �������� ä�� �޽��� ����
	// ��� �������� ������ (ä���� ���� ���� ����)
	SendPacket_Sector(SessionID, SendBuff, &SecCheck);

	return true;
}

// Accept ���� (ä�� ������ ����)
// ��Ʈ��ũ ����� �ƴϸ�, Net�������� Chat ������ �˷��ش�.
// 
// Parameter : SessionID, Packet
// return : ���� �� true
//		  : �̹� �������� ������� false
bool CChatServer::Packet_Chat_Join(ULONGLONG SessionID, BINGNODE* Packet)
{
	// 1) ���� ���ϴ� ������ ����ȯ
	st_Protocol_NetChat_OnClientJoin* NowPacket = (st_Protocol_NetChat_OnClientJoin*)Packet;

	// 2) Player Alloc()
	stPlayer* JoinPlayer = m_PlayerPool->Alloc();

	// 3) SessionID ����
	JoinPlayer->m_ullSessionID = SessionID;

	// 4) Player ���� �ڷᱸ���� ���� �߰�
	if (InsertPlayerFunc(SessionID, JoinPlayer) == false)
	{
		printf("duplication SessionID!!\n");
		return false;
	}

	return true;
}

// ���� ���� (ä�� �������� ����)
// ��Ʈ��ũ ����� �ƴϸ�, Net�������� Chat ������ �˷��ش�.
// 
// Parameter : SessionID, Packet
// return : �̹� ���������� �� false
bool CChatServer::Packet_Chat_Leave(ULONGLONG SessionID, BINGNODE* Packet)
{
	// 1) ���� ���ϴ� ������ ����ȯ
	st_Protocol_NetChat_OnClientLeave* NowPacket = (st_Protocol_NetChat_OnClientLeave*)Packet;

	// 2) Player �ڷᱸ������ ����
	stPlayer* ErasePlayer = ErasePlayerFunc(SessionID);
	if (ErasePlayer == nullptr)
	{
		printf("Not Find Player!!\n");
		return false;
	}

	// 3) Player Free()
	m_PlayerPool->Free(ErasePlayer);	

	return true;
}





// ------------------------------------------------
// �ܺο��� ȣ���ϴ� �Լ�
// ------------------------------------------------

// ������
//
// Parameter : ������Ʈ �����带 ���� �� ����� �̺�Ʈ
CChatServer::CChatServer(HANDLE* UpdateThreadEvent)
{
	// ����ü �޽��� TLS �޸�Ǯ �����Ҵ�
	// �� 100���� ûũ. (1���� 200���� �����͸� �ٷ��, 200*100 = 20000. �� 20000���� ����ü �޽��� ��� ����)
	m_MessagePool = new CMemoryPoolTLS<BINGNODE>(100, false);

	// �÷��̾� ����ü TLS �޸�Ǯ �����Ҵ�
	// �� 100���� ûũ. (1���� 200���� �����͸� �ٷ��, 200*100 = 20000. �� 20000���� �÷��̾� ����ü ��� ����)
	m_PlayerPool = new CMemoryPoolTLS<stPlayer>(100, false);

	// ������ ť �����Ҵ� (��Ʈ��ũ�� ���������� �ϰ� ������ ť)
	// ����� 0�� ������, UpdateThread���� ť�� ������� üũ�ϰ� ���� �����ϱ� ������.
	m_LFQueue = new CLF_Queue<BINGNODE*>(0);

	// ������Ʈ������ ����� �뵵 �̺�Ʈ ���� �� ����� ����
	// 
	// �޴��� ���� Event 
	// ���� ���� �� non-signalled ���¤�
	// �̸� ���� Event
	*UpdateThreadEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	m_UpdateThreadEvent = UpdateThreadEvent;	
}

// �Ҹ���
//
// Parameter : Event ��ü �ּ�
CChatServer::~CChatServer()
{
	// ť�� ���� ��������. �� �ȿ�, m_MessagePool�� �����ϴ� �����Ͱ� ���� �� �ֱ� ������
	delete m_LFQueue;

	// ����ü �޽��� TLS �޸� Ǯ ��������
	delete m_MessagePool;

	// �÷��̾� ����ü TLS �޸�Ǯ ��������
	delete m_PlayerPool;

	// ���� ��ž.
	Stop();
}

// ��Ŷ ó�� �Լ�
// ���ο��� ���� ��Ŷ ó��
// 
// Parameter : ����
// return : ����
void CChatServer::PacketHandling()
{
	// 1. ť���� �ϰ� 1�� ������
	BINGNODE* NowWork;

	if (m_LFQueue->Dequeue(NowWork) == -1)
		m_ChatDump->Crash();

	// 2. Type�� ���� ���� ó��
	// �б� ���� �������� ���̱� ����, ���� ���� ȣ��� �� �� �ͺ��� switch case�� ��ܿ� ��ġ.
	switch (NowWork->Type)
	{
		// ä�ü��� ä�ú����� ��û
	case en_PACKET_CS_CHAT_REQ_MESSAGE:
		Packet_Chat_Message(NowWork->SessionID, NowWork);

		break;

		// ä�ü��� ���� �̵� ��û
	case en_PACKET_CS_CHAT_REQ_SECTOR_MOVE:
		Packet_Sector_Move(NowWork->SessionID, NowWork);

		break;		

		// Accept ����
	case JOIN_USER_PROTOCOL_TYPE:
		Packet_Chat_Join(NowWork->SessionID, NowWork);

		break;

		// �������� Leave
	case LEAVE_USER_PROTOCOL_TYPE:
		Packet_Chat_Leave(NowWork->SessionID, NowWork);
		break;

		// ��Ʈ��Ʈ
	case en_PACKET_CS_CHAT_REQ_HEARTBEAT:
		break;

	default:
		printf("Type Error!!! (%d)\n", NowWork->Type);
		break;
	}

	// 3. �ϰ��� �޸�Ǯ�� Free
	m_MessagePool->Free(NowWork);

}

// ó���� �ϰ��� �ִ��� üũ�ϴ� �Լ�
// ���������δ� QueueSize�� üũ�ϴ� ��.
//
// Parameter : ����
// return : ���� �ϰ� ��
LONG CChatServer::WorkCheck()
{
	return m_LFQueue->GetInNode();
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
	st_Protocol_NetChat_OnClientJoin* NowMessage = (st_Protocol_NetChat_OnClientJoin*)m_MessagePool->Alloc();

	// 2.  ���� ID ä���
	NowMessage->SessionID = SessionID;

	// 3. Ÿ�� �ֱ�
	NowMessage->Type = JOIN_USER_PROTOCOL_TYPE;

	// 4. �޽����� ť�� �ִ´�.
	m_LFQueue->Enqueue((BINGNODE*)NowMessage);

	// 5. �ڰ��ִ� Update�����带 �����.
	SetEvent(*m_UpdateThreadEvent);

}


void CChatServer::OnClientLeave(ULONGLONG SessionID)
{
	// ȣ�� ���� : ������ �������� ���� ��
	// ȣ�� ��ġ : NetServer�� ��Ŀ������
	// �ϴ� �ൿ : �������� ����ִ� ť��, ���� ���� �޽����� �ִ´�.

		// 1. �ϰ� Alloc
	st_Protocol_NetChat_OnClientLeave* NowMessage = (st_Protocol_NetChat_OnClientLeave*)m_MessagePool->Alloc();

	// 2.  ���� ID ä���
	NowMessage->SessionID = SessionID;

	// 3. Ÿ�� �ֱ�
	NowMessage->Type = LEAVE_USER_PROTOCOL_TYPE;

	// 4. �޽����� ť�� �ִ´�.
	m_LFQueue->Enqueue((BINGNODE*)NowMessage);

	// 5. �ڰ��ִ� Update�����带 �����.
	SetEvent(*m_UpdateThreadEvent);

}


void CChatServer::OnRecv(ULONGLONG SessionID, CProtocolBuff_Net* Payload)
{
	// ȣ�� ���� : �������� ��Ŷ�� ���� ��
	// ȣ�� ��ġ : NetServer�� ��Ŀ������
	// �ϴ� �ൿ : �������� ����ִ� �޽��� ť��, �����͸� �ִ´�.	

	// 1. �ϰ� Alloc
	BINGNODE* NowMessage = m_MessagePool->Alloc();

	// 2. ���� ID ä���
	NowMessage->SessionID = SessionID;

	// 3. Copy
	// � �޽����� ���� �� 8����Ʈ�� SessioniID�� ����ֱ� ������ 8����Ʈ �ں��� ����.
	int GetUseSize = Payload->GetUseSize();
	memcpy_s(&NowMessage[8], GetUseSize, Payload->GetBufferPtr(), GetUseSize);

	// 4.�޽����� ť�� �ִ´�.
	m_LFQueue->Enqueue(NowMessage);

	// 5. �ڰ��ִ� Update�����带 �����.
	SetEvent(*m_UpdateThreadEvent);
}


void CChatServer::OnSend(ULONGLONG SessionID, DWORD SendSize)
{}

void CChatServer::OnWorkerThreadBegin()
{}

void CChatServer::OnWorkerThreadEnd()
{}

void CChatServer::OnError(int error, const TCHAR* errorStr)
{}
