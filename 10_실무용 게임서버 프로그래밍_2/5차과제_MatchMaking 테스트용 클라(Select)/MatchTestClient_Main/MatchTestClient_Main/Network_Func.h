#ifndef __NETWORK_FUNC_H__
#define __NETWORK_FUNC_H__

#include "ObjectPool/Object_Pool_LockFreeVersion.h"
#include "CrashDump/CrashDump.h"
#include "Log/Log.h"
#include "Http_Exchange/HTTP_Exchange.h"
#include "RingBuff/RingBuff.h"
#include "ProtocolBuff/ProtocolBuff(Net)_ObjectPool.h"
#include "LockFree_Queue/LockFree_Queue.h"

#include <unordered_map>
#include <unordered_set>

using namespace std;
using namespace Library_Jingyu;


class cMatchTestDummy
{
	// ���� ����ü
	struct stDummy
	{
		UINT64 m_ui64AccountNo;
		SOCKET m_sock;
		TCHAR m_tcToken[64];
		char m_cToekn[64];

		TCHAR m_tcServerIP[20];
		int m_iPort;

		// ��ġ����ŷ ������ Connect�ߴ��� üũ. false�� Connect ����.
		bool m_bMatchConnect;

		// ��ġ����ŷ ������ Login �Ǿ����� üũ (�α��� ��Ŷ ���°� ���� �޾Ҵ���)
		// false�� ���� �α��� �ȵ�
		bool m_bMatchLogin;

		// �α��� ��Ŷ ���� ����
		// false�� �Ⱥ���
		bool m_bLoginPacketSend;

		// ���ú� ����
		CRingBuff m_RecvBuff;
		
		// ���� ����
		CRingBuff m_SendBuff;

		stDummy()
		{
			m_bMatchConnect = false;
			m_bMatchLogin = false;
			m_bLoginPacketSend = false;
			m_sock = INVALID_SOCKET;
		}

		// Disconenct �� �ʱ�ȭ�ϴ� �Լ�
		void Reset()
		{
			m_sock = INVALID_SOCKET;
			m_bMatchLogin = false;
			m_bMatchConnect = false;
			m_bLoginPacketSend = false;
			m_RecvBuff.ClearBuffer();
			m_SendBuff.ClearBuffer();
		}
	};
	
	// ��� ����ü
#pragma pack(push, 1)
	struct stProtocolHead
	{
		BYTE	m_Code;
		WORD	m_Len;
		BYTE	m_RandXORCode;
		BYTE	m_Checksum;
	};
#pragma pack(pop)

	// ----------------
	// ��� ����
	// ----------------

	// ����
	// �����ڿ��� ���´�.
	CCrashDump* m_Dump;

	// �α�
	// �����ڿ��� ���´�.
	CSystemLog* m_Log;

	// HTTP ��ſ�
	HTTP_Exchange* HTTP_Post;

	// ���� ��
	int m_iDummyCount;

	// ���� ��ī��Ʈ �ѹ�
	UINT64 m_ui64StartAccountNo;

	// Config��.
	BYTE m_bCode;
	BYTE m_bXORCode_1;
	BYTE m_bXORCode_2;

	// ------------------------------

	// ���� ���� umap
	// 
	// Key : AccountNo, Value : stDummy*
	unordered_map<UINT64, stDummy*> m_uamp_Dummy;

	// ���� ����ü pool (TLS)
	CMemoryPoolTLS<stDummy> *m_DummyStructPool;

	// ������ Socket ����, ���̸� �˻��Ѵ�.
	//
	// Key : socket, Value : stDummy*
	unordered_map<SOCKET, stDummy*> m_uamp_Socket_and_AccountNo;

	// ------------------------------
	

private:
	// ------------------------------------
	// ���� �ڷᱸ�� ������ �Լ�
	// ------------------------------------

	// ���� ���� �ڷᱸ���� ���� �߰�
	// ���� umap���� ������
	// 
	// Parameter : AccountNo, stDummy*
	// return : �߰� ���� ��, true
	//		  : AccountNo�� �ߺ��� ��(�̹� �������� ����) false
	bool InsertDummyFunc(UINT64 AccountNo, stDummy* insertDummy);

	// ���� ���� �ڷᱸ������, ���� �˻�
	// ���� map���� ������
	// 
	// Parameter : AccountNo
	// return : �˻� ���� ��, stDummy*
	//		  : �˻� ���� �� nullptr
	stDummy* FindDummyFunc(UINT64 AccountNo);



private:
	// ------------------------------------
	// SOCKET ����, ���� �ڷᱸ�� ������ �Լ�
	// ------------------------------------

