#include "stdafx.h"
#include "Network_Func.h"
#include <map>
#include <list>

using namespace std;


// ������ �� ũ��
#define MAP_WIDTH			6400
#define MAP_HEIGHT			6400

// ���� 1���� ũ��
#define SECTOR_WIDTH		300
#define SECTOR_HEIGHT		200

// ���ӿ��� ���Ǵ� ���� ��
// ���� ���� : 6400/200 = 32, ���� ���� : 6400/300 = 21.333...(����������� 21�� ó��)
// ��� �� : �� ������ / ���� 1���� ũ��
#define SECTOR_WIDTH_COUNT		(MAP_WIDTH / SECTOR_WIDTH) + 1
#define SECTOR_HEIGHT_COUNT		(MAP_HEIGHT / SECTOR_HEIGHT) + 0

// ������ �����Ǵ� ��ġ ����Ʈ (���� �Ⱦ�����)
#define DEFAULT_X	6000
#define DEFAULT_Y	100

// ���� ����
#define PLAYER_ACTION_IDLE	0	// ����
#define PLAYER_ACTION_MOVE	1	// �̵�

// 1�����ӿ� �̵��ϴ� ��ǥ
#define PLAYER_XMOVE_PIXEL 6 // 3
#define PLAYER_YMOVE_PIXEL 4 // 2

// ���� �̵�����
#define dfRANGE_MOVE_TOP	0 + 1
#define dfRANGE_MOVE_LEFT	0 + 2
#define dfRANGE_MOVE_RIGHT	MAP_WIDTH - 3
#define dfRANGE_MOVE_BOTTOM	MAP_HEIGHT - 1

// ���巹Ŀ�� üũ ������ (�ȼ�����)
#define dfERROR_RANGE		50

// ���� �� ����
#define dfATTACK1_RANGE_X   80
#define dfATTACK2_RANGE_X   90
#define dfATTACK3_RANGE_X   100
#define dfATTACK1_RANGE_Y   10
#define dfATTACK2_RANGE_Y   10
#define dfATTACK3_RANGE_Y   20

// ���� �� ������
#define dfATTACK1_DAMAGE      1
#define dfATTACK2_DAMAGE      2
#define dfATTACK3_DAMAGE      3

// ���� ��, ���� ĳ���� �߰��� ���߱� ����, ĳ������ Y ��ǥ���� ���� ��.
#define dfATTACK_Y			  40


extern int		g_LogLevel;			// Main���� �Է��� �α� ��� ����. �ܺκ���
extern TCHAR	g_szLogBuff[1024];		// Main���� �Է��� �α� ��¿� �ӽ� ����. �ܺκ���


// �÷��̾� ����.
struct stPlayer
{
	// ȸ�� ����
	stAccount* m_Account;

	// �÷��̾� ID
	DWORD	m_dwPlayerID;

	// �÷��̾��� ����/X/Y/HP ����
	BYTE	m_byDir;
	short	m_wNowX;
	short	m_wNowY;
	BYTE	m_byHP;

	// ���� �׼�, ���� �׼�
	BYTE	m_byOldAction;
	BYTE	m_byNowAction;

	// ���� ������ �ִ� ������ X,Y�ε���
	WORD m_SectorX;
	WORD m_SectorY;

	// ���巹Ŀ���� ����, �׼� ���� ������ X,Y��ǥ ����(������ǥ)
	short	m_wActionX;
	short	m_wActionY;

	// ���巹Ŀ���� ����, NowAction�� ���� �ð�. ���� �� �ѹ��� �̵����� ������ 0
	ULONGLONG m_dwACtionTick;
};

// ȸ�� ����
// �α��� ��, �÷��̾������� ���Եȴ�.
struct stAccount
{
	// �ش� ȸ�� ����. map�� Ű
	SOCKET m_sock;

	// �α��� ��, �÷��̾� ������ ���.
	stPlayer* m_LoginPlayer = nullptr;	

	// �ش� ȸ���� IP�� Port;
	TCHAR m_tIP[30];
	WORD m_wPort;	

	// ����, ���ú� ����
	CRingBuff m_SendBuff;
	CRingBuff m_RecvBuff;

};

struct stSectorCheck
{
	// �� ���� Sector Index�� ����Ǿ� �ִ���
	DWORD m_dwCount;

	// ������ X,Y �ε��� ��ǥ ����.
	POINT m_Sector[9];
};


// ȸ�� ���� ���� map
// Key : ȸ���� SOCKET
map <SOCKET, stAccount*> map_Account;

// �� ȸ�� �� ī��Ʈ
DWORD g_dwAcceptCount = 0;	// �� ȸ�� �� ī��Ʈ

// �÷��̾� ID ����ũ ��. 1���� ����.
DWORD g_dwPlayerID = 1;

// ���Ϳ� �ִ� �÷��̾� ����.
// Value: �÷��̾� ����ü
list <stPlayer*> list_Sector[SECTOR_HEIGHT_COUNT][SECTOR_WIDTH_COUNT];







// ---------------------------------
// ��Ÿ �Լ�
// ---------------------------------
// ��Ʈ��ũ ���μ���
// ���⼭ false�� ��ȯ�Ǹ�, ���α׷��� ����ȴ�.
bool Network_Process(SOCKET* listen_sock)
{
	BEGIN("Network_Process");

	// Ŭ��� ��ſ� ����� ����
	FD_SET rset;
	FD_SET wset;

	SOCKADDR_IN clinetaddr;
	map <SOCKET, stAccount*>::iterator itor;
	map <SOCKET, stAccount*>::iterator enditor;
	TIMEVAL tval;
	tval.tv_sec = 0;
	tval.tv_usec = 0;

	itor = map_Account.begin();

	// ���� ���� ����. 
	rset.fd_count = 0;
	wset.fd_count = 0;

	rset.fd_array[rset.fd_count] = *listen_sock;
	rset.fd_count++;

	while (1)
	{
		enditor = map_Account.end();

		// ��� ����� ��ȸ�ϸ�, �ش� ������ �б� �°� ���� �¿� �ִ´�.
		// �ִٰ� 64���� �ǰų�, end�� �����ϸ� break
		while (itor != enditor)
		{
			// �ش� Ŭ���̾�Ʈ���� ���� �����Ͱ� �ִ��� üũ�ϱ� ����, ��� Ŭ�� rset�� ���� ����
			rset.fd_array[rset.fd_count++] = itor->second->m_sock;

			// ����, �ش� Ŭ���̾�Ʈ�� SendBuff�� ���� ������, wset���� ���� ����.
			if (itor->second->m_SendBuff.GetUseSize() != 0)
				wset.fd_array[wset.fd_count++] = itor->second->m_sock;

			// 64�� �� á����, ���� �����ߴ��� üũ		
			++itor;

			if (rset.fd_count == FD_SETSIZE || itor == enditor)
				break;

		}

		// Select()
		DWORD dCheck = select(0, &rset, &wset, 0, &tval);

		// Select()��� ó��
		if (dCheck == SOCKET_ERROR)
		{
			_tprintf(_T("select ����(%d)\n"), WSAGetLastError());
			return false;
		}		

		// select�� ���� 0���� ũ�ٸ� ���� �Ұ� �ִٴ� ���̴� ���� ����
		if (dCheck > 0)
		{
			// ���� ���� ó��
			DWORD rsetCount = 0;
			if(rset.fd_array[rsetCount] == *listen_sock && rset.fd_count > 0)
			{
				int addrlen = sizeof(clinetaddr);
				SOCKET client_sock = accept(*listen_sock, (SOCKADDR*)&clinetaddr, &addrlen);

				// ������ �߻��ϸ�, �׳� �� ������ ���� ��� ����
				if (client_sock == INVALID_SOCKET)
					_tprintf(_T("accept ���� %d\n"), WSAGetLastError());

				// ������ �߻����� �ʾҴٸ�, �������� ������ ��. �̿� ���� ó��
				else
					Accept(&client_sock, clinetaddr);	// Accept ó��.


				rsetCount++;
			}

			DWORD wsetCount = 0;

			while (1)
			{
				if (rsetCount < rset.fd_count)
				{
					stAccount* NowAccount = ClientSearch_AcceptList(rset.fd_array[rsetCount]);

					// Recv() ó��
					// ����, RecvProc()�Լ��� false�� ���ϵȴٸ�, �ش� ���� ���� ����
					if (RecvProc(NowAccount) == false)
						Disconnect(NowAccount);

					rsetCount++;
				}

				if (wsetCount < wset.fd_count)
				{
					stAccount* NowAccount = ClientSearch_AcceptList(wset.fd_array[wsetCount]);

					// Send() ó��
					// ����, SendProc()�Լ��� false�� ���ϵȴٸ�, �ش� ���� ���� ����
					if (SendProc(NowAccount) == false)
						Disconnect(NowAccount);

					wsetCount++;
				}

				if (rsetCount == rset.fd_count && wsetCount == wset.fd_count)
					break;

			}			
			
		}

		// ����, ��� Client�� ���� Selectó���� ��������, �̹� �Լ��� ���⼭ ����.
		if (itor == enditor)
			break;

		// select �غ�	
		rset.fd_count = 0;
		wset.fd_count = 0;
	}

	END("Network_Process");
	return true;
}

// Accept ó��
bool Accept(SOCKET* client_sock, SOCKADDR_IN clinetaddr)
{
	BEGIN("Accept");

	// -----------------------------
	// 1. ���ڷ� ���� clinet_sock�� �ش�Ǵ� ���� ����
	// -----------------------------
	stAccount* NewAccount = new stAccount;

	// -----------------------------
	// 2. �÷��̾� ���� �� ���� ����
	// -----------------------------
	stPlayer* NowPlayer = new stPlayer;

	NowPlayer->m_Account = NewAccount;
	NowPlayer->m_dwPlayerID = g_dwPlayerID;
	NowPlayer->m_byHP = 100;
	NowPlayer->m_byDir = dfPACKET_MOVE_DIR_LL;
	NowPlayer->m_wNowX = (rand() % 6000) + 100;
	NowPlayer->m_wNowY = (rand() % 6000) + 100;
	NowPlayer->m_byOldAction = PLAYER_ACTION_IDLE;
	NowPlayer->m_byNowAction = PLAYER_ACTION_IDLE;

	// ������ ������ ��ǥ ����, ��ġ�� ���� ���� (���� ��ǥ / ���� 1���� ũ��)
	NowPlayer->m_SectorX = NowPlayer->m_wNowX / SECTOR_WIDTH;
	NowPlayer->m_SectorY = NowPlayer->m_wNowY / SECTOR_HEIGHT;

	// ���巹Ŀ�׿� ���� ����. ���ʿ��� �� 0�̴�.
	NowPlayer->m_wActionX = NowPlayer->m_wActionY = 0;

	// -----------------------------
	// 3. ȸ�� ���� ���� ����. (m_LoginPlayer�� ���ο� ������ �����ؼ� ����. ���� ��� �α����̴� ������.)
	// -----------------------------
	NewAccount->m_sock = *client_sock;
	NewAccount->m_LoginPlayer = NowPlayer;

	// ���Ϳ� �÷��̾� �߰�
	list_Sector[NowPlayer->m_SectorY][NowPlayer->m_SectorX].push_back(NowPlayer);

	// IP�� Port ����
	TCHAR Buff[33];
	InetNtop(AF_INET, &clinetaddr.sin_addr, Buff, sizeof(Buff));
	WORD port = ntohs(clinetaddr.sin_port);
	_tcscpy_s(NewAccount->m_tIP, _countof(Buff), Buff);
	NewAccount->m_wPort = port;

	// -----------------------------
	// 4. ������ ������ ip, port, �÷��̾� ID �α׷� ���
	// -----------------------------
	/*SYSTEMTIME lst;
	GetLocalTime(&lst);
	_LOG(dfLOG_LEVEL_ERROR, L"[%02d/%02d/%02d %02d:%02d:%02d] Accept : [%s : %d / PlayerID : %d]\n",
		lst.wYear - 2000, lst.wMonth, lst.wDay, lst.wHour, lst.wMinute, lst.wSecond, Buff, port, g_dwPlayerID);*/
	
	// -----------------------------
	// 5. ȸ���� map�� �߰�
	// -----------------------------
	typedef pair<SOCKET, stAccount*> Itn_pair;
	map_Account.insert(Itn_pair(*client_sock, NewAccount));

	// -----------------------------
	// 6. �÷��̾� �� ����.
	// -----------------------------
	g_dwPlayerID++;	

	// -----------------------------
	// 7. "�� ����" ��Ŷ ����
	// -----------------------------
	CProtocolBuff header(dfNETWORK_PACKET_HEADER_SIZE);
	CProtocolBuff payload;
	Network_Send_CreateMyCharacter(&header, &payload, g_dwPlayerID - 1, dfPACKET_MOVE_DIR_LL, NowPlayer->m_wNowX, NowPlayer->m_wNowY, 100);

	// -----------------------------
	// 8. ������� SendBuff�� �ֱ�
	// -----------------------------
	if (SendPacket(NewAccount, &header, &payload) == FALSE)
		return FALSE;

	// -----------------------------
	// 9. �ֺ� 9������ ���� Index�� �˾ƿ´�.
	// -----------------------------
	stSectorCheck SecCheck;
	GetSector(NewAccount->m_LoginPlayer->m_SectorX, NewAccount->m_LoginPlayer->m_SectorY, &SecCheck);

	// -----------------------------
	// 10. "��"���� "�ٸ� ����� ����" ��Ŷ �ֱ�.
	// �ֺ� 9���⿡ �ִ� �������� ������ ���� ������.
	// -----------------------------
	if (Accept_Surport(NowPlayer, &SecCheck) == FALSE)
		return FALSE;

	// -----------------------------
	// 11. "���� �� �ٸ� ���"���� "�ٸ� ��� ����" ��Ŷ �ֱ� (���⼭ �ٸ������ "��" �̴�)
	// -----------------------------
	CProtocolBuff Ohter_header(dfNETWORK_PACKET_HEADER_SIZE);
	CProtocolBuff Otehr_payload;
	Network_Send_CreateOtherCharacter(&Ohter_header, &Otehr_payload, NewAccount->m_LoginPlayer->m_dwPlayerID, 
		dfPACKET_MOVE_DIR_LL, NewAccount->m_LoginPlayer->m_wNowX, NewAccount->m_LoginPlayer->m_wNowY, NewAccount->m_LoginPlayer->m_byHP);

	if (SendPacket_Sector(NowPlayer, &Ohter_header, &Otehr_payload, &SecCheck) == FALSE)
		return FALSE;

	// -----------------------------
	// 12. �� ������ �� ����
	// -----------------------------
	g_dwAcceptCount++;

	END("Accept");

	return TRUE;
}

