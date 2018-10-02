#include "pch.h"

#pragma comment(lib,"ws2_32")
#include <Ws2tcpip.h>

#include <process.h>
#include <strsafe.h>


#include "NetworkLib_MMOServer.h"
#include "LockFree_Queue\LockFree_Queue.h"
#include "RingBuff\RingBuff.h"
#include "Log\Log.h"
#include "CrashDump\CrashDump.h"
#include "NormalTemplate_Queue\Normal_Queue_Template.h"

ULONGLONG g_ullAcceptTotal_MMO;
LONG	  g_lAcceptTPS_MMO;
LONG	g_lSendPostTPS_MMO;



namespace Library_Jingyu
{

#define _MyCountof(_array)		sizeof(_array) / (sizeof(_array[0]))

	// �α� ���� �������� �ϳ� �ޱ�.
	CSystemLog* cMMOServer_Log = CSystemLog::GetInstance();

	// ���� ���� ���� �ϳ� �ޱ�
	CCrashDump* cMMOServer_Dump = CCrashDump::GetInstance();

	// �� ���� ������ �� �ִ� WSABUF�� ī��Ʈ
#define dfSENDPOST_MAX_WSABUF			300

	// ------------------
	// �̳� Ŭ���� 
	// ------------------
	struct CMMOServer::stSession
	{
		friend class CMMOServer;

	protected:
		// -----------
		// ����� ���� ���� ����
		// -----------

		// Auth To Game Falg
		// true�� Game���� ��ȯ�� ����
		LONG m_lAuthToGameFlag;

	private:
		// -----------
		// ��� ����
		// -----------

		// ������ ��� ����
		euSessionModeState m_euMode;

		// �α׾ƿ� �÷���
		// true�� �α׾ƿ� �� ����.
		LONG m_lLogoutFlag;		

		// CompleteRecvPacket(Queue)
		CNormalQueue<CProtocolBuff_Net*>* m_CRPacketQueue;

		// ���ǰ� ����� ����
		SOCKET m_Client_sock;

		// �ش� ������ ����ִ� �迭 �ε���
		WORD m_lIndex;

		// ����� ������ IP�� Port
		TCHAR m_IP[30];
		USHORT m_prot;

		// Send overlapped����ü
		OVERLAPPED m_overSendOverlapped;

		// ���� ID. �������� ��� �� ���.
		ULONGLONG m_ullSessionID;

		// I/O ī��Ʈ. WSASend, WSARecv�� 1�� ����.
		// 0�̵Ǹ� ���� ����� ������ �Ǵ�.
		// ���� : ����� ������ WSARecv�� ������ �ɱ� ������ 0�� �� ���� ����.
		LONG	m_lIOCount;

		// ����, WSASend�� �� ���� �����͸� �����ߴ°�. (����Ʈ �ƴ�! ī��Ʈ. ����!)
		int m_iWSASendCount;

		// Send�� ����ȭ ���۵� ������ ������ ����
		CProtocolBuff_Net* m_PacketArray[dfSENDPOST_MAX_WSABUF];

		// Send���� �������� üũ. 1�̸� Send��, 0�̸� Send�� �ƴ�
		LONG	m_lSendFlag;

		// Send����. ������ť ����. ��Ŷ����(����ȭ ����)�� �����͸� �ٷ��.
		CLF_Queue<CProtocolBuff_Net*>* m_SendQueue;

		// Recv overlapped����ü
		OVERLAPPED m_overRecvOverlapped;

		// Recv����. �Ϲ� ������. 
		CRingBuff m_RecvQueue;			

	public:
		// -----------------
		// �����ڿ� �Ҹ���
		// -----------------
		stSession()
		{
			m_SendQueue = new CLF_Queue<CProtocolBuff_Net*>(0, false);
			m_CRPacketQueue = new CNormalQueue<CProtocolBuff_Net*>;
			m_lIOCount = 0;
			m_lSendFlag = FALSE;
			m_iWSASendCount = 0;
			m_euMode = euSessionModeState::MODE_NONE;
			m_lLogoutFlag = FALSE;
			m_lAuthToGameFlag = FALSE;
		}

		~stSession()
		{
			delete m_SendQueue;
			delete m_CRPacketQueue;
		}

	public:
		// -----------------
		// ���� �����Լ�
		// -----------------

		// Auth �����忡�� ó��
		virtual void OnAuth_ClientJoin() = 0;
		virtual void OnAuth_ClientLeave() = 0;
		virtual void OnAuth_Packet(CProtocolBuff_Net* Packet) = 0;

		// Game �����忡�� ó��
		virtual void OnGame_ClientJoin() = 0;
		virtual void OnGame_ClientLeave() = 0;
		virtual void OnGame_Packet(CProtocolBuff_Net* Packet) = 0;

		// Release��
		virtual void OnGame_Release() = 0;

	};


	// -----------------------
	// �����ڿ� �Ҹ���
	// -----------------------
	CMMOServer::CMMOServer()
	{
		// ���� �������� false�� ���� 
		m_bServerLife = false;

		// Auth���� ������ �ϰ� Pool �����Ҵ�
		m_pAcceptPool = new CMemoryPoolTLS<stAuthWork>(0, false);

		// Auth���� ������ �ϰ� ���� ť
		m_pASQ = new CLF_Queue< stAuthWork*>(0);
	}

