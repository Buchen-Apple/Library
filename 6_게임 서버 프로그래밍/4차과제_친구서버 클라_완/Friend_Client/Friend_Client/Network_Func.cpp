#include "stdafx.h"
#include "Network_Func.h"

// ���� ����
SOCKET sock;							// �� ����	
#define SERVERIP	L"127.0.0.1"		// ���� IP
CRingBuff m_SendBuff;					// �� ���� ����
UINT64 g_uiMyAccount = 0;				// ���� �α����� ȸ���� ȸ����ȣ
TCHAR g_tMyNickName[dfNICK_MAX_LEN];	// ���� �α����� ȸ���� �г���;

// ���� �ʱ�ȭ, ���� ����, Ŀ��Ʈ �� �Լ�
bool Network_Init()
{
	// ���� �ʱ�ȭ
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
	{
		_tprintf(_T("WSAStartup ����!\n"));
		return false;
	}

	// ���� ����
	sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sock == INVALID_SOCKET)
	{
		_tprintf(_T("sock / socket ����!\n"));
		return false;
	}

	// Connect
	SOCKADDR_IN clientaddr;
	ZeroMemory(&clientaddr, sizeof(clientaddr));	
	clientaddr.sin_family = AF_INET;
	clientaddr.sin_port = htons(dfNETWORK_PORT);	
	InetPton(AF_INET, SERVERIP, &clientaddr.sin_addr.s_addr);

	DWORD dCheck = connect(sock, (SOCKADDR*)&clientaddr, sizeof(clientaddr));
	if (dCheck == SOCKET_ERROR) 
	{
		_tprintf(_T("connect() ����. Error : %d\n"), WSAGetLastError());
		return false;
	}

	return true;
}

// �޴��� ���� �׼� ó��
bool PacketProc(int SelectNum)
{
	bool check;
	switch (SelectNum)
	{
		// ȸ�� �߰�
		case 1:
			check = Network_Res_AccountAdd();
			if (check == false)
				return false;
			return true;

		// �α���
		case 2:
			check = Network_Res_Login();
			if (check == false)
				return false;
			return true;

		// ȸ�� ���
		case 3:			
			check = Network_Res_AccountList();
			if (check == false)
				return false;
			return true;

		// ģ�� ���
		case 4:
			check = Network_Res_FriendList();
			if (check == false)
				return false;
			return true;

		// ���� ģ����û
		case 5:
			check = Network_Res_ReplyList();
			if (check == false)
				return false;
			return true;

		// ���� ģ����û
		case 6:
			check = Network_Res_RequestList();
			if (check == false)
				return false;
			return true;

		// ģ����û ������
		case 7:
			check = Network_Res_FriendRequest();
			if (check == false)
				return false;
			return true;

		// ���� ģ����û ���
		case 8:
			check = Network_Res_FriendCancel();
			if (check == false)
				return false;
			return true;

		// ���� ģ����û ����
		case 9:
			check = Network_Res_FriendAgree();
			if (check == false)
				return false;
			return true;

		// ���� ģ����û ����
		case 10:
			check = Network_Res_FriendDeny();
			if (check == false)
				return false;
			return true;

		// ģ�� ����
		case 11:
			check = Network_Res_FriendRemove();
			if (check == false)
				return false;
			return true;

		default:
			fputs("�˼� ���� �޴�\n", stdout);
			return true;		
	}

}

// ��Ŷ ��� ����
void CreateHeader(CProtocolBuff* headerBuff, WORD MsgType, WORD PayloadSize)
{
	BYTE byCode = dfPACKET_CODE;
	*headerBuff << byCode;
	*headerBuff << MsgType;
	*headerBuff << PayloadSize;
}

// �� �α������� ����ϴ� �Լ�
void LoginShow()
{
	if (g_uiMyAccount == 0)
		fputs("# �α����� �ȵǾ� �ֽ��ϴ�.\n\n", stdout);
	else
		_tprintf_s(_T("\n�г��� : %s\nAccountNo : %lld\n\n"), g_tMyNickName, g_uiMyAccount);
}