// Accept ��, �� ���� 9���� ������ ������ ������ ������ �Ѹ���
bool Accept_Surport(stPlayer* Player, stSectorCheck* Sector)
{
	BEGIN("Accept_Surport");

	// 1. SecCheck�ȿ� �ִ� �������� ���, "��"���� �˷���� �Ѵ�.
	for (DWORD i = 0; i < Sector->m_dwCount; ++i)
	{
		// 3. itor�� "i" ��° ������ ����Ʈ�� ����Ų��.
		list <stPlayer*>::iterator itor = list_Sector[Sector->m_Sector[i].y][Sector->m_Sector[i].x].begin();
		list <stPlayer*>::iterator enditor = list_Sector[Sector->m_Sector[i].y][Sector->m_Sector[i].x].end();

		for (; itor != enditor; ++itor)
		{
			if ((*itor) == Player)
				continue;

			// "i" ������ n��° ���� ��Ŷ ����
			CProtocolBuff header(dfNETWORK_PACKET_HEADER_SIZE);
			CProtocolBuff payload;	

			BYTE Dir = dfPACKET_MOVE_DIR_RR;

			if ((*itor)->m_byDir == dfPACKET_MOVE_DIR_LL ||
				(*itor)->m_byDir == dfPACKET_MOVE_DIR_LU ||
				(*itor)->m_byDir == dfPACKET_MOVE_DIR_UU ||
				(*itor)->m_byDir == dfPACKET_MOVE_DIR_LD)
			{
				Dir = dfPACKET_MOVE_DIR_LL;
			}

			Network_Send_CreateOtherCharacter(&header, &payload, (*itor)->m_dwPlayerID,
				Dir, (*itor)->m_wNowX, (*itor)->m_wNowY, (*itor)->m_byHP);

			// 4. ������ ��Ŷ�� �̹��� ���� SendBuff�� �ֱ�.
			if (SendPacket(Player->m_Account, &header, &payload) == FALSE)
				return FALSE;

			// �̵� ���̸�, �̵� ��Ŷ ����
			if ((*itor)->m_byNowAction == PLAYER_ACTION_MOVE)
			{
				CProtocolBuff header(dfNETWORK_PACKET_HEADER_SIZE);
				CProtocolBuff payload;				

				Network_Send_MoveStart(&header, &payload, (*itor)->m_dwPlayerID,
					(*itor)->m_byDir, (*itor)->m_wNowX, (*itor)->m_wNowY);

				// 5. ������ ��Ŷ�� �̹��� ���� SendBuff�� �ֱ�.
				if (SendPacket(Player->m_Account, &header, &payload) == FALSE)
					return FALSE;

			}
		}
	}

	END("Accept_Surport");

	return TRUE;
}

// Disconnect ó��
void Disconnect(stAccount* Account)
{
	// ���ܻ��� üũ
	// 1. �ش� ������ �α��� ���� �����ΰ�.
	// NowUser�� ���� nullptr�̸�, �ش� ������ ��ã�� ��. false�� �����Ѵ�.
	if (Account == nullptr)
	{
		//_LOG(dfLOG_LEVEL_ERROR, L"Disconnect(). Accept ���� �ƴ� ������ ������� ���� �õ�\n");
		return;
	}	

	// 2. ���� ������ ������ ip, port, AccountNo ���	
	/*SYSTEMTIME lst;
	GetLocalTime(&lst);
	_LOG(dfLOG_LEVEL_DEBUG, L"[%02d/%02d/%02d %02d:%02d:%02d] Disconnect : [%s : %d / PlayerID : %d]\n",
		lst.wYear - 2000, lst.wMonth, lst.wDay, lst.wHour, lst.wMinute, lst.wSecond, 
		Account->m_tIP, Account->m_wPort, Account->m_LoginPlayer->m_dwPlayerID);*/

	_tprintf(L"Disconnect : [%s:%d / PlayerID : %d]\n", Account->m_tIP, Account->m_wPort, Account->m_LoginPlayer->m_dwPlayerID);
		
	// 3. �ش� ���� ���� 9���⿡ �ִ� �����鿡�� "�ٸ� ĳ���� ���� ����" ��Ŷ �߼�. 
	// ����ڴ� �����Ѵ�.
	CProtocolBuff header(dfNETWORK_PACKET_HEADER_SIZE);
	CProtocolBuff payload;
	Network_Send_RemoveOtherCharacteor(&header, &payload, Account->m_LoginPlayer->m_dwPlayerID);

	// �ֺ� 9������ ���� Index�� �˾ƿ´�.
	stSectorCheck SecCheck;
	GetSector(Account->m_LoginPlayer->m_SectorX, Account->m_LoginPlayer->m_SectorY, &SecCheck);

	// �ֱ�
	SendPacket_Sector(Account->m_LoginPlayer, &header, &payload, &SecCheck);

	// 4. �ش� �÷��̾, ���� List���� ����
	list_Sector[Account->m_LoginPlayer->m_SectorY][Account->m_LoginPlayer->m_SectorX].remove(Account->m_LoginPlayer);

	// 5. �ش� ������ ���� close
	SOCKET Temp = Account->m_sock;	
	closesocket(Account->m_sock);

	// 6. ���� ������ �÷��̾� ����
	delete Account->m_LoginPlayer;

	// 7. ���� ������ ȸ�� ����
	delete Account;
	map_Account.erase(Temp);

	// 8. Accept ���� �� ����
	g_dwAcceptCount--;
}

// ���巹Ŀ�� �Լ�
// ���ڷ� (���� �׼�, �׼� ���� �ð�, �׼� ���� ��ġ, (OUT)��� �� ��ǥ)�� �޴´�.
void DeadReckoning(BYTE NowAction, ULONGLONG ActionTick, int ActionStartX, int ActionStartY, int* ResultX, int* ResultY)
{
	BEGIN("DeadReckoning");

	// ----------------------------
	// �׼� ���۽ð����� ���� �ð������� �ð����� ���Ѵ�. 
	// �� �ð����� �� �������� �������� ����Ѵ�.
	// ----------------------------
	int IntervalFrame = (GetTickCount64() - ActionTick) / 40; // (40�� 1������. 25���������� ����)

	int PosX, PosY;

	// ---------------------------
	// 1. ���� ���������� X��, Y�� �󸶳� ������ ����
	// ��, PosX, PosY�� ����ϴ� ��.
	// ---------------------------
	int iMoveX = IntervalFrame * PLAYER_XMOVE_PIXEL;
	int iMoveY = IntervalFrame * PLAYER_YMOVE_PIXEL;

	// �׼ǿ� ���� �̵����� �޶���
	switch (NowAction)
	{
	case dfPACKET_MOVE_DIR_LL:
		PosX = ActionStartX - iMoveX;
		PosY = ActionStartY;
		break;

	case dfPACKET_MOVE_DIR_LU:
		PosX = ActionStartX - iMoveX;
		PosY = ActionStartY - iMoveY;
		break;

	case dfPACKET_MOVE_DIR_UU:
		PosX = ActionStartX;
		PosY = ActionStartY - iMoveY;
		break;

	case dfPACKET_MOVE_DIR_RU:
		PosX = ActionStartX + iMoveX;
		PosY = ActionStartY - iMoveY;
		break;

	case dfPACKET_MOVE_DIR_RR:
		PosX = ActionStartX + iMoveX;
		PosY = ActionStartY;
		break;

	case dfPACKET_MOVE_DIR_RD:
		PosX = ActionStartX + iMoveX;
		PosY = ActionStartY + iMoveY;
		break;

	case dfPACKET_MOVE_DIR_DD:
		PosX = ActionStartX;
		PosY = ActionStartY + iMoveY;
		break;

	case dfPACKET_MOVE_DIR_LD:
		PosX = ActionStartX - iMoveX;
		PosY = ActionStartY + iMoveY;
		break;
	}


	// -------------------------
	// 2. ���� ��ǥ�� ȭ�� �̵� ������ �������, ��� ��ŭ�� �������� ���Ѵ�.
	// -------------------------
	int RemoveFrame = 0;
	int Value;

	if (PosX <= dfRANGE_MOVE_LEFT)
	{
		Value = abs(dfRANGE_MOVE_LEFT - abs(PosX)) / PLAYER_XMOVE_PIXEL;
		RemoveFrame = max(Value, RemoveFrame);
	}

	if (PosX >= dfRANGE_MOVE_RIGHT)
	{
		Value = abs(dfRANGE_MOVE_RIGHT - abs(PosX)) / PLAYER_XMOVE_PIXEL;
		RemoveFrame = max(Value, RemoveFrame);
	}

	if (PosY <= dfRANGE_MOVE_TOP)
	{
		Value = abs(dfRANGE_MOVE_TOP - abs(PosY)) / PLAYER_YMOVE_PIXEL;
		RemoveFrame = max(Value, RemoveFrame);
	}

	if (PosY >= dfRANGE_MOVE_BOTTOM)
	{
		Value = abs(dfRANGE_MOVE_BOTTOM - abs(PosY)) / PLAYER_YMOVE_PIXEL;
		RemoveFrame = max(Value, RemoveFrame);
	}

	// -------------------------
	// 3. ��� �������� ������, �׸�ŭ ��ġ �ٽ� ���
	// -------------------------
	if (RemoveFrame > 0)
	{
		IntervalFrame -= RemoveFrame;

		iMoveX = IntervalFrame * PLAYER_XMOVE_PIXEL;
		iMoveY = IntervalFrame * PLAYER_YMOVE_PIXEL;

		switch (NowAction)
		{
		case dfPACKET_MOVE_DIR_LL:
			PosX = ActionStartX - iMoveX;
			PosY = ActionStartY;
			break;

		case dfPACKET_MOVE_DIR_LU:
			PosX = ActionStartX - iMoveX;
			PosY = ActionStartY - iMoveY;
			break;

		case dfPACKET_MOVE_DIR_UU:
			PosX = ActionStartX;
			PosY = ActionStartY - iMoveY;
			break;

		case dfPACKET_MOVE_DIR_RU:
			PosX = ActionStartX + iMoveX;
			PosY = ActionStartY - iMoveY;
			break;

		case dfPACKET_MOVE_DIR_RR:
			PosX = ActionStartX + iMoveX;
			PosY = ActionStartY;
			break;

		case dfPACKET_MOVE_DIR_RD:
			PosX = ActionStartX + iMoveX;
			PosY = ActionStartY + iMoveY;
			break;

		case dfPACKET_MOVE_DIR_DD:
			PosX = ActionStartX;
			PosY = ActionStartY + iMoveY;
			break;

		case dfPACKET_MOVE_DIR_LD:
			PosX = ActionStartX - iMoveX;
			PosY = ActionStartY + iMoveY;
			break;

		}		
	}

	// -------------------------
	// 4. ��� ����� �Ϸ�Ǿ�����, PosX�� PosY�� ���� �� ������ ��´�.
	// -------------------------
	PosX = min(PosX, dfRANGE_MOVE_RIGHT);
	PosX = max(PosX, dfRANGE_MOVE_LEFT);
	PosY = min(PosY, dfRANGE_MOVE_BOTTOM);
	PosY = max(PosY, dfRANGE_MOVE_TOP);

	// -------------------------
	// 5. ���ڷι��� �ƿ������� ����
	// -------------------------
	*ResultX = PosX;
	*ResultY = PosY;

	END("DeadReckoning");
}

