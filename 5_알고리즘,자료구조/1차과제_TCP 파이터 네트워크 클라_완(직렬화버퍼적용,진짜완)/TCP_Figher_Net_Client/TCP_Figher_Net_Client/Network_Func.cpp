#include "stdafx.h"
#include "RingBuff.h"
#include "ProtocolBuff.h"
#include "ExceptionClass.h"

// Main�� ���� ���̺귯��
#include "PlayerObject.h"
#include "Network_Func.h"
#include "EffectObject.h"

// ���� ������
SOCKET client_sock;								// �� ����
BOOL bSendFlag = FALSE;							// ���� ���� �÷���
BOOL bConnectFlag = FALSE;						// Ŀ��Ʈ ���� �÷���
CRingBuff SendBuff(100);						// ���� ����
CRingBuff RecvBuff(16);						// ���ú� ����
HWND WindowHwnd;								// ������ �ڵ�.
DWORD MyID;										// �� ID�Ҵ� �����صα�
extern CList<BaseObject*> list;					// Main���� �Ҵ��� ��ü ���� ����Ʈ	
extern BaseObject* g_pPlayerObject;				// Main���� �Ҵ� �� �÷��̾� ��ü

// �ؽ�Ʈ �ƿ����� ���� ��� �Լ�
void ErrorTextOut(const TCHAR* ErrorLog)
{
	TCHAR Log[100];
	_tcscpy(Log, ErrorLog);
	HDC hdc = GetDC(WindowHwnd);
	TextOut(hdc, 100, 0, ErrorLog, _tcslen(Log));
	ReleaseDC(WindowHwnd, hdc);
}

