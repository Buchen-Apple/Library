#include "stdafx.h"
#include "RingBuff.h"

// Main�� ���� ���̺귯��
#include "PlayerObject.h"
#include "Network_Func.h"
#include "EffectObject.h"

// ���� ������
SOCKET client_sock;								// �� ����
BOOL bSendFlag = FALSE;							// ���� ���� �÷���
BOOL bConnectFlag = FALSE;						// Ŀ��Ʈ ���� �÷���
CRingBuff SendBuff;								// ���� ����
CRingBuff RecvBuff;								// ���ú� ����
HWND WindowHwnd;								// ������ �ڵ�.
DWORD MyID;										// �� ID�Ҵ� �����صα�
extern CList<BaseObject*> list;					// Main���� �Ҵ��� ��ü ���� ����Ʈ	
extern BaseObject* g_pPlayerObject;				// Main���� �Ҵ� �� �÷��̾� ��ü

// �ؽ�Ʈ �ƿ����� ���� ��� �Լ�
void ErrorTextOut(const TCHAR* ErrorLog)
{
	TCHAR Log[30];
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
	// bConnectFlag(Ŀ��Ʈ ����)�� FALSE��� ���� ���� �ȵȰŴ� �׳� ����.
	if (bConnectFlag == FALSE)
		return TRUE;

	//////////////////////////////////////////////////
	// recv() ó��.
	// ������ ���� �� ����. WSAGetLastError()�� ������ ���߸� �������� �� �ִ�.
	//////////////////////////////////////////////////
	while (1)
	{
		// �ӽ� ���Ź��� ����
		char TempBuff[1000];

		int retval = recv(client_sock, TempBuff, sizeof(TempBuff), 0);
		if (retval == SOCKET_ERROR)
		{
			// WSAEWOULDBLOCK������ �߻��ϸ�, ���� ���۰� ����ִٴ� ��. recv�Ұ� ���ٴ� ���̴� while�� ����
			if (WSAGetLastError() == WSAEWOULDBLOCK)
				break;

			// WSAEWOULDBLOCK������ �ƴϸ� ���� �̻��� ���̴� ���� ����
			else
			{
				ErrorTextOut(_T("RecvProc(). retval �� ������ �ƴ� ������ ��. ���� ����"));
				return FALSE;		// FALSE�� ���ϵǸ�, �ش� ������ ������ �����.
			}

		}

		// ������ ������, ���� �����ʹ� ������ ���ú� ť�� �ִ´�. ���� �Ŀ� �ؼ��Ѵ�.��
		// retval����, �̹��� �����ۿ� �־�� �ϴ� ũ�Ⱑ �ִ�.
		DWORD BuffArray = 0;
		while (retval > 0)
		{
			int EnqueueCheck = RecvBuff.Enqueue(&TempBuff[BuffArray], retval);

			// -1�̸� �����۰� ������.
			if (EnqueueCheck == -1)
			{
				ErrorTextOut(_T("RecvProc(). ��ť �� ������ ����. ���� ����"));
				return FALSE;	// FALSE�� ���ϵǸ�, �ش� ������ ������ �����.
			}

			// -1�� �ƴϸ� �ϴ� �־��ٴ� ��(����� 0�� ������ 0�� ��ȯ�ϴµ�, �̷� ���� �������� �ʴ´�.)
			retval -= EnqueueCheck;
			BuffArray += EnqueueCheck;
		}
	}


	//////////////////////////////////////////////////
	// �Ϸ� ��Ŷ ó�� �κ�
	// RecvBuff�� ����ִ� ��� �ϼ� ��Ŷ�� ó���Ѵ�.
	//////////////////////////////////////////////////
	while (1)
	{
		// 1. RecvBuff�� �ּ����� ����� �ִ��� üũ. (���� = ��� ������� ���ų� �ʰ�. ��, �ϴ� �����ŭ�� ũ�Ⱑ �ִ��� üũ)
		st_NETWORK_PACKET_HEADER st_header;
		int len = sizeof(st_header);

		// RecvBuff�� ��� ���� ���� ũ�Ⱑ ��� ũ�⺸�� �۴ٸ�, �Ϸ� ��Ŷ�� ���ٴ� ���̴� while�� ����.
		if (RecvBuff.GetUseSize() < len)
			break;
		

		// 2. ����� Peek���� Ȯ���Ѵ�.		
		DWORD BuffArray = 0;
		while (len > 0)
		{
			int PeekSize = RecvBuff.Peek((char*)&st_header + BuffArray, len);

			// Peek()�� ���۰� ������� -1�� ��ȯ�Ѵ�. ���۰� ������� �ϴ� ���´�.
			if (PeekSize == -1)
			{
				ErrorTextOut(_T("RecvProc(). �Ϸ���Ŷ ��� ó�� �� RecvBuff�����. ���� ����"));
				return FALSE;	// FALSE�� ���ϵǸ�, �ش� ������ ������ �����.
			}

			len -= PeekSize;
			BuffArray += PeekSize;
		}
		len = sizeof(st_header);	// �� while���� ���鼭 len�� ���ҽ�������, ��� ������ �ٽ� �ֱ�.


		// 3. ����� code Ȯ��(���������� �� �����Ͱ� �´°�)
		if (st_header.byCode != dfNETWORK_PACKET_CODE)
		{
			ErrorTextOut(_T("RecvProc(). �Ϸ���Ŷ ��� ó�� �� ������Ŷ �ƴ�. ���� ����"));
			return FALSE;	// FALSE�� ���ϵǸ�, �ش� ������ ������ �����.
		}


		// 4. ����� Len���� RecvBuff�� ������ ������ ��. (�ϼ� ��Ŷ ������ = ��� + Len + �����ڵ�)
		// ��� ���, �ϼ� ��Ŷ ����� �ȵǸ� while�� ����.
		if (RecvBuff.GetUseSize() < (len + st_header.bySize + 1))
			break;


		// 5. RecvBuff���� Peek�ߴ� ����� ����� (�̹� Peek������, �׳� Remove�Ѵ�)
		RecvBuff.RemoveData(len);


		// 6. RecvBuff���� Len��ŭ �ӽ� ���۷� �̴´�. (��ť�̴�. Peek �ƴ�)
		BuffArray = 0;
		char PayloadBuff[1000];
		DWORD PayloadSize = st_header.bySize;

		while (PayloadSize > 0)
		{
			int DequeueSize = RecvBuff.Dequeue(&PayloadBuff[BuffArray], PayloadSize);

			// Dequeue()�� ���۰� ������� -1�� ��ȯ. ���۰� ������� �ϴ� ���´�.
			if (DequeueSize == -1)
			{
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
			ErrorTextOut(_T("RecvProc(). �Ϸ���Ŷ �����ڵ� ó�� �� RecvBuff�����. ���� ����"));
			return FALSE;	// FALSE�� ���ϵǸ�, �ش� ������ ������ �����.
		}

		// 8. �����ڵ� Ȯ��
		if(EndCode != dfNETWORK_PACKET_END)
		{
			ErrorTextOut(_T("RecvProc(). �Ϸ���Ŷ �����ڵ� ó�� �� ������Ŷ �ƴ�. ���� ����"));
			return FALSE;	// FALSE�� ���ϵǸ�, �ش� ������ ������ �����.
		}

		// 9. ����� Ÿ�Կ� ���� �б�ó��. �Լ��� ó���Ѵ�.
		BOOL check = PacketProc(st_header.byType, PayloadBuff);
		if (check == FALSE)
			return FALSE;
	}	
	
	return TRUE;
}

// ���� Send �Լ�
BOOL SendProc()
{
	// 1. �ӽ� ���� ����
	char TempBuff[1000];

	// 2. Send���� ���� Ȯ��.
	if (bSendFlag == FALSE)
		return TRUE;

	// 3. SendBuff�� ���� �����Ͱ� �ִ��� Ȯ��.
	if (SendBuff.GetUseSize() == 0)
		return TRUE;

	int size = SendBuff.GetUseSize();
	// SendBuff�� �� �������� or Send����(����)���� ��� send
	while (1)
	{
		// 4. SendBuff�� �����Ͱ� �ִ��� Ȯ��(���� üũ��)
		if (SendBuff.GetUseSize() == 0)
			break;

		// 5. SendBuff�� �����͸� �ӽù��۷� Peek
		int PeekSize = SendBuff.Peek(TempBuff, size);
		if (PeekSize == -1)
		{
			ErrorTextOut(_T("SendProc(). Peek�� ���� �����. ���� ����"));
			return FALSE;
		}

		// 6. Send()
		int SendSize = send(client_sock, TempBuff, PeekSize, 0);

		// 7. ���� üũ. �����̸� �� �̻� �������� SendFlag�� FALSE�� ����� while�� ����
		if (SendSize == SOCKET_ERROR)
		{
			if (WSAGetLastError() == WSAEWOULDBLOCK)
			{
				bSendFlag = FALSE;
				break;
			}
			else
			{
				ErrorTextOut(_T("SendProc(). Send�� ���� �߻�. ���� ����"));
				return FALSE;
			}
		}

		// 8. ���� ����� ��������, �� ��ŭ remove
		size -= SendSize;
		SendBuff.RemoveData(SendSize);
	}

	return TRUE;
}

// ��Ŷ Ÿ�Կ� ���� ��Ŷó�� �Լ�
BOOL PacketProc(BYTE PacketType, char* Packet)
{
	BOOL check;
	// Recv������ ó��
	switch (PacketType)
	{
		// �� ĳ���� ����
	case dfPACKET_SC_CREATE_MY_CHARACTER:
		check = PacketProc_CharacterCreate_My(Packet);
		if (check = FALSE)
			return FALSE;
		return TRUE;

		// �ٸ� ��� ĳ���� ����
	case dfPACKET_SC_CREATE_OTHER_CHARACTER:
		check = PacketProc_CharacterCreate_Other(Packet);
		if (check = FALSE)
			return FALSE;
		return TRUE;

		// ĳ���� ����
	case dfPACKET_SC_DELETE_CHARACTER:
		check = PacketProc_CharacterDelete(Packet);
		if (check = FALSE)
			return FALSE;
		return TRUE;

		// �ٸ� ĳ���� �̵� ���� ��Ŷ
	case dfPACKET_SC_MOVE_START:
		check = PacketProc_MoveStart(Packet);
		if (check = FALSE)
			return FALSE;
		return TRUE;

		// �ٸ� ĳ���� �̵� ���� ��Ŷ
	case dfPACKET_SC_MOVE_STOP:
		check = PacketProc_MoveStop(Packet);
		if (check = FALSE)
			return FALSE;
		return TRUE;

		// �ٸ� ĳ���� ���� ��Ŷ (1�� ����)
	case dfPACKET_SC_ATTACK1:
		check = PacketProc_Atk_01(Packet);
		if (check = FALSE)
			return FALSE;
		return TRUE;

		// �ٸ� ĳ���� ���� ��Ŷ (2�� ����)
	case dfPACKET_SC_ATTACK2:
		check = PacketProc_Atk_02(Packet);
		if (check = FALSE)
			return FALSE;
		return TRUE;

		// �ٸ� ĳ���� ���� ��Ŷ (3�� ����)
	case dfPACKET_SC_ATTACK3:
		check = PacketProc_Atk_03(Packet);
		if (check = FALSE)
			return FALSE;
		return TRUE;

		// ������ ��Ŷ
	case dfPACKET_SC_DAMAGE:
		check = PacketProc_Damage(Packet);
		if (check = FALSE)
			return FALSE;
		return TRUE;


	default:
		ErrorTextOut(_T("PacketProc(). �� �� ���� ��Ŷ"));
		return FALSE;
	}
}


////////////////////////////////////////////////
// PacketProc()���� ��Ŷ ó�� �� ȣ���ϴ� �Լ�
///////////////////////////////////////////////
// [�� ĳ���� ���� ��Ŷ] ó�� �Լ�
BOOL PacketProc_CharacterCreate_My(char* Packet)
{
	if (g_pPlayerObject != NULL)
		ErrorTextOut(_T("PacketProc_CharacterCreate_My(). �ִ� ĳ���� �� �����϶�� ��!"));		// �̰� ����ó�� �ؾ��ϴ��� �����..

	stPACKET_SC_CREATE_CHARACTER* CreatePacket = (stPACKET_SC_CREATE_CHARACTER*)Packet;
	MyID = CreatePacket->ID;

	// �÷��̾� ID, ������Ʈ Ÿ��(1: �÷��̾� / 2: ����Ʈ), X��ǥ, Y��ǥ, �ٶ󺸴� ���� ����
	g_pPlayerObject = new PlayerObject(CreatePacket->ID, 1, CreatePacket->X, CreatePacket->Y, CreatePacket->Direction);
	
	// �� ĳ���� ����, HP ����
	((PlayerObject*)g_pPlayerObject)->MemberSetFunc(TRUE, CreatePacket->HP);	

	// ���� �׼� ���� (���̵� ���)
	g_pPlayerObject->ActionInput(dfACTION_IDLE);

	// ���� ���� ��������Ʈ ����
	if(CreatePacket->Direction == dfPACKET_MOVE_DIR_RR)
		g_pPlayerObject->SetSprite(CSpritedib::ePLAYER_STAND_R01, 5, 5);			
	else if (CreatePacket->Direction == dfPACKET_MOVE_DIR_LL)
		g_pPlayerObject->SetSprite(CSpritedib::ePLAYER_STAND_L01, 5, 5);			// ���� ���� ��������Ʈ ���� (Idle ������)

	// ����Ʈ�� �߰�
	list.push_back(g_pPlayerObject);

	return TRUE;
}

// [�ٸ� ��� ĳ���� ���� ��Ŷ] ó�� �Լ�
BOOL PacketProc_CharacterCreate_Other(char* Packet)
{
	stPACKET_SC_CREATE_CHARACTER* OtherCreatePacket = (stPACKET_SC_CREATE_CHARACTER*)Packet;

	// �÷��̾� ID, ������Ʈ Ÿ��(1: �÷��̾� / 2: ����Ʈ), X��ǥ, Y��ǥ, �ٶ󺸴� ���� ����
	BaseObject* AddPlayer = new PlayerObject(OtherCreatePacket->ID, 1, OtherCreatePacket->X, OtherCreatePacket->Y, OtherCreatePacket->Direction);

	// �� ĳ���� ����, HP ����
	((PlayerObject*)AddPlayer)->MemberSetFunc(FALSE, OtherCreatePacket->HP);

	// ���� �׼� ���� (���̵� ���)
	AddPlayer->ActionInput(dfACTION_IDLE);

	// ���� ���� ��������Ʈ ����
	if (OtherCreatePacket->Direction == dfDIRECTION_RIGHT)
		AddPlayer->SetSprite(CSpritedib::ePLAYER_STAND_R01, 5, 5);
	else if (OtherCreatePacket->Direction == dfDIRECTION_LEFT)
		AddPlayer->SetSprite(CSpritedib::ePLAYER_STAND_L01, 5, 5);			// ���� ���� ��������Ʈ ���� (Idle ������)

	// ����Ʈ�� �߰�
	list.push_back(AddPlayer);

	return TRUE;
}

// [ĳ���� ���� ��Ŷ] ó�� �Լ�
BOOL PacketProc_CharacterDelete(char* Packet)
{
	stPACKET_SC_CREATE_CHARACTER* DeletePacket = (stPACKET_SC_CREATE_CHARACTER*)Packet;

	// 1. �ش� ��Ŷ�� ����� ID�� ������ ã�Ƴ���.
	CList<BaseObject*>::iterator itor;
	for (itor = list.begin(); itor != list.end(); itor++)
	{
		if ((*itor)->GetObjectID() == DeletePacket->ID)
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
	stPACKET_SC_MOVE_START* MStartPacket = (stPACKET_SC_MOVE_START*)Packet;	

	// 1. �ش� ��Ŷ�� ����� ID�� ������ ã�Ƴ���.
	CList<BaseObject*>::iterator itor;
	PlayerObject* MovePlayer;

	for (itor = list.begin(); itor != list.end(); itor++)
	{
		if ((*itor)->GetObjectID() == MStartPacket->ID)
		{
			// 2. ��Ŷ�� ����� Dir�� ���� X,Y�� �׼��� ���� (8����)
			MovePlayer = (PlayerObject*)(*itor);

			// x, y ����
			MovePlayer->MoveCurXY(MStartPacket->X, MStartPacket->Y);

			// �׼� ����(��ġ Ű ���� �� ó�� ����)
			MovePlayer->ActionInput(MStartPacket->Direction);
			return TRUE;
		}
	}

	return FALSE;
}

// [�ٸ� ĳ���� �̵� ����] ��Ŷ
BOOL PacketProc_MoveStop(char* Packet)
{
	stPACKET_SC_MOVE_STOP* MStopPacket = (stPACKET_SC_MOVE_STOP*)Packet;

	// 1. �ش� ��Ŷ�� ����� ID�� ������ ã�Ƴ���.
	CList<BaseObject*>::iterator itor;
	PlayerObject* MovePlayer;

	for (itor = list.begin(); itor != list.end(); itor++)
	{
		if ((*itor)->GetObjectID() == MStopPacket->ID)
		{
			// 2. ��Ŷ�� ����� Dir�� ���� X,Y�� �׼��� ���� (8����)
			MovePlayer = (PlayerObject*)(*itor);

			// x, y ����
			MovePlayer->MoveCurXY(MStopPacket->X, MStopPacket->Y);

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
	stPACKET_SC_ATTACK* AtkPacket = (stPACKET_SC_ATTACK*)Packet;

	// 1. �ش� ��Ŷ�� ����� ID�� ������ ã�Ƴ���.
	CList<BaseObject*>::iterator itor;
	PlayerObject* AtkPlayer;

	for (itor = list.begin(); itor != list.end(); itor++)
	{
		if ((*itor)->GetObjectID() == AtkPacket->ID)
		{
			// 2. ��Ŷ�� ����� Dir�� ���� X,Y�� ���� ����
			AtkPlayer = (PlayerObject*)(*itor);

			// x, y ����
			AtkPlayer->MoveCurXY(AtkPacket->X, AtkPacket->Y);

			// ���� ����
			AtkPlayer->DirChange(AtkPacket->Direction);

			// �ش� ���� Ű�� ���� �� ó�� ó��
			AtkPlayer->ActionInput(dfACTION_ATTACK_01_LEFT);

			// �׸��� ���� Ÿ�� ����
			if(AtkPacket->Direction == dfPACKET_MOVE_DIR_LL)
				AtkPlayer->ChangeNowAtkType(dfACTION_ATTACK_01_LEFT);
			if (AtkPacket->Direction == dfPACKET_MOVE_DIR_RR)
				AtkPlayer->ChangeNowAtkType(dfACTION_ATTACK_01_RIGHT);

			return TRUE;
		}
	}

	return FALSE;
}

// [�ٸ� ĳ���� ���� 2�� ����] ��Ŷ
BOOL PacketProc_Atk_02(char* Packet)
{
	stPACKET_SC_ATTACK* AtkPacket = (stPACKET_SC_ATTACK*)Packet;

	// 1. �ش� ��Ŷ�� ����� ID�� ������ ã�Ƴ���.
	CList<BaseObject*>::iterator itor;
	PlayerObject* AtkPlayer;

	for (itor = list.begin(); itor != list.end(); itor++)
	{
		if ((*itor)->GetObjectID() == AtkPacket->ID)
		{
			// 2. ��Ŷ�� ����� Dir�� ���� X,Y�� �׼��� ���� (8����)
			AtkPlayer = (PlayerObject*)(*itor);

			// x, y ����
			AtkPlayer->MoveCurXY(AtkPacket->X, AtkPacket->Y);

			// ���� ����
			AtkPlayer->DirChange(AtkPacket->Direction);

			// �ش� ���� Ű�� ���� �� ó�� ó��
			AtkPlayer->ActionInput(dfACTION_ATTACK_02_LEFT);

			// �׸��� ���� Ÿ�� ����
			if (AtkPacket->Direction == dfPACKET_MOVE_DIR_LL)
				AtkPlayer->ChangeNowAtkType(dfACTION_ATTACK_02_LEFT);
			if (AtkPacket->Direction == dfPACKET_MOVE_DIR_RR)
				AtkPlayer->ChangeNowAtkType(dfACTION_ATTACK_02_RIGHT);

			return TRUE;
		}
	}

	return FALSE;
}

// [�ٸ� ĳ���� ���� 3�� ����] ��Ŷ
BOOL PacketProc_Atk_03(char* Packet)
{
	stPACKET_SC_ATTACK* AtkPacket = (stPACKET_SC_ATTACK*)Packet;

	// 1. �ش� ��Ŷ�� ����� ID�� ������ ã�Ƴ���.
	CList<BaseObject*>::iterator itor;
	PlayerObject* AtkPlayer;

	for (itor = list.begin(); itor != list.end(); itor++)
	{
		if ((*itor)->GetObjectID() == AtkPacket->ID)
		{
			// 2. ��Ŷ�� ����� Dir�� ���� X,Y�� �׼��� ���� (8����)
			AtkPlayer = (PlayerObject*)(*itor);

			// x, y ����
			AtkPlayer->MoveCurXY(AtkPacket->X, AtkPacket->Y);

			// ���� ����
			AtkPlayer->DirChange(AtkPacket->Direction);

			// �ش� ���� Ű�� ���� �� ó�� ó��
			AtkPlayer->ActionInput(dfACTION_ATTACK_03_LEFT);

			// �׸��� ���� Ÿ�� ����
			if (AtkPacket->Direction == dfPACKET_MOVE_DIR_LL)
				AtkPlayer->ChangeNowAtkType(dfACTION_ATTACK_03_LEFT);
			if (AtkPacket->Direction == dfPACKET_MOVE_DIR_RR)
				AtkPlayer->ChangeNowAtkType(dfACTION_ATTACK_03_RIGHT);

			return TRUE;
		}
	}

	return FALSE;

}

// [������] ��Ŷ
BOOL PacketProc_Damage(char* Packet)
{
	stPACKET_SC_DAMAGE* DamagePacket = (stPACKET_SC_DAMAGE*)Packet;

	// 1. �ش� ��Ŷ�� ����� ������ ������ ã�Ƴ���.	
	CList<BaseObject*>::iterator itor;	

	for (itor = list.begin(); itor != list.end(); itor++)
	{
		if ((*itor)->GetObjectID() == DamagePacket->DamageID)
		{
			// 2. ��Ŷ�� ����� HP�� �������� HP�� �ݿ�
			PlayerObject* DamagePlayer = (PlayerObject*)(*itor);

			// HP ����
			DamagePlayer->DamageFunc(DamagePacket->DamageHP, NULL);

			// ����, ���� �����ڿ��ٸ� �� hp�� ���ҵǾ��� ���̴�. �� HP�� 0�� �Ǹ� ���� ����
			if (((PlayerObject*)g_pPlayerObject)->GetHP() == 0)
				return FALSE;

			break;
		}
	}

	// 3. ������ ������ ã�Ƴ� �� ����Ʈ ����
	for (itor = list.begin(); itor != list.end(); itor++)
	{
		if ((*itor)->GetObjectID() == DamagePacket->AttackID)
		{
			// 4. ������ ��ü���� ������ ����. �����ڰ� ������ ����Ʈ ���� ����.			
			PlayerObject* AtkPlayer = (PlayerObject*)(*itor);

			// -1�� ������, HP�� �ݿ��� ���ϰڴٴ� ��.
			AtkPlayer->DamageFunc(-1, DamagePacket->DamageID);

			// 5. �����ڰ� ������ ����Ʈ ����!
			if (DamagePacket->DamageID != 0)
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
void SendProc_MoveStart(st_NETWORK_PACKET_HEADER* header, stPACKET_CS_MOVE_START* packet, int dir, int x, int y)
{
	header->byCode = dfNETWORK_PACKET_CODE;
	header->bySize = sizeof(stPACKET_CS_MOVE_START);
	header->byType = dfPACKET_CS_MOVE_START;

	packet->Direction = dir;
	packet->X = x;
	packet->Y = y;
}

// ���� ��Ŷ �����
void SendProc_MoveStop(st_NETWORK_PACKET_HEADER* header, stPACKET_CS_MOVE_STOP* packet, int dir, int x, int y)
{
	header->byCode = dfNETWORK_PACKET_CODE;
	header->bySize = sizeof(stPACKET_CS_MOVE_STOP);
	header->byType = dfPACKET_CS_MOVE_STOP;

	packet->Direction = dir;
	packet->X = x;
	packet->Y = y;
}

// ���� ��Ŷ ����� (1�� ����)
void SendProc_Atk_01(st_NETWORK_PACKET_HEADER* header, stPACKET_CS_ATTACK* packet, int dir, int x, int y)
{
	header->byCode = dfNETWORK_PACKET_CODE;
	header->bySize = sizeof(stPACKET_CS_ATTACK);
	header->byType = dfPACKET_CS_ATTACK1;

	packet->Direction = dir;
	packet->X = x;
	packet->Y = y;
}

// ���� ��Ŷ ����� (2�� ����)
void SendProc_Atk_02(st_NETWORK_PACKET_HEADER* header, stPACKET_CS_ATTACK* packet, int dir, int x, int y)
{
	header->byCode = dfNETWORK_PACKET_CODE;
	header->bySize = sizeof(stPACKET_CS_ATTACK);
	header->byType = dfPACKET_CS_ATTACK2;

	packet->Direction = dir;
	packet->X = x;
	packet->Y = y;
}

// ���� ��Ŷ ����� (3�� ����)
void SendProc_Atk_03(st_NETWORK_PACKET_HEADER* header, stPACKET_CS_ATTACK* packet, int dir, int x, int y)
{
	header->byCode = dfNETWORK_PACKET_CODE;
	header->bySize = sizeof(stPACKET_CS_ATTACK);
	header->byType = dfPACKET_CS_ATTACK3;

	packet->Direction = dir;
	packet->X = x;
	packet->Y = y;
}


/////////////////////////
// Send ť�� ������ �ֱ�
/////////////////////////
BOOL SendPacket(st_NETWORK_PACKET_HEADER* header, char* packet)
{
	// 1. �� ĳ���Ͱ� �����Ǿ��ִ� �������� üũ. ���� ���� �ȵ����� �׳� ���ư���.
	if (g_pPlayerObject == NULL)
		return TRUE;

	// 2. ���� q�� ��� �ֱ�
	int Size = sizeof(st_NETWORK_PACKET_HEADER);
	DWORD BuffArray = 0;
	while (Size > 0)
	{
		int EnqueueCheck = SendBuff.Enqueue((char*)header + BuffArray, Size);
		if (EnqueueCheck == -1)
		{
			ErrorTextOut(_T("SendPacket() ��� �ִ� �� ����ť ����. ��������"));
			return FALSE;
		}

		Size -= EnqueueCheck;
		BuffArray += EnqueueCheck;

	}

	// 3. ���� q�� ���̷ε� �ֱ�
	int PayloadLen = header->bySize;
	BuffArray = 0;
	while (PayloadLen > 0)
	{
		int EnqueueCheck = SendBuff.Enqueue(packet + BuffArray, PayloadLen);
		if(EnqueueCheck == -1)
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
	if(EnqueueCheck == -1)
	{
		ErrorTextOut(_T("SendPacket() �����ڵ� �ִ� �� ����ť ����. ��������"));
		return FALSE;
	}

	// 5. ���� Send �õ� �Լ� ȣ��
	SendProc();

	return TRUE;
}