// �α� ��� �Լ�
void Log(TCHAR *szString, int LogLevel)
{
	_tprintf(L"%s", szString);

	// �α� ������ Warning, Error�� ���Ϸ� �����Ѵ�.
	if (LogLevel >= dfLOG_LEVEL_WARNING)
	{
		// ���� �⵵��, �� �˾ƿ���.
		SYSTEMTIME lst;
		GetLocalTime(&lst);

		// ���� �̸� �����
		TCHAR tFileName[30];
		swprintf_s(tFileName, _countof(tFileName), L"%d%02d Log.txt", lst.wYear, lst.wMonth);

		// ���� ����
		FILE *fp;
		_tfopen_s(&fp, tFileName, L"at");

		fwprintf(fp, L"%s", szString);

		fclose(fp);

	}
}








// ---------------------------------
// ����ó���� �Լ�
// ---------------------------------
// ���� ü���� üũ
void SectorChange(stAccount* Account)
{
	BEGIN("SectorChange");

	// 1. ���� ��ǥ ����, ���� Index ���ϱ�
	WORD SectorX = Account->m_LoginPlayer->m_wNowX / SECTOR_WIDTH;
	WORD SectorY = Account->m_LoginPlayer->m_wNowY / SECTOR_HEIGHT;

	// 2. ���� ���Ϳ� Index�� ������ retrun. ���Ͱ� �ȹٲ��.
	if (Account->m_LoginPlayer->m_SectorX == SectorX &&
		Account->m_LoginPlayer->m_SectorY == SectorY)
		return;

	// 3. ������� ���� ���Ͱ� �ٲ��.
	// ���� List���� �� ���� ��, ���ο� List�� �̵�
	//_LOG(dfLOG_LEVEL_DEBUG, L"[DEBUG] ���� ����[%d, %d -> %d, %d]\n", 
	//						Account->m_LoginPlayer->m_SectorY, Account->m_LoginPlayer->m_SectorX, SectorY, SectorX);
	list_Sector[Account->m_LoginPlayer->m_SectorY][Account->m_LoginPlayer->m_SectorX].remove(Account->m_LoginPlayer);
	list_Sector[SectorY][SectorX].push_back(Account->m_LoginPlayer);

	//  ĳ���� ����, �߰� �۾� ����
	CharacterSectorUpdate(Account->m_LoginPlayer, SectorX, SectorY);
	
	// 4. �׸��� �� ���� ����
	Account->m_LoginPlayer->m_SectorX = SectorX;
	Account->m_LoginPlayer->m_SectorY = SectorY;

	// 5. ����� ���Ϳ� ���� ȭ�� ����Ʈ (������)
	//Print_Sector(Account->m_LoginPlayer);

	END("SectorChange");
}

// ���� ���� 9���� ���ϱ�
// ���ڷ� ���� X,Y�� ��������, ���ڷ� ���� ����ü�� 9������ �־��ش�.
void GetSector(int SectorX, int SectorY, stSectorCheck* Sector)
{
	BEGIN("GetSector");

	int iCurX, iCurY;

	SectorX--;
	SectorY--;

	Sector->m_dwCount = 0;

	for (iCurY = 0; iCurY < 3; iCurY++)
	{
		if (SectorY + iCurY < 0 || SectorY + iCurY >= SECTOR_HEIGHT_COUNT)
			continue;

		for (iCurX = 0; iCurX < 3; iCurX++)
		{
			if (SectorX + iCurX < 0 || SectorX + iCurX >= SECTOR_WIDTH_COUNT)
				continue;

			Sector->m_Sector[Sector->m_dwCount].x = SectorX + iCurX;
			Sector->m_Sector[Sector->m_dwCount].y = SectorY + iCurY;
			Sector->m_dwCount++;

		}
	}

	END("GetSector");
}

// ���� ���Ϳ� SendBuff
// ���ڷ� ���� Sector�� �������� SendBuff�� ���ڷ� ���� ��Ŷ �ֱ�
bool SendPacket_Sector(stPlayer* Player, CProtocolBuff* header, CProtocolBuff* payload, stSectorCheck* Sector)
{
	BEGIN("SendPacket_Sector");

	// 1. Sector���� �����鿡�� ���ڷ� ���� ��Ŷ�� �ִ´�.
	for (DWORD i = 0; i < Sector->m_dwCount; ++i)
	{
		// 2. itor�� "i" ��° ������ ����Ʈ�� ����Ų��.
		list <stPlayer*>::iterator itor = list_Sector[Sector->m_Sector[i].y][Sector->m_Sector[i].x].begin();
		list <stPlayer*>::iterator enditor = list_Sector[Sector->m_Sector[i].y][Sector->m_Sector[i].x].end();

		for (; itor != enditor; ++itor)
		{
			// 3. ����, �� �ڽ��̶�� �Ⱥ���.
			if ((*itor) == Player)
				continue;

			// NowAccount���� ���ڷ� ���� ��Ŷ ����.
			if (SendPacket((*itor)->m_Account, header, payload) == FALSE)
				return FALSE;
		}
	}

	END("SendPacket_Sector");

	return TRUE;
}

// 1�� ���Ϳ� SendBuff
// ���ڷ� ���� ���� 1���� �������� SendBuff�� ���ڷ� ���� ��Ŷ �ֱ�.
bool SendPacket_SpecialSector(stPlayer* Player, CProtocolBuff* header, CProtocolBuff* payload, int SectorX, int SectorY)
{
	BEGIN("SendPacket_SpecialSector");

	// 1. Ư�� ���� ������ SendBuff�� ��Ŷ �ֱ�.
	list <stPlayer*>::iterator itor = list_Sector[SectorY][SectorX].begin();
	list <stPlayer*>::iterator enditor = list_Sector[SectorY][SectorX].end();

	for (; itor != enditor; ++itor)
	{
		// �����Դ� �Ⱥ�����!
		if ((*itor) == Player)
			continue;

		// NowAccount���� ���ڷ� ���� ��Ŷ ����.
		if (SendPacket((*itor)->m_Account, header, payload) == FALSE)
			return FALSE;
	}

	END("SendPacket_SpecialSector");

	return TRUE;
}

// �õ� ���� 9��, �űԼ��� 9�� ���ϱ�
// ���ڷ� ���� �õ� ������ǥ, �� ������ǥ�� �������� ĳ���͸� �����ؾ��ϴ� ����, ĳ���͸� �߰��ؾ��ϴ� ���͸� ���Ѵ�.
void GetUpdateSectorArount(int OldSectorX, int OldSectorY, int NewSectorX, int NewSectorY, stSectorCheck* RemoveSector, stSectorCheck* AddSector)
{
	BEGIN("GetUpdateSectorArount");

	DWORD iCntOld, iCntCur;
	bool bFind;
	
	stSectorCheck OldSector, NewSector;

	OldSector.m_dwCount = 0;
	NewSector.m_dwCount = 0;

	RemoveSector->m_dwCount = 0;
	AddSector->m_dwCount = 0;

	// �õ� ���Ϳ� �ű� ���� ��ǥ ����.
	GetSector(OldSectorX, OldSectorY, &OldSector);
	GetSector(NewSectorX, NewSectorY, &NewSector);

	// --------------------------------------------------
	// OldSector ��, NewSector�� ���� ������ ã�Ƽ� RemoveSector�� ����.
	// --------------------------------------------------
	for (iCntOld = 0; iCntOld < OldSector.m_dwCount; ++iCntOld)
	{
		bFind = false;
		for (iCntCur = 0; iCntCur < NewSector.m_dwCount; ++iCntCur)
		{
			if (OldSector.m_Sector[iCntOld].x == NewSector.m_Sector[iCntCur].x &&
				OldSector.m_Sector[iCntOld].y == NewSector.m_Sector[iCntCur].y)
			{
				bFind = true;
				break;
			}
		}
		// bFind�� false�� �������� ��ã��. ��, �����ؾ��ϴ� Sector�̴�.
		if (bFind == false)
		{
			RemoveSector->m_Sector[RemoveSector->m_dwCount] = OldSector.m_Sector[iCntOld];
			RemoveSector->m_dwCount++;			
		}
	}

	// --------------------------------------------------
	// NewSector ��, OldSector�� ���� ������ ã�Ƽ� AddSector�� ����.
	// --------------------------------------------------
	for (iCntCur = 0; iCntCur < NewSector.m_dwCount; ++iCntCur)
	{
		bFind = false;
		for (iCntOld = 0; iCntOld < OldSector.m_dwCount; ++iCntOld)
		{
			if (OldSector.m_Sector[iCntOld].x == NewSector.m_Sector[iCntCur].x &&
				OldSector.m_Sector[iCntOld].y == NewSector.m_Sector[iCntCur].y)
			{
				bFind = true;
				break;
			}
		}
		// bFind�� false�� �������� ��ã��. ��, �߰��ؾ��ϴ� Sector�̴�.
		if (bFind == false)
		{
			AddSector->m_Sector[AddSector->m_dwCount] = NewSector.m_Sector[iCntCur];
			AddSector->m_dwCount++;
		}
	}

	END("GetUpdateSectorArount");

}

