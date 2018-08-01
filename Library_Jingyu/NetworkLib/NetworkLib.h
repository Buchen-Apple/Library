#ifndef __NETWORK_LIB_H__
#define __NETWORK_LIB_H__
#include <windows.h>
#include <map>
#include "ProtocolBuff\ProtocolBuff.h"

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
		// ����Ʈ ������ �ڵ�
		HANDLE*  m_hAcceptHandle;

		// ��Ŀ������ �ڵ� �迭, 
		HANDLE* m_hWorkerHandle;

		// ��Ŀ������ ��, ����Ʈ ������ ��, ������ üũ ������ ��
		int m_iW_ThreadCount;
		int m_iA_ThreadCount;

		// IOCP �ڵ�
		HANDLE m_hIOCPHandle;

		// ��������
		SOCKET m_soListen_sock;

		// ���� �����ϴ� �ڷᱸ��(map)
		// Ű : SessionID
		// �� : stSession ����ü
		map<ULONGLONG, stSession*> map_Session;

		// map�� ����ϴ� SRWLock
		SRWLOCK m_srwSession_map_srwl;

		// ������ ���� ���� ����, ���� ������ ���� ���� ����
		int m_iOSErrorCode;
		euError m_iMyErrorCode;

		// ������ ������ �� ���� 1�� �����ϴ� ������ Ű.
		//ULONGLONG m_ullUniqueSessionID;

		// ���� �������� ���� ��
		ULONGLONG m_ullJoinUserCount;

		// �ִ� ���� ���� ���� ��
		int m_iMaxJoinUser;

		// ���� ���� ����. true�� �۵��� / false�� �۵��� �ƴ�
		bool m_bServerLife;

		// Exclusive �� �ɱ�, Ǯ��
#define	Lock_Exclusive_Map()		LockMap_Exclusive_Func()
#define Unlock_Exclusive_Map()	UnlockMap_Exclusive_Func()

		// Shared  �� �ɱ�, Ǯ��
#define	Lock_Shared_Map()		LockMap_Shared_Func()
#define Unlock_Shared_Map()	UnlockMap_Shared_Func()

	private:
		// ----------------------
		// private �Լ���
		// ----------------------
		// Exclusive �� �ɱ�, �� Ǯ��
		void LockMap_Exclusive_Func();
		void UnlockMap_Exclusive_Func();

		// Shared �� �ɱ�, �� Ǯ��
		void LockMap_Shared_Func();
		void UnlockMap_Shared_Func();

		// ClinetID�� stSession������ �˾ƿ��� �Լ�
		stSession* FineSessionPtr(ULONGLONG ClinetID);

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
		// [���� IP(���ε� �� IP), ��Ʈ, ��Ŀ������ ��, ����Ʈ ������ ��, TCP_NODELAY ��� ����(true�� ���), �ִ� ������ ��] �Է¹���.
		//
		// return false : ���� �߻� ��. �����ڵ� ���� �� false ����
		// return true : ����
		bool Start(const TCHAR* bindIP, USHORT port, int WorkerThreadCount, int AcceptThreadCount, bool Nodelay, int MaxConnect);

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