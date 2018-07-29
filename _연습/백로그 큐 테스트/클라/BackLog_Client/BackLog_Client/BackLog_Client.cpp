// BackLog_Client.cpp: 콘솔 응용 프로그램의 진입점을 정의합니다.
//

#include "stdafx.h"
#pragma comment(lib, "ws2_32")
#include <WS2tcpip.h>


int _tmain()
{
	// 윈속 초기화
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
	{
		printf("Error_1(%d)\n", WSAGetLastError());
		return 0;
	}
	

	// connect 준비
	SOCKADDR_IN clientaddr;
	clientaddr.sin_family = AF_INET;
	clientaddr.sin_port = htons(20000);
	InetPton(AF_INET, L"127.0.0.1", &clientaddr.sin_addr.s_addr);

	int Count = 0;
	while (1)
	{
		// 소켓 생성
		SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (sock == INVALID_SOCKET)
		{
			printf("Error_2(%d)\n", WSAGetLastError());
			return 0;
		}

		// 커넥트
		int Check = connect(sock, (SOCKADDR*)&clientaddr, sizeof(clientaddr));
		if (Check == SOCKET_ERROR)
		{
			printf("Error_3(%d)\n", WSAGetLastError());
			break;
		}

		closesocket(sock);
		Count++;
	}

	printf("%d\n", Count);
	


	// 클로즈
	
	WSACleanup();

    return 0;
}

