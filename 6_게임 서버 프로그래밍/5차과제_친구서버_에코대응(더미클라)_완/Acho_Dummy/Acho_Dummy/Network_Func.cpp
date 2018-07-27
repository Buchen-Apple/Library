#include "stdafx.h"
#include "Network_Func.h"

#include <mmsystem.h>
#pragma comment(lib,"winmm.lib")

#include <time.h>

#include <map>

using namespace std;

// ���� ����ü
struct stDummyClient
{
	SOCKET		m_sock;							// �ش� ������ ����
	CRingBuff	m_RecvBuff;						// ���ú� ����
	CRingBuff	m_SendBuff;						// ���� ����

	UINT64		m_AccountNo;					// ���̿� �����(�α��� ��) ���� ȸ�� �ѹ�. 0�� ���, �α��� �ȵ� ����. ����� ��� ���� �� 0
	DWORD		m_SendTime = 0;					// �̹��� ��Ŷ ���� �ð�

	char*		m_SendString;					// ������ ���� ����Ʈ���ڿ� ����. ������ ������ ���������� üũ�Ѵ�.
	bool		m_bRecvCheck = false;			// ���� Send �� recv�� ��ٸ��� ��� true;	
};


// ��������
map<SOCKET, stDummyClient*> map_DummyClient;	// ���� Ŭ���̾�Ʈ�� �����ϴ� ����ü
int g_iTPSCount;								// ���� TPS�� send Ƚ���� üũ��

// �����Ͻ� ���� �� ������
DWORD g_TotalLaytencyTime = 1;					// 1�� ���� �� �����Ͻ� �ð�
DWORD g_TotalLaytencyCount;						// 1�� ���� �����Ͻ� üũ ī��Ʈ.

// �����Ͻ� ��� üũ ������
DWORD g_TotalLaytencyTimeAvg;					// ������� �� �����Ͻ� �ð� (��� ����)
DWORD g_TotalLaytencyCountAvg;					// ������� g_TotalLaytencyTimeAvg�� ���� ���� Ƚ��. ���� 1�ʸ��� 1 ���� (��� ����)

// �ܺκ���
extern int g_iDummyCount;						// main���� ������ Dummy ī��Ʈ												
extern TCHAR g_tIP[30];							// main���� ������ ������ IP
extern UINT64 g_iErrorCount;					// ���� �߻� Ƚ�� üũ�ϴ� ī��Ʈ. �����ϸ� ���� �ʱ�ȭ���� �ʴ´�.


// ��Ʈ��ũ ���� �Լ� (�����ʱ�ȭ, Ŀ��Ʈ ��..)
bool Network_Init(int* TryCount, int* FailCount, int* SuccessCount)
{
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
		stDummyClient* NewDummy = new stDummyClient;

		// 1. ���� ����
		NewDummy->m_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (NewDummy->m_sock == INVALID_SOCKET)
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

		DWORD dCheck = connect(NewDummy->m_sock, (SOCKADDR*)&clientaddr, sizeof(clientaddr));

		// 3. Ŀ��Ʈ �õ� Ƚ�� ����
		(*TryCount)++;


		// 3. Ŀ��Ʈ ���� ��, ���� ī��Ʈ 1 ����
		if (dCheck == SOCKET_ERROR)
		{
			_tprintf(_T("connect ����. %d (Error : %d)\n"), i + 1, WSAGetLastError());
			(*FailCount)++;
		}

		// Ŀ��Ʈ ���� ��, ���� ī��Ʈ 1����. �׸��� �ش� ���̸� map�� �߰�
		else
		{
			_tprintf(_T("connect ����. %d\n"), i + 1);
			map_DummyClient.insert(pair<SOCKET, stDummyClient*>(NewDummy->m_sock, NewDummy));

			(*SuccessCount)++;
		}
	}

	return true;
}

