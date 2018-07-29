// BackLog_Server.cpp: 콘솔 응용 프로그램의 진입점을 정의합니다.
//

#include "stdafx.h"
#pragma comment(lib, "ws2_32")
#include <WS2tcpip.h>

#define PORT	20000


int _tmain()
{
	// 윈속 초기화
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
	{
		printf("Error_1\n");
		return 0;
	}

	// 소켓 생성
	SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sock == INVALID_SOCKET)
	{
		printf("Error_2\n");
		return 0;
	}


	// 논블락으로 할 경우, 논블락으로 변경

	// 바인딩
	SOCKADDR_IN serveraddr;
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_port = htons(PORT);
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);

	int check = bind(sock, (SOCKADDR*)&serveraddr, sizeof(serveraddr));
	if (check == SOCKET_ERROR)
	{
		printf("Error_3\n");
		return 0;
	}

	// 리슨
	//check = listen(sock, SOMAXCONN);
	check = listen(sock, 5);
	if (check == SOCKET_ERROR)
	{
		printf("Error_4\n");
		return 0;
	}

	// Accept()
	/*SOCKADDR_IN clientaddr;
	int len = sizeof(clientaddr);
	SOCKET clientsock = accept(sock, (SOCKADDR*)&clientaddr, &len);

	if (clientsock == INVALID_SOCKET)
	{
		printf("Error_5\n");
		return 0;
	}*/

	_gettch();

	


	// closesocket()

	// WSACleanup()

	closesocket(sock);
	WSACleanup();


    return 0;
}

