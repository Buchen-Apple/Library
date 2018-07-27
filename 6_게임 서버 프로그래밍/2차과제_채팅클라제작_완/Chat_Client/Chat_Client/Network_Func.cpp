#include "stdafx.h"
#include "Chat_Client.h"

#pragma comment(lib, "ws2_32")
#include <WS2tcpip.h>

#include "Network_Func.h"
#include "RingBuff\RingBuff.h"
#pragma warning(disable:4996)

#include <list>
#include <map>

using namespace std;

// ���� ����ü
struct stUser
{
	DWORD m_UserID;
	TCHAR m_NickName[dfNICK_MAX_LEN] = _T("0");	// ���� �г���
};

// �� ����ü
struct stRoom
{
	DWORD m_RoomID;							// �� ���� ID
	TCHAR* m_RoomName;						// �� �̸�

	map <DWORD, stUser*> m_JoinUserList;	// �ش� �뿡 �������� ������. ���� ID�� key	
};


// ������ ���� ����������
HINSTANCE Instance;								// �� �ν��Ͻ�
HWND LobbyhWnd;									// �κ� ���̾�α� �ڵ�
HWND* RoomhWnd;									// �� ���̾�α� �ڵ�
int nCmdShow;									// cmdShow ����

// ��� ���� ����������
SOCKET client_sock;								// �� ����
BOOL bConnectFlag;								// ���� ���θ� üũ�ϴ� ����.
BOOL bSendFlag;									// Send ���� ����/ �Ұ��ɻ��¸� üũ�ϴ� ����
CRingBuff SendBuff(1000);						// �� ���� ����
CRingBuff RecvBuff(1000);						// �� ���ú� ����

// �� ���� ���� ����������
TCHAR g_NickName[dfNICK_MAX_LEN];				// �� �г���
DWORD g_MyUserID;								// �� ���� ID
DWORD g_JoinRoomNumber;							// ���� �������� �� ID.
stRoom* MyRoom;									// ���� �������� ��

// �� �� ��Ÿ ��������
map <DWORD, stRoom*> map_RoomList;				// �� ���(map)
TCHAR MyHopeRoomName[ROOMNAME_SIZE];			// ���� ��û�� �� �̸��� �����ϴ� ����. [ä�ù� ���� ����!] �޽����ڽ��� ������ϴ��� üũ�ϴ� �뵵

// RoomDialogProc() �Լ� extern ����
extern INT_PTR CALLBACK RoomDialolgProc(HWND, UINT, WPARAM, LPARAM);


/////////////////////////
// ��Ÿ �Լ�
/////////////////////////

// ���� ���� ���޹ޱ�. (�κ� ���̾�α� �ڵ�(����), �ν��Ͻ� ��..)
void InfoGet(HWND g_LobbyhWnd, HWND* g_RoomhWnd, HINSTANCE hInst, int cmdShow)
{
	LobbyhWnd = g_LobbyhWnd;
	RoomhWnd = g_RoomhWnd;
	Instance = hInst;
	nCmdShow = cmdShow;

	// ����, ���� ������ ���� NULL�̴�
	MyRoom = nullptr;
}

// ���� �ʱ�ȭ, ���� ����, Ŀ��Ʈ �� �Լ�
BOOL NetworkInit(TCHAR tIP[30], TCHAR tNickName[dfNICK_MAX_LEN])
{
	_tcscpy_s(g_NickName, dfNICK_MAX_LEN, tNickName);

	// ���� �ʱ�ȭ
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
	{
		MessageBox(LobbyhWnd, _T("���� �ʱ�ȭ ����"), _T("���ӽ���"), MB_ICONERROR);
		return FALSE;
	}

	// ���� ����
	client_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (client_sock == SOCKET_ERROR)
	{
		MessageBox(LobbyhWnd, _T("���� ���� ����"), _T("���Ͻ���"), MB_ICONERROR);
		return FALSE;
	}

	// WSAAsyncSelect(). ���ÿ� �񵿱� ������ �ȴ�.
	int retval = WSAAsyncSelect(client_sock, LobbyhWnd, WM_SOCK, FD_WRITE | FD_READ | FD_CONNECT | FD_CLOSE);
	if (retval == SOCKET_ERROR)
	{
		MessageBox(LobbyhWnd, _T("���ũ������ ����"), _T("���ũ����Ʈ ����"), MB_ICONERROR);
		return FALSE;
	}

	// Connect
	ULONG uIP;
	SOCKADDR_IN clientaddr;
	ZeroMemory(&clientaddr, sizeof(clientaddr));

	InetPton(AF_INET, tIP, &uIP);
	clientaddr.sin_family = AF_INET;
	clientaddr.sin_port = htons(dfNETWORK_PORT);
	clientaddr.sin_addr.s_addr = uIP;

	retval = connect(client_sock, (SOCKADDR*)&clientaddr, sizeof(clientaddr));
	if (retval == SOCKET_ERROR)
	{
		if (WSAGetLastError() != WSAEWOULDBLOCK)
		{
			MessageBox(LobbyhWnd, _T("Ŀ��Ʈ ����"), _T("Ŀ��Ʈ ����"), MB_ICONERROR);
			return FALSE;
		}
	}

	return TRUE;
}

// ���� ����, ���� ���� �� �Լ�
void NetworkClose()
{
	closesocket(client_sock);
	WSACleanup();
}