// ��Ʈ��ũ ���μ���
// ���⼭ false�� ��ȯ�Ǹ�, ���α׷��� ����ȴ�.
bool Network_Process()
{
	//  ��ſ� ����� ����
	FD_SET rset;
	FD_SET wset;
	TIMEVAL tval;
	tval.tv_sec = 0;
	tval.tv_usec = 0;

	map <SOCKET, stDummyClient*>::iterator itor;
	map <SOCKET, stDummyClient*>::iterator Enditor;
	itor = map_DummyClient.begin();

	DWORD dwFDCount = 0;

	// ���� �ʱ�ȭ
	rset.fd_count = 0;
	wset.fd_count = 0;
	SOCKET* NowSock[FD_SETSIZE];

	while (1)
	{
		Enditor = map_DummyClient.end();

		// ��� ����� ��ȸ�ϸ�, �ش� ���̸� �б� �°� ���� �¿� �ִ´�.
		// �ִٰ� 64���� �ǰų�, end�� �����ϸ� break
		while (itor != Enditor)
		{
			// �ش� ���̿��� ���� �����Ͱ� �ִ��� üũ�ϱ� ����, ��� ���̸� rset�� ���� ����
			FD_SET(itor->second->m_sock, &rset);
			NowSock[dwFDCount] = &itor->second->m_sock;

			// ����, �ش� ������ SendBuff�� ���� ������, wset���� ���� ����.
			if (itor->second->m_SendBuff.GetUseSize() != 0)
				FD_SET(itor->second->m_sock, &wset);

			// 64�� �� á����, ���� �����ߴ��� üũ		
			++itor;
			if (dwFDCount + 1 == FD_SETSIZE || itor == Enditor)
				break;

			++dwFDCount;
		}

		// Select()
		DWORD dCheck = select(0, &rset, &wset, 0, &tval);

		// Select()��� ó��
		if (dCheck == SOCKET_ERROR)
		{
			_tprintf(_T("select ����(%d)\n"), WSAGetLastError());
			return true;
		}

		// select�� ���� 0���� ũ�ٸ� ���� �Ұ� �ִٴ� ���̴� ���� ����
		else if (dCheck > 0)
		{
			for (DWORD i = 0; i <= dwFDCount; ++i)
			{
				// ���� �� ó��
				if (FD_ISSET(*NowSock[i], &wset))
				{
					stDummyClient* NowDummy = DummySearch(*NowSock[i]);

					// Send() ó��
					// ����, SendProc()�Լ��� false�� ���ϵȴٸ�, �ش� ���� ���� ����
					if (SendProc(&NowDummy->m_SendBuff, NowDummy->m_sock) == false)
					{
						Disconnect(*NowSock[i]);
						continue;
					}
				}

				// �б� �� ó��
				if (FD_ISSET(*NowSock[i], &rset))
				{
					stDummyClient* NowDummy = DummySearch(*NowSock[i]);						

					// Recv() ó��
					// ����, RecvProc()�Լ��� false�� ���ϵȴٸ�, �ش� ���� ���� ����
					if (RecvProc(NowDummy) == false)
						Disconnect(*NowSock[i]);
				}				
			}
		}

		// ����, ��� ���̿� ���� Selectó���� ��������, �̹� �Լ��� ���⼭ ����.
		if (itor == Enditor)
			break;

		// select �غ�	
		rset.fd_count = 0;
		wset.fd_count = 0;
		dwFDCount = 0;
	}

	return true;
}

// ���ڷ� ���� Socket ���� �������� [���� ���]���� [���̸� ��󳽴�].(�˻�)
// ���� ��, �ش� ������ ���� ����ü�� �ּҸ� ����
// ���� �� nullptr ����
stDummyClient* DummySearch(SOCKET sock)
{
	stDummyClient* NowDummy = nullptr;
	map <SOCKET, stDummyClient*>::iterator iter;

	iter = map_DummyClient.find(sock);
	if (iter == map_DummyClient.end())
		return nullptr;

	NowDummy = iter->second;
	return NowDummy;
}

// Disconnect ó��
void Disconnect(SOCKET sock)
{
	// ���ڷ� ���� sock�� �������� ���� ��Ͽ��� ���� �˾ƿ���
	stDummyClient* NowDummy = DummySearch(sock);

	// ���ܻ��� üũ
	// 1. �ش� ���̰� ���� ���ΰ�.
	// NowUser�� ���� nullptr�̸�, �ش� ���̸� ��ã�� ��. false�� �����Ѵ�.
	if (NowDummy == nullptr)
	{
		_tprintf(_T("Disconnect(). ���� ������ ���� ���̸� �����Ϸ��� ��.\n"));
		return;
	}

	// 2. �ش� Accept ������ AcceptList���� ����
	map_DummyClient.erase(sock);

	// 3. ���� ������ ������ ip, port, AccountNo, socket ���	
	_tprintf(_T("Disconnect : AccountNo : %lld / Socket : %lld]\n"), NowDummy->m_AccountNo, NowDummy->m_sock);

	// 4. �ش� ������ ���� close
	closesocket(NowDummy->m_sock);

	// 5. �ش� Accept���� ���� ����
	delete NowDummy;
}




