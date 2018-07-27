#include "stdafx.h"
#include <mmsystem.h>
#pragma comment(lib,"winmm.lib")
#include "Network_Func.h"

#include <map>

#define dfNETWORK_PORT		20000

// ������ �� ũ��
#define MAP_WIDTH			6400
#define MAP_HEIGHT			6400

// ���� ����
#define PLAYER_ACTION_IDLE		0	// ����
#define PLAYER_ACTION_MOVE		1	// �̵�

// 1�����ӿ� �̵��ϴ� ��ǥ
#define PLAYER_XMOVE_PIXEL 3
#define PLAYER_YMOVE_PIXEL 2

// ���� �̵�����
#define dfRANGE_MOVE_TOP	0 + 1
#define dfRANGE_MOVE_LEFT	0 + 2
#define dfRANGE_MOVE_RIGHT	MAP_WIDTH - 3
#define dfRANGE_MOVE_BOTTOM	MAP_HEIGHT - 1

// AIEnum
enum AI 
{
	MOVE_LL = 0, MOVE_LU, MOVE_UU, MOVE_RU, MOVE_RR, MOVE_RD, MOVE_DD, MOVE_LD, MOVE_STOP,
	ATTACK_01_LEFT, ATTACK_01_RIGHT, ATTACK_02_LEFT, ATTACK_02_RIGHT, ATTACK_03_LEFT, ATTACK_03_RIGHT
};



using namespace std;

// ���� ���� ����ü
struct stSession
{
	SOCKET m_sock;

	CRingBuff m_RecvBuff;
	CRingBuff m_SendBuff;

	// ����� ����Ŭ�� �˰��ִ�.
	stDummyClient* m_DummyClient = nullptr;
};

// ���� Ŭ�� ����ü
struct stDummyClient
{	
	// �÷��̾� ID
	DWORD	m_dwClientID;

	// ���� X,Y ��ǥ
	WORD	m_wNowX;
	WORD	m_wNowY;

	// ���� ����
	BYTE	m_byDir;

	// ü��
	BYTE	m_byHP;

	// ���� �׼�, ���� �׼�
	BYTE	m_byOldAction;
	BYTE	m_byNowAction;

	// ������ �˰��ִ�. ���ʴ� nullptr
	stSession* m_Session = nullptr;

	// -------------------
	// ���巹Ŀ�� ����
	// -------------------
	// ���巹Ŀ���� ����, NowAction�� ���� �ð�. 
	ULONGLONG m_ullActionTick;

	// ���巹Ŀ���� ����, �׼� ���� ������ X,Y��ǥ ����(������ǥ)
	short	m_wActionX;
	short	m_wActionY;

	// -------------------
	// AI ����
	// -------------------
	// AI ���� �ð�. �ð��� �����ص״ٰ� n�ʿ� �ѹ� AI�� �Ѵ�.
	ULONGLONG m_ullAIStartTick;
};



// �ܺκ���
extern int		g_iDummyCount;					// main���� ������ Dummy ī��Ʈ												
extern TCHAR	g_tIP[30];						// main���� ������ ������ IP

extern int		g_LogLevel;						// Main���� �Է��� �α� ��� ����. �ܺκ���
extern TCHAR	g_szLogBuff[1024];				// Main���� �Է��� �α� ��¿� �ӽ� ����. �ܺκ���
												
extern int g_iRecvByte;							// Recv, Send����Ʈ
extern int g_iSendByte;

extern DWORD g_dwJoinUserCount;					// Accept�� ���� ��

extern DWORD g_dwSyncCount;						// ��ũ���� Ƚ�� ī��Ʈ


// �÷��̾� ���� map
map <DWORD, stDummyClient*> map_Clientmap;

// ���� ���� map
map <SOCKET, stSession*> map_Sessionmap;

// ������ �÷��̾� ���� �迭
stDummyClient** g_PlayerArray;
DWORD g_PlayerArrayCount;

// -------------------
// RTT ����
// -------------------
// RTT���´��� üũ. true�� ������ ���� ����.
bool g_RTTSendCheck = true;

// RTT ���� ���� �ð� ����.
ULONGLONG g_RTTSendTime;

// ���´� SendData�� ����ص״ٰ� �� �Դ��� üũ�Ѵ�.
DWORD g_RTTSendData;	

// 1�ʵ��� RTT�� ���� �� ���� Ƚ�� ����
extern int g_RTTSendCount;

// 1�ʵ����� RTT ��� �ð� ����.
ULONGLONG g_RTTAvgTime;

// RTT Max
extern ULONGLONG g_RTTMax;




// ---------------------------------
// ��Ÿ �Լ�
// ---------------------------------
// ��Ʈ��ũ ���� �Լ� (�����ʱ�ȭ, Ŀ��Ʈ ��..)
bool Network_Init(int* TryCount, int* FailCount, int* SuccessCount)
{
	// �Է��� ���� �� ��ŭ �����Ҵ�
	g_PlayerArray = new stDummyClient*[g_iDummyCount];

	// ���� �ʱ�ȭ
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
	{
		_tprintf(_T("WSAStartup ����!\n"));
		return false;
	}

	// Dummy ī��Ʈ��ŭ ���鼭 ���� ����
	for (int i = 0; i < g_iDummyCount; ++i)
	{
		stSession* NewAccount = new stSession;

		// 1. ���� ����
		NewAccount->m_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (NewAccount->m_sock == INVALID_SOCKET)
		{
			_tprintf(_T("%d��° ���� ���� ����!\n"), i);
			return false;
		}

		// 2. Connect
		SOCKADDR_IN clientaddr;
		ZeroMemory(&clientaddr, sizeof(clientaddr));
		clientaddr.sin_family = AF_INET;
		clientaddr.sin_port = htons(dfNETWORK_PORT);
		InetPton(AF_INET, g_tIP, &clientaddr.sin_addr.s_addr);

		DWORD dCheck = connect(NewAccount->m_sock, (SOCKADDR*)&clientaddr, sizeof(clientaddr));

		// 3. Ŀ��Ʈ �õ� Ƚ�� ����
		(*TryCount)++;

		// 3. Ŀ��Ʈ ���� ��, ���� ī��Ʈ 1 ����
		if (dCheck == SOCKET_ERROR)
		{
			_tprintf(_T("connect ����. %d (Error : %d)\n"), i + 1, WSAGetLastError());
			(*FailCount)++;
		}
		// Ŀ��Ʈ ���� ��, ���� ī��Ʈ 1����. 
		// ���� �������, �÷��̾� ��Ͽ� �߰�
		else
		{
			// Ŭ�� ���� ����
			stDummyClient* NewClient = new stDummyClient;
			NewClient->m_Session = NewAccount;	
						
			// ���� ���� ���� �� map�� �߰�.
			NewAccount->m_DummyClient = NewClient;
			map_Sessionmap.insert(pair<SOCKET, stSession*>(NewAccount->m_sock, NewAccount));

			// ȭ�� ���
			_tprintf(_T("connect ����. %d\n"), i + 1);

			// ���� ī��Ʈ ����
			(*SuccessCount)++;
			g_dwJoinUserCount++;		
			
		}

	}

	return true;
}