// ��Ʈ��ũ ó�� �Լ�
BOOL NetworkProc(LPARAM lParam)
{
	// ���� ���� ó��
	// lParam�� ���� 16��Ʈ�� �����ڵ尡, ���� 16��Ʈ�� �߻��� �̺�Ʈ�� ����ִ�.
	if (WSAGETSELECTERROR(lParam) != 0)
	{
		// ������ �����ִ� ����
		if (WSAGETSELECTERROR(lParam) == 10061)
			MessageBox(LobbyhWnd, _T("NetworkProc(). ������ �����ֽ��ϴ�."), _T("���� ����"), 0);

		else if (WSAGETSELECTERROR(lParam) == 10053)
			MessageBox(LobbyhWnd, _T("NetworkProc(). ��밡 ���ڱ� ������ ���� ��."), _T("���� ����"), 0);

		// �� �� ����
		else
			MessageBox(LobbyhWnd, _T("NetworkProc(). ��Ʈ��ũ ���� �߻�"), _T("���� ����"), 0);

		return FALSE;
	}

	// ���� ��Ʈ��ũ ó��
	switch (WSAGETSELECTEVENT(lParam))
	{
	case FD_CONNECT:
		{
			bConnectFlag = TRUE;

			// ���ῡ �����ϸ�, "�α��� ��û" ��Ŷ�� ����ť�� �־�д�.
			// �׸���, FD_CONNECT ������ �߻��ϴ� FD_WRITE ��, �α��� ��û�� ������.
			// ������ �Ǹ�, [�α��� ��û -> �α��� ��û ��� Recv -> ��ȭ�� ��� ��û -> ��ȭ�� ��� ��û Recv -> ��]�� ������ �����͸� �ְ�޴� ����.
			CProtocolBuff header(dfHEADER_SIZE);
			CProtocolBuff payload;

			// "�α��� ��û" ��Ŷ ����
			CreatePacket_Req_Login((char*)&header, (char*)&payload);

			// Send���ۿ� �ֱ�
			// �ִ� �����ϸ� ���� ����.
			if (SendPacket(&header, &payload) == FALSE)
				return FALSE;
		}
		return TRUE;

	case FD_WRITE:
		bSendFlag = TRUE;

		// SendProc()���� ������ �߻��ϸ� FALSE�� ���ϵȴ�.
		if (SendProc() == FALSE)
			return FALSE;
		return TRUE;

	case FD_READ:
		// RecvProc()���� ������ �߻��ϸ� FALSE�� ���ϵȴ�.
		if (RecvProc() == FALSE)
			return FALSE;

		return TRUE;

	case FD_CLOSE:
		MessageBox(LobbyhWnd, _T("NetworkProc(). FD_CLOSE �߻�."), _T("���� ����"), 0);
		return FALSE;

	default:
		MessageBox(LobbyhWnd, _T("NetworkProc(). �� �� ���� ��Ŷ"), _T("���� ����"), 0);
		return TRUE;
	}
}

// ��� �����ϱ�
void CreateHeader(CProtocolBuff* header, CProtocolBuff* payload, WORD Type)
{
	// ��� �ڵ�, Ÿ��, ���̷ε� ������ ����
	BYTE byCode = dfPACKET_CODE;
	WORD wMsgType = Type;
	WORD wPayloadSize = payload->GetUseSize();

	// ����� üũ�� ����
	CProtocolBuff ChecksumBuff;
	DWORD UseSize = payload->GetUseSize();

	memcpy(ChecksumBuff.GetBufferPtr(), payload->GetBufferPtr(), UseSize);
	ChecksumBuff.MoveWritePos(UseSize);

	for (int i = 0; i< dfNICK_MAX_LEN; ++i)
		ChecksumBuff << g_NickName[i];

	DWORD Checksum = 0;
	for (int i = 0; i < wPayloadSize; ++i)
	{
		BYTE bPayloadByte;
		ChecksumBuff >> bPayloadByte;
		Checksum += bPayloadByte;
	}

	BYTE* MsgType = (BYTE*)&wMsgType;
	for (int i = 0; i < sizeof(wMsgType); ++i)
		Checksum += MsgType[i];

	BYTE SaveChecksum = Checksum % 256;

	// ��� �ϼ��ϱ�
	*header << byCode;
	*header << SaveChecksum;
	*header << wMsgType;
	*header << wPayloadSize;
}

// �� ä���� ���� �� ������. �׸��� �� �����Ϳ� ����ϱ���� �ϴ� �Լ�
void ChatLogic()
{
	// 1. ���� �Է��� ���ڸ� �����Ϳ��� �����´�. (�� ���̾�α��� ä�� �������̴�)
	TCHAR Typing[CHAT_SIZE];
	GetDlgItemText(*RoomhWnd, IDC_ROOM_TYPING, Typing, CHAT_SIZE);
	DWORD size = (DWORD)_tcslen(Typing);

	// 2. "ä�� �۽�" ��Ŷ ����
	CProtocolBuff header(dfHEADER_SIZE);
	CProtocolBuff payload;

	CreatePacket_Req_ChatSend((char*)&header, (char*)&payload, size, Typing);

	// 3. ���� ��Ŷ�� SendBuff�� �ִ´�.
	SendPacket(&header, &payload);

	// 4. SendBuff�� �����͸� Send�Ѵ�.
	SendProc();

	// 5. ������ ����Ʈ�� �߰��Ѵ� (�� ä���� ���� �������� �׳� ����Ѵ�)
	TCHAR ShowString[4 + CHAT_SIZE];	// "�� : " (�� 4�� ����)�� �߰��Ǿ 4�� ��.
	swprintf_s(ShowString, _countof(ShowString), _T("%s : %s"), _T("��"), Typing);
	HWND RoomChathWnd = GetDlgItem(*RoomhWnd, IDC_ROOM_CHAT);
	SendMessage(RoomChathWnd, LB_ADDSTRING, 0, (LPARAM)ShowString);

	// 6. �ڵ� ��ũ�� ���.
	// ���� ����Ʈ �ڽ��� �߰��� Index�� ���� ������
	// �ε��� -1�� Top �ε����� �����Ѵ�. �׷��� �ڵ� ��ũ�� �Ǵ°� ó�� ���δ�.
	LRESULT iIndex = SendMessage(RoomChathWnd, LB_GETCOUNT, 0, 0);
	SendMessage(RoomChathWnd, LB_SETTOPINDEX, iIndex - 1, 0);

	// 7. �������� ������ �������� �����. ä�� ���������ϱ�!
	SetDlgItemText(*RoomhWnd, IDC_ROOM_TYPING, _T(""));
}





/////////////////////////
// ��Ŷ ���� �Լ�
/////////////////////////

// "�α��� ��û" ��Ŷ ����
void CreatePacket_Req_Login(char* header, char* Packet)
{
	// 1. ���̷ε� ����
	CProtocolBuff* PayloadBuff = (CProtocolBuff*)Packet;
	for(int i=0; i< dfNICK_MAX_LEN; ++i)
		*PayloadBuff << g_NickName[i];

	// 2. ��� �����ϱ�
	CreateHeader((CProtocolBuff*)header, PayloadBuff, df_REQ_LOGIN);
}

