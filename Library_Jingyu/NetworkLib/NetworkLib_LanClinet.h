/*
������ ����� LanServer
*/


#ifndef __NETWORK_LIB_LANCLIENT_H__
#define __NETWORK_LIB_LANCLIENT_H__

#include <windows.h>
#include "ProtocolBuff\ProtocolBuff_ObjectPool.h"
#include "LockFree_Queue\LockFree_Queue.h"
#include "RingBuff\RingBuff.h"

using namespace std;

namespace Library_Jingyu
{
	// --------------
	// CLanClient Ŭ������, ���� ���� �� ��ſ� ���ȴ�.
	// ���� ������ �����, ���� �޴����� ���� / �����ϴ� ���� Ŭ��� ������ �����Ѵ� (����������)
	// �� �� ���� �κ��̴�.
	// --------------
	class CLanClient
	{
	protected:
		// ----------------------
		// ���� enum����
		// ----------------------
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
			// ------------ ��������� LanServer ��ü������ ����� ����. �ۿ��� �Ű� �� �ʿ� ����

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


	private:
		// �� ���� ������ �� �ִ� WSABUF�� ī��Ʈ
#define dfSENDPOST_MAX_WSABUF			300

		// ----------------------
		// private ����ü 
		// ----------------------	
		
		// ���� ����ü
		struct stSession
		{
			// ���ǰ� ����� ����
			SOCKET m_Client_sock;

			// �ش� ������ ����ִ� �迭 �ε���
			WORD m_lIndex;

			// Send overlapped����ü
			OVERLAPPED m_overSendOverlapped;

			// ���� ID. �������� ��� �� ���.
			ULONGLONG m_ullSessionID;

			// �ش� �ε��� �迭�� ������ �Ǿ����� üũ
			// TRUE�̸� ������ �Ǿ���.(����� �ƴ�), FALSE�̸� ������ �ȵ���.(�����)
			LONG m_lReleaseFlag;

			// I/O ī��Ʈ. WSASend, WSARecv�� 1�� ����.
			// 0�̵Ǹ� ���� ����� ������ �Ǵ�.
			// ���� : ����� ������ WSARecv�� ������ �ɱ� ������ 0�� �� ���� ����.
			LONG	m_lIOCount;

			// ����, WSASend�� �� ���� �����͸� �����ߴ°�. (����Ʈ �ƴ�! ī��Ʈ. ����!)
			int m_iWSASendCount;

			// Send�� ����ȭ ���۵� ������ ������ ����
			CProtocolBuff_Lan* m_PacketArray[dfSENDPOST_MAX_WSABUF];

			// Send���� �������� üũ. 1�̸� Send��, 0�̸� Send�� �ƴ�
			LONG	m_lSendFlag;

			// Send����. ������ť ����. ��Ŷ����(����ȭ ����)�� �����͸� �ٷ��.
			CLF_Queue<CProtocolBuff_Lan*>* m_SendQueue;

			// Recv overlapped����ü
			OVERLAPPED m_overRecvOverlapped;

			// Recv����. �Ϲ� ������. 
			CRingBuff m_RecvQueue;

			// ������ 
			stSession()
			{
				m_Client_sock = INVALID_SOCKET;
				m_SendQueue = new CLF_Queue<CProtocolBuff_Lan*>(1024, false);
				m_lIOCount = 0;
				m_lReleaseFlag = TRUE;
				m_lSendFlag = FALSE;
				m_iWSASendCount = 0;
			}

			// �Ҹ���
			~stSession()
			{
				delete m_SendQueue;
			}

		};

		// ----------------------
		// private ������
		// ----------------------

		// ��������
		SOCKET m_soListen_sock;

		// ��Ŀ  ������ �ڵ� �迭
		HANDLE* m_hWorkerHandle;

		// IOCP �ڵ�
		HANDLE m_hIOCPHandle;

		// ��Ŀ������ ��
		int m_iW_ThreadCount;

		// �����ϰ��� �ϴ� ������ IP�� Port
		TCHAR	m_tcServerIP[20];
		WORD	m_wPort;

		// ���� ID
		ULONGLONG m_ullUniqueSessionID;

		// ���� ����ü
		stSession m_stSession;

		// ������� �ɼ� ����
		bool m_bNodelay;

		// --------------------------

		// ������ ���� ���� ����, ���� ������ ���� ���� ����
		int m_iOSErrorCode;
		euError m_iMyErrorCode;

		// �̻�� �ε��� ���� ������ ���� ��.
		int m_iMaxStackCount;

		// ���� ������ �������� Ŭ��
		// ������ 1���̰�����, stop()�Լ����� ��� ����Ǹ� ��Ŀ�����带 �����Ű�� ������, üũ�뵵�� ����.
		ULONGLONG m_ullJoinUserCount;

		// Ŭ���̾�Ʈ ���� ����. true�� ������, false�� ������ �ƴ�
		bool m_bClienetConnect;


	private:
		// ----------------------
		// private �Լ���
		// ----------------------	

		// ������ ������ connect�ϴ� �Լ�
		bool ConnectFunc();