// ���� �ʱ�ȭ, ���� ����, Ŀ��Ʈ �� �Լ�
BOOL NetworkInit(HWND hWnd, TCHAR tIP[30])
{
	WindowHwnd = hWnd;	// ������ �ڵ� ������ �ֱ�.

	// ���� �ʱ�ȭ
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
	{
		MessageBox(WindowHwnd, _T("���� �ʱ�ȭ ����"), _T("���ӽ���"), MB_ICONERROR);
		return FALSE;
	}

	// ���� ����
	client_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (client_sock == SOCKET_ERROR)
	{
		MessageBox(WindowHwnd, _T("���� ���� ����"), _T("���Ͻ���"), MB_ICONERROR);
		return FALSE;
	}

	// WSAAsyncSelect(). ���ÿ� �񵿱� ������ �ȴ�.
	int retval = WSAAsyncSelect(client_sock, WindowHwnd, WM_SOCK, FD_WRITE | FD_READ | FD_CONNECT | FD_CLOSE);
	if (retval == SOCKET_ERROR)
	{
		MessageBox(WindowHwnd, _T("���ũ������ ����"), _T("���ũ����Ʈ ����"), MB_ICONERROR);
		return FALSE;
	}

	// Connect
	ULONG uIP;
	SOCKADDR_IN clientaddr;
	ZeroMemory(&clientaddr, sizeof(clientaddr));

	InetPton(AF_INET, tIP, &uIP);
	clientaddr.sin_family = AF_INET;
	clientaddr.sin_port = htons(SERVERPORT);
	clientaddr.sin_addr.s_addr = uIP;

	retval = connect(client_sock, (SOCKADDR*)&clientaddr, sizeof(clientaddr));
	if (retval == SOCKET_ERROR)
	{
		if (WSAGetLastError() != WSAEWOULDBLOCK)
		{
			MessageBox(hWnd, _T("Ŀ��Ʈ ����"), _T("Ŀ��Ʈ ����"), MB_ICONERROR);
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

// ��Ʈ��ũ ó�� �Լ�.
// �ش� �Լ����� FALSE�� ���ϵǸ�, �ش� ������ ������ �����.
BOOL NetworkProc(LPARAM lParam)
{
	// ���� ���� ó��
	if (WSAGETSELECTERROR(lParam) != 0)
	{
		ErrorTextOut(_T("NetworkProc(). ��Ʈ��ũ ���� �߻�"));
		return FALSE;
	}

	// ���� ��Ʈ��ũ ó��
	switch (WSAGETSELECTEVENT(lParam))
	{
	case FD_CONNECT:
		bConnectFlag = TRUE;
		return TRUE;

	case FD_WRITE:
		bSendFlag = TRUE;

		// SendProc()���� ������ �߻��ϸ� FALSE�� ���ϵȴ�.
		if (SendProc() == FALSE)
			return FALSE;		
		return TRUE;

	case FD_READ:
		// RecvProc()���� ������ �߻��ϸ� FALSE�� ���ϵȴ�.
		if(RecvProc() == FALSE)
			return FALSE;	

		return TRUE;

	case FD_CLOSE:
		return FALSE;

	default:
		ErrorTextOut(_T("NetworkProc(). �� �� ���� ��Ŷ"));
		return TRUE;
	}
}

// ��Ʈ��ũ ó�� ��, Recv ó�� �Լ�
BOOL RecvProc()
{
	// �׽�Ʈ �ڵ�
	static  int a = 0;
	
	// bConnectFlag(Ŀ��Ʈ ����)�� FALSE��� ���� ���� �ȵȰŴ� �׳� ����.
	if (bConnectFlag == FALSE)
		return TRUE;

	//////////////////////////////////////////////////
	// recv()
	// ������ ���� �� ����. WSAGetLastError()�� ������ ���߸� �������� �� �ִ�.
	//////////////////////////////////////////////////

	// �׽�Ʈ �ڵ�
	if (a == 0)
	{
		int* rear = (int*)RecvBuff.GetWriteBufferPtr();
		int* front = (int*)RecvBuff.GetReadBufferPtr();

		*rear = 1;
		*front = 1;
		a = 1;
	}

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
			ErrorTextOut(ErrorText);
			return FALSE;		// FALSE�� ���ϵǸ�, ������ �����.
		}
	}

	// 8. ������� ������ Rear�� �̵���Ų��.
	RecvBuff.MoveWritePos(retval -1);

	//////////////////////////////////////////////////
	// �Ϸ� ��Ŷ ó�� �κ�
	// RecvBuff�� ����ִ� ��� �ϼ� ��Ŷ�� ó���Ѵ�.
	//////////////////////////////////////////////////
	while (1)
	{
		// 1. RecvBuff�� �ּ����� ����� �ִ��� üũ. (���� = ��� ������� ���ų� �ʰ�. ��, �ϴ� �����ŭ�� ũ�Ⱑ �ִ��� üũ)		
		int len = dfNETWORK_PACKET_HEADER_SIZE;
		BYTE HeaderBuff[dfNETWORK_PACKET_HEADER_SIZE];

		// RecvBuff�� ��� ���� ���� ũ�Ⱑ ��� ũ�⺸�� �۴ٸ�, �Ϸ� ��Ŷ�� ���ٴ� ���̴� while�� ����.
		if (RecvBuff.GetUseSize() < len)
			break;		

		// 2. ����� Peek���� Ȯ���Ѵ�.		
		// Peek �ȿ�����, ��� �ؼ����� len��ŭ �д´�. ���۰� �� ���� ���� �̻�!
		int PeekSize = RecvBuff.Peek((char*)&HeaderBuff, len);

		// Peek()�� ���۰� ������� -1�� ��ȯ�Ѵ�. ���۰� ������� �ϴ� ���´�.
		if (PeekSize == -1)
		{
			int* rear = (int*)RecvBuff.GetReadBufferPtr();
			int* front = (int*)RecvBuff.GetWriteBufferPtr();

			ErrorTextOut(_T("RecvProc(). �Ϸ���Ŷ ��� ó�� �� RecvBuff�����. ���� ����"));
			return FALSE;	// FALSE�� ���ϵǸ�, �ش� ������ ������ �����.
		}

		// 3. ����� code Ȯ��(���������� �� �����Ͱ� �´°�)
		if (HeaderBuff[0] != dfNETWORK_PACKET_CODE)
		{
			int* rear = (int*)RecvBuff.GetReadBufferPtr();
			int* front = (int*)RecvBuff.GetWriteBufferPtr();
			int Size = RecvBuff.GetUseSize();

			ErrorTextOut(_T("RecvProc(). �Ϸ���Ŷ ��� ó�� �� ������Ŷ �ƴ�. ���� ����"));				
			return FALSE;	// FALSE�� ���ϵǸ�, �ش� ������ ������ �����.
		}

		// 4. ����� Len���� RecvBuff�� ������ ������ ��. (�ϼ� ��Ŷ ������ = ��� + Len + �����ڵ�)
		// ��� ���, �ϼ� ��Ŷ ����� �ȵǸ� while�� ����.
		if (RecvBuff.GetUseSize() < (len + HeaderBuff[1] + 1))
			break;

		// 5. RecvBuff���� Peek�ߴ� ����� ����� (�̹� Peek������, �׳� Remove�Ѵ�)
		RecvBuff.RemoveData(len);

		// 6. RecvBuff���� Len��ŭ ���̷ε� �ӽ� ���۷� �̴´�. (��ť�̴�. Peek �ƴ�)
		DWORD BuffArray = 0;
		char PayloadBuff[1000];
		DWORD PayloadSize = HeaderBuff[1];

		while (PayloadSize > 0)
		{
			int DequeueSize = RecvBuff.Dequeue(&PayloadBuff[BuffArray], PayloadSize);

			// Dequeue()�� ���۰� ������� -1�� ��ȯ. ���۰� ������� �ϴ� ���´�.
			if (DequeueSize == -1)
			{
				int* rear = (int*)RecvBuff.GetReadBufferPtr();
				int* front = (int*)RecvBuff.GetWriteBufferPtr();
				int Size = RecvBuff.GetUseSize();

				ErrorTextOut(_T("RecvProc(). �Ϸ���Ŷ ���̷ε� ó�� �� RecvBuff�����. ���� ����"));
				return FALSE;	// FALSE�� ���ϵǸ�, �ش� ������ ������ �����.
			}

			PayloadSize -= DequeueSize;
			BuffArray += DequeueSize;
		}

		// 7. RecvBuff���� �����ڵ� 1Byte����.	(��ť�̴�. Peek �ƴ�)
		char EndCode;
		int DequeueSize = RecvBuff.Dequeue(&EndCode, 1);
		if (DequeueSize == -1)
		{
			int* rear = (int*)RecvBuff.GetReadBufferPtr();
			int* front = (int*)RecvBuff.GetWriteBufferPtr();
			int Size = RecvBuff.GetUseSize();

			ErrorTextOut(_T("RecvProc(). �Ϸ���Ŷ �����ڵ� ó�� �� RecvBuff�����. ���� ����"));
			return FALSE;	// FALSE�� ���ϵǸ�, �ش� ������ ������ �����.
		}

		// 8. �����ڵ� Ȯ��
		if(EndCode != dfNETWORK_PACKET_END)
		{
			int* rear = (int*)RecvBuff.GetReadBufferPtr();
			int* front = (int*)RecvBuff.GetWriteBufferPtr();
			int Size = RecvBuff.GetUseSize();

			ErrorTextOut(_T("RecvProc(). �Ϸ���Ŷ �����ڵ� ó�� �� ������Ŷ �ƴ�. ���� ����"));
			return FALSE;	// FALSE�� ���ϵǸ�, �ش� ������ ������ �����.
		}

		// 9. ����� Ÿ�Կ� ���� �б�ó��. �Լ��� ó���Ѵ�.
		BOOL check = PacketProc(HeaderBuff[2], PayloadBuff);
		if (check == FALSE)
			return FALSE;
	}	
	
	return TRUE;
}

// ���� Send �Լ�
BOOL SendProc()
{
	// 1. Send���� ���� Ȯ��.
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

		// 11. ���� üũ. �����̸� �� �̻� �������� SendFlag�� FALSE�� ����� while�� ����
		if (SendSize == SOCKET_ERROR)
		{
			if (WSAGetLastError() == WSAEWOULDBLOCK)
			{
				bSendFlag = FALSE;
				break;
			}
			else
			{
				TCHAR Test[80] = _T("SendProc(). Send�� ���� �߻�. ���� ����");
				TCHAR ErrorText[80];

				swprintf_s(ErrorText, _countof(ErrorText), _T("%s %d"), Test, WSAGetLastError());
				ErrorTextOut(ErrorText);
				return FALSE;
			}
		}

		// 12. ���� ����� ��������, �� ��ŭ remove
		SendBuff.RemoveData(SendSize);

	}

	return TRUE;
}

