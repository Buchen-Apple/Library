#ifndef __NETWORK_LIB_MMOSERVER_H__
#define __NETWORK_LIB_MMOSERVER_H__

#include <windows.h>
#include "ProtocolBuff\ProtocolBuff(Net)_ObjectPool.h"
#include "ObjectPool\Object_Pool_LockFreeVersion.h"
#include "LockFree_Stack\LockFree_Stack.h"
#include "LockFree_Queue\LockFree_Queue.h"

// ------------------------
// MMOServer
// ------------------------
namespace Library_Jingyu
{
	class CMMOServer
	{
	protected:
		// enum ���漱��
		enum class euError : int
		{
			NETWORK_LIB_ERROR__NORMAL = 0,					// ���� ���� �⺻ ����
			NETWORK_LIB_ERROR__WINSTARTUP_FAIL,				// ���� �ʱ�ȭ �ϴٰ� ������
			NETWORK_LIB_ERROR__CREATE_IOCP_PORT,			// IPCP ����ٰ� ������
			NETWORK_LIB_ERROR__W_THREAD_CREATE_FAIL,		// ��Ŀ������ ����ٰ� ���� 
			NETWORK_LIB_ERROR__A_THREAD_CREATE_FAIL,		// ����Ʈ ������ ����ٰ� ���� 
			NETWORK_LIB_ERROR__CREATE_SOCKET_FAIL,			// ���� ���� ���� 
			NETWORK_LIB_ERROR__BINDING_FAIL,				// ���ε� ����
			NETWORK_LIB_ERROR__LISTEN_FAIL,					// ���� ����
			NETWORK_LIB_ERROR__SOCKOPT_FAIL,				// ���� �ɼ� ���� ����
			NETWORK_LIB_ERROR__WSAENOBUFS,					// WSASend, WSARecv�� ���ۻ����� ����
			// ------------ ��������� NetServer ��ü������ ����� ����. �ۿ��� �Ű� �� �ʿ� ����

			NETWORK_LIB_ERROR__IOCP_ERROR,					// IOCP ��ü ����
			NETWORK_LIB_ERROR__NOT_FIND_CLINET,				// map �˻� ���� �Ҷ� Ŭ���̾�Ʈ�� ��ã�����.
			NETWORK_LIB_ERROR__SEND_QUEUE_SIZE_FULL,		// Enqueue����� ���� ����
			NETWORK_LIB_ERROR__QUEUE_DEQUEUE_EMPTY,			// Dequeue ��, ť�� ����ִ� ����. Peek�� �õ��ϴµ� ť�� ����� ��Ȳ�� ����
			NETWORK_LIB_ERROR__WSASEND_FAIL,				// SendPost���� WSASend ����			
			NETWORK_LIB_ERROR__A_THREAD_ABNORMAL_EXIT,		// ����Ʈ ������ ������ ����. ���� accept()�Լ����� �̻��� ������ ���°�.
			NETWORK_LIB_ERROR__A_THREAD_IOCPCONNECT_FAIL,	// ����Ʈ �����忡�� IOCP ���� ����
			NETWORK_LIB_ERROR__W_THREAD_ABNORMAL_EXIT,		// ��Ŀ ������ ������ ����. 
			NETWORK_LIB_ERROR__WFSO_ERROR,					// WaitForSingleObject ����.
			NETWORK_LIB_ERROR__IOCP_IO_FAIL,				// IOCP���� I/O ���� ����. �� ����, ���� Ƚ���� I/O�� ��õ��Ѵ�.
			NETWORK_LIB_ERROR__JOIN_USER_FULL,				// ������ Ǯ�̶� �� �̻� ���� ������
			NETWORK_LIB_ERROR__RECV_CODE_ERROR,				// RecvPost���� ��� �ڵ尡 �ٸ� �������� ���.
			NETWORK_LIB_ERROR__RECV_CHECKSUM_ERROR,			// RecvPost���� Decode �� üũ���� �ٸ�
			NETWORK_LIB_ERROR__RECV_LENBIG_ERROR,			// RecvPost���� ��� �ȿ� Len�� ������������ ŭ.
		};

		// ���� ��� enum
		enum class euSessionModeState
		{
			MODE_NONE = 0,			// ���� �̻�� ����
			MODE_AUTH,				// ���� �� ������� ����
			MODE_AUTH_TO_GAME,		// ���Ӹ��� ��ȯ
			MODE_GAME,				// ���� �� ���Ӹ�� ����
			MODE_LOGOUT_IN_AUTH,	// ������忡�� ���� �غ�
			MODE_LOGOUT_IN_GAME,	// ���Ӹ�忡�� ���� �غ�
			MODE_WAIT_LOGOUT		//���� ����
		};



		// ------------------
		// �̳� Ŭ���� ���漱��
		// ------------------
		struct stSession;

	private:

		// Accept Socket Queue���� ������ �ϰ� ����ü
		struct stAuthWork
		{
			SOCKET m_clienet_Socket;
			TCHAR m_tcIP[30];
			USHORT m_usPort;
		};


	private:
		// ------------------
		// ��� ����
		// ------------------

		// Accept Socket Queue�� Pool
		CMemoryPoolTLS<stAuthWork>* m_pAcceptPool;

		// Accept Socket Queue
		CLF_Queue<stAuthWork*>* m_pASQ;


		// Net������ Config��.
		BYTE m_bCode;
		BYTE m_bXORCode_1;
		BYTE m_bXORCode_2;

		// ��������
		SOCKET m_soListen_sock;

		// ��Ŀ, ����Ʈ ������ �ڵ� �迭
		HANDLE*	m_hAcceptHandle;
		HANDLE* m_hWorkerHandle;

