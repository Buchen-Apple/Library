#include "pch.h"

#include <Ws2tcpip.h>
#include <mmsystem.h>
#pragma comment(lib,"winmm.lib")

#include <strsafe.h>

#include "rapidjson\document.h"
#include "rapidjson\writer.h"
#include "rapidjson\stringbuffer.h"

#include "Network_Func.h"
#include "Protocol_Set/CommonProtocol_2.h"
#include "Parser/Parser_Class.h"

using namespace rapidjson;

// ��ū
#define TOKEN_T	L"sakljflksajfklsafnkscksjlfasf12345adflsadlfjkefafkj%kldjsklf121"
#define TOKEN_C	"sakljflksajfklsafnkscksjlfasf12345adflsadlfjkefafkj%kldjsklf121"

// VerCode
#define VERCODE 123456

// Disconenct Ȯ��
#define DISCONNECT_RATE 20


namespace Library_Jingyu
{
	// ����ȭ ���� 1���� ũ��
	// �� ������ ���� ������ �����ؾ� ��.
	LONG g_lNET_BUFF_SIZE = 512;
}

// ------------------------------------
// �ڷᱸ�� ������ �Լ�
// ------------------------------------

// ���� ���� �ڷᱸ���� ���� �߰�
// ���� umap���� ������
// 
// Parameter : AccountNo, stDummy*
// return : �߰� ���� ��, true
//		  : AccountNo�� �ߺ��� ��(�̹� �������� ����) false
bool cMatchTestDummy::InsertDummyFunc(UINT64 AccountNo, stDummy* insertDummy)
{
	// 1. umap�� �߰�		
	auto ret = m_uamp_Dummy.insert(make_pair(AccountNo, insertDummy));

	// 2. �ߺ��� Ű�� �� false ����.
	if (ret.second == false)
		return false;

	// 3. �ƴϸ� true ����
	return true;
}

// ���� ���� �ڷᱸ������, ���� �˻�
// ���� map���� ������
// 
// Parameter : AccountNo
// return : �˻� ���� ��, stDummy*
//		  : �˻� ���� �� nullptr
cMatchTestDummy::stDummy* cMatchTestDummy::FindDummyFunc(UINT64 AccountNo)
{
	// 1. umap���� �˻�
	auto FindPlayer = m_uamp_Dummy.find(AccountNo);

	// 2. �˻� ���� �� nullptr ����
	if (FindPlayer == m_uamp_Dummy.end())
		return nullptr;

	// 3. �˻� ���� ��, ã�� stPlayer* ����
	return FindPlayer->second;
}



// ------------------------------------
// SOCKET ����, ���� �ڷᱸ�� ������ �Լ�
// ------------------------------------

// ���� ���� �ڷᱸ���� ���� �߰�
// ���� umap���� ������
// 
// Parameter : SOCKET, stDummy*
// return : �߰� ���� ��, true
//		  : SOCKET�� �ߺ��� ��(�̹� �������� ����) false
bool cMatchTestDummy::InsertDummyFunc_SOCKET(SOCKET sock, stDummy* insertDummy)
{
	// 1. umap�� �߰�		
	auto ret = m_uamp_Socket_and_AccountNo.insert(make_pair(sock, insertDummy));

	// 2. �ߺ��� Ű�� �� false ����.
	if (ret.second == false)
		return false;

	// 3. �ƴϸ� true ����
	return true;
}

// ���� ���� �ڷᱸ������, ���� �˻�
// ���� map���� ������
// 
// Parameter : SOCKET
// return : �˻� ���� ��, stDummy*
//		  : �˻� ���� �� nullptr
cMatchTestDummy::stDummy* cMatchTestDummy::FindDummyFunc_SOCKET(SOCKET sock)
{
	// 1. umap���� �˻�
	auto FindPlayer = m_uamp_Socket_and_AccountNo.find(sock);

	// 2. �˻� ���� �� nullptr ����
	if (FindPlayer == m_uamp_Socket_and_AccountNo.end())
		return nullptr;

	// 3. �˻� ���� ��, ã�� stPlayer* ����
	return FindPlayer->second;

}