// ��Ŷ Ÿ�Կ� ���� ��Ŷó�� �Լ�
BOOL PacketProc(BYTE PacketType, char* Packet)
{
	BOOL check;
	try
	{
		// Recv������ ó��
		switch (PacketType)
		{
			// �� ĳ���� ����
		case dfPACKET_SC_CREATE_MY_CHARACTER:
			check = PacketProc_CharacterCreate_My(Packet);
			if (check = FALSE)
				return FALSE;
			break;

			// �ٸ� ��� ĳ���� ����
		case dfPACKET_SC_CREATE_OTHER_CHARACTER:
			check = PacketProc_CharacterCreate_Other(Packet);
			if (check = FALSE)
				return FALSE;
			break;

			// ĳ���� ����
		case dfPACKET_SC_DELETE_CHARACTER:
			check = PacketProc_CharacterDelete(Packet);
			if (check = FALSE)
				return FALSE;
			break;

			// �ٸ� ĳ���� �̵� ���� ��Ŷ
		case dfPACKET_SC_MOVE_START:
			check = PacketProc_MoveStart(Packet);
			if (check = FALSE)
				return FALSE;
			break;

			// �ٸ� ĳ���� �̵� ���� ��Ŷ
		case dfPACKET_SC_MOVE_STOP:
			check = PacketProc_MoveStop(Packet);
			if (check = FALSE)
				return FALSE;
			break;

			// �ٸ� ĳ���� ���� ��Ŷ (1�� ����)
		case dfPACKET_SC_ATTACK1:
			check = PacketProc_Atk_01(Packet);
			if (check = FALSE)
				return FALSE;
			break;

			// �ٸ� ĳ���� ���� ��Ŷ (2�� ����)
		case dfPACKET_SC_ATTACK2:
			check = PacketProc_Atk_02(Packet);
			if (check = FALSE)
				return FALSE;
			break;

			// �ٸ� ĳ���� ���� ��Ŷ (3�� ����)
		case dfPACKET_SC_ATTACK3:
			check = PacketProc_Atk_03(Packet);
			if (check = FALSE)
				return FALSE;
			break;

			// ������ ��Ŷ
		case dfPACKET_SC_DAMAGE:
			check = PacketProc_Damage(Packet);
			if (check = FALSE)
				return FALSE;
			break;


		default:
			ErrorTextOut(_T("PacketProc(). �� �� ���� ��Ŷ"));
			return FALSE;
		}

	}
	catch (CException exc)
	{
		TCHAR* text = (TCHAR*)exc.GetExceptionText();
		ErrorTextOut(text);
		return FALSE;
	}

	return TRUE;
}


