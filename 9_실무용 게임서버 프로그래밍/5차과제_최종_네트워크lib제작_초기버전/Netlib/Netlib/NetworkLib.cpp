#include "stdafx.h"

#pragma comment(lib,"ws2_32")
#include <Ws2tcpip.h>


#include <process.h>
#include <Windows.h>

#include "NetworkLib.h"
#include "RingBuff\RingBuff.h"

using namespace Library_Jingyu;




// ������
CLanServer::CLanServer()
{
	m_iW_ThreadCount = 0;
	m_iA_ThreadCount = 0;
	m_hWorkerHandle = nullptr;
	m_hIOCPHandle = NULL;
	m_soListen_sock = NULL;
}

// �Ҹ���
CLanServer::~CLanServer()
{
	// ��Ŀ ������ ����

	// ��Ŀ ������ �ڵ� ����

	// ��Ŀ ������ �ڵ� �迭 ��������.
	delete[] m_hWorkerHandle;
}


// enum class
enum class CLanServer::euError : int
{
	NETWORK_LIB_ERROR__WINSTARTUP_FAIL = 0,			// ���� �ʱ�ȭ �ϴٰ� ������
	NETWORK_LIB_ERROR__CREATE_IOCP_PORT,			// IPCP ����ٰ� ������
	NETWORK_LIB_ERROR__W_THREAD_FAIL,				// ��Ŀ������ ����ٰ� ���� 
	NETWORK_LIB_ERROR__CREATE_SOCKET_FAIL,			// ���� ���� ���� 
	NETWORK_LIB_ERROR__BINDING_FAIL,				// ���ε� ����
	NETWORK_LIB_ERROR__LISTEN_FAIL,					// ���� ����
	NETWORK_LIB_ERROR__SOCKOPT_FAIL,				// ���� �ɼ� ���� ����

};


// ���� ����ü
struct CLanServer::stSession
{
	SOCKET m_Client_sock;

	TCHAR m_IP[30];
	USHORT m_prot;

	ULONGLONG m_ullSessionID;

	LONG	m_lIOCount = 0;

	// Send���� �������� üũ. 1�̸� Send��, 0�̸� Send�� �ƴ�
	LONG	m_lSendFlag = 0;

	CRingBuff m_RecvQueue;
	CRingBuff m_SendQueue;

	OVERLAPPED m_overRecvOverlapped;
	OVERLAPPED m_overSendOverlapped;	
};

