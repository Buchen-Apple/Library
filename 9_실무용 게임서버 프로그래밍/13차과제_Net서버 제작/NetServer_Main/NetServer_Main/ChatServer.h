#ifndef __CHAT_SERVER_H__
#define __CHAT_SERVER_H__

#include "NetworkLib\NetworkLib_NetServer.h"
#include "ObjectPool\Object_Pool_LockFreeVersion.h"
#include "CrashDump\CrashDump.h"

#include "LockFree_Queue\LockFree_Queue.h"
//#include "ProtocolStruct.h"

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
	struct st_WorkNode
	{
		WORD m_wType;
		ULONGLONG m_ullSessionID;
		CProtocolBuff_Net* m_pPacket;
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
		WCHAR m_tLoginID[20];

		// �г���
		WCHAR m_tNickName[20];

		// ���� ��ǥ X,Y
		// ���ʿ��� ��� 12345
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
	// !! ĳ�� ��Ʈ�� ����� ���� ���Ǵ� �͵鳢�� �������Ѽ� ����
	// !! ��κ�, ���� �� �����ϴ�(Write)�ϴ� �������̱� ������ �б�����, �������� �������� ������ ����.

	// ����
	CCrashDump* m_ChatDump = CCrashDump::GetInstance();

	// �޽��� ����ü�� �ٷ� TLS�޸�Ǯ
	// �ϰ� ����ü�� �ٷ��.
	CMemoryPoolTLS<st_WorkNode>* m_MessagePool;

	// �޽����� ���� ������ ť
	// �ּ�(8����Ʈ)�� �ٷ��.
	CLF_Queue<st_WorkNode*>* m_LFQueue;

	// �÷��̾� ����ü�� �ٷ� TLS�޸�Ǯ
	CMemoryPoolTLS<stPlayer>* m_PlayerPool;

	// �÷��̾� ����ü�� �ٷ�� map
	// Key : SessionID
	// Value : stPlayer*
	map<ULONGLONG, stPlayer*> m_mapPlayer;

	// ���� ����Ʈ
	list< stPlayer*> m_listSecotr[SECTOR_Y_COUNT][SECTOR_X_COUNT];	
	
	// ������Ʈ ������ �ڵ�
	HANDLE hUpdateThraed;

	// ������Ʈ ������ ����� �뵵 Event
	HANDLE UpdateThreadEvent;

	// ������Ʈ ������ ���� �뵵 Event
	HANDLE UpdateThreadEXITEvent;
	   	  

private:
	// -------------------------------------
	// Ŭ���� ���ο����� ����ϴ� ��� �Լ�
	// -------------------------------------
	
	// Player ���� �ڷᱸ����, ���� �߰�
	// ���� map���� ������
	// 
	// Parameter : SessionID, stPlayer*
	// return : �߰� ���� ��, true
	//		  : SessionID�� �ߺ��� ��(�̹� �������� ����) false
	bool InsertPlayerFunc(ULONGLONG SessionID, stPlayer* insertPlayer);

	// Player ���� �ڷᱸ������, ���� �˻�
	// ���� map���� ������
	// 
	// Parameter : SessionID
	// return : �˻� ���� ��, stPalyer*
	//		  : �˻� ���� �� nullptr
	stPlayer* FindPlayerFunc(ULONGLONG SessionID);

	// Player ���� �ڷᱸ������, ���� ���� (�˻� �� ����)
	// ���� map���� ������
	// 
	// Parameter : SessionID
	// return : ���� ��, ���ŵ� ���� stPalyer*
	//		  : �˻� ���� ��(���������� ���� ����) nullptr
	stPlayer* ErasePlayerFunc(ULONGLONG SessionID);

	// ���� ���� 9���� ���ϱ�
	// ���ڷ� ���� X,Y�� ��������, ���ڷ� ���� ����ü�� 9������ �־��ش�.
	//
	// Parameter : ���� SectorX, ���� SectorY, (out)&stSectorCheck
	// return : ����
	void GetSector(int SectorX, int SectorY, stSectorCheck* Sector);

	// ���ڷ� ���� 9�� ������ ��� ����(������ ��Ŷ�� ���� Ŭ�� ����)���� SendPacket ȣ��
	//
	// parameter : ���� ����, ���� 9��
	// return : ����
	void SendPacket_Sector(CProtocolBuff_Net* SendBuff, stSectorCheck* Sector);

	// ������Ʈ ������
	static UINT	WINAPI	UpdateThread(LPVOID lParam);



	// -------------------------------------
	// Ŭ���� ���ο����� ����ϴ� ��Ŷ ó�� �Լ�
	// -------------------------------------

	// ���� ��Ŷó�� �Լ�
	// OnClientJoin���� ȣ��
	// 
	// Parameter : SessionID
	// return : ����
	void Packet_Join(ULONGLONG SessionID);

	// ���� ��Ŷó�� �Լ�
	// OnClientLeave���� ȣ��
	// 
	// Parameter : SessionID
	// return : ����
	void Packet_Leave(ULONGLONG SessionID);

	// �Ϲ� ��Ŷó�� �Լ�
	// 
	// Parameter : SessionID, CProtocolBuff_Net*
	// return : ����
	void Packet_Normal(ULONGLONG SessionID, CProtocolBuff_Net* Packet);
		


	// -------------------------------------
	// '�Ϲ� ��Ŷ ó�� �Լ�'���� ó���Ǵ� �Ϲ� ��Ŷ��
	// -------------------------------------

	// ���� �̵���û ��Ŷ ó��
	//
	// Parameter : SessionID, CProtocolBuff_Net*
	// return : ����
	void Packet_Sector_Move(ULONGLONG SessionID, CProtocolBuff_Net* Packet);

	// ä�� ������ ��û
	//
	// Parameter : SessionID, CProtocolBuff_Net*
	// return : ����
	void Packet_Chat_Message(ULONGLONG SessionID, CProtocolBuff_Net* Packet);

	// �α��� ��û
	//
	// Parameter : SessionID, CProtocolBuff_Net*
	// return : ����
	void Packet_Chat_Login(ULONGLONG SessionID, CProtocolBuff_Net* Packet);


private:
	// -------------------------------------
	// NetServer�� �����Լ�
	// -------------------------------------

	virtual bool OnConnectionRequest(TCHAR* IP, USHORT port);

	virtual void OnClientJoin(ULONGLONG SessionID);

	virtual void OnClientLeave(ULONGLONG SessionID);

	virtual void OnRecv(ULONGLONG SessionID, CProtocolBuff_Net* Payload);

	virtual void OnSend(ULONGLONG SessionID, DWORD SendSize);

	virtual void OnWorkerThreadBegin();

	virtual void OnWorkerThreadEnd();

	virtual void OnError(int error, const TCHAR* errorStr);

public:
	// -------------------------------------
	// Ŭ���� �ܺο��� ��� ������ �Լ�
	// -------------------------------------

	// ������
	CChatServer();

	//�Ҹ���
	~CChatServer();

	// ä�� ���� ���� �Լ�
	// ���������� NetServer�� Start�� ���� ȣ��
	// [���� IP(���ε� �� IP), ��Ʈ, ��Ŀ������ ��, Ȱ��ȭ��ų ��Ŀ������ ��, ����Ʈ ������ ��, TCP_NODELAY ��� ����(true�� ���), �ִ� ������ ��, ��Ŷ Code, XOR 1���ڵ�, XOR 2���ڵ�] �Է¹���.
	//
	// return false : ���� �߻� ��. �����ڵ� ���� �� false ����
	// return true : ����
	bool ServerStart(const TCHAR* bindIP, USHORT port, int WorkerThreadCount, int ActiveWThreadCount, int AcceptThreadCount, bool Nodelay, int MaxConnect,
		BYTE Code, BYTE XORCode1, BYTE XORCode2);

	// ä�� ���� ���� �Լ�
	//
	// Parameter : ����
	// return : ����
	void ServerStop();

	// �׽�Ʈ��. ť ��� �� ���
	LONG GetQueueInNode()
	{	return m_LFQueue->GetInNode();	}

	// �׽�Ʈ��. �� ���� �÷��̾� �� ���
	LONG JoinPlayerCount()
	{
		return m_mapPlayer.size();
	}

};



#endif // !__CHAT_SERVER_H__
