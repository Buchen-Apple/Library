#ifndef __CHAT_SERVER_H__
#define __CHAT_SERVER_H__

#include "NetworkLib\NetworkLib_NetServer.h"
#include "ObjectPool\Object_Pool_LockFreeVersion.h"
#include "CrashDump\CrashDump.h"

#include "LockFree_Queue\LockFree_Queue.h"

#include <map>
#include <list>


// 1�� ���� ���� ��
#define SECTOR_X_COUNT	50
#define SECTOR_Y_COUNT	50

using namespace Library_Jingyu;

////////////////////////////////////////////
// !! ä�ü��� !!
////////////////////////////////////////////
class CChatServer :public CNetServer
{
	// -------------------------------------
	// inner ����ü
	// -------------------------------------

	// �ϰ� ����ü
	struct stWork
	{
		ULONGLONG m_SessionID;
		CProtocolBuff_Net* m_Packet;
	};

	// ���� ������ ��� ����ü
	struct stSectorCheck
	{
		// �� ���� Sector Index�� ����Ǿ� �ִ���
		DWORD m_dwCount;

		// ������ X,Y �ε��� ��ǥ ����.
		POINT m_Sector[9];
	};

	// �÷��̾� ����ü
	struct stPlayer
	{
		// ���� ���� ��
		INT64 m_i64AccountNo;

		// ���� ID (�α��� �� ����ϴ� ID)
		TCHAR m_tLoginID[20];

		// �г���
		TCHAR m_tNickName[20];

		// ���� ��ǥ X,Y
		WORD m_wSectorX;
		WORD m_wSectorY;

		// ��ū (���� Ű)
		char m_cToken[64];

		// ���� ID (��Ʈ��ũ�� ��� �� ����ϴ� Ű��)
		ULONGLONG m_ullSessionID;
	};

private:
	// -------------------------------------
	// ��� ����
	// -------------------------------------

	// ����
	CCrashDump* m_ChatDump = CCrashDump::GetInstance();

	// ������ ����ü�� �ٷ� TLS�޸�Ǯ
	// stWork ����ü�� �ٷ��.
	CMemoryPoolTLS<stWork>* m_MessagePool;

	// �÷��̾� ����ü�� �ٷ�� map
	// Key : SessionID
	// Value : stPlayer*
	map<ULONGLONG, stPlayer*> m_mapPlayer;

	// �÷��̾� ����ü�� �ٷ� TLS�޸�Ǯ
	CMemoryPoolTLS<stPlayer>* m_PlayerPool;

	// ���� ����Ʈ
	list< stPlayer*> m_listSecotr[SECTOR_Y_COUNT][SECTOR_X_COUNT];

	// �޽����� ���� ������ ť
	// stWork�� �ּ�(8����Ʈ)�� �ٷ��.
	CLF_Queue<stWork*>* m_LFQueue;

	// ������Ʈ ������ ����� �뵵 �̺�Ʈ
	// !!!! �ܺο��� ���޹޴´� (�����ڿ���) !!!!
	HANDLE* m_UpdateThreadEvent;

private:
	// -------------------------------------
	// Ŭ���� ���ο��� ����ϴ� �Լ�
	// -------------------------------------

	// ���� ���� 9���� ���ϱ�
	// ���ڷ� ���� X,Y�� ��������, ���ڷ� ���� ����ü�� 9������ �־��ش�.
	//
	// Parameter : ���� SectorX, ���� SectorY, (out)&stSectorCheck
	// return : ����
	void GetSector(int SectorX, int SectorY, stSectorCheck* Sector);

	// ���ڷ� ���� 9�� ������ ��� ����(������ ��Ŷ�� ���� Ŭ�� ����)���� SendPacket ȣ��
	//
	// parameter : SessionID, ���� ����, ���� 9��
	// return : ����
	void SendPacket_Sector(ULONGLONG ClinetID, CProtocolBuff_Net* SendBuff, stSectorCheck* Sector);

	// ���� �̵���û ��Ŷ ó��
	// ó�� �� SendPacket() ȣ����� ó��.
	//
	// Parameter : SessionID, Packet
	// return : map���� ���� �˻� ���� �� false
	bool Packet_Sector_Move(ULONGLONG SessionID, CProtocolBuff_Net* Packet);

	// ä�� ������ ��û
	//
	// Parameter : SessionID, Packet
	// return : map���� ���� �˻� ���� �� false
	bool Packet_Chat_Message(ULONGLONG SessionID, CProtocolBuff_Net* Packet);


private:
	// -------------------------------------
	// NetServer�� �����Լ�
	// -------------------------------------

	virtual bool OnConnectionRequest(TCHAR* IP, USHORT port);

	virtual void OnClientJoin(ULONGLONG ClinetID);

	virtual void OnClientLeave(ULONGLONG ClinetID);

	virtual void OnRecv(ULONGLONG ClinetID, CProtocolBuff_Net* Payload);

	virtual void OnSend(ULONGLONG ClinetID, DWORD SendSize);

	virtual void OnWorkerThreadBegin();

	virtual void OnWorkerThreadEnd();

	virtual void OnError(int error, const TCHAR* errorStr);

public:
	// -------------------------------------
	// Ŭ���� �ܺο��� ��� ������ �Լ�
	// -------------------------------------
	// ������
	//
	// Parameter : ������Ʈ �����带 ���� �� ����� �̺�Ʈ
	CChatServer(HANDLE* UpdateThreadEvent);

	//�Ҹ���
	~CChatServer();

	// ��Ŷ ó�� �Լ�
	void PacketHandling();

	// ó���� �ϰ��� �ִ��� üũ�ϴ� �Լ�
	// ���������δ� QueueSize�� üũ�ϴ� ��.
	LONG WorkCheck();

};



#endif // !__CHAT_SERVER_H__