//���� ���� �ڷᱸ������, ���� ���� (�˻� �� ����)
// ���� umap���� ������
// 
// Parameter : SOCKET
// return : ���� ��, ���ŵ� ������ stDummy*
//		  : �˻� ���� ��(���������� ���� ����) nullptr
cMatchTestDummy::stDummy* cMatchTestDummy::EraseDummyFunc_SOCKET(SOCKET socket)
{
	// 1. umap���� ���� �˻�
	auto FindPlayer = m_uamp_Socket_and_AccountNo.find(socket);
	if (FindPlayer == m_uamp_Socket_and_AccountNo.end())
	{
		return nullptr;
	}

	// 2. �����Ѵٸ�, ������ �� �޾Ƶα�
	stDummy* ret = FindPlayer->second;

	// 3. �ʿ��� ����
	m_uamp_Socket_and_AccountNo.erase(FindPlayer);
	return ret;
}






// ------------------------------------
// ���ο����� ����ϴ� �Լ�
// ------------------------------------


// ��ġ����ŷ ������ Connect
//
// Parameter : ����
// return : ����
void cMatchTestDummy::MatchConnect()
{
	// ���÷� �޾Ƶα�
	int Count = m_iDummyCount;
	UINT64 TempStartNo = m_ui64StartAccountNo;

	// ���� �� ��ŭ ���鼭, Connect �õ�
	int Temp = 0;
	while (1)
	{
		// 1. ���� �˻�
		stDummy* NowDummy = FindDummyFunc(TempStartNo);
		if (NowDummy == nullptr)
			m_Dump->Crash();

		// 2. Connect ���� ���� ���̸� Connect �õ�
		if (NowDummy->m_bMatchConnect == false)
		{
			// ���� ����
			NowDummy->m_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
			if (NowDummy->m_sock == INVALID_SOCKET)
				m_Dump->Crash();

			// ���� �������� ����
			ULONG on = 1;
			if (ioctlsocket(NowDummy->m_sock, FIONBIO, &on) == SOCKET_ERROR)
				m_Dump->Crash();

			SOCKADDR_IN clientaddr;
			ZeroMemory(&clientaddr, sizeof(clientaddr));
			clientaddr.sin_family = AF_INET;
			clientaddr.sin_port = htons(NowDummy->m_iPort);
			InetPton(AF_INET, NowDummy->m_tcServerIP, &clientaddr.sin_addr.S_un.S_addr);

			// connect �õ�
			int iReConnectCount = 0;
			while (1)
			{
				connect(NowDummy->m_sock, (SOCKADDR*)&clientaddr, sizeof(clientaddr));

				DWORD Check = WSAGetLastError();

				// �̹� ����� ���
				if (Check == WSAEISCONN)
				{
					break;
				}

				// ������ �� ���
				else if (Check == WSAEWOULDBLOCK)
				{
					// �����, ���ܼ�, ����
					FD_SET wset;
					FD_SET exset;
					wset.fd_count = 0;
					exset.fd_count = 0;

					wset.fd_array[wset.fd_count] = NowDummy->m_sock;
					wset.fd_count++;

					exset.fd_array[exset.fd_count] = NowDummy->m_sock;
					exset.fd_count++;

					// timeval ����. 500m/s  ���
					TIMEVAL tval;
					tval.tv_sec = 0;
					tval.tv_usec = 500000;

					// Select()
					DWORD retval = select(0, 0, &wset, &exset, &tval);


					// ���� �߻����� üũ
					if (retval == SOCKET_ERROR)
					{
						printf("Select error..(%d)\n", WSAGetLastError());

						closesocket(NowDummy->m_sock);
						m_Dump->Crash();
					}

					// Ÿ�Ӿƿ� üũ
					else if (retval == 0)
					{
						printf("Select Timeout..\n");

						// 5ȸ ��õ��Ѵ�.
						iReConnectCount++;
						if (iReConnectCount == 5)
						{
							closesocket(NowDummy->m_sock);
							m_Dump->Crash();
						}

						continue;
					}

					// ������ �ִٸ�, ���ܼ¿� ������ �Դ��� üũ
					else if (exset.fd_count > 0)
					{
						//���ܼ� �����̸� ������ ��.
						closesocket(NowDummy->m_sock);
						m_Dump->Crash();
					}

					// ����¿� ������ �Դ��� üũ
					else if (wset.fd_count > 0)
					{
						break;
					}

					// ������� ���� ���� �� ����..
					else
					{
						printf("Select ---> UnknownError..(retval %d, LastError : %d)\n", retval, WSAGetLastError());

						closesocket(NowDummy->m_sock);
						m_Dump->Crash();
					}

				}
			}

			// �ٽ� ������ ������� ����
			on = 0;
			if (ioctlsocket(NowDummy->m_sock, FIONBIO, &on) == SOCKET_ERROR)
			{
				closesocket(NowDummy->m_sock);
				m_Dump->Crash();
			}

			// ���� ���� �� flag ����
			NowDummy->m_bMatchConnect = true;

			// SOCKET�� ���� �ڷᱸ���� �߰�
			InsertDummyFunc_SOCKET(NowDummy->m_sock, NowDummy);
		}

		// 3. ������ üũ
		++Temp;
		++TempStartNo;
		if (Temp == Count)
			break;
	}		
}

