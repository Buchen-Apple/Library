#ifndef __NETWORK_LIB_MMOSERVER_H__
#define __NETWORK_LIB_MMOSERVER_H__

#include <windows.h>
#include "ProtocolBuff\ProtocolBuff(Net)_ObjectPool.h"
#include "ObjectPool\Object_Pool_LockFreeVersion.h"
#include "LockFree_Stack\LockFree_Stack.h"
#include "LockFree_Queue\LockFree_Queue.h"
#include "RingBuff\RingBuff.h"
#include "NormalTemplate_Queue\Normal_Queue_Template.h"

// ------------------------
// MMOServer
// ------------------------
namespace Library_Jingyu
{

	class CMMOServer
	{

		// �� ���� ������ �� �ִ� WSABUF�� ī��Ʈ
#define dfSENDPOST_MAX_WSABUF			300

	protected:
		// enum ���漱��
		enum class euError : int
		{
			NETWORK_LIB_ERROR__NORMAL = 0,					// ���� ���� �⺻ ����
			NETWORK_LIB_ERROR__WINSTARTUP_FAIL,				// ���� �ʱ�ȭ �ϴٰ� ������
			NETWORK_LIB_ERROR__CREATE_IOCP_PORT,			// IPCP ����ٰ� ������
			NETWORK_LIB_ERROR__THREAD_CREATE_FAIL,			// ������ ����ٰ� ���� 
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
			MODE_WAIT_LOGOUT		// ���� ����
		};

		// �� �����忡�� ó���ϴ� ��Ŷ�� �� enum
		enum class euDEFINE
		{
			// ******* AUTH ������� *******
			eu_AUTH_PACKET_COUNT = 1,			// 1�����ӵ���, 1���� ������ �� ���� ��Ŷ�� ó���� ���ΰ�
			eu_AUTH_SLEEP = 1,					// Sleep�ϴ� �ð�(�и�������)	
			eu_AUTH_NEWUSER_PACKET_COUNT = 50,   // 1�����ӵ���, Accept Socket Queue���� ���� ��Ŷ�� ��. (��, 1�����ӿ� None���� Auth�� ����Ǵ� ���� ��)


			// ******* GAME ������� *******
			eu_GAME_PACKET_COUNT = 200,			// 1�����ӿ�, 1���� ������ �� ���� ��Ŷ�� ó���� ���ΰ�
			eu_GAME_SLEEP = 1,					// Sleep�ϴ� �ð�(�и�������)	
			eu_GAME_NEWUSER_JOIN_COUNT = 50,	// 1������ ����, AUTH_IN_GAME���� GAME���� ����Ǵ� ������ ��	

			// ******* RELEAE ������� *******
			eu_RELEASE_SLEEP = 1					// Sleep�ϴ� �ð�(�и�������)	
		};


		// ------------------
		// �̳� Ŭ����
		// ------------------

		// ���� Ŭ����
		class cSession
		{
			friend class CMMOServer;

			// -----------
			// ��� ����
			// -----------

			// Auth To Game Falg
			// true�� Game���� ��ȯ�� ����
			LONG m_lAuthToGameFlag;

			// ������ ��� ����
			euSessionModeState m_euMode;

			// �α׾ƿ� �÷���
			// true�� �α׾ƿ� �� ����.
			LONG m_lLogoutFlag;

			// CompleteRecvPacket(Queue)
			CNormalQueue<CProtocolBuff_Net*>* m_CRPacketQueue;

			// ���ǰ� ����� ����
			SOCKET m_Client_sock;

			// �ش� ������ ����ִ� �迭 �ε���
			WORD m_lIndex;

			// ����� ������ IP�� Port
			TCHAR m_IP[30];
			USHORT m_prot;

			// Send overlapped����ü
			OVERLAPPED m_overSendOverlapped;

			// ���� ID. �������� ��� �� ���.
			ULONGLONG m_ullSessionID;

			// I/O ī��Ʈ. WSASend, WSARecv�� 1�� ����.
			// 0�̵Ǹ� ���� ����� ������ �Ǵ�.
			// ���� : ����� ������ WSARecv�� ������ �ɱ� ������ 0�� �� ���� ����.
			LONG	m_lIOCount;

			// ����, WSASend�� �� ���� �����͸� �����ߴ°�. (����Ʈ �ƴ�! ī��Ʈ. ����!)
			int m_iWSASendCount;

			// Send�� ����ȭ ���۵� ������ ������ ����
			CProtocolBuff_Net* m_PacketArray[dfSENDPOST_MAX_WSABUF];

			// Send���� �������� üũ. 1�̸� Send��, 0�̸� Send�� �ƴ�
			LONG	m_lSendFlag;

			// Send����. ������ť ����. ��Ŷ����(����ȭ ����)�� �����͸� �ٷ��.
			CLF_Queue<CProtocolBuff_Net*>* m_SendQueue;

			// Recv overlapped����ü
			OVERLAPPED m_overRecvOverlapped;

			// Recv����. �Ϲ� ������. 
			CRingBuff m_RecvQueue;

			// ������ ��Ŷ �����
			void* m_LastPacket = nullptr;

			// ��� �ڵ�
			BYTE m_bCode;

			// XOR1 �ڵ�
			BYTE m_bXORCode_1;

			// XOR2 �ڵ�
			BYTE m_bXORCode_2;

		public:
			// -----------------
			// �����ڿ� �Ҹ���
			// -----------------
			cSession();
			virtual ~cSession();

		protected:
			// -----------------
			// �����Լ�
			// -----------------