	CMMOServer::~CMMOServer()
	{
		// ������ �������̾�����, ���� �������� ����
		if (m_bServerLife == true)
			Stop();
	}


	// -----------------------
	// ���ο����� ����ϴ� �Լ�
	// -----------------------

	// Start���� ������ �� �� ȣ���ϴ� �Լ�.
	// 1. ����� �Ϸ���Ʈ �ڵ� ��ȯ
	// 2. ��Ŀ������ ����, �ڵ��ȯ, �ڵ� �迭 ��������
	// 3. ����Ʈ ������ ����, �ڵ� ��ȯ
	// 4. �������� �ݱ�
	// 5. ���� ����
	void CMMOServer::ExitFunc(int w_ThreadCount)
	{
		// 1. ����� �Ϸ���Ʈ ���� ����.
		// null�� �ƴҶ���! (��, ����� �Ϸ���Ʈ�� ������� ���!)
		// �ش� �Լ��� ��𼭳� ȣ��Ǳ⶧����, ���� �̰� �ؾ��ϴ��� �׻� Ȯ��.
		if (m_hIOCPHandle != NULL)
			CloseHandle(m_hIOCPHandle);

		// 2. ��Ŀ������ ���� ����. Count�� 0�� �ƴҶ���!
		if (w_ThreadCount > 0)
		{
			// ���� �޽����� ������
			for (int h = 0; h < w_ThreadCount; ++h)
				PostQueuedCompletionStatus(m_hIOCPHandle, 0, 0, 0);

			// ��� ��Ŀ������ ���� ���.
			WaitForMultipleObjects(w_ThreadCount, m_hWorkerHandle, TRUE, INFINITE);

			// ��� ��Ŀ�����尡 ��������� �ڵ� ��ȯ
			for (int h = 0; h < w_ThreadCount; ++h)
				CloseHandle(m_hWorkerHandle[h]);

			// ��Ŀ������ �ڵ� �迭 ��������. 
			// Count�� 0���� ũ�ٸ� ������ �����Ҵ��� �� ���� ����
			delete[] m_hWorkerHandle;
		}

		// 3. ����Ʈ ������ ����, �ڵ� ��ȯ
		if (m_iA_ThreadCount > 0)
			CloseHandle(m_hAcceptHandle);

		// 4. �������� �ݱ�
		if (m_soListen_sock != NULL)
			closesocket(m_soListen_sock);

		// 5. ���� ����
		// ���� �ʱ�ȭ ���� ���� ���¿��� WSAClenup() ȣ���ص� �ƹ� ���� ����
		WSACleanup();
	}

	// ���� �������� ���½�Ų��.
	// Stop() �Լ����� ���.
	void CMMOServer::Reset()
	{
		m_iW_ThreadCount = 0;
		m_iA_ThreadCount = 0;
		m_hWorkerHandle = nullptr;
		m_hIOCPHandle = 0;
		m_soListen_sock = 0;
		m_iMaxJoinUser = 0;
		m_ullJoinUserCount = 0;
	}

	// RecvPost�Լ�
	//
	// return 0 : ���������� WSARecv() �Ϸ�
	// return 1 : RecvQ�� ���� ����
	// return 2 : I/O ī��Ʈ�� 0�̵Ǿ� ������ ����
	int CMMOServer::RecvPost(stSession* NowSession)
	{
		// ------------------
		// �񵿱� ����� ����
		// ------------------
		// 1. WSABUF ����.
		WSABUF wsabuf[2];
		int wsabufCount = 0;

		int FreeSize = NowSession->m_RecvQueue.GetFreeSize();
		int Size = NowSession->m_RecvQueue.GetNotBrokenPutSize();

		// ���ú� ������ ũ�� Ȯ��
		// ������� ������ 1����
		if (FreeSize <= 0)
			return 1;

		if (Size < FreeSize)
		{
			wsabuf[0].buf = NowSession->m_RecvQueue.GetRearBufferPtr();
			wsabuf[0].len = Size;

			wsabuf[1].buf = NowSession->m_RecvQueue.GetBufferPtr();
			wsabuf[1].len = FreeSize - Size;
			wsabufCount = 2;

		}
		else
		{
			wsabuf[0].buf = NowSession->m_RecvQueue.GetRearBufferPtr();
			wsabuf[0].len = Size;

			wsabufCount = 1;
		}

		// 2. Overlapped ����ü �ʱ�ȭ
		ZeroMemory(&NowSession->m_overRecvOverlapped, sizeof(NowSession->m_overRecvOverlapped));

		// 3. WSARecv()
		DWORD recvBytes = 0, flags = 0;
		InterlockedIncrement(&NowSession->m_lIOCount);

		// 4. ���� ó��
		if (WSARecv(NowSession->m_Client_sock, wsabuf, wsabufCount, &recvBytes, &flags, &NowSession->m_overRecvOverlapped, NULL) == SOCKET_ERROR)
		{
			int Error = WSAGetLastError();

			// �񵿱� ������� ���۵Ȱ� �ƴ϶��
			if (Error != WSA_IO_PENDING)
			{
				if (InterlockedDecrement(&NowSession->m_lIOCount) == 0)
				{
					// �α׾ƿ� �÷��� ����
					NowSession->m_lLogoutFlag = TRUE;
					return 2;
				}

				// ������ ���� �����̶��, I/Oī��Ʈ ������ ���� �ƴ϶� ������Ѵ�.
				if (Error == WSAENOBUFS)
				{
					// �� ����, �����쿡�� ����
					m_iOSErrorCode = Error;
					m_iMyErrorCode = euError::NETWORK_LIB_ERROR__WSAENOBUFS;

					// ���� ��Ʈ�� �����
					TCHAR tcErrorString[300];
					StringCchPrintf(tcErrorString, 300, _T("WSANOBUFS. UserID : %lld, [%s:%d]"),
						NowSession->m_ullSessionID, NowSession->m_IP, NowSession->m_prot);

					// �α� ��� (�α� ���� : ����)
					cMMOServer_Log->LogSave(L"NetServer", CSystemLog::en_LogLevel::LEVEL_ERROR, L"WSARecv --> %s : NetError(%d), OSError(%d)",
						tcErrorString, (int)m_iMyErrorCode, m_iOSErrorCode);

					// ���� �Լ� ȣ��
					OnError((int)euError::NETWORK_LIB_ERROR__WSAENOBUFS, tcErrorString);

					// ������ϴ� �˴ٿ� ȣ��
					shutdown(NowSession->m_Client_sock, SD_BOTH);
				}
			}
		}

		return 0;

	}