// ��� ������ SendQ�� �α��� ��Ŷ Enqueue
//
// Parameter : ����
// return : ����
void cMatchTestDummy::MatchLogin()
{
	// ���÷� �޾Ƶα�
	int Count = m_iDummyCount;
	UINT64 TempStartNo = m_ui64StartAccountNo;

	// Connect�� �� ������ ���� �α��� ��Ŷ ����
	int Temp = 0;
	while (1)
	{
		// 1. ���� �˻�
		stDummy* NowDummy = FindDummyFunc(TempStartNo);
		if (NowDummy == nullptr)
			m_Dump->Crash();

		// 2. Connect �Ǿ���, Login �ȵ�����, �α��� ��Ŷ�� �Ⱥ��� ���� ���
		if (NowDummy->m_bMatchConnect == true && 
			NowDummy->m_bMatchLogin == false && 
			NowDummy->m_bLoginPacketSend == false)
		{
			// SendBuff �Ҵ�ޱ�
			CProtocolBuff_Net* SendBuff = CProtocolBuff_Net::Alloc();

			// ���� ��Ŷ ����
			WORD Type = en_PACKET_CS_MATCH_REQ_LOGIN;
			UINT VerCode = VERCODE;

			SendBuff->PutData((char*)&Type, 2);
			SendBuff->PutData((char*)&NowDummy->m_ui64AccountNo, 8);
			SendBuff->PutData(NowDummy->m_cToekn, 64);
			SendBuff->PutData((char*)&VerCode, 4);

			// SendQ�� �ֱ�
			SendPacket(NowDummy, SendBuff);

			// �α��� ��Ŷ ���� ���·� ����
			NowDummy->m_bLoginPacketSend = true;

			// ����ȭ ���� ���۷��� ī��Ʈ 1 ����. 0 �Ǹ� �޸�Ǯ�� ��ȯ
			CProtocolBuff_Net::Free(SendBuff);
		}

		// 3. ������ üũ
		++Temp;
		++TempStartNo;
		if (Temp == Count)
			break;
	}

}