//////////////////////////
// ��Ŷ ���� �Լ���
/////////////////////////
// "ȸ�� �߰�" ��Ŷ ����
bool Network_Res_AccountAdd()
{
	// 1. �߰��� ȸ�� �г��� �Է� (scanf�� ���� ������� ��)
	TCHAR Nick[dfNICK_MAX_LEN];
	fputs("ȸ�� �г��� �Է�:", stdout);

	_tscanf_s(_T("%s"), Nick, (UINT)sizeof(Nick));

	CProtocolBuff header(dfHEADER_SIZE);
	CProtocolBuff payload;

	// 2. Payload �����
	for(int i=0; i<dfNICK_MAX_LEN; ++i)
		payload << Nick[i];

	// 3. ��� �����
	CreateHeader(&header, df_REQ_ACCOUNT_ADD, payload.GetUseSize());

	// 4. ������ SendBuff�� �ֱ�
	SendPacket(&header, &payload);

	// 5. ������ Send
	SendProc();

	// 6. ���ú� �Ѵ�. 
	CProtocolBuff RecvBuff;
	bool check = RecvProc(&RecvBuff, df_RES_ACCOUNT_ADD);
	if (check == false)
		return false;

	// 7. ��Ŷ�� �޾����� ��Ŷó���Ѵ�.
	Network_Req_AccountAdd(&RecvBuff);

	return true;

}

// "�α���" ��Ŷ ����
bool Network_Res_Login()
{
	// 1. �α��� �� ȸ���� ID �Է� (scanf�� ���� ������� ��)
	fputs("�α��� AccountNo:", stdout);

	UINT64 AccountNo;
	_tscanf_s(_T("%lld"), &AccountNo);

	CProtocolBuff header(dfHEADER_SIZE);
	CProtocolBuff payload;

	// 2. Payload �����
	payload << AccountNo;

	// 3. ��� �����
	CreateHeader(&header, df_REQ_LOGIN, payload.GetUseSize());

	// 4. ������ SendBuff�� �ֱ�
	SendPacket(&header, &payload);

	// 5. ������ Send
	SendProc();

	// 6. ���ú� �Ѵ�.
	CProtocolBuff RecvBuff;
	bool check = RecvProc(&RecvBuff, df_RES_LOGIN);
	if (check == false)
		return false;

	// 7. ��Ŷ�� �޾����� ��Ŷó���Ѵ�.
	Network_Req_Login(&RecvBuff);

	return true;

}

// "ȸ�� ��� ��û" ��Ŷ ����
bool Network_Res_AccountList()
{
	CProtocolBuff header(dfHEADER_SIZE);
	CProtocolBuff payload;

	// 1. Payload ����� (���̷ε� ����)

	// 2. ��� �����
	CreateHeader(&header, df_REQ_ACCOUNT_LIST, payload.GetUseSize());

	// 4. ������ SendBuff�� �ֱ�
	SendPacket(&header, &payload);

	// 5. ������ Send
	SendProc();

	// 6. ���ú� �Ѵ�.
	CProtocolBuff RecvBuff;
	bool check = RecvProc(&RecvBuff, df_RES_ACCOUNT_LIST);
	if (check == false)
		return false;

	// 7. ��Ŷ�� �޾����� ��Ŷó���Ѵ�.
	Network_Req_AccountList(&RecvBuff);

	return true;

}

// "ģ����� ��û" ��Ŷ ����
bool Network_Res_FriendList()
{
	CProtocolBuff header(dfHEADER_SIZE);
	CProtocolBuff payload;

	// 1. Payload ����� (���̷ε� ����)

	// 2. ��� �����
	CreateHeader(&header, df_REQ_FRIEND_LIST, payload.GetUseSize());

	// 4. ������ SendBuff�� �ֱ�
	SendPacket(&header, &payload);

	// 5. ������ Send
	SendProc();

	// 6. ���ú� �Ѵ�.
	CProtocolBuff RecvBuff;
	bool check = RecvProc(&RecvBuff, df_RES_FRIEND_LIST);
	if (check == false)
		return false;

	// 7. ��Ŷ�� �޾����� ��Ŷó���Ѵ�.
	Network_Req_FriendList(&RecvBuff);

	return true;
}