	// -----------------------
	// �������
	// -----------------------

	// ��Ŀ ������
	UINT	WINAPI	CMMOServer::WorkerThread(LPVOID lParam)
	{

	}

	// Accept ������
	UINT	WINAPI	CMMOServer::AcceptThread(LPVOID lParam)
	{
		// --------------------------
		// Accept ��Ʈ
		// --------------------------
		SOCKET client_sock;
		SOCKADDR_IN	clientaddr;
		int addrlen = sizeof(clientaddr);

		CMMOServer* g_This = (CMMOServer*)lParam;

		// ������ ������ �� ���� 1�� �����ϴ� ������ Ű.
		ULONGLONG ullUniqueSessionID = 0;
		TCHAR tcTempIP[30];
		USHORT port;
		WORD iIndex;

		while (1)
		{
			ZeroMemory(&clientaddr, sizeof(clientaddr));
			client_sock = accept(g_This->m_soListen_sock, (SOCKADDR*)&clientaddr, &addrlen);
			if (client_sock == INVALID_SOCKET)
			{
				int Error = WSAGetLastError();
				// 10004��(WSAEINTR) ������ ��������. �Լ�ȣ���� �ߴܵǾ��ٴ� ��.
				// 10038��(WSAEINTR) �� ������ �ƴ� �׸� �۾��� �õ��ߴٴ� ��. �̹� ���������� closesocket�Ȱ��̴� ���� �ƴ�.
				if (Error == WSAEINTR || Error == WSAENOTSOCK)
				{
					// Accept ������ ���� ����
					break;
				}

				// �װ� �ƴ϶�� OnError �Լ� ȣ��
				// ������ ����, �� ���� ����
				g_This->m_iOSErrorCode = Error;
				g_This->m_iMyErrorCode = euError::NETWORK_LIB_ERROR__A_THREAD_ABNORMAL_EXIT;

				// ���� ��Ʈ�� �����
				TCHAR tcErrorString[300];
				StringCchPrintf(tcErrorString, 300, L"accpet(). Abonormal_exit : NetError(%d), OSError(%d)",
					(int)g_This->m_iMyErrorCode, g_This->m_iOSErrorCode);

				// ���� �߻� �Լ� ȣ��
				g_This->OnError((int)euError::NETWORK_LIB_ERROR__A_THREAD_ABNORMAL_EXIT, tcErrorString);

				break;
			}

			g_ullAcceptTotal_MMO++;	// �׽�Ʈ��!!
			InterlockedIncrement(&g_lAcceptTPS_MMO); // �׽�Ʈ��!!

			// ------------------
			// �ִ� ������ �� �̻� ���� �Ұ�
			// ------------------
			if (g_This->m_iMaxJoinUser <= g_This->m_ullJoinUserCount)
			{
				closesocket(client_sock);

				// �� ���� ����
				g_This->m_iMyErrorCode = euError::NETWORK_LIB_ERROR__JOIN_USER_FULL;

				// ���� ��Ʈ�� �����
				TCHAR tcErrorString[300];
				StringCchPrintf(tcErrorString, 300, L"accpet(). User Full!!!! (%lld)",
					g_This->m_ullJoinUserCount);

				// ���� �߻� �Լ� ȣ��
				g_This->OnError((int)euError::NETWORK_LIB_ERROR__JOIN_USER_FULL, tcErrorString);

				// continue
				continue;
			}
			
			// ------------------
			// �ϰ� Alloc
			// ------------------
			stAuthWork* NowWork = g_This->m_pAcceptPool->Alloc();



			// ------------------
			// IP�� ��Ʈ �˾ƿ���.
			// ------------------
			InetNtop(AF_INET, &clientaddr.sin_addr, NowWork->m_tcIP, 30);
			NowWork->m_usPort = ntohs(clientaddr.sin_port);	


			// Auth���� ����
			g_This->m_pASQ->Enqueue(NowWork);

		}

	}