	// ���� ���� �ڷᱸ���� ���� �߰�
	// ���� umap���� ������
	// 
	// Parameter : SOCKET, stDummy*
	// return : �߰� ���� ��, true
	//		  : SOCKET�� �ߺ��� ��(�̹� �������� ����) false
	bool InsertDummyFunc_SOCKET(SOCKET sock, stDummy* insertDummy);

	// ���� ���� �ڷᱸ������, ���� �˻�
	// ���� map���� ������
	// 
	// Parameter : SOCKET
	// return : �˻� ���� ��, stDummy*
	//		  : �˻� ���� �� nullptr
	stDummy* FindDummyFunc_SOCKET(SOCKET sock);

	//���� ���� �ڷᱸ������, ���� ���� (�˻� �� ����)
	// ���� umap���� ������
	// 
	// Parameter : SOCKET
	// return : ���� ��, ���ŵ� ������ stDummy*
	//		  : �˻� ���� ��(���������� ���� ����) nullptr
	stDummy* EraseDummyFunc_SOCKET(SOCKET socket);






private:
	// ------------------------------------
	// ���ο����� ����ϴ� �Լ�
	// ------------------------------------

	// ��ġ����ŷ ������ Connect
	//
	// Parameter : ����
	// return : ����
	void MatchConnect();

	// ��� ������ SendQ�� �α��� ��Ŷ Enqueue
	//
	// Parameter : ����
	// return : ����
	void MatchLogin();

	// Select ó��
	//
	// Parameter : ����
	// return : ����
	void SelectFunc();

	// Match ������ Disconnect. (Ȯ���� ����)
	// 
	// Parameter : ����
	// return : ����
	void MatchDisconenct();

	// �ش� ������ SendBuff�� �α��� ��Ŷ �ֱ�
	//
	// Parameter : stDummy*, CProtocolBuff_Net* (��� �ϼ����� ����)
	// return : ���� �� true
	//		  : ���� �� false
	bool SendPacket(stDummy* NowDummy, CProtocolBuff_Net* payload);
	
	// ������ ���ڷ� ���� ���� 1��ü�� ������ ���� �Լ�.
	//
	// Parameter : stDummy*
	// returb : ����
	void Disconnect(stDummy* NowDummy);


private:
	// ------------------------------------
	// ��Ʈ��ũ ó�� �Լ�
	// ------------------------------------

	// Recv() ó��
	//
	// Parameter : stDummy*
	// return : ������ ���ܾ� �ϴ� ������, false ����
	bool RecvProc(stDummy* NowDummy);

	// SendBuff�� �����͸� Send�ϱ�
	//
	// Parameter : stDummy*
	// return : ������ ���ܾ� �ϴ� ������, false ����
	bool SendProc(stDummy* NowDummy);

	// Recv �� ������ ó�� �κ�
	//
	// Parameter : stDummy*, ���̷ε�(CProtocolBuff_Net*)
	// return : ����
	void Network_Packet(stDummy* NowDummy, CProtocolBuff_Net* payload);


private:
	// ------------------------------------
	// ��Ŷ ó�� �Լ�
	// ------------------------------------

	// Match������ ���� �α��� ��Ŷ�� ���� ó��
	//
	// Parameter : stDummy*, ���̷ε�(CProtocolBuff_Net*)
	// return : ����
	void PACKET_Login_Match(stDummy* NowDummy, CProtocolBuff_Net* payload);



public:
	// ------------------------------------
	// �ܺο��� ��� ���� �Լ�
	// ------------------------------------

	// ��ġ����ŷ ������ IP�� Port �˾ƿ��� �Լ�
	// �� ���̰�, �ڽ��� ����� Server�� IP�� Port�� �������.
	//
	// Parameter : ����
	// return : ����
	void MatchInfoGet();

	// ���� ���� �Լ�
	//
	// Parameter : ������ ���� ��, ���� AccountNo
	// return : ����
	void CreateDummpy(int Count, int StartAccountNo);

	// ���� Run �Լ�
	// 1. ��ġ����ŷ ������ IP�� Port �˾ƿ��� (Connect �ȵ� ���� ���)
	// 2. ��ġ����ŷ Connect (Connect �ȵ� ���� ���)
	// 3. �α��� ��Ŷ Enqueue (Connect �Ǿ�����, Login �ȵ� ���� ���)
	// 4. Select ó�� (Connect �� ���� ���)
	// 5. Disconnect. (Connect ��, Login �� ���� ���. Ȯ���� Disconnect)
	//
	// Parameter : ����
	// return : ����
	void DummyRun();


public:
	// ------------------------------------
	// ������, �Ҹ���
	// ------------------------------------

	// ������
	cMatchTestDummy();

	// �Ҹ���
	~cMatchTestDummy();
};



#endif // !__NETWORK_FUNC_H__