// "��ȭ�� ��� ��û" ��Ŷ ����
void CreatePacket_Req_RoomList(char* header)
{
	// 1. "��ȭ�� ��� ��û"�� ���̷ε尡 ����
	CProtocolBuff payload;

	// 2. ��� �����ϱ�
	CreateHeader((CProtocolBuff*)header, &payload, df_REQ_ROOM_LIST);
	
}

// "�� ���� ��û" ��Ŷ ����
void CreatePacket_Req_RoomCreate(char* header, char* Packet, TCHAR RoomName[ROOMNAME_SIZE])
{
	CProtocolBuff* PayloadBuff = (CProtocolBuff*)Packet;

	// 1. ���̷ε� ����
	// �ϴ� �� ���� ������ �˾ƿ���
	WORD RoomNameSize = (WORD)(_tcslen(RoomName) * 2);
	*PayloadBuff << RoomNameSize;
	
	// �� �̸�, ���̷ε忡 �ֱ�
	for (int i = 0; i < RoomNameSize / 2; ++i)
		*PayloadBuff << RoomName[i];

	_tcscpy_s(MyHopeRoomName, RoomNameSize, RoomName);

	// 2. ��� �����ϱ�
	CreateHeader((CProtocolBuff*)header, PayloadBuff, df_REQ_ROOM_CREATE);
}

// "�� ���� ��û" ��Ŷ ����
void CreatePacket_Req_RoomJoin(char* header, char* Packet, DWORD RoomNo)
{
	CProtocolBuff* PayloadBuff = (CProtocolBuff*)Packet;

	// 1. ���̷ε� ����	
	// �˾ƿ� �� �ѹ��� Payload ä���
	*PayloadBuff << RoomNo;

	// 2. ��� �����
	CreateHeader((CProtocolBuff*)header, PayloadBuff, df_REQ_ROOM_ENTER);
}

// "ä�� �۽�" ��Ŷ ����
void CreatePacket_Req_ChatSend(char* header, char* Packet, DWORD MessageSize, TCHAR Chat[CHAT_SIZE])
{
	CProtocolBuff* PayloadBuff = (CProtocolBuff*)Packet;

	// 1. ���̷ε� ����
	// ä�� ������ ���� (���ڿ��� �����ڵ� �������̴�. ����Ʈ Size�� ���� �� ����.)
	WORD Size = (WORD)(MessageSize * 2);
	*PayloadBuff << Size;

	// ä�� ���� ����
	for(DWORD i=0; i<MessageSize; ++i)
		*PayloadBuff << Chat[i];

	// 2. ��� �����
	CreateHeader((CProtocolBuff*)header, PayloadBuff, df_REQ_CHAT);

}

// "�� ���� ��û" ��Ŷ ����
void CreatePacket_Req_RoomLeave(char* header)
{
	// 1. ���̷ε� ����
	// �� ���� ��û�� ���̷ε尡 ����.
	CProtocolBuff PayloadBuff;

	// 2. ��� �����
	CreateHeader((CProtocolBuff*)header, &PayloadBuff, df_REQ_ROOM_LEAVE);
}



/////////////////////////
// Send ó��
/////////////////////////

// Send���ۿ� ������ �ֱ�
BOOL SendPacket(CProtocolBuff* headerBuff, CProtocolBuff* payloadBuff)
{
	// ����� �������� Ȯ��
	if (bConnectFlag == FALSE)
		return TRUE;

	// 1. ���� q�� ��� �ֱ�
	int Size = headerBuff->GetUseSize();
	DWORD BuffArray = 0;
	int a = 0;
	while (Size > 0)
	{
		int EnqueueCheck = SendBuff.Enqueue(headerBuff->GetBufferPtr() + BuffArray, Size);
		if (EnqueueCheck == -1)
		{
			MessageBox(LobbyhWnd, _T("SendPacket(). ��� �ִ� �� ����ť ����. ���� ����"), _T("���� ����"), MB_ICONERROR);
			return FALSE;
		}

		Size -= EnqueueCheck;
		BuffArray += EnqueueCheck;
	}

	// 2. ���� q�� ���̷ε� �ֱ�
	WORD PayloadLen = payloadBuff->GetUseSize();
	BuffArray = 0;
	while (PayloadLen > 0)
	{
		int EnqueueCheck = SendBuff.Enqueue(payloadBuff->GetBufferPtr() + BuffArray, PayloadLen);
		if (EnqueueCheck == -1)
		{
			MessageBox(LobbyhWnd, _T("SendPacket(). ���̷ε� �ִ� �� ����ť ����. ���� ����"), _T("���� ����"), MB_ICONERROR);
			return FALSE;
		}

		PayloadLen -= EnqueueCheck;
		BuffArray += EnqueueCheck;
	}

	return TRUE;
}

// Send������ �����͸� Send�ϱ�
BOOL SendProc()
{
	// 1.Send���� ���� Ȯ��.
	if (bSendFlag == FALSE)
		return TRUE;

	// 2. SendBuff�� ���� �����Ͱ� �ִ��� Ȯ��.
	if (SendBuff.GetUseSize() == 0)
		return TRUE;

	// 3. ���� �������� �������� ���� ������
	char* sendbuff = SendBuff.GetBufferPtr();

	// 4. SendBuff�� �� �������� or Send����(����)���� ��� send
	while (1)
	{
		// 5. SendBuff�� �����Ͱ� �ִ��� Ȯ��(���� üũ��)
		if (SendBuff.GetUseSize() == 0)
			break;

		// 6. �� ���� ���� �� �ִ� �������� ���� ���� ���
		int Size = SendBuff.GetNotBrokenGetSize();

		// 7. ���� ������ 0�̶�� 1�� ����. send() �� 1����Ʈ�� �õ��ϵ��� �ϱ� ���ؼ�.
		// �׷��� send()�� ������ �ߴ��� Ȯ�� ����
		if (Size == 0)
			Size = 1;

		// 8. front ������ ����
		int *front = (int*)SendBuff.GetReadBufferPtr();

		// 9. front�� 1ĭ �� index�� �˾ƿ���.
		int BuffArrayCheck = SendBuff.NextIndex(*front, 1);

		// 10. Send()
		int SendSize = send(client_sock, &sendbuff[BuffArrayCheck], Size, 0);

		// 11. ���� üũ. �����̸� �� �̻� �������� while�� ����. ���� Select�� �ٽ� ����
		if (SendSize == SOCKET_ERROR)
		{
			if (WSAGetLastError() == WSAEWOULDBLOCK)
				break;

			else
			{
				MessageBox(LobbyhWnd, _T("SendProc(). Send�� ���� �߻�"), _T("���� ����"), MB_ICONERROR);
				return FALSE;
			}
		}

		// 12. ���� ����� ��������, �� ��ŭ remove
		SendBuff.RemoveData(SendSize);
	}

	return TRUE;
}




