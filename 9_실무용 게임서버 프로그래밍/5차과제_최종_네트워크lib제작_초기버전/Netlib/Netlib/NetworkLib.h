#ifndef __NETWORK_LIB_H__
#define __NETWORK_LIB_H__
#include <map>

using namespace std;


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

private:
	// ----------------------
	// private ������
	// ----------------------
	// ����Ʈ ������ �ڵ�
	HANDLE  m_hAcceptHandle;

	// ��Ŀ������ ��, ����Ʈ ������ ��
	int m_iW_ThreadCount, m_iA_ThreadCount;

	// IOCP �ڵ�
	static HANDLE m_hIOCPHandle;

	// ��Ŀ������ �ڵ� �迭, 
	static HANDLE* m_hWorkerHandle;

	// ��������
	static SOCKET m_soListen_sock;

	// ���� �����ϴ� �ڷᱸ��(map)
	// Ű : SessionID
	// �� : stSession ����ü
	static map<ULONGLONG, stSession*> map_Session;

	// map�� ����ϴ� SRWLock
	static SRWLOCK m_srwSession_map_srwl;

#define	LockSession()	AcquireSRWLockExclusive(&m_srwSession_map_srwl)
#define UnlockSession()	ReleaseSRWLockExclusive(&m_srwSession_map_srwl)

private:
	// ----------------------
	// private �Լ���
	// ----------------------
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

public:
	// �����ڿ� �Ҹ���
	CLanServer();
	~CLanServer();

	// [���� IP(���ε� �� IP), ��Ʈ, ��Ŀ������ ��, ���̱� �ɼ�, �ִ� ������ ��] �Է¹���.
	// return false : ���� �߻� ��. OnError()�Լ� ȣ�� �� false ����
	// return true : ��� ����
	bool Start(TCHAR* bindIP, USHORT port, int WorkerThreadCount, bool Nagle, int MaxConnect);

	// ���� ��ž.
	void Stop();

	// �ܺο���, ������ ������ ���� �� ȣ���ϴ� �Լ�.
	bool Disconnect();

	// �ܺο���, � �����͸� ������ ������ ȣ���ϴ� �Լ�.
	// SendPacket�� �׳� �ƹ����� �ϸ� �ȴ�.
	// �ش� ������ SendQ�� �־�״ٰ� ���� �Ǹ� ������.
	bool SendPacket();

public:	

	// Accept �� ����ó�� �Ϸ� �� ȣ��
	virtual void OnClientJoin() = 0;

	// Disconnect �� ȣ��ȴ�.
	virtual void OnClientLeave() = 0;

	// Accept ����, ȣ��ȴ�.
	// return false : Ŭ���̾�Ʈ ���� �ź�
	// return true : ���� ���
	virtual bool OnConnectioinRequest() = 0;

	// ��Ŷ ���� �Ϸ� �� ȣ��Ǵ� �Լ�.
	virtual void OnRecv() = 0;

	// ��Ŷ �۽� �Ϸ� �� ȣ��Ǵ� �Լ�
	virtual void OnSend() = 0;

	// ��Ŀ ������ GQCS �ٷ� �ϴܿ��� ȣ��
	virtual void OnWorkerThreadBegin() = 0;

	// ��Ŀ������ 1��Ǫ ���� �� ȣ��Ǵ� �Լ�.
	virtual void OnWorkerThreadEnd() = 0;

	// ���� �߻� �� ȣ��Ǵ� �Լ�.
	virtual void OnError(int error, const TCHAR* errorStr) = 0;

};
// static ������ ����
HANDLE CLanServer::m_hIOCPHandle;
HANDLE* CLanServer::m_hWorkerHandle;
SOCKET CLanServer::m_soListen_sock;
map<ULONGLONG, CLanServer::stSession*> CLanServer::map_Session;
SRWLOCK CLanServer::m_srwSession_map_srwl;







#endif // !__NETWORK_LIB_H__