///////////////////////////////
// ȭ�� ��¿� �� ��ȯ �Լ�
///////////////////////////////
// ��� �����Ͻ� ��ȯ �Լ�
DWORD GetAvgLaytency()
{
	//////////////��� �� ��ȯ�ϴ� �ڵ� //////////////////		
	// 1. ��� ���	
	
	// �̹� ������(1��)�� �����Ͻ� �� ���ϱ�
	g_TotalLaytencyTimeAvg += (g_TotalLaytencyTime / g_TotalLaytencyCount) ;

	// g_TotalLaytencyTimeAvg�� ���� ���� Ƚ�� ����
	g_TotalLaytencyCountAvg++;

	// ��� ���
	DWORD LaytencyAvg = g_TotalLaytencyTimeAvg / g_TotalLaytencyCountAvg;
	
	// 2. ���� ������ ���� �� �ʱ�ȭ
	g_TotalLaytencyTime = 0;
	g_TotalLaytencyCount = 0;

	return LaytencyAvg;	
	
	
	
	


	//////////////���� �� ��ȯ�ϴ� �ڵ� //////////////////

	/*
	// 1. �̹� ������(1��)�� �����Ͻ� �� ���ϱ�
	// ms�����̱� ������ 1000�Ѵ�.
	if (g_TotalLaytencyCount == 0)
		g_TotalLaytencyCount = 1;
	DWORD NowLaytency = (g_TotalLaytencyTime / g_TotalLaytencyCount) / 1000;

	// 2. ���� ������ ���� �� �ʱ�ȭ
	g_TotalLaytencyTime = 1;
	g_TotalLaytencyCount = 0;

	return NowLaytency;
	*/
	
}


// TPS ��ȯ �Լ�
int GetTPS()
{
	int temp = g_iTPSCount;
	g_TotalLaytencyCount = g_iTPSCount;
	g_iTPSCount = 0;

	return temp;
}


///////////////////////////////
// ������ �� ��.
///////////////////////////////
// ���� ���ڿ� ���� �� SendBuff�� �ִ´�.
void DummyWork_CreateString()
{
	map<SOCKET, stDummyClient*>::iterator itor;

	// ó������ ������ ��ȸ. (�������ѵ�.. �� �� ��� �ʿ�)
	for (itor = map_DummyClient.begin(); itor != map_DummyClient.end(); ++itor)
	{
		// ���ú� ������� ���̶��, ���ڿ� ���� ���Ѵ�.
		if (itor->second->m_bRecvCheck == true)
			continue;

		// 1. N���� ������ ���� ���� �� ���ڿ��� ����.
		WORD Size = rand() % 1000;
		char* SendByte = new char[Size];
		for (int i = 0; i<Size; ++i)
			SendByte[i] = rand() % 128;

		//  2. �ش� ������ m_SendString�� ����
		itor->second->m_SendString = new char[Size];
		memcpy(itor->second->m_SendString, SendByte, Size);

		// 3. ���̷ε� ����		
		CProtocolBuff payload;
		payload << Size;
		memcpy(payload.GetBufferPtr() + 2, SendByte, Size);
		payload.MoveWritePos(Size);

		// 4. ��� ����
		CProtocolBuff header(dfHEADER_SIZE);
		BYTE byCode = dfPACKET_CODE;
		WORD MsgType = df_REQ_STRESS_ECHO;
		WORD PayloadSize = Size + 2;

		header << byCode;
		header << MsgType;
		header << PayloadSize;

		// 5. ���� ��Ŷ�� �ش� ������, SendBuff�� �ֱ�
		SendPacket(itor->second, &header, &payload);

		// �ش� ���̸� "recv ������"�� ����
		itor->second->m_bRecvCheck = true;
	}
}