// Select ó��
//
// Parameter : ����
// return : ����
void cMatchTestDummy::SelectFunc()
{
	// ��ſ� ����� ����
	FD_SET rset;
	FD_SET wset;

	unordered_map <UINT64, stDummy*>::iterator itor;
	unordered_map <UINT64, stDummy*>::iterator enditor;
	TIMEVAL tval;
	tval.tv_sec = 0;
	tval.tv_usec = 0;

	itor = m_uamp_Dummy.begin();

	// ���� ���� ����. 
	rset.fd_count = 0;
	wset.fd_count = 0;

	while (1)
	{
		enditor = m_uamp_Dummy.end();

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
			if (WSAGetLastError() != WSAEINVAL && rset.fd_count != 0 && wset.fd_count != 0)
				_tprintf(_T("select ����(%d)\n"), WSAGetLastError());

			m_Dump->Crash();
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
					stDummy* NowDummy = FindDummyFunc_SOCKET(rset.fd_array[rsetCount]);

					// Recv() ó��
					// ����, RecvProc()�Լ��� false�� ���ϵȴٸ�, �ش� ���� ���� ����
					if (RecvProc(NowDummy) == false)
						Disconnect(NowDummy);

					rsetCount++;
				}

				if (wsetCount < wset.fd_count)
				{
					stDummy* NowDummy = FindDummyFunc_SOCKET(wset.fd_array[wsetCount]);

					// Send() ó��
					// ����, SendProc()�Լ��� false�� ���ϵȴٸ�, �ش� ���� ���� ����
					if (SendProc(NowDummy) == false)
						Disconnect(NowDummy);

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

}

// Match ������ Disconnect. (Ȯ���� ����)
// 
// Parameter : ����
// return : ����
void cMatchTestDummy::MatchDisconenct()
{
	// ���÷� �޾Ƶα�
	int Count = m_iDummyCount;
	UINT64 TempStartNo = m_ui64StartAccountNo;

	// Connect�� �� ������ ���� Ȯ���� ���� Disconenct()
	int Temp = 0;
	while (1)
	{
		// 1. ���� �˻�
		stDummy* NowDummy = FindDummyFunc(TempStartNo);
		if (NowDummy == nullptr)
			m_Dump->Crash();

		// 2. ��ġ����ŷ ������ �α��� �� ������ ���� ���� ����
		if (NowDummy->m_bMatchLogin == true)
		{
			// 1. Ȯ�� ���ϱ�
			int random = (rand() % 100) + 1;	// 1 ~ 100������ �� �� �ϳ� ����

			// random ���� ������ Ȯ������ ������ üũ
			if (random <= DISCONNECT_RATE)	
			{
				// ���� ���� Ȯ���� ��÷�� ��. Disconenct �Ѵ�.
				Disconnect(NowDummy);
			}
		}

		// 3. ������ üũ
		++Temp;
		++TempStartNo;
		if (Temp == Count)
			break;
	}
}


// �ش� ������ SendQ�� ��Ŷ �ֱ�
//
// Parameter : stDummy*, CProtocolBuff_Net*
// return : ���� �� true
//		  : ���� �� false
bool cMatchTestDummy::SendPacket(stDummy* NowDummy, CProtocolBuff_Net* payload)
{
	// 1. ����� �־� ��Ŷ �ϼ�
	payload->Encode(m_bCode, m_bXORCode_1, m_bXORCode_2);

	// 2. ��ť. ���̷ε��� �����͸� Enqueue�Ѵ�.
	NowDummy->m_SendBuff.Enqueue(payload->GetBufferPtr(), payload->GetUseSize());	

	return true;
}


// ������ ���ڷ� ���� ���� 1��ü�� ������ ���� �Լ�.
//
// Parameter : stDummy*
// returb : ����
void cMatchTestDummy::Disconnect(stDummy* NowDummy)
{
	SOCKET TempSocket = NowDummy->m_sock;

	// SOCKET ���� �ڷᱸ������ Erase
	// SOCKET�� ���� ������ �� ���� ����Ǳ� ������ ������� �Ѵ�.
	// AccountNo�� �����Ǵ� �ڷᱸ�������� �������� ����.
	if(EraseDummyFunc_SOCKET(TempSocket) == nullptr)
		m_Dump->Crash();

	// ���� �ʱ�ȭ	
	NowDummy->Reset();

	// �ش� ������ ���� close	
	closesocket(TempSocket);
}




// ------------------------------------
// ��Ʈ��ũ ó�� �Լ�
// ------------------------------------

// Recv() ó��
//
// Parameter : stDummy*
// return : ������ ���ܾ� �ϴ� ������, false ����
bool cMatchTestDummy::RecvProc(stDummy* NowDummy)
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
		if (WSAGetLastError() == WSAECONNRESET || WSAGetLastError() == WSAECONNABORTED)
		{
			m_Dump->Crash();
			return false;
		}

		// WSAEWOULDBLOCK������ �ƴϸ� ���� �̻��� ���̴� ���� ����
		else
		{
			m_Dump->Crash();
			return false;	// FALSE�� ���ϵǸ�, ������ �����.
		}
	}

	// 0�� ���ϵǴ� ���� ��������.
	else if (retval == 0)
	{
		m_Dump->Crash();
		return false;
	}

	// 8. ������� ������ Rear�� �̵���Ų��.
	NowDummy->m_RecvBuff.MoveWritePos(retval - 1);
	   

	//////////////////////////////////////////////////
	// �Ϸ� ��Ŷ ó�� �κ�
	// RecvBuff�� ����ִ� ��� �ϼ� ��Ŷ�� ó���Ѵ�.
	//////////////////////////////////////////////////
	while (1)
	{
		// 1. RecvBuff�� �ּ����� ����� �ִ��� üũ. (���� = ��� ������� ���ų� �ʰ�. ��, �ϴ� �����ŭ�� ũ�Ⱑ �ִ��� üũ)	
		stProtocolHead Header;
		int len = sizeof(Header);

		// RecvBuff�� ��� ���� ���� ũ�Ⱑ ��� ũ�⺸�� �۴ٸ�, �Ϸ� ��Ŷ�� ���ٴ� ���̴� while�� ����.
		if (NowDummy->m_RecvBuff.GetUseSize() < len)
			break;

		// 2. ����� Peek���� Ȯ���Ѵ�.		
		// Peek �ȿ�����, ��� �ؼ����� len��ŭ �д´�. ���۰� �� ���� ���� �̻�!
		int PeekSize = NowDummy->m_RecvBuff.Peek((char*)&Header, len);

		// Peek()�� ���۰� ������� -1�� ��ȯ�Ѵ�. ���۰� ������� �ϴ� ���´�.
		if (PeekSize == -1)
		{
			m_Dump->Crash();
			return false;	// FALSE�� ���ϵǸ�, ������ �����.
		}

		// 4. ����� Len���� RecvBuff�� ������ ������ ��. (�ϼ� ��Ŷ ������ = ��� ������ + ���̷ε� Size)
		// ��� ���, �ϼ� ��Ŷ ����� �ȵǸ� while�� ����.
		if (NowDummy->m_RecvBuff.GetUseSize() < (len + Header.m_Len))
			break;

		// 4. ����� code Ȯ��(���������� �� �����Ͱ� �´°�)
		if (Header.m_Code != m_bCode)
		{
			m_Dump->Crash();
			return false;	// FALSE�� ���ϵǸ�, ������ �����.
		}

		// 5. RecvBuff���� Peek�ߴ� ����� ����� (�̹� Peek������, �׳� Remove�Ѵ�)
		NowDummy->m_RecvBuff.RemoveData(len);

		// 6. RecvBuff���� ���̷ε� Size ��ŭ ���̷ε� �ӽ� ���۷� �̴´�. (��ť�̴�. Peek �ƴ�)
		CProtocolBuff_Net* PayloadBuff = CProtocolBuff_Net::Alloc();
		WORD PayloadSize = Header.m_Len;

		// 7. RecvBuff���� ���̷ε� Size ��ŭ ���̷ε� ����ȭ ���۷� �̴´�. (��ť�̴�. Peek �ƴ�)
		int DequeueSize = NowDummy->m_RecvBuff.Dequeue(PayloadBuff->GetBufferPtr(), PayloadSize);

		// ���۰� ����ְų�, ���� ���ϴ¸�ŭ �����Ͱ� �����ٸ�, ���̾ȵ�. (�� if�������� �ִٰ� �ߴµ� ������� ���ٴ°�)
		// ���������� ���� ���� ����.
		if ((DequeueSize == -1) || (DequeueSize != PayloadSize))
			m_Dump->Crash();	
		
		// 8. �о�� ��ŭ rear�� �̵���Ų��. 
		PayloadBuff->MoveWritePos(DequeueSize);

		// 9. ��� Decode
		if (PayloadBuff->Decode(PayloadSize, Header.m_RandXORCode, Header.m_Checksum, m_bXORCode_1, m_bXORCode_2) == false)
		{
			m_Dump->Crash();
		}

		// 7. ����� ����ִ� Ÿ�Կ� ���� �б�ó��.
		Network_Packet(NowDummy, PayloadBuff);

		// 8. ����ȭ ���� ����
		CProtocolBuff_Net::Free(PayloadBuff);
		
	}

	return true;
}


