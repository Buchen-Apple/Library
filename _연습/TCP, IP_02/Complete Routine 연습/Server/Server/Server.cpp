// Server.cpp: 콘솔 응용 프로그램의 진입점을 정의합니다.
//

#include "stdafx.h"

#pragma comment(lib, "ws2_32")
#include <Ws2tcpip.h>

#include <Windows.h>
#include <process.h>



#define SERVER_PORT	9000
#define BUFF_SIZE	512

// 소켓 정보 저장을 위한 구조체
struct SOCKETINFO
{
	WSAOVERLAPPED overlapped;
	SOCKET sock;
	char buff[BUFF_SIZE + 1];
	int recvBytes;
	int sendBytes;
	WSABUF	wsabuf;

};

SOCKET client_sock;
HANDLE hReadEvent, hWriteEvent;


UINT WINAPI WorkerThread(LPVOID arg);
void CALLBACK CompletionRoutine(DWORD dwError, DWORD cbTransferred, LPWSAOVERLAPPED lpOverlapped, DWORD dwFlags);

int _tmain()
{
	int retval;

	// 초기화
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return 1;

	// 생성
	SOCKET listen_sock = socket(AF_INET, SOCK_STREAM, NULL);
	if (listen_sock == INVALID_SOCKET)
		return 1;

	// 바인딩
	SOCKADDR_IN serveraddr;
	ZeroMemory(&serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_port = htons(SERVER_PORT);
	serveraddr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);

	retval = bind(listen_sock, (SOCKADDR*)&serveraddr, sizeof(serveraddr));

	if (retval == SOCKET_ERROR)
		return 1;

	// 리슨
	retval = listen(listen_sock, SOMAXCONN);
	if (retval == SOCKET_ERROR)
		return 1;

	// 이벤트 생성
	hReadEvent = CreateEvent(NULL, FALSE, TRUE, NULL);
	hWriteEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

	// 스레드 생성
	HANDLE	hThread = (HANDLE)_beginthreadex(NULL, 0, WorkerThread, NULL, 0, NULL);

	CloseHandle(hThread);

	SOCKADDR_IN	clientaddr;

	while (1)
	{
		WaitForSingleObject(hReadEvent, INFINITE);

		// Accept
		int len = sizeof(clientaddr);
		client_sock = accept(listen_sock, (SOCKADDR*)&clientaddr, &len);
		if (client_sock == INVALID_SOCKET)
			return 1;

		SetEvent(hWriteEvent);
	}

	WSACleanup();

    return 0;
}

// 하는 일 : 알림 대기상태에서 대기하다가 새로운 유저가 접속하면 깨어나서 완료 루틴을 연결해준다.

// 새로운 클라이언트가 접속하거나, 완료루틴이 완료되면 알림 대기상태에서 깨어난다.
// 새로운 클라이언트가 접속했으면 접속한 클라 정보 보여준 후, 해당 클라의 완료루틴 지정
// 완료루틴 호출이 끝난거면, 얘는 할일이 없으니 다시 알림 대기상태로 들어간다.