/////////////////////////
// Recv ó�� �Լ���
/////////////////////////
bool RecvProc(stDummyClient* NowDummy)
{
	////////////////////////////////////////////////////////////////////////
	// ���� ���ۿ� �ִ� ��� recv �����͸�, �ش� ������ recv�����۷� ���ú�.
	////////////////////////////////////////////////////////////////////////

	// 1. recv ������ ������ ������.
	char* recvbuff = NowDummy->m_RecvBuff.GetBufferPtr();

	// 2. �� ���� �� �� �ִ� �������� ���� ���� ���
	int Size = NowDummy->m_RecvBuff.GetNotBrokenPutSize();

	// 3. �� ���� �� �� �ִ� ������ 0�̶��
	if (Size == 0)
	{
		// rear 1ĭ �̵�
		NowDummy->m_RecvBuff.MoveWritePos(1);

		// �׸��� �� ���� �� �� �ִ� ��ġ �ٽ� �˱�.
		Size = NowDummy->m_RecvBuff.GetNotBrokenPutSize();
	}

	// 4. �װ� �ƴ϶��, 1ĭ �̵��� �ϱ�. 		
	else
	{
		// rear 1ĭ �̵�
		NowDummy->m_RecvBuff.MoveWritePos(1);
	}

	// 5. 1ĭ �̵��� rear ������
	int* rear = (int*)NowDummy->m_RecvBuff.GetWriteBufferPtr();

	// 6. recv()
	int retval = recv(NowDummy->m_sock, &recvbuff[*rear], Size, 0);			
	
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
		else if (WSAGetLastError() == WSAECONNRESET || WSAGetLastError() == WSAECONNABORTED)
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

	// 8. ������� ������ Rear�� �̵���Ų��.
	NowDummy->m_RecvBuff.MoveWritePos(retval - 1);



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
		if (NowDummy->m_RecvBuff.GetUseSize() < len)
			break;

		// 2. ����� Peek���� Ȯ���Ѵ�.		
		// Peek �ȿ�����, ��� �ؼ����� len��ŭ �д´�. ���۰� �� ���� ���� �̻�!
		int PeekSize = NowDummy->m_RecvBuff.Peek((char*)&HeaderBuff, len);

		// Peek()�� ���۰� ������� -1�� ��ȯ�Ѵ�. ���۰� ������� �ϴ� ���´�.
		if (PeekSize == -1)
		{
			_tprintf(_T("RecvProc(). �Ϸ���Ŷ ��� ó�� �� RecvBuff�����. ���� ����\n"));
			return false;	// FALSE�� ���ϵǸ�, ������ �����.
		}

		// 3. ����� code Ȯ��(���������� �� �����Ͱ� �´°�)
		if (HeaderBuff.byCode != dfPACKET_CODE)
		{
			_tprintf(_T("RecvProc(). �Ϸ���Ŷ ��� ó�� �� PacketCodeƲ��. ���� ����\n"));
			return false;	// FALSE�� ���ϵǸ�, ������ �����.
		}

		// 4. ����� Len���� RecvBuff�� ������ ������ ��. (�ϼ� ��Ŷ ������ = ��� ������ + ���̷ε� Size )
		// ��� ���, �ϼ� ��Ŷ ����� �ȵǸ� while�� ����.
		if (NowDummy->m_RecvBuff.GetUseSize() < (len + HeaderBuff.wPayloadSize))
			break;

		// 5. RecvBuff���� Peek�ߴ� ����� ����� (�̹� Peek������, �׳� Remove�Ѵ�)
		NowDummy->m_RecvBuff.RemoveData(len);

		// 6. RecvBuff���� ���̷ε� Size ��ŭ ���̷ε� �ӽ� ���۷� �̴´�. (��ť�̴�. Peek �ƴ�)
		DWORD BuffArray = 0;
		CProtocolBuff PayloadBuff;
		DWORD PayloadSize = HeaderBuff.wPayloadSize;

		while (PayloadSize > 0)
		{
			int DequeueSize = NowDummy->m_RecvBuff.Dequeue(PayloadBuff.GetBufferPtr() + BuffArray, PayloadSize);

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

		// 7. ����� ����ִ� Ÿ�Կ� ���� �б�ó��. (������ ����ó�� �Լ� ȣ��)
		bool check = Network_Res_Acho(HeaderBuff.wMsgType, NowDummy, &PayloadBuff);
		if (check == false)
			return false;
	}

	return true;
}

// "���� ��Ŷ" ó��
bool Network_Res_Acho(WORD Type, stDummyClient* NowDummy, CProtocolBuff* payload)
{
	/////// ���� ���� �ϼ��� �����͸� �޾Ҵٴ� ���̴� �����Ͻ� üũ ///////////
	g_TotalLaytencyTime += timeGetTime() - NowDummy->m_SendTime;
	NowDummy->m_SendTime = 0;

	// �ϴ�, recv �����¸� false�� ����. ���� ������ send�� ����
	NowDummy->m_bRecvCheck = false;

	// 1. ��Ŷ Ÿ�� üũ
	// ���ڰ� �ƴ϶��,
	if (Type != df_RES_STRESS_ECHO)
	{
		// Error �޽��� ǥ��
		fputs("Type Error!!!\n", stdout);

		// ����ī��Ʈ �߰�
		g_iErrorCount++;

		return true;
	}

	// 2. ���ڿ� üũ
	// �ϴ� ���ڿ� ������ ������ (����Ʈ ����)
	WORD wStringSize;
	*payload >> wStringSize;

	// ������ ��ŭ ���ڿ� ��������
	char* TestString = new char[wStringSize];
	memcpy(TestString, payload->GetBufferPtr() + 2, wStringSize);

	// ���� ���´� ���ڿ��� �ƴ϶��
	if (memcmp(NowDummy->m_SendString, TestString, wStringSize) != 0)
	{
		// Error �޽��� ǥ��
		fputs("Error Data!!!\n", stdout);

		// ����ī��Ʈ �߰�
		g_iErrorCount++;
	}

	// 3. ������ ���ڿ� ��������
	delete NowDummy->m_SendString;
	delete TestString;

	return true;
}




