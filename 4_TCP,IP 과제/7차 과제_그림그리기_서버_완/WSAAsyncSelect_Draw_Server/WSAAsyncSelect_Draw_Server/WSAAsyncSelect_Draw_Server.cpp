#pragma comment(lib, "ws2_32")
#include <tchar.h>
#include <stdio.h>
#include <WS2tcpip.h>
#include <locale.h>
#include "RingBuff.h"

#define SERVERPORT 25000
#define USERMAX	500
#define WM_SOCK_ACCEPT (WM_USER+1)
#define WM_SOCK (WM_USER+2)

// ���� ����ü
// ���� : ����� ����� ��ſ� ���Ǵ� ����ü
struct Session
{
	SOCKET m_Sock;
	CRingBuff* m_RecvBuff;
	CRingBuff* m_SendBuff;
	TCHAR m_ClientIP[30];
	DWORD m_ClinetPort;
	BOOL m_SendFlag;
};

// ���̷ε�
#pragma pack(push, 1)
struct st_Draw_Packet
{
	int iStartX;
	int iStartY;
	int iEndX;
	int iEndY;
};
#pragma pack(pop)

// �ʿ� ��ɵ� �Լ�
BOOL AddUser(SOCKET sock, SOCKADDR_IN);		// Ŭ�� �߰�
BOOL ProcClose(SOCKET);						// Ŭ�� ����
int IndexSearch(SOCKET);					// ������ �ѱ�� SessionList�� �ε����� ã���ִ� �Լ�.
BOOL ProcRead(SOCKET);						// ������ �ޱ�
BOOL ProcWrite(SOCKET);						// ������ ������
BOOL ProcWriteBroadCast(SOCKET);			// ��ε� ĳ��Ʈ�� ������. ��� �������� ������. (������ ��Ŷ�� ���� ���� ���� �̷��� ����. �� �� ����)
int SendPacket(char*, int, Session*);		// ���ú� ť�� �����͸� ���� ť�� �����Ѵ�.
BOOL ProcPacket(SOCKET);					// ���ú� ť�� ������ ó��

// ���� ����
HINSTANCE hInst;	// ���� �ν��Ͻ��Դϴ�.
SOCKET listen_sock;	// ��������
Session* SessionList[USERMAX];
int dAcceptUserCount = 0;		// Accept�� ���� ��

LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);

