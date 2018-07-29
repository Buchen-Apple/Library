// Project1.cpp: 콘솔 응용 프로그램의 진입점을 정의합니다.
//

#include "stdafx.h"
#pragma comment(lib, "ws2_32")
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <stdlib.h>
#include <process.h>

#define SERVERPORT	9000
#define BUFF_SIZE	512

struct SOCKETINFO
{
	WSAOVERLAPPED overlapped;	// 이 중, 마지막 인자인 hEvent만 사용 (이벤트 핸들)
	SOCKET sock;
	char buf[BUFF_SIZE + 1];
	int recvbytes;
	int sendbytes;
	WSABUF wsabuf;
};

int nTotalSockets = 0;
SOCKETINFO* SocketInfoArray[WSA_MAXIMUM_WAIT_EVENTS];	// 소켓의 정보들 배열 저장소.
WSAEVENT EventArray[WSA_MAXIMUM_WAIT_EVENTS];			// 이벤트 배열 저장소
CRITICAL_SECTION cs;

// 비동기 입출력 처리 함수
UINT WINAPI WorkerThread(VOID* arg);

// 소켓 관리 함수
BOOL AddSocketInfo(SOCKET sock);
void RemoveSocketInfo(int nIndex);


int main()
{
	int retval;
	InitializeCriticalSection(&cs);

	// 윈속 초기화
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return 1;

	// socket()
	SOCKET listen_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (listen_sock == INVALID_SOCKET)
	{
		printf("listen sock / socket() fail\n");
		return 1;
	}

	// bind()
	SOCKADDR_IN serveraddr;
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serveraddr.sin_port = htons(SERVERPORT);
	retval = bind(listen_sock, (SOCKADDR*)&serveraddr, sizeof(serveraddr));
	if (retval == SOCKET_ERROR)
	{
		int error = WSAGetLastError();
		printf("listen sock / bind() fail. ErrorCode : %d\n", error);
		return 1;
	}

	// listen()
	retval = listen(listen_sock, SOMAXCONN);
	if (retval == SOCKET_ERROR)
	{
		int error = WSAGetLastError();
		printf("listen sock / listen() fail. ErrorCode : %d\n", error);
		return 1;
	}

	// 더미 이벤트 객체 생성
	WSAEVENT hEvent = WSACreateEvent();
	if (hEvent == WSA_INVALID_EVENT)
	{
		int error = WSAGetLastError();
		printf("WSACreateEvent() fail. ErrorCode : %d\n", error);
		return 1;
	}
	EventArray[nTotalSockets++] = hEvent;

	// 스레드 생성
	HANDLE hThread = (HANDLE)_beginthreadex(NULL, 0, WorkerThread, NULL, 0, NULL);
	if (hThread == NULL)
	{
		printf("_beginthreadex() fail\n");
		return 1;
	}	

	CloseHandle(hThread);


	// 데이터 통신에 사용할 변수
	SOCKET client_sock;
	SOCKADDR_IN clientaddr;
	int addrlen;
	DWORD recvbyte, flags;

	while (1)
	{
		//accept
		addrlen = sizeof(clientaddr);
		client_sock = accept(listen_sock, (SOCKADDR*)&clientaddr, &addrlen);	// 입력 받기 전 까지는 블락상태.
		if (listen_sock == INVALID_SOCKET)
		{
			printf("listen_sock / accept() fail\n");
			break;
		}
		TCHAR TextBuff[30];
		InetNtop(AF_INET, (SOCKADDR*)&clientaddr.sin_addr, TextBuff, sizeof(TextBuff));
		_tprintf(_T("Join....IP : %s, Port : %d\n"), TextBuff, ntohs(clientaddr.sin_port));

		// 소켓 정보 추가
		if (AddSocketInfo(client_sock) == FALSE)
		{
			closesocket(client_sock);
			_tprintf(_T("Exit....IP : %s, Port : %d\n"), TextBuff, ntohs(clientaddr.sin_port));
			continue;
		}

		// 비동기 입출력 시작
		SOCKETINFO* ptr = SocketInfoArray[nTotalSockets - 1];
		flags = 0;
		retval = WSARecv(ptr->sock, &ptr->wsabuf, 1, &recvbyte, &flags, &ptr->overlapped, NULL);
		if (retval == SOCKET_ERROR)
		{
			// 에러라도 WSA_IO_PENDING는, 지금 즉시 할게 없다는 뜻이다.
			// 즉, WSA_IO_PENDING는 사실 에러로 취급하지 않아도 된다.
			if (WSAGetLastError() != WSA_IO_PENDING)
			{
				printf("WSARecv() fail. ErrorCode : %d\n", WSAGetLastError());
				RemoveSocketInfo(nTotalSockets - 1);
				continue;
			}
			
		}

		//소켓의 개수 변화를 알림 (nTotalSockets)
		WSASetEvent(EventArray[0]);

	}

	WSACleanup();
	DeleteCriticalSection(&cs);
    return 0;
}