/////////////////////////
// Send ó��
/////////////////////////
// Send���ۿ� ������ �ֱ�
bool SendPacket(stDummyClient* NowDummy, CProtocolBuff* headerBuff, CProtocolBuff* payloadBuff)
{
	// 1. ���� q�� ��� �ֱ�
	int Size = headerBuff->GetUseSize();
	DWORD BuffArray = 0;
	int a = 0;
	while (Size > 0)
	{
		int EnqueueCheck = NowDummy->m_SendBuff.Enqueue(headerBuff->GetBufferPtr() + BuffArray, Size);
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
		int EnqueueCheck = NowDummy->m_SendBuff.Enqueue(payloadBuff->GetBufferPtr() + BuffArray, PayloadLen);
		if (EnqueueCheck == -1)
		{
			_tprintf(_T("SendPacket(). ���̷ε� �ִ� �� ����ť ����. ���� ����\n"));
			return false;
		}

		PayloadLen -= EnqueueCheck;
		BuffArray += EnqueueCheck;
	}

	/////// ���� ť�� �ְ� �� �ٷ� �����Ͻ� �ð� ���� ///////////
	NowDummy->m_SendTime = timeGetTime();
	g_iTPSCount++;
	return true;
}

// Send������ �����͸� Send�ϱ�
bool SendProc(CRingBuff* SendBuff, SOCKET sock)
{
	// 2. SendBuff�� ���� �����Ͱ� �ִ��� Ȯ��.
	if (SendBuff->GetUseSize() == 0)
		return true;

	// 3. ���� �������� �������� ���� ������
	char* sendbuff = SendBuff->GetBufferPtr();

	// 4. SendBuff�� �� �������� or Send����(����)���� ��� send
	while (1)
	{
		// 5. SendBuff�� �����Ͱ� �ִ��� Ȯ��(���� üũ��)
		if (SendBuff->GetUseSize() == 0)
			break;

		// 6. �� ���� ���� �� �ִ� �������� ���� ���� ���
		int Size = SendBuff->GetNotBrokenGetSize();

		// 7. ���� ������ 0�̶�� 1�� ����. send() �� 1����Ʈ�� �õ��ϵ��� �ϱ� ���ؼ�.
		// �׷��� send()�� ������ �ߴ��� Ȯ�� ����
		if (Size == 0)
			Size = 1;

		// 8. front ������ ����
		int *front = (int*)SendBuff->GetReadBufferPtr();

		// 9. front�� 1ĭ �� index�� �˾ƿ���.
		int BuffArrayCheck = SendBuff->NextIndex(*front, 1);		

		// 10. Send()
		int SendSize = send(sock, &sendbuff[BuffArrayCheck], Size, 0);

		// 11. ���� üũ. �����̸� �� �̻� �������� while�� ����. ���� Select�� �ٽ� ����
		if (SendSize == SOCKET_ERROR)
		{
			if (WSAGetLastError() == WSAEWOULDBLOCK)
				break;

			// 10053, 10054 �Ѵ� ��� ������ ������ ����
			// WSAECONNABORTED(10053) :  �������ݻ��� ������ Ÿ�Ӿƿ��� ���� ����(����ȸ��. virtual circle)�� ������ ���
			// WSAECONNRESET(10054) : ���� ȣ��Ʈ�� ������ ����.
			// �� ���� �׳� return false�� ���� ����� ������ ���´�.
			else if (WSAGetLastError() == WSAECONNRESET || WSAGetLastError() == WSAECONNABORTED)
				return false;

			else
			{
				_tprintf(_T("SendProc(). Send�� ���� �߻�. ���� ����\n"));
				return false;
			}


		}

		// 12. ���� ����� ��������, �� ��ŭ remove
		SendBuff->RemoveData(SendSize);
	}

	return true;
}