// ��Ʈ��ũ ���μ���
// ���⼭ false�� ��ȯ�Ǹ�, ���α׷��� ����ȴ�.
bool Network_Process()
{
	// ��ſ� ����� ����
	FD_SET rset;
	FD_SET wset;

	map <SOCKET, stSession*>::iterator itor;
	map <SOCKET, stSession*>::iterator enditor;
	TIMEVAL tval;
	tval.tv_sec = 0;
	tval.tv_usec = 0;

	itor = map_Sessionmap.begin();

	// ���� ���� ����. 
	rset.fd_count = 0;
	wset.fd_count = 0;

	while (1)
	{
		enditor = map_Sessionmap.end();

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
			if(WSAGetLastError() != WSAEINVAL && rset.fd_count != 0 && wset.fd_count != 0)
				_tprintf(_T("select ����(%d)\n"), WSAGetLastError());

			return false;
		}

		// select�� ���� 0���� ũ�ٸ� ���� �Ұ� �ִٴ� ���̴� ���� ����
		if (dCheck > 0)
		{
			DWORD rsetCount = 0;
			DWORD wsetCount = 0;

			while (1)
			{
				if (rsetCount < rset.fd_count)
				{
					stSession* NowAccount = ClientSearch_AcceptList(rset.fd_array[rsetCount]);

					// Recv() ó��
					// ����, RecvProc()�Լ��� false�� ���ϵȴٸ�, �ش� ���� ���� ����
					if (RecvProc(NowAccount) == false)
						Disconnect(NowAccount);

					rsetCount++;
				}

				if (wsetCount < wset.fd_count)
				{
					stSession* NowAccount = ClientSearch_AcceptList(wset.fd_array[wsetCount]);

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

	return true;
}

// Disconnect ó��
void Disconnect(stSession* Account)
{
	// ���ܻ��� üũ
	// 1. �ش� ������ �α��� ���� �����ΰ�.
	// NowUser�� ���� nullptr�̸�, �ش� ������ ��ã�� ��. false�� �����Ѵ�.
	if (Account == nullptr)
	{
		//_LOG(dfLOG_LEVEL_ERROR, L"Disconnect(). Accept ���� �ƴ� ������ ������� ���� �õ�\n");
		return;
	}

	fputs("Disconnect!\n", stdout);

	// �ش� ������ ���� close
	closesocket(Account->m_sock);

	// �ش� ������ ���� �ʿ��� ����
	map_Sessionmap.erase(Account->m_sock);

	// �ش� ������ �÷��̾� �ʿ��� ����
	map_Clientmap.erase(Account->m_DummyClient->m_dwClientID);

	// �迭������ ����
	for (int i = 0; i < g_PlayerArrayCount; ++i)
	{
		// ������ ���̸� ã������
		if (g_PlayerArray[i] == Account->m_DummyClient)
		{
			// ���� ���� ���̸� �� ��ġ�� ����
			g_PlayerArray[i] = g_PlayerArray[g_PlayerArrayCount - 1];
			g_PlayerArrayCount--;
			break;
		}
	}
	
	// �÷��̾� ��������
	delete Account->m_DummyClient;

	// ���� ��������
	delete Account;	

	

	// ������ ���� �� ����
	g_dwJoinUserCount--;	
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

// ���巹Ŀ�� �Լ�
// ���ڷ� (���� �׼�, �׼� ���� �ð�, �׼� ���� ��ġ, (OUT)��� �� ��ǥ)�� �޴´�.
void DeadReckoning(BYTE NowAction, ULONGLONG ActionTick, int ActionStartX, int ActionStartY, int* ResultX, int* ResultY)
{
	// ----------------------------
	// �׼� ���۽ð����� ���� �ð������� �ð����� ���Ѵ�. 
	// �� �ð����� �� �������� �������� ����Ѵ�.
	// ----------------------------
	int IntervalFrame = (GetTickCount64() - ActionTick) / 20; // (20�� 1������. 50���������� ����)

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
	// 4. ��� ����� �Ϸ�Ǿ�����, PosX�� PosY�� ���� �� ������ ��´�.(���ó��)
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
}

// �ʴ� RTT ���.
ULONGLONG RTTAvr()
{	
	ULONGLONG returnRTT = 0;

	if (g_RTTAvgTime == 0 || g_RTTSendCount == 0)
		return returnRTT;

	// ��� ���ϱ�
	returnRTT = g_RTTAvgTime / g_RTTSendCount;

	// ���� ����� ���� �� �ʱ�ȭ
	g_RTTAvgTime = 0;

	return returnRTT;
}







// ---------------------------------
// �˻��� �Լ�
// --------------------------------
// ���ڷ� ���� Socket ���� �������� [ȸ�� ���]���� [������ ��󳽴�].(�˻�)
// ���� ��, �ش� ������ ���� ����ü�� �ּҸ� ����
// ���� �� nullptr ����
stSession* ClientSearch_AcceptList(SOCKET sock)
{
	map <SOCKET, stSession*>::iterator iter;

	iter = map_Sessionmap.find(sock);
	if (iter == map_Sessionmap.end())
		return nullptr;

	return iter->second;
}

// ���ڷ� ���� ���� ID�� [�÷��̾� ���]���� [�÷��̾ ��󳽴�.] (�˻�)
// ���� ��, �ش� �÷��̾��� ����ü �ּҸ� ����
// ���� �� nullptr ����
stDummyClient* ClientSearch_ClientList(DWORD ID)
{
	map <DWORD, stDummyClient*>::iterator iter;

	iter = map_Clientmap.find(ID);
	if (iter == map_Clientmap.end())
		return nullptr;

	return iter->second;
}






// ---------------------------------
// AI �Լ�
// ---------------------------------
void DummyAI()
{
	static DWORD dwNowDummyID = 0;			// ���� üũ�ؾ��ϴ� ������ ID
	static DWORD dwAIPlayTick = 3000;		// AI�� 3000�и�������(3��)�� 1ȸ ����.
	int Count = 0;

	// ��� Playerĳ���͸�, ���鼭 AI �� ���� �Ǿ����� �Ѵ�. 1�����ӿ� 20�� ó��. 1�ʿ� 50�������̴�, 1�ʿ� �� 1000���� ���� ó��
	while (Count < 20 && Count < g_PlayerArrayCount && dwNowDummyID < g_PlayerArrayCount)
	{
		// RTT��Ŷ�� ������.
		if (g_RTTSendCheck == true)
		{
			// RTT ��Ŷ �����.
			CProtocolBuff header(dfNETWORK_PACKET_HEADER_SIZE);
			CProtocolBuff payload;

			Network_Send_Echo(&header, &payload);

			// 0��° ������ ������ۿ� �ְ�
			SendPacket(g_PlayerArray[dwNowDummyID]->m_Session, &header, &payload);

			// ���� �ð� ����
			g_RTTSendTime = GetTickCount64();

			g_RTTSendCheck = false;
		}

		// 3�ʰ� �Ǿ��ٸ�
		if ((GetTickCount64() - g_PlayerArray[dwNowDummyID]->m_ullAIStartTick) >= dwAIPlayTick)
		{			
			// AI�� �Ѵ�.
			// AI�� �̵�����(8����), ����, ���ݽ���(1,2,3��. ���� 1���� ��,�� 2����) �� 1�� ����. �� 15��

			int AICheck = rand() % 14;

			switch (AICheck)
			{
			case AI::MOVE_LL:
			{
				// ������ AI�� �� �������� ����
				if (g_PlayerArray[dwNowDummyID]->m_byNowAction == PLAYER_ACTION_MOVE && g_PlayerArray[dwNowDummyID]->m_byDir == dfPACKET_MOVE_DIR_LL)
					break;

				// AI ���� �ð� ����
				g_PlayerArray[dwNowDummyID]->m_ullAIStartTick = GetTickCount64();

				// Idel ���°� �ƴϸ� ���巹Ŀ������ ��ǥ �̵�
				if(g_PlayerArray[dwNowDummyID]->m_byNowAction != PLAYER_ACTION_IDLE)
					Action_Move(g_PlayerArray[dwNowDummyID]);

				//  �׼� ���� �ð�, ���� �׼�, ���� ����
				g_PlayerArray[dwNowDummyID]->m_ullActionTick = GetTickCount64();
				g_PlayerArray[dwNowDummyID]->m_byNowAction = PLAYER_ACTION_MOVE;
				g_PlayerArray[dwNowDummyID]->m_byDir = dfPACKET_MOVE_DIR_LL;				

				// �̵� ���� ��Ŷ�� �����.
				CProtocolBuff header(dfNETWORK_PACKET_HEADER_SIZE);
				CProtocolBuff payload;
				Network_Send_MoveStart(g_PlayerArray[dwNowDummyID]->m_byDir, g_PlayerArray[dwNowDummyID]->m_wNowX, g_PlayerArray[dwNowDummyID]->m_wNowY, &header, &payload);

				// �ش� ������ SendBuff�� �ִ´�.
				SendPacket(g_PlayerArray[dwNowDummyID]->m_Session, &header, &payload);				
			}
			break;

			
			case AI::MOVE_LU:
			{
				// ������ AI�� �� �������� ����
				if (g_PlayerArray[dwNowDummyID]->m_byNowAction == PLAYER_ACTION_MOVE && g_PlayerArray[dwNowDummyID]->m_byDir == dfPACKET_MOVE_DIR_LU)
					break;	

				// AI ���� �ð� ����
				g_PlayerArray[dwNowDummyID]->m_ullAIStartTick = GetTickCount64();
				
				// Idel ���°� �ƴϸ� ���巹Ŀ������ ��ǥ �̵�
				if (g_PlayerArray[dwNowDummyID]->m_byNowAction != PLAYER_ACTION_IDLE)
					Action_Move(g_PlayerArray[dwNowDummyID]);

				// �ð�, ���� �׼�, ���� ����
				g_PlayerArray[dwNowDummyID]->m_ullActionTick = GetTickCount64();
				g_PlayerArray[dwNowDummyID]->m_byNowAction = PLAYER_ACTION_MOVE;
				g_PlayerArray[dwNowDummyID]->m_byDir = dfPACKET_MOVE_DIR_LU;
				
				// �̵� ���� ��Ŷ�� �����.
				CProtocolBuff header(dfNETWORK_PACKET_HEADER_SIZE);
				CProtocolBuff payload;
				Network_Send_MoveStart(g_PlayerArray[dwNowDummyID]->m_byDir, g_PlayerArray[dwNowDummyID]->m_wNowX, g_PlayerArray[dwNowDummyID]->m_wNowY, &header, &payload);

				// �ش� ������ SendBuff�� �ִ´�.
				SendPacket(g_PlayerArray[dwNowDummyID]->m_Session, &header, &payload);

				
			}
			break;

			
			case AI::MOVE_UU:
			{
				// ������ AI�� �� �������� ����
				if (g_PlayerArray[dwNowDummyID]->m_byNowAction == PLAYER_ACTION_MOVE && g_PlayerArray[dwNowDummyID]->m_byDir == dfPACKET_MOVE_DIR_UU)
					break;		

				// AI ���� �ð� ����
				g_PlayerArray[dwNowDummyID]->m_ullAIStartTick = GetTickCount64();

				// Idel ���°� �ƴϸ� ���巹Ŀ������, ���� �ൿ�� ��ǥ ����
				if (g_PlayerArray[dwNowDummyID]->m_byNowAction != PLAYER_ACTION_IDLE)
					Action_Move(g_PlayerArray[dwNowDummyID]);

				// �ð�, ���� �׼�, ���� ����
				g_PlayerArray[dwNowDummyID]->m_ullActionTick = GetTickCount64();
				g_PlayerArray[dwNowDummyID]->m_byNowAction = PLAYER_ACTION_MOVE;
				g_PlayerArray[dwNowDummyID]->m_byDir = dfPACKET_MOVE_DIR_UU;

				// �̵� ���� ��Ŷ�� �����.
				CProtocolBuff header(dfNETWORK_PACKET_HEADER_SIZE);
				CProtocolBuff payload;
				Network_Send_MoveStart(g_PlayerArray[dwNowDummyID]->m_byDir, g_PlayerArray[dwNowDummyID]->m_wNowX, g_PlayerArray[dwNowDummyID]->m_wNowY, &header, &payload);

				// �ش� ������ SendBuff�� �ִ´�.
				SendPacket(g_PlayerArray[dwNowDummyID]->m_Session, &header, &payload);

				
			}
			break;
			
			case AI::MOVE_RU:
			{
				// ������ AI�� �� �������� ����
				if (g_PlayerArray[dwNowDummyID]->m_byNowAction == PLAYER_ACTION_MOVE && g_PlayerArray[dwNowDummyID]->m_byDir == dfPACKET_MOVE_DIR_RU)
					break;	

				// AI ���� �ð� ����
				g_PlayerArray[dwNowDummyID]->m_ullAIStartTick = GetTickCount64();

				// Idel ���°� �ƴϸ� ���巹Ŀ������, ���� �ൿ�� ��ǥ ����
				if (g_PlayerArray[dwNowDummyID]->m_byNowAction != PLAYER_ACTION_IDLE)
					Action_Move(g_PlayerArray[dwNowDummyID]);

				// �ð�, ���� �׼�, ���� ����
				g_PlayerArray[dwNowDummyID]->m_ullActionTick = GetTickCount64();
				g_PlayerArray[dwNowDummyID]->m_byNowAction = PLAYER_ACTION_MOVE;
				g_PlayerArray[dwNowDummyID]->m_byDir = dfPACKET_MOVE_DIR_RU;

				// �̵� ���� ��Ŷ�� �����.
				CProtocolBuff header(dfNETWORK_PACKET_HEADER_SIZE);
				CProtocolBuff payload;
				Network_Send_MoveStart(g_PlayerArray[dwNowDummyID]->m_byDir, g_PlayerArray[dwNowDummyID]->m_wNowX, g_PlayerArray[dwNowDummyID]->m_wNowY, &header, &payload);

				// �ش� ������ SendBuff�� �ִ´�.
				SendPacket(g_PlayerArray[dwNowDummyID]->m_Session, &header, &payload);
				
			}
			break;
			
			case AI::MOVE_RR:
			{
				// ������ AI�� �� �������� ����
				if (g_PlayerArray[dwNowDummyID]->m_byNowAction == PLAYER_ACTION_MOVE && g_PlayerArray[dwNowDummyID]->m_byDir == dfPACKET_MOVE_DIR_RR)
					break;	

				// AI ���� �ð� ����
				g_PlayerArray[dwNowDummyID]->m_ullAIStartTick = GetTickCount64();

				// Idel ���°� �ƴϸ� ���巹Ŀ������ ��ǥ �̵�
				if (g_PlayerArray[dwNowDummyID]->m_byNowAction != PLAYER_ACTION_IDLE)
					Action_Move(g_PlayerArray[dwNowDummyID]);

				// �׼� ���� �ð�, ���� �׼�, ���� ����
				g_PlayerArray[dwNowDummyID]->m_ullActionTick = GetTickCount64();
				g_PlayerArray[dwNowDummyID]->m_byNowAction = PLAYER_ACTION_MOVE;
				g_PlayerArray[dwNowDummyID]->m_byDir = dfPACKET_MOVE_DIR_RR;

				// �̵� ���� ��Ŷ�� �����.
				CProtocolBuff header(dfNETWORK_PACKET_HEADER_SIZE);
				CProtocolBuff payload;
				Network_Send_MoveStart(g_PlayerArray[dwNowDummyID]->m_byDir, g_PlayerArray[dwNowDummyID]->m_wNowX, g_PlayerArray[dwNowDummyID]->m_wNowY, &header, &payload);

				// �ش� ������ SendBuff�� �ִ´�.
				SendPacket(g_PlayerArray[dwNowDummyID]->m_Session, &header, &payload);
			}
			break;

			case AI::MOVE_RD:
			{
				// ������ AI�� �� �������� ����
				if (g_PlayerArray[dwNowDummyID]->m_byNowAction == PLAYER_ACTION_MOVE && g_PlayerArray[dwNowDummyID]->m_byDir == dfPACKET_MOVE_DIR_RD)
					break;	

				// AI ���� �ð� ����
				g_PlayerArray[dwNowDummyID]->m_ullAIStartTick = GetTickCount64();

				// Idel ���°� �ƴϸ� ���巹Ŀ������ ��ǥ �̵�
				if (g_PlayerArray[dwNowDummyID]->m_byNowAction != PLAYER_ACTION_IDLE)
					Action_Move(g_PlayerArray[dwNowDummyID]);

				// �׼� ���� �ð�, ���� �׼�, ���� ����
				g_PlayerArray[dwNowDummyID]->m_ullActionTick = GetTickCount64();
				g_PlayerArray[dwNowDummyID]->m_byNowAction = PLAYER_ACTION_MOVE;
				g_PlayerArray[dwNowDummyID]->m_byDir = dfPACKET_MOVE_DIR_RD;

				// �̵� ���� ��Ŷ�� �����.
				CProtocolBuff header(dfNETWORK_PACKET_HEADER_SIZE);
				CProtocolBuff payload;
				Network_Send_MoveStart(g_PlayerArray[dwNowDummyID]->m_byDir, g_PlayerArray[dwNowDummyID]->m_wNowX, g_PlayerArray[dwNowDummyID]->m_wNowY, &header, &payload);

				// �ش� ������ SendBuff�� �ִ´�.
				SendPacket(g_PlayerArray[dwNowDummyID]->m_Session, &header, &payload);
			}
			break;

			case AI::MOVE_DD:
			{
				// ������ AI�� �� �������� ����
				if (g_PlayerArray[dwNowDummyID]->m_byNowAction == PLAYER_ACTION_MOVE && g_PlayerArray[dwNowDummyID]->m_byDir == dfPACKET_MOVE_DIR_DD)
					break;	

				// AI ���� �ð� ����
				g_PlayerArray[dwNowDummyID]->m_ullAIStartTick = GetTickCount64();

				// Idel ���°� �ƴϸ� ���巹Ŀ������ ��ǥ �̵�
				if (g_PlayerArray[dwNowDummyID]->m_byNowAction != PLAYER_ACTION_IDLE)
					Action_Move(g_PlayerArray[dwNowDummyID]);

				// �׼� ���� �ð�, ���� �׼�, ���� ����
				g_PlayerArray[dwNowDummyID]->m_ullActionTick = GetTickCount64();
				g_PlayerArray[dwNowDummyID]->m_byNowAction = PLAYER_ACTION_MOVE;
				g_PlayerArray[dwNowDummyID]->m_byDir = dfPACKET_MOVE_DIR_DD;

				// �̵� ���� ��Ŷ�� �����.
				CProtocolBuff header(dfNETWORK_PACKET_HEADER_SIZE);
				CProtocolBuff payload;
				Network_Send_MoveStart(g_PlayerArray[dwNowDummyID]->m_byDir, g_PlayerArray[dwNowDummyID]->m_wNowX, g_PlayerArray[dwNowDummyID]->m_wNowY, &header, &payload);

				// �ش� ������ SendBuff�� �ִ´�.
				SendPacket(g_PlayerArray[dwNowDummyID]->m_Session, &header, &payload);
			}
			break;

			case AI::MOVE_LD:
			{
				// ������ AI�� �� �������� ����
				if (g_PlayerArray[dwNowDummyID]->m_byNowAction == PLAYER_ACTION_MOVE && g_PlayerArray[dwNowDummyID]->m_byDir == dfPACKET_MOVE_DIR_LD)
					break;

				// AI ���� �ð� ����
				g_PlayerArray[dwNowDummyID]->m_ullAIStartTick = GetTickCount64();

				// Idel ���°� �ƴϸ� ���巹Ŀ������ ��ǥ �̵�
				if (g_PlayerArray[dwNowDummyID]->m_byNowAction != PLAYER_ACTION_IDLE)
					Action_Move(g_PlayerArray[dwNowDummyID]);

				// �׼� ���� �ð�, ���� �׼�, ���� ����
				g_PlayerArray[dwNowDummyID]->m_ullActionTick = GetTickCount64();
				g_PlayerArray[dwNowDummyID]->m_byNowAction = PLAYER_ACTION_MOVE;
				g_PlayerArray[dwNowDummyID]->m_byDir = dfPACKET_MOVE_DIR_LD;

				// �̵� ���� ��Ŷ�� �����.
				CProtocolBuff header(dfNETWORK_PACKET_HEADER_SIZE);
				CProtocolBuff payload;
				Network_Send_MoveStart(g_PlayerArray[dwNowDummyID]->m_byDir, g_PlayerArray[dwNowDummyID]->m_wNowX, g_PlayerArray[dwNowDummyID]->m_wNowY, &header, &payload);

				// �ش� ������ SendBuff�� �ִ´�.
				SendPacket(g_PlayerArray[dwNowDummyID]->m_Session, &header, &payload);
			}
			break;
			
			case AI::MOVE_STOP:
			{
				// ������ AI�� �� �������� ����
				if (g_PlayerArray[dwNowDummyID]->m_byNowAction == PLAYER_ACTION_IDLE)
					break;

				// AI ���� �ð� ����
				g_PlayerArray[dwNowDummyID]->m_ullAIStartTick = GetTickCount64();

				// ���巹Ŀ������ ��ǥ �̵�
				Action_Move(g_PlayerArray[dwNowDummyID]);		

				// �ð�, ���� �׼�, ���� ����	
				g_PlayerArray[dwNowDummyID]->m_ullActionTick = GetTickCount64();
				g_PlayerArray[dwNowDummyID]->m_byNowAction = PLAYER_ACTION_IDLE;	
				g_PlayerArray[dwNowDummyID]->m_byDir = dfPACKET_MOVE_DIR_RR;

				if (g_PlayerArray[dwNowDummyID]->m_byDir == dfPACKET_MOVE_DIR_LL ||
					g_PlayerArray[dwNowDummyID]->m_byDir == dfPACKET_MOVE_DIR_LU ||
					g_PlayerArray[dwNowDummyID]->m_byDir == dfPACKET_MOVE_DIR_UU ||
					g_PlayerArray[dwNowDummyID]->m_byDir == dfPACKET_MOVE_DIR_LD)
				{
					g_PlayerArray[dwNowDummyID]->m_byDir = dfPACKET_MOVE_DIR_LL;
				}

				// ���� ���� ��Ŷ�� �����.
				CProtocolBuff header(dfNETWORK_PACKET_HEADER_SIZE);
				CProtocolBuff payload;
				Network_Send_MoveStop(g_PlayerArray[dwNowDummyID]->m_byDir, g_PlayerArray[dwNowDummyID]->m_wNowX, g_PlayerArray[dwNowDummyID]->m_wNowY, &header, &payload);

				// �ش� ������ SendBuff�� �ִ´�.
				SendPacket(g_PlayerArray[dwNowDummyID]->m_Session, &header, &payload);
				
			}
			break;		
			
			case AI::ATTACK_01_LEFT:
			{	
				// AI ���� �ð� ����
				g_PlayerArray[dwNowDummyID]->m_ullAIStartTick = GetTickCount64();

				// �̵����̾��ٸ� ���巹Ŀ������ ��ǥ �̵�
				if(g_PlayerArray[dwNowDummyID]->m_byNowAction == PLAYER_ACTION_MOVE)
					Action_Move(g_PlayerArray[dwNowDummyID]);

				// -----------------------------
				// ���ݽ��� ��Ŷ �����
				// -----------------------------
				// �ð�, ���� �׼�, ���� ����	
				g_PlayerArray[dwNowDummyID]->m_ullActionTick = GetTickCount64();
				g_PlayerArray[dwNowDummyID]->m_byNowAction = PLAYER_ACTION_IDLE;
				g_PlayerArray[dwNowDummyID]->m_byDir = dfPACKET_MOVE_DIR_LL;

				// ���� ���� ��Ŷ�� �����.
				CProtocolBuff header_Attack(dfNETWORK_PACKET_HEADER_SIZE);
				CProtocolBuff payload_Attack;
				Network_Send_Attack_01(g_PlayerArray[dwNowDummyID]->m_byDir, g_PlayerArray[dwNowDummyID]->m_wNowX, g_PlayerArray[dwNowDummyID]->m_wNowY, &header_Attack, &payload_Attack);

				// �ش� ������ SendBuff�� �ִ´�.
				SendPacket(g_PlayerArray[dwNowDummyID]->m_Session, &header_Attack, &payload_Attack);

			}
			break;
			
			
			case AI::ATTACK_01_RIGHT:
			{
				// AI ���� �ð� ����
				g_PlayerArray[dwNowDummyID]->m_ullAIStartTick = GetTickCount64();

				// �̵����̾��ٸ� ���巹Ŀ������ ��ǥ �̵�
				if (g_PlayerArray[dwNowDummyID]->m_byNowAction == PLAYER_ACTION_MOVE)
					Action_Move(g_PlayerArray[dwNowDummyID]);

				// -----------------------------
				// ���ݽ��� ��Ŷ �����
				// -----------------------------
				// �ð�, ���� �׼�, ���� ����	
				g_PlayerArray[dwNowDummyID]->m_ullActionTick = GetTickCount64();
				g_PlayerArray[dwNowDummyID]->m_byNowAction = PLAYER_ACTION_IDLE;
				g_PlayerArray[dwNowDummyID]->m_byDir = dfPACKET_MOVE_DIR_RR;

				// ���� ���� ��Ŷ�� �����.
				CProtocolBuff header_Attack(dfNETWORK_PACKET_HEADER_SIZE);
				CProtocolBuff payload_Attack;
				Network_Send_Attack_01(g_PlayerArray[dwNowDummyID]->m_byDir, g_PlayerArray[dwNowDummyID]->m_wNowX, g_PlayerArray[dwNowDummyID]->m_wNowY, &header_Attack, &payload_Attack);

				// �ش� ������ SendBuff�� �ִ´�.
				SendPacket(g_PlayerArray[dwNowDummyID]->m_Session, &header_Attack, &payload_Attack);

			}
			break;

			case AI::ATTACK_02_LEFT:
			{
				// AI ���� �ð� ����
				g_PlayerArray[dwNowDummyID]->m_ullAIStartTick = GetTickCount64();

				// �̵����̾��ٸ� ���巹Ŀ������ ��ǥ �̵�
				if (g_PlayerArray[dwNowDummyID]->m_byNowAction == PLAYER_ACTION_MOVE)
					Action_Move(g_PlayerArray[dwNowDummyID]);

				// -----------------------------
				// ���ݽ��� ��Ŷ �����
				// -----------------------------
				// �ð�, ���� �׼�, ���� ����	
				g_PlayerArray[dwNowDummyID]->m_ullActionTick = GetTickCount64();
				g_PlayerArray[dwNowDummyID]->m_byNowAction = PLAYER_ACTION_IDLE;
				g_PlayerArray[dwNowDummyID]->m_byDir = dfPACKET_MOVE_DIR_LL;

				// ���� ���� ��Ŷ�� �����.
				CProtocolBuff header_Attack(dfNETWORK_PACKET_HEADER_SIZE);
				CProtocolBuff payload_Attack;
				Network_Send_Attack_02(g_PlayerArray[dwNowDummyID]->m_byDir, g_PlayerArray[dwNowDummyID]->m_wNowX, g_PlayerArray[dwNowDummyID]->m_wNowY, &header_Attack, &payload_Attack);

				// �ش� ������ SendBuff�� �ִ´�.
				SendPacket(g_PlayerArray[dwNowDummyID]->m_Session, &header_Attack, &payload_Attack);

			}
			break;

			case AI::ATTACK_02_RIGHT:
			{
				// AI ���� �ð� ����
				g_PlayerArray[dwNowDummyID]->m_ullAIStartTick = GetTickCount64();

				// �̵����̾��ٸ� ���巹Ŀ������ ��ǥ �̵�
				if (g_PlayerArray[dwNowDummyID]->m_byNowAction == PLAYER_ACTION_MOVE)
					Action_Move(g_PlayerArray[dwNowDummyID]);
				
				// -----------------------------
				// ���ݽ��� ��Ŷ �����
				// -----------------------------
				// �ð�, ���� �׼�, ���� ����	
				g_PlayerArray[dwNowDummyID]->m_ullActionTick = GetTickCount64();
				g_PlayerArray[dwNowDummyID]->m_byNowAction = PLAYER_ACTION_IDLE;
				g_PlayerArray[dwNowDummyID]->m_byDir = dfPACKET_MOVE_DIR_RR;

				// ���� ���� ��Ŷ�� �����.
				CProtocolBuff header_Attack(dfNETWORK_PACKET_HEADER_SIZE);
				CProtocolBuff payload_Attack;
				Network_Send_Attack_02(g_PlayerArray[dwNowDummyID]->m_byDir, g_PlayerArray[dwNowDummyID]->m_wNowX, g_PlayerArray[dwNowDummyID]->m_wNowY, &header_Attack, &payload_Attack);

				// �ش� ������ SendBuff�� �ִ´�.
				SendPacket(g_PlayerArray[dwNowDummyID]->m_Session, &header_Attack, &payload_Attack);

			}
			break;

			case AI::ATTACK_03_LEFT:
			{
				// AI ���� �ð� ����
				g_PlayerArray[dwNowDummyID]->m_ullAIStartTick = GetTickCount64();

				// �̵����̾��ٸ� ���巹Ŀ������ ��ǥ �̵�
				if (g_PlayerArray[dwNowDummyID]->m_byNowAction == PLAYER_ACTION_MOVE)
					Action_Move(g_PlayerArray[dwNowDummyID]);
				
				// -----------------------------
				// ���ݽ��� ��Ŷ �����
				// -----------------------------
				// ���� ����
				// �ð�, ���� �׼�, ���� ����	
				g_PlayerArray[dwNowDummyID]->m_ullActionTick = GetTickCount64();
				g_PlayerArray[dwNowDummyID]->m_byNowAction = PLAYER_ACTION_IDLE;
				g_PlayerArray[dwNowDummyID]->m_byDir = dfPACKET_MOVE_DIR_LL;

				// ���� ���� ��Ŷ�� �����.
				CProtocolBuff header_Attack(dfNETWORK_PACKET_HEADER_SIZE);
				CProtocolBuff payload_Attack;
				Network_Send_Attack_03(g_PlayerArray[dwNowDummyID]->m_byDir, g_PlayerArray[dwNowDummyID]->m_wNowX, g_PlayerArray[dwNowDummyID]->m_wNowY, &header_Attack, &payload_Attack);

				// �ش� ������ SendBuff�� �ִ´�.
				SendPacket(g_PlayerArray[dwNowDummyID]->m_Session, &header_Attack, &payload_Attack);

			}
			break;

			case AI::ATTACK_03_RIGHT:
			{
				// AI ���� �ð� ����
				g_PlayerArray[dwNowDummyID]->m_ullAIStartTick = GetTickCount64();

				// �̵����̾��ٸ� ���巹Ŀ������ ��ǥ �̵�
				if (g_PlayerArray[dwNowDummyID]->m_byNowAction == PLAYER_ACTION_MOVE)
					Action_Move(g_PlayerArray[dwNowDummyID]);
				
				// -----------------------------
				// ���ݽ��� ��Ŷ �����
				// -----------------------------
				// �ð�, ���� �׼�, ���� ����	
				g_PlayerArray[dwNowDummyID]->m_ullActionTick = GetTickCount64();
				g_PlayerArray[dwNowDummyID]->m_byNowAction = PLAYER_ACTION_IDLE;
				g_PlayerArray[dwNowDummyID]->m_byDir = dfPACKET_MOVE_DIR_RR;

				// ���� ���� ��Ŷ�� �����.
				CProtocolBuff header_Attack(dfNETWORK_PACKET_HEADER_SIZE);
				CProtocolBuff payload_Attack;
				Network_Send_Attack_03(g_PlayerArray[dwNowDummyID]->m_byDir, g_PlayerArray[dwNowDummyID]->m_wNowX, g_PlayerArray[dwNowDummyID]->m_wNowY, &header_Attack, &payload_Attack);

				// �ش� ������ SendBuff�� �ִ´�.
				SendPacket(g_PlayerArray[dwNowDummyID]->m_Session, &header_Attack, &payload_Attack);
			}
			break;
			
			
			default:
				break;
			}
		}

		Count++;
		dwNowDummyID++;
		if (dwNowDummyID == g_dwJoinUserCount)
			dwNowDummyID = 0;
	}	
}







// ---------------------------------
// Update()���� ���� �׼� ó�� �Լ�
// ---------------------------------
// ���� �̵� �׼� ó��
void Action_Move(stDummyClient* NowPlayer)
{
	// ���巹Ŀ������ �̵�
	int ResultX, ResultY;	
	DeadReckoning(NowPlayer->m_byDir, NowPlayer->m_ullActionTick, NowPlayer->m_wActionX, NowPlayer->m_wActionY, &ResultX, &ResultY);
	
	// �̵� �� ��ǥ ����	
	NowPlayer->m_wNowX = ResultX;
	NowPlayer->m_wNowY = ResultY;
	NowPlayer->m_wActionX = ResultX;
	NowPlayer->m_wActionY = ResultY;	
}









// ---------------------------------
// Recv ó�� �Լ���
// ---------------------------------
// Recv() ó��
bool RecvProc(stSession* Account)
{
	if (Account == nullptr)
		return false;

	////////////////////////////////////////////////////////////////////////
	// ���� ���ۿ� �ִ� ��� recv �����͸�, �ش� ������ recv�����۷� ���ú�.
	////////////////////////////////////////////////////////////////////////
	
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
			_tprintf(_T("RecvProc(). retval �� ������ �ƴ� ������ ��(%d). ���� ����\n"), WSAGetLastError());
			return false;	// FALSE�� ���ϵǸ�, ������ �����.
		}
	}

	// 0�� ���ϵǴ� ���� ��������.
	else if (retval == 0)
		return false;

	g_iRecvByte += retval;

	// 8. ������� ������ Rear�� �̵���Ų��.
	Account->m_RecvBuff.MoveWritePos(retval - 1);


	//////////////////////////////////////////////////
	// �Ϸ� ��Ŷ ó�� �κ�
	// RecvBuff�� ����ִ� ��� �ϼ� ��Ŷ�� ó���Ѵ�.
	//////////////////////////////////////////////////
	while (1)
	{
		char* b = Account->m_RecvBuff.GetReadBufferPtr();
		
		// 1. RecvBuff�� �ּ����� ����� �ִ��� üũ. (���� = ��� ������� ���ų� �ʰ�. ��, �ϴ� �����ŭ�� ũ�Ⱑ �ִ��� üũ)	
		st_PACKET_HEADER HeaderBuff;
		int len = sizeof(HeaderBuff);

		// RecvBuff�� ��� ���� ���� ũ�Ⱑ ��� ũ�⺸�� �۴ٸ�, �Ϸ� ��Ŷ�� ���ٴ� ���̴� while�� ����.
		if (Account->m_RecvBuff.GetUseSize() < len)
			break;

		// 2. ����� Peek���� Ȯ���Ѵ�.		
		// Peek �ȿ�����, ��� �ؼ����� len��ŭ �д´�. ���۰� �� ���� ���� �̻�!
		int PeekSize = Account->m_RecvBuff.Peek((char*)&HeaderBuff, len);

		// Peek()�� ���۰� ������� -1�� ��ȯ�Ѵ�. ���۰� ������� �ϴ� ���´�.
		if (PeekSize == -1)
		{
			_tprintf(_T("RecvProc(). �Ϸ���Ŷ ��� ó�� �� RecvBuff�����. ���� ����\n"));
			return false;	// FALSE�� ���ϵǸ�, ������ �����.
		}

		// 4. ����� Len���� RecvBuff�� ������ ������ ��. (�ϼ� ��Ŷ ������ = ��� ������ + ���̷ε� Size + endCode(1����Ʈ))
		// ��� ���, �ϼ� ��Ŷ ����� �ȵǸ� while�� ����.
		if (Account->m_RecvBuff.GetUseSize() < (len + HeaderBuff.bySize + 1))
			break;

		// 4. ����� code Ȯ��(���������� �� �����Ͱ� �´°�)
		if (HeaderBuff.byCode != dfNETWORK_PACKET_CODE)
		{
			_tprintf(_T("RecvProc(). �Ϸ���Ŷ ��� ó�� �� PacketCodeƲ��. ���� ����\n"));
			return false;	// FALSE�� ���ϵǸ�, ������ �����.
		}

		// 5. RecvBuff���� Peek�ߴ� ����� ����� (�̹� Peek������, �׳� Remove�Ѵ�)
		Account->m_RecvBuff.RemoveData(len);

		// 6. RecvBuff���� ���̷ε� Size ��ŭ ���̷ε� �ӽ� ���۷� �̴´�. (��ť�̴�. Peek �ƴ�)
		DWORD BuffArray = 0;
		CProtocolBuff PayloadBuff;
		DWORD PayloadSize = HeaderBuff.bySize;

		while (PayloadSize > 0)
		{
			int DequeueSize = Account->m_RecvBuff.Dequeue(PayloadBuff.GetBufferPtr() + BuffArray, PayloadSize);

			// Dequeue()�� ���۰� ������� -1�� ��ȯ. ���۰� ������� �ϴ� ���´�.
			if (DequeueSize == -1)
			{
				_tprintf(_T("RecvProc(). �Ϸ���Ŷ ���̷ε� ó�� �� RecvBuff�����. ���� ����\n"));
				return false;	// FALSE�� ���ϵǸ�, ������ �����.
			}

			PayloadSize -= DequeueSize;
			BuffArray += DequeueSize;
		}
		PayloadBuff.MoveWritePos(BuffArray);

		// 7. RecvBuff���� �����ڵ� 1Byte����.	(��ť�̴�. Peek �ƴ�)
		BYTE EndCode;
		DWORD EndCodeSize = 1;
		BuffArray = 0;
		while (EndCodeSize > 0)
		{
			int DequeueSize = Account->m_RecvBuff.Dequeue((char*)&EndCode + BuffArray, 1);
			if (DequeueSize == -1)
			{
				_tprintf(_T("RecvProc(). �Ϸ���Ŷ �����ڵ� ó�� �� RecvBuff�����. ���� ����\n"));
				return FALSE;	// FALSE�� ���ϵǸ�, �ش� ������ ������ �����.
			}

			EndCodeSize -= DequeueSize;
			BuffArray += DequeueSize;
		}

		// 8. �����ڵ� Ȯ��
		if (EndCode != dfNETWORK_PACKET_END)
		{
			_tprintf(_T("RecvProc(). �Ϸ���Ŷ �����ڵ� ó�� �� ������Ŷ �ƴ�. ���� ����\n"));
			return FALSE;	// FALSE�� ���ϵǸ�, �ش� ������ ������ �����.
		}

		// 7. ����� ����ִ� Ÿ�Կ� ���� �б�ó��.
		bool check = PacketProc(HeaderBuff.byType, Account->m_DummyClient, &PayloadBuff);
		if (check == false)
			return false;
	}

	return true;
}

// ��Ŷ ó��
// PacketProc() �Լ����� false�� ���ϵǸ� �ش� ������ ������ �����.
bool PacketProc(WORD PacketType, stDummyClient* Player, CProtocolBuff* Payload)
{
	/*_LOG(dfLOG_LEVEL_DEBUG, L"[DEBUG] PacketRecv [PlayerID : %d / PacketType : %d]\n",
		Player->m_dwClientID, PacketType);*/

	bool check = true;

	try
	{
		switch (PacketType)
		{

		// ��ĳ���� ����
		case dfPACKET_SC_CREATE_MY_CHARACTER:
		{
			check = Network_Recv_CreateMyCharacter(Player, Payload);
			if (check == false)
				return false;	// �ش� ���� ���� ����
		}
		break;

		// �ٸ� ��� ĳ���� ����
		case dfPACKET_SC_CREATE_OTHER_CHARACTER:
		{
			check = Network_Recv_CreateOtherCharacter(Player, Payload);
			if (check == false)
				return false;	// �ش� ���� ���� ����
		}
		break;

		// ĳ���� ����
		case dfPACKET_SC_DELETE_CHARACTER:
		{
			check = Network_Recv_DeleteCharacter(Player, Payload);
			if (check == false)
				return false;	// �ش� ���� ���� ����
		}
		break;

		// �ٸ� ���� �̵� ����
		case dfPACKET_SC_MOVE_START:
		{
			check = Network_Recv_OtherMoveStart(Player, Payload);
			if (check == false)
				return false;	// �ش� ���� ���� ����
		}
		break;

		// �ٸ� ���� �̵� ����
		case dfPACKET_SC_MOVE_STOP:
		{
			check = Network_Recv_OtherMoveStop(Player, Payload);
			if (check == false)
				return false;	// �ش� ���� ���� ����
		}
		break;

		// �ٸ� ���� ���� 1�� ����
		case dfPACKET_SC_ATTACK1:
		{
			check = Network_Recv_OtherAttack_01(Player, Payload);
			if (check == false)
				return false;	// �ش� ���� ���� ����
		}
		break;

		// �ٸ� ���� ���� 2�� ����
		case dfPACKET_SC_ATTACK2:
		{
			check = Network_Recv_OtherAttack_02(Player, Payload);
			if (check == false)
				return false;	// �ش� ���� ���� ����
		}
		break;

		// �ٸ� ���� ���� 3�� ����
		case dfPACKET_SC_ATTACK3:
		{
			check = Network_Recv_OtherAttack_03(Player, Payload);
			if (check == false)
				return false;	// �ش� ���� ���� ����
		}
		break;

		// ������ ��Ŷ ó��
		case dfPACKET_SC_DAMAGE:
		{
			check = Network_Recv_Damage(Player, Payload);
			if (check == false)
				return false;	// �ش� ���� ���� ����
		}
		break;

		// ��ũ ��Ŷ
		case dfPACKET_SC_SYNC:
		{
			check = Network_Recv_Sync(Player, Payload);
			if (check == false)
				return false;	// �ش� ���� ���� ����
		}
		break;

		// ���� ��Ŷ
		case dfPACKET_SC_ECHO:
		{
			check = Network_Recv_Echo(Player, Payload);
			if (check == false)
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

// �� ĳ���� ����
bool Network_Recv_CreateMyCharacter(stDummyClient* Player, CProtocolBuff* Payload)
{
	if (Player == nullptr)
	{
		// ���ڷι��� Player�� nullptr��
		_LOG(dfLOG_LEVEL_DEBUG, L"[DEBUG] ��ĳ���� ����. ���ڷ� ���� Player�� nullptr \n");
		return false;
	}

	// 1. ���̷ε忡�� ���� ������ �̾Ƴ���.
	DWORD	ID;
	BYTE	Direction;
	WORD	X;
	WORD	Y;
	BYTE	HP;

	*Payload >> ID;
	*Payload >> Direction;
	*Payload >> X;
	*Payload >> Y;
	*Payload >> HP;

	// 2. ���� ������ �ð� ����
	Player->m_dwClientID = ID;
	Player->m_byDir = Direction;
	Player->m_wNowX = X;
	Player->m_wNowY = Y;
	Player->m_byHP = HP;
	Player->m_ullAIStartTick = GetTickCount64();

	// Idle ������ ������ �� ó�� ó��
	Player->m_ullActionTick = GetTickCount64(); 
	Player->m_wActionX = X;						
	Player->m_wActionY = Y;

	// 3. ���� �׼��� Idle�̴�
	Player->m_byNowAction = PLAYER_ACTION_IDLE;

	// 4. ������ ���̴� ���� ���� map�� �߰�.
	map_Clientmap.insert(pair<DWORD, stDummyClient*>(ID, Player));

	// 5. �׸��� �迭���� �߰�
	g_PlayerArray[g_PlayerArrayCount] = Player;
	g_PlayerArrayCount++;

	return true;
}

// �ٸ� ��� ĳ���� ����
bool Network_Recv_CreateOtherCharacter(stDummyClient* Player, CProtocolBuff* Payload)
{
	// �ٸ���� ĳ���� ������ ����
	return true;
}

// ĳ���� ����
bool Network_Recv_DeleteCharacter(stDummyClient* Player, CProtocolBuff* Payload)
{
	// �Ұž���

	return true;
}

// �ٸ� ���� �̵� ����
bool Network_Recv_OtherMoveStart(stDummyClient* Player, CProtocolBuff* Payload)
{
	// �ٸ� ���� �̵� ���� ��Ŷ�� ���õȴ�.
	return true;
}

// �ٸ� ���� �̵� ����
bool Network_Recv_OtherMoveStop(stDummyClient* Player, CProtocolBuff* Payload)
{
	// �ٸ� ���� �̵� ���� ��Ŷ�� ���õȴ�.
	return true;
}

// �ٸ� ���� ���� 1�� ����
bool Network_Recv_OtherAttack_01(stDummyClient* Player, CProtocolBuff* Payload)
{
	// ����
	return true;
}

// �ٸ� ���� ���� 2�� ����
bool Network_Recv_OtherAttack_02(stDummyClient* Player, CProtocolBuff* Payload)
{
	// ����
	return true;
}

// �ٸ� ���� ���� 3�� ����
bool Network_Recv_OtherAttack_03(stDummyClient* Player, CProtocolBuff* Payload)
{
	// ����
	return true;
}

// ������ ��Ŷ
bool Network_Recv_Damage(stDummyClient* Player, CProtocolBuff* Payload)
{
	// ����
	return true;
}

// ��ũ ��Ŷ
bool Network_Recv_Sync(stDummyClient* Player, CProtocolBuff* Payload)
{
	// ID, X, Y �˾ƿ���
	DWORD ID;
	WORD X;
	WORD Y;

	*Payload >> ID;
	*Payload >> X;
	*Payload >> Y;

	// ������ �� ��ũ��Ŷ�� �ƴϸ� ����.
	if (Player->m_dwClientID != ID)
	{
		//_LOG(dfLOG_LEVEL_DEBUG, L"[DEBUG] ��ũ ��Ŷ. payload�� ID��, ���ڷ� ���� Player�� ID�� �ٸ� \n");
		return true;
	}

	printf("[X %d, Y %d] -> [X %d, Y %d]\n", Player->m_wNowX, Player->m_wNowY, X, Y);

	// X,Y ����
	Player->m_wNowX = X;
	Player->m_wNowY = Y;
	
	//fputs("Sync!\n", stdout);
	g_dwSyncCount++;

	return true;
}

// ���� ��Ŷ
bool Network_Recv_Echo(stDummyClient* Player, CProtocolBuff* Payload)
{
	g_RTTSendCheck = true;

	// ����
	DWORD Time;
	*Payload >> Time;

	if (g_RTTSendData != Time)
		return false;

	LONGLONG temp = GetTickCount64() - g_RTTSendTime;

	// Max����
	if (g_RTTMax < temp)
		g_RTTMax = temp;

	// �ð� ����
	g_RTTAvgTime += temp;

	// �޾����� ī��Ʈ ����
	g_RTTSendCount++;

	// ���� �ϴ°� ����. ���� RTT �߰�
	return true;
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

// �̵� ���� ��Ŷ �����
bool Network_Send_MoveStart(BYTE Dir, WORD X, WORD Y, CProtocolBuff *header, CProtocolBuff* payload)
{
	// ���̷ε� �����
	*payload << Dir;
	*payload << X;
	*payload << Y;

	// ��� �����
	CreateHeader(header, payload->GetUseSize(), dfPACKET_CS_MOVE_START);

	return true;
}

// �̵� ���� ��Ŷ �����
bool Network_Send_MoveStop(BYTE Dir, WORD X, WORD Y, CProtocolBuff *header, CProtocolBuff* payload)
{
	// ���̷ε� �����
	*payload << Dir;
	*payload << X;
	*payload << Y;

	// ��� �����
	CreateHeader(header, payload->GetUseSize(), dfPACKET_CS_MOVE_STOP);

	return true;
}

// ���� 1�� ���� ��Ŷ �����
bool Network_Send_Attack_01(BYTE Dir, WORD X, WORD Y, CProtocolBuff *header, CProtocolBuff* payload)
{
	// ���̷ε� �����
	*payload << Dir;
	*payload << X;
	*payload << Y;

	// ��� �����
	CreateHeader(header, payload->GetUseSize(), dfPACKET_CS_ATTACK1);

	return true;

}

// ���� 2�� ���� ��Ŷ �����
bool Network_Send_Attack_02(BYTE Dir, WORD X, WORD Y, CProtocolBuff *header, CProtocolBuff* payload)
{
	// ���̷ε� �����
	*payload << Dir;
	*payload << X;
	*payload << Y;

	// ��� �����
	CreateHeader(header, payload->GetUseSize(), dfPACKET_CS_ATTACK2);

	return true;

}

// ���� 3�� ���� ��Ŷ �����
bool Network_Send_Attack_03(BYTE Dir, WORD X, WORD Y, CProtocolBuff *header, CProtocolBuff* payload)
{
	// ���̷ε� �����
	*payload << Dir;
	*payload << X;
	*payload << Y;

	// ��� �����
	CreateHeader(header, payload->GetUseSize(), dfPACKET_CS_ATTACK3);

	return true;

}

// ���� ��Ŷ �����
bool Network_Send_Echo(CProtocolBuff *header, CProtocolBuff* payload)
{
	// DWORD�� �ð� ������.
	DWORD Time = timeGetTime();

	// ����صα�
	g_RTTSendData = Time;

	// ���̷ε忡 �ֱ�
	*payload << Time;

	// ��� �����
	CreateHeader(header, payload->GetUseSize(), dfPACKET_CS_ECHO);

	return true;
}






// ---------------------------------
// Send
// ---------------------------------
// SendBuff�� ������ �ֱ�
bool SendPacket(stSession* Account, CProtocolBuff* header, CProtocolBuff* payload)
{
	char* recvbuff = Account->m_RecvBuff.GetBufferPtr();

	// 1. ���� q�� ��� �ֱ�
	int Size = dfNETWORK_PACKET_HEADER_SIZE;
	DWORD BuffArray = 0;
	while (Size > 0)
	{
		int EnqueueCheck = Account->m_SendBuff.Enqueue(header->GetBufferPtr() + BuffArray, Size);
		if (EnqueueCheck == -1)
		{
			printf("SendPacket() ����ִ� �� ������ ����.(PlayerID : %d)\n", Account->m_DummyClient->m_dwClientID);
			return FALSE;
		}

		Size -= EnqueueCheck;
		BuffArray += EnqueueCheck;
	}

	// 2. ���� q�� ���̷ε� �ֱ�
	int PayloadLen = payload->GetUseSize();

	BuffArray = 0;
	while (PayloadLen > 0)
	{
		int EnqueueCheck = Account->m_SendBuff.Enqueue(payload->GetBufferPtr() + BuffArray, PayloadLen);
		if (EnqueueCheck == -1)
		{
			printf("SendPacket() ���̷ε� �ִ� �� ������ ����.(PlayerID : %d)\n", Account->m_DummyClient->m_dwClientID);
			return FALSE;
		}

		PayloadLen -= EnqueueCheck;
		BuffArray += EnqueueCheck;
	}

	// 3. ���� q�� �����ڵ� �ֱ�
	char EndCode = dfNETWORK_PACKET_END;
	int EnqueueCheck = Account->m_SendBuff.Enqueue(&EndCode, 1);
	if (EnqueueCheck == -1)
	{
		printf("SendPacket() �����ڵ� �ִ� �� ������ ����.(PlayerID : %d)\n", Account->m_DummyClient->m_dwClientID);
		return FALSE;
	}

	return TRUE;
}

// SendBuff�� �����͸� Send�ϱ�
bool SendProc(stSession* Account)
{
	if (Account == nullptr)
		return false;	

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

		g_iSendByte += SendSize;

		// 11. ���� ����� ��������, �� ��ŭ remove
		Account->m_SendBuff.RemoveData(SendSize);
	}

	return true;
}