// SendBuff�� �����͸� Send�ϱ�
//
// Parameter : stDummy*
// return : ������ ���ܾ� �ϴ� ������, false ����
bool cMatchTestDummy::SendProc(stDummy* NowDummy)
{
	// 1. SendBuff�� ���� �����Ͱ� �ִ��� Ȯ��.
	if (NowDummy->m_SendBuff.GetUseSize() == 0)
		return true;

	// 2. ���� �������� �������� ���� ������
	char* sendbuff = NowDummy->m_SendBuff.GetBufferPtr();

	// 3. SendBuff�� �� �������� or Send����(����)���� ��� send
	while (1)
	{
		// 4. SendBuff�� �����Ͱ� �ִ��� Ȯ��(���� üũ��)
		if (NowDummy->m_SendBuff.GetUseSize() == 0)
			break;

		// 5. �� ���� ���� �� �ִ� �������� ���� ���� ���
		int Size = NowDummy->m_SendBuff.GetNotBrokenGetSize();

		// 6. ���� ������ 0�̶�� 1�� ����. send() �� 1����Ʈ�� �õ��ϵ��� �ϱ� ���ؼ�.
		// �׷��� send()�� ������ �ߴ��� Ȯ�� ����
		if (Size == 0)
			Size = 1;

		// 7. front ������ ����
		int *front = (int*)NowDummy->m_SendBuff.GetReadBufferPtr();

		// 8. front�� 1ĭ �� index�� �˾ƿ���.
		int BuffArrayCheck = NowDummy->m_SendBuff.NextIndex(*front, 1);

		// 9. Send()
		int SendSize = send(NowDummy->m_sock, &sendbuff[BuffArrayCheck], Size, 0);

		// 10. ���� üũ. �����̸� �� �̻� �������� while�� ����. ���� Select�� �ٽ� ����
		if (SendSize == SOCKET_ERROR)
		{
			DWORD Error = WSAGetLastError();

			if (Error == WSAEWOULDBLOCK ||
				Error == 10054)
				break;

			else
			{				
				m_Dump->Crash();
				return false;
			}

		}

		// 11. ���� ����� ��������, �� ��ŭ remove
		NowDummy->m_SendBuff.RemoveData(SendSize);
	}

	return true;
}


