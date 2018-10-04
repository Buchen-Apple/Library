#include "pch.h"

#pragma comment(lib,"ws2_32")
#include <Ws2tcpip.h>

#include <process.h>
#include <strsafe.h>

#include "NetworkLib_MMOServer.h"
#include "Log\Log.h"
#include "CrashDump\CrashDump.h"


ULONGLONG g_ullAcceptTotal_MMO;
LONG	  g_lAcceptTPS_MMO;
LONG	g_lSendPostTPS_MMO;


// ------------------------
// cSession�� �Լ�
// (MMOServer�� �̳�Ŭ����)
// ------------------------
namespace Library_Jingyu
{
	// ������
	CMMOServer::cSession::cSession()
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

	// �Ҹ���
	CMMOServer::cSession::~cSession()
	{
		delete m_SendQueue;
		delete m_CRPacketQueue;
	}


	// ������ ������ ���� �� ȣ���ϴ� �Լ�. �ܺ� ���� ���.
	// ���̺귯������ ������!��� ��û�ϴ� �� ��
	//
	// Parameter : ����
	// return : ����
	void CMMOServer::cSession::Disconnect()
	{
		// �ش� ������ ����,�˴ٿ� ������.
		// ���� �ڿ������� I/Oī��Ʈ�� ���ҵǾ InDisconnect�ȴ�.
		shutdown(m_Client_sock, SD_BOTH);
	}

	// �ܺο���, � �����͸� ������ ������ ȣ���ϴ� �Լ�.
	// SendPacket�� �׳� �ƹ����� �ϸ� �ȴ�.
	// �ش� ������ SendQ�� �־�״ٰ� ���� �Ǹ� ������.
	//
	// Parameter : SessionID, SendBuff, LastFlag(����Ʈ FALSE)
	// return : ����
	void CMMOServer::cSession::SendPacket(CProtocolBuff_Net* payloadBuff, LONG LastFlag)
	{
		// 1. m_LastPacket�� nullptr�� �ƴ϶��, ������ ���� ����. ť�� ���� �ʴ´�.
		if (m_LastPacket != nullptr)
		{
			CProtocolBuff_Net::Free(payloadBuff);
			return;
		}

		// 2. ���ڷ� ���� Flag�� true���, ������ ��Ŷ�� �ּҸ� ����
		if (LastFlag == TRUE)
			m_LastPacket = payloadBuff;

		// 3. ����� �־, ��Ŷ �ϼ��ϱ�		
		payloadBuff->Encode(m_bCode, m_bXORCode_1, m_bXORCode_2);

		// 4. ��ť. ��Ŷ�� "�ּ�"�� ��ť�Ѵ�(8����Ʈ)
		// ����, ������ ����� ��Ŷ ���۷��� ī��Ʈ ������Ű�� �� ���
		m_SendQueue->Enqueue(payloadBuff);
	}

	// �ش� ������ ��带 GAME���� �����ϴ� �Լ�
	//
	// Parameter : ����
	// return : ����
	void CMMOServer::cSession::SetMode_GAME()
	{
		m_lAuthToGameFlag = TRUE;
	}

}



// ------------------------
// MMOServer
// ------------------------
namespace Library_Jingyu
{

#define _MyCountof(_array)		sizeof(_array) / (sizeof(_array[0]))

	// �α� ���� �������� �ϳ� �ޱ�.
	CSystemLog* cMMOServer_Log = CSystemLog::GetInstance();

	// ���� ���� ���� �ϳ� �ޱ�
	CCrashDump* cMMOServer_Dump = CCrashDump::GetInstance();

	// ��� ������
#define dfNETWORK_PACKET_HEADER_SIZE_NETSERVER	5



		// ��� ����ü
#pragma pack(push, 1)
	struct CMMOServer::stProtocolHead
	{
		BYTE	m_Code;
		WORD	m_Len;
		BYTE	m_RandXORCode;
		BYTE	m_Checksum;
	};
#pragma pack(pop)
	  

	// -----------------------
	// �����ڿ� �Ҹ���
	// -----------------------

	// ������
	CMMOServer::CMMOServer()
	{
		// ���� �������� false�� ���� 
		m_bServerLife = false;

		// Auth���� ������ �ϰ� Pool �����Ҵ�
		m_pAcceptPool = new CMemoryPoolTLS<stAuthWork>(0, false);

		// Auth���� ������ �ϰ� ���� ť
		m_pASQ = new CLF_Queue< stAuthWork*>(0);

		// �̻�� �ε��� nullptr�� ����.
		m_stEmptyIndexStack = nullptr;

		// ������ ���� 0���� ����
		m_iA_ThreadCount = 0;
		m_iW_ThreadCount = 0;
		m_iS_ThreadCount = 0;
		m_iAuth_ThreadCount = 0;
		m_iGame_ThreadCount = 0;

		// Auth ������ �����, Game ������ ����� �̺�Ʈ
		// 
		// �ڵ� ���� Event 
		// ���� ���� �� non-signalled ����
		// �̸� ���� Event	
		m_hAuthExitEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
		m_hGameExitEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
		m_hSendExitEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	}

	// �Ҹ���
	CMMOServer::~CMMOServer()
	{
		// ������ �������̾�����, ���� �������� ����
		if (m_bServerLife == true)
			Stop();

		// ����� �̺�Ʈ ����
		CloseHandle(m_hAuthExitEvent);
		CloseHandle(m_hGameExitEvent);
		CloseHandle(m_hSendExitEvent);
	}
	



	// -----------------------
	// �ܺο��� ��� ������ �Լ�
	// -----------------------

	// ���� �۵������� Ȯ��
	//
	// Parameter : ����
	// return : �۵����� �� true.
	bool CMMOServer::GetServerState()
	{
		return m_bServerLife;
	}




	// -----------------------
	// ���ο����� ����ϴ� �Լ�
	// -----------------------

	// Start���� ������ �� �� ȣ���ϴ� �Լ�.
	//
	// Parameter : ��Ŀ�������� ��
	// return : ����
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

		// 4. ���� ������ ����, �ڵ� ��ȯ
		if (m_iS_ThreadCount > 0)
			CloseHandle(m_hSendHandle);

		// 5. � ������ ����, �ڵ� ��ȯ
		if (m_iAuth_ThreadCount > 0)
			CloseHandle(m_hAuthHandle);

		// 6. ���� ������ ����, �ڵ� ��ȯ
		if (m_iGame_ThreadCount > 0)
			CloseHandle(m_hGameHandle);

		// 7. �������� �ݱ�
		if (m_soListen_sock != NULL)
			closesocket(m_soListen_sock);

		// 8. �̻�� �ε��� ��������.
		if (m_stEmptyIndexStack != nullptr)
			delete m_stEmptyIndexStack;

