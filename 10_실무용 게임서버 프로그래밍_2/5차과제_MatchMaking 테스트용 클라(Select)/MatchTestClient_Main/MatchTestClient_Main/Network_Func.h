#ifndef __NETWORK_FUNC_H__
#define __NETWORK_FUNC_H__

#include "ObjectPool/Object_Pool_LockFreeVersion.h"
#include "CrashDump/CrashDump.h"
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

		// ���ú� ����
		CRingBuff m_RecvBuff;

		// Send����. ������ť ����. ��Ŷ����(����ȭ ����)�� �����͸� �ٷ��.
		CLF_Queue<CProtocolBuff_Net*>* m_SendQueue;

		stDummy()
		{
			m_bMatchConnect = false;
			m_bMatchLogin = false;
			m_sock = INVALID_SOCKET;
		}
	};

	// ----------------
	// ��� ����
	// ----------------

	// ����
	// �����ڿ��� ���´�.
	CCrashDump* m_Dump;

	// HTTP ��ſ�
	HTTP_Exchange* HTTP_Post;

	// ���� ��
	int m_iDummyCount;

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

	// �ش� ������ SendBuff�� �α��� ��Ŷ �ֱ�
	//
	// Parameter : stDummy*, CProtocolBuff_Net* (��� �ϼ����� ����)
	// return : ���� �� true
	//		  : ���� �� false
	bool SendPacket(stDummy* NowDummy, CProtocolBuff_Net* payload);


public:
	// ------------------------------------
	// �ܺο��� ��� ���� �Լ�
	// ------------------------------------

	// ���� ���� �Լ�
	//
	// Parameter : ������ ���� ��
	// return : ����
	void CreateDummpy(int Count);

	// ��ġ����ŷ ������ IP�� Port �˾ƿ��� �Լ�
	// �� ���̰�, �ڽ��� ����� Server�� IP�� Port�� �������.
	//
	// Parameter : ����
	// return : ����
	void MatchInfoGet();

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
