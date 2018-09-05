/*
������ ����� Network Library
*/


#ifndef __NETWORK_LIB_NETSERVER_H__
#define __NETWORK_LIB_NETSERVER_H__

#include <windows.h>
#include "ProtocolBuff\ProtocolBuff(Net)_ObjectPool.h"
#include "LockFree_Stack\LockFree_Stack.h"

using namespace std;




namespace Library_Jingyu
{
	// --------------
	// CNetServer Ŭ������, ���� ���� �� ��ſ� ���ȴ�.
	// ���� ������ �����, ���� �޴����� ���� / �����ϴ� ���� Ŭ��� ������ �����Ѵ� (����������)
	// �� �� ���� �κ��̴�.
	// --------------
	class CNetServer
	{
	private:
		// ----------------------
		// private ����ü or enum ���漱��
		// ----------------------
		// ���� enum����
		enum class euError : int;

		// Session����ü ���漱��
		struct stSession;

		// ----------------------
		// private ������
		// ----------------------

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
		CLF_Stack<ULONGLONG>* m_stEmptyIndexStack;

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


	private:
		// ----------------------
		// private �Լ���
		// ----------------------	
		// SendPacket, Disconnect �� �ܺο��� ȣ���ϴ� �Լ�����, ���Ŵ� �Լ�.
		// ���� ���� �ƴ����� ��ó�� ���.
		//
		// Parameter : SessionID
		// return : ���������� ���� ã���� ��, �ش� ���� ������
		//		  : I/Oī��Ʈ�� 0�̵Ǿ� ������ ������, nullptr
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

		// Accept ������
		static UINT	WINAPI	AcceptThread(LPVOID lParam);

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
		CNetServer();
		~CNetServer();


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
		// �ܺο��� ȣ�� ������ �Լ�
		// -----------------------

		// ----------------------------- ��� �Լ��� ---------------------------		

		// ������ ������ ���� �� ȣ���ϴ� �Լ�. �ܺ� ���� ���.
		// ���̺귯������ ������!��� ��û�ϴ� �� ��
		//
		// return true : �ش� �������� �˴ٿ� �� ����.
		// return false : ���������� ���� ������ ���������Ϸ��� ��.
		bool Disconnect(ULONGLONG ClinetID);

		// �ܺο���, � �����͸� ������ ������ ȣ���ϴ� �Լ�.
		// SendPacket�� �׳� �ƹ����� �ϸ� �ȴ�.
		// �ش� ������ SendQ�� �־�״ٰ� ���� �Ǹ� ������.
		//
		// Parameter : SessionID, SendBuff
		// return : ����
		void SendPacket(ULONGLONG SessionID, CProtocolBuff_Net* payloadBuff);


		// ----------------------------- ���� �Լ��� ---------------------------
		// ������ ���� ���
		int WinGetLastError();

		// �� ���� ��� 
		int NetLibGetLastError();

		// ���� ������ �� ���
		ULONGLONG GetClientCount();

		// ���� �������� ���
		// return true : ������
		// return false : ������ �ƴ�
		bool GetServerState();

		// �̻�� ���� ���� ������ ��� ���
		LONG GetStackNodeCount();

	public:
		// -----------------------
		// ���� �����Լ�
		// -----------------------

		// Accept ����, ȣ��ȴ�.
		//
		// parameter : ������ ������ IP, Port
		// return false : Ŭ���̾�Ʈ ���� �ź�
		// return true : ���� ���
		virtual bool OnConnectionRequest(TCHAR* IP, USHORT port) = 0;

		// ���� �� ȣ��Ǵ� �Լ� (AcceptThread���� Accept �� ȣ��)
		//
		// parameter : ������ �������� �Ҵ�� ����Ű
		// return : ����
		virtual void OnClientJoin(ULONGLONG SessionID) = 0;

		// ���� ���� �� ȣ��Ǵ� �Լ� (InDIsconnect �ȿ��� ȣ��)
		//
		// parameter : ���� ����Ű
		// return : ����
		virtual void OnClientLeave(ULONGLONG SessionID) = 0;

		// ��Ŷ ���� �Ϸ� �� ȣ��Ǵ� �Լ�.
		//
		// parameter : ���� ����Ű, ���� ��Ŷ
		// return : ����
		virtual void OnRecv(ULONGLONG SessionID, CProtocolBuff_Net* Payload) = 0;

		// ��Ŷ �۽� �Ϸ� �� ȣ��Ǵ� �Լ�
		//
		// parameter : ���� ����Ű, Send �� ������
		// return : ����
		virtual void OnSend(ULONGLONG SessionID, DWORD SendSize) = 0;

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






#endif // !__NETWORK_LIB_NETSERVER_H__