/////////////////////////
// Recv ó�� �Լ�
/////////////////////////

// ��Ʈ��ũ ó�� ��, Recv ó�� �Լ�
BOOL RecvProc()
{
	// bConnectFlag(Ŀ��Ʈ ����)�� FALSE��� ���� ���� �ȵȰŴ� �׳� ����.
	if (bConnectFlag == FALSE)
		return TRUE;

	//////////////////////////////////////////////////
	// recv()
	//////////////////////////////////////////////////

	// 1. ���ú� �����۷� recv()
	char* recvbuff = RecvBuff.GetBufferPtr();

	// 2. �� ���� �� �� �ִ� �������� ���� ���� ���
	int Size = RecvBuff.GetNotBrokenPutSize();

	// 3. �� ���� �� �� �ִ� ������ 0�̶��
	if (Size == 0)
	{
		// rear 1ĭ �̵�
		RecvBuff.MoveWritePos(1);

		// �׸��� �� ���� �� �� �ִ� ��ġ �ٽ� �˱�.
		Size = RecvBuff.GetNotBrokenPutSize();
	}

	// 4. �װ� �ƴ϶��, 1ĭ �̵��� �ϱ�. 		
	else
	{
		// rear 1ĭ �̵�
		RecvBuff.MoveWritePos(1);
	}

	// 5. 1ĭ �̵��� rear ������
	int* rear = (int*)RecvBuff.GetWriteBufferPtr();


	// 6. recv()
	int retval = recv(client_sock, &recvbuff[*rear], Size, 0);

	// 7. ���ú� ����üũ
	if (retval == SOCKET_ERROR)
	{
		// WSAEWOULDBLOCK������ �߻��ϸ�, ���� ���۰� ����ִٴ� ��. recv�Ұ� ���ٴ� ���̴� while�� ����
		if (WSAGetLastError() == WSAEWOULDBLOCK)
			return TRUE;

		// WSAEWOULDBLOCK������ �ƴϸ� ���� �̻��� ���̴� ���� ����
		else
		{
			TCHAR Test[80] = _T("RecvProc(). retval �� ������ �ƴ� ������ ��. ���� ����");
			TCHAR ErrorText[80];
			swprintf_s(ErrorText, _countof(ErrorText), _T("%s %d"), Test, WSAGetLastError());

			MessageBox(LobbyhWnd, ErrorText, _T("���� ����"), 0);
			return FALSE;		// FALSE�� ���ϵǸ�, ������ �����.
		}
	}

	// 8. ������� ������ Rear�� �̵���Ų��.
	RecvBuff.MoveWritePos(retval - 1);

	//////////////////////////////////////////////////
	// �Ϸ� ��Ŷ ó�� �κ�
	// RecvBuff�� ����ִ� ��� �ϼ� ��Ŷ�� ó���Ѵ�.
	//////////////////////////////////////////////////
	while (1)
	{
		// 1. RecvBuff�� �ּ����� ����� �ִ��� üũ. (���� = ��� ������� ���ų� �ʰ�. ��, �ϴ� �����ŭ�� ũ�Ⱑ �ִ��� üũ)		
		st_PACKET_HEADER HeaderBuff;
		int len = sizeof(HeaderBuff);

		// RecvBuff�� ��� ���� ���� ũ�Ⱑ ��� ũ�⺸�� �۴ٸ�, �Ϸ� ��Ŷ�� ���ٴ� ���̴� while�� ����.
		if (RecvBuff.GetUseSize() < len)
			break;

		// 2. ����� Peek���� Ȯ���Ѵ�.		
		// Peek �ȿ�����, ��� �ؼ����� len��ŭ �д´�. ���۰� �� ���� ���� �̻�!
		int PeekSize = RecvBuff.Peek((char*)&HeaderBuff, len);

		// Peek()�� ���۰� ������� -1�� ��ȯ�Ѵ�. ���۰� ������� �ϴ� ���´�.
		if (PeekSize == -1)
		{
			MessageBox(LobbyhWnd, _T("RecvProc(). �Ϸ���Ŷ ��� ó�� �� RecvBuff�����. ���� ����"), _T("���� ����"), 0);
			return FALSE;	// FALSE�� ���ϵǸ�, �ش� ������ ������ �����.
		}

		// 3. ����� code Ȯ��(���������� �� �����Ͱ� �´°�)
		if (HeaderBuff.byCode != dfPACKET_CODE)
		{
			MessageBox(LobbyhWnd, _T("RecvProc(). �Ϸ���Ŷ ��� ó�� �� ������Ŷ �ƴ�. ���� ����"), _T("���� ����"), 0);
			return FALSE;	// FALSE�� ���ϵǸ�, �ش� ������ ������ �����.
		}

		// 4. ����� Len���� RecvBuff�� ������ ������ ��. (�ϼ� ��Ŷ ������ = ��� ������ + ���̷ε� Size )
		// ��� ���, �ϼ� ��Ŷ ����� �ȵǸ� while�� ����.
		if (RecvBuff.GetUseSize() < (len + HeaderBuff.wPayloadSize))
			break;

		// 5. RecvBuff���� Peek�ߴ� ����� ����� (�̹� Peek������, �׳� Remove�Ѵ�)
		RecvBuff.RemoveData(len);

		// 6. RecvBuff���� Len��ŭ ���̷ε� �ӽ� ���۷� �̴´�. (��ť�̴�. Peek �ƴ�)
		DWORD BuffArray = 0;
		CProtocolBuff PayloadBuff;
		DWORD PayloadSize = HeaderBuff.wPayloadSize;

		while (PayloadSize > 0)
		{
			int DequeueSize = RecvBuff.Dequeue(PayloadBuff.GetBufferPtr() + BuffArray, PayloadSize);

			// Dequeue()�� ���۰� ������� -1�� ��ȯ. ���۰� ������� �ϴ� ���´�.
			if (DequeueSize == -1)
			{
				MessageBox(LobbyhWnd, _T("RecvProc(). �Ϸ���Ŷ ���̷ε� ó�� �� RecvBuff�����. ���� ����"), _T("���� ����"), 0);
				return FALSE;	// FALSE�� ���ϵǸ�, �ش� ������ ������ �����.
			}

			PayloadSize -= DequeueSize;
			BuffArray += DequeueSize;
		}
		PayloadBuff.MoveWritePos(BuffArray);

		// 7. ����� üũ�� Ȯ�� (2�� ����)
		CProtocolBuff ChecksumBuff;
		memcpy(ChecksumBuff.GetBufferPtr(), PayloadBuff.GetBufferPtr(), PayloadBuff.GetUseSize());
		ChecksumBuff.MoveWritePos(PayloadBuff.GetUseSize());

		DWORD Checksum = 0;
		for (int i = 0; i < HeaderBuff.wPayloadSize; ++i)
		{
			BYTE ChecksumAdd;
			ChecksumBuff >> ChecksumAdd;
			Checksum += ChecksumAdd;
		}

		BYTE* MsgType = (BYTE*)&HeaderBuff.wMsgType;
		for (int i = 0; i < sizeof(HeaderBuff.wMsgType); ++i)
			Checksum += MsgType[i];

		Checksum = Checksum % 256;

		if (Checksum != HeaderBuff.byCheckSum)
		{
			_tprintf(_T("RecvProc(). �Ϸ���Ŷ üũ�� ����. ���� ����\n"));
			return false;	// FALSE�� ���ϵǸ�, ������ �����.
		}

		// 8. ����� Ÿ�Կ� ���� �б�ó��. �Լ��� ó���Ѵ�.
		BOOL check = PacketProc(HeaderBuff.wMsgType, (char*)&PayloadBuff);
		if (check == FALSE)
			return FALSE;
	}

	return TRUE;
}