		// SendPacket, Disconnect �� �ܺο��� ȣ���ϴ� �Լ�����, ���Ŵ� �Լ�.
		// ���� ���� �ƴ����� ��ó�� ���.
		//
		// Parameter : SessionID
		// return : ���������� ���� ã���� ��, �ش� ���� ������
		//			���� �� nullptr
		stSession* GetSessionLOCK(ULONGLONG SessionID);

		// SendPacket, Disconnect �� �ܺο��� ȣ���ϴ� �Լ�����, �� �����ϴ� �Լ�
		// ���� ���� �ƴ����� ��ó�� ���.
		//
		// parameter : ���� ������
		// return : ���������� ���� ��, true
		//		  : I/Oī��Ʈ�� 0�̵Ǿ� ������ ������, false
		bool GetSessionUnLOCK(stSession* NowSession);

		// ��Ŀ ������
		static UINT	WINAPI	WorkerThread(LPVOID lParam);

		// �߰��� ���� ���������� ȣ���ϴ� �Լ�
		// 1. ���� ����
		// 2. ����� �Ϸ���Ʈ �ڵ� ��ȯ
		// 3. ��Ŀ������ ����, �ڵ��ȯ, �ڵ� �迭 ��������
		// 4. �������� �ݱ�
		void ExitFunc(int w_ThreadCount);

		// RecvProc �Լ�. ť�� ���� üũ �� PacketProc���� �ѱ��.
		void RecvProc(stSession* NowSession);

		// RecvPost�Լ�
		//
		// return true : ���������� WSARecv() �Ϸ� or ��¶�� ����� ������ �ƴ�
		// return false : I/Oī��Ʈ�� 0�̵Ǿ ����� ������
		bool RecvPost(stSession* NowSession);

		// SendPost�Լ�
		//
		// return true : ���������� WSASend() �Ϸ� or ��¶�� ����� ������ �ƴ�
		// return false : I/Oī��Ʈ�� 0�̵Ǿ ����� ������
		bool SendPost(stSession* NowSession);

		// ���ο��� ������ ������ ���� �Լ�.
		void InDisconnect(stSession* NowSession);

		// ���� �������� ���½�Ų��.
		void Reset();


	public:
		// -----------------------
		// �����ڿ� �Ҹ���
		// -----------------------
		CLanClient();
		virtual ~CLanClient();

	public:
		// -----------------------
		// �ܺο��� ȣ�� ������ �Լ�
		// -----------------------

		// ----------------------------- ��� �Լ��� ---------------------------
		
		// ���� ����
		// [���� IP(Ŀ��Ʈ �� IP), ��Ʈ, ��Ŀ������ ��, Ȱ��ȭ��ų ��Ŀ������ ��, TCP_NODELAY ��� ����(true�� ���)] �Է¹���.
		//
		// return false : ���� �߻� ��. �����ڵ� ���� �� false ����
		// return true : ����
		bool Start(const TCHAR* ConnectIP, USHORT port, int WorkerThreadCount, int ActiveWThreadCount, bool Nodelay);

		// ���� ��ž.
		void Stop();

		// �ܺο���, � �����͸� ������ ������ ȣ���ϴ� �Լ�.
		// SendPacket�� �׳� �ƹ����� �ϸ� �ȴ�.
		// �ش� ������ SendQ�� �־�״ٰ� ���� �Ǹ� ������.
		//
		// return true : SendQ�� ���������� ������ ����.
		// return true : SendQ�� ������ �ֱ� ����.
		void SendPacket(ULONGLONG SessionID, CProtocolBuff_Lan* payloadBuff);


		// ----------------------------- ���� �Լ��� ---------------------------
		// ������ ���� ���
		int WinGetLastError();

		// �� ���� ��� 
		int NetLibGetLastError();

		// ���� ������ �� ���
		ULONGLONG GetClientCount();

		// Ŭ�� ���������� Ȯ��
		// return true : ������
		// return false : ������ �ƴ�
		bool GetClinetState();

	public:
		// -----------------------
		// ���� �����Լ�
		// -----------------------

		// ��ǥ ������ ���� ���� ��, ȣ��Ǵ� �Լ� (ConnectFunc���� ���� ���� �� ȣ��)
		//
		// parameter : ����Ű
		// return : ����
		virtual void OnConnect(ULONGLONG ClinetID) = 0;

		// ��ǥ ������ ���� ���� �� ȣ��Ǵ� �Լ� (InDIsconnect �ȿ��� ȣ��)
		//
		// parameter : ����Ű
		// return : ����
		virtual void OnDisconnect(ULONGLONG ClinetID) = 0;

		// ��Ŷ ���� �Ϸ� �� ȣ��Ǵ� �Լ�.
		//
		// parameter : ����Ű, ���� ��Ŷ
		// return : ����
		virtual void OnRecv(ULONGLONG ClinetID, CProtocolBuff_Lan* Payload) = 0;

		// ��Ŷ �۽� �Ϸ� �� ȣ��Ǵ� �Լ�
		//
		// parameter : ����Ű, Send �� ������
		// return : ����
		virtual void OnSend(ULONGLONG ClinetID, DWORD SendSize) = 0;

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






#endif // !__NETWORK_LIB_LANCLIENT_H__