// "���� ģ����û ����" ��Ŷ ����
bool Network_Res_ReplyList()
{
	CProtocolBuff header(dfHEADER_SIZE);
	CProtocolBuff payload;

	// 1. Payload ����� (���̷ε� ����)

	// 2. ��� �����
	CreateHeader(&header, df_REQ_FRIEND_REPLY_LIST, payload.GetUseSize());

	// 4. ������ SendBuff�� �ֱ�
	SendPacket(&header, &payload);

	// 5. ������ Send
	SendProc();

	// 6. ���ú� �Ѵ�.
	CProtocolBuff RecvBuff;
	bool check = RecvProc(&RecvBuff, df_RES_FRIEND_REPLY_LIST);
	if (check == false)
		return false;

	// 7. ��Ŷ�� �޾����� ��Ŷó���Ѵ�.
	Network_Req_ReplyList(&RecvBuff);

	return true;
}

// "���� ģ����û ����" ��Ŷ ����
bool Network_Res_RequestList()
{
	CProtocolBuff header(dfHEADER_SIZE);
	CProtocolBuff payload;

	// 1. Payload ����� (���̷ε� ����)

	// 2. ��� �����
	CreateHeader(&header, df_REQ_FRIEND_REQUEST_LIST, payload.GetUseSize());

	// 4. ������ SendBuff�� �ֱ�
	SendPacket(&header, &payload);

	// 5. ������ Send
	SendProc();

	// 6. ���ú� �Ѵ�.
	CProtocolBuff RecvBuff;
	bool check = RecvProc(&RecvBuff, df_RES_FRIEND_REQUEST_LIST);
	if (check == false)
		return false;

	// 7. ��Ŷ�� �޾����� ��Ŷó���Ѵ�.
	Network_Req_RequestList(&RecvBuff);

	return true;
}

// "ģ����û ������" ��Ŷ ����
bool Network_Res_FriendRequest()
{
	// 1. ģ�� ��û�� ȸ�� ID �Է� (scanf�� ���� ������� ��)
	fputs("��û�� AccountNo �Է�:", stdout);

	UINT64 AccountNo;
	_tscanf_s(_T("%lld"), &AccountNo);

	CProtocolBuff header(dfHEADER_SIZE);
	CProtocolBuff payload;

	// 1. Payload �����
	payload << AccountNo;

	// 2. ��� �����
	CreateHeader(&header, df_REQ_FRIEND_REQUEST, payload.GetUseSize());

	// 4. ������ SendBuff�� �ֱ�
	SendPacket(&header, &payload);

	// 5. ������ Send
	SendProc();

	// 6. ���ú� �Ѵ�.
	CProtocolBuff RecvBuff;
	bool check = RecvProc(&RecvBuff, df_RES_FRIEND_REQUEST);
	if (check == false)
		return false;

	// 7. ��Ŷ�� �޾����� ��Ŷó���Ѵ�.
	Network_Req_FriendRequest(&RecvBuff);

	return true;

}

// "ģ����û ���" ��Ŷ ����
bool Network_Res_FriendCancel()
{
	// 1. ���� ��û ����� ȸ�� ID �Է� (scanf�� ���� ������� ��)
	fputs("��û ����� AccountNo �Է�:", stdout);

	UINT64 AccountNo;
	_tscanf_s(_T("%lld"), &AccountNo);

	CProtocolBuff header(dfHEADER_SIZE);
	CProtocolBuff payload;

	// 1. Payload �����
	payload << AccountNo;

	// 2. ��� �����
	CreateHeader(&header, df_REQ_FRIEND_CANCEL, payload.GetUseSize());

	// 4. ������ SendBuff�� �ֱ�
	SendPacket(&header, &payload);

	// 5. ������ Send
	SendProc();

	// 6. ���ú� �Ѵ�.
	CProtocolBuff RecvBuff;
	bool check = RecvProc(&RecvBuff, df_RES_FRIEND_CANCEL);
	if (check == false)
		return false;

	// 7. ��Ŷ�� �޾����� ��Ŷó���Ѵ�.
	Network_Req_FriendCancel(&RecvBuff);

	return true;
}