// 비동기 입출력 처리 함수(스레드)
UINT WINAPI WorkerThread(VOID* arg)
{
	int retval;

	while (1)
	{
		// 이벤트 관찰
		DWORD index = WSAWaitForMultipleEvents(nTotalSockets, EventArray, FALSE, WSA_INFINITE, FALSE);	// 입력을 받아서 깨어나기 전까지는 여기서 블락상태.

		// WSA_WAIT_FAILED에러는 WSAWaitForMultipleEvents()함수의 실패 시 반환되는 에러.
		if (index == WSA_WAIT_FAILED)
			continue;

		// WSAWaitForMultipleEvents()는 해당 소켓의 인덱스 + WSA_WAIT_EVENT_0를 반환하기 때문에, WSA_WAIT_EVENT_0를 빼야 진짜 인덱스를 알 수 있다.
		index -= WSA_WAIT_EVENT_0;

		// 해당 이벤트를 다시 '비신호' 상태로 만든다. 그래야 다음에 또 받을거 있나 확인할 수 있으니.
		WSAResetEvent(EventArray[index]);

		// 만약, index가 0이라면 그냥 소켓 개수 변화를 알려준 것. 다시 continue;
		if (index == 0)
			continue;

		// 클라 정보 얻기
		SOCKETINFO* ptr = SocketInfoArray[index];
		SOCKADDR_IN clientaddr;
		int addrlen = sizeof(clientaddr);
		getpeername(ptr->sock, (SOCKADDR*)&clientaddr, &addrlen);

		// 비동기 입출력 결과 확인
		DWORD cbTransferred, flags;
		retval = WSAGetOverlappedResult(ptr->sock, &ptr->overlapped, &cbTransferred, FALSE, &flags);
		if (retval == FALSE || cbTransferred == 0)
		{
			// retval == FALSE면 뭔가 오류 발생. cbTransferred == 0면 정상종료.
			TCHAR TextBuff[30];
			InetNtop(AF_INET, (SOCKADDR*)&clientaddr.sin_addr, TextBuff, sizeof(TextBuff));			
			_tprintf(_T("Exit....IP : %s, Port : %d\n"), TextBuff, ntohs(clientaddr.sin_port));
			RemoveSocketInfo(index);
			continue;
		}

		// 데이터 전송량 갱신
		// ptr->recvbytes == 0이면, 이전에 받은 데이터를 send했다는 것(242번쨰 줄) 이제 데이터를 받으면 된다.
		if (ptr->recvbytes == 0)
		{			
			ptr->recvbytes = cbTransferred;
			ptr->sendbytes = 0;

			// 받은 데이터 출력
			ptr->buf[ptr->recvbytes] = '\0';
			TCHAR TextBuff[30];
			InetNtop(AF_INET, (SOCKADDR*)&clientaddr.sin_addr, TextBuff, sizeof(TextBuff));
			_tprintf(_T("[%s / %d] : "), TextBuff, ntohs(clientaddr.sin_port));
			printf("%s\n", ptr->buf);
		}

		// ptr->recvbytes == 0이 아니면, 보냈다는 것이니 보낸 데이터 갱신
		else
			ptr->sendbytes += cbTransferred;


		// 데이터 갱신 후, 받은 데이터 > 보낸 데이터라면, 아직 데이터를 덜 보냈다는 뜻이니 데이터를 보낸다.
		if (ptr->recvbytes > ptr->sendbytes)
		{
			ZeroMemory(&ptr->overlapped, sizeof(ptr->overlapped));
			ptr->overlapped.hEvent = EventArray[index];
			ptr->wsabuf.buf = ptr->buf + ptr->sendbytes;		// 보낸 데이터만큼 >>로 이동시켜서, 이어서 보내도록 한다.
			ptr->wsabuf.len = ptr->recvbytes - ptr->sendbytes;	// 받은 데이터 - 보낸 데이터를 하면, 보내야하는 남은 데이터가 나온다.

			DWORD sendbytes;
			flags = 0;
			retval = WSASend(ptr->sock, &ptr->wsabuf, 1, &sendbytes, flags, &ptr->overlapped, NULL);

			if (retval == SOCKET_ERROR)
			{
				// 에러라도 WSA_IO_PENDING는, 지금 즉시 할게 없다는 뜻이다.
				// 즉, WSA_IO_PENDING는 사실 에러로 취급하지 않아도 된다.
				if (WSAGetLastError() != WSA_IO_PENDING)
					printf("WSASend() fail. ErrorCode : %d\n", WSAGetLastError());

				continue;
			}
		}

		// 데이터 갱신 후, 받은 데이터 < 보낸 데이터라면, 아직 데이터를 덜 받았다는 뜻이니 데이터를 받는다.
		else
		{
			ptr->recvbytes = 0;

			ZeroMemory(&ptr->overlapped, sizeof(ptr->overlapped));
			ptr->overlapped.hEvent = EventArray[index];
			ptr->wsabuf.buf = ptr->buf;
			ptr->wsabuf.len = BUFF_SIZE;

			DWORD recvbyte;
			flags = 0;
			retval = WSARecv(ptr->sock, &ptr->wsabuf, 1, &recvbyte, &flags, &ptr->overlapped, NULL);
			if (retval == SOCKET_ERROR)
			{
				if (WSAGetLastError() != WSA_IO_PENDING)
					printf("WSARecv() fail. ErrorCode : %d\n", WSAGetLastError());

				continue;
			}
		}
	}
}