// Recv �� ������ ó�� �κ�
//
// Parameter : stDummy*, ���̷ε�(CProtocolBuff_Net*)
// return : ����
void cMatchTestDummy::Network_Packet(stDummy* NowDummy, CProtocolBuff_Net* payload)
{
	// Ÿ�� �и�
	WORD Type;
	payload->GetData((char*)&Type, 2);
	
	// Ÿ�Կ� ���� �б� ó��
	try
	{
		switch (Type)
		{
			// �α��� ��û ���
		case en_PACKET_CS_MATCH_RES_LOGIN:
			PACKET_Login_Match(NowDummy, payload);
			break;

		default:
			TCHAR ErrStr[1024];
			StringCchPrintf(ErrStr, 1024, _T("Network_Packet(). TypeError. Type : %d, AccountNo : %lld"),
				Type, NowDummy->m_ui64AccountNo);

			throw CException(ErrStr);
		}

	}
	catch (CException& exc)
	{
		// �α� ��� (�α� ���� : ����)
		m_Log->LogSave(L"TestClinet", CSystemLog::en_LogLevel::LEVEL_ERROR, L"%s",
			(TCHAR*)exc.GetExceptionText());

		// Crash
		m_Dump->Crash();
	}
}





// ------------------------------------
// ��Ŷ ó�� �Լ�
// ------------------------------------