// "������û ����" ��Ŷ ����
bool Network_Res_FriendAgree()
{
	// 1. ��û ������ ȸ�� ID �Է� (scanf�� ���� ������� ��)
	fputs("������ ȸ���� AccountNo �Է�:", stdout);

	UINT64 AccountNo;
	_tscanf_s(_T("%lld"), &AccountNo);

	CProtocolBuff header(dfHEADER_SIZE);
	CProtocolBuff payload;

	// 1. Payload �����
	payload << AccountNo;

	// 2. ��� �����
	CreateHeader(&header, df_REQ_FRIEND_AGREE, payload.GetUseSize());

	// 4. ������ SendBuff�� �ֱ�
	SendPacket(&header, &payload);

	// 5. ������ Send
	SendProc();

	// 6. ���ú� �Ѵ�.
	CProtocolBuff RecvBuff;
	bool check = RecvProc(&RecvBuff, df_RES_FRIEND_AGREE);
	if (check == false)
		return false;

	// 7. ��Ŷ�� �޾����� ��Ŷó���Ѵ�.
	Network_Req_FriendAgree(&RecvBuff);

	return true;
}

// "������û ����" ��Ŷ ����
bool Network_Res_FriendDeny()
{
	// 1. ������ ȸ�� ID �Է� (scanf�� ���� ������� ��)
	fputs("������ ȸ���� AccountNo �Է�:", stdout);

	UINT64 AccountNo;
	_tscanf_s(_T("%lld"), &AccountNo);

	CProtocolBuff header(dfHEADER_SIZE);
	CProtocolBuff payload;

	// 1. Payload �����
	payload << AccountNo;

	// 2. ��� �����
	CreateHeader(&header, df_REQ_FRIEND_DENY, payload.GetUseSize());

	// 4. ������ SendBuff�� �ֱ�
	SendPacket(&header, &payload);

	// 5. ������ Send
	SendProc();

	// 6. ���ú� �Ѵ�.
	CProtocolBuff RecvBuff;
	bool check = RecvProc(&RecvBuff, df_RES_FRIEND_DENY);
	if (check == false)
		return false;

	// 7. ��Ŷ�� �޾����� ��Ŷó���Ѵ�.
	Network_Req_FriendDeny(&RecvBuff);

	return true;
}

// "ģ�� ����" ��Ŷ ����
bool Network_Res_FriendRemove()
{
	// 1. ģ���� ���� ȸ�� ID �Է� (scanf�� ���� ������� ��)
	fputs("ģ�� ������ AccountNo �Է�:", stdout);

	UINT64 AccountNo;
	_tscanf_s(_T("%lld"), &AccountNo);

	CProtocolBuff header(dfHEADER_SIZE);
	CProtocolBuff payload;

	// 1. Payload �����
	payload << AccountNo;

	// 2. ��� �����
	CreateHeader(&header, df_REQ_FRIEND_REMOVE, payload.GetUseSize());

	// 4. ������ SendBuff�� �ֱ�
	SendPacket(&header, &payload);

	// 5. ������ Send
	SendProc();

	// 6. ���ú� �Ѵ�.
	CProtocolBuff RecvBuff;
	bool check = RecvProc(&RecvBuff, df_RES_FRIEND_REMOVE);
	if (check == false)
		return false;

	// 7. ��Ŷ�� �޾����� ��Ŷó���Ѵ�.
	Network_Req_FriendRemove(&RecvBuff);

	return true;
}





//////////////////////////
// ���� ��Ŷ ó�� �Լ���
/////////////////////////
// "ȸ�� �߰� ���" ��Ŷ ó��
void Network_Req_AccountAdd(CProtocolBuff* payload)
{
	// 1. ���̷ε忡�� ���� ������ AccountNo�� �˾Ƴ���.
	UINT64 AccountNo;
	*payload >> AccountNo;

	// 2. ���������� �߰��ƴٰ� ȭ�鿡 ǥ���Ѵ�.
	printf("\n�ű� ȸ�� �߰� �Ϸ� AccountNo:%lld\n\n", AccountNo);
}

// "�α��� ���" ��Ŷ ó��
void Network_Req_Login(CProtocolBuff* payload)
{
	// 1. �α��� �� ȸ���� AccountNo �˾ƿ���
	UINT64 AccountNo;
	*payload >> AccountNo;

	// ����, AccountNo�� 0�̶��, �α��� ������ ��
	if (AccountNo == 0)
	{
		fputs("\n�α��� ����...\n\n", stdout);
		return;
	}

	// 2. Account�� 0�� �ƴ϶��, AccountNo ���� ��, �г��ӵ� ���´�.
	g_uiMyAccount = AccountNo;
	*payload >> g_tMyNickName;

	// 3. �α��� ���� �޽��� ǥ��
	_tprintf_s(_T("\n�α��� ����! [�г��� : %s, AccountNo : %lld]\n\n"), g_tMyNickName, g_uiMyAccount);
}