////////////////////////////////////////////////
// PacketProc()���� ��Ŷ ó�� �� ȣ���ϴ� �Լ�
///////////////////////////////////////////////
// [�� ĳ���� ���� ��Ŷ] ó�� �Լ�
BOOL PacketProc_CharacterCreate_My(char* Packet)
{
	if (g_pPlayerObject != NULL)
		ErrorTextOut(_T("PacketProc_CharacterCreate_My(). �ִ� ĳ���� �� �����϶�� ��!"));		// �̰� ����ó�� �ؾ��ϴ��� �����..

	// ����ȭ ���ۿ� ��Ŷ �ֱ�
	CProtocolBuff CreatePacket(dfPACKET_SC_CREATE_MY_CHARACTER_SIZE);
	for (int i = 0; i < dfPACKET_SC_CREATE_MY_CHARACTER_SIZE; ++i)
		CreatePacket << Packet[i];

	// ID, Dir, X, Y, HP �и�
	DWORD	byID;
	BYTE	byDir;
	WORD	byX;
	WORD	byY;
	BYTE	byHP;

	CreatePacket >> byID;
	CreatePacket >> byDir;
	CreatePacket >> byX;
	CreatePacket >> byY;
	CreatePacket >> byHP;

	// �÷��̾� ID, ������Ʈ Ÿ��(1: �÷��̾� / 2: ����Ʈ), X��ǥ, Y��ǥ, �ٶ󺸴� ���� ����
	g_pPlayerObject = new PlayerObject(byID, 1, byX, byY, byDir);

	// �� ĳ���� ����, HP ����
	((PlayerObject*)g_pPlayerObject)->MemberSetFunc(TRUE, byHP);

	// ���� �׼� ���� (���̵� ���)
	g_pPlayerObject->ActionInput(dfACTION_IDLE);

	// ���� ���� ��������Ʈ ����
	if(byDir == dfPACKET_MOVE_DIR_RR)
		g_pPlayerObject->SetSprite(CSpritedib::ePLAYER_STAND_R01, 5, 5);			
	else if (byDir == dfPACKET_MOVE_DIR_LL)
		g_pPlayerObject->SetSprite(CSpritedib::ePLAYER_STAND_L01, 5, 5);			// ���� ���� ��������Ʈ ���� (Idle ������)

	// ����Ʈ�� �߰�
	list.push_back(g_pPlayerObject);

	return TRUE;
}

// [�ٸ� ��� ĳ���� ���� ��Ŷ] ó�� �Լ�
BOOL PacketProc_CharacterCreate_Other(char* Packet)
{
	// ����ȭ ���ۿ� ��Ŷ �ֱ�
	CProtocolBuff OtherCreatePacket(dfPACKET_SC_CREATE_OTHER_CHARACTER_SIZE);
	for (int i = 0; i < dfPACKET_SC_CREATE_OTHER_CHARACTER_SIZE; ++i)
		OtherCreatePacket << Packet[i];

	// ID, Dir, X, Y, HP �и�
	DWORD	byID;
	BYTE	byDir;
	WORD	byX;
	WORD	byY;
	BYTE	byHP;

	OtherCreatePacket >> byID;
	OtherCreatePacket >> byDir;
	OtherCreatePacket >> byX;
	OtherCreatePacket >> byY;
	OtherCreatePacket >> byHP;

	// �÷��̾� ID, ������Ʈ Ÿ��(1: �÷��̾� / 2: ����Ʈ), X��ǥ, Y��ǥ, �ٶ󺸴� ���� ����
	BaseObject* AddPlayer = new PlayerObject(byID, 1, byX, byY, byDir);

	// �� ĳ���� ����, HP ����
	((PlayerObject*)AddPlayer)->MemberSetFunc(FALSE, byHP);

	// ���� �׼� ���� (���̵� ���)
	AddPlayer->ActionInput(dfACTION_IDLE);

	// ���� ���� ��������Ʈ ����
	if (byDir == dfDIRECTION_RIGHT)
		AddPlayer->SetSprite(CSpritedib::ePLAYER_STAND_R01, 5, 5);
	else if (byDir == dfDIRECTION_LEFT)
		AddPlayer->SetSprite(CSpritedib::ePLAYER_STAND_L01, 5, 5);			// ���� ���� ��������Ʈ ���� (Idle ������)

	// ����Ʈ�� �߰�
	list.push_back(AddPlayer);

	return TRUE;
}