// 소켓 정보 등록
BOOL AddSocketInfo(SOCKET sock)
{
	EnterCriticalSection(&cs);
	if (nTotalSockets >= WSA_MAXIMUM_WAIT_EVENTS)
	{
		printf("WSA_MAXIMUM_WAIT_EVENTS\n");
		return FALSE;
	}

	SOCKETINFO* ptr = new SOCKETINFO;
	if (ptr == NULL)
	{
		printf("AddSocketInfo() / new SOCKETINFO fail.\n");
		return FALSE;
	}

	WSAEVENT hEvent = WSACreateEvent();
	if(hEvent == WSA_INVALID_EVENT)
	{
		printf("AddSocketInfo() / WSACreateEvent() fail.\n");
		return FALSE;
	}

	ZeroMemory(&ptr->overlapped, sizeof(ptr->overlapped));
	ptr->overlapped.hEvent = hEvent;
	ptr->sock = sock;
	ptr->recvbytes = ptr->sendbytes = 0;
	ptr->wsabuf.buf = ptr->buf;
	ptr->wsabuf.len = BUFF_SIZE;
	SocketInfoArray[nTotalSockets] = ptr;
	EventArray[nTotalSockets] = hEvent;
	++nTotalSockets;

	LeaveCriticalSection(&cs);
	return TRUE;

}

// 소켓 정보 삭제
void RemoveSocketInfo(int nIndex)
{
	EnterCriticalSection(&cs);

	SOCKETINFO* ptr = SocketInfoArray[nIndex];
	closesocket(ptr->sock);
	delete ptr;
	WSACloseEvent(EventArray[nIndex]);

	if (nIndex != (nTotalSockets - 1))
	{
		SocketInfoArray[nIndex] = SocketInfoArray[nTotalSockets - 1];
		EventArray[nIndex] = EventArray[nTotalSockets - 1];
	}

	--nTotalSockets;

	LeaveCriticalSection(&cs);
}