// "ȸ����� ��û" ��Ŷ ó��
void Network_Req_AccountList(CProtocolBuff* payload)
{
	// 1. ȸ�� �� �˾ƿ���
	UINT AccountSize;
	*payload >> AccountSize;
	fputs("-----------ȸ�� ���-----------\n\n", stdout);
	printf("�� ȸ�� �� : %u��\n\n", AccountSize);

	// 2. ȸ�� ����ŭ ���鼭 AccountNo�� �г��� ������
	for (UINT i = 0; i < AccountSize; ++i)
	{
		// AccountNo
		UINT64 AccountNo;
		*payload >> AccountNo;

		// �г���
		TCHAR Nick[dfNICK_MAX_LEN];
		*payload >> Nick;

		// �ΰ� ǥ���ϱ�
		_tprintf(_T("%lld / %s \n"), AccountNo, Nick);
	}
	fputc('\n', stdout);
}

// "ģ����� ��û" ��Ŷ ó��
void Network_Req_FriendList(CProtocolBuff* payload)
{
	// 1. ģ�� �� �˾ƿ���
	UINT FriendSize;
	*payload >> FriendSize;
	fputs("-----------ģ�� ���-----------\n\n", stdout);
	printf("�� ģ�� �� : %u��\n\n", FriendSize);

	// 2. ģ�� ����ŭ ���鼭 AccountNo�� �г��� ������
	for (UINT i = 0; i < FriendSize; ++i)
	{
		// AccountNo
		UINT64 AccountNo;
		*payload >> AccountNo;

		// �г���
		TCHAR Nick[dfNICK_MAX_LEN];
		*payload >> Nick;

		// �ΰ� ǥ���ϱ�
		_tprintf(_T("�� ģ��. %lld / %s \n"), AccountNo, Nick);
	}
	fputc('\n', stdout);

}

// "���� ģ����û ���� ���" ��Ŷ ó��
void Network_Req_ReplyList(CProtocolBuff* payload)
{
	// 1. ���� ģ����û �� �˾ƿ���
	UINT FriendSize;
	*payload >> FriendSize;
	fputs("-----------���� ģ����û ���-----------\n\n", stdout);
	printf("�� ���� ��û �� : %u��\n\n", FriendSize);

	// 2. ģ�� ����ŭ ���鼭 AccountNo�� �г��� ������
	for (UINT i = 0; i < FriendSize; ++i)
	{
		// AccountNo
		UINT64 AccountNo;
		*payload >> AccountNo;

		// �г���
		TCHAR Nick[dfNICK_MAX_LEN];
		*payload >> Nick;

		// �ΰ� ǥ���ϱ�
		_tprintf(_T("��û ����! %lld / %s \n"), AccountNo, Nick);
	}
	fputc('\n', stdout);
}

// "���� ģ����û ���� ���" ��Ŷ ó��
void Network_Req_RequestList(CProtocolBuff* payload)
{
	// 1. ���� ģ����û �� �˾ƿ���
	UINT FriendSize;
	*payload >> FriendSize;
	fputs("-----------���� ģ����û ���-----------\n\n", stdout);
	printf("�� ���� ��û �� : %u��\n\n", FriendSize);

	// 2. ģ�� ����ŭ ���鼭 AccountNo�� �г��� ������
	for (UINT i = 0; i < FriendSize; ++i)
	{
		// AccountNo
		UINT64 AccountNo;
		*payload >> AccountNo;

		// �г���
		TCHAR Nick[dfNICK_MAX_LEN];
		*payload >> Nick;

		// �ΰ� ǥ���ϱ�
		_tprintf(_T("��û��... %lld / %s \n"), AccountNo, Nick);
	}
	fputc('\n', stdout);

}