// ��Ŷ Ÿ�Կ� ���� ��Ŷó�� �Լ�
BOOL PacketProc(WORD PacketType, char* Packet)
{
	BOOL check;
	try
	{
		// Recv������ ó��
		switch (PacketType)
		{
			// ä�� �۽� ���(���� ���� �濡 �ִ�, �ٸ� ����� ä�ø� ��)
			// ���� �������� ����� �ȳ����.
		case df_RES_CHAT:
			check = Network_Res_ChatRecv(Packet);
			if (check == FALSE)
				return FALSE;
			break;

			// ���� �ִ� �濡 �ٸ� ����� ����
		case df_RES_USER_ENTER:
			check = Network_Res_UserEnter(Packet);
			if (check == FALSE)
				return FALSE;
			break;

			// �� ���� ��� (���� ������ ���� �͵� �´�.)
			// ��, ���� ���� �濡 �ִ� �������� �����嵵 ����� �� �ִ�.
		case df_RES_ROOM_LEAVE:
			check = Network_Res_RoomLeave(Packet);
			if (check == FALSE)
				return FALSE;
			break;

			// ��ȭ�� ���� ��� (���� ���� + �ٸ������ ������ �͵� ��)
		case df_RES_ROOM_CREATE:
			check = Network_Res_RoomCreate(Packet);
			if (check == FALSE)
				return FALSE;
			break;

			// ��ȭ�� ���� ��� (���� ����)
		case df_RES_ROOM_ENTER:
			check = Network_Res_RoomJoin(Packet);
			if (check == FALSE)
				return FALSE;
			break;

			// �� ���� ���
			// ������ ���� ����´�.
		case df_RES_ROOM_DELETE:
			check = Network_Res_RoomDelete(Packet);
			if (check == FALSE)
				return FALSE;
			break;

			// �α��� ��û ��� (���� ����)
		case df_RES_LOGIN:
			check = Network_Res_Login(Packet);
			if (check == FALSE)
				return FALSE;
			break;

			// ��ȭ�� ����Ʈ ��� (���� ����)
		case df_RES_ROOM_LIST:
			check = Network_Res_RoomList(Packet);
			if (check == FALSE)
				return FALSE;
			break;			

		default:
			MessageBox(LobbyhWnd, _T("PacketProc(). �� �� ���� ��Ŷ"), _T("���� ����"), 0);
			return FALSE;
		}

	}
	catch (CException exc)
	{
		TCHAR* text = (TCHAR*)exc.GetExceptionText();
		MessageBox(LobbyhWnd, text, _T("PacketProc(). ���� �߻�"), 0);
		return FALSE;
	}

	return TRUE;
}

// "�α��� ��û ���" ��Ŷ ó��
BOOL Network_Res_Login(char* Packet)
{
	CProtocolBuff* LoginPacket = (CProtocolBuff*)Packet;

	// 1. ���(1BYTE)�̾ƿ���.
	BYTE result;
	*LoginPacket >> result;

	// 2. ����� df_RESULT_LOGIN_OK���� üũ. 
	if (result != df_RESULT_LOGIN_OK)
	{
		// df_RESULT_LOGIN_OK�� �ƴ϶��, �߸��� ��.
		// ���� �߻���Ű�� ����
		if(result == df_RESULT_LOGIN_DNICK)
			MessageBox(LobbyhWnd, _T("Network_Res_Login(). �ߺ� �г���"), _T("���� ����"), 0);
		else
			MessageBox(LobbyhWnd, _T("Network_Res_Login(). �α��� ��û ����"), _T("���� ����"), 0);

		return FALSE;
	}

	// 3. ����� df_RESULT_LOGIN_OK���, ����� No�� �̾ƿ´�.
	// �׸��� �� UserID�� �����Ѵ�.
	*LoginPacket >> g_MyUserID;

	// 4. "��ȭ�� ��� ��û" ��Ŷ ����
	CProtocolBuff header(dfHEADER_SIZE);
	CProtocolBuff payload;
	CreatePacket_Req_RoomList((char*)&header);

	// 5. SendBuff�� �ֱ�.
	if (SendPacket(&header, &payload) == FALSE)
		return FALSE;

	// 6. Send�ϱ�
	if (SendProc() == FALSE)
	{
		MessageBox(LobbyhWnd, _T("Network_Res_Login(). ��ȭ�� ��� ��û Send����"), _T("���� ����"), 0);
		return FALSE;
	}

	return TRUE;

}

