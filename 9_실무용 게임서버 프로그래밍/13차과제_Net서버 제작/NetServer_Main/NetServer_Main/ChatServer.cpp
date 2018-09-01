#include "pch.h"
#include "ChatServer.h"

#include "CommonProtocol.h"
#include "ProtocolStruct.h"

#include <strsafe.h>


// ���� ������ �˷��ִ� �������� Type
#define JOIN_USER_PROTOCOL_TYPE		15000

// ���� ������ �˷��ִ� �������� Type
#define LEAVE_USER_PROTOCOL_TYPE	15001

// ------------------------------------------------
// ����ü & Ŭ���� ��� �Լ�
// ------------------------------------------------

// ������
//
// Parameter : ������Ʈ �����带 ���� �� ����� �̺�Ʈ
CChatServer::CChatServer(HANDLE* UpdateThreadEvent)
{
	// ����ü �޽��� TLS �޸�Ǯ �����Ҵ�
	m_MessagePool = new CMemoryPoolTLS<stWork>(100, false);

	// �÷��̾� ����ü TLS �޸�Ǯ �����Ҵ�
	m_PlayerPool = new CMemoryPoolTLS<stPlayer>(100, false);

	// ������ ť �����Ҵ� (��Ʈ��ũ�� ���������� �ϰ� ������ ť)
	// ����� 0�� ������, UpdateThread���� ť�� ������� üũ�ϰ� ���� �����ϱ� ������.
	m_LFQueue = new CLF_Queue<stWork*>(0);

	// ������Ʈ������ ����� �뵵 �̺�Ʈ ���� �� ����� ����
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
	// 1. ť���� ������ 1�� ������
	stWork* NowWork;

	if (m_LFQueue->Dequeue(NowWork) == -1)
		m_ChatDump->Crash();

	// 2. SessionID�� Type�� ���ڷ� �޾Ƶд�.
	ULONGLONG ullSessionID = NowWork->m_SessionID;

	CProtocolBuff_Net* localPacket = NowWork->m_Packet;
	WORD wType;
	localPacket->PutData((char*)&wType, 2);

	// 3. Type�� ���� ���� ó��
	switch (wType)
	{
		// ä�ü��� ���� �̵� ��û
	case en_PACKET_CS_CHAT_REQ_SECTOR_MOVE:
		Packet_Sector_Move(NowWork->m_SessionID, localPacket);		

		break;

		// ä�ü��� ä�ú����� ��û
	case en_PACKET_CS_CHAT_REQ_MESSAGE:
		Packet_Chat_Message(NowWork->m_SessionID, localPacket);

		break;

		// ��Ʈ��Ʈ
	case en_PACKET_CS_CHAT_REQ_HEARTBEAT:
		break;

	default:
		break;
	}

	// 4. �� �� �޽��� Free
	CProtocolBuff_Net::Free(localPacket);

	// 5. �ϰ� ����ü Free
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

// ���� �̵���û ��Ŷ ó��
//
// Parameter : SessionID, Packet
// return : map���� ���� �˻� ���� �� false
bool CChatServer::Packet_Sector_Move(ULONGLONG SessionID, CProtocolBuff_Net* Packet)
{
	// 1) ��Ŷ�� st_Protocol_CS_CHAT_REQ_SECTOR_MOVE ���·� �޾ƿ���
	st_Protocol_CS_CHAT_REQ_SECTOR_MOVE NowMessage;
	Packet->PutData((const char*)&NowMessage, Packet->GetUseSize());

	// 2) map���� ���� �˻�
	auto FindPlayer = m_mapPlayer.find(SessionID);
	if (FindPlayer == m_mapPlayer.end())
	{
		printf("Not Find Player!!\n");
		return false;
	}

	// 3) ������ ���� ���� ����
	FindPlayer->second->m_wSectorX = NowMessage.SectorX;
	FindPlayer->second->m_wSectorY = NowMessage.SectorY;

	// 4) Ŭ���̾�Ʈ���� ���� ��Ŷ ���� (���� �̵� ���)
	st_Protocol_CS_CHAT_RES_SECTOR_MOVE stSend;

	// Ÿ��, AccountNo, SecotrX, SecotrY
	stSend.Type = en_PACKET_CS_CHAT_RES_SECTOR_MOVE;
	stSend.AccountNo = NowMessage.AccountNo;
	stSend.SectorX = NowMessage.SectorX;
	stSend.SectorY = NowMessage.SectorY;

	CProtocolBuff_Net* SendBuff = CProtocolBuff_Net::Alloc();

	SendBuff->PutData((const char*)&stSend, sizeof(st_Protocol_CS_CHAT_RES_SECTOR_MOVE));

	// 5) Ŭ�󿡰� ��Ŷ ������(��Ȯ���� NetServer�� ������ۿ� �ֱ�)
	if (SendPacket(SessionID, SendBuff) == false)
	{
		// ���⼭ false�� �߸� �ʿ��� ���� ����?
		m_mapPlayer.erase(SessionID);

		// �׸��� Player ����ü�� �޸�Ǯ�� ��ȯ�Ѵ�.
		m_PlayerPool->Free(FindPlayer->second);
	}

	return true;

}


// ä�� ������ ��û
//
// Parameter : SessionID, Packet
// return : map���� ���� �˻� ���� �� false
bool CChatServer::Packet_Chat_Message(ULONGLONG SessionID, CProtocolBuff_Net* Packet)
{
	// 1) ��Ŷ�� st_Protocol_CS_CHAT_REQ_MESSAGE ���·� �޾ƿ���
	st_Protocol_CS_CHAT_REQ_MESSAGE NowMessage;
	Packet->PutData((const char*)&NowMessage, Packet->GetUseSize());

	NowMessage.Message[NowMessage.MessageLen] = (WCHAR)L'\0';

	// 2) map���� ���� �˻�
	auto FindPlayer = m_mapPlayer.find(SessionID);
	if (FindPlayer == m_mapPlayer.end())
	{
		printf("Not Find Player!!\n");
		return false;
	}

	// 3) Ŭ���̾�Ʈ���� ���� ��Ŷ ����. (ä�ú����� ����)
	st_Protocol_CS_CHAT_RES_MESSAGE stSend;

	// Ÿ��, AccountNo, ID, Nickname, MessageLen, Message
	stSend.Type = en_PACKET_CS_CHAT_RES_MESSAGE;
	stSend.AccountNo = NowMessage.AccountNo;

	StringCbCopy(stSend.ID, 20, FindPlayer->second->m_tLoginID);
	StringCbCopy(stSend.Nickname, 20, FindPlayer->second->m_tNickName);

	stSend.MessageLen = NowMessage.MessageLen;
	StringCbCopy(stSend.Message, NowMessage.MessageLen, NowMessage.Message);

	CProtocolBuff_Net* SendBuff = CProtocolBuff_Net::Alloc();

	SendBuff->PutData((const char*)&stSend, sizeof(st_Protocol_CS_CHAT_RES_MESSAGE));

	// 4) �ش� ������ �ֺ� 9�� ���� ����.
	stSectorCheck SecCheck;
	GetSector(FindPlayer->second->m_wSectorX, FindPlayer->second->m_wSectorY, &SecCheck);

	// 5) �ֺ� �������� ä�� �޽��� ����
	// ��� �������� ������ (ä���� ���� ���� ����)
	SendPacket_Sector(SessionID, SendBuff, &SecCheck);

	return true;
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
void CChatServer::SendPacket_Sector(ULONGLONG ClinetID, CProtocolBuff_Net* SendBuff, stSectorCheck* Sector)
{
	// 1. Sector���� �����鿡�� ���ڷ� ���� ��Ŷ�� �ִ´�.
	for (DWORD i = 0; i < Sector->m_dwCount; ++i)
	{
		// 2. itor�� "i" ��° ������ ����Ʈ�� ����Ų��.
		auto itor = m_listSecotr[Sector->m_Sector[i].y][Sector->m_Sector[i].x].begin();

		for (; itor != m_listSecotr[Sector->m_Sector[i].y][Sector->m_Sector[i].x].end(); ++itor)
		{
			// SendPacket
			SendPacket(ClinetID, SendBuff);
		}
	}
}



// ------------------------------------------------
// ä�ü����� �����Լ�
// ------------------------------------------------

bool CChatServer::OnConnectionRequest(TCHAR* IP, USHORT port)
{
	// ����, IP�� Port�� �̻��� ��(������� �ؿ�?)�� false ����.
	// false�� ���ϵǸ� ������ �ȵȴ�.

	return true;
}

void CChatServer::OnClientJoin(ULONGLONG ClinetID)
{
	// ȣ�� ���� : ������ ���� ���������� ���ӵǾ��� ��
	// ȣ�� ��ġ : NetServer�� ��Ŀ������
	// �ϴ� �ൿ : �������� ����ִ� ť��, �����͸� �ִ´�.

	// 1. �޽��� ����ü ���� �޸�Ǯ���� Alloc
	stWork* NowMessage = m_MessagePool->Alloc();

	// 2. ��Ŷ Alloc
	CProtocolBuff_Net* Temp = CProtocolBuff_Net::Alloc();

	// 3. ���� ID ä���
	NowMessage->m_SessionID = ClinetID;

	// 4. Ÿ�� �ֱ�
	WORD Type = JOIN_USER_PROTOCOL_TYPE;
	Temp->PutData((const char*)&Type, 2);

	// 5. �޽����� ����
	NowMessage->m_Packet = Temp;

	// 6. �޽����� ť�� �ִ´�.
	m_LFQueue->Enqueue((stWork*)NowMessage);

	// 7. �ڰ��ִ� Update�����带 �����.
	SetEvent(*m_UpdateThreadEvent);

}


void CChatServer::OnClientLeave(ULONGLONG ClinetID)
{
	// ȣ�� ���� : ������ �������� ���� ��
	// ȣ�� ��ġ : NetServer�� ��Ŀ������
	// �ϴ� �ൿ : �������� ����ִ� ť��, �����͸� �ִ´�.


}

void CChatServer::OnRecv(ULONGLONG ClinetID, CProtocolBuff_Net* Payload)
{
	// ȣ�� ���� : �������� ��Ŷ�� ���� ��
	// ȣ�� ��ġ : NetServer�� ��Ŀ������
	// �ϴ� �ൿ : �������� ����ִ� �޽��� ť��, �����͸� �ִ´�.	

	// 1. �޽��� ����ü ���� �޸�Ǯ���� Alloc
	stWork* NowMessage = m_MessagePool->Alloc();

	// 2. ���� ID ä���
	NowMessage->m_SessionID = ClinetID;

	// 3. NowMessage ����
	// refCount 1 ���� , ��Ŷ ����
	Payload->Add();
	NowMessage->m_Packet = Payload;

	// 4.�޽����� ť�� �ִ´�.
	m_LFQueue->Enqueue(NowMessage);

	// 5. �ڰ��ִ� Update�����带 �����.
	SetEvent(*m_UpdateThreadEvent);
}


void CChatServer::OnSend(ULONGLONG ClinetID, DWORD SendSize)
{}

void CChatServer::OnWorkerThreadBegin()
{}

void CChatServer::OnWorkerThreadEnd()
{}

void CChatServer::OnError(int error, const TCHAR* errorStr)
{}