// "ģ����û ������ ���" ��Ŷ ó��
void Network_Req_FriendRequest(CProtocolBuff* payload)
{
	// 1. ��û�ߴ� AccountNo �˾ƿ���
	UINT64 AccountNo;
	*payload >> AccountNo;

	// 2. ��� �˾ƿ���
	BYTE result;
	*payload >> result;

	// 3. result�� df_RESULT_FRIEND_REQUEST_OK�� �ƴ϶�� ���� �޽��� ǥ��
	if (result != df_RESULT_FRIEND_REQUEST_OK)
	{
		fputs("\nģ����û ����...\n\n", stdout);
		return;
	}

	// 4. df_RESULT_FRIEND_REQUEST_OK��� ģ����û ���� ��� ǥ��
	printf("\nģ����û ����! AccountNo : %lld\n\n", AccountNo);

}

// "ģ����û ��� ���" ��Ŷ ó��
void Network_Req_FriendCancel(CProtocolBuff* payload)
{
	// 1. ��û�ߴ� AccountNo �˾ƿ���
	UINT64 AccountNo;
	*payload >> AccountNo;

	// 2. ��� �˾ƿ���
	BYTE result;
	*payload >> result;

	// 3. result�� df_RESULT_FRIEND_CANCEL_OK�� �ƴ϶�� ���� �޽��� ǥ��
	if (result != df_RESULT_FRIEND_CANCEL_OK)
	{
		fputs("\nģ����û ��� ����...\n\n", stdout);
		return;
	}

	// 4. df_RESULT_FRIEND_CANCEL_OK��� ���� ��� ǥ��
	printf("\nģ����û ��� ����! AccountNo : %lld\n\n", AccountNo);

}

// "������û ���� ���" ��Ŷ ó��
void Network_Req_FriendAgree(CProtocolBuff* payload)
{
	// 1. ��û�ߴ� AccountNo �˾ƿ���
	UINT64 AccountNo;
	*payload >> AccountNo;

	// 2. ��� �˾ƿ���
	BYTE result;
	*payload >> result;

	// 3. result�� df_RESULT_FRIEND_AGREE_OK�� �ƴ϶�� ���� �޽��� ǥ��
	if (result != df_RESULT_FRIEND_AGREE_OK)
	{
		fputs("\n��û���� ����...\n\n", stdout);
		return;
	}

	// 4. df_RESULT_FRIEND_AGREE_OK��� ���� ��� ǥ��
	printf("\n��û���� ����! ģ���� ������ϴ�. [AccountNo : %lld]\n\n", AccountNo);

}

// "������û ���� ���" ��Ŷ ó��
void Network_Req_FriendDeny(CProtocolBuff* payload)
{
	// 1. ��û�ߴ� AccountNo �˾ƿ���
	UINT64 AccountNo;
	*payload >> AccountNo;

	// 2. ��� �˾ƿ���
	BYTE result;
	*payload >> result;

	// 3. result�� df_RESULT_FRIEND_DENY_OK�� �ƴ϶�� ���� �޽��� ǥ��
	if (result != df_RESULT_FRIEND_DENY_OK)
	{
		fputs("\n��û���� ����...\n\n", stdout);
		return;
	}

	// 4. df_RESULT_FRIEND_DENY_OK��� ���� ��� ǥ��
	printf("\n��û���� ����! AccountNo : %lld\n\n", AccountNo);
}

// "ģ������ ���" ��Ŷ ó��
void Network_Req_FriendRemove(CProtocolBuff* payload)
{
	// 1. ��û�ߴ� AccountNo �˾ƿ���
	UINT64 AccountNo;
	*payload >> AccountNo;

	// 2. ��� �˾ƿ���
	BYTE result;
	*payload >> result;

	// 3. result�� df_RESULT_FRIEND_REMOVE_OK�� �ƴ϶�� ���� �޽��� ǥ��
	if (result != df_RESULT_FRIEND_REMOVE_OK)
	{
		fputs("\nģ�� ���� ����...\n\n", stdout);
		return;
	}

	// 4. df_RESULT_FRIEND_REMOVE_OK��� ���� ��� ǥ��
	printf("\nģ�� ���� ����! ģ���� 1�� ��������ϴ�. AccountNo : %lld\n\n", AccountNo);
}