	// Auth ������
	UINT	WINAPI	CMMOServer::AuthThread(LPVOID lParam)
	{
		CMMOServer* g_This = (CMMOServer*)lParam;

		// ������ ������ �� ���� 1�� �����ϴ� ������ Ű.
		ULONGLONG ullUniqueSessionID = 0;

		// �ε��� ���� ����
		WORD iIndex;		

		// AUTH��忡�� �� ���� ó���ϴ� ��Ŷ�� ��. 
		const int PACKET_WORK_COUNT = 1;

		while (1)
		{
			// ------------------
			// 1�̸� �����帶�� 1ȸ�� �Ͼ��.
			// ------------------	



			// ------------------
			// Part 1. �ű� ������ ��Ŷ ó��
			// ------------------
			int Size = g_This->m_pASQ->GetInNode();

			int i = 0;
			stAuthWork* NowWork;
			while (i < Size)
			{				
				// 1. �ϰ� ��ť
				if (g_This->m_pASQ->Dequeue(NowWork) == -1)
					cMMOServer_Dump->Crash();


				// 2. ���� ����, IP���� �Ǵ��ؼ� ���� �߰� �۾��� �ʿ��� ��찡 ���� ���� ������ ȣ��	
				// false�� ���Ӱź�, 
				// true�� ���� ��� ����. true���� �Ұ� ������ OnConnectionRequest�Լ� ���ڷ� ������ ������.
				if (g_This->OnConnectionRequest(NowWork->m_tcIP, NowWork->m_usPort) == false)
					continue;
				

				// 3. ���� ����ü ���� �� ����
				// 1) �̻�� �ε��� �˾ƿ���
				iIndex = g_This->m_stEmptyIndexStack->Pop();

				// 2) I/O ī��Ʈ ����
				// ���� ���
				InterlockedIncrement(&g_This->m_stSessionArray[iIndex].m_lIOCount);

				// 3) ���� �����ϱ�
				// -- ���� ID(�ͽ� Ű)�� �ε��� �Ҵ�
				ULONGLONG MixKey = ((ullUniqueSessionID << 16) | iIndex);
				ullUniqueSessionID++;

				g_This->m_stSessionArray[iIndex].m_ullSessionID = MixKey;
				g_This->m_stSessionArray[iIndex].m_lIndex = iIndex;

				// -- ����
				g_This->m_stSessionArray[iIndex].m_Client_sock = NowWork->m_clienet_Socket;

				// -- IP�� Port
				StringCchCopy(g_This->m_stSessionArray[iIndex].m_IP, _MyCountof(g_This->m_stSessionArray[iIndex].m_IP), NowWork->m_tcIP);
				g_This->m_stSessionArray[iIndex].m_prot = NowWork->m_usPort;

				// 4) ������ ��� �������� Auth ���·� ����
				g_This->m_stSessionArray[iIndex].m_euMode = euSessionModeState::MODE_AUTH;


				// 4. IOCP ����
				// ���ϰ� IOCP ����
				if (CreateIoCompletionPort((HANDLE)NowWork->m_clienet_Socket, g_This->m_hIOCPHandle, (ULONG_PTR)&g_This->m_stSessionArray[iIndex], 0) == NULL)
				{
					// ������ ����, �� ���� ����
					g_This->m_iOSErrorCode = WSAGetLastError();
					g_This->m_iMyErrorCode = euError::NETWORK_LIB_ERROR__A_THREAD_IOCPCONNECT_FAIL;

					// ���� ��Ʈ�� �����
					TCHAR tcErrorString[300];
					StringCchPrintf(tcErrorString, 300, L"accpet(). IOCP_Connect Error : NetError(%d), OSError(%d)",
						(int)g_This->m_iMyErrorCode, g_This->m_iOSErrorCode);

					// ���� �߻� �Լ� ȣ��
					g_This->OnError((int)euError::NETWORK_LIB_ERROR__A_THREAD_IOCPCONNECT_FAIL, tcErrorString);

					break;
				}

				// ������ �� ����. disconnect������ ���Ǵ� �����̱� ������ ���Ͷ� ���
				InterlockedIncrement(&g_This->m_ullJoinUserCount);
				

				// 5. ��� ���������� �Ϸ�Ǿ����� ���� �� ó�� �Լ� ȣ��.
				g_This->m_stSessionArray[iIndex].OnAuth_ClientJoin();
							   

				// 6. �񵿱� ����� ����
				// ���ú� ���۰� ��á���� 1����
				int ret = g_This->RecvPost(&g_This->m_stSessionArray[iIndex]);

				// �������״�, I/Oī��Ʈ --. 0�̶�� �α׾ƿ� �÷��� ����.
				if (InterlockedDecrement(&g_This->m_stSessionArray[iIndex].m_lIOCount) == 0)
					g_This->m_stSessionArray[iIndex].m_lLogoutFlag = TRUE;

				// I/Oī��Ʈ ���ҽ��״µ� 0�� �ƴ϶��, ret üũ.
				// ret�� 1�̶�� ���� ���´�.
				else if (ret == 1)
					shutdown(g_This->m_stSessionArray[iIndex].m_Client_sock, SD_BOTH);
			}



			// ------------------
			// Part 2. AUTH��� ���ǵ� ��Ŷ ó�� + Logout Flag ó�� 1��
			// ------------------
			i = 0;
			while (i < g_This->m_iMaxJoinUser)
			{
				// �ش� ������ MODE_AUTH���� Ȯ�� ---------------------------
				if (g_This->m_stSessionArray[i].m_euMode == euSessionModeState::MODE_AUTH)
				{

					// LogOutFlag�� TRUE��� 
					if (g_This->m_stSessionArray[i].m_lLogoutFlag = TRUE)
					{
						// ��带 MODE_LOGOUT_IN_AUTH�� ����
						g_This->m_stSessionArray[i].m_euMode = euSessionModeState::MODE_LOGOUT_IN_AUTH;
					}

					// LogOutFlag�� FALSE�� ������ ���� ���� ó��
					else
					{
						// 1. CompleteRecvPacket ť�� ������ Ȯ��.
						int iQSize = g_This->m_stSessionArray[i].m_CRPacketQueue->GetNodeSize();

						// 2. ť�� ��尡 1�� �̻� ������, ��Ŷ ó��
						if (iQSize > 0)
						{
							// !! ��Ŷ ó�� Ƚ�� ������ �α� ����, PACKET_WORK_COUNT���� ����� ���� �� ������, PACKET_WORK_COUNT�� ���� !!
							if (iQSize > PACKET_WORK_COUNT)
								iQSize = PACKET_WORK_COUNT;

							CProtocolBuff_Net* NowPacket;

							int TempCount = 0;
							while (TempCount < iQSize)
							{
								// ��� ����
								if (g_This->m_stSessionArray[i].m_CRPacketQueue->Dequeue(NowPacket) == -1)
									cMMOServer_Dump->Crash();

								// ��Ŷ ���۷��� ī��Ʈ 1 ����
								NowPacket->Add();

								// ��Ŷ ó��
								g_This->m_stSessionArray[i].OnAuth_Packet(NowPacket);

								// ��Ŷ ���۷��� ī��Ʈ 1 ����
								CProtocolBuff_Net::Free(NowPacket);

								// �ݺ��� ī��Ʈ ����.
								TempCount++;
							}
						}
					}
				}				
				

				++i;
			}



			// ------------------
			// Part 3. OnAuth_Update
			// ------------------
			g_This->OnAuth_Update();



			// ------------------
			// Part 4. Logout Flag ó�� 2�� + Auth To Game ó��
			// ------------------
			i = 0;
			while (i < g_This->m_iMaxJoinUser)
			{
				// �ش� ������ MODE_LOGOUT_IN_AUTH���� Ȯ�� ---------------------------
				if (g_This->m_stSessionArray[i].m_euMode == euSessionModeState::MODE_LOGOUT_IN_AUTH)
				{
					// SendFlag�� FALSE���� Ȯ��
					if (InterlockedOr(&g_This->m_stSessionArray[i].m_lSendFlag, 0) == FALSE)
					{
						// �����ٰ� �˷��ش�.
						// ��� ���� �� �˷��ָ�, ���� �˷��ֱ� ����, GAME�ʿ��� Release�Ǿ� Release�� ���� �� ���ɼ�.
						g_This->m_stSessionArray[i].OnAuth_ClientLeave();

						// MODE_WAIT_LOGOUT���� ��� ����
						g_This->m_stSessionArray[i].m_euMode = euSessionModeState::MODE_WAIT_LOGOUT;						
					}

					// Ȥ�� Auth TO Game �÷��� Ȯ��
					else if (g_This->m_stSessionArray[i].m_lAuthToGameFlag == TRUE)
					{
						// �����ٰ� �˷��ش�.
						// ��� ���� �� �˷��ָ�, OnAuth_ClientLeave�� ȣ��Ǳ⵵ ����, GAME�ʿ��� ���� OnGame_ClinetJoin �� ���ɼ�
						g_This->m_stSessionArray[i].OnAuth_ClientLeave();

						// MODE_AUTH_TO_GAME���� ��� ����
						g_This->m_stSessionArray[i].m_euMode = euSessionModeState::MODE_AUTH_TO_GAME;

					}
				}
			}		
		}
		
	}