// [���� IP(���ε� �� IP), ��Ʈ, ��Ŀ������ ��, ���̱� �ɼ�, �ִ� ������ ��] �Է¹���.
bool CLanServer::Start(TCHAR* bindIP, USHORT port, int WorkerThreadCount, bool Nagle, int MaxConnect)
{
	// ���� �ʱ�ȭ
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
	{
		// ���� �ڵ� ��ȯ �� �������� ����.
		ExitFunc(m_iW_ThreadCount);

		// ���� �Լ� ȣ��(���� �ʱ�ȭ) �� false ����
		//OnError((int)euError::NETWORK_LIB_ERROR__WINSTARTUP_FAIL, L"WSAStartup() Error...!");
		OnError(WSAGetLastError(), L"WSAStartup() Error...!");
		return false;
	}


	// ����� �Ϸ���Ʈ ����
	// ����� �Ϸ� ��Ʈ ����
	m_hIOCPHandle = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
	if (m_hIOCPHandle == NULL)
	{
		// ���� �ڵ� ��ȯ �� �������� ����.
		ExitFunc(m_iW_ThreadCount);

		// ���� �Լ� ȣ��(����� �Ϸ���Ʈ ���� ����) �� false ����
		//OnError((int)euError::NETWORK_LIB_ERROR__CREATE_IOCP_PORT, L"CreateIoCompletionPort() Error...!");
		OnError(WSAGetLastError(), L"CreateIoCompletionPort() Error...!");
		return false;
	}

	// ��Ŀ ������ ����
	m_iW_ThreadCount = WorkerThreadCount;
	m_hWorkerHandle = new HANDLE[WorkerThreadCount];

	for (int i = 0; i < m_iW_ThreadCount; ++i)
	{
		// suspend(������ �������� ����) ���·� ����.
		// ��������� �� ������ ������ ����
		m_hWorkerHandle[i] = (HANDLE)_beginthreadex(0, 0, WorkerThread, 0, 0, 0);
		if (m_hWorkerHandle[i] == 0)
		{
			// ���� �ڵ� ��ȯ �� �������� ����.
			ExitFunc(i);

			// ���� �Լ� ȣ��(��Ŀ������ ���� ����) �� false ����
			//OnError((int)euError::NETWORK_LIB_ERROR__W_THREAD_FAIL, L"_beginthreadex() WorkerThread Create Error...!");
			OnError(WSAGetLastError(), L"_beginthreadex() WorkerThread Create Error...!");
			return false;
		}
	}

	// ���� ����
	m_soListen_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (m_soListen_sock == INVALID_SOCKET)
	{
		// ���� �ڵ� ��ȯ �� �������� ����.
		ExitFunc(m_iW_ThreadCount);

		// ���� �Լ� ȣ��(���� ���� ���� ����) �� false ����
		// OnError((int)euError::NETWORK_LIB_ERROR__CREATE_SOCKET_FAIL, L"socket() Error...!");
		OnError(WSAGetLastError(), L"socket() Error...!");
		return false;
		
	}

	// ���ε�
	SOCKADDR_IN serveraddr;
	ZeroMemory(&serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_port = htons(port);
	InetPton(AF_INET, bindIP, &serveraddr.sin_addr.s_addr);

	int retval = bind(m_soListen_sock, (SOCKADDR*)&serveraddr, sizeof(serveraddr));
	if (retval == SOCKET_ERROR)
	{
		// ���� �ڵ� ��ȯ �� �������� ����.
		ExitFunc(m_iW_ThreadCount);

		// ���� �Լ� ȣ��(���ε� ����) �� false ����
		// OnError((int)euError::NETWORK_LIB_ERROR__BINDING_FAIL, L"bind() Error...!");
		OnError(WSAGetLastError(), L"bind() Error...!");
		return false;
	}

	// ����
	if (listen(m_soListen_sock, SOMAXCONN) == SOCKET_ERROR)
	{
		// ���� �ڵ� ��ȯ �� �������� ����.
		ExitFunc(m_iW_ThreadCount);

		// ���� �Լ� ȣ��(���� ����) �� false ����
		// OnError((int)euError::NETWORK_LIB_ERROR__LISTEN_FAIL, L"listen() Error...!");
		OnError(WSAGetLastError(), L"listen() Error...!");
		return false;
	}

	// ����� ���� ������ �۽Ź��� ũ�⸦ 0���� ����. �׷��� ���������� �񵿱� ��������� ����
	// ���� ���ϸ� �ٲٸ� ��� Ŭ�� �۽Ź��� ũ��� 0�̵ȴ�.
	int optval = 0;
	if (setsockopt(m_soListen_sock, SOL_SOCKET, SO_SNDBUF, (char*)&optval, sizeof(optval)) == SOCKET_ERROR)
	{
		// ���� �ڵ� ��ȯ �� �������� ����.
		ExitFunc(m_iW_ThreadCount);

		// ���� �Լ� ȣ��(���� �ɼǺ��� ����) �� false ����
		// OnError((int)euError::NETWORK_LIB_ERROR__SOCKOPT_FAIL, L"setsockopt() Error...!");
		OnError(WSAGetLastError(), L"setsockopt() Error...!");
		return false;
	}

	// ����Ʈ ������ ����
	m_hAcceptHandle = (HANDLE)_beginthreadex(NULL, 0, AcceptThread, 0, 0, NULL);
	if (m_hAcceptHandle == NULL)
	{
		// ���� �ڵ� ��ȯ �� �������� ����.
		ExitFunc(m_iW_ThreadCount);

		// ���� �Լ� ȣ��(���� �ɼǺ��� ����) �� false ����
		OnError(WSAGetLastError(), L"_beginthreadex() Accept Thread Error...!");
		return false;
	}

	// ����Ʈ ������ ���� 1�� ����. �ʿ� �������� ������ Ȥ�� �𸣴� ī��Ʈ
	m_iA_ThreadCount = 1;

	// ���� ������ !!
	return true;
}

// �߰��� ���� ���������� ȣ���ϴ� �Լ�
// 1. ���� ����
// 2. ����� �Ϸ���Ʈ �ڵ� ��ȯ
// 3. ��Ŀ������ ����, �ڵ��ȯ, �ڵ� �迭 ��������
// 4. ����Ʈ ������ ����, �ڵ� ��ȯ
// 5. �������� �ݱ�
void CLanServer::ExitFunc(int w_ThreadCount)
{
	// 1. ���� ����
	// ���� �ʱ�ȭ ���� ���� ���¿��� WSAClenup() ȣ���ص� �ƹ� ���� ����
	WSACleanup();

	// 2. ����� �Ϸ���Ʈ ���� ����.
	// null�� �ƴҶ���! (��, ����� �Ϸ���Ʈ�� ������� ���!)
	// �ش� �Լ��� ��𼭳� ȣ��Ǳ⶧����, ���� �̰� �ؾ��ϴ��� �׻� Ȯ��.
	if (m_hIOCPHandle != NULL)
		CloseHandle(m_hIOCPHandle);

	// 3. ��Ŀ������ ���� ����. Count�� 0�� �ƴҶ���!
	if (w_ThreadCount > 0)
	{
		// ���� �޽����� ������
		for (int h = 0; h < w_ThreadCount; ++h)
			PostQueuedCompletionStatus(m_hIOCPHandle, 0, 0, 0);

		// ��� ��Ŀ������ ���� ���.
		WaitForMultipleObjects(w_ThreadCount, m_hWorkerHandle, TRUE, INFINITE);

		// ��� ��Ŀ�����尡 ��������� �ڵ� ��ȯ
		for (int h = 0; h < w_ThreadCount; ++h)
			CloseHandle(m_hWorkerHandle[h]);

		// ��Ŀ������ �ڵ� �迭 ��������. 
		// Count�� 0���� ũ�ٸ� ������ �����Ҵ��� �� ���� ����
		delete[] m_hWorkerHandle;
	}

	// 4. ����Ʈ ������ ����, �ڵ� ��ȯ
	if (m_iA_ThreadCount > 0)
		CloseHandle(m_hAcceptHandle);

	// 5. �������� �ݱ�
	if (m_soListen_sock != NULL)
		closesocket(m_soListen_sock);
}


// ��Ŀ������
UINT WINAPI	CLanServer::WorkerThread(LPVOID lParam)
{
	return 0;
}


// Accept ������
UINT WINAPI	CLanServer::AcceptThread(LPVOID lParam)
{
	// --------------------------
	// Accept ��Ʈ
	// --------------------------
	SOCKET client_sock;
	SOCKADDR_IN	clientaddr;
	int addrlen;

	while (true)
	{
		addrlen = sizeof(clientaddr);
		client_sock = accept(m_soListen_sock, (SOCKADDR*)&clientaddr, &addrlen);
		if (client_sock == INVALID_SOCKET)
		{
			// Accept ������ ���� ����
			break;
		}

		//InterlockedIncrement(&JoinUser);

		// ------------------
		// ���� ����ü ���� �� ����
		// ------------------

		stSession* NewSession = new stSession;
		InetNtop(AF_INET, &clientaddr.sin_addr, NewSession->m_IP, 30);
		NewSession->m_prot = ntohs(clientaddr.sin_port);
		NewSession->m_Client_sock = client_sock;

		// ------------------
		// map ��� �� IOCP ����
		// ------------------
		// ���õ� ����ü�� map�� ���
		LockSession();
		map_Session.insert(pair<ULONGLONG, stSession*>(NewSession->m_ullSessionID, NewSession));
		UnlockSession();


		// ���ϰ� IOCP ����
		CreateIoCompletionPort((HANDLE)client_sock, m_hIOCPHandle, (ULONG_PTR)NewSession, 0);


		// -----------
		// ���� ���� ȭ�鿡 ���
		// -----------
		//_tprintf(L"[TCP ����] Ŭ���̾�Ʈ ���� : IP �ּ�=%s, ��Ʈ=%d\n", IP, NewSession->m_prot);


		// ------------------
		// �񵿱� ����� ����
		// ------------------
		/*RecvPost_Accept(NewSession);
		AcceptCount++;*/
	}

	return 0;
}


