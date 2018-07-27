#pragma comment(lib, "ws2_32")
#include <stdio.h>
#include <WS2tcpip.h>
#include <locale.h>
#include <tchar.h>

#define BUFSIZE 512
#define PORT 20000

int _tmain()
{
	_tsetlocale(LC_ALL, _T("korea"));
	int retval;

	// ���� �ʱ�ȭ
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
	{
		_tprintf(_T("���� �ʱ�ȭ ����\n"));
		return 1;
	}

	// socket()
	SOCKET sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if(sock == INVALID_SOCKET)
	{
		_tprintf(_T("socket() ����\n"));
		return 1;
	}

	// ��ε� ĳ���� Ȱ��ȭ
	BOOL bEnable = TRUE;
	retval = setsockopt(sock, SOL_SOCKET, SO_BROADCAST, (char*)&bEnable, sizeof(bEnable));
	if (retval == SOCKET_ERROR)
	{
		_tprintf(_T("��ε�ĳ���� Ȱ��ȭ ����\n"));
		return 1;
	}

	// ���� ����ü �ʱ�ȭ
	SOCKADDR_IN remoteaddr;	
	ULONG IPBuff;
	ZeroMemory(&remoteaddr, sizeof(remoteaddr));

	InetPton(AF_INET, _T("255.255.255.255"), &IPBuff);	
	remoteaddr.sin_family = AF_INET;	
	remoteaddr.sin_addr.s_addr = IPBuff;
	remoteaddr.sin_port = htons(PORT);

	// ������ ��ſ� ����� ����
	DWORD count = 0;
	DWORD count2 = 0;
	SOCKADDR_IN recvaddr;
	int addrlen;
	BYTE protocol[10] = {0xff, 0xee, 0xdd, 0xaa, 0x00, 0x99, 0x77, 0x55, 0x33, 0x11};

	FD_SET rset;
	TIMEVAL tval;
	tval.tv_sec = 0;
	tval.tv_usec = 200000;

	// ��ε� ĳ��Ʈ ������ ������
	while (count < 10)
	{		
		TCHAR IP[30];

		// sendto()
		retval = sendto(sock, (const char*)protocol, 10, 0, (SOCKADDR*)&remoteaddr, sizeof(remoteaddr));
		if (retval == SOCKET_ERROR)
		{
			_tprintf(_T("sendto ����\n"));
			return 1;
		}

		// select()
		FD_ZERO(&rset);
		FD_SET(sock, &rset);
		retval = select(0, &rset, 0, 0, &tval);
		ZeroMemory(&recvaddr, sizeof(recvaddr));

		// recvfrom()
		if (FD_ISSET(sock, &rset))
		{
			TCHAR Name[30];
			addrlen = sizeof(recvaddr);
			retval = recvfrom(sock, (char*)Name, sizeof(Name), 0, (SOCKADDR*)&recvaddr, &addrlen);			
			if (retval == SOCKET_ERROR)
			{
				_tprintf(_T("recvfrom fail\n"));
				return 1;			
			}	
			
			InetNtop(AF_INET, &recvaddr.sin_addr, IP, sizeof(IP));
			Name[retval/2] = '\0';
			_tprintf(_T("\nName : %s, IP : %s, Port : %d\n"), Name, IP, ntohs(recvaddr.sin_port));

			count++;			
		}
		count2++;
		remoteaddr.sin_port = htons(PORT + count2);
		fputc('.', stdout);
		
	}

	closesocket(sock);
	WSACleanup();
	return 0;
}