// [ĳ���� ���� ��Ŷ] ó�� �Լ�
BOOL PacketProc_CharacterDelete(char* Packet)
{
	// ����ȭ ���ۿ� ��Ŷ �ֱ�
	CProtocolBuff DeletePacket(dfPACKET_SC_DELETE_CHARACTER_SIZE);
	for (int i = 0; i < dfPACKET_SC_DELETE_CHARACTER_SIZE; ++i)
		DeletePacket << Packet[i];

	// ID�и�
	DWORD	byID;
	DeletePacket >> byID;

	// 1. �ش� ��Ŷ�� ����� ID�� ������ ã�Ƴ���.
	CList<BaseObject*>::iterator itor;
	for (itor = list.begin(); itor != list.end(); itor++)
	{
		if ((*itor)->GetObjectID() == byID)
		{
			// 2. �ش� ������ ����Ʈ���� ���� �� ����
			list.erase(itor);
			return TRUE;
		}
	}		

	return FALSE;
}

// [�ٸ� ĳ���� �̵� ����] ��Ŷ
BOOL PacketProc_MoveStart(char* Packet)
{
	// ����ȭ ���ۿ� ��Ŷ �ֱ�
	CProtocolBuff MStartPacket(dfPACKET_SC_MOVE_START_SIZE);
	for (int i = 0; i < dfPACKET_SC_MOVE_START_SIZE; ++i)
		MStartPacket << Packet[i];

	// ID, Dir, X, Y �и�
	DWORD	byID;
	BYTE	byDir;
	WORD	byX;
	WORD	byY;

	MStartPacket >> byID;
	MStartPacket >> byDir;
	MStartPacket >> byX;
	MStartPacket >> byY;

	// 1. �ش� ��Ŷ�� ����� ID�� ������ ã�Ƴ���.
	CList<BaseObject*>::iterator itor;
	PlayerObject* MovePlayer;

	for (itor = list.begin(); itor != list.end(); itor++)
	{
		if ((*itor)->GetObjectID() == byID)
		{
			// 2. ��Ŷ�� ����� Dir�� ���� X,Y�� �׼��� ���� (8����)
			MovePlayer = (PlayerObject*)(*itor);

			// x, y ����
			MovePlayer->MoveCurXY(byX, byY);

			// �׼� ����(��ġ Ű ���� �� ó�� ����)
			MovePlayer->ActionInput(byDir);
			return TRUE;
		}
	}

	return FALSE;
}

// [�ٸ� ĳ���� �̵� ����] ��Ŷ
BOOL PacketProc_MoveStop(char* Packet)
{
	// ����ȭ ���ۿ� ��Ŷ �ֱ�
	CProtocolBuff MStopPacket(dfPACKET_SC_MOVE_STOP_SIZE);
	for (int i = 0; i < dfPACKET_SC_MOVE_STOP_SIZE; ++i)
		MStopPacket << Packet[i];

	// ID, Dir, X, Y �и�
	DWORD	byID;
	BYTE	byDir;
	WORD	byX;
	WORD	byY;

	MStopPacket >> byID;
	MStopPacket >> byDir;
	MStopPacket >> byX;
	MStopPacket >> byY;


	// 1. �ش� ��Ŷ�� ����� ID�� ������ ã�Ƴ���.
	CList<BaseObject*>::iterator itor;
	PlayerObject* MovePlayer;

	for (itor = list.begin(); itor != list.end(); itor++)
	{
		if ((*itor)->GetObjectID() == byID)
		{
			// 2. ��Ŷ�� ����� Dir�� ���� X,Y�� �׼��� ���� (8����)
			MovePlayer = (PlayerObject*)(*itor);

			// x, y ����
			MovePlayer->MoveCurXY(byX, byY);

			// ���̵� �׼� ����(��ġ Ű ���� �� ó�� ����)
			MovePlayer->ActionInput(dfACTION_IDLE);

			return TRUE;
		}
	}

	return FALSE;
}