/////////////////////////
// recv ó��
/////////////////////////
bool RecvProc(CProtocolBuff* RecvBuff, WORD MsgType)
{	
	// 1. ��� recv()
	st_PACKET_HEADER header;
	int retval = recv(sock, (char*)&header, dfHEADER_SIZE, 0);

	// recv���� 0�� ���ϵǸ� ������ ����Ȱ�.
	if (retval == 0)
	{
		fputs("������ ������ ������.\n", stdout);
		return false;
	}

	// ���� ��ٸ��� �ִ� ��Ŷ�� ���� �������� üũ
	if (header.wMsgType != MsgType)
	{
		fputs("���� ���´� ��Ŷ�� ������ �ƴ�.\n", stdout);
		return false;
	}
	
	// 2. ����� ���̷ε常ŭ recv()
	retval = recv(sock, RecvBuff->GetBufferPtr(), header.wPayloadSize, 0);
	if (retval == 0)
	{
		fputs("������ ������ ������.\n", stdout);
		return false;
	}

	RecvBuff->MoveWritePos(retval);		

	return true;
}





/////////////////////////
// Send ó��
/////////////////////////
// Send���ۿ� ������ �ֱ�
bool SendPacket(CProtocolBuff* headerBuff, CProtocolBuff* payloadBuff)
{
	// 1. ���� q�� ��� �ֱ�
	int Size = headerBuff->GetUseSize();
	DWORD BuffArray = 0;
	int a = 0;
	while (Size > 0)
	{
		int EnqueueCheck = m_SendBuff.Enqueue(headerBuff->GetBufferPtr() + BuffArray, Size);
		if (EnqueueCheck == -1)
		{
			_tprintf(_T("SendPacket(). ��� �ִ� �� ����ť ����. ���� ����\n"));
			return false;
		}

		Size -= EnqueueCheck;
		BuffArray += EnqueueCheck;
	}

	// 2. ���� q�� ���̷ε� �ֱ�
	WORD PayloadLen = payloadBuff->GetUseSize();
	BuffArray = 0;
	while (PayloadLen > 0)
	{
		int EnqueueCheck = m_SendBuff.Enqueue(payloadBuff->GetBufferPtr() + BuffArray, PayloadLen);
		if (EnqueueCheck == -1)
		{
			_tprintf(_T("SendPacket(). ���̷ε� �ִ� �� ����ť ����. ���� ����\n"));
			return false;
		}

		PayloadLen -= EnqueueCheck;
		BuffArray += EnqueueCheck;
	}


	return true;
}

// Send������ �����͸� Send�ϱ�
bool SendProc()
{	
	// 1. SendBuff�� ���� �����Ͱ� �ִ��� Ȯ��.
	if (m_SendBuff.GetUseSize() == 0)
		return true;

	// 3. ���� �������� �������� ���� ������
	char* sendbuff = m_SendBuff.GetBufferPtr();

	// 4. SendBuff�� �� �������� or Send����(����)���� ��� send
	while (1)
	{
		// 5. SendBuff�� �����Ͱ� �ִ��� Ȯ��(���� üũ��)
		if (m_SendBuff.GetUseSize() == 0)
			break;

		// 6. �� ���� ���� �� �ִ� �������� ���� ���� ���
		int Size = m_SendBuff.GetNotBrokenGetSize();

		// 7. ���� ������ 0�̶�� 1�� ����. send() �� 1����Ʈ�� �õ��ϵ��� �ϱ� ���ؼ�.
		// �׷��� send()�� ������ �ߴ��� Ȯ�� ����
		if (Size == 0)
			Size = 1;

		// 8. front ������ ����
		int *front = (int*)m_SendBuff.GetReadBufferPtr();

		// 9. front�� 1ĭ �� index�� �˾ƿ���.
		int BuffArrayCheck = m_SendBuff.NextIndex(*front, 1);

		// 10. Send()
		int SendSize = send(sock, &sendbuff[BuffArrayCheck], Size, 0);

		// 11. ���� üũ. �����̸� �� �̻� �������� while�� ����. ���� Select�� �ٽ� ����
		if (SendSize == SOCKET_ERROR)
		{
			if (WSAGetLastError() == WSAEWOULDBLOCK)
				break;

			else
			{
				_tprintf(_T("SendProc(). Send�� ���� �߻�. ���� ����\n"));
				return false;
			}
		}

		// 12. ���� ����� ��������, �� ��ŭ remove
		m_SendBuff.RemoveData(SendSize);
	}

	return true;
}
