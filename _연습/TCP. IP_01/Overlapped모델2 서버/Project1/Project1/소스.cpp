#pragma comment(lib, "ws2_32")
#include <stdlib.h>
#include <stdio.h>
#include <process.h>
#include <WS2tcpip.h>
#include <tchar.h>
#include <locale.h>

#define SERVERPORT 9000
#define BUFSIZE 512

struct SOCKETINFO
{
	WSAOVERLAPPED overlapped;
	SOCKET sock;
	char buf[BUFSIZE + 1];
	int recvbytes;
	int sendbytes;
	WSABUF wsabuf;
};

SOCKET client_sock;
HANDLE hReadEvent, hWriteEvent;

// �񵿱� ����� ���۰� ó�� �Լ�
UINT WINAPI WorkerThread(LPVOID arg);
void CALLBACK CompletionRoutine(DWORD dwError, DWORD cbTransferred, LPWSAOVERLAPPED lpOverlapped, DWORD dwFlags);


int main()
{
	int retval;
	setlocale(LC_ALL, "korean");

	// ���� �ʱ�ȭ
	WSADATA wsa;
	if (WSAStartup(0x0202, &wsa) != 0)
		return 1;

	// ���� ����
	SOCKET listen_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (listen_sock == INVALID_SOCKET)
		return 1;

	// ���ε�
	SOCKADDR_IN serveraddr;
	ZeroMemory(&serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_port = htons(SERVERPORT);
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
	retval = bind(listen_sock, (SOCKADDR*)&serveraddr, sizeof(serveraddr));
	if (retval == SOCKET_ERROR)
		return 1;

	// listen
	retval = listen(listen_sock, SOMAXCONN);
	if (retval == SOCKET_ERROR)
		return 1;

	// �̺�Ʈ ��ü ����
	hReadEvent = CreateEvent(NULL, FALSE, TRUE, NULL);
	if (hReadEvent == NULL)
		return 1;

	hWriteEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	if (hWriteEvent == NULL)
		return 1;

	// ������ ����
	HANDLE hThread = (HANDLE)_beginthreadex(NULL, 0, WorkerThread, NULL, 0, NULL);
	if (hThread == NULL)
		return 1;
	CloseHandle(hThread);

	while (1)
	{
		// worker thread���� clinet_sock�� ���� �о��� ���, ��ȣ ���°� �ȴ�.
		WaitForSingleObject(hReadEvent, INFINITE);

		//accept
		client_sock = accept(listen_sock, NULL, NULL);
		if (client_sock == INVALID_SOCKET)
			break;

		SetEvent(hWriteEvent);
	}


	// ���� ����
	closesocket(listen_sock);
	WSACleanup();

	return 0;
}

UINT WINAPI WorkerThread(LPVOID arg)
{
	int retval;

	while (1)
	{
		while (1)
		{
			// alertable wait
			DWORD result = WaitForSingleObjectEx(hWriteEvent, INFINITE, TRUE);
			if (result == WAIT_OBJECT_0)	// WAIT_OBJECT_0�� ���ο� Ŭ�� �����ߴٴ� ���̴�.
				break;
			if (result != WAIT_IO_COMPLETION)	// WAIT_IO_COMPLETION�� �񵿱� ����°� �̿� ���� �Ϸ� ��ƾ�� �Ϸ�Ǿ��ٴ� ��. ���� ���⼭�� WAIT_IO_COMPLETION�� �ƴҶ� ������ ��������.
				return 1;						// ��, WAIT_IO_COMPLETION�� �ƴ϶�� �ٽ� alertable wait ���·� ���ư���.

		}

		// ������ Ŭ���̾�Ʈ ���� ���
		SOCKADDR_IN clientaddr;
		TCHAR TextBuff[30];

		int addr = sizeof(clientaddr);
		getpeername(client_sock, (SOCKADDR*)&clientaddr, &addr);
		InetNtop(AF_INET, &clientaddr.sin_addr, TextBuff, sizeof(TextBuff));
		_tprintf(_T("connect client. IP : %s, port : %d\n"), TextBuff, ntohs(clientaddr.sin_port));

		// ���� ���� ����ü �Ҵ�� �ʱ�ȭ
		SOCKETINFO* ptr = new SOCKETINFO;
		if (ptr == NULL)
			return 1;

		ZeroMemory(&ptr->overlapped, sizeof(ptr->overlapped));
		ptr->sock = client_sock;
		SetEvent(hReadEvent);	// ���� �����忡��, �б� �Ϸ��ߴٴ� ���� �����Ѵ�.
		ptr->recvbytes = ptr->sendbytes = 0;
		ptr->wsabuf.buf = ptr->buf;
		ptr->wsabuf.len = BUFSIZE;

		// �񵿱� ����� ����
		DWORD recvbytes;
		DWORD flags = 0;
		retval = WSARecv(ptr->sock, &ptr->wsabuf, 1, &recvbytes, &flags, &ptr->overlapped, CompletionRoutine);
		if (retval == SOCKET_ERROR)
			if (WSAGetLastError() != WSA_IO_PENDING)
				return 1;
	}

	return 0;
}

void CALLBACK CompletionRoutine(DWORD dwError, DWORD cbTransferred, LPWSAOVERLAPPED lpOverlapped, DWORD dwFlags)
{
	int retval;

	// Ŭ�� ���� ���
	SOCKETINFO* ptr = (SOCKETINFO*)lpOverlapped; 
	SOCKADDR_IN clientaddr;
	int addrlen = sizeof(clientaddr);
	getpeername(ptr->sock, (SOCKADDR*)&clientaddr, &addrlen);

	// �񵿱� ����� ��� Ȯ��
	if (dwError != 0 || cbTransferred == 0)
		//dwError�� 0�� �ƴ� ��� : ���� �߻�. cbTransferred�� 0�� ��� : ���۵� ������ ���� 0. ��, ����� ���� ����
	{
		closesocket(ptr->sock);
		delete ptr;
		return;
	}

	// ������ ���۷� ����
	if (ptr->recvbytes == 0)
	{
		ptr->recvbytes = cbTransferred;
		ptr->sendbytes = 0;

		// ���� ������ ���
		ptr->buf[ptr->recvbytes] = '\0';
		TCHAR TextBuff[30];
		InetNtop(AF_INET, &clientaddr.sin_addr, TextBuff, sizeof(TextBuff));

		_tprintf(_T("[%s:%d] "), TextBuff, ntohs(clientaddr.sin_port));
		printf("%s\n", ptr->buf);
	}
	else
		ptr->sendbytes += cbTransferred;

	if (ptr->recvbytes > ptr->sendbytes)
	{
		// ������ ������
		ZeroMemory(&ptr->overlapped, sizeof(ptr->overlapped));
		ptr->wsabuf.buf = ptr->buf + ptr->sendbytes;
		ptr->wsabuf.len = ptr->recvbytes - ptr->sendbytes;

		DWORD sendbytes;
		retval = WSASend(ptr->sock, &ptr->wsabuf, 1, &sendbytes, 0, &ptr->overlapped, CompletionRoutine);
		if (retval == SOCKET_ERROR)
			if (WSAGetLastError() != WSA_IO_PENDING)
				return;
	}
	else
	{
		ptr->recvbytes = 0;

		// ������ �ޱ�
		ZeroMemory(&ptr->overlapped, sizeof(ptr->overlapped));
		ptr->wsabuf.buf = ptr->buf;
		ptr->wsabuf.len = BUFSIZE;

		DWORD recvbytes;
		DWORD flags = 0;
		retval = WSARecv(ptr->sock, &ptr->wsabuf, 1, &recvbytes, &flags, &ptr->overlapped, CompletionRoutine);
		if (retval == SOCKET_ERROR)
			if (WSAGetLastError() != WSA_IO_PENDING)
				return;

	}
}