// [�ٸ� ĳ���� ���� 1�� ����] ��Ŷ
BOOL PacketProc_Atk_01(char* Packet)
{
	// ����ȭ ���ۿ� ��Ŷ �ֱ�
	CProtocolBuff AtkPacket(dfPACKET_SC_ATTACK1_SIZE);
	for (int i = 0; i < dfPACKET_SC_ATTACK1_SIZE; ++i)
		AtkPacket << Packet[i];

	// ID, Dir, X, Y �и�
	DWORD	byID;
	BYTE	byDir;
	WORD	byX;
	WORD	byY;

	AtkPacket >> byID;
	AtkPacket >> byDir;
	AtkPacket >> byX;
	AtkPacket >> byY;

	// 1. �ش� ��Ŷ�� ����� ID�� ������ ã�Ƴ���.
	CList<BaseObject*>::iterator itor;
	PlayerObject* AtkPlayer;

	for (itor = list.begin(); itor != list.end(); itor++)
	{
		if ((*itor)->GetObjectID() == byID)
		{
			// 2. ��Ŷ�� ����� Dir�� ���� X,Y�� ���� ����
			AtkPlayer = (PlayerObject*)(*itor);

			// x, y ����
			AtkPlayer->MoveCurXY(byX, byY);

			// ���� ����
			AtkPlayer->DirChange(byDir);

			// �ش� ���� Ű�� ���� �� ó�� ó��
			AtkPlayer->ActionInput(dfACTION_ATTACK_01_LEFT);

			// �׸��� ���� Ÿ�� ����
			if (byDir == dfPACKET_MOVE_DIR_LL)
				AtkPlayer->ChangeNowAtkType(dfACTION_ATTACK_01_LEFT);
			if (byDir == dfPACKET_MOVE_DIR_RR)
				AtkPlayer->ChangeNowAtkType(dfACTION_ATTACK_01_RIGHT);

			return TRUE;
		}
	}

	return FALSE;
}

// [�ٸ� ĳ���� ���� 2�� ����] ��Ŷ
BOOL PacketProc_Atk_02(char* Packet)
{
	// ����ȭ ���ۿ� ��Ŷ �ֱ�
	CProtocolBuff AtkPacket(dfPACKET_SC_ATTACK2_SIZE);
	for (int i = 0; i < dfPACKET_SC_ATTACK2_SIZE; ++i)
		AtkPacket << Packet[i];

	// ID, Dir, X, Y �и�
	DWORD	byID;
	BYTE	byDir;
	WORD	byX;
	WORD	byY;

	AtkPacket >> byID;
	AtkPacket >> byDir;
	AtkPacket >> byX;
	AtkPacket >> byY;

	// 1. �ش� ��Ŷ�� ����� ID�� ������ ã�Ƴ���.
	CList<BaseObject*>::iterator itor;
	PlayerObject* AtkPlayer;

	for (itor = list.begin(); itor != list.end(); itor++)
	{
		if ((*itor)->GetObjectID() == byID)
		{
			// 2. ��Ŷ�� ����� Dir�� ���� X,Y�� �׼��� ���� (8����)
			AtkPlayer = (PlayerObject*)(*itor);

			// x, y ����
			AtkPlayer->MoveCurXY(byX, byY);

			// ���� ����
			AtkPlayer->DirChange(byDir);

			// �ش� ���� Ű�� ���� �� ó�� ó��
			AtkPlayer->ActionInput(dfACTION_ATTACK_02_LEFT);

			// �׸��� ���� Ÿ�� ����
			if (byDir == dfPACKET_MOVE_DIR_LL)
				AtkPlayer->ChangeNowAtkType(dfACTION_ATTACK_02_LEFT);
			if (byDir == dfPACKET_MOVE_DIR_RR)
				AtkPlayer->ChangeNowAtkType(dfACTION_ATTACK_02_RIGHT);

			return TRUE;
		}
	}

	return FALSE;
}

// [�ٸ� ĳ���� ���� 3�� ����] ��Ŷ
BOOL PacketProc_Atk_03(char* Packet)
{
	// ����ȭ ���ۿ� ��Ŷ �ֱ�
	CProtocolBuff AtkPacket(dfPACKET_SC_ATTACK3_SIZE);
	for (int i = 0; i < dfPACKET_SC_ATTACK3_SIZE; ++i)
		AtkPacket << Packet[i];

	// ID, Dir, X, Y �и�
	DWORD	byID;
	BYTE	byDir;
	WORD	byX;
	WORD	byY;

	AtkPacket >> byID;
	AtkPacket >> byDir;
	AtkPacket >> byX;
	AtkPacket >> byY;

	// 1. �ش� ��Ŷ�� ����� ID�� ������ ã�Ƴ���.
	CList<BaseObject*>::iterator itor;
	PlayerObject* AtkPlayer;

	for (itor = list.begin(); itor != list.end(); itor++)
	{
		if ((*itor)->GetObjectID() == byID)
		{
			// 2. ��Ŷ�� ����� Dir�� ���� X,Y�� �׼��� ���� (8����)
			AtkPlayer = (PlayerObject*)(*itor);

			// x, y ����
			AtkPlayer->MoveCurXY(byX, byY);

			// ���� ����
			AtkPlayer->DirChange(byDir);

			// �ش� ���� Ű�� ���� �� ó�� ó��
			AtkPlayer->ActionInput(dfACTION_ATTACK_03_LEFT);

			// �׸��� ���� Ÿ�� ����
			if (byDir == dfPACKET_MOVE_DIR_LL)
				AtkPlayer->ChangeNowAtkType(dfACTION_ATTACK_03_LEFT);
			if (byDir == dfPACKET_MOVE_DIR_RR)
				AtkPlayer->ChangeNowAtkType(dfACTION_ATTACK_03_RIGHT);

			return TRUE;
		}
	}

	return FALSE;
}