// ���Ͱ� �̵��Ǿ��� �� ȣ��Ǵ� �Լ�. ���� �������� ĳ���� ����/�߰��� üũ�Ѵ�.
void CharacterSectorUpdate(stPlayer* NowPlayer, int NewSectorX, int NowSectorY)
{
	BEGIN("CharacterSectorUpdate");

	DWORD iCnt;
	list<stPlayer*> *pSectorList;
	list<stPlayer*>::iterator itor;
	list<stPlayer*>::iterator enditor;

	// RemoveSector�� AddSector ���ϱ�.
	stSectorCheck RemoveSector, AddSector;
	GetUpdateSectorArount(NowPlayer->m_SectorX, NowPlayer->m_SectorY, NewSectorX, NowSectorY, &RemoveSector, &AddSector);

	// --------------------------------
	// RemoveSector�� ������
	// -------------------------------
	// 1. RemoveSector�� "�� ����" ��Ŷ ������
	CProtocolBuff header(dfNETWORK_PACKET_HEADER_SIZE);
	CProtocolBuff payload;

	Network_Send_RemoveOtherCharacteor(&header, &payload, NowPlayer->m_dwPlayerID);

	for (iCnt = 0; iCnt < RemoveSector.m_dwCount; ++iCnt)
	{
		// ���� 1���� ������ SendBuff�� ���ڷ� ���޹��� ��Ŷ�� �ִ´�.
		SendPacket_SpecialSector(NowPlayer, &header, &payload, RemoveSector.m_Sector[iCnt].x, RemoveSector.m_Sector[iCnt].y);
	}

	// 2. ������ "RemoveSector�� ������ ����" ��Ŷ ������
	for (iCnt = 0; iCnt < RemoveSector.m_dwCount; ++iCnt)
	{
		pSectorList = &list_Sector[RemoveSector.m_Sector[iCnt].y][RemoveSector.m_Sector[iCnt].x];

		itor = pSectorList->begin();
		enditor = pSectorList->end();

		for (; itor != enditor; ++itor)
		{
			CProtocolBuff header(dfNETWORK_PACKET_HEADER_SIZE);
			CProtocolBuff payload;

			Network_Send_RemoveOtherCharacteor(&header, &payload, (*itor)->m_dwPlayerID);

			SendPacket(NowPlayer->m_Account, &header, &payload);
		}
	}

	// --------------------------------
	// AddSector�� ������
	// -------------------------------
	// 1. AddSector�� "�� ����" ��Ŷ ������
	CProtocolBuff Create_header(dfNETWORK_PACKET_HEADER_SIZE);
	CProtocolBuff Create_payload;

	BYTE Dir = dfPACKET_MOVE_DIR_RR;
	if (NowPlayer->m_byDir == dfPACKET_MOVE_DIR_LL ||
		NowPlayer->m_byDir == dfPACKET_MOVE_DIR_LU ||
		NowPlayer->m_byDir == dfPACKET_MOVE_DIR_UU ||
		NowPlayer->m_byDir == dfPACKET_MOVE_DIR_LD)
	{
		Dir = dfPACKET_MOVE_DIR_LL;
	}

	Network_Send_CreateOtherCharacter(&Create_header, &Create_payload, NowPlayer->m_dwPlayerID,	Dir, NowPlayer->m_wNowX, NowPlayer->m_wNowY, NowPlayer->m_byHP);

	for (iCnt = 0; iCnt < AddSector.m_dwCount; ++iCnt)
	{
		// ���� 1���� ������ SendBuff�� ���ڷ� ���޹��� ��Ŷ�� �ִ´�.
		SendPacket_SpecialSector(NowPlayer, &Create_header, &Create_payload, AddSector.m_Sector[iCnt].x, AddSector.m_Sector[iCnt].y);
	}

	// 2. AddSector�� "�� �̵�" ��Ŷ ������
	CProtocolBuff Move_header(dfNETWORK_PACKET_HEADER_SIZE);
	CProtocolBuff Move_payload;

	Network_Send_MoveStart(&Move_header, &Move_payload, NowPlayer->m_dwPlayerID, NowPlayer->m_byDir, NowPlayer->m_wNowX, NowPlayer->m_wNowY);

	for (iCnt = 0; iCnt < AddSector.m_dwCount; ++iCnt)
	{
		// ���� 1���� ������ SendBuff�� ���ڷ� ���޹��� ��Ŷ�� �ִ´�.
		SendPacket_SpecialSector(NowPlayer, &Move_header, &Move_payload, AddSector.m_Sector[iCnt].x, AddSector.m_Sector[iCnt].y);
	}


	// 3. ������ AddSector�� ĳ���͵� ���� ��Ŷ ������
	for (iCnt = 0; iCnt < AddSector.m_dwCount; ++iCnt)
	{
		pSectorList = &list_Sector[AddSector.m_Sector[iCnt].y][AddSector.m_Sector[iCnt].x];

		itor = pSectorList->begin();
		enditor = pSectorList->end();

		// ���͸��� ��ϵ� ĳ���͵��� �̾Ƽ�, ������Ŷ ���� �� ������ ����
		for (; itor != enditor; ++itor)
		{
			// ���� �ƴҶ��� ����.
			if ((*itor) == NowPlayer)
				continue;


			// ������ ���� ������Ŷ ���� �� ������ ����
			CProtocolBuff Create_header(dfNETWORK_PACKET_HEADER_SIZE);
			CProtocolBuff Create_payload;
			BYTE Dir = dfPACKET_MOVE_DIR_RR;
			if ((*itor)->m_byDir == dfPACKET_MOVE_DIR_LL ||
				(*itor)->m_byDir == dfPACKET_MOVE_DIR_LU ||
				(*itor)->m_byDir == dfPACKET_MOVE_DIR_UU ||
				(*itor)->m_byDir == dfPACKET_MOVE_DIR_LD)
			{
				Dir = dfPACKET_MOVE_DIR_LL;
			}

			Network_Send_CreateOtherCharacter(&Create_header, &Create_payload, (*itor)->m_dwPlayerID,
				Dir, (*itor)->m_wNowX, (*itor)->m_wNowY, (*itor)->m_byHP);

			SendPacket(NowPlayer->m_Account, &Create_header, &Create_payload);

			
			// ���� ��Ŷ ���� ������, �̵����̶�� �̵���Ŷ ���� ������.
			if ((*itor)->m_byNowAction == PLAYER_ACTION_MOVE)
			{
				CProtocolBuff Move_header(dfNETWORK_PACKET_HEADER_SIZE);
				CProtocolBuff payload;

				Network_Send_MoveStart(&Move_header, &payload, (*itor)->m_dwPlayerID, (*itor)->m_byDir, (*itor)->m_wNowX, (*itor)->m_wNowY);

				SendPacket(NowPlayer->m_Account, &Move_header, &payload);
			}

		}
	}

	END("CharacterSectorUpdate");
}

// �� ����, 9������ Index, �ο� ���� ��� �Լ�. ������
void Print_Sector(stPlayer* NowPlayer)
{
	// 1. ���� ��ġ�� ������ Index �˾ƿ���
	WORD SectorX = NowPlayer->m_SectorX;
	WORD SectorY = NowPlayer->m_SectorY;

	// 2. �� �ε��� ����, 9���� Index�� ���� �� ���
	// �� 1��
	printf("[%d,%d : %zd]   [%d,%d : %zd]   [%d,%d : %zd]  \n",
		SectorY - 1, SectorX - 1, list_Sector[SectorY - 1][SectorX - 1].size(),
		SectorY - 1, SectorX - 0, list_Sector[SectorY - 1][SectorX - 0].size(),
		SectorY - 1, SectorX + 1, list_Sector[SectorY - 1][SectorX + 1].size()
	);

	// �߰� 1��
	printf("[%d,%d : %zd]   [%d,%d : %zd]   [%d,%d : %zd]  \n",
		SectorY - 0, SectorX - 1, list_Sector[SectorY - 0][SectorX - 1].size(),
		SectorY - 0, SectorX - 0, list_Sector[SectorY - 0][SectorX - 0].size(),
		SectorY - 0, SectorX + 1, list_Sector[SectorY - 0][SectorX + 1].size()
	);

	// �Ʒ� 1��
	printf("[%d,%d : %zd]   [%d,%d : %zd]   [%d,%d : %zd]  \n\n",
		SectorY + 1, SectorX - 1, list_Sector[SectorY + 1][SectorX - 1].size(),
		SectorY + 1, SectorX - 0, list_Sector[SectorY + 1][SectorX - 0].size(),
		SectorY + 1, SectorX + 1, list_Sector[SectorY + 1][SectorX + 1].size()
	);

}







// ---------------------------------
// Update()���� ���� �׼� ó�� �Լ�
// ---------------------------------
// �� �÷��̾� �׼� üũ
void ActionProc()
{
	BEGIN("ActionProc");

	// �������� ��� �÷��̾ üũ�ϸ� �ൿ�� �Ѵ�.
	map <SOCKET, stAccount*>::iterator	itor;
	map <SOCKET, stAccount*>::iterator	enditor;

	enditor = map_Account.end();

	// 1. ���� ������ �׼ǿ� ���� ó��
	for (itor = map_Account.begin(); itor != enditor; ++itor)
	{
		// ���� �ٸ� ó���� �� ���� ������ switch case ������ ó��
		switch (itor->second->m_LoginPlayer->m_byNowAction)
		{
			// �̵� �׼�
		case PLAYER_ACTION_MOVE:
			// �̵� ���϶��� 1�����Ӹ��� ������ ������ ������ ��ġ��ŭ �̵�
			Action_Move(itor->second->m_LoginPlayer);

			// �̵� �� ��ǥ�� ���� ���� �̵����� üũ
			SectorChange(itor->second);
			
			break;

		default:
			break;
		}

		// ó�� ��, ���� �׼��� ���� �׼����� �����Ѵ�. �׷���, ���� ó�� �� üũ ����
		itor->second->m_LoginPlayer->m_byOldAction = itor->second->m_LoginPlayer->m_byNowAction;
	}

	END("ActionProc");
}

// ���� �̵� �׼� ó��
void Action_Move(stPlayer* NowPlayer)
{
	BEGIN("Action_Move");

	// 1. ���⿡ ���� X,Y�� �̵�
	switch (NowPlayer->m_byDir)
	{
		// LL
	case dfPACKET_MOVE_DIR_LL:
		NowPlayer->m_wNowX -= PLAYER_XMOVE_PIXEL;

		// �����̻� ������ ���ϰ� ����
		if (NowPlayer->m_wNowX < dfRANGE_MOVE_LEFT)
			NowPlayer->m_wNowX = dfRANGE_MOVE_LEFT;
		break;

		// LU
	case dfPACKET_MOVE_DIR_LU:
		// ���� Y��ǥ�� dfRANGE_MOVE_TOP(���� �� ��)�̶��, x��ǥ �������� ����
		if (NowPlayer->m_wNowY != dfRANGE_MOVE_TOP)
		{
			NowPlayer->m_wNowX -= PLAYER_XMOVE_PIXEL;
			if (NowPlayer->m_wNowX < dfRANGE_MOVE_LEFT)
				NowPlayer->m_wNowX = dfRANGE_MOVE_LEFT;
		}

		// ����, X��ǥ�� dfRANGE_MOVE_LEFT(���� ���� ��)�̶��, Y��ǥ�� �������� ����. 
		if (NowPlayer->m_wNowX != dfRANGE_MOVE_LEFT)
		{
			NowPlayer->m_wNowY -= PLAYER_YMOVE_PIXEL;
			if (NowPlayer->m_wNowY < dfRANGE_MOVE_TOP)
				NowPlayer->m_wNowY = dfRANGE_MOVE_TOP;
		}

		break;

		// UU
	case dfPACKET_MOVE_DIR_UU:
		NowPlayer->m_wNowY -= PLAYER_YMOVE_PIXEL;
		if (NowPlayer->m_wNowY < dfRANGE_MOVE_TOP)
			NowPlayer->m_wNowY = dfRANGE_MOVE_TOP;
		break;

		// RU
	case dfPACKET_MOVE_DIR_RU:
		// ���� Y��ǥ�� dfRANGE_MOVE_TOP(���� �� ��)�̶��, x��ǥ �������� ����
		if (NowPlayer->m_wNowY != dfRANGE_MOVE_TOP)
		{
			NowPlayer->m_wNowX += PLAYER_XMOVE_PIXEL;
			if (NowPlayer->m_wNowX > dfRANGE_MOVE_RIGHT)
				NowPlayer->m_wNowX = dfRANGE_MOVE_RIGHT;
		}

		// ����, X��ǥ�� dfRANGE_MOVE_RIGHT(���� ������ ��)�̶��, Y��ǥ�� �������� ����. 
		if (NowPlayer->m_wNowX != dfRANGE_MOVE_RIGHT)
		{
			NowPlayer->m_wNowY -= PLAYER_YMOVE_PIXEL;
			if (NowPlayer->m_wNowY < dfRANGE_MOVE_TOP)
				NowPlayer->m_wNowY = dfRANGE_MOVE_TOP;
		}
		break;

		// RR
	case dfPACKET_MOVE_DIR_RR:
		NowPlayer->m_wNowX += PLAYER_XMOVE_PIXEL;
		if (NowPlayer->m_wNowX > dfRANGE_MOVE_RIGHT)
			NowPlayer->m_wNowX = dfRANGE_MOVE_RIGHT;
		break;

		// RD
	case dfPACKET_MOVE_DIR_RD:
		// ���� Y��ǥ�� dfRANGE_MOVE_BOTTOM(���� �ٴ� ��)�̶��, X��ǥ �������� ����
		if (NowPlayer->m_wNowY != dfRANGE_MOVE_BOTTOM)
		{
			NowPlayer->m_wNowX += PLAYER_XMOVE_PIXEL;
			if (NowPlayer->m_wNowX > dfRANGE_MOVE_RIGHT)
				NowPlayer->m_wNowX = dfRANGE_MOVE_RIGHT;
		}

		// ����, X��ǥ�� dfRANGE_MOVE_RIGHT(���� ������ ��)�̶��, Y��ǥ�� �������� ����. 
		if (NowPlayer->m_wNowX != dfRANGE_MOVE_RIGHT)
		{
			NowPlayer->m_wNowY += PLAYER_YMOVE_PIXEL;
			if (NowPlayer->m_wNowY > dfRANGE_MOVE_BOTTOM)
				NowPlayer->m_wNowY = dfRANGE_MOVE_BOTTOM;
		}
		break;

		// DD
	case dfPACKET_MOVE_DIR_DD:
		NowPlayer->m_wNowY += PLAYER_YMOVE_PIXEL;
		if (NowPlayer->m_wNowY > dfRANGE_MOVE_BOTTOM)
			NowPlayer->m_wNowY = dfRANGE_MOVE_BOTTOM;
		break;

		// LD
	case dfPACKET_MOVE_DIR_LD:
		// ���� Y��ǥ�� dfRANGE_MOVE_BOTTOM(���� �ٴ� ��)�̶��, X��ǥ �������� ����
		if (NowPlayer->m_wNowY != dfRANGE_MOVE_BOTTOM)
		{
			NowPlayer->m_wNowX -= PLAYER_XMOVE_PIXEL;
			if (NowPlayer->m_wNowX < dfRANGE_MOVE_LEFT)
				NowPlayer->m_wNowX = dfRANGE_MOVE_LEFT;
		}

		// ����, X��ǥ�� dfRANGE_MOVE_LEFT(���� ���� ��)�̶��, Y��ǥ�� �������� ����.
		if (NowPlayer->m_wNowX != dfRANGE_MOVE_LEFT)
		{
			NowPlayer->m_wNowY += PLAYER_YMOVE_PIXEL;
			if (NowPlayer->m_wNowY > dfRANGE_MOVE_BOTTOM)
				NowPlayer->m_wNowY = dfRANGE_MOVE_BOTTOM;
		}
		break;

	default:
		printf("Action_Move() �˼� ���� ����.(PlayerID: %d)\n", NowPlayer->m_dwPlayerID);
		break;
	}

	END("Action_Move");

	//printf("X : %d / Y : %d\n", NowPlayer->m_wNowX, NowPlayer->m_wNowY);
}