// Match������ ���� �α��� ��Ŷ�� ���� ó��
//
// Parameter : stDummy*, ���̷ε�(CProtocolBuff_Net*)
// return : ����
void cMatchTestDummy::PACKET_Login_Match(stDummy* NowDummy, CProtocolBuff_Net* payload)
{
	// 1. ������
	BYTE Status;
	payload->GetData((char*)&Status, 1);

	// 2. ������ �ƴ϶�� �ܺη� Throw ����
	if (Status != 1)
	{
		TCHAR ErrStr[1024];
		StringCchPrintf(ErrStr, 1024, _T("PACKET_Login_Match(). Login Response Status Error : %d(AccountNo : %lld)"),
			Status, NowDummy->m_ui64AccountNo);

		throw CException(ErrStr);		
	}

	// 3. �����̶�� Flag ����
	NowDummy->m_bMatchLogin = true;		// ��ġ����ŷ�� �α��� ���·� ����	
	NowDummy->m_bLoginPacketSend = false;	// ��ġ����ŷ�� �α�����Ŷ �Ⱥ��� ���·� ����
}







// ------------------------------------
// �ܺο��� ��� ���� �Լ�
// ------------------------------------


// ��ġ����ŷ ������ IP�� Port �˾ƿ��� �Լ�
// �� ���̰�, �ڽ��� ����� Server�� IP�� Port�� �������.
//
// Parameter : ����
// return : ����
void cMatchTestDummy::MatchInfoGet()
{
	// ���÷� �޾Ƶα�
	int Count = m_iDummyCount;
	UINT64 TempStartNo = m_ui64StartAccountNo;
	HTTP_Exchange* pHTTP_Post = HTTP_Post;

	// ���� �� ��ŭ ���鼭, ��ġ����ŷ ������ IP �˾ƿ�
	TCHAR RequestBody[2000];
	TCHAR Body[1000];

	int Temp = 0;
	while (1)
	{
		// 1. ���� umap���� stDummy* �˾ƿ���
		stDummy* NowDummy = FindDummyFunc(TempStartNo);
		if (NowDummy == nullptr)
			m_Dump->Crash();

		// 2. Connect ���� ���� ���̸� ��ġ����ŷ�� ������ �˾ƿ´�.
		if (NowDummy->m_bMatchConnect == false)
		{
			// get_matchmaking.php�� http ���.
			ZeroMemory(RequestBody, sizeof(RequestBody));
			ZeroMemory(Body, sizeof(Body));

			swprintf_s(Body, _Mycountof(Body), L"{\"accountno\" : %lld, \"sessionkey\" : \"%s\"}", TempStartNo, NowDummy->m_tcToken);
			if (pHTTP_Post->HTTP_ReqANDRes((TCHAR*)_T("Lobby/get_matchmaking.php"), Body, RequestBody) == false)
				m_Dump->Crash();


			// Json������ �Ľ��ϱ�
			GenericDocument<UTF16<>> Doc;
			Doc.Parse(RequestBody);


			// ��� Ȯ�� 
			int Result;
			Result = Doc[_T("result")].GetInt();

			// ����� 1�� �ƴϸ� Crash()
			if (Result != 1)
				m_Dump->Crash();


			// P�� Port ����
			const TCHAR* TempServerIP = Doc[_T("ip")].GetString();
			_tcscpy_s(NowDummy->m_tcServerIP, 20, TempServerIP);
			NowDummy->m_iPort = Doc[_T("port")].GetInt();
		}


		// 3. ������ üũ
		++Temp;
		++TempStartNo;
		if (Temp == Count)
			break;
	}

}


