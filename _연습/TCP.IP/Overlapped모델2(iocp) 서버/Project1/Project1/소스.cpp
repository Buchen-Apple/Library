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

// �۾��� ������ �Լ�
UINT WINAPI WorkerThread(LPVOID arg);

int main()
{
	setlocale(LC_ALL, "korean");
	int retval;

	// ���� �ʱ�ȭ
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return 1;

	// ����� �Ϸ� ��Ʈ(I/O Completion Port) ����
	HANDLE hcp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);	// ���� ��ũ����, IOCP�ڵ�, �ΰ�����, ���ÿ� �۵� ���� ������. ������� �Է�
	if (hcp == NULL)
		return 1;

	// CPU ���� Ȯ��
	SYSTEM_INFO si;
	GetSystemInfo(&si);

	// CPU ���� * 2 ���� �۾��� ������ ����
	HANDLE hThread;
	for (int i = 0; i < (int)si.dwNumberOfProcessors * 2; ++i)
	{
		hThread = (HANDLE)_beginthreadex(NULL, 0, WorkerThread, hcp, 0, NULL);
		if (hThread == NULL)
			return 1;

		CloseHandle(hThread);
	}

	// socket()
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

	// ����
	retval = listen(listen_sock, SOMAXCONN);
	if (retval == SOCKET_ERROR)
		return 1;

	// ������ ��ſ� ���Ǵ� ����
	SOCKET client_sock;
	SOCKADDR_IN clientaddr;
	int addrlen;
	DWORD recvbytes, flags;

	while (1)
	{
		// accept()
		addrlen = sizeof(clientaddr);
		client_sock = accept(listen_sock, (SOCKADDR*)&clientaddr, &addrlen);
		if (client_sock == SOCKET_ERROR)
			return 1;

		TCHAR TextBuf[30];
		InetNtop(AF_INET, &clientaddr.sin_addr, TextBuf, sizeof(TextBuf));
		_tprintf(_T("[TCP ����] Ŭ�� ���� : IP �ּ�=%s, ��Ʈ��ȣ=%d\n"), TextBuf, ntohs(clientaddr.sin_port));

		// ���ϰ� ����� �Ϸ� ��Ʈ ����
		CreateIoCompletionPort((HANDLE)client_sock, hcp, client_sock, 0);

		// ���� ���� ����ü �Ҵ�
		SOCKETINFO* ptr = new SOCKETINFO;
		if (ptr == NULL)
			break;

		ZeroMemory(&ptr->overlapped, sizeof(ptr->overlapped));
		ptr->sock = client_sock;
		ptr->recvbytes = ptr->sendbytes = 0;
		ptr->wsabuf.buf = ptr->buf;
		ptr->wsabuf.len = BUFSIZE;

		// �񵿱� ����� ����
		flags = 0;
		retval = WSARecv(client_sock, &ptr->wsabuf, 1, &recvbytes, &flags, &ptr->overlapped, NULL);
		if (retval == SOCKET_ERROR)
		{
			if (WSAGetLastError() != WSA_IO_PENDING)
				_tprintf(_T("WSARecv() ����\n"));

			continue;
		}

	}


	closesocket(listen_sock);
	WSACleanup();

	return 0;
}

UINT WINAPI WorkerThread(LPVOID arg)
{
	int retval;
	HANDLE hcp = (HANDLE)arg;

	while (1)
	{
		// �񵿱� ����� �Ϸ� ��ٸ���
		DWORD cbTransferred;
		SOCKET clinet_sock;
		SOCKETINFO* ptr;
		retval = GetQueuedCompletionStatus(hcp, &cbTransferred, &clinet_sock, (LPOVERLAPPED*)&ptr, INFINITE);	

		// Ŭ���̾�Ʈ ���� ���
		SOCKADDR_IN clientaddr;
		int addrlen = sizeof(clientaddr);
		getpeername(ptr->sock, (SOCKADDR*)&clientaddr, &addrlen);
		TCHAR TextBuf[30];
		InetNtop(AF_INET, (SOCKADDR*)&clientaddr.sin_addr, TextBuf, sizeof(TextBuf));

		// �񵿱� ����� ��� Ȯ��.
		// retval�� 0�̸� GetQueuedCompletionStatus() ����, cbTransferred�� 0�̸� ���۵� ����Ʈ�� 0. ��, ���� ���� ����.
		// �Ʒ������� ����ó���� �ؾ��Ѵ�.
		if (retval == 0 || cbTransferred == 0)
		{
			if (retval == 0)
			{
				DWORD temp1, temp2;

				// WSAGetOverlappedResult()�Լ��� Overlapped IO�� ����� Ȯ���Ѵ�.
				_tprintf(_T("WSAGetOverlappedResult() ����. %d\n"), WSAGetLastError());
			}

			closesocket(ptr->sock);
			_tprintf(_T("[TCP ����] Ŭ���̾�Ʈ ���� : IP �ּ�=%s, ��Ʈ ��ȣ=%d\n"), TextBuf, ntohs(clientaddr.sin_port));
			delete ptr;
			continue;
		}

		// ������ ���۷� ����
		if (ptr->recvbytes == 0)
		{
			ptr->recvbytes = cbTransferred;
			ptr->sendbytes = 0;

			// ���� ������ ���
			ptr->buf[ptr->recvbytes] = '\0';
			_tprintf(_T("[%s:%d] "), TextBuf, ntohs(clientaddr.sin_port));
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
			retval = WSASend(ptr->sock, &ptr->wsabuf, 1, &sendbytes, 0, &ptr->overlapped, NULL);
			if (retval == SOCKET_ERROR)
			{
				if (WSAGetLastError() != WSA_IO_PENDING)
					_tprintf(_T("WSASend ����!"));
				continue;
			}

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
			retval = WSARecv(ptr->sock, &ptr->wsabuf, 1, &recvbytes, &flags, &ptr->overlapped, NULL);
			if (retval == SOCKET_ERROR)
			{
				if(WSAGetLastError() != WSA_IO_PENDING)
					_tprintf(_T("WSARecv ����!"));
				continue;
			}
		}
	}

	return 0;
}