// ---------------------------------
// �˻��� �Լ�
// --------------------------------
// ���ڷ� ���� Socket ���� �������� [ȸ�� ���]���� [������ ��󳽴�].(�˻�)
// ���� ��, �ش� ������ ���� ����ü�� �ּҸ� ����
// ���� �� nullptr ����
stAccount* ClientSearch_AcceptList(SOCKET sock)
{
	BEGIN("ClientSearch_AcceptList");

	stAccount* NowAccount = nullptr;
	map <SOCKET, stAccount*>::iterator iter;

	iter = map_Account.find(sock);
	if (iter == map_Account.end())
		return nullptr;

	NowAccount = iter->second;

	END("ClientSearch_AcceptList");
	return NowAccount;
}








// ---------------------------------
// ��Ŷ ���� �Լ�
// ---------------------------------
// ��� ���� �Լ�
void CreateHeader(CProtocolBuff* header, BYTE PayloadSize, BYTE PacketType)
{
	// ��Ŷ �ڵ� (1Byte)
	*header << dfNETWORK_PACKET_CODE;

	// ���̷ε� ������ (1Byte)
	*header << PayloadSize;

	// ��Ŷ Ÿ�� (1Byte)
	*header << PacketType;

	// ������ (1Byte)
	BYTE temp = 12;
	*header << temp;
}

// "�� ĳ���� ����" ��Ŷ ����
void Network_Send_CreateMyCharacter(CProtocolBuff* header, CProtocolBuff* payload, DWORD ID, BYTE Dir, WORD X, WORD Y, BYTE HP)
{
	// 1. ���̷ε� ����
	*payload << ID;
	*payload << Dir;
	*payload << X;
	*payload << Y;
	*payload << HP;

	// 2. ��� ����
	CreateHeader(header, payload->GetUseSize(), dfPACKET_SC_CREATE_MY_CHARACTER);
}

// "�ٸ� ��� ĳ���� ����" ��Ŷ ����
void Network_Send_CreateOtherCharacter(CProtocolBuff* header, CProtocolBuff* payload, DWORD ID, BYTE Dir, WORD X, WORD Y, BYTE HP)
{
	// 1. ���̷ε� ����
	*payload << ID;
	*payload << Dir;
	*payload << X;
	*payload << Y;
	*payload << HP;

	// 2. ��� ����
	CreateHeader(header, payload->GetUseSize(), dfPACKET_SC_CREATE_OTHER_CHARACTER);
}

// "�ٸ� ��� ����" ��Ŷ ����
void Network_Send_RemoveOtherCharacteor(CProtocolBuff* header, CProtocolBuff* payload, DWORD ID)
{
	// 1. ���̷ε� ����
	*payload << ID;

	// 2. ��� ����
	CreateHeader(header, payload->GetUseSize(), dfPACKET_SC_DELETE_CHARACTER);
}

// "�ٸ� ��� �̵� ����" ��Ŷ ����
void Network_Send_MoveStart(CProtocolBuff* header, CProtocolBuff* payload, DWORD ID, BYTE Dir, WORD X, WORD Y)
{
	// 1. ���̷ε� ����
	*payload << ID;
	*payload << Dir;
	*payload << X;
	*payload << Y;

	// 2. ��� ����
	CreateHeader(header, payload->GetUseSize(), dfPACKET_SC_MOVE_START);
}

// "�ٸ� ��� ����" ��Ŷ ����
void Network_Send_MoveStop(CProtocolBuff* header, CProtocolBuff* payload, DWORD ID, BYTE Dir, WORD X, WORD Y)
{
	// 1. ���̷ε� ����
	*payload << ID;
	*payload << Dir;
	*payload << X;
	*payload << Y;

	// 2. ��� ����
	CreateHeader(header, payload->GetUseSize(), dfPACKET_SC_MOVE_STOP);
}

// "��ũ ���߱�" ��Ŷ ����
void Network_Send_Sync(CProtocolBuff* header, CProtocolBuff* payload, DWORD ID, WORD X, WORD Y)
{
	// 1. ���̷ε� ����
	*payload << ID;
	*payload << X;
	*payload << Y;

	// 2. ��� ����
	CreateHeader(header, payload->GetUseSize(), dfPACKET_SC_SYNC);
}

// "1�� ���� ����" ��Ŷ �����
void Network_Send_Attack_1(CProtocolBuff* header, CProtocolBuff* payload, DWORD ID, BYTE Dir, WORD X, WORD Y)
{
	// 1. ���̷ε� ����
	*payload << ID;
	*payload << Dir;
	*payload << X;
	*payload << Y;

	// 2. ��� ����
	CreateHeader(header, payload->GetUseSize(), dfPACKET_SC_ATTACK1);
}

// "2�� ���� ����" ��Ŷ �����
void Network_Send_Attack_2(CProtocolBuff* header, CProtocolBuff* payload, DWORD ID, BYTE Dir, WORD X, WORD Y)
{
	// 1. ���̷ε� ����
	*payload << ID;
	*payload << Dir;
	*payload << X;
	*payload << Y;

	// 2. ��� ����
	CreateHeader(header, payload->GetUseSize(), dfPACKET_SC_ATTACK2);

}

// "3�� ���� ����" ��Ŷ �����
void Network_Send_Attack_3(CProtocolBuff* header, CProtocolBuff* payload, DWORD ID, BYTE Dir, WORD X, WORD Y)
{
	// 1. ���̷ε� ����
	*payload << ID;
	*payload << Dir;
	*payload << X;
	*payload << Y;

	// 2. ��� ����
	CreateHeader(header, payload->GetUseSize(), dfPACKET_SC_ATTACK3);

}

// "������" ��Ŷ �����
void Network_Send_Damage(CProtocolBuff *header, CProtocolBuff* payload, DWORD AttackID, DWORD DamageID, BYTE HP)
{
	// 1. ���̷ε� ����
	*payload << AttackID;
	*payload << DamageID;
	*payload << HP;

	// 2. ��� ����
	CreateHeader(header, payload->GetUseSize(), dfPACKET_SC_DAMAGE);
}







// ---------------------------------
// Recv ó�� �Լ���
// ---------------------------------
// Recv() ó��
bool RecvProc(stAccount* Account)
{
	if (Account == nullptr)
		return false;

	////////////////////////////////////////////////////////////////////////
	// ���� ���ۿ� �ִ� ��� recv �����͸�, �ش� ������ recv�����۷� ���ú�.
	////////////////////////////////////////////////////////////////////////
	
	BEGIN("RecvProc");

	// 1. recv ������ ������ ������.
	char* recvbuff = Account->m_RecvBuff.GetBufferPtr();

	// 2. �� ���� �� �� �ִ� �������� ���� ���� ���
	int Size = Account->m_RecvBuff.GetNotBrokenPutSize();

	// 3. �� ���� �� �� �ִ� ������ 0�̶��
	if (Size == 0)
	{
		// rear 1ĭ �̵�
		Account->m_RecvBuff.MoveWritePos(1);

		// �׸��� �� ���� �� �� �ִ� ��ġ �ٽ� �˱�.
		Size = Account->m_RecvBuff.GetNotBrokenPutSize();
	}

	// 4. �װ� �ƴ϶��, 1ĭ �̵��� �ϱ�. 		
	else
	{
		// rear 1ĭ �̵�
		Account->m_RecvBuff.MoveWritePos(1);		
	}

	// 5. 1ĭ �̵��� rear ������
	int* rear = (int*)Account->m_RecvBuff.GetWriteBufferPtr();	

	// 6. recv()
	int retval = recv(Account->m_sock, &recvbuff[*rear], Size, 0);	

	// 7. ���ú� ����üũ
	if (retval == SOCKET_ERROR)
	{		
		// WSAEWOULDBLOCK������ �߻��ϸ�, ���� ���۰� ����ִٴ� ��. recv�Ұ� ���ٴ� ���̴� �Լ� ����.
		if (WSAGetLastError() == WSAEWOULDBLOCK)
			return true;

		// 10053, 10054 �Ѵ� ��� ������ ������ ����
		// WSAECONNABORTED(10053) :  �������ݻ��� ������ Ÿ�Ӿƿ��� ���� ����(����ȸ��. virtual circle)�� ������ ���
		// WSAECONNRESET(10054) : ���� ȣ��Ʈ�� ������ ����.
		// �� ���� �׳� return false�� ���� ����� ������ ���´�.
		if (WSAGetLastError() == WSAECONNRESET || WSAGetLastError() == WSAECONNABORTED)
			return false;

		// WSAEWOULDBLOCK������ �ƴϸ� ���� �̻��� ���̴� ���� ����
		else
		{
			SYSTEMTIME lst;
			GetLocalTime(&lst);
			_LOG(dfLOG_LEVEL_ERROR, L"[%02d/%02d/%02d %02d:%02d:%02d] [ERROR]RecvProc(). retval �� ������ �ƴ� ������ ��(%d). ���� ����\n",
			lst.wYear - 2000, lst.wMonth, lst.wDay, lst.wHour, lst.wMinute, lst.wSecond, WSAGetLastError());
			return false;	// FALSE�� ���ϵǸ�, ������ �����.
		}
	}

	// 0�� ���ϵǴ� ���� ��������.
	else if (retval == 0)
		return false;

	// 8. ������� ������ Rear�� �̵���Ų��.
	Account->m_RecvBuff.MoveWritePos(retval - 1);


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
		if (Account->m_RecvBuff.GetUseSize() < len)
			break;

		// 2. ����� Peek���� Ȯ���Ѵ�.		
		// Peek �ȿ�����, ��� �ؼ����� len��ŭ �д´�. ���۰� �� ���� ���� �̻�!
		int PeekSize = Account->m_RecvBuff.Peek((char*)&HeaderBuff, len);
		if (PeekSize == -1)
		{
			SYSTEMTIME lst;
			GetLocalTime(&lst);
			_LOG(dfLOG_LEVEL_ERROR, L"[%02d/%02d/%02d %02d:%02d:%02d] [ERROR]RecvProc(). �Ϸ���Ŷ ��� ó�� �� RecvBuff�����. ���� ����\n",
				lst.wYear - 2000, lst.wMonth, lst.wDay, lst.wHour, lst.wMinute, lst.wSecond);

			return false;	// FALSE�� ���ϵǸ�, ������ �����.
		}

		// 3. ����� Len���� RecvBuff�� ������ ������ ��. (�ϼ� ��Ŷ ������ = ��� ������ + ���̷ε� Size + endCode(1����Ʈ))
		// ��� ���, �ϼ� ��Ŷ ����� �ȵǸ� while�� ����.
		if (Account->m_RecvBuff.GetUseSize() < (len + HeaderBuff.bySize + 1))
			break;

		// 4. ����� code Ȯ��(���������� �� �����Ͱ� �´°�)
		if (HeaderBuff.byCode != dfNETWORK_PACKET_CODE)
		{
			SYSTEMTIME lst;
			GetLocalTime(&lst);
			_LOG(dfLOG_LEVEL_ERROR, L"[%02d/%02d/%02d %02d:%02d:%02d] [ERROR]RecvProc(). �Ϸ���Ŷ ��� ó�� �� PacketCodeƲ��. ���� ����\n",
				lst.wYear - 2000, lst.wMonth, lst.wDay, lst.wHour, lst.wMinute, lst.wSecond);

			_tprintf(_T("RecvProc(). �Ϸ���Ŷ ��� ó�� �� PacketCodeƲ��. ���� ����\n"));
			return false;	// FALSE�� ���ϵǸ�, ������ �����.
		}		

		// 5. RecvBuff���� Peek�ߴ� ����� ����� (�̹� Peek������, �׳� Remove�Ѵ�)
		Account->m_RecvBuff.RemoveData(len);

		// 6. RecvBuff���� ���̷ε� Size ��ŭ ���̷ε� �ӽ� ���۷� �̴´�. (��ť�̴�. Peek �ƴ�)
		CProtocolBuff PayloadBuff;
		DWORD PayloadSize = HeaderBuff.bySize;

		int DequeueSize = Account->m_RecvBuff.Dequeue(PayloadBuff.GetBufferPtr(), PayloadSize);
		if (DequeueSize == -1)
		{
			SYSTEMTIME lst;
			GetLocalTime(&lst);
			_LOG(dfLOG_LEVEL_ERROR, L"[%02d/%02d/%02d %02d:%02d:%02d] [ERROR]RecvProc(). �Ϸ���Ŷ ���̷ε� ó�� �� RecvBuff�����. ���� ����\n",
				lst.wYear - 2000, lst.wMonth, lst.wDay, lst.wHour, lst.wMinute, lst.wSecond);

			return false;	// FALSE�� ���ϵǸ�, ������ �����.
		}
		PayloadBuff.MoveWritePos(DequeueSize);
		

		// 7. RecvBuff���� �����ڵ� 1Byte����.	(��ť�̴�. Peek �ƴ�)
		BYTE EndCode;
		DequeueSize = Account->m_RecvBuff.Dequeue((char*)&EndCode, 1);
		if (DequeueSize == -1)
		{
			SYSTEMTIME lst;
			GetLocalTime(&lst);
			_LOG(dfLOG_LEVEL_ERROR, L"[%02d/%02d/%02d %02d:%02d:%02d] [ERROR]RecvProc(). �Ϸ���Ŷ �����ڵ� ó�� �� RecvBuff�����. ���� ����\n",
				lst.wYear - 2000, lst.wMonth, lst.wDay, lst.wHour, lst.wMinute, lst.wSecond);

			return FALSE;	// FALSE�� ���ϵǸ�, �ش� ������ ������ �����.
		}			

		// 8. �����ڵ� Ȯ��
		if (EndCode != dfNETWORK_PACKET_END)
		{			
			SYSTEMTIME lst;
			GetLocalTime(&lst);
			_LOG(dfLOG_LEVEL_ERROR, L"[%02d/%02d/%02d %02d:%02d:%02d] [ERROR]RecvProc(). �Ϸ���Ŷ �����ڵ� ó�� �� ������Ŷ �ƴ�. ���� ����\n",
				lst.wYear - 2000, lst.wMonth, lst.wDay, lst.wHour, lst.wMinute, lst.wSecond);

			return FALSE;	// FALSE�� ���ϵǸ�, �ش� ������ ������ �����.
		}		

		// 7. ����� ����ִ� Ÿ�Կ� ���� �б�ó��.
		bool check = PacketProc(HeaderBuff.byType, Account, &PayloadBuff);
		if (check == false)
			return false;	
	}

	END("RecvProc");
	return true;
}