	// Game ������
	UINT	WINAPI	CMMOServer::GameThread(LPVOID lParam)
	{}



	// -----------------------
	// �ܺο��� ��� ������ �Լ�
	// -----------------------

	// ���� ����
	//
	// Parameter : stSession* ������, Max ��(�ִ� ���� ���� ���� ��)
	// return : ����
	void CMMOServer::SetSession(stSession* pSession, int Max)
	{
		// ���� �迭 ����
		for (int i = 0; i < Max; ++i)
			m_stSessionArray[i] = pSession[i];

	}





	// -----------------------
	// ��ӿ����� ȣ�� ������ �Լ�
	// -----------------------
	
	// ���� ����
	// [���� IP(���ε� �� IP), ��Ʈ, ��Ŀ������ ��, Ȱ��ȭ��ų ��Ŀ������ ��, ����Ʈ ������ ��, TCP_NODELAY ��� ����(true�� ���), �ִ� ������ ��, ��Ŷ Code, XOR 1���ڵ�, XOR 2���ڵ�] �Է¹���.
	//
	// return false : ���� �߻� ��. �����ڵ� ���� �� false ����
	// return true : ����
	bool CMMOServer::Start(const TCHAR* bindIP, USHORT port, int WorkerThreadCount, int ActiveWThreadCount, int AcceptThreadCount, bool Nodelay, int MaxConnect,
		BYTE Code, BYTE XORCode1, BYTE XORCode2)
	{

		// rand����
		srand((UINT)time(NULL));

		// Config ������ ����
		m_bCode = Code;
		m_bXORCode_1 = XORCode1;
		m_bXORCode_2 = XORCode2;

		// ���� �����ϴϱ� �����ڵ�� �ʱ�ȭ
		m_iOSErrorCode = 0;
		m_iMyErrorCode = (euError)0;

		// ���� �ʱ�ȭ
		WSADATA wsa;
		int retval = WSAStartup(MAKEWORD(2, 2), &wsa);
		if (retval != 0)
		{
			// ������ ����, �� ���� ����
			m_iOSErrorCode = retval;
			m_iMyErrorCode = euError::NETWORK_LIB_ERROR__WINSTARTUP_FAIL;

			// ���� �ڵ� ��ȯ �� �������� ����.
			ExitFunc(m_iW_ThreadCount);

			// �α� ��� (�α� ���� : ����)
			cMMOServer_Log->LogSave(L"MMOServer", CSystemLog::en_LogLevel::LEVEL_ERROR, L"Start() --> WSAStartup() Error : NetError(%d), OSError(%d)",
				(int)m_iMyErrorCode, m_iOSErrorCode);

			// false ����
			return false;
		}

		// ����� �Ϸ� ��Ʈ ����
		m_hIOCPHandle = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, NULL, ActiveWThreadCount);
		if (m_hIOCPHandle == NULL)
		{
			// ������ ����, �� ���� ����
			m_iOSErrorCode = WSAGetLastError();
			m_iMyErrorCode = euError::NETWORK_LIB_ERROR__CREATE_IOCP_PORT;

			// ���� �ڵ� ��ȯ �� �������� ����.
			ExitFunc(m_iW_ThreadCount);

			// �α� ��� (�α� ���� : ����)
			cMMOServer_Log->LogSave(L"MMOServer", CSystemLog::en_LogLevel::LEVEL_ERROR, L"Start() --> CreateIoCompletionPort() Error : NetError(%d), OSError(%d)",
				(int)m_iMyErrorCode, m_iOSErrorCode);

			// false ����
			return false;
		}