int APIENTRY _tmain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	setlocale(LC_ALL, "korean");

	// TODO: ���⿡ �ڵ带 �Է��մϴ�.

	// ���� ���ڿ��� �ʱ�ȭ�մϴ�.
	WNDCLASSEXW wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(NULL));
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = MAKEINTRESOURCEW(NULL);
	wcex.lpszClassName = TEXT("WSAAsyncSelect");
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(NULL));

	RegisterClassExW(&wcex);

	hInst = hInstance; // �ν��Ͻ� �ڵ��� ���� ������ �����մϴ�.

	HWND hWnd = CreateWindowW(TEXT("WSAAsyncSelect"), TEXT("6������"), WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

	if (!hWnd)
		return FALSE;

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	// �����ʱ�ȭ
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
	{
		_tprintf(_T("���� �ʱ�ȭ ����\n"));
		return 1;
	}

	// socket()
	listen_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (listen_sock == INVALID_SOCKET)
	{
		_tprintf(_T("�������� socket() ����\n"));
		return 1;
	}

	// bind()
	SOCKADDR_IN serveraddr;
	ZeroMemory(&serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = INADDR_ANY;
	serveraddr.sin_port = htons(SERVERPORT);
	int retval = bind(listen_sock, (SOCKADDR*)&serveraddr, sizeof(serveraddr));
	if (retval == SOCKET_ERROR)
	{
		_tprintf(_T("bind() ����\n"));
		return 1;
	}

	// listen()
	retval = listen(listen_sock, SOMAXCONN);
	if (retval == SOCKET_ERROR)
	{
		_tprintf(_T("listen() ����\n"));
		return 1;
	}

	// listen_sock������ WSAAsyncSelect()
	retval = WSAAsyncSelect(listen_sock, hWnd, WM_SOCK_ACCEPT, FD_ACCEPT);
	if (retval == SOCKET_ERROR)
	{
		_tprintf(_T("�������� WSAAsyncSelect() ����\n"));
		return 1;
	}

	_tprintf(_T("���� �غ� �Ϸ�\n"));

	// �޽��� ����.
	MSG msg;

	while (GetMessage(&msg, nullptr, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	WSACleanup();
	closesocket(listen_sock);
	return (int)msg.wParam;
}


// �� ����
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		int addrlen;
		int retval;

	case WM_SOCK_ACCEPT:
		// ���� ���� üũ
		if (WSAGETSELECTERROR(lParam))
		{
			_tprintf(_T("WM_SOCK_ACCEPT ���� �߻� : %d\n"), WSAGETSELECTERROR(lParam));
			ProcClose(wParam);	// ������ �߻��ϸ�, �ش� ������ ������ ���´�.
			break;
		}

		// accept() �޽��� ó��
		SOCKADDR_IN clientaddr;
		SOCKET clinet_sock;
		addrlen = sizeof(clientaddr);
		clinet_sock = accept(listen_sock, (SOCKADDR*)&clientaddr, &addrlen);
		if (clinet_sock == INVALID_SOCKET)
		{
			_tprintf(_T("accpet() ���� �߻�!\n"));
			break;
		}

		// ������ Ŭ���̾�Ʈ�� ���� �߰� 
		retval = AddUser(clinet_sock, clientaddr);
		if (retval == FALSE)
			break;

		// ������ Ŭ���̾�Ʈ WSAAsyncSelect()
		retval = WSAAsyncSelect(clinet_sock, hWnd, WM_SOCK, FD_READ | FD_WRITE | FD_CLOSE | FD_CONNECT);
		if (retval == SOCKET_ERROR)
		{
			_tprintf(_T("������ Ŭ���̾�Ʈ WSAAsyncSelect() �����߻�!\n"));
			break;
		}

		break;

	case WM_SOCK:
		// ���� ���� üũ
		if (WSAGETSELECTERROR(lParam))
		{
			// 10053�� ������ ���� ������¶�� ���̴�. ��, Ŭ�� ���� �����Ѱ��̴� ���� ������ �ƴ�. �׳� ������ ��
			if (WSAGETSELECTERROR(lParam) != 10053)
				_tprintf(_T("WM_SOCK ���� �߻� : %d\n"), WSAGETSELECTERROR(lParam));

			ProcClose(wParam);
			break;
		}

		// �޽��� ó��
		switch (WSAGETSELECTEVENT(lParam))
		{
		case FD_READ:
			ProcRead(wParam);
			ProcPacket(wParam);
			break;

		case FD_WRITE:
			// �ش� ������ send ���� ���°� �Ǿ���. �ش� ������ sendFlag�� TRUE�� �����.
		{
			int iIndex = IndexSearch(wParam);
			if (iIndex == -1)
				break;

			SessionList[iIndex]->m_SendFlag = TRUE;

			// �׸���, Ȥ�� Write�Ұ� ������ �Ѵ�.
			BOOL Check = ProcWriteBroadCast(wParam);
			if (Check == FALSE)
				break;	// ������ FALSE�� ���͵� ���� �Ұ� ����..
		}

		break;

		case FD_CLOSE:
			ProcClose(wParam);
			break;

		default:
			_tprintf(_T("�� �� ���� ��Ŷ\n"));
			break;
		}
		break;

	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

// �ε��� ã��
int IndexSearch(SOCKET sock)
{
	// �ش� ������ ������ ����Ʈ�� �� �� �ε��� �������� ã�Ƴ���.
	int iIndex = 0;

	while (1)
	{
		if (SessionList[iIndex]->m_Sock == sock)
			break;

		iIndex++;

		if (iIndex >= dAcceptUserCount)
		{
			_tprintf(_T("���� �����Դϴ�\n"));
			return -1;
		}
	}

	return iIndex;
}

// ������ ���� �߰�
BOOL AddUser(SOCKET sock, SOCKADDR_IN clientaddr)
{
	// ���ο� ���� ������ ������ �޸� ����
	Session* newUser = new Session;
	if (newUser == NULL)
	{
		_tprintf(_T("AddUser() �Լ� ����! �޸� ���� ����\n"));
		return FALSE;
	}

	// ���� ���� �� üũ. ���� ���� Max�� �� �̻� ���� �߰� �Ұ���
	if (dAcceptUserCount == USERMAX)
	{
		_tprintf(_T("AddUser() �Լ� ����! ������ ���� á���ϴ�.\n"));
		return FALSE;
	}

	// IP �ּ� ��ȯ
	TCHAR tIPBuff[30];
	InetNtop(AF_INET, &clientaddr.sin_addr, tIPBuff, sizeof(tIPBuff));

	// ���ο� ���� ���� ����
	newUser->m_Sock = sock;
	newUser->m_SendFlag = FALSE;
	_tcscpy_s(newUser->m_ClientIP, _countof(newUser->m_ClientIP), tIPBuff);
	newUser->m_ClinetPort = ntohs(clientaddr.sin_port);
	newUser->m_RecvBuff = new CRingBuff;
	newUser->m_SendBuff = new CRingBuff;

	// ����Ʈ�� �߰�
	SessionList[dAcceptUserCount++] = newUser;

	_tprintf(_T("[%s:%d] ����!\n"), newUser->m_ClientIP, newUser->m_ClinetPort);

	return TRUE;
}

// ���� ����
BOOL ProcClose(SOCKET sock)
{
	// ����� �� ������ ����Ʈ�� �� �� �ε��� �������� ã�Ƴ���.
	int iIndex = IndexSearch(sock);
	if (iIndex == -1)
		return FALSE;

	// ã�� ������ RemoveUser�� ����
	Session* RemoveUser = SessionList[iIndex];
	_tprintf(_T("[%s:%d] ���� ����!\n"), RemoveUser->m_ClientIP, RemoveUser->m_ClinetPort);

	// ��� ������ �����Ѵ�.
	delete RemoveUser;

	// ����Ʈ�� ��ĭ�� ä���.
	if (iIndex < dAcceptUserCount - 1)
		SessionList[iIndex] = SessionList[dAcceptUserCount - 1];

	// �������� ���� �� 1 ����
	dAcceptUserCount--;

	return TRUE;
}

// ������ �ޱ�
BOOL ProcRead(SOCKET sock)
{
	DWORD retval;

	// �����͸� �޾ƾ� �� ������ ����Ʈ�� �� �� �ε��� �������� ã�Ƴ���.
	int iIndex = IndexSearch(sock);
	if (iIndex == -1)
		return FALSE;

	// ��� ������ recvUser�� ����
	Session* recvUser = SessionList[iIndex];

	// recv()
	while (1)
	{
		// �ӽ� ���۷� recv()
		char Buff[1000];
		retval = recv(recvUser->m_Sock, Buff, sizeof(Buff), 0);

		// ������ �߻��ߴٸ� ���� ó��
		if (retval == SOCKET_ERROR)
		{
			// WSAEWOULDBLOCK������ �߻��ϸ�, ���� ���۰� ����ְų� ������ recv �Ұ��� ����. recv�Ұ� ���ٴ� ���̴� while�� ����
			if (WSAGetLastError() == WSAEWOULDBLOCK)
				break;

			// WSAEWOULDBLOCK������ �ƴϸ� �׳� ���´�. ���� �̻��ѳ��̴�.
			else
			{
				_tprintf(_T("ProcRead(). recv ��, �̻��� ����\n"));
				ProcClose(recvUser->m_Sock);
				return FALSE;
			}
		}
		else if (retval == 0)
		{
			_tprintf(_T("ProcRead(). ���� ����� ����\n"));
			ProcClose(recvUser->m_Sock);
			return FALSE;
		}

		// recv()�� �����͸� �ش� ������ recvBuff�� �����Ѵ�.
		DWORD BuffArray = 0;

		while (retval > 0)
		{
			int EnqueueCheck = recvUser->m_RecvBuff->Enqueue(&Buff[BuffArray], retval);

			// Enqueue() �Լ���, ���۰� ���� á���� -1�� �����Ѵ�.
			// ���۰� ������ ������ ���� �̻��� ���̴� �׳� ���´�.
			if (EnqueueCheck == -1)
			{
				_tprintf(_T("ProcRead(). ���۰� ���� �� ����\n"));
				ProcClose(recvUser->m_Sock);
				return FALSE;
			}

			retval -= EnqueueCheck;
			BuffArray += EnqueueCheck;
		}
	}

	return TRUE;
}

// ������ ������
BOOL ProcWrite(SOCKET sock)
{
	// �����͸� ���� ������ ã�Ƴ���. ����Ʈ�� �� �� �ε��� �������� ã�Ƴ���.
	int iIndex = IndexSearch(sock);
	if (iIndex == -1)
		return FALSE;

	// �ش� ������ SendFlag�� FALSE��� ���� �Ұ��� ����.
	if (SessionList[iIndex]->m_SendFlag == FALSE)
	{
		_tprintf(_T("ProcWrite(). m_SendFlag�� FALSE�� ����\n"));
		return FALSE;
	}

	// ��� ������ recvUser�� ����
	Session* sendUser = SessionList[iIndex];


	// ���� ��Ŷ�� �����͸�, ��뿡�� send()
	int BuffArray = 0;
	int iSize = sendUser->m_SendBuff->GetUseSize();

	while (iSize > 0)
	{
		// �ϴ� �ӽ� ���۷� ������ ����(�� ���� ��ť�� �ƴ϶� Peek)
		char Buff[1000];
		int PeekSize;
		PeekSize = sendUser->m_SendBuff->Peek(&Buff[BuffArray], iSize);
		if (PeekSize == -1)
		{
			_tprintf(_T("ProcWrite(). ���� ť�� ����ִ� ����\n"));
			return FALSE;
		}

		iSize -= PeekSize;
		BuffArray += PeekSize;

		// �ӽ� ������ �����͸� ��뿡�� send
		int sendCheck = send(sendUser->m_Sock, Buff, PeekSize, 0);

		// ������ �߻�������
		if (sendCheck == SOCKET_ERROR)
		{
			// �������� üũ. �����̶��, Send�Ұ��� ���°� �� ���̴�(�� ���� ���۳� ��� ���� ���۰� ���� �� ��Ȳ?)
			// ���� �÷��׸� FASLE�� ����� ������.
			if (WSAGetLastError() == WSAEWOULDBLOCK)
			{
				_tprintf(_T("ProcWrite(). WSAEWOULDBLOCK�� �� ����\n"));
				sendUser->m_SendFlag = FALSE;
				return FALSE;
			}

			// �� �� ������ �׳� ���´�.
			ProcClose(sendUser->m_Sock);
		}

		// ���忡 ���������� ������ ������ ��ŭ Remove�Ѵ�
		sendUser->m_SendBuff->RemoveData(sendCheck);
	}

	return TRUE;
}

// ��ε� ĳ��Ʈ�� ������ ������
BOOL ProcWriteBroadCast(SOCKET sock)
{
	// � ������ �����͸� ���������� ã�Ƴ���.
	int iIndex = IndexSearch(sock);
	if (iIndex == -1)
		return FALSE;

	// ��� ������ sendUser�� ����
	Session* sendUser = SessionList[iIndex];

	// ���� �� ����� ���Ѵ�. (���� ���ۿ� �ִ� ��� ������ �� ������)
	int iSize = sendUser->m_SendBuff->GetUseSize();
	int BuffArray = 0;

	// ��ε� ĳ��Ʈ
	while (iSize > 0)
	{
		// �ϴ� �ӽ� ���۷� ������ ����(�� ���� ��ť�� �ƴ϶� Peek)
		char Buff[1000];
		int PeekSize = sendUser->m_SendBuff->Peek(&Buff[BuffArray], iSize);
		if (PeekSize == -1)
		{
			_tprintf(_T("ProcWrite(). ���� ť�� ����ִ� ����\n"));
			return FALSE;
		}

		iSize -= PeekSize;
		BuffArray += PeekSize;		

		// �̹��� Peek �� �����͸� ��� �������� ������ ������.
		for (int i = 0; i < dAcceptUserCount; ++i)
		{
			// �ش� ������ SendFlag�� FALSE��� ���� �Ұ��� ����.
			if (SessionList[i]->m_SendFlag == FALSE)
			{
				_tprintf(_T("ProcWrite(). m_SendFlag�� FALSE�� ����\n"));
				continue;
			}

			// ���� 1�� ����
			Session* goalUser = SessionList[i];

			// �ӽ� ���ۿ� �ִ� �����͸�, ���� 1���� send()
			int BuffArray = 0;
			int TempPeekSize = PeekSize;

			while (TempPeekSize > 0)
			{
				int sendCheck = send(goalUser->m_Sock, &Buff[BuffArray], TempPeekSize, 0);

				// ������ �߻�������
				if (sendCheck == SOCKET_ERROR)
				{
					// �������� üũ. �����̶��, Send�Ұ��� ���°� �� ���̴�(�� ���� ���۳� ��� ���� ���۰� ���� �� ��Ȳ?)
					// ���� �÷��׸� FASLE�� ����� ������.
					if (WSAGetLastError() == WSAEWOULDBLOCK)
					{
						_tprintf(_T("ProcWrite(). WSAEWOULDBLOCK�� �� ����\n"));
						sendUser->m_SendFlag = FALSE;
						continue;
					}

					// �� �� ������ �׳� ���´�.
					ProcClose(goalUser->m_Sock);
				}

				TempPeekSize -= sendCheck;
				BuffArray += sendCheck;
			}	
		}

		// ��� �������� ������, ������ Peek �� ������ ��ŭ Remove�Ѵ�
		sendUser->m_SendBuff->RemoveData(PeekSize);
	}	

	return TRUE;

}

// char�� �����͸� size��ŭ Session�� ���� ���ۿ� �ֱ�.
// ��, �����͸� ����ť�� �̵�
// ��ȯ�� : -1(���۰� ������ ����), 0(���� ����)
int SendPacket(char* Buff, int iSize, Session* User)
{
	// Buff�� �����͸� ���� ť�� �̵���Ų��.
	// Buff�� �� ��������.
	int BuffArray = 0;

	while (iSize > 0)
	{
		// Buff���� iSize��ŭ User�� ����ť�� ��ť�Ѵ�.
		int realEnqeueuSize = User->m_SendBuff->Enqueue(&Buff[BuffArray], iSize);

		// Enqueue() �Լ���, ���۰� ���� á���� -1�� �����Ѵ�.
		// ���۰� ������ ������ ���� �̻��� ���̴� �׳� ���´�.
		if (realEnqeueuSize == -1)
		{
			_tprintf(_T("ProcRead(). ���۰� ���� �� ����\n"));
			ProcClose(User->m_Sock);
			return -1;
		}
		iSize -= realEnqeueuSize;
		BuffArray += realEnqeueuSize;
	}

	return 0;
}

// ���ú� ť�� ������ ó��
BOOL ProcPacket(SOCKET sock)
{
	// �����͸� ó���� ������ ã�Ƴ���.����Ʈ�� �� �� �ε��� �������� ã�Ƴ���.
	int iIndex = IndexSearch(sock);
	if (iIndex == -1)
		return FALSE;

	// User �������ٰ�, ���� ó���� ������ ��´�.
	Session* User = SessionList[iIndex];

	// ������ ���ú� ť�� ���� �� ��� Ȯ��. ����, ������� ���̷ε��� ����� ����ִ�.
	unsigned short header = 0;
	int Check = User->m_RecvBuff->Peek((char*)&header, 2);
	if (Check == -1)
	{
		_tprintf(_T("ProcPacket(). ���ú� ť�� �������\n"));
		return FALSE;
	}

	while (1)
	{
		// ��Ŷ 1���� �ϼ��� �� ������ �׶� ��Ŷ ó��
		if (User->m_RecvBuff->GetUseSize() >= header + 2)
		{
			int DequeueSize;
			int BuffSize = 2;
			int BuffArray = 0;

			// �ݺ��ϸ� �� 2����Ʈ ����� �ӽù��۷� ������. (�ݺ��ϴ� ������, ���۰� �߷��� �� ���� 2����Ʈ�� ���о�� ���� �ֱ� ������.
			while (BuffSize > 0)
			{				
				DequeueSize = User->m_RecvBuff->Dequeue((char*)&header + BuffArray, BuffSize);
				if (DequeueSize == -1)
				{
					_tprintf(_T("ProcPacket(). ���ú� ť�� �������(���)\n"));
					return FALSE;
				}

				BuffSize -= DequeueSize;
				BuffArray += DequeueSize;
			}			

			
			st_Draw_Packet DrawInfo;
			BuffSize = header;
			BuffArray = 0;

			// ���������� ����� ����ִ� ��(���̷ε��� ����) ��ŭ ���۷� ������.
			while (BuffSize > 0)
			{
				DequeueSize = User->m_RecvBuff->Dequeue((char*)&DrawInfo + BuffArray, BuffSize);
				if (DequeueSize == -1)
				{
					_tprintf(_T("ProcPacket(). ���ú� ť�� �������(���̷ε�)\n"));
					return FALSE;
				}

				BuffSize -= DequeueSize;
				BuffArray += DequeueSize;
			}
			

			// ���, ���̷ε��� �����͸� ������Ŷ�� �ִ´�.
			int check = SendPacket((char*)&header, 2, User);
			if (check == -1)
				return FALSE;

			check = SendPacket((char*)&DrawInfo, header, User);
			if (check == -1)
				return FALSE;

			// ���� ��Ŷó���� ��뿡�� �����ϴ°Źۿ� ����.
			//ProcWrite(User->m_Sock);
			ProcWriteBroadCast(User->m_Sock);
		}

		// ��Ŷ 1���� �ϼ����� ������ ��������.
		else
			break;

	}	

	return TRUE;
}