// "��ȭ�� ��� ��û ���" ��Ŷ ó��
BOOL Network_Res_RoomList(char* Packet)
{	
	CProtocolBuff* RoomListPacket = (CProtocolBuff*)Packet;

	// 1. �� �г��Ӱ� ����ID�� ����ƽ�� ǥ���Ѵ�.
	TCHAR tUserID[10];
	_itot_s(g_MyUserID, tUserID, _countof(tUserID), 10);

	SetDlgItemText(LobbyhWnd, IDC_NICKNAME, g_NickName);
	SetDlgItemText(LobbyhWnd, IDC_USERNUM, tUserID);

	// 2. �� ����Ʈ�� �� ǥ���ϱ� + �� ��Ͽ� �� �����ϱ�.
	// �� ���� �̾ƿ���
	WORD RoomCount;
	*RoomListPacket >> RoomCount;

	// �� ������ŭ �ݺ��ϸ鼭 ���� �̾ƿ�, �� ��Ͽ� ����.
	HWND RoomListBox = GetDlgItem(LobbyhWnd, IDC_LIST1);
	for (int i = 0; i < RoomCount; ++i)
	{
		// �̹� ���� ������ ������ ����ü ����
		stRoom* NowRoom = new stRoom;

		//	4Byte : �� No
		DWORD RoomNo;
		*RoomListPacket >> RoomNo;

		//	2Byte : ���̸� byte size
		WORD RoomNameSize;
		*RoomListPacket >> RoomNameSize;

		//	Size  : ���̸� (�����ڵ�)
		TCHAR* RoomName = new TCHAR[(RoomNameSize/2)+1];
		int j;
		for (j = 0; j < RoomNameSize/2; ++j)
			*RoomListPacket >> RoomName[j];

		RoomName[j] = '\0';

		// ������� ���� ����
		NowRoom->m_RoomID = RoomNo;
		NowRoom->m_RoomName = RoomName;

		//	1Byte : �����ο�
		//	{
		//		WHCAR[15] : �г���
		//	}
		// �̰� ���⸸�ϰ� ������� �ʴ´�. 
		BYTE JoinCount;
		*RoomListPacket >> JoinCount;

		for (int h = 0; h < JoinCount; ++h)
		{
			TCHAR JoinUserNick[dfNICK_MAX_LEN];
			for (int a = 0; a < dfNICK_MAX_LEN; ++a)
				*RoomListPacket >> JoinUserNick[a];
		}

		// ���õ� ����, �� ��Ͽ� ����
		typedef pair<DWORD, stRoom*> Itn_pair;
		map_RoomList.insert(Itn_pair(RoomNo, NowRoom));

		// ���� ����Ʈ �ڽ��� ǥ��
		DWORD ListIndex = (DWORD)SendMessage(RoomListBox, LB_ADDSTRING, 0, (LPARAM)RoomName);
		SendMessage(RoomListBox, LB_SETITEMDATA, WPARAM(ListIndex), (LPARAM)RoomNo);
	}	

	return TRUE;
}

// "�� ���� ��û ���" ��Ŷ ó��
BOOL Network_Res_RoomCreate(char* Packet)
{
	CProtocolBuff* RoomCreatePacket = (CProtocolBuff*)Packet;

	// 1. ����� �̾ƿ´�.
	BYTE result;
	*RoomCreatePacket >> result;

	// 2. ����� df_RESULT_LOGIN_OK���� üũ.
	if (result != df_RESULT_ROOM_CREATE_OK)
	{
		// df_RESULT_ROOM_CREATE_OK�� �ƴ϶��, �߸��� ��.
		// ���� �߻���Ű�� ����
		MessageBox(LobbyhWnd, _T("Network_Res_RoomCreate(). ����� ��û ����� OK�� �ƴ�"), _T("���� ����"), MB_ICONERROR);
		return FALSE;
	}

	// 3. OK��� �� No, ������ ����ƮSize, ������(�����ڵ�) �̾ƿͼ� ���ο� �� �ϼ�
	stRoom* NewRoom = new stRoom;

	// �� �ѹ�.
	DWORD RoomNo;
	*RoomCreatePacket >> RoomNo;

	// �� ���� ����ƮSize
	WORD RoomNameSize;
	*RoomCreatePacket >> RoomNameSize;

	// �� ����
	TCHAR* RoomName = new TCHAR[(RoomNameSize / 2) + 1];
	int i;
	for (i = 0; i < RoomNameSize / 2; ++i)
		*RoomCreatePacket >> RoomName[i];

	RoomName[i] = '\0';

	// �̾ƿ� ���� ����
	NewRoom->m_RoomID = RoomNo;
	NewRoom->m_RoomName = RoomName;	

	// 4. ���� ������ ��û�� ���̶��, ä�ù��� �����Ǿ��ٴ� �޽��� �ڽ� ǥ��
	if(_tcscmp(MyHopeRoomName, RoomName) == 0)
		MessageBox(LobbyhWnd, _T("ä�ù� ���� �Ϸ�"), _T("�� ����"), NULL);

	// ������ OK�� ������
	// 5. �߰��� ���� ����Ʈ �ڽ��� ǥ���Ѵ�.
	DWORD ListIndex = (DWORD)SendMessage(GetDlgItem(LobbyhWnd, IDC_LIST1), LB_ADDSTRING, 0, (LPARAM)RoomName);
	SendMessage(GetDlgItem(LobbyhWnd, IDC_LIST1), LB_SETITEMDATA, (WPARAM)ListIndex, (LPARAM)RoomNo);

	// 6. ���õ� ����, �� ��Ͽ� �߰�
	typedef pair<DWORD, stRoom*> Itn_pair;
	map_RoomList.insert(Itn_pair(RoomNo, NewRoom));

	// 7. �� �̸��� �Է��ϴ� ����Ʈ�� �������� �����Ѵ�.
	SetDlgItemText(LobbyhWnd, IDC_ROOMNAME, _T(""));

	return TRUE;
}