		// ��Ŀ ������ ����
		m_iW_ThreadCount = WorkerThreadCount;
		m_hWorkerHandle = new HANDLE[WorkerThreadCount];

		for (int i = 0; i < m_iW_ThreadCount; ++i)
		{
			m_hWorkerHandle[i] = (HANDLE)_beginthreadex(0, 0, WorkerThread, this, 0, 0);
			if (m_hWorkerHandle[i] == 0)
			{
				// ������ ����, �� ���� ����
				m_iOSErrorCode = errno;
				m_iMyErrorCode = euError::NETWORK_LIB_ERROR__W_THREAD_CREATE_FAIL;

				// ���� �ڵ� ��ȯ �� �������� ����.
				ExitFunc(i);

				// �α� ��� (�α� ���� : ����)
				cMMOServer_Log->LogSave(L"MMOServer", CSystemLog::en_LogLevel::LEVEL_ERROR, L"Start() --> WorkerThread Create Error : NetError(%d), OSError(%d)",
					(int)m_iMyErrorCode, m_iOSErrorCode);

				// false ����
				return false;
			}
		}

		// ���� ����
		m_soListen_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (m_soListen_sock == INVALID_SOCKET)
		{
			// ������ ����, �� ���� ����
			m_iOSErrorCode = WSAGetLastError();
			m_iMyErrorCode = euError::NETWORK_LIB_ERROR__CREATE_SOCKET_FAIL;

			// ���� �ڵ� ��ȯ �� �������� ����.
			ExitFunc(m_iW_ThreadCount);

			// �α� ��� (�α� ���� : ����)
			cMMOServer_Log->LogSave(L"MMOServer", CSystemLog::en_LogLevel::LEVEL_ERROR, L"Start() --> socket() Error : NetError(%d), OSError(%d)",
				(int)m_iMyErrorCode, m_iOSErrorCode);

			// false ����
			return false;

		}

		// ���ε�
		SOCKADDR_IN serveraddr;
		ZeroMemory(&serveraddr, sizeof(serveraddr));
		serveraddr.sin_family = AF_INET;
		serveraddr.sin_port = htons(port);
		InetPton(AF_INET, bindIP, &serveraddr.sin_addr.s_addr);