// [������] ��Ŷ
BOOL PacketProc_Damage(char* Packet)
{
	// ����ȭ ���ۿ� ��Ŷ �ֱ�
	CProtocolBuff DamagePacket(dfPACKET_SC_DAMAGE_SIZE);
	for (int i = 0; i < dfPACKET_SC_DAMAGE_SIZE; ++i)
		DamagePacket << Packet[i];

	// ������ ID, ������ ID, �������� ���� HP �и�
	DWORD	AttackID;
	DWORD	DamageID;
	BYTE	DamageHP;

	DamagePacket >> AttackID;
	DamagePacket >> DamageID;
	DamagePacket >> DamageHP;


	// 1. �ش� ��Ŷ�� ����� ������ ������ ã�Ƴ���.	
	CList<BaseObject*>::iterator itor;

	for (itor = list.begin(); itor != list.end(); itor++)
	{
		if ((*itor)->GetObjectID() == DamageID)
		{
			// 2. ��Ŷ�� ����� HP�� �������� HP�� �ݿ�
			PlayerObject* DamagePlayer = (PlayerObject*)(*itor);

			// HP ����
			DamagePlayer->DamageFunc(DamageHP, NULL);

			// ����, ���� �����ڿ��ٸ� �� hp�� ���ҵǾ��� ���̴�. �� HP�� 0�� �Ǹ� ���� ����
			if (((PlayerObject*)g_pPlayerObject)->GetHP() == 0)
				return FALSE;

			break;
		}
	}

	// 3. ������ ������ ã�Ƴ� �� ����Ʈ ����
	for (itor = list.begin(); itor != list.end(); itor++)
	{
		if ((*itor)->GetObjectID() == AttackID)
		{
			// 4. ������ ��ü���� ������ ����. �����ڰ� ������ ����Ʈ ���� ����.			
			PlayerObject* AtkPlayer = (PlayerObject*)(*itor);

			// -1�� ������, HP�� �ݿ��� ���ϰڴٴ� ��.
			AtkPlayer->DamageFunc(-1, DamageID);

			// 5. �����ڰ� ������ ����Ʈ ����!
			if (DamageID != 0)
			{
				// �������� ���� Ÿ��(���� 1,2,3��)�� ���� ����Ʈ ����
				BaseObject* Effect = new EffectObject(NULL, 2, AtkPlayer->GetCurX(), AtkPlayer->GetCurY(), AtkPlayer->GetNowAtkType());

				// ������ ����Ʈ�� ���� ��������Ʈ ����
				Effect->SetSprite(CSpritedib::eEFFECT_SPARK_01, 4, 4);

				// ����Ʈ�� ���
				list.push_back(Effect);
			}

			return TRUE;
		}
	}

	return FALSE;
}


/////////////////////////
// Send ��Ŷ ����� �Լ�
/////////////////////////
// �̵� ��Ŷ �����
void SendProc_MoveStart(char* header, char* packet, int dir, int x, int y)
{
	// ��� ����
	CProtocolBuff* headerBuff = (CProtocolBuff*)header;

	BYTE byCode = dfNETWORK_PACKET_CODE;
	BYTE bySize = dfPACKET_CS_MOVE_START_SIZE;
	BYTE byType = dfPACKET_CS_MOVE_START;

	*headerBuff << byCode;
	*headerBuff << bySize;
	*headerBuff << byType;

	// �̵� ���� ��Ŷ ����
	CProtocolBuff* paylodBuff = (CProtocolBuff*)packet;

	BYTE byDir = dir;
	WORD byX = x;
	WORD byY = y;

	*paylodBuff << byDir;
	*paylodBuff << byX;
	*paylodBuff << byY;
}

// ���� ��Ŷ �����
void SendProc_MoveStop(char* header, char* packet, int dir, int x, int y)
{
	// ��� ����
	CProtocolBuff* headerBuff = (CProtocolBuff*)header;

	BYTE byCode = dfNETWORK_PACKET_CODE;
	BYTE bySize = dfPACKET_CS_MOVE_STOP_SIZE;
	BYTE byType = dfPACKET_CS_MOVE_STOP;

	*headerBuff << byCode;
	*headerBuff << bySize;
	*headerBuff << byType;

	// ���� ��Ŷ ����
	CProtocolBuff* paylodBuff = (CProtocolBuff*)packet;

	BYTE byDir = dir;
	WORD byX = x;
	WORD byY = y;

	*paylodBuff << byDir;
	*paylodBuff << byX;
	*paylodBuff << byY;
}

