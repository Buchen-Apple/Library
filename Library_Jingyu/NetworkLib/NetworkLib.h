/*
������ ����� Network Library
*/


#ifndef __NETWORK_LIB_H__

#define __NETWORK_LIB_H__
#include <windows.h>
#include "ProtocolBuff\ProtocolBuff_ObjectPool.h"
#include "LockFree_Stack\LockFree_Stack.h"

using namespace std;



namespace Library_Jingyu
{
	// --------------
	// CLanServer Ŭ������, ���� ���� �� ��ſ� ���ȴ�.
	// ���� ������ �����, ���� �޴����� ���� / �����ϴ� ���� Ŭ��� ������ �����Ѵ� (����������)
	// �� �� ���� �κ��̴�.
	// --------------
	class CLanServer
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

		// ���� �������� ���� ��
		ULONGLONG m_ullJoinUserCount;

		// �ִ� ���� ���� ���� ��
		int m_iMaxJoinUser;

		// ���� ���� ����. true�� �۵��� / false�� �۵��� �ƴ�
		bool m_bServerLife;


	private:
		// ----------------------
		// private �Լ���
		// ----------------------		
		// ���յ� Ű�� �Է¹�����, Index �����ϴ� �Լ�
		WORD GetSessionIndex(ULONGLONG MixKey);

		// ���յ� Ű�� �Է¹�����, ��¥ ����Ű�� �����ϴ� �Լ�.
		ULONGLONG GetRealSessionKey(ULONGLONG MixKey);

		// CProtocolBuff�� ��� �ִ� �Լ�
		void SetProtocolBuff_HeaderSet(CProtocolBuff* Packet);

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
		CLanServer();
		~CLanServer();

	public:
		// -----------------------
		// �ܺο��� ȣ�� ������ �Լ�
		// -----------------------

		// ----------------------------- ��� �Լ��� ---------------------------

		// ���� ����
		// [���� IP(���ε� �� IP), ��Ʈ, ��Ŀ������ ��, Ȱ��ȭ��ų ��Ŀ������ ��, ����Ʈ ������ ��, TCP_NODELAY ��� ����(true�� ���), �ִ� ������ ��] �Է¹���.
		//
		// return false : ���� �߻� ��. �����ڵ� ���� �� false ����
		// return true : ����
		bool Start(const TCHAR* bindIP, USHORT port, int WorkerThreadCount, int ActiveWThreadCount, int AcceptThreadCount, bool Nodelay, int MaxConnect);

		// ���� ��ž.
		void Stop();

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
		// return true : SendQ�� ���������� ������ ����.
		// return true : SendQ�� ������ �ֱ� ����.
		bool SendPacket(ULONGLONG ClinetID, CProtocolBuff* payloadBuff);


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
		// return false : Ŭ���̾�Ʈ ���� �ź�
		// return true : ���� ���
		virtual bool OnConnectionRequest(TCHAR* IP, USHORT port) = 0;

		// Accept ��, ����ó������ �� �Ϸ�� �� ȣ��ȴ�
		virtual void OnClientJoin(ULONGLONG ClinetID) = 0;

		// InDisconnect �� ȣ��ȴ�. (���� ���ο��� ��Ŀ��Ʈ �� ȣ��)
		virtual void OnClientLeave(ULONGLONG ClinetID) = 0;

		// ��Ŷ ���� �Ϸ� �� ȣ��Ǵ� �Լ�.
		// �ϼ��� ��Ŷ 1���� ����� ȣ���. PacketProc���� �����ϸ� �ȴ�.
		virtual void OnRecv(ULONGLONG ClinetID, CProtocolBuff* Payload) = 0;

		// ��Ŷ �۽� �Ϸ� �� ȣ��Ǵ� �Լ�
		// ���� �Ϸ������� �޾��� ��, ������ �̵� �� �� �ϰ� ȣ��ȴ�.
		virtual void OnSend(ULONGLONG ClinetID, DWORD SendSize) = 0;

		// ��Ŀ ������ GQCS �ٷ� �ϴܿ��� ȣ��
		virtual void OnWorkerThreadBegin() = 0;

		// ��Ŀ������ 1���� ���� �� ȣ��Ǵ� �Լ�.
		virtual void OnWorkerThreadEnd() = 0;

		// ���� �߻� �� ȣ��Ǵ� �Լ�.
		virtual void OnError(int error, const TCHAR* errorStr) = 0;

	};

}






#endif // !__NETWORK_LIB_H__