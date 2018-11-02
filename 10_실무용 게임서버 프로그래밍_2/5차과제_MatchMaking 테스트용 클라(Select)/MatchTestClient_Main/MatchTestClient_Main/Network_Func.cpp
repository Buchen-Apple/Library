#include "pch.h"

#include <Ws2tcpip.h>
#include <mmsystem.h>
#pragma comment(lib,"winmm.lib")

#include "rapidjson\document.h"
#include "rapidjson\writer.h"
#include "rapidjson\stringbuffer.h"

#include "Network_Func.h"
#include "Protocol_Set/CommonProtocol_2.h"

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

	// ���� �� ��ŭ ���鼭, Connect �õ�
	int Temp = 0;
	while (1)
	{
		// 1. ���� �˻�
		stDummy* NowDummy = FindDummyFunc(Temp + 1);
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

		}

		// 3. Temp++ ��, Count��ŭ ��������� break;
		Temp++;
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

	// Connect�� �� ������ ���� �α��� ��Ŷ ����
	int Temp = 0;
	while (1)
	{
		// 1. ���� �˻�
		UINT AccountNo = Temp + 1;
		stDummy* NowDummy = FindDummyFunc(AccountNo);
		if (NowDummy == nullptr)
			m_Dump->Crash();

		// 2. Connect �Ǿ���, Login �ȵ� ���� ���
		if (NowDummy->m_bMatchConnect == true && 
			NowDummy->m_bMatchLogin == false)
		{
			// SendBuff �Ҵ�ޱ�
			CProtocolBuff_Net* SendBuff = CProtocolBuff_Net::Alloc();

			// ���� ��Ŷ ����
			WORD Type = en_PACKET_CS_MATCH_REQ_LOGIN;
			UINT VerCode = VERCODE;

			SendBuff->PutData((char*)&Type, 2);
			SendBuff->PutData((char*)&AccountNo, 8);
			SendBuff->PutData(NowDummy->m_cToekn, 64);
			SendBuff->PutData((char*)&VerCode, 4);

			// SendQ�� �ֱ�
			SendPacket(NowDummy, SendBuff);
		}

		// 3. Temp++ ��, Count��ŭ ��������� break;
		Temp++;
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

	// 2. ��ť. ��Ŷ�� "�ּ�"�� ��ť�Ѵ�(8����Ʈ)
	// ����ȭ ���� ���۷��� ī��Ʈ 1 ����
	payload->Add();
	NowDummy->m_SendQueue->Enqueue(payload);

	// 3. ����ȭ ���� ���۷��� ī��Ʈ 1 ����. 0 �Ǹ� �޸�Ǯ�� ��ȯ
	CProtocolBuff_Net::Free(payload);

	return true;
}



// ------------------------------------
// �ܺο��� ��� ���� �Լ�
// ------------------------------------

// ���� ���� �Լ�
//
// Parameter : ������ ���� ��
// return : ����
void cMatchTestDummy::CreateDummpy(int Count)
{
	// �Է¹��� Count��ŭ ���� ����
	m_iDummyCount = Count;
	int Temp = 0;
	while (1)
	{
		// 1. ���� ����ü �Ҵ�
		stDummy* NowDummy = m_DummyStructPool->Alloc();

		// 2. �� ����
		NowDummy->m_ui64AccountNo = Temp + 1;

		TCHAR tcToken[65] = TOKEN_T;
		_tcscpy_s(NowDummy->m_tcToken, 64, tcToken);

		char cToken[65] = TOKEN_C;
		strcpy_s(NowDummy->m_cToekn, 64, cToken);

		// 3. umap�� �߰�
		if (InsertDummyFunc(Temp + 1, NowDummy) == false)
			m_Dump->Crash();

		// 4. Temp++ ��, Count��ŭ ��������� break;
		Temp++;
		if (Temp == Count)
			break;
	}
}


// ��ġ����ŷ ������ IP�� Port �˾ƿ��� �Լ�
// �� ���̰�, �ڽ��� ����� Server�� IP�� Port�� �������.
//
// Parameter : ����
// return : ����
void cMatchTestDummy::MatchInfoGet()
{	
	// ���÷� �޾Ƶα�
	int Count = m_iDummyCount;
	HTTP_Exchange* pHTTP_Post = HTTP_Post;		

	// ���� �� ��ŭ ���鼭, ��ġ����ŷ ������ IP �˾ƿ�
	TCHAR RequestBody[2000];
	TCHAR Body[1000];
	int Temp = 0;

	while (1)
	{
		// 1. ���� umap���� stDummy* �˾ƿ���
		stDummy* NowDummy = FindDummyFunc(Temp + 1);
		if (NowDummy == nullptr)
			m_Dump->Crash();

		// 2. Connect ���� ���� ���̸� ��ġ����ŷ�� ������ �˾ƿ´�.
		if (NowDummy->m_bMatchConnect == false)
		{
			// get_matchmaking.php�� http ���.
			ZeroMemory(RequestBody, sizeof(RequestBody));
			ZeroMemory(Body, sizeof(Body));

			swprintf_s(Body, _Mycountof(Body), L"{\"accountno\" : %d, \"sessionkey\" : \"%s\"}", Temp + 1, NowDummy->m_tcToken);
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

		// 3. Temp++ ��, Count��ŭ ��������� break;
		Temp++;
		if (Temp == Count)
			break;
	}

}

// ���� Run �Լ�
// 1. ��ġ����ŷ ������ IP�� Port �˾ƿ��� (Connect �ȵ� ���� ���)
// 2. ��ġ����ŷ Connect (Connect �ȵ� ���� ���)
// 3. �α��� ��Ŷ Enqueue (Connect �Ǿ�����, Login �ȵ� ���� ���)
// 4. Select ó�� (Connect �� ���� ���)
// 5. Disconnect. (Connect ��, Login �� ���� ���. Ȯ���� Disconnect)
//
// Parameter : ����
// return : ����
void cMatchTestDummy::DummyRun()
{
	// --------------------------------- 
	// 1. ��ġ����ŷ�� �ּ� �˾ƿ���
	// --------------------------------- 
	MatchInfoGet();

	// --------------------------------- 
	// 2. ��ġ����ŷ�� Connect
	// --------------------------------- 
	MatchConnect();

	// --------------------------------- 
	// 3. ��ġ����ŷ���� ���� �α��� ��Ŷ Enqueue
	// --------------------------------- 
	MatchLogin();

	// --------------------------------- 
	// 4. Select ó��
	// --------------------------------- 

}



// ------------------------------------
// ������, �Ҹ���
// ------------------------------------

// ������
cMatchTestDummy::cMatchTestDummy()
{
	// ���� �� 0���� ����
	m_iDummyCount = 0;

	// m_uamp_Dummy ���� �Ҵ�
	m_uamp_Dummy.reserve(5000);

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