// ��Ŷ ó��
// PacketProc() �Լ����� false�� ���ϵǸ� �ش� ������ ������ �����.
bool PacketProc(WORD PacketType, stAccount* Account, CProtocolBuff* Payload)
{
	/*_LOG(dfLOG_LEVEL_DEBUG, L"[DEBUG] PacketRecv [PlayerID : %d / PacketType : %d]\n",
		Account->m_LoginPlayer->m_dwPlayerID, PacketType);*/
	
	bool check = true;

	try
	{
		switch (PacketType)
		{

			// �̵����� ��Ŷ
		case dfPACKET_CS_MOVE_START:
			{
				check = Network_Req_MoveStart(Account, Payload);
				if (check == false)
					return false;	// �ش� ���� ���� ����
			}
			break;	

			// ����
		case dfPACKET_CS_MOVE_STOP:
		{
			check = Network_Req_MoveStop(Account, Payload);
			if (check == false)
				return false;	// �ش� ���� ���� ����
		}
		break;

		// ����1
		case dfPACKET_CS_ATTACK1:
		{
			check = Network_Req_Attack_1(Account, Payload);
			if (check == false)
				return false;	// �ش� ���� ���� ����
		}
		break;

		// ����2
		case dfPACKET_CS_ATTACK2:
		{
			check = Network_Req_Attack_2(Account, Payload);
			if (check == false)
				return false;	// �ش� ���� ���� ����
		}
		break;

		// ����3
		case dfPACKET_CS_ATTACK3:
		{
			check = Network_Req_Attack_3(Account, Payload);
			if (check == false)
				return false;	// �ش� ���� ���� ����
		}
		break;

		// ����
		case dfPACKET_CS_ECHO:
		{
			check = Network_Req_StressTest(Account, Payload);
			if(check == false)
				return false;	// �ش� ���� ���� ����
		}
		break;
		

		default:
			_tprintf(_T("�̻��� ��Ŷ�Դϴ�.\n"));
			//return false;
			break;
		}


	}
	catch (CException exc)
	{
		TCHAR* text = (TCHAR*)exc.GetExceptionText();
		_tprintf(_T("%s.\n"), text);
		return false;

	}

	return true;
}

// "ĳ���� �̵� ����" ��Ŷ ó��
bool Network_Req_MoveStart(stAccount* Account, CProtocolBuff* payload)
{
	// ----------------------------------
	// 1. ������ ���� Dir, X, Y ������
	// ----------------------------------
	BYTE Dir;
	WORD X;
	WORD Y;

	*payload >> Dir;
	*payload >> X;
	*payload >> Y;

	
	// ----------------------------------
	// 2, ���巹Ŀ�� ���� üũ
	// ������ ���� x,y�� ������ x,y �ʹ� ū ���̰� ����, ���巹Ŀ�� üũ	
	// ----------------------------------
	if (abs(Account->m_LoginPlayer->m_wNowX - X) > dfERROR_RANGE ||
		abs(Account->m_LoginPlayer->m_wNowY - Y) > dfERROR_RANGE)
	{
		int resultX, resultY;

		if (Dir == dfPACKET_MOVE_DIR_RU)
			int abc = 10;

		// ���巹Ŀ�� �ϱ�
		DeadReckoning(Account->m_LoginPlayer->m_byDir, Account->m_LoginPlayer->m_dwACtionTick,
			Account->m_LoginPlayer->m_wActionX, Account->m_LoginPlayer->m_wActionY, &resultX, &resultY);

		// ������ ��ǥ�� ��Ȯ��. �׷��� ���̰� ���̳��ٸ�, ��ũ��Ŷ ����
		if (abs(resultX - X) > dfERROR_RANGE ||
			abs(resultY - Y) > dfERROR_RANGE)
		{
			// Sync ��Ŷ �����
			CProtocolBuff header(dfNETWORK_PACKET_HEADER_SIZE);
			CProtocolBuff payload;
			Network_Send_Sync(&header, &payload, Account->m_LoginPlayer->m_dwPlayerID, resultX, resultY);
			
			// ������ ������
			if (SendPacket(Account, &header, &payload) == FALSE)
				return FALSE;

			SYSTEMTIME lst;
			GetLocalTime(&lst);

			_LOG(dfLOG_LEVEL_ERROR, L"[%02d/%02d/%02d %02d:%02d:%02d] [ERROR] MoverStart Sync! [%d, %d] -> [%d, %d]\n",
				lst.wYear - 2000, lst.wMonth, lst.wDay, lst.wHour, lst.wMinute, lst.wSecond, X, Y, resultX, resultY);
		}

		// ���巹Ŀ�� ����� ������ X,Y ����
		Account->m_LoginPlayer->m_wNowX = resultX;
		Account->m_LoginPlayer->m_wNowY = resultY;

	}

	// ���巹Ŀ���� ���ߴٸ�, ������ X/Y�� ��ǥ ����. ��ũ�� ���� �ʿ� ����.
	else
	{
		Account->m_LoginPlayer->m_wNowX = X;
		Account->m_LoginPlayer->m_wNowY = Y;
	}

	// ----------------------------------
	// 3. ��ǥ �� ���� ����
	// ----------------------------------
	Account->m_LoginPlayer->m_byDir = Dir;
	Account->m_LoginPlayer->m_byNowAction = PLAYER_ACTION_MOVE;

	// ���巹Ŀ�׿� ���� ����
	Account->m_LoginPlayer->m_wActionX = X;
	Account->m_LoginPlayer->m_wActionY = Y;
	Account->m_LoginPlayer->m_dwACtionTick = GetTickCount64();
		
	
	// 3.  ������ X,Y�� ������, �ش� ������ �ֺ� ���� 9�� ������ SendBuff�� "�ٸ� ĳ���� �̵� ����" ��Ŷ �ֱ�. (�ڱ� �ڽ����״� �Ⱥ���)
	CProtocolBuff header(dfNETWORK_PACKET_HEADER_SIZE);
	CProtocolBuff payload_2;
	Network_Send_MoveStart(&header, &payload_2, Account->m_LoginPlayer->m_dwPlayerID, Dir, X, Y);

	// ���� ��ǥ ����, ���� Index ���ϱ�
	int SectorX = X / SECTOR_WIDTH;
	int SectorY = Y / SECTOR_HEIGHT; 

	stSectorCheck secSector;
	GetSector(SectorX, SectorY, &secSector);

	SendPacket_Sector(Account->m_LoginPlayer, &header, &payload_2, &secSector);
	

	return true;
}

// "ĳ���� �̵� ����" ��Ŷ ó��
bool Network_Req_MoveStop(stAccount* Account, CProtocolBuff* payload)
{
	// ----------------------------------
	// 1. �ش� ������ Dir, X, Y�� ������
	// ----------------------------------
	BYTE Dir;
	WORD X;
	WORD Y;

	*payload >> Dir;
	*payload >> X;
	*payload >> Y;


	// ----------------------------------
	// 2, ���巹Ŀ�� ���� üũ
	// ������ ���� x,y�� ������ x,y �ʹ� ū ���̰� ����, ���巹Ŀ�� üũ	
	// ----------------------------------
	if (abs(Account->m_LoginPlayer->m_wNowX - X) > dfERROR_RANGE ||
		abs(Account->m_LoginPlayer->m_wNowY - Y) > dfERROR_RANGE)
	{
		int resultX, resultY;

		// ���巹Ŀ�� �ϱ�
		DeadReckoning(Account->m_LoginPlayer->m_byDir, Account->m_LoginPlayer->m_dwACtionTick,
			Account->m_LoginPlayer->m_wActionX, Account->m_LoginPlayer->m_wActionY, &resultX, &resultY);

		// ������ ��ǥ�� ��Ȯ��. �׷��� ���̰� ���̳��ٸ�, ��ũ��Ŷ ����
		if (abs(resultX - X) > dfERROR_RANGE ||
			abs(resultY - Y) > dfERROR_RANGE)
		{
			// Sync ��Ŷ �����
			CProtocolBuff header(dfNETWORK_PACKET_HEADER_SIZE);
			CProtocolBuff payload;
			Network_Send_Sync(&header, &payload, Account->m_LoginPlayer->m_dwPlayerID, resultX, resultY);
			
			// ������ ������
			if (SendPacket(Account, &header, &payload) == FALSE)
				return FALSE;			

			SYSTEMTIME lst;
			GetLocalTime(&lst);

			_LOG(dfLOG_LEVEL_ERROR, L"[%02d/%02d/%02d %02d:%02d:%02d] [ERROR] MoverStop Sync! [%d, %d] -> [%d, %d]\n",
				lst.wYear - 2000, lst.wMonth, lst.wDay, lst.wHour, lst.wMinute, lst.wSecond, X, Y, resultX, resultY);

		}

		// ���巹Ŀ�� ����� ������ X,Y ����
		Account->m_LoginPlayer->m_wNowX = resultX;
		Account->m_LoginPlayer->m_wNowY = resultY;
		
	}

	// ���巹Ŀ���� ���ߴٸ�, ������ X/Y�� ��ǥ ����. ��ũ�� ���� �ʿ� ����.
	else
	{
		Account->m_LoginPlayer->m_wNowX = X;
		Account->m_LoginPlayer->m_wNowY = Y;
	}


	// ----------------------------------
	// 3. ���� ���� ��, ���� ���¸� ������ ����
	// ----------------------------------
	Account->m_LoginPlayer->m_byDir = Dir;
	Account->m_LoginPlayer->m_byNowAction = PLAYER_ACTION_IDLE;



	// ----------------------------------
	// 4. �ش� ���� �ֺ� 9�� ���Ϳ� �ִ� �����鿡�� ������Ŷ ������. (�� �ڽ� ����)
	// ----------------------------------
	CProtocolBuff header(dfNETWORK_PACKET_HEADER_SIZE);
	CProtocolBuff payload_2;
	Network_Send_MoveStop(&header, &payload_2, Account->m_LoginPlayer->m_dwPlayerID, Dir, X, Y);
	
	// ���� ��ǥ ����, ���� Index ���ϱ�
	int SectorX = Account->m_LoginPlayer->m_wNowX / SECTOR_WIDTH;
	int SectorY = Account->m_LoginPlayer->m_wNowY / SECTOR_HEIGHT;

	// �ֺ� 9���� ���ϱ�
	stSectorCheck secSector;
	GetSector(SectorX, SectorY, &secSector);

	// ������
	SendPacket_Sector(Account->m_LoginPlayer, &header, &payload_2, &secSector);
	
	return true;
}

