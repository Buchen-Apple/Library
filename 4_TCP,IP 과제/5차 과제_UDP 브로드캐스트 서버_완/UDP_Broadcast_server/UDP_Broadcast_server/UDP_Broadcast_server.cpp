#pragma comment(lib, "ws2_32")
#include <stdio.h>
#include <WS2tcpip.h>
#include <locale.h>
#include <tchar.h>

int _tmain()
{
	setlocale(LC_ALL, "korean");
	TCHAR tRoomName[30];
	DWORD dPortNumber;
	int retval;
	UINT size = sizeof(tRoomName);

	// ��Ʈ�� �� �̸� �Է¹ޱ�
	_tprintf(_T("�� �̸� �Է� : "));
	_tscanf_s(_T("%s"), tRoomName, size);

	_tprintf(_T("��Ʈ : "));
	_tscanf_s(_T("%d"), &dPortNumber);

	// ���� �ʱ�ȭ
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2,2), &wsa) != 0)
	{
		_tprintf(_T("���� �ʱ�ȭ ����\n"));
		return 1;
	}

	// ���� ����
	SOCKET sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (sock == INVALID_SOCKET)
	{
		_tprintf(_T("socket() ����\n"));
		return 1;
	}

	// ���ε�
	SOCKADDR_IN recvaddr;
	ZeroMemory(&recvaddr, sizeof(recvaddr));
	recvaddr.sin_family = AF_INET;
	recvaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	recvaddr.sin_port = htons(dPortNumber);

	retval = bind(sock, (SOCKADDR*)&recvaddr, sizeof(recvaddr));
	if (retval == SOCKET_ERROR)
	{
		_tprintf(_T("bind() ����\n"));
		return 1;
	}
	
	BYTE bRecvBuff[10];
	BYTE bCheckBuff[10] = {0xff, 0xee, 0xdd, 0xaa, 0x00, 0x99, 0x77, 0x55, 0x33, 0x11};
	SOCKADDR_IN peeraddr;
	while (1)
	{
		// ������ ���ú� ���
		int addrlen = sizeof(peeraddr);
		retval = recvfrom(sock, (char*)bRecvBuff, sizeof(bRecvBuff), 0, (SOCKADDR*)&peeraddr, &addrlen);
		if (retval == SOCKET_ERROR)
		{
			_tprintf(_T("recvfrom() ���� : %d\n"), WSAGetLastError());
			return 1;
		}

		// ���ú� ��, ����� �´ٸ� ���� ��뿡�� �� �̸� ����
		if (memcmp(bRecvBuff, bCheckBuff, sizeof(bCheckBuff)) == 0)
		{
			retval = sendto(sock, (char*)tRoomName, _tcslen(tRoomName) *2, 0, (SOCKADDR*)&peeraddr, sizeof(peeraddr));
			if (retval == SOCKET_ERROR)
			{
				_tprintf(_T("sendto() ����\n"));
				return 1;
			}
			TCHAR tIPBuff[30];
			InetNtop(AF_INET, &peeraddr.sin_addr, tIPBuff, sizeof(tIPBuff));

			_tprintf(_T("[%s:%d] sendto ����!\n"), tIPBuff, ntohs(peeraddr.sin_port));
		}	

	}

	closesocket(sock);
	WSACleanup();
	return 0;
}