// ���� ���� �Լ�
//
// Parameter : ������ ���� ��, ���� AccountNo
// return : ����
void cMatchTestDummy::CreateDummpy(int Count, int StartAccountNo)
{
	// �Է¹��� Count��ŭ ���� ����
	m_iDummyCount = Count;
	m_ui64StartAccountNo = StartAccountNo;

	int Temp = 0;
	while (1)
	{
		// 1. ���� ����ü �Ҵ�
		stDummy* NowDummy = m_DummyStructPool->Alloc();

		// 2. �� ����
		NowDummy->m_ui64AccountNo = StartAccountNo;

		TCHAR tcToken[65] = TOKEN_T;
		_tcscpy_s(NowDummy->m_tcToken, 64, tcToken);

		char cToken[65] = TOKEN_C;
		strcpy_s(NowDummy->m_cToekn, 64, cToken);

		// 3. umap�� �߰�
		if (InsertDummyFunc(StartAccountNo, NowDummy) == false)
			m_Dump->Crash();

		// 4. Temp++ ��, Count��ŭ ��������� break;
		Temp++;
		StartAccountNo++;
		if (Temp == Count)
			break;
	}
}

// ���� Run �Լ�
// 1. ��ġ����ŷ Connect (Connect �ȵ� ���� ���)
// 2. �α��� ��Ŷ Enqueue (Connect �Ǿ�����, Login �ȵ� ����, �׸��� �α��� ��Ŷ�� ������ ���� ���� ���)
// 3. Select ó�� (Connect �� ���� ���)
// 4. Disconnect. (Connect ��, Login �� ���� ���. Ȯ���� Disconnect)
//
// Parameter : ����
// return : ����
void cMatchTestDummy::DummyRun()
{
	// --------------------------------- 
	// 1. ��ġ����ŷ�� Connect
	// --------------------------------- 
	MatchConnect();

	// --------------------------------- 
	// 2. ��ġ����ŷ���� ���� �α��� ��Ŷ Enqueue
	// --------------------------------- 
	MatchLogin();

	// --------------------------------- 
	// 3. Select ó��
	// --------------------------------- 
	SelectFunc();

	// --------------------------------- 
	// 4. Disconnect ó��
	// --------------------------------- 
	MatchDisconenct();
}



// ------------------------------------
// ������, �Ҹ���
// ------------------------------------

// ������
cMatchTestDummy::cMatchTestDummy()
{
	// srand ���� (m_Log ������ �ּҸ� �׳� Seed ������ ���)
	srand((UINT)m_Log);

	// �α� �޾ƿ���
	m_Log = CSystemLog::GetInstance();

	// ------------------- �α� ������ ���� ����
	m_Log->SetDirectory(L"TestClinet");
	m_Log->SetLogLeve((CSystemLog::en_LogLevel)0);

	// ���� �� 0���� ����
	m_iDummyCount = 0;

	// m_uamp_Dummy ���� �Ҵ�
	m_uamp_Dummy.reserve(5000);
	m_uamp_Socket_and_AccountNo.reserve(5000);

	// ���� ����ü pool �����Ҵ�
	m_DummyStructPool = new CMemoryPoolTLS<stDummy>(0, false);

	// ���� �̱��� ������
	m_Dump = CCrashDump::GetInstance();

	// HTTP ��ſ� �����Ҵ�
	HTTP_Post = new HTTP_Exchange((TCHAR*)_T("127.0.0.1"), 80);

	// ����� ���� Config�� 
	m_bCode = 119;
	m_bXORCode_1 = 50;
	m_bXORCode_2 = 132;

	// ���� �ʱ�ȭ
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
	{
		fputs("Winsock Init Fail...\n", stdout);
		m_Dump->Crash();
	}
}

// �Ҹ���
cMatchTestDummy::~cMatchTestDummy()
{
	// ���� ����ü pool ��������
	delete m_DummyStructPool;

	// HTTP ��ſ� ��������
	delete HTTP_Post;

	// ���� ����
	WSACleanup();
}