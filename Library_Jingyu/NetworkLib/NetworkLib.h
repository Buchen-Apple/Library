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
		HANDLE  m_hAcceptHandle;

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
		ULONGLONG m_ullSessionID;

		// ���� �������� ���� ��
		ULONGLONG m_ullJoinUserCount;

		// ���� ���� ����. true�� �۵��� / false�� �۵��� �ƴ�
		bool m_bServerLife;

#define	Lock_Map()	LockMap_Func()
#define Unlock_Map()	UnlockMap_Func()

	private:
		// ----------------------
		// private �Լ���
		// ----------------------
		// �� �ɱ�, �� Ǯ��
		void LockMap_Func();
		void UnlockMap_Func();

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
		bool RecvProc(stSession* NowSession);

		// Accept�� RecvPost�Լ�
		bool RecvPost_Accept(stSession* NowSession);

		// RecvPost�Լ�
		bool RecvPost(stSession* NowSession);

		// SendPost�Լ�
		bool SendPost(stSession* NowSession);

		// ���ο��� ������ ������ ���� �Լ�.
		void InDisconnect(stSession* NowSession);

		// ���� �������� �ʱⰪ���� �ʱ�ȭ
		void Init();


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
		// [���� IP(���ε� �� IP), ��Ʈ, ��Ŀ������ ��, TCP_NODELAY ��� ����(true�� ���), �ִ� ������ ��] �Է¹���.
		// return false : ���� �߻� ��. �����ڵ� ���� �� false ����
		// return true : ����
		bool Start(const TCHAR* bindIP, USHORT port, int WorkerThreadCount, bool Nodelay, int MaxConnect);

		// ���� ��ž.
		void Stop();

		// ������ ������ ���� �� ȣ���ϴ� �Լ�. �ܺ� ���� ���.
		// ���̺귯������ ������!��� ��û�ϴ� �� ��
		bool Disconnect(ULONGLONG ClinetID);

		// �ܺο���, � �����͸� ������ ������ ȣ���ϴ� �Լ�.
		// SendPacket�� �׳� �ƹ����� �ϸ� �ȴ�.
		// �ش� ������ SendQ�� �־�״ٰ� ���� �Ǹ� ������.
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