// "�� ���� ��û ���" ��Ŷ ó��
BOOL Network_Res_RoomJoin(char* Packet)
{
	CProtocolBuff* RoomJoinPacket = (CProtocolBuff*)Packet;

	// 1. ����� �̾ƿ´�.
	BYTE result;
	*RoomJoinPacket >> result;

	// 2. ����� df_RESULT_LOGIN_OK���� üũ.
	if (result != df_RESULT_ROOM_ENTER_OK)
	{
		// df_RESULT_ROOM_ENTER_OK�� �ƴ϶��, �߸��� ��.
		// ���� �߻���Ű�� ����
		MessageBox(LobbyhWnd, _T("Network_Res_RoomJoin(). �� ���� ��û ����� OK�� �ƴ�"), _T("���� ����"), MB_ICONERROR);
		return FALSE;
	}

	// 3. OK��� �� ������ ���´�.
	// �� No�� �� ��, �� No�� ���� �˾ƿ´�.
	DWORD RoomNo;
	*RoomJoinPacket >> RoomNo;	

	stRoom* NowRoom = new stRoom;
	NowRoom = nullptr;
	map <DWORD, stRoom*>::iterator itor;
	for (itor = map_RoomList.begin(); itor != map_RoomList.end(); ++itor)
	{
		if (itor->second->m_RoomID == RoomNo)
		{
			NowRoom = itor->second;
			break;
		}
	}

	if (NowRoom == nullptr)
	{
		MessageBox(LobbyhWnd, _T("Network_Res_RoomJoin(). ��������, ���� �濡 �����϶�� ��."), _T("���� ����"), MB_ICONERROR);
		return FALSE;
	}

	// �� ���� ����Ʈ Size
	WORD RoomNameSize;
	*RoomJoinPacket >> RoomNameSize;

	// �� ����
	TCHAR* RoomName = new TCHAR[(RoomNameSize / 2) + 1];
	int i;
	for (i = 0; i < RoomNameSize / 2; ++i)
		*RoomJoinPacket >> RoomName[i];

	RoomName[i] = '\0';

	// �����ο�
	BYTE JoinCount;
	*RoomJoinPacket >> JoinCount;
	for (int i = 0; i < JoinCount; ++i)
	{
		stUser* NowUser = new stUser;

		// �г��� ����
		TCHAR RoomJoinUSer[dfNICK_MAX_LEN];
		for (int j = 0; j < dfNICK_MAX_LEN; ++j)
			*RoomJoinPacket >> RoomJoinUSer[j];

		RoomJoinUSer[_tcslen(RoomJoinUSer)] = '\0';

		// ����� ID ����
		DWORD UserID;
		*RoomJoinPacket >> UserID;

		// ���� ����
		_tcscpy_s(NowUser->m_NickName, _countof(RoomJoinUSer), RoomJoinUSer);
		NowUser->m_UserID = UserID;

		// ������ ����, �濡 �߰��ϱ�
		typedef pair<DWORD, stUser*> Itn_pair;
		NowRoom->m_JoinUserList.insert(Itn_pair(UserID, NowUser));
	}

	// 4. ���� �濡 �߰��Ѵ�.
	stUser* MyInfo = new stUser;
	MyInfo->m_UserID = g_MyUserID;
	_tcscpy_s(MyInfo->m_NickName, _countof(g_NickName), g_NickName);

	typedef pair<DWORD, stUser*> Itn_pair;
	NowRoom->m_JoinUserList.insert(Itn_pair(g_MyUserID, MyInfo));
	
	// 5. �� ���̾�α� ����
	*RoomhWnd = CreateDialog(Instance, MAKEINTRESOURCE(IDD_ROOM), LobbyhWnd, RoomDialolgProc);

	// 6. �� ���̾�α� ���� ����
	// �� �̸� ǥ��
	SetDlgItemText(*RoomhWnd, IDC_ROOM_NAME, NowRoom->m_RoomName);

	// �� ���� ����Ʈ��, ���� �г��� ǥ��
	HWND RoomUserhWnd = GetDlgItem(*RoomhWnd, IDC_ROOM_USER);
	map <DWORD, stUser*>::iterator Useritor;
	for (Useritor = NowRoom->m_JoinUserList.begin(); Useritor != NowRoom->m_JoinUserList.end(); ++Useritor)
		SendMessage(RoomUserhWnd, LB_ADDSTRING, 0, (LPARAM)Useritor->second->m_NickName);

	// 7. ������ ǥ���ϱ�
	ShowWindow(*RoomhWnd, nCmdShow);	

	// 8. ���� ������ ���ȣ��, �� ������ ����Ѵ�.
	g_JoinRoomNumber = RoomNo;
	MyRoom = NowRoom;

	return TRUE;
}

// "ä�� ���" ��Ŷ ó�� (���� ���� �濡 �ִ� �ٸ������ ä�� ó���ϱ�)
BOOL Network_Res_ChatRecv(char* Packet)
{
	CProtocolBuff* ChatRecvPacket = (CProtocolBuff*)Packet;

	// 1. �۽��� No�˾ƿ���
	DWORD SenderNo;
	*ChatRecvPacket >> SenderNo;

	// 2. �޽��� Size �˾ƿ���
	WORD ChatSize;
	*ChatRecvPacket >> ChatSize;

	// 3. ä�� ���� �˾ƿ���
	TCHAR* Chat = new TCHAR[(ChatSize / 2) + 1];
	int i;
	for(i=0; i<ChatSize / 2; ++i)
		*ChatRecvPacket >> Chat[i];

	Chat[i] = '\0';

	// 4. ���� �ִ� �濡, �ش� ������ �˾ƿ´�.
	stUser* NowUser = nullptr;
	map <DWORD, stUser*>::iterator useritor;
	for (useritor = MyRoom->m_JoinUserList.begin(); useritor != MyRoom->m_JoinUserList.end(); ++useritor)
	{
		// ���� ��ȣ�� ��ġ�Ѵٸ�, �����ص״� NowUser��, �ش� �����͸� ���.
		if (useritor->first == SenderNo)
		{
			NowUser = useritor->second;
			break;
		}
	}	

	if (NowUser == nullptr)
	{
		MessageBox(LobbyhWnd, _T("Network_Res_ChatRecv(). ä�� ó�� ��, �濡 ���� ������ ä���� �����."), _T("���� ����"), MB_ICONERROR);
		return FALSE;
	}

	// 5. ������ ����Ʈ �ڽ��� �߰��Ѵ� (�� ä���� ���� �������� �׳� �߰��Ѵ�)
	TCHAR ShowString[dfNICK_MAX_LEN + CHAT_SIZE];
	swprintf_s(ShowString, _countof(ShowString), _T("%s : %s"), NowUser->m_NickName, Chat);
	HWND RoomChathWnd = GetDlgItem(*RoomhWnd, IDC_ROOM_CHAT);
	SendMessage(RoomChathWnd, LB_ADDSTRING, 0, (LPARAM)ShowString);

	// 6. �ڵ� ��ũ�� ���.
	// ���� ����Ʈ �ڽ��� �߰��� Index�� ���� ������
	// �ε��� -1�� Top �ε����� �����Ѵ�. �׷��� �ڵ� ��ũ�� �Ǵ°� ó�� ���δ�.
	LRESULT iIndex = SendMessage(RoomChathWnd, LB_GETCOUNT, 0, 0);
	SendMessage(RoomChathWnd, LB_SETTOPINDEX, iIndex - 1, 0);

	return TRUE;
}