		// IOCP �ڵ�
		HANDLE m_hIOCPHandle;

		// ��Ŀ������ ��, ����Ʈ ������ ��
		int m_iA_ThreadCount;
		int m_iW_ThreadCount;


		// ----- ���� ������ -------
		// ���� ���� �迭
		stSession* m_stSessionArray;

		// �̻�� �ε��� ���� ����
		CLF_Stack<WORD>* m_stEmptyIndexStack;
		// --------------------------

		// ������ ���� ���� ����, ���� ������ ���� ���� ����
		int m_iOSErrorCode;
		euError m_iMyErrorCode;

		// �ִ� ���� ���� ���� ��
		int m_iMaxJoinUser;

		// ���� �������� ���� ��
		ULONGLONG m_ullJoinUserCount;

		// ���� ���� ����. true�� �۵��� / false�� �۵��� �ƴ�
		bool m_bServerLife;


	public:
		// -----------------------
		// �����ڿ� �Ҹ���
		// -----------------------
		CMMOServer();
		virtual ~CMMOServer();


	private:
		// -----------------------
		// ���ο����� ����ϴ� �Լ�
		// -----------------------

		// Start���� ������ �� �� ȣ���ϴ� �Լ�.
		// 1. ����� �Ϸ���Ʈ �ڵ� ��ȯ
		// 2. ��Ŀ������ ����, �ڵ��ȯ, �ڵ� �迭 ��������
		// 3. ����Ʈ ������ ����, �ڵ� ��ȯ
		// 4. �������� �ݱ�
		// 5. ���� ����
		void ExitFunc(int w_ThreadCount);

		// ���� �������� ���½�Ų��.
		// Stop() �Լ����� ���.
		void Reset();

		// RecvPost�Լ�
		//
		// return 0 : ���������� WSARecv() �Ϸ�
		// return 1 : RecvQ�� ���� ����
		// return 2 : I/O ī��Ʈ�� 0�̵Ǿ� ������ ����
		int RecvPost(stSession* NowSession);


	private:
		// -----------------------
		// �������
		// -----------------------

		// ��Ŀ ������
		static UINT	WINAPI	WorkerThread(LPVOID lParam);

		// Accept ������
		static UINT	WINAPI	AcceptThread(LPVOID lParam);

		// Auth ������
		static UINT	WINAPI	AuthThread(LPVOID lParam);

		// Game ������
		static UINT	WINAPI	GameThread(LPVOID lParam);

	public:
		// -----------------------
		// �ܺο��� ��� ������ �Լ�
		// -----------------------

		// ���� ����
		//
		// Parameter : stSession* ������, Max ��(�ִ� ���� ���� ���� ��)
		// return : ����
		void SetSession(stSession* pSession, int Max);



	protected:
		// -----------------------
		// ��ӿ����� ȣ�� ������ �Լ�
		// -----------------------

		// ���� ����
		// [���� IP(���ε� �� IP), ��Ʈ, ��Ŀ������ ��, Ȱ��ȭ��ų ��Ŀ������ ��, ����Ʈ ������ ��, TCP_NODELAY ��� ����(true�� ���), �ִ� ������ ��, ��Ŷ Code, XOR 1���ڵ�, XOR 2���ڵ�] �Է¹���.
		//
		// return false : ���� �߻� ��. �����ڵ� ���� �� false ����
		// return true : ����
		bool Start(const TCHAR* bindIP, USHORT port, int WorkerThreadCount, int ActiveWThreadCount, int AcceptThreadCount, bool Nodelay, int MaxConnect,
			BYTE Code, BYTE XORCode1, BYTE XORCode2);

		// ���� ��ž.
		void Stop();

		public:
			// -----------------------
			// ���� �����Լ�
			// -----------------------

			// AuthThread���� 1Loop���� 1ȸ ȣ��.
			// 1�������� ���������� ó���ؾ� �ϴ� ���� �Ѵ�.
			// 
			// parameter : ����
			// return : ����
			virtual void OnAuth_Update() = 0;

			// GameThread���� 1Loop���� 1ȸ ȣ��.
			// 1�������� ���������� ó���ؾ� �ϴ� ���� �Ѵ�.
			// 
			// parameter : ����
			// return : ����
			virtual void OnGame_Update() = 0;			

			// ���ο� ���� ���� ��, Auth���� ȣ��ȴ�.
			//
			// parameter : ������ ������ IP, Port
			// return false : Ŭ���̾�Ʈ ���� �ź�
			// return true : ���� ���
			virtual bool OnConnectionRequest(TCHAR* IP, USHORT port) = 0;			

			// ��Ŀ �����尡 ��� �� ȣ��Ǵ� �Լ�.
			// GQCS �ٷ� �ϴܿ��� ȣ��
			// 
			// parameter : ����
			// return : ����
			virtual void OnWorkerThreadBegin() = 0;

			// ��Ŀ �����尡 ���� �� ȣ��Ǵ� �Լ�
			// GQCS �ٷ� ������ ȣ��
			// 
			// parameter : ����
			// return : ����
			virtual void OnWorkerThreadEnd() = 0;

			// ���� �߻� �� ȣ��Ǵ� �Լ�.
			//
			// parameter : ���� �ڵ�(���� ������ �����ڵ�� WinGetLastError() �Լ��� ��� ����. ���� ��� 0�� ���ϵ�)
			//			 : ���� �ڵ忡 ���� ��Ʈ��
			// return : ����
			virtual void OnError(int error, const TCHAR* errorStr) = 0;

	



	};
}

#endif // !__NETWORK_LIB_MMOSERVER_H__