// "���� 1��" ��Ŷ ó��
bool Network_Req_Attack_1(stAccount* Account, CProtocolBuff* payload)
{
	// ---------------------------------
	// ���� ����, X,Y ������. ������ �¿츸 ����
	// ---------------------------------
	BYTE Dir;
	WORD X, Y;

	*payload >> Dir;
	*payload >> X;
	*payload >> Y;

	// �ϴ� �����ڸ� Idle���·� �����.
	Account->m_LoginPlayer->m_byNowAction = PLAYER_ACTION_IDLE;

	// ---------------------------------
	// ������ ����, 9���⿡ ���Ϳ� "1�� ���� ����" ��Ŷ�� ������.
	// ---------------------------------	
	// ���� ���� ��Ŷ�� �����.
	CProtocolBuff Attack_Header(dfNETWORK_PACKET_HEADER_SIZE);
	CProtocolBuff Attack_Payload;
	Network_Send_Attack_1(&Attack_Header, &Attack_Payload, Account->m_LoginPlayer->m_dwPlayerID, Dir, X, Y);

	// ������ ���� 9���� ���Ϳ� ���ݽ��� ��Ŷ�� ������. (������ �ڽ� ����)
	stSectorCheck secSector;
	GetSector(Account->m_LoginPlayer->m_SectorX, Account->m_LoginPlayer->m_SectorY, &secSector);

	if (SendPacket_Sector(Account->m_LoginPlayer, &Attack_Header, &Attack_Payload, &secSector) == FALSE)
		return FALSE;
	
	// ---------------------------------
	// ������ ��Ŷ ó��
	// ---------------------------------
	if (Network_Requ_Attck_1_Damage(Account->m_LoginPlayer->m_dwPlayerID, Account->m_LoginPlayer->m_wNowX, Account->m_LoginPlayer->m_wNowY, Dir) == FALSE)
		return FALSE;


	return TRUE;
}

// "���� 2��" ��Ŷ ó��
bool Network_Req_Attack_2(stAccount* Account, CProtocolBuff* payload)
{
	// ---------------------------------
	// ���� ����, X,Y ������. ������ �¿츸 ����
	// ---------------------------------
	BYTE Dir;
	WORD X, Y;

	*payload >> Dir;
	*payload >> X;
	*payload >> Y;

	// �ϴ� �����ڸ� Idle���·� �����.
	Account->m_LoginPlayer->m_byNowAction = PLAYER_ACTION_IDLE;


	// ---------------------------------
	// ������ ����, 9���⿡ ���Ϳ� "2�� ���� ����" ��Ŷ�� ������.
	// ---------------------------------	
	// ���� ���� ��Ŷ�� �����.
	CProtocolBuff Attack_Header(dfNETWORK_PACKET_HEADER_SIZE);
	CProtocolBuff Attack_Payload;
	Network_Send_Attack_2(&Attack_Header, &Attack_Payload, Account->m_LoginPlayer->m_dwPlayerID, Dir, X, Y);

	// ������ ���� 9���� ���Ϳ� ���ݽ��� ��Ŷ�� ������. (������ �ڽ� ����)
	stSectorCheck secSector;
	GetSector(Account->m_LoginPlayer->m_SectorX, Account->m_LoginPlayer->m_SectorY, &secSector);

	if (SendPacket_Sector(Account->m_LoginPlayer, &Attack_Header, &Attack_Payload, &secSector) == FALSE)
		return FALSE;

	// ---------------------------------
	// ������ ��Ŷ ó��
	// ---------------------------------
	if (Network_Requ_Attck_2_Damage(Account->m_LoginPlayer->m_dwPlayerID, Account->m_LoginPlayer->m_wNowX, Account->m_LoginPlayer->m_wNowY, Dir) == FALSE)
		return FALSE;

	return TRUE;
}

// "���� 3��" ��Ŷ ó��
bool Network_Req_Attack_3(stAccount* Account, CProtocolBuff* payload)
{
	// ---------------------------------
	// ���� ����, X,Y ������. ������ �¿츸 ����
	// ---------------------------------
	BYTE Dir;
	WORD X, Y;

	*payload >> Dir;
	*payload >> X;
	*payload >> Y;

	// �ϴ� �����ڸ� Idle���·� �����.
	Account->m_LoginPlayer->m_byNowAction = PLAYER_ACTION_IDLE;


	// ---------------------------------
	// ������ ����, 9���⿡ ���Ϳ� "3�� ���� ����" ��Ŷ�� ������.
	// ---------------------------------	
	// ���� ���� ��Ŷ�� �����.
	CProtocolBuff Attack_Header(dfNETWORK_PACKET_HEADER_SIZE);
	CProtocolBuff Attack_Payload;
	Network_Send_Attack_3(&Attack_Header, &Attack_Payload, Account->m_LoginPlayer->m_dwPlayerID, Dir, X, Y);

	// ������ ���� 9���� ���Ϳ� ���ݽ��� ��Ŷ�� ������. (������ �ڽ� ����)
	stSectorCheck secSector;
	GetSector(Account->m_LoginPlayer->m_SectorX, Account->m_LoginPlayer->m_SectorY, &secSector);

	if (SendPacket_Sector(Account->m_LoginPlayer, &Attack_Header, &Attack_Payload, &secSector) == FALSE)
		return FALSE;

	// ---------------------------------
	// ������ ��Ŷ ó��
	// ---------------------------------
	if (Network_Requ_Attck_3_Damage(Account->m_LoginPlayer->m_dwPlayerID, Account->m_LoginPlayer->m_wNowX, Account->m_LoginPlayer->m_wNowY, Dir) == FALSE)
		return FALSE;

	return TRUE;
}

// "���� 1��" ��Ŷ�� ������ ó��. Network_Req_Attack_1()���� ���.
bool Network_Requ_Attck_1_Damage(DWORD AttackID, int AttackX, int AttackY, BYTE AttackDir)
{
	// ���� Y��ǥ�� ĳ������ �߰������� ����
	AttackY -= dfATTACK_Y;

	// ���� ���⿡ �ٸ��� ó��.
	map<SOCKET, stAccount*>::iterator itor = map_Account.begin();
	map<SOCKET, stAccount*>::iterator enditor = map_Account.end();

	switch (AttackDir)
	{
	case dfPACKET_MOVE_DIR_LL:
	{
		for (; itor != enditor; ++itor)
		{
			// �������� ���ݹ����� �����ڰ� �ִ��� üũ
			// ���� ������ ������, ������ ���� 9���⿡ ������ ��Ŷ�� ������.
			if (itor->second->m_LoginPlayer->m_wNowX > AttackX - dfATTACK1_RANGE_X &&
				itor->second->m_LoginPlayer->m_wNowX < AttackX &&
				itor->second->m_LoginPlayer->m_wNowY - dfATTACK_Y > AttackY - dfATTACK1_RANGE_Y &&
				itor->second->m_LoginPlayer->m_wNowY - dfATTACK_Y < AttackY + dfATTACK1_RANGE_Y)
			{
				// �������� HP ���ҽ�Ű��.
				itor->second->m_LoginPlayer->m_byHP -= dfATTACK1_DAMAGE;

				// ������ ��Ŷ �����
				CProtocolBuff header(dfNETWORK_PACKET_HEADER_SIZE);
				CProtocolBuff payload;
				Network_Send_Damage(&header, &payload, AttackID, itor->second->m_LoginPlayer->m_dwPlayerID, itor->second->m_LoginPlayer->m_byHP);

				// ������ ���� 9���� �������� ������ (������ �ڽſ��Դ� �Ȱ���.)
				stSectorCheck secSector;
				GetSector(itor->second->m_LoginPlayer->m_SectorX, itor->second->m_LoginPlayer->m_SectorY, &secSector);

				SendPacket_Sector(itor->second->m_LoginPlayer, &header, &payload, &secSector);

				// ������ �ڽſ��� ������
				if (SendPacket(itor->second, &header, &payload) == FALSE)
					return FALSE;

				// �������� ���� Hp�� 0�̶�� ���� �����Ų��.
				//if (itor->second->m_LoginPlayer->m_byHP == 0)
					//Disconnect(itor->second);


				break;
			}

		}

	}
	break;

	case dfPACKET_MOVE_DIR_RR:
	{
		for (; itor != enditor; ++itor)
		{
			// �������� ���ݹ����� �����ڰ� �ִ��� üũ
			// ���� ������ ������, ������ ���� 9���⿡ ������ ��Ŷ�� ������.
			if (itor->second->m_LoginPlayer->m_wNowX < AttackX + dfATTACK1_RANGE_X &&
				itor->second->m_LoginPlayer->m_wNowX > AttackX &&
				itor->second->m_LoginPlayer->m_wNowY - dfATTACK_Y > AttackY - dfATTACK1_RANGE_Y &&
				itor->second->m_LoginPlayer->m_wNowY - dfATTACK_Y < AttackY + dfATTACK1_RANGE_Y)
			{
				// �������� HP ���ҽ�Ű��.
				itor->second->m_LoginPlayer->m_byHP -= dfATTACK1_DAMAGE;

				// ������ ��Ŷ �����
				CProtocolBuff header(dfNETWORK_PACKET_HEADER_SIZE);
				CProtocolBuff payload;
				Network_Send_Damage(&header, &payload, AttackID, itor->second->m_LoginPlayer->m_dwPlayerID, itor->second->m_LoginPlayer->m_byHP);

				// ������ ���� 9���� �������� ������ (������ �ڽſ��Դ� �Ȱ���.)
				stSectorCheck secSector;
				GetSector(itor->second->m_LoginPlayer->m_SectorX, itor->second->m_LoginPlayer->m_SectorY, &secSector);

				if (SendPacket_Sector(itor->second->m_LoginPlayer, &header, &payload, &secSector) == FALSE)
					return FALSE;

				// ������ �ڽſ��� ������
				if (SendPacket(itor->second->m_LoginPlayer->m_Account, &header, &payload) == FALSE)
					return FALSE;

				// �������� ���� Hp�� 0�̶�� ���� �����Ų��.
				//if (itor->second->m_LoginPlayer->m_byHP == 0)
					//Disconnect(itor->second);

				break;
			}

		}
	}
	break;

	}


	return TRUE;
}