		// 9. ���� ����
		// ���� �ʱ�ȭ ���� ���� ���¿��� WSAClenup() ȣ���ص� �ƹ� ���� ����
		WSACleanup();
	}

	// ���� �������� ���½�Ų��.
	// Stop() �Լ����� ���.
	void CMMOServer::Reset()
	{
		// �̻�� �ε��� nullptr�� ����.
		m_stEmptyIndexStack = nullptr;

		// ������ ���� 0���� ����
		m_iA_ThreadCount = 0;
		m_iW_ThreadCount = 0;
		m_iS_ThreadCount = 0;
		m_iAuth_ThreadCount = 0;
		m_iGame_ThreadCount = 0;

		m_hIOCPHandle = 0;
		m_soListen_sock = 0;
		m_iMaxJoinUser = 0;
		m_ullJoinUserCount = 0;
	}

	// ���ο��� ������ ������ ���� �Լ�.
	//
	// Parameter : ���� ���� ������
	// return : ����
	void CMMOServer::InDisconnect(cSession* DeleteSession)
	{
		// ���� ID �ʱ�ȭ
		DeleteSession->m_ullSessionID = 0xffffffffffffffff;

		// �ش� ������ 'Send ����ȭ ����(Send�ߴ� ����ȭ ���� ����. ���� �Ϸ����� ������ ����ȭ���۵�)'�� �ִ� �����͸� Free�Ѵ�.
		for (int i = 0; i < DeleteSession->m_iWSASendCount; ++i)
			CProtocolBuff_Net::Free(DeleteSession->m_PacketArray[i]);

		// SendCount �ʱ�ȭ
		DeleteSession->m_iWSASendCount = 0;

		// CompleteRecvPacket ����
		int UseSize = DeleteSession->m_CRPacketQueue->GetNodeSize();

		CProtocolBuff_Net* DeletePacket;
		int i = 0;
		while (i < UseSize)
		{
			// ��ť ��, ����ȭ ���� �޸�Ǯ�� Free�Ѵ�.
			if (DeleteSession->m_CRPacketQueue->Dequeue(DeletePacket) == -1)
				cMMOServer_Dump->Crash();

			CProtocolBuff_Net::Free(DeletePacket);

			i++;
		}

		// ���� ť ����
		UseSize = DeleteSession->m_SendQueue->GetInNode();

		i = 0;
		while (i < UseSize)
		{
			// ��ť ��, ����ȭ ���� �޸�Ǯ�� Free�Ѵ�.
			if (DeleteSession->m_SendQueue->Dequeue(DeletePacket) == -1)
				cMMOServer_Dump->Crash();

			CProtocolBuff_Net::Free(DeletePacket);

			i++;
		}

		// ���ú� ť �ʱ�ȭ
		DeleteSession->m_RecvQueue.ClearBuffer();

		// SendFlag �ʱ�ȭ
		DeleteSession->m_lSendFlag = FALSE;		

		// ��� �ʱ�ȭ
		DeleteSession->m_euMode = euSessionModeState::MODE_NONE;

		// �α׾ƿ� Flag �ʱ�ȭ
		DeleteSession->m_lLogoutFlag = FALSE;

		// Auth TO Game Flag �ʱ�ȭ
		DeleteSession->m_lAuthToGameFlag = FALSE;		

		// ���� �� ���� �� ����
		InterlockedDecrement(&m_ullJoinUserCount);

		// Ŭ���� ����
		closesocket(DeleteSession->m_Client_sock);

		// ������ �Ǿ��ٰ� �˷��ش�.
		DeleteSession->OnGame_ClientRelease();

		// �̻�� �ε��� ���ÿ� �ݳ�
		m_stEmptyIndexStack->Push(DeleteSession->m_lIndex);

		return;
	}
	
	// RecvProc �Լ�.
	// �ϼ��� ��Ŷ�� ���ڷι��� Session�� CompletionRecvPacekt (Queue)�� �ִ´�.
	//
	// Parameter : ���� ������
	// return : ����
	void CMMOServer::RecvProc(cSession* NowSession)
	{
		// -----------------
		// Recv ť ���� ó��
		// -----------------

		while (1)
		{
			// 1. RecvBuff�� �ּ����� ����� �ִ��� üũ. (���� = ��� ������� ���ų� �ʰ�. ��, �ϴ� �����ŭ�� ũ�Ⱑ �ִ��� üũ)	
			// ����� ���� -----------> [Code(1byte) - Len(2byte) - Rand XOR Code(1byte) - CheckSum(1byte)] 
			stProtocolHead Header;

			// RecvBuff�� ��� ���� ���� ũ�Ⱑ ��� ũ�⺸�� �۴ٸ�, �Ϸ� ��Ŷ�� ���ٴ� ���̴� while�� ����.
			int UseSize = NowSession->m_RecvQueue.GetUseSize();
			if (UseSize < dfNETWORK_PACKET_HEADER_SIZE_NETSERVER)
			{
				break;
			}

			// 2. ����� Peek���� Ȯ���Ѵ�. 
			// ���۰� ����ִ°� ���� �ȵȴ�. �̹� ������ �ִٰ� �˻��߱� ������. Crash ����
			if (NowSession->m_RecvQueue.Peek((char*)&Header, dfNETWORK_PACKET_HEADER_SIZE_NETSERVER) == -1)
				cMMOServer_Dump->Crash();

			// 3. ����� �ڵ� Ȯ��. �� ���� �´���
			if (Header.m_Code != m_bCode)
			{
				// �� ���� ����. ������ ������ ����.
				m_iMyErrorCode = euError::NETWORK_LIB_ERROR__RECV_CODE_ERROR;

				// ���� ��Ʈ�� �����
				TCHAR tcErrorString[300];
				StringCchPrintf(tcErrorString, 300, _T("RecvRingBuff_CodeError.UserID : %lld, [%s:%d]"),
					NowSession->m_ullSessionID, NowSession->m_IP, NowSession->m_prot);

				// ���� �Լ� ȣ��
				OnError((int)euError::NETWORK_LIB_ERROR__RECV_CODE_ERROR, tcErrorString);

				// �˴ٿ� ȣ��
				shutdown(NowSession->m_Client_sock, SD_BOTH);

				// ������ ���� �����̴� ���� �ƹ��͵� ���ϰ� ����
				return;
			}

			// 4. ��� �ȿ� ����ִ� Len(���̷ε� ����)�� �ʹ� ũ�� ������
			WORD PayloadLen = Header.m_Len;
			if (PayloadLen > 512)
			{
				// �� ���� ����. ������ ������ ����.
				m_iMyErrorCode = euError::NETWORK_LIB_ERROR__RECV_LENBIG_ERROR;

				// ���� ��Ʈ�� �����
				TCHAR tcErrorString[300];
				StringCchPrintf(tcErrorString, 300, _T("RecvRingBuff_LenBig.UserID : %lld, [%s:%d]"),
					NowSession->m_ullSessionID, NowSession->m_IP, NowSession->m_prot);

				// ���� �Լ� ȣ��
				OnError((int)euError::NETWORK_LIB_ERROR__RECV_LENBIG_ERROR, tcErrorString);

				// �˴ٿ� ȣ��
				shutdown(NowSession->m_Client_sock, SD_BOTH);

				// ������ ���� �����̴� ���� �ƹ��͵� ���ϰ� ����
				return;

			}


			// 5. �ϼ��� ��Ŷ�� �ִ��� Ȯ��. (�ϼ� ��Ŷ ������ = ��� ������ + ���̷ε� Size)
			// ��� ���, �ϼ� ��Ŷ ����� �ȵǸ� while�� ����.			
			if (UseSize < (dfNETWORK_PACKET_HEADER_SIZE_NETSERVER + PayloadLen))
				break;


			// 6. RecvBuff���� Peek�ߴ� ����� ����� (�̹� Peek������, �׳� Remove�Ѵ�)
			NowSession->m_RecvQueue.RemoveData(dfNETWORK_PACKET_HEADER_SIZE_NETSERVER);


			// 7. ����ȭ ������ rear�� ������ 5����(�տ� 5����Ʈ�� �������)���� �����Ѵ�.
			// ������ clear()�� �̿��� rear�� 0���� �����д�.
			CProtocolBuff_Net* PayloadBuff = CProtocolBuff_Net::Alloc();
			PayloadBuff->Clear();


			// 8. RecvBuff���� ���̷ε� Size ��ŭ ���̷ε� ����ȭ ���۷� �̴´�. (��ť�̴�. Peek �ƴ�)
			int DequeueSize = NowSession->m_RecvQueue.Dequeue(PayloadBuff->GetBufferPtr(), PayloadLen);

			// ���۰� ����ְų�, ���� ���ϴ¸�ŭ �����Ͱ� �����ٸ�, ���̾ȵ�. (�� if�������� �ִٰ� �ߴµ� ������� ���ٴ°�)
			// ���������� ���� ���� ����.
			if ((DequeueSize == -1) || (DequeueSize != PayloadLen))
				cMMOServer_Dump->Crash();


			// 9. �о�� ��ŭ rear�� �̵���Ų��. 
			PayloadBuff->MoveWritePos(DequeueSize);


			// 10. ��� Decode
			if (PayloadBuff->Decode(PayloadLen, Header.m_RandXORCode, Header.m_Checksum, m_bXORCode_1, m_bXORCode_2) == false)
			{
				// �Ҵ���� ��Ŷ Free
				CProtocolBuff_Net::Free(PayloadBuff);

				// �� ���� ����. ������ ������ ����.
				m_iMyErrorCode = euError::NETWORK_LIB_ERROR__RECV_CHECKSUM_ERROR;

				// ���� ��Ʈ�� �����
				TCHAR tcErrorString[300];
				StringCchPrintf(tcErrorString, 300, _T("RecvRingBuff_Empry.UserID : %lld, [%s:%d]"),
					NowSession->m_ullSessionID, NowSession->m_IP, NowSession->m_prot);

				// ���� �Լ� ȣ��
				OnError((int)euError::NETWORK_LIB_ERROR__RECV_CHECKSUM_ERROR, tcErrorString);

				// ������ϴ� �˴ٿ� ȣ��
				shutdown(NowSession->m_Client_sock, SD_BOTH);

				// ������ ���� �����̴� ���� �ƹ��͵� ���ϰ� ����
				return;
			}


			// 11. �ϼ��� ��Ŷ�� CompletionRecvPacekt (Queue)�� �ִ´�.
			NowSession->m_CRPacketQueue->Enqueue(PayloadBuff);
		}

		return;
	}
	
	// RecvPost�Լ�
	//
	// return 0 : ���������� WSARecv() �Ϸ�
	// return 1 : RecvQ�� ���� ����
	// return 2 : I/O ī��Ʈ�� 0�̵Ǿ� ������ ����
	int CMMOServer::RecvPost(cSession* NowSession)
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
		CMMOServer* g_This = (CMMOServer*)lParam;

		DWORD cbTransferred;
		cSession* stNowSession;
		OVERLAPPED* overlapped;

		while (1)
		{
			// ������ �ʱ�ȭ
			cbTransferred = 0;
			stNowSession = nullptr;
			overlapped = nullptr;

			// GQCS �Ϸ� �� �Լ� ȣ��
			g_This->OnWorkerThreadEnd();

			// �񵿱� ����� �Ϸ� ���
			// GQCS ���
			GetQueuedCompletionStatus(g_This->m_hIOCPHandle, &cbTransferred, (PULONG_PTR)&stNowSession, &overlapped, INFINITE);

			// --------------
			// �Ϸ� üũ
			// --------------
			// overlapped�� nullptr�̶��, IOCP ������.
			if (overlapped == NULL)
			{
				// �� �� 0�̸� ������ ����.
				if (cbTransferred == 0 && stNowSession == NULL)
				{
					// printf("��Ŀ ������ ��������\n");

					break;
				}
				// �װ� �ƴϸ� IOCP ���� �߻��� ��

				// ������ ����, �� ���� ����
				g_This->m_iOSErrorCode = GetLastError();
				g_This->m_iMyErrorCode = euError::NETWORK_LIB_ERROR__IOCP_ERROR;

				// ���� ��Ʈ�� �����
				TCHAR tcErrorString[300];
				StringCchPrintf(tcErrorString, 300, L"IOCP_Error --> GQCS return Error : NetError(%d), OSError(%d)",
					(int)g_This->m_iMyErrorCode, g_This->m_iOSErrorCode);

				// OnError ȣ��
				g_This->OnError((int)euError::NETWORK_LIB_ERROR__IOCP_ERROR, tcErrorString);

				break;
			}

			// GQCS ��� �� �Լ�ȣ��
			g_This->OnWorkerThreadBegin();

			// -----------------
			// Recv ����
			// -----------------
			// WSArecv()�� �Ϸ�� ���, ���� �����Ͱ� 0�� �ƴϸ� ���� ó��
			if (&stNowSession->m_overRecvOverlapped == overlapped && cbTransferred > 0)
			{
				// rear �̵�
				stNowSession->m_RecvQueue.MoveWritePos(cbTransferred);

				// 1. ������ ��, ��Ŷ ó��
				g_This->RecvProc(stNowSession);

				// 2. ���ú� �ٽ� �ɱ�.
				// ���ú� ť�� ��á���� ���� ���´�.
				if (g_This->RecvPost(stNowSession) == 1)
				{
					// I/O ī��Ʈ ���ҽ��״µ� 0�̵Ǹ� shutdown�����͵� ���� �α׾ƿ� �÷��� ����
					if (InterlockedDecrement(&stNowSession->m_lIOCount) == 0)
					{
						stNowSession->m_lLogoutFlag = TRUE;
						continue;
					}

					// 0�� �ƴ϶�� ���������� ���� ���´�.
					shutdown(stNowSession->m_Client_sock, SD_BOTH);
					continue;
				}

			}

			// -----------------
			// Send ����
			// -----------------
			// WSAsend()�� �Ϸ�� ���, ���� �����Ͱ� 0�� �ƴϸ� ����ó��
			else if (&stNowSession->m_overSendOverlapped == overlapped && cbTransferred > 0)
			{
				// !! �׽�Ʈ ��¿� !!
				// sendpostTPS �߰�
				InterlockedAdd(&g_lSendPostTPS_MMO, stNowSession->m_iWSASendCount);

				// 1. ������ ���� ������ ��� ����
				if (stNowSession->m_LastPacket != nullptr)
				{
					// ����ȭ ���� ����
					int i = 0;
					bool Flag = false;
					while (i < stNowSession->m_iWSASendCount)
					{
						CProtocolBuff_Net::Free(stNowSession->m_PacketArray[i]);

						// ������ ��Ŷ�� �� ������, falg�� True�� �ٲ۴�.
						if (stNowSession->m_PacketArray[i] == stNowSession->m_LastPacket)
							Flag = true;

						++i;
					}

					stNowSession->m_iWSASendCount = 0;  // ���� ī��Ʈ 0���� ����.

					// ������ �� ������, �ش� ������ ������ ���´�.
					if (Flag == true)
					{
						// I/O ī��Ʈ ���ҽ��״µ� 0�̵Ǹ� shutdown�����͵� ���� �α׾ƿ� �÷��� ����
						if (InterlockedDecrement(&stNowSession->m_lIOCount) == 0)
						{
							stNowSession->m_lLogoutFlag = TRUE;	
							continue;
						}

						// 0�� �ƴ϶�� ���������� ���� ���´�.
						shutdown(stNowSession->m_Client_sock, SD_BOTH);
						continue;
					}
				}

				// 2. ������ ���� ������ �ƴ� ��� ����
				else
				{
					// ����ȭ ���� ����
					int i = 0;
					while (i < stNowSession->m_iWSASendCount)
					{
						CProtocolBuff_Net::Free(stNowSession->m_PacketArray[i]);
						++i;
					}

					stNowSession->m_iWSASendCount = 0;  // ���� ī��Ʈ 0���� ����.	
				}

				// 3. ���� ���� ���·� ����
				stNowSession->m_lSendFlag = FALSE;
			}

			// -----------------
			// I/Oī��Ʈ ���� �� ���� ó��
			// -----------------
			// I/Oī��Ʈ ���� ��, 0�̶������ �α׾ƿ� �÷��� ����
			if (InterlockedDecrement(&stNowSession->m_lIOCount) == 0)
				stNowSession->m_lLogoutFlag = TRUE;
		}

		return 0;
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

		// �ִ� ���� ���� ����, ���÷� �޾Ƶα�
		int iMaxUser = g_This->m_iMaxJoinUser;

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

		return 0;
	}

	// Auth ������
	UINT	WINAPI	CMMOServer::AuthThread(LPVOID lParam)
	{
		CMMOServer* g_This = (CMMOServer*)lParam;

		// ������ ������ �� ���� 1�� �����ϴ� ������ Ű.
		ULONGLONG ullUniqueSessionID = 0;

		// �ε��� ���� ����
		WORD wIndex;		

		// AUTH��忡�� �� ���� ó���ϴ� ��Ŷ�� ��. 
		const int PACKET_WORK_COUNT = 1;

		// �ִ� ���� ���� ����, ���÷� �޾Ƶα�
		int iMaxUser = g_This->m_iMaxJoinUser;

		while (1)
		{
			// ------------------
			// 1�̸� �����帶�� 1ȸ�� �Ͼ��.
			// ------------------	
			DWORD Ret = WaitForSingleObject(g_This->m_hAuthExitEvent, 1);

			// �̻��� ��ȣ��� 
			if (Ret == WAIT_FAILED)
			{
				DWORD Error = GetLastError();
				printf("AuthThread Exit Error!!! (%d) \n", Error);
				break;
			}

			// ���� ����
			else if (Ret == WAIT_OBJECT_0)
				break;



			// ------------------
			// Part 1. �ű� ������ ��Ŷ ó��
			// ------------------
			int Size = g_This->m_pASQ->GetInNode();

			int i = 0;
			stAuthWork* NowWork;

			// �ϰ�ť�� ���� �������� ���� �Ѵ�.
			while (i < Size)
			{				
				// 1. �ϰ� ��ť
				if (g_This->m_pASQ->Dequeue(NowWork) == -1)
					cMMOServer_Dump->Crash();				

				// 2. �ִ� ������ �� �̻� ���� �Ұ�
				if (iMaxUser <= g_This->m_ullJoinUserCount)
				{
					closesocket(NowWork->m_clienet_Socket);

					// NowWork ��ȯ
					g_This->m_pAcceptPool->Free(NowWork);

					// �� ���� ����
					g_This->m_iMyErrorCode = euError::NETWORK_LIB_ERROR__JOIN_USER_FULL;

					// ���� ��Ʈ�� �����
					TCHAR tcErrorString[300];
					StringCchPrintf(tcErrorString, 300, L"AuthThread(). User Full!!!! (%lld)",
						g_This->m_ullJoinUserCount);

					// ���� �߻� �Լ� ȣ��
					g_This->OnError((int)euError::NETWORK_LIB_ERROR__JOIN_USER_FULL, tcErrorString);

					// continue
					continue;
				}

				// 3. ���� ����, IP���� �Ǵ��ؼ� ���� �߰� �۾��� �ʿ��� ��찡 ���� ���� ������ ȣ��	
				// false�� ���Ӱź�, 
				// true�� ���� ��� ����. true���� �Ұ� ������ OnConnectionRequest�Լ� ���ڷ� ������ ������.
				if (g_This->OnConnectionRequest(NowWork->m_tcIP, NowWork->m_usPort) == false)
				{
					// NowWork ��ȯ
					g_This->m_pAcceptPool->Free(NowWork);

					continue;
				}				

				// 4. ���� ����ü ���� �� ����
				// 1) �̻�� �ε��� �˾ƿ���
				wIndex = g_This->m_stEmptyIndexStack->Pop();

				// 2) I/O ī��Ʈ ����
				// ���� ���
				InterlockedIncrement(&g_This->m_stSessionArray[wIndex]->m_lIOCount);

				// 3) ���� �����ϱ�
				// -- ���� ID(�ͽ� Ű)�� �ε��� �Ҵ�
				ULONGLONG MixKey = ((ullUniqueSessionID << 16) | wIndex);
				ullUniqueSessionID++;

				g_This->m_stSessionArray[wIndex]->m_ullSessionID = MixKey;
				g_This->m_stSessionArray[wIndex]->m_lIndex = wIndex;

				// -- LastPacket �ʱ�ȭ
				g_This->m_stSessionArray[wIndex]->m_LastPacket = nullptr;

				// -- ����
				g_This->m_stSessionArray[wIndex]->m_Client_sock = NowWork->m_clienet_Socket;

				// -- IP�� Port
				StringCchCopy(g_This->m_stSessionArray[wIndex]->m_IP, _MyCountof(g_This->m_stSessionArray[wIndex]->m_IP), NowWork->m_tcIP);
				g_This->m_stSessionArray[wIndex]->m_prot = NowWork->m_usPort;

				// 4) ������ ��� �������� Auth ���·� ����
				g_This->m_stSessionArray[wIndex]->m_euMode = euSessionModeState::MODE_AUTH;


				// 5. IOCP ����
				// ���ϰ� IOCP ����
				if (CreateIoCompletionPort((HANDLE)NowWork->m_clienet_Socket, g_This->m_hIOCPHandle, (ULONG_PTR)&g_This->m_stSessionArray[wIndex], 0) == NULL)
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


				// 6. ������ �� ����. InDisconnect������ ���Ǵ� �����̱� ������ ���Ͷ� ���
				InterlockedIncrement(&g_This->m_ullJoinUserCount);

				
				// 7. NowWork �� ������ ��ȯ
				g_This->m_pAcceptPool->Free(NowWork);
							   				

				// 8. ��� ���������� �Ϸ�Ǿ����� ���� �� ó�� �Լ� ȣ��.
				g_This->m_stSessionArray[wIndex]->OnAuth_ClientJoin();


				// 9. �񵿱� ����� ����
				// ���ú� ���۰� ��á���� 1����
				int ret = g_This->RecvPost(&(*g_This->m_stSessionArray[wIndex]));

				// �������״�, I/Oī��Ʈ --. 0�̶�� �α׾ƿ� �÷��� ����.
				if (InterlockedDecrement(&g_This->m_stSessionArray[wIndex]->m_lIOCount) == 0)
					g_This->m_stSessionArray[wIndex]->m_lLogoutFlag = TRUE;

				// I/Oī��Ʈ ���ҽ��״µ� 0�� �ƴ϶��, ret üũ.
				// ret�� 1�̶�� ���� ���´�.
				else if (ret == 1)
					shutdown(g_This->m_stSessionArray[wIndex]->m_Client_sock, SD_BOTH);

				++i;
			}



			// ------------------
			// Part 2. AUTH��� ���ǵ� ��Ŷ ó�� + Logout Flag ó�� 1��
			// ------------------
			i = 0;
			while (i < iMaxUser)
			{
				// �ش� ������ MODE_AUTH���� Ȯ�� ---------------------------
				if (g_This->m_stSessionArray[i]->m_euMode == euSessionModeState::MODE_AUTH)
				{

					// LogOutFlag�� TRUE��� 
					if (g_This->m_stSessionArray[i]->m_lLogoutFlag = TRUE)
					{
						// ��带 MODE_LOGOUT_IN_AUTH�� ����
						g_This->m_stSessionArray[i]->m_euMode = euSessionModeState::MODE_LOGOUT_IN_AUTH;
					}

					// LogOutFlag�� FALSE�� ������ ���� ���� ó��
					else
					{
						// 1. CompleteRecvPacket ť�� ������ Ȯ��.
						int iQSize = g_This->m_stSessionArray[i]->m_CRPacketQueue->GetNodeSize();

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
								if (g_This->m_stSessionArray[i]->m_CRPacketQueue->Dequeue(NowPacket) == -1)
									cMMOServer_Dump->Crash();

								// ��Ŷ ó��
								g_This->m_stSessionArray[i]->OnAuth_Packet(NowPacket);

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
			// Part 4. Logout Flag ó�� 2�� + AUTH���� GAME���� ��� ��ȯ
			// ------------------
			i = 0;
			while (i < iMaxUser)
			{
				// �ش� ������ MODE_LOGOUT_IN_AUTH���� Ȯ�� ---------------------------
				if (g_This->m_stSessionArray[i]->m_euMode == euSessionModeState::MODE_LOGOUT_IN_AUTH)
				{
					// SendFlag�� FALSE���� Ȯ��
					if (InterlockedOr(&g_This->m_stSessionArray[i]->m_lSendFlag, 0) == FALSE)
					{
						// �����ٰ� �˷��ش�.
						// ��� ���� �� �˷��ָ�, ���� �˷��ֱ� ����, GAME�ʿ��� Release�Ǿ� Release�� ���� �� ���ɼ�.
						g_This->m_stSessionArray[i]->OnAuth_ClientLeave();

						// MODE_WAIT_LOGOUT���� ��� ����
						g_This->m_stSessionArray[i]->m_euMode = euSessionModeState::MODE_WAIT_LOGOUT;
					}

					// Ȥ�� Auth TO Game �÷��� Ȯ��
					// AUTH���� GAME���� ��� ��ȯ
					else if (g_This->m_stSessionArray[i]->m_lAuthToGameFlag == TRUE)
					{
						// �����ٰ� �˷��ش�.
						// ��� ���� �� �˷��ָ�, OnAuth_ClientLeave�� ȣ��Ǳ⵵ ����, GAME�ʿ��� ���� OnGame_ClinetJoin �� ���ɼ�
						g_This->m_stSessionArray[i]->OnAuth_ClientLeave();

						// MODE_AUTH_TO_GAME���� ��� ����
						g_This->m_stSessionArray[i]->m_euMode = euSessionModeState::MODE_AUTH_TO_GAME;

					}
				}

				++i;
			}		
					   	
		}

		return 0;		
	}

	// Game ������
	UINT	WINAPI	CMMOServer::GameThread(LPVOID lParam)
	{
		CMMOServer* g_This = (CMMOServer*)lParam;

		// GAME��忡�� �� ���� ó���ϴ� ��Ŷ�� ��. 
		const int PACKET_WORK_COUNT = 1;

		// �ִ� ���� ���� ����, ���÷� �޾Ƶα�
		int iMaxUser = g_This->m_iMaxJoinUser;

		while (1)
		{
			// ------------------
			// 1�̸� �����帶�� 1ȸ�� �Ͼ��.
			// ------------------	
			DWORD Ret = WaitForSingleObject(g_This->m_hGameExitEvent, 1);

			// �̻��� ��ȣ��� 
			if (Ret == WAIT_FAILED)
			{
				DWORD Error = GetLastError();
				printf("GameThread Exit Error!!! (%d) \n", Error);
				break;
			}

			// ���� ����
			else if (Ret == WAIT_OBJECT_0)
				break;


			// ------------------
			// Part 1. Game ���� ��ȯ
			// ------------------
			int i = 0;
			while (i < iMaxUser)
			{
				// ������ MODE_AUTH_TO_GAME ������� Ȯ�� -----------------
				if (g_This->m_stSessionArray[i]->m_euMode == euSessionModeState::MODE_AUTH_TO_GAME)
				{
					// �´ٸ�, OnGame_ClientJoint ȣ��
					g_This->m_stSessionArray[i]->OnGame_ClientJoin();

					// ��� ����
					g_This->m_stSessionArray[i]->m_euMode = euSessionModeState::MODE_GAME;
				}

				++i;
			}


			// ------------------
			// Part 2. Game ��� ���ǵ� ��Ŷ ó�� + Logout Flag ó�� 1��
			// ------------------
			i = 0;
			while (i < iMaxUser)
			{
				// ������ MODE_GAME ������� Ȯ�� -----------------
				if (g_This->m_stSessionArray[i]->m_euMode == euSessionModeState::MODE_GAME)
				{
					// LogOutFlag�� TRUE��� 
					if (g_This->m_stSessionArray[i]->m_lLogoutFlag = TRUE)
					{
						// ��带 MODE_LOGOUT_IN_GAME�� ����
						g_This->m_stSessionArray[i]->m_euMode = euSessionModeState::MODE_LOGOUT_IN_GAME;
					}

					// LogOutFlag�� FALSE�� ������ ���� ���� ó��
					else
					{
						// 1. CompleteRecvPacket ť�� ������ Ȯ��.
						int iQSize = g_This->m_stSessionArray[i]->m_CRPacketQueue->GetNodeSize();

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
								if (g_This->m_stSessionArray[i]->m_CRPacketQueue->Dequeue(NowPacket) == -1)
									cMMOServer_Dump->Crash();								

								// ��Ŷ ó��
								g_This->m_stSessionArray[i]->OnGame_Packet(NowPacket);

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
			// Part 3. Game ����� ������Ʈ ó��
			// ------------------
			g_This->OnGame_Update();


			// ------------------
			// Part 4. Logout Flag ó�� 2��
			// ------------------
			i = 0;
			while (i < iMaxUser)
			{
				// ������ MODE_LOGOUT_IN_GAME ������� Ȯ�� -----------------
				if (g_This->m_stSessionArray[i]->m_euMode == euSessionModeState::MODE_LOGOUT_IN_GAME)
				{
					// SendFlag�� FALSE���� Ȯ��
					if (InterlockedOr(&g_This->m_stSessionArray[i]->m_lSendFlag, 0) == FALSE)
					{
						// Game��忡�� �������� �˷��ش�.
						g_This->m_stSessionArray[i]->OnGame_ClientLeave();

						// �´ٸ�, MODE_WAIT_LOGOUT���� ��� ����
						g_This->m_stSessionArray[i]->m_euMode = euSessionModeState::MODE_WAIT_LOGOUT;
					}

				}

				++i;
			}


			// ------------------
			// Part 5. Release ����
			// ------------------
			i = 0;
			while (i < iMaxUser)
			{
				// ������ MODE_WAIT_LOGOUT ������� Ȯ�� -----------------
				if (g_This->m_stSessionArray[i]->m_euMode == euSessionModeState::MODE_WAIT_LOGOUT)
				{	
					// Release
					// ���ο��� OnGame_ClientRelease ȣ��
					g_This->InDisconnect(&(*g_This->m_stSessionArray[i]));
					
				}

				++i;
			}
		}

		return 0;
	}

	// Send ������
	UINT	WINAPI	CMMOServer::SendThread(LPVOID lParam)
	{
		CMMOServer* g_This = (CMMOServer*)lParam;

		// �ִ� ���� ���� ����, ���÷� �޾Ƶα�
		int iMaxUser = g_This->m_iMaxJoinUser;

		while (1)
		{
			// ------------------
			// 1�̸� �����帶�� 1ȸ�� �Ͼ��.
			// ------------------	
			DWORD Ret = WaitForSingleObject(g_This->m_hSendExitEvent, 1);

			// �̻��� ��ȣ��� 
			if (Ret == WAIT_FAILED)
			{
				DWORD Error = GetLastError();
				printf("SendThread Exit Error!!! (%d) \n", Error);
				break;
			}

			// ���� ����
			else if (Ret == WAIT_OBJECT_0)
				break;

			// ------------------
			// ��� ������ ������� ���� �õ�
			// ------------------
			int iArrayIndex = 0;
			while (iArrayIndex < iMaxUser)
			{
				cSession* NowSession = &(*g_This->m_stSessionArray[iArrayIndex]);

				// ------------------
				// send ���� �������� üũ
				// ------------------
				// SendFlag(1������)�� TRUE(2������)�� ����.
				// ���⼭ TRUE�� ���ϵǴ� ����, �̹� NowSession->m_SendFlag�� 1(���� ��)�̾��ٴ� ��.
				if (InterlockedExchange(&NowSession->m_lSendFlag, TRUE) == TRUE)
				{
					++iArrayIndex;
					continue;
				}

				// ------------------
				// ������ ��� Ȯ��
				// ------------------
				// ��尡 Auth Ȥ�� Game�̾�� ��. �ƴϸ� ����� ����.
				if (NowSession->m_euMode != euSessionModeState::MODE_AUTH &&
					NowSession->m_euMode != euSessionModeState::MODE_GAME)
				{
					// WSASend �Ȱɾ��� ������, ���� ���� ���·� �ٽ� ����.
					NowSession->m_lSendFlag = FALSE;

					++iArrayIndex;
					continue;
				}


				// ------------------
				// SendBuff�� �����Ͱ� �ִ��� Ȯ��
				// ------------------
				int UseSize = NowSession->m_SendQueue->GetInNode();
				if (UseSize == 0)
				{
					// WSASend �Ȱɾ��� ������, ���� ���� ���·� �ٽ� ����.
					NowSession->m_lSendFlag = FALSE;

					++iArrayIndex;
					continue;
				}


				// ------------------
				// �����Ͱ� �ִٸ� Send�ϱ�
				// ------------------

				// 1. WSABUF ����.
				WSABUF wsabuf[dfSENDPOST_MAX_WSABUF];

				if (UseSize > dfSENDPOST_MAX_WSABUF)
					UseSize = dfSENDPOST_MAX_WSABUF;

				int iPacketIndex = 0;
				while (iPacketIndex < UseSize)
				{
					if (NowSession->m_SendQueue->Dequeue(NowSession->m_PacketArray[iPacketIndex]) == -1)
						cMMOServer_Dump->Crash();

					wsabuf[iPacketIndex].buf = NowSession->m_PacketArray[iPacketIndex]->GetBufferPtr();
					wsabuf[iPacketIndex].len = NowSession->m_PacketArray[iPacketIndex]->GetUseSize();

					iPacketIndex++;
				}

				NowSession->m_iWSASendCount = UseSize;

				// 2. Overlapped ����ü �ʱ�ȭ
				ZeroMemory(&NowSession->m_overSendOverlapped, sizeof(NowSession->m_overSendOverlapped));

				// 3. WSASend()
				DWORD SendBytes = 0, flags = 0;
				InterlockedIncrement(&NowSession->m_lIOCount);

				// 4. ���� ó��
				if (WSASend(NowSession->m_Client_sock, wsabuf, UseSize, &SendBytes, flags, &NowSession->m_overSendOverlapped, NULL) == SOCKET_ERROR)
				{
					int Error = WSAGetLastError();

					// �񵿱� ������� ���۵Ȱ� �ƴ϶��
					if (Error != WSA_IO_PENDING)
					{
						if (InterlockedDecrement(&NowSession->m_lIOCount) == 0)
						{
							// �α׾ƿ� �÷��� ����
							NowSession->m_lLogoutFlag = TRUE;
						}

						// ������ ���� �����̶��
						if (Error == WSAENOBUFS)
						{
							// �� ����, �����쿡�� ����
							g_This->m_iOSErrorCode = Error;
							g_This->m_iMyErrorCode = euError::NETWORK_LIB_ERROR__WSAENOBUFS;

							// ���� ��Ʈ�� �����
							TCHAR tcErrorString[300];
							StringCchPrintf(tcErrorString, 300, _T("WSANOBUFS. UserID : %lld, [%s:%d]"),
								NowSession->m_ullSessionID, NowSession->m_IP, NowSession->m_prot);

							// �α� ��� (�α� ���� : ����)
							cMMOServer_Log->LogSave(L"NetServer", CSystemLog::en_LogLevel::LEVEL_ERROR, L"WSASend --> %s : NetError(%d), OSError(%d)",
								tcErrorString, (int)g_This->m_iMyErrorCode, g_This->m_iOSErrorCode);

							// ���� �Լ� ȣ��
							g_This->OnError((int)euError::NETWORK_LIB_ERROR__WSAENOBUFS, tcErrorString);

							// ���´�.
							shutdown(NowSession->m_Client_sock, SD_BOTH);
						}
					}
				}

				++iArrayIndex;
			}
					
		}

		return 0;
	}

	




	// -----------------------
	// ��� ���迡���� ȣ�� ������ �Լ�
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

		// �ִ� ���� ���� ���� �� ����
		m_iMaxJoinUser = MaxConnect;

		// �̻�� ���� ���� ���� �����Ҵ�. (������ ����) 
		// �׸��� �̸� Max��ŭ �����α�
		m_stEmptyIndexStack = new CLF_Stack<WORD>;
		for (int i = 0; i < MaxConnect; ++i)
			m_stEmptyIndexStack->Push(i);


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
				m_iMyErrorCode = euError::NETWORK_LIB_ERROR__THREAD_CREATE_FAIL;

				// ���� �ڵ� ��ȯ �� �������� ����.
				ExitFunc(i);

				// �α� ��� (�α� ���� : ����)
				cMMOServer_Log->LogSave(L"MMOServer", CSystemLog::en_LogLevel::LEVEL_ERROR, L"Start() --> WorkerThread Create Error : NetError(%d), OSError(%d)",
					(int)m_iMyErrorCode, m_iOSErrorCode);

				// false ����
				return false;
			}
		}	
		

		// Send ������ ����
		m_iS_ThreadCount = 1;
		m_hSendHandle = new HANDLE[m_iS_ThreadCount];
		for (int i = 0; i < m_iS_ThreadCount; ++i)
		{
			m_hAcceptHandle[i] = (HANDLE)_beginthreadex(NULL, 0, SendThread, this, 0, NULL);
			if (m_hAcceptHandle == NULL)
			{
				// ������ ����, �� ���� ����
				m_iOSErrorCode = errno;
				m_iMyErrorCode = euError::NETWORK_LIB_ERROR__THREAD_CREATE_FAIL;

				// ���� �ڵ� ��ȯ �� �������� ����.
				ExitFunc(m_iW_ThreadCount);

				// �α� ��� (�α� ���� : ����)
				cMMOServer_Log->LogSave(L"MMOServer", CSystemLog::en_LogLevel::LEVEL_ERROR, L"Start() --> Send Thread Create Error : NetError(%d), OSError(%d)",
					(int)m_iMyErrorCode, m_iOSErrorCode);

				// false ����
				return false;
			}
		}

		// Auth ������ ����
		m_iAuth_ThreadCount = 1;
		m_hAuthHandle = new HANDLE[m_iAuth_ThreadCount];
		for (int i = 0; i < m_iAuth_ThreadCount; ++i)
		{
			m_hAcceptHandle[i] = (HANDLE)_beginthreadex(NULL, 0, AuthThread, this, 0, NULL);
			if (m_hAcceptHandle == NULL)
			{
				// ������ ����, �� ���� ����
				m_iOSErrorCode = errno;
				m_iMyErrorCode = euError::NETWORK_LIB_ERROR__THREAD_CREATE_FAIL;

				// ���� �ڵ� ��ȯ �� �������� ����.
				ExitFunc(m_iW_ThreadCount);

				// �α� ��� (�α� ���� : ����)
				cMMOServer_Log->LogSave(L"MMOServer", CSystemLog::en_LogLevel::LEVEL_ERROR, L"Start() --> Auth Thread Create Error : NetError(%d), OSError(%d)",
					(int)m_iMyErrorCode, m_iOSErrorCode);

				// false ����
				return false;
			}
		}


		// Game ������ ����
		m_iGame_ThreadCount = 1;
		m_hGameHandle = new HANDLE[m_iGame_ThreadCount];
		for (int i = 0; i < m_iGame_ThreadCount; ++i)
		{
			m_hAcceptHandle[i] = (HANDLE)_beginthreadex(NULL, 0, GameThread, this, 0, NULL);
			if (m_hAcceptHandle == NULL)
			{
				// ������ ����, �� ���� ����
				m_iOSErrorCode = errno;
				m_iMyErrorCode = euError::NETWORK_LIB_ERROR__THREAD_CREATE_FAIL;

				// ���� �ڵ� ��ȯ �� �������� ����.
				ExitFunc(m_iW_ThreadCount);

				// �α� ��� (�α� ���� : ����)
				cMMOServer_Log->LogSave(L"MMOServer", CSystemLog::en_LogLevel::LEVEL_ERROR, L"Start() --> Game Thread Create Error : NetError(%d), OSError(%d)",
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
				m_iMyErrorCode = euError::NETWORK_LIB_ERROR__THREAD_CREATE_FAIL;

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
			shutdown(m_stSessionArray[i]->m_Client_sock, SD_BOTH);
		}

		// ��� ������ ����Ǿ����� üũ
		while (1)
		{
			if (m_ullJoinUserCount == 0)
				break;

			Sleep(1);
		}

		// 3. Auth ������ ����
		SetEvent(m_hAuthExitEvent);

		// Auth������ ���� ���
		retval = WaitForMultipleObjects(m_iAuth_ThreadCount, m_hAuthHandle, TRUE, INFINITE);

		// ���ϰ��� [WAIT_OBJECT_0 ~ WAIT_OBJECT_0 + m_iAuth_ThreadCount - 1] �� �ƴ϶��, ���� ������ �߻��� ��. ���� ��´�
		if (retval < WAIT_OBJECT_0 &&
			retval > WAIT_OBJECT_0 + m_iAuth_ThreadCount - 1)
		{
			// ���� ���� WAIT_FAILED�� ���, GetLastError()�� Ȯ���ؾ���.
			if (retval == WAIT_FAILED)
				m_iOSErrorCode = GetLastError();

			// �װ� �ƴ϶�� ���ϰ��� �̹� ������ �������.
			else
				m_iOSErrorCode = retval;

			m_iMyErrorCode = euError::NETWORK_LIB_ERROR__WFSO_ERROR;

			// �α� ��� (�α� ���� : ����)
			cMMOServer_Log->LogSave(L"NetServer", CSystemLog::en_LogLevel::LEVEL_ERROR, L"Stop() --> Auth Thread EXIT Error : NetError(%d), OSError(%d)",
				(int)m_iMyErrorCode, m_iOSErrorCode);

			// ���� �߻� �Լ� ȣ��
			OnError((int)euError::NETWORK_LIB_ERROR__WFSO_ERROR, L"Stop() --> Accept Thread EXIT Error");
		}


		// 4. Game ������ ����
		SetEvent(m_hGameHandle);

		// Game ������ ���� ���
		retval = WaitForMultipleObjects(m_iGame_ThreadCount, m_hGameHandle, TRUE, INFINITE);

		// ���ϰ��� [WAIT_OBJECT_0 ~ WAIT_OBJECT_0 + m_iGame_ThreadCount - 1] �� �ƴ϶��, ���� ������ �߻��� ��. ���� ��´�
		if (retval < WAIT_OBJECT_0 &&
			retval > WAIT_OBJECT_0 + m_iGame_ThreadCount - 1)
		{
			// ���� ���� WAIT_FAILED�� ���, GetLastError()�� Ȯ���ؾ���.
			if (retval == WAIT_FAILED)
				m_iOSErrorCode = GetLastError();

			// �װ� �ƴ϶�� ���ϰ��� �̹� ������ �������.
			else
				m_iOSErrorCode = retval;

			m_iMyErrorCode = euError::NETWORK_LIB_ERROR__WFSO_ERROR;

			// �α� ��� (�α� ���� : ����)
			cMMOServer_Log->LogSave(L"NetServer", CSystemLog::en_LogLevel::LEVEL_ERROR, L"Stop() --> Game Thread EXIT Error : NetError(%d), OSError(%d)",
				(int)m_iMyErrorCode, m_iOSErrorCode);

			// ���� �߻� �Լ� ȣ��
			OnError((int)euError::NETWORK_LIB_ERROR__WFSO_ERROR, L"Stop() --> Accept Thread EXIT Error");
		}

		// 5. Send ������ ����
		SetEvent(m_hSendHandle);

		// Game ������ ���� ���
		retval = WaitForMultipleObjects(m_iS_ThreadCount, m_hSendHandle, TRUE, INFINITE);

		// ���ϰ��� [WAIT_OBJECT_0 ~ WAIT_OBJECT_0 + m_iS_ThreadCount - 1] �� �ƴ϶��, ���� ������ �߻��� ��. ���� ��´�
		if (retval < WAIT_OBJECT_0 &&
			retval > WAIT_OBJECT_0 + m_iS_ThreadCount - 1)
		{
			// ���� ���� WAIT_FAILED�� ���, GetLastError()�� Ȯ���ؾ���.
			if (retval == WAIT_FAILED)
				m_iOSErrorCode = GetLastError();

			// �װ� �ƴ϶�� ���ϰ��� �̹� ������ �������.
			else
				m_iOSErrorCode = retval;

			m_iMyErrorCode = euError::NETWORK_LIB_ERROR__WFSO_ERROR;

			// �α� ��� (�α� ���� : ����)
			cMMOServer_Log->LogSave(L"NetServer", CSystemLog::en_LogLevel::LEVEL_ERROR, L"Stop() --> Send Thread EXIT Error : NetError(%d), OSError(%d)",
				(int)m_iMyErrorCode, m_iOSErrorCode);

			// ���� �߻� �Լ� ȣ��
			OnError((int)euError::NETWORK_LIB_ERROR__WFSO_ERROR, L"Stop() --> Accept Thread EXIT Error");
		}		


		// 6. ��Ŀ ������ ����
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

		// 7. ���� ���ҽ� ��ȯ

		// 1) Accept Socekt Queue ��� ��ȯ
		stAuthWork* DeleteWork;
		for (int i = 0; i < m_pASQ->GetInNode(); ++i)
		{
			m_pASQ->Dequeue(DeleteWork);

			m_pAcceptPool->Free(DeleteWork);
		}

		// 2) ����Ʈ ������ �ڵ� ��ȯ
		for (int i = 0; i < m_iA_ThreadCount; ++i)
			CloseHandle(m_hAcceptHandle[i]);

		// 3) ��Ŀ ������ �ڵ� ��ȯ
		for (int i = 0; i < m_iW_ThreadCount; ++i)
			CloseHandle(m_hWorkerHandle[i]);

		// 4) � ������ �ڵ� ��ȯ
		for (int i = 0; i < m_iAuth_ThreadCount; ++i)
			CloseHandle(m_hAuthHandle[i]);

		// 5) ���� ������ �ڵ� ��ȯ
		for (int i = 0; i < m_iGame_ThreadCount; ++i)
			CloseHandle(m_hGameHandle[i]);

		// 6) ���� ������ �ڵ� ��ȯ
		for (int i = 0; i < m_iS_ThreadCount; ++i)
			CloseHandle(m_hSendHandle[i]);
		
		// 7) ���� ������ �迭 ��������
		delete[] m_hWorkerHandle;
		delete[] m_hAcceptHandle;
		delete[] m_hSendHandle;
		delete[] m_hAuthHandle;
		delete[] m_hGameHandle;

		// 8) IOCP�ڵ� ��ȯ
		CloseHandle(m_hIOCPHandle);

		// 9) ���� ����
		WSACleanup();

		// 10) ���� �̻�� �ε��� ���� ���� ��������
		delete m_stEmptyIndexStack;		

		// 8. ���� ���� �ʱ�ȭ
		Reset();

		// 9. ���� ������ �ƴ� ���·� ����
		m_bServerLife = false;

		// 10. ���� ���� �α� ���		
		cMMOServer_Log->LogSave(L"MMOServer", CSystemLog::en_LogLevel::LEVEL_SYSTEM, L"ServerStop...");
	}
	   
	// ���� ����
	//
	// Parameter : cSession* ������, Max ��(�ִ� ���� ���� ���� ��), HeadCode, XORCode1, XORCode2
	// return : ����
	void CMMOServer::SetSession(cSession* pSession, int Max, BYTE HeadCode, BYTE XORCode1, BYTE XORCode2)
	{
		// ���� �迭 ����
		int iIndex = 0;
		while (iIndex < Max)
		{
			m_stSessionArray[iIndex] = &pSession[iIndex];
			m_stSessionArray[iIndex]->m_bCode = HeadCode;
			m_stSessionArray[iIndex]->m_bXORCode_1 = XORCode1;
			m_stSessionArray[iIndex]->m_bXORCode_2 = XORCode2;

			++iIndex;
		}		

	}
	   	  
}