			// Auth �����忡�� ó��
			virtual void OnAuth_ClientJoin();
			virtual void OnAuth_ClientLeave(bool bGame = false);
			virtual void OnAuth_Packet(CProtocolBuff_Net* Packet);

			// Game �����忡�� ó��
			virtual void OnGame_ClientJoin();
			virtual void OnGame_ClientLeave();
			virtual void OnGame_Packet(CProtocolBuff_Net* Packet);

			// Release��
			virtual void OnGame_ClientRelease();


		protected:
			// -----------------
			// ��Ʈ��ũ�� ����Լ�
			// -----------------

			// ������ ���� �� ȣ���ϴ� �Լ�. �ܺ� ���� ���.
			// ���̺귯������ ������!��� ��û�ϴ� �� ��
			//
			// Parameter : ����
			// return : ����
			void Disconnect();

			// �ܺο���, �������� � �����͸� ������ ������ ȣ���ϴ� �Լ�.
			// SendPacket�� �׳� �ƹ����� �ϸ� �ȴ�.
			// �ش� ������ SendQ�� �־�״ٰ� ���� �Ǹ� ������.
			//
			// Parameter : SendBuff, LastFlag(����Ʈ FALSE)
			// return : ����
			void SendPacket(CProtocolBuff_Net* payloadBuff, LONG LastFlag = FALSE);


			// -----------------
			// �Ϲ� ��� �Լ�
			// -----------------
			// �ش� ������ ��带 GAME���� �����ϴ� �Լ�
			// ���ο����� m_lAuthToGameFlag ������ TRUE�� �����.
			//
			// Parameter : ����
			// return : ����
			void SetMode_GAME();

		};

		// ��� ����ü
		struct stProtocolHead;

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

		// ��Ŀ, ����Ʈ, ����, �, ���� ������ �ڵ� �迭
		HANDLE*	m_hAcceptHandle;
		HANDLE* m_hWorkerHandle;
		HANDLE* m_hSendHandle;
		HANDLE* m_hAuthHandle;
		HANDLE* m_hGameHandle;

		// ������ ������
		HANDLE m_hReleaseHandle;

		// IOCP �ڵ�
		HANDLE m_hIOCPHandle;

		// ��Ŀ������ ��, ����Ʈ ������ ��, ���� ������ ��, � ������ ��, ���� ������ ��
		int m_iA_ThreadCount;
		int m_iW_ThreadCount;
		int m_iS_ThreadCount;
		int m_iAuth_ThreadCount;
		int m_iGame_ThreadCount;


		// ----- ���� ������ -------
		// ���� ���� �迭
		cSession** m_stSessionArray;

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

		// Auth, Game, Send ������ ���� �̺�Ʈ
		HANDLE m_hAuthExitEvent;
		HANDLE m_hGameExitEvent;
		HANDLE m_hSendExitEvent;
		HANDLE m_hReleaseExitEvent;


	public:
		// -----------------------
		// �����ڿ� �Ҹ���
		// -----------------------
		CMMOServer();
		virtual ~CMMOServer();


	public:
		// -----------------------
		// ���� �Լ�
		// -----------------------

		// ���� �۵������� Ȯ��
		//
		// Parameter : ����
		// return : �۵����� �� true.
		bool GetServerState();

		// ���� ���� ���� �� ���
		//
		// Parameter : ����
		// return : ������ �� (ULONGLONG)
		ULONGLONG GetClientCount();

		// Accept Socket Queue ���� �ϰ� �� ���
		//
		// Parameter : ����
		// return : �ϰ� ��(LONG)
		LONG GetASQ_Count();

		// Accept Socket Queue Pool�� �� ûũ �� ���
		// 
		// Parameter : ����
		// return : �� ûũ �� (LONG)
		LONG GetChunkCount();

		// Accept Socket Queue Pool�� �ۿ��� ������� ûũ �� ���
		// 
		// Parameter : ����
		// return : �ۿ��� ������� ûũ �� (LONG)
		LONG GetOutChunkCount();


	private:
		// -----------------------
		// ���ο����� ����ϴ� �Լ�
		// -----------------------

		// Start���� ������ �� �� ȣ���ϴ� �Լ�.
		//
		// Parameter : ��Ŀ�������� ��
		// return : ����
		void ExitFunc(int w_ThreadCount);

		// ���� �������� ���½�Ų��.
		// Stop() �Լ����� ���.
		void Reset();

		// ���ο��� ������ ������ ���� �Լ�.
		//
		// Parameter : ���� ���� ������
		// return : ����
		void InDisconnect(cSession* DeleteSession);

		// RecvProc �Լ�.
		// �ϼ��� ��Ŷ�� ���ڷι��� Session�� CompletionRecvPacekt (Queue)�� �ִ´�.
		//
		// Parameter : ���� ������
		// return : ����
		void RecvProc(cSession* NowSession);

		// RecvPost�Լ�
		//
		// return 0 : ���������� WSARecv() �Ϸ�
		// return 1 : RecvQ�� ���� ����
		// return 2 : I/O ī��Ʈ�� 0�̵Ǿ� ������ ����
		int RecvPost(cSession* NowSession);	




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

		// Send ������
		static UINT WINAPI	SendThread(LPVOID lParam);		

		// Release ������
		static UINT WINAPI	ReleaseThread(LPVOID lParam);


	protected:
		// -----------------------
		// ��� ���迡���� ȣ�� ������ �Լ�
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

		// ���� ����
		//
		// Parameter : cSession* ������, Max ��(�ִ� ���� ���� ���� ��), HeadCode, XORCode1, XORCode2
		// return : ����
		void SetSession(cSession* pSession, int Max, BYTE HeadCode, BYTE XORCode1, BYTE XORCode2);


	private:
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