// "���� 2��" ��Ŷ�� ������ ó��. Network_Req_Attack_2()���� ���.
bool Network_Requ_Attck_2_Damage(DWORD AttackID, int AttackX, int AttackY, BYTE AttackDir)
{
	// ���� Y��ǥ�� ĳ������ �߰������� ����
	AttackY -= dfATTACK_Y;

	// ���� ���⿡ �ٸ��� ó��.
	map<SOCKET, stAccount*>::iterator itor = map_Account.begin();
	map<SOCKET, stAccount*>::iterator enditor = map_Account.end();

	switch (AttackDir)
	{
	case dfPACKET_MOVE_DIR_LL:
	{
		for (; itor != enditor; ++itor)
		{
			// �������� ���ݹ����� �����ڰ� �ִ��� üũ
			// ���� ������ ������, ������ ���� 9���⿡ ������ ��Ŷ�� ������.
			if (itor->second->m_LoginPlayer->m_wNowX > AttackX - dfATTACK2_RANGE_X &&
				itor->second->m_LoginPlayer->m_wNowX < AttackX &&
				itor->second->m_LoginPlayer->m_wNowY - dfATTACK_Y > AttackY - dfATTACK2_RANGE_Y &&
				itor->second->m_LoginPlayer->m_wNowY - dfATTACK_Y < AttackY + dfATTACK2_RANGE_Y)
			{
				// �������� HP ���ҽ�Ű��.
				itor->second->m_LoginPlayer->m_byHP -= dfATTACK2_DAMAGE;

				// ������ ��Ŷ �����
				CProtocolBuff header(dfNETWORK_PACKET_HEADER_SIZE);
				CProtocolBuff payload;
				Network_Send_Damage(&header, &payload, AttackID, itor->second->m_LoginPlayer->m_dwPlayerID, itor->second->m_LoginPlayer->m_byHP);

				// ������ ���� 9���� �������� ������ (������ �ڽſ��Դ� �Ȱ���.)
				stSectorCheck secSector;
				GetSector(itor->second->m_LoginPlayer->m_SectorX, itor->second->m_LoginPlayer->m_SectorY, &secSector);

				SendPacket_Sector(itor->second->m_LoginPlayer, &header, &payload, &secSector);

				// ������ �ڽſ��� ������
				if (SendPacket(itor->second, &header, &payload) == FALSE)
					return FALSE;

				// �������� ���� Hp�� 0�̶�� ���� �����Ų��.
				//if (itor->second->m_LoginPlayer->m_byHP == 0)
					//Disconnect(itor->second);

				break;
			}

		}

	}
	break;

	case dfPACKET_MOVE_DIR_RR:
	{
		for (; itor != enditor; ++itor)
		{
			// �������� ���ݹ����� �����ڰ� �ִ��� üũ
			// ���� ������ ������, ������ ���� 9���⿡ ������ ��Ŷ�� ������.
			if (itor->second->m_LoginPlayer->m_wNowX < AttackX + dfATTACK2_RANGE_X &&
				itor->second->m_LoginPlayer->m_wNowX > AttackX &&
				itor->second->m_LoginPlayer->m_wNowY - dfATTACK_Y > AttackY - dfATTACK2_RANGE_Y &&
				itor->second->m_LoginPlayer->m_wNowY - dfATTACK_Y < AttackY + dfATTACK2_RANGE_Y)
			{
				// �������� HP ���ҽ�Ű��.
				itor->second->m_LoginPlayer->m_byHP -= dfATTACK2_DAMAGE;

				// ������ ��Ŷ �����
				CProtocolBuff header(dfNETWORK_PACKET_HEADER_SIZE);
				CProtocolBuff payload;
				Network_Send_Damage(&header, &payload, AttackID, itor->second->m_LoginPlayer->m_dwPlayerID, itor->second->m_LoginPlayer->m_byHP);

				// ������ ���� 9���� �������� ������ (������ �ڽſ��Դ� �Ȱ���.)
				stSectorCheck secSector;
				GetSector(itor->second->m_LoginPlayer->m_SectorX, itor->second->m_LoginPlayer->m_SectorY, &secSector);

				if (SendPacket_Sector(itor->second->m_LoginPlayer, &header, &payload, &secSector) == FALSE)
					return FALSE;

				// ������ �ڽſ��� ������
				if (SendPacket(itor->second->m_LoginPlayer->m_Account, &header, &payload) == FALSE)
					return FALSE;

				// �������� ���� Hp�� 0�̶�� ���� �����Ų��.
				//if (itor->second->m_LoginPlayer->m_byHP == 0)
				//Disconnect(itor->second);

				break;
			}

		}
	}
	break;

	}

	return TRUE;

}

// "���� 3��" ��Ŷ�� ������ ó��. Network_Req_Attack_3()���� ���.
bool Network_Requ_Attck_3_Damage(DWORD AttackID, int AttackX, int AttackY, BYTE AttackDir)
{
	// ���� Y��ǥ�� ĳ������ �߰������� ����
	AttackY -= dfATTACK_Y;

	// ���� ���⿡ �ٸ��� ó��.
	map<SOCKET, stAccount*>::iterator itor = map_Account.begin();
	map<SOCKET, stAccount*>::iterator enditor = map_Account.end();

	switch (AttackDir)
	{
	case dfPACKET_MOVE_DIR_LL:
	{
		for (; itor != enditor; ++itor)
		{
			// �������� ���ݹ����� �����ڰ� �ִ��� üũ
			// ���� ������ ������, ������ ���� 9���⿡ ������ ��Ŷ�� ������.
			if (itor->second->m_LoginPlayer->m_wNowX > AttackX - dfATTACK3_RANGE_X &&
				itor->second->m_LoginPlayer->m_wNowX < AttackX &&
				itor->second->m_LoginPlayer->m_wNowY - dfATTACK_Y > AttackY - dfATTACK3_RANGE_Y &&
				itor->second->m_LoginPlayer->m_wNowY - dfATTACK_Y < AttackY + dfATTACK3_RANGE_Y)
			{
				// �������� HP ���ҽ�Ű��.
				itor->second->m_LoginPlayer->m_byHP -= dfATTACK3_DAMAGE;

				// ������ ��Ŷ �����
				CProtocolBuff header(dfNETWORK_PACKET_HEADER_SIZE);
				CProtocolBuff payload;
				Network_Send_Damage(&header, &payload, AttackID, itor->second->m_LoginPlayer->m_dwPlayerID, itor->second->m_LoginPlayer->m_byHP);

				// ������ ���� 9���� �������� ������ (������ �ڽſ��Դ� �Ȱ���.)
				stSectorCheck secSector;
				GetSector(itor->second->m_LoginPlayer->m_SectorX, itor->second->m_LoginPlayer->m_SectorY, &secSector);

				SendPacket_Sector(itor->second->m_LoginPlayer, &header, &payload, &secSector);

				// ������ �ڽſ��� ������
				if (SendPacket(itor->second, &header, &payload) == FALSE)
					return FALSE;

				// �������� ���� Hp�� 0�̶�� ���� �����Ų��.
				//if (itor->second->m_LoginPlayer->m_byHP == 0)
					//Disconnect(itor->second);

				break;
			}

		}

	}
	break;

	case dfPACKET_MOVE_DIR_RR:
	{
		for (; itor != enditor; ++itor)
		{
			// �������� ���ݹ����� �����ڰ� �ִ��� üũ
			// ���� ������ ������, ������ ���� 9���⿡ ������ ��Ŷ�� ������.
			if (itor->second->m_LoginPlayer->m_wNowX < AttackX + dfATTACK3_RANGE_X &&
				itor->second->m_LoginPlayer->m_wNowX > AttackX &&
				itor->second->m_LoginPlayer->m_wNowY - dfATTACK_Y > AttackY - dfATTACK3_RANGE_Y &&
				itor->second->m_LoginPlayer->m_wNowY - dfATTACK_Y < AttackY + dfATTACK3_RANGE_Y)
			{
				// �������� HP ���ҽ�Ű��.
				itor->second->m_LoginPlayer->m_byHP -= dfATTACK3_DAMAGE;

				// ������ ��Ŷ �����
				CProtocolBuff header(dfNETWORK_PACKET_HEADER_SIZE);
				CProtocolBuff payload;
				Network_Send_Damage(&header, &payload, AttackID, itor->second->m_LoginPlayer->m_dwPlayerID, itor->second->m_LoginPlayer->m_byHP);

				// ������ ���� 9���� �������� ������ (������ �ڽſ��Դ� �Ȱ���.)
				stSectorCheck secSector;
				GetSector(itor->second->m_LoginPlayer->m_SectorX, itor->second->m_LoginPlayer->m_SectorY, &secSector);

				if (SendPacket_Sector(itor->second->m_LoginPlayer, &header, &payload, &secSector) == FALSE)
					return FALSE;

				// ������ �ڽſ��� ������
				if (SendPacket(itor->second->m_LoginPlayer->m_Account, &header, &payload) == FALSE)
					return FALSE;

				// �������� ���� Hp�� 0�̶�� ���� �����Ų��.
				//if (itor->second->m_LoginPlayer->m_byHP == 0)
				//Disconnect(itor->second);

				break;
			}

		}
	}
	break;

	}
	return TRUE;
}

// ���ڿ� ��Ŷ ó��(RTTüũ)
bool Network_Req_StressTest(stAccount* Account, CProtocolBuff* Packet)
{
	// 4����Ʈ ��������
	DWORD Time;
	*Packet >> Time;

	// 2. ���̷ε� ����
	// ���̷ε忡 �ð� ����
	CProtocolBuff Payload;
	Payload << Time;

	// 3. ��� ����
	CProtocolBuff header(dfNETWORK_PACKET_HEADER_SIZE);
	CreateHeader(&header, Payload.GetUseSize(), dfPACKET_SC_ECHO);
		
	// 4. ���� ��Ŷ�� �ش� ������, SendBuff�� �ֱ�
	SendPacket(Account, &header, &Payload);

	return true;
}





// ---------------------------------
// Send
// ---------------------------------
// SendBuff�� ������ �ֱ�
BOOL SendPacket(stAccount* Account, CProtocolBuff* header, CProtocolBuff* payload)
{	
	BEGIN("SendPacket");
	char* recvbuff = Account->m_SendBuff.GetBufferPtr();

	// 1. ���� q�� ��� �ֱ�
	int Size = dfNETWORK_PACKET_HEADER_SIZE;
	int EnqueueCheck = Account->m_SendBuff.Enqueue(header->GetBufferPtr(), Size);
	if (EnqueueCheck == -1)
	{
		SYSTEMTIME lst;
		GetLocalTime(&lst);
		_LOG(dfLOG_LEVEL_ERROR, L"[%02d/%02d/%02d %02d:%02d:%02d] [ERROR]SendPacket() ����ִ� �� ������ ����.(PlayerID : %d)\n",
			lst.wYear - 2000, lst.wMonth, lst.wDay, lst.wHour, lst.wMinute, lst.wSecond, Account->m_LoginPlayer->m_dwPlayerID);

		return FALSE;
	}	

	// 2. ���� q�� ���̷ε� �ֱ�
	int PayloadLen = payload->GetUseSize();
	EnqueueCheck = Account->m_SendBuff.Enqueue(payload->GetBufferPtr(), PayloadLen);
	if (EnqueueCheck == -1)
	{
		SYSTEMTIME lst;
		GetLocalTime(&lst);
		_LOG(dfLOG_LEVEL_ERROR, L"[%02d/%02d/%02d %02d:%02d:%02d] [ERROR]SendPacket() ���̷ε� �ִ� �� ������ ����.(PlayerID : %d)\n",
			lst.wYear - 2000, lst.wMonth, lst.wDay, lst.wHour, lst.wMinute, lst.wSecond, Account->m_LoginPlayer->m_dwPlayerID);

		return FALSE;
	}	

	// 3. ���� q�� �����ڵ� �ֱ�
	char EndCode = dfNETWORK_PACKET_END;
	EnqueueCheck = Account->m_SendBuff.Enqueue(&EndCode, 1);
	if (EnqueueCheck == -1)
	{
		SYSTEMTIME lst;
		GetLocalTime(&lst);
		_LOG(dfLOG_LEVEL_ERROR, L"[%02d/%02d/%02d %02d:%02d:%02d] [ERROR]SendPacket() �����ڵ� �ִ� �� ������ ����.(PlayerID : %d)\n",
			lst.wYear - 2000, lst.wMonth, lst.wDay, lst.wHour, lst.wMinute, lst.wSecond, Account->m_LoginPlayer->m_dwPlayerID);

		return FALSE;	
	}	

	END("SendPacket");

	return TRUE;
}

// SendBuff�� �����͸� Send�ϱ�
bool SendProc(stAccount* Account)
{
	if (Account == nullptr)
		return false;


	BEGIN("SendProc");

	// 1. SendBuff�� ���� �����Ͱ� �ִ��� Ȯ��.
	if (Account->m_SendBuff.GetUseSize() == 0)
		return true;

	// 2. ���� �������� �������� ���� ������
	char* sendbuff = Account->m_SendBuff.GetBufferPtr();

	// 3. SendBuff�� �� �������� or Send����(����)���� ��� send
	while (1)
	{
		// 4. SendBuff�� �����Ͱ� �ִ��� Ȯ��(���� üũ��)
		if (Account->m_SendBuff.GetUseSize() == 0)
			break;

		// 5. �� ���� ���� �� �ִ� �������� ���� ���� ���
		int Size = Account->m_SendBuff.GetNotBrokenGetSize();

		// 6. ���� ������ 0�̶�� 1�� ����. send() �� 1����Ʈ�� �õ��ϵ��� �ϱ� ���ؼ�.
		// �׷��� send()�� ������ �ߴ��� Ȯ�� ����
		if (Size == 0)
			Size = 1;

		// 7. front ������ ����
		int *front = (int*)Account->m_SendBuff.GetReadBufferPtr();

		// 8. front�� 1ĭ �� index�� �˾ƿ���.
		int BuffArrayCheck = Account->m_SendBuff.NextIndex(*front, 1);

		// 9. Send()
		int SendSize = send(Account->m_sock, &sendbuff[BuffArrayCheck], Size, 0);

		// 10. ���� üũ. �����̸� �� �̻� �������� while�� ����. ���� Select�� �ٽ� ����
		if (SendSize == SOCKET_ERROR)
		{
			if (WSAGetLastError() == WSAEWOULDBLOCK || 
				WSAGetLastError() == 10054)
				break;

			else
			{
				_tprintf(_T("SendProc(). Send�� ���� �߻�. ���� ���� (%d)\n"), WSAGetLastError());
				return false;
			}

		}

		// 11. ���� ����� ��������, �� ��ŭ remove
		Account->m_SendBuff.RemoveData(SendSize);		
	}

	END("SendProc");

	return true;
}