		retval = bind(m_soListen_sock, (SOCKADDR*)&serveraddr, sizeof(serveraddr));
		if (retval == SOCKET_ERROR)
		{
			// ������ ����, �� ���� ����
			m_iOSErrorCode = WSAGetLastError();
			m_iMyErrorCode = euError::NETWORK_LIB_ERROR__BINDING_FAIL;

			// ���� �ڵ� ��ȯ �� �������� ����.
			ExitFunc(m_iW_ThreadCount);

			// �α� ��� (�α� ���� : ����)
			cMMOServer_Log->LogSave(L"MMOServer", CSystemLog::en_LogLevel::LEVEL_ERROR, L"Start() --> bind() Error : NetError(%d), OSError(%d)",
				(int)m_iMyErrorCode, m_iOSErrorCode);

			// false ����
			return false;
		}

		// ����
		retval = listen(m_soListen_sock, SOMAXCONN);
		if (retval == SOCKET_ERROR)
		{
			// ������ ����, �� ���� ����
			m_iOSErrorCode = WSAGetLastError();
			m_iMyErrorCode = euError::NETWORK_LIB_ERROR__LISTEN_FAIL;

			// ���� �ڵ� ��ȯ �� �������� ����.
			ExitFunc(m_iW_ThreadCount);

			// �α� ��� (�α� ���� : ����)
			cMMOServer_Log->LogSave(L"MMOServer", CSystemLog::en_LogLevel::LEVEL_ERROR, L"Start() --> listen() Error : NetError(%d), OSError(%d)",
				(int)m_iMyErrorCode, m_iOSErrorCode);

			// false ����
			return false;
		}		

		// ���ڷ� ���� ������� �ɼ� ��� ���ο� ���� ���̱� �ɼ� ����
		// �̰� true�� ������� ����ϰڴٴ� ��(���̱� �������Ѿ���)
		if (Nodelay == true)
		{
			BOOL optval = TRUE;
			retval = setsockopt(m_soListen_sock, IPPROTO_TCP, TCP_NODELAY, (char*)&optval, sizeof(optval));

			if (retval == SOCKET_ERROR)
			{
				// ������ ����, �� ���� ����
				m_iOSErrorCode = WSAGetLastError();
				m_iMyErrorCode = euError::NETWORK_LIB_ERROR__SOCKOPT_FAIL;

				// ���� �ڵ� ��ȯ �� �������� ����.
				ExitFunc(m_iW_ThreadCount);

				// �α� ��� (�α� ���� : ����)
				cMMOServer_Log->LogSave(L"MMOServer", CSystemLog::en_LogLevel::LEVEL_ERROR, L"Start() --> setsockopt() Nodelay apply Error : NetError(%d), OSError(%d)",
					(int)m_iMyErrorCode, m_iOSErrorCode);

				// false ����
				return false;
			}
		}

		// �ִ� ���� ���� ���� �� ����
		m_iMaxJoinUser = MaxConnect;

		// �̻�� ���� ���� ���� �����Ҵ�. (������ ����) 
		// �׸��� �̸� Max��ŭ �����α�
		m_stEmptyIndexStack = new CLF_Stack<WORD>;
		for (int i = 0; i < MaxConnect; ++i)
			m_stEmptyIndexStack->Push(i);

		// ����Ʈ ������ ����
		m_iA_ThreadCount = AcceptThreadCount;
		m_hAcceptHandle = new HANDLE[m_iA_ThreadCount];
		for (int i = 0; i < m_iA_ThreadCount; ++i)
		{
			m_hAcceptHandle[i] = (HANDLE)_beginthreadex(NULL, 0, AcceptThread, this, 0, NULL);
			if (m_hAcceptHandle == NULL)
			{
				// ������ ����, �� ���� ����
				m_iOSErrorCode = errno;
				m_iMyErrorCode = euError::NETWORK_LIB_ERROR__A_THREAD_CREATE_FAIL;

				// ���� �ڵ� ��ȯ �� �������� ����.
				ExitFunc(m_iW_ThreadCount);

				// �α� ��� (�α� ���� : ����)
				cMMOServer_Log->LogSave(L"MMOServer", CSystemLog::en_LogLevel::LEVEL_ERROR, L"Start() --> Accept Thread Create Error : NetError(%d), OSError(%d)",
					(int)m_iMyErrorCode, m_iOSErrorCode);

				// false ����
				return false;
			}
		}



		// ���� ������ !!
		m_bServerLife = true;

		// ���� ���� �α� ���		
		// �̰�, ��ӹ޴� �ʿ��� ��°ɷ� ����. �ݼ��� ��ü�� ���������� �۵����� ����.
		//cNetLibLog->LogSave(L"MMOServer", CSystemLog::en_LogLevel::LEVEL_SYSTEM, L"ServerOpen...");