UINT WINAPI WorkerThread(LPVOID arg)
{
	int retval;

	while (1)
	{
		while (1)
		{
			// 알림 대기 상태
			DWORD result = WaitForSingleObjectEx(hWriteEvent, INFINITE, TRUE);

			// 새로운 클라가 접속했다면 해당 while문 종료
			if (result == WAIT_OBJECT_0)
				break;

			// WAIT_IO_COMPLETION가 아니라면, 뭔가 이상한 오류가 뜬 것이니 스레드 종료.
			// WAIT_IO_COMPLETION라면 다시 WaitForSingleObjectEx()로 가서 알림 대기상태가 끝날때까지 기다린다.
			else if (result != WAIT_IO_COMPLETION)
				return 1;
		}


		// 접속한 클라 정보 출력
		SOCKADDR_IN clientaddr;
		int addrlen = sizeof(clientaddr);
		getpeername(client_sock, (SOCKADDR*)&clientaddr, &addrlen);

		char str[30];
		inet_ntop(AF_INET, &clientaddr.sin_addr, (char*)&str, sizeof(str));

		printf("\n[TCP Server] Client Connect : IP = %s, Port = %d\n", str, ntohs(clientaddr.sin_port));


		// 소켓 정보 구조체 할당과 초기화
		SOCKETINFO* ptr = new SOCKETINFO;
		if (ptr == nullptr)
			return 1;

		ZeroMemory(&ptr->overlapped, sizeof(ptr->overlapped));
		ptr->sock = client_sock;
		SetEvent(hReadEvent);

		ptr->recvBytes = ptr->sendBytes = 0;
		ptr->wsabuf.buf = ptr->buff;
		ptr->wsabuf.len = BUFF_SIZE;

		// 비동기 입출력 시작
		DWORD recvBytes;
		DWORD flags = 0;
		retval = WSARecv(ptr->sock, &ptr->wsabuf, 1, &recvBytes, &flags, &ptr->overlapped, CompletionRoutine);
		if (retval == SOCKET_ERROR)
			if(WSAGetLastError() != WSA_IO_PENDING)
				return 1;
	}

	return 0;
}

void CALLBACK CompletionRoutine(DWORD dwError, DWORD cbTransferred, LPWSAOVERLAPPED lpOverlapped, DWORD dwFlags)
{
	int retval;

	// 클라 정보얻기
	SOCKETINFO* ptr = (SOCKETINFO*)lpOverlapped;
	SOCKADDR_IN clientaddr;
	int addrlen = sizeof(clientaddr);

	getpeername(ptr->sock, (SOCKADDR*)&clientaddr, &addrlen);

	// 비동기 입출력 결과 확인
	// 뭔가 문제가 발생했다면
	if (dwError != 0 || cbTransferred == 0)
	{
		if (dwError != 0 && dwError != WSAECONNRESET)
			return;

		closesocket(ptr->sock);

		char str[30];
		inet_ntop(AF_INET, &clientaddr.sin_addr, (char*)&str, sizeof(str));

		printf("[TCP Server] Client close : IP = %s, Port = %d\n", str, ntohs(clientaddr.sin_port));

		delete ptr;
		return;
	}

	// 데이터 전송량 갱신
	if (ptr->recvBytes == 0)
	{
		ptr->recvBytes = cbTransferred;
		ptr->sendBytes = 0;

		// 받은 데이터 출력
		ptr->buff[ptr->recvBytes] = '\0';

		char str[30];
		inet_ntop(AF_INET, &clientaddr.sin_addr, (char*)&str, sizeof(str));

		printf("[TCP %s:%d] %s\n", str, ntohs(clientaddr.sin_port), ptr->buff);

	}

	else
		ptr->sendBytes += cbTransferred;


	// 데이터 보내기
	if (ptr->recvBytes > ptr->sendBytes)
	{
		ZeroMemory(&ptr->overlapped, sizeof(ptr->overlapped));
		ptr->wsabuf.buf = ptr->buff + ptr->sendBytes;
		ptr->wsabuf.len = ptr->recvBytes - ptr->sendBytes;


		DWORD sendBytes;
		retval = WSASend(ptr->sock, &ptr->wsabuf, 1, &sendBytes, 0, &ptr->overlapped, CompletionRoutine);

		if (retval == SOCKET_ERROR)
			return;

	}

	else
	{
		ptr->recvBytes = 0;

		// 데이터 받기
		ZeroMemory(&ptr->overlapped, sizeof(ptr->overlapped));
		ptr->wsabuf.buf = ptr->buff;
		ptr->wsabuf.len = BUFF_SIZE;

		DWORD recvBytes;
		DWORD flags = 0;
		retval = WSARecv(ptr->sock, &ptr->wsabuf, 1, &recvBytes, &flags, &ptr->overlapped, CompletionRoutine);
		if (retval == SOCKET_ERROR)
			if (WSAGetLastError() != WSA_IO_PENDING)
				return;
	}

}