// "Ÿ ����� ����" ��Ŷ ó��
BOOL Network_Res_UserEnter(char* Packet)
{
	CProtocolBuff* UserEnterPacket = (CProtocolBuff*)Packet;

	// 1. �г��� ������ (�����ڵ� 15����)
	TCHAR EnterUser[dfNICK_MAX_LEN];
	for (int i = 0; i < dfNICK_MAX_LEN; ++i)
		*UserEnterPacket >> EnterUser[i];

	// 2. ���� No ������
	DWORD JoinUserNo;
	*UserEnterPacket >> JoinUserNo;

	// 3. ���� �����ϱ�
	stUser* NewUser = new stUser;

	NewUser->m_UserID = JoinUserNo;
	_tcscpy_s(NewUser->m_NickName, 15, EnterUser);

	// 4. �� �濡 �ش� ���� �߰�
	typedef pair<DWORD, stUser*> Itn_pair;
	MyRoom->m_JoinUserList.insert(Itn_pair(JoinUserNo, NewUser));

	// 5. �ش� ������, �� ���� ���� ��Ͽ� ǥ��
	SendMessage(GetDlgItem(*RoomhWnd, IDC_ROOM_USER), LB_ADDSTRING, 0, (LPARAM)EnterUser);

	return TRUE;
}

// "�� ���� ��û ���" ��Ŷ ó�� (���� �Ⱥ����͵� ��)
BOOL Network_Res_RoomLeave(char* Packet)
{
	CProtocolBuff* RoomLeavePacket = (CProtocolBuff*)Packet;

	// 1. �濡�� ������ ���� No�� �˾ƿ´�.
	DWORD LeaveUserNo;
	*RoomLeavePacket >> LeaveUserNo;

	// 2. ������ ������ ������ Ȯ���Ѵ�.
	// ��, ���� ���� ������ ��û���� Ȯ��
	if (LeaveUserNo == g_MyUserID)
		EndDialog(*RoomhWnd, TRUE);

	// 3. ������ ������ ���� �ƴ϶��, �Ʒ� ���� ó��
	else
	{
		// �ϴ�, �ش� ������ ã�´�.
		BOOL Check = FALSE;
		map<DWORD, stUser*>::iterator itor;
		for (itor = MyRoom->m_JoinUserList.begin(); itor != MyRoom->m_JoinUserList.end(); ++itor)
		{
			// ������ ã������
			if (itor->first == LeaveUserNo)
			{	
				// ������ �� �� ���� ���(����Ʈ �ڽ�)���� �����Ѵ�.
				HWND RoomUserhWnd = GetDlgItem(*RoomhWnd, IDC_ROOM_USER);
				LRESULT index = SendMessage(RoomUserhWnd, LB_FINDSTRING, -1, (LPARAM)itor->second->m_NickName);
				SendMessage(RoomUserhWnd, LB_DELETESTRING, index, 0);

				// ������ �� �濡�� ���ܽ�Ų��.
				MyRoom->m_JoinUserList.erase(itor->first);

				// �׸��� �ش� ��� ���õ� ���� ������ ����
				delete itor->second;

				Check = TRUE;
				break;
			}
		}
		
		// ������ ��ã�Ҵٸ� ���� �߻�
		if (Check == FALSE)
		{
			MessageBox(LobbyhWnd, _T("Network_Res_RoomLeave(). �濡 ���� ������ �濡�� �����ٰ� �� ."), _T("���� ����"), MB_ICONERROR);
			return FALSE;
		}			
	}

	return TRUE;
}

// "�� ���� ���" ��Ŷ ó��
BOOL Network_Res_RoomDelete(char* Packet)
{
	CProtocolBuff* RoomDeletePacket = (CProtocolBuff*)Packet;

	// 1. ������ �� ID�� �˾ƿ´�.
	DWORD DeleteRoomID;
	*RoomDeletePacket >> DeleteRoomID;

	// 2. �ش� ID�� ����, �� ��Ͽ��� ã�´�.
	stRoom* DeleteRoom = nullptr;
	map <DWORD, stRoom*>::iterator itor;
	for (itor = map_RoomList.begin(); itor != map_RoomList.end(); ++itor)
	{
		if (itor->first == DeleteRoomID)
		{			
			DeleteRoom = itor->second;
			break;
		}
	}

	if (DeleteRoom == nullptr)
	{
		MessageBox(LobbyhWnd, _T("Network_Res_RoomDelete(). ���� ���� �����϶�� �� ."), _T("���� ����"), MB_ICONERROR);
		return FALSE;
	}

	// 3. ã�� ����, �� ����Ʈ �ڽ����� �����Ѵ�.
	// ������ �� �� ���� ���(����Ʈ �ڽ�)���� �����Ѵ�.
	HWND RoomListhWnd = GetDlgItem(LobbyhWnd, IDC_LIST1);
	LRESULT index = SendMessage(RoomListhWnd, LB_FINDSTRING, -1, (LPARAM)DeleteRoom->m_RoomName);
	SendMessage(RoomListhWnd, LB_DELETESTRING, index, 0);

	// 4. �� ��Ͽ��� �����Ѵ�. (������)
	map_RoomList.erase(DeleteRoomID);	

	// 5. �׸��� �ش� ��� ���õ� ���� ������ ����
	delete DeleteRoom->m_RoomName;
	delete DeleteRoom;

	return TRUE;
}