		return true;
	}


	// ���� ��ž.
	void CMMOServer::Stop()
	{
		// 1. Accept ������ ����. �� �̻� ������ ������ �ȵǴ� Accept������ ���� ����
		// Accept ������� ���������� closesocket�ϸ� �ȴ�.
		closesocket(m_soListen_sock);

		// Accept������ ���� ���
		DWORD retval = WaitForMultipleObjects(m_iA_ThreadCount, m_hAcceptHandle, TRUE, INFINITE);

		// ���ϰ��� [WAIT_OBJECT_0 ~ WAIT_OBJECT_0 + m_iW_ThreadCount - 1] �� �ƴ϶��, ���� ������ �߻��� ��. ���� ��´�
		if (retval < WAIT_OBJECT_0 &&
			retval > WAIT_OBJECT_0 + m_iW_ThreadCount - 1)
		{
			// ���� ���� WAIT_FAILED�� ���, GetLastError()�� Ȯ���ؾ���.
			if (retval == WAIT_FAILED)
				m_iOSErrorCode = GetLastError();

			// �װ� �ƴ϶�� ���ϰ��� �̹� ������ �������.
			else
				m_iOSErrorCode = retval;

			m_iMyErrorCode = euError::NETWORK_LIB_ERROR__WFSO_ERROR;

			// �α� ��� (�α� ���� : ����)
			cMMOServer_Log->LogSave(L"NetServer", CSystemLog::en_LogLevel::LEVEL_ERROR, L"Stop() --> Accept Thread EXIT Error : NetError(%d), OSError(%d)",
				(int)m_iMyErrorCode, m_iOSErrorCode);

			// ���� �߻� �Լ� ȣ��
			OnError((int)euError::NETWORK_LIB_ERROR__WFSO_ERROR, L"Stop() --> Accept Thread EXIT Error");
		}

		// 2. ��� �������� Shutdown
		// ��� �������� �˴ٿ� ����
		for (int i = 0; i < m_iMaxJoinUser; ++i)
		{
			shutdown(m_stSessionArray[i].m_Client_sock, SD_BOTH);


			/*if (m_stSessionArray[i].m_lReleaseFlag == FALSE)
			{
				shutdown(m_stSessionArray[i].m_Client_sock, SD_BOTH);
			}*/
		}

		// ��� ������ ����Ǿ����� üũ
		while (1)
		{
			if (m_ullJoinUserCount == 0)
				break;

			Sleep(1);
		}

		// 3. ��Ŀ ������ ����
		for (int i = 0; i < m_iW_ThreadCount; ++i)
			PostQueuedCompletionStatus(m_hIOCPHandle, 0, 0, 0);

		// ��Ŀ������ ���� ���
		retval = WaitForMultipleObjects(m_iW_ThreadCount, m_hWorkerHandle, TRUE, INFINITE);

		// ���ϰ��� [WAIT_OBJECT_0 ~ WAIT_OBJECT_0 + m_iW_ThreadCount - 1] �� �ƴ϶��, ���� ������ �߻��� ��. ���� ��´�
		if (retval < WAIT_OBJECT_0 &&
			retval > WAIT_OBJECT_0 + m_iW_ThreadCount - 1)
		{
			// ���� ���� WAIT_FAILED�� ���, GetLastError()�� Ȯ���ؾ���.
			if (retval == WAIT_FAILED)
				m_iOSErrorCode = GetLastError();

			// �װ� �ƴ϶�� ���ϰ��� �̹� ������ �������.
			else
				m_iOSErrorCode = retval;

			// �� ���� ����
			m_iMyErrorCode = euError::NETWORK_LIB_ERROR__W_THREAD_ABNORMAL_EXIT;

			// �α� ��� (�α� ���� : ����)
			cMMOServer_Log->LogSave(L"NetServer", CSystemLog::en_LogLevel::LEVEL_ERROR, L"Stop() --> Worker Thread EXIT Error : NetError(%d), OSError(%d)",
				(int)m_iMyErrorCode, m_iOSErrorCode);

			// ���� �߻� �Լ� ȣ��
			OnError((int)euError::NETWORK_LIB_ERROR__W_THREAD_ABNORMAL_EXIT, L"Stop() --> Worker Thread EXIT Error");
		}

		// 4. ���� ���ҽ� ��ȯ
		// 1) ����Ʈ ������ �ڵ� ��ȯ
		for (int i = 0; i < m_iA_ThreadCount; ++i)
			CloseHandle(m_hAcceptHandle[i]);

		// 2) ��Ŀ ������ �ڵ� ��ȯ
		for (int i = 0; i < m_iW_ThreadCount; ++i)
			CloseHandle(m_hWorkerHandle[i]);

		// 3) ��Ŀ ������ �迭, ����Ʈ ������ �迭 ��������
		delete[] m_hWorkerHandle;
		delete[] m_hAcceptHandle;

		// 4) IOCP�ڵ� ��ȯ
		CloseHandle(m_hIOCPHandle);

		// 5) ���� ����
		WSACleanup();

		// 6) ���� �迭, ���� �̻�� �ε��� ���� ���� ��������
		delete[] m_stSessionArray;
		delete m_stEmptyIndexStack;

		// 5. ���� ������ �ƴ� ���·� ����
		m_bServerLife = false;

		// 6. ���� ���� �ʱ�ȭ
		Reset();

		// 7. ���� ���� �α� ���		
		cMMOServer_Log->LogSave(L"MMOServer", CSystemLog::en_LogLevel::LEVEL_SYSTEM, L"ServerStop...");

	}



}