// ���� ��Ŷ ����� (1�� ����)
void SendProc_Atk_01(char* header, char* packet, int dir, int x, int y)
{
	// ��� ����
	CProtocolBuff* headerBuff = (CProtocolBuff*)header;

	BYTE byCode = dfNETWORK_PACKET_CODE;
	BYTE bySize = dfPACKET_CS_ATTACK1_SIZE;
	BYTE byType = dfPACKET_CS_ATTACK1;

	*headerBuff << byCode;
	*headerBuff << bySize;
	*headerBuff << byType;

	// ���� ��Ŷ ����
	CProtocolBuff* paylodBuff = (CProtocolBuff*)packet;

	BYTE byDir = dir;
	WORD byX = x;
	WORD byY = y;

	*paylodBuff << byDir;
	*paylodBuff << byX;
	*paylodBuff << byY;
}

// ���� ��Ŷ ����� (2�� ����)
void SendProc_Atk_02(char* header, char* packet, int dir, int x, int y)
{

	// ��� ����
	CProtocolBuff* headerBuff = (CProtocolBuff*)header;

	BYTE byCode = dfNETWORK_PACKET_CODE;
	BYTE bySize = dfPACKET_CS_ATTACK2_SIZE;
	BYTE byType = dfPACKET_CS_ATTACK2;

	*headerBuff << byCode;
	*headerBuff << bySize;
	*headerBuff << byType;

	// ���� ��Ŷ ����
	CProtocolBuff* paylodBuff = (CProtocolBuff*)packet;

	BYTE byDir = dir;
	WORD byX = x;
	WORD byY = y;

	*paylodBuff << byDir;
	*paylodBuff << byX;
	*paylodBuff << byY;
}

// ���� ��Ŷ ����� (3�� ����)
void SendProc_Atk_03(char* header, char* packet, int dir, int x, int y)
{
	// ��� ����
	CProtocolBuff* headerBuff = (CProtocolBuff*)header;

	BYTE byCode = dfNETWORK_PACKET_CODE;
	BYTE bySize = dfPACKET_CS_ATTACK3_SIZE;
	BYTE byType = dfPACKET_CS_ATTACK3;

	*headerBuff << byCode;
	*headerBuff << bySize;
	*headerBuff << byType;

	// ���� ��Ŷ ����
	CProtocolBuff* paylodBuff = (CProtocolBuff*)packet;

	BYTE byDir = dir;
	WORD byX = x;
	WORD byY = y;

	*paylodBuff << byDir;
	*paylodBuff << byX;
	*paylodBuff << byY;
}


/////////////////////////
// Send �����ۿ� ������ �ֱ�
/////////////////////////
BOOL SendPacket(char* header, char* packet)
{
	// 1. �� ĳ���Ͱ� �����Ǿ��ִ� �������� üũ. ���� ���� �ȵ����� �׳� ���ư���.
	if (g_pPlayerObject == NULL)
		return TRUE;

	// 2. ���� q�� ��� �ֱ�
	CProtocolBuff* headerBuff = (CProtocolBuff*)header;

	int Size = dfNETWORK_PACKET_HEADER_SIZE;
	DWORD BuffArray = 0;
	while (Size > 0)
	{
		int EnqueueCheck = SendBuff.Enqueue(headerBuff->GetBufferPtr() + BuffArray, Size);
		if (EnqueueCheck == -1)
		{
			ErrorTextOut(_T("SendPacket() ��� �ִ� �� ����ť ����. ��������"));
			return FALSE;
		}

		Size -= EnqueueCheck;
		BuffArray += EnqueueCheck;

	}

	// 3. ���� q�� ���̷ε� �ֱ�
	CProtocolBuff* payloadBuff = (CProtocolBuff*)packet;

	BYTE PayloadLen;
	*headerBuff >> PayloadLen;
	*headerBuff >> PayloadLen;

	BuffArray = 0;
	while (PayloadLen > 0)
	{
		int EnqueueCheck = SendBuff.Enqueue(payloadBuff->GetBufferPtr() + BuffArray, PayloadLen);
		if (EnqueueCheck == -1)
		{
			ErrorTextOut(_T("SendPacket() ���̷ε� �ִ� �� ����ť ����. ��������"));
			return FALSE;
		}

		PayloadLen -= EnqueueCheck;
		BuffArray += EnqueueCheck;
	}

	// 4. ���� q�� �����ڵ� �ֱ�
	char EndCode = dfNETWORK_PACKET_END;
	int EnqueueCheck = SendBuff.Enqueue(&EndCode, 1);
	if (EnqueueCheck == -1)
	{
		ErrorTextOut(_T("SendPacket() �����ڵ� �ִ� �� ����ť ����. ��������"));
		return FALSE;
	}

	// 5. ���� Send �õ� �Լ� ȣ��
	SendProc();

	return TRUE;
}