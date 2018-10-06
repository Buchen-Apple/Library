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
LONG	g_lRecvTPS_MMO;

LONG g_lAuthModeUserCount;
LONG g_lGameModeUserCount;


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


	// Auth �����忡�� ó��
	void CMMOServer::cSession::OnAuth_ClientJoin() {}
	void CMMOServer::cSession::OnAuth_ClientLeave(bool bGame) {}
	void CMMOServer::cSession::OnAuth_Packet(CProtocolBuff_Net* Packet) {}

	// Game �����忡�� ó��
	void CMMOServer::cSession::OnGame_ClientJoin() {}
	void CMMOServer::cSession::OnGame_ClientLeave() {}
	void CMMOServer::cSession::OnGame_Packet(CProtocolBuff_Net* Packet) {}

	// Release��
	void CMMOServer::cSession::OnGame_ClientRelease() {}



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

		// ���� �迭 nullptr�� ����
		m_stSessionArray = nullptr;

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
	// ���� �Լ�
	// -----------------------

	// ���� �۵������� Ȯ��
	//
	// Parameter : ����
	// return : �۵����� �� true.
	bool CMMOServer::GetServerState()
	{
		return m_bServerLife;
	}

	// ���� ���� ���� �� ���
	//
	// Parameter : ����
	// return : ������ �� (ULONGLONG)
	ULONGLONG CMMOServer::GetClientCount()
	{
		return m_ullJoinUserCount;
	}

	// Accept Socket Queue ���� �ϰ� �� ���
	//
	// Parameter : ����
	// return : �ϰ� ��(LONG)
	LONG CMMOServer::GetASQ_Count()
	{
		return m_pASQ->GetInNode();
	}

	// Accept Socket Queue Pool�� �� ûũ �� ���
	// 
	// Parameter : ����
	// return : �� ûũ �� (LONG)
	LONG CMMOServer::GetChunkCount()
	{
		return m_pAcceptPool->GetAllocChunkCount();
	}

	// Accept Socket Queue Pool�� �ۿ��� ������� ûũ �� ���
	// 
	// Parameter : ����
	// return : �ۿ��� ������� ûũ �� (LONG)
	LONG CMMOServer::GetOutChunkCount()
	{
		return m_pAcceptPool->GetOutChunkCount();
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
		// SendFlag�� TRUE�� �����.
		// �ٵ�, �̹� TRUE���ٸ�, ���� Send���̴� �׳� ������.
		if (InterlockedExchange(&DeleteSession->m_lSendFlag, TRUE) == TRUE)		
			return;		

		// ���� ID �ʱ�ȭ
		DeleteSession->m_ullSessionID = 0xffffffffffffffff;

		// �ش� ������ 'Send ����ȭ ����(Send�ߴ� ����ȭ ���� ����. ���� �Ϸ����� ������ ����ȭ���۵�)'�� �ִ� �����͸� Free�Ѵ�.
		int i = 0;
		while (i < DeleteSession->m_iWSASendCount)
		{
			CProtocolBuff_Net::Free(DeleteSession->m_PacketArray[i]);
			++i;
		}


		// SendCount �ʱ�ȭ
		DeleteSession->m_iWSASendCount = 0;

		// CompleteRecvPacket ����
		int UseSize = DeleteSession->m_CRPacketQueue->GetNodeSize();

		CProtocolBuff_Net* DeletePacket;
		i = 0;
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

		// ��� �ʱ�ȭ
		DeleteSession->m_euMode = euSessionModeState::MODE_NONE;		

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
			InterlockedAdd(&g_lRecvTPS_MMO, 1);
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

		// -------------- �ʿ��� ���� �޾Ƶα�

		// ��������
		SOCKET Listen_sock = g_This->m_soListen_sock;

		// Accept Socket Pool
		CMemoryPoolTLS<stAuthWork>* AcceptPool = g_This->m_pAcceptPool;

		// Accept Socket Queue
		CLF_Queue<stAuthWork*>* pASQ = g_This->m_pASQ;

		while (1)
		{
			ZeroMemory(&clientaddr, sizeof(clientaddr));
			client_sock = accept(Listen_sock, (SOCKADDR*)&clientaddr, &addrlen);
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
			stAuthWork* NowWork = AcceptPool->Alloc();



			// ------------------
			// �ϰ��� ���� ����
			// ------------------
			InetNtop(AF_INET, &clientaddr.sin_addr, NowWork->m_tcIP, 30);
			NowWork->m_usPort = ntohs(clientaddr.sin_port);	
			NowWork->m_clienet_Socket = client_sock;


			// Auth���� ����
			pASQ->Enqueue(NowWork);

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

		// --------- �ʿ��� ������ ���÷� �޾Ƶα�

		// AUTH��忡�� 1���� ��������, 1�����ӿ� ó���ϴ� ��Ŷ�� �ִ� ��. 
		const int PACKET_WORK_COUNT = (int)CMMOServer::euDEFINE::eu_AUTH_PACKET_COUNT;

		// �ִ� ���� ���� ����, ���÷� �޾Ƶα�
		const int MAX_USER = g_This->m_iMaxJoinUser;

		// Sleep ���ڷ� �޾Ƶα�
		const int SLEEP_VALUE = (int)CMMOServer::euDEFINE::eu_AUTH_SLEEP;

		// 1�����ӿ� ó���ϴ� Accept Socket Queue�� ��
		const int NEWUSER_PACKET_COUNT = (int)CMMOServer::euDEFINE::eu_AUTH_NEWUSER_PACKET_COUNT;

		// ���� �̺�Ʈ
		HANDLE* ExitEvent = &g_This->m_hAuthExitEvent;

		// Accept Socket Pool
		CMemoryPoolTLS<stAuthWork>* pAcceptPool = g_This->m_pAcceptPool;

		// Accept Socket Queue
		CLF_Queue<stAuthWork*>* pASQ = g_This->m_pASQ;

		// ���� �������� ���� ��
		ULONGLONG* ullJoinUserCount = &g_This->m_ullJoinUserCount;

		// ���� ���� �迭
		cSession** SessionArray = g_This->m_stSessionArray;

		// �̻�� �ε��� ���� ����
		CLF_Stack<WORD>* EmptyIndexStack = g_This->m_stEmptyIndexStack;

		while (1)
		{
			// ------------------
			// iSleepValuea ��ŭ �ڴٰ� �Ͼ��.
			// ------------------	
			DWORD Ret = WaitForSingleObject(*ExitEvent, SLEEP_VALUE);

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
			int iSize = pASQ->GetInNode();

			// ����� NEWUSER_PACKET_COUNT���� ũ�ٸ� NEWUSER_PACKET_COUNT�� �����.
			if (iSize > NEWUSER_PACKET_COUNT)
				iSize = NEWUSER_PACKET_COUNT;

			stAuthWork* NowWork;

			// �ϰ�ť�� ���� �������� ���� �Ѵ�.
			// �ϳ� ó���� �� ���� iSize�� 1�� ������Ų��.
			while (iSize > 0)
			{				
				// 1. �ϰ� ��ť
				if (pASQ->Dequeue(NowWork) == -1)
					cMMOServer_Dump->Crash();				

				// 2. �ִ� ������ �� �̻� ���� �Ұ�
				if (MAX_USER <= *ullJoinUserCount)
				{
					closesocket(NowWork->m_clienet_Socket);

					// NowWork ��ȯ
					pAcceptPool->Free(NowWork);

					// �� ���� ����
					g_This->m_iMyErrorCode = euError::NETWORK_LIB_ERROR__JOIN_USER_FULL;

					// ���� ��Ʈ�� �����
					TCHAR tcErrorString[300];
					StringCchPrintf(tcErrorString, 300, L"AuthThread(). User Full!!!! (%lld)",
						*ullJoinUserCount);

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
					pAcceptPool->Free(NowWork);

					continue;
				}				

				// 4. ���� ����ü ���� �� ����
				// 1) �̻�� �ε��� �˾ƿ���
				wIndex = EmptyIndexStack->Pop();

				// 2) I/O ī��Ʈ ����
				// ���� ���
				InterlockedIncrement(&SessionArray[wIndex]->m_lIOCount);

				// 3) ���� �����ϱ�
				// -- ���� ID(�ͽ� Ű)�� �ε��� �Ҵ�
				ULONGLONG MixKey = ((ullUniqueSessionID << 16) | wIndex);
				ullUniqueSessionID++;

				SessionArray[wIndex]->m_ullSessionID = MixKey;
				SessionArray[wIndex]->m_lIndex = wIndex;

				// -- LastPacket �ʱ�ȭ
				SessionArray[wIndex]->m_LastPacket = nullptr;

				// -- ����
				SessionArray[wIndex]->m_Client_sock = NowWork->m_clienet_Socket;

				// -- IP�� Port
				StringCchCopy(SessionArray[wIndex]->m_IP, _MyCountof(SessionArray[wIndex]->m_IP), NowWork->m_tcIP);
				SessionArray[wIndex]->m_prot = NowWork->m_usPort;

				// -- SendFlag �ʱ�ȭ
				SessionArray[wIndex]->m_lSendFlag = FALSE;				

				// -- �α׾ƿ� Flag �ʱ�ȭ
				SessionArray[wIndex]->m_lLogoutFlag = FALSE;

				// -- Auth TO Game Flag �ʱ�ȭ
				SessionArray[wIndex]->m_lAuthToGameFlag = FALSE;

				// 4) ������ ��� �������� Auth ���·� ����

				// Auth ��� ���� �� ����
				++g_lAuthModeUserCount;

				SessionArray[wIndex]->m_euMode = euSessionModeState::MODE_AUTH;



				// 5. IOCP ����
				// ���ϰ� IOCP ����
				if (CreateIoCompletionPort((HANDLE)NowWork->m_clienet_Socket, g_This->m_hIOCPHandle, (ULONG_PTR)SessionArray[wIndex], 0) == NULL)
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
				InterlockedIncrement(ullJoinUserCount);

				
				// 7. NowWork �� ������ ��ȯ
				pAcceptPool->Free(NowWork);
							   				

				// 8. ��� ���������� �Ϸ�Ǿ����� ���� �� ó�� �Լ� ȣ��.
				SessionArray[wIndex]->OnAuth_ClientJoin();


				// 9. �񵿱� ����� ����
				// ���ú� ���۰� ��á���� 1����
				int ret = g_This->RecvPost(SessionArray[wIndex]);

				// �������״�, I/Oī��Ʈ --. 0�̶�� �α׾ƿ� �÷��� ����.
				if (InterlockedDecrement(&SessionArray[wIndex]->m_lIOCount) == 0)
					SessionArray[wIndex]->m_lLogoutFlag = TRUE;

				// I/Oī��Ʈ ���ҽ��״µ� 0�� �ƴ϶��, ret üũ.
				// ret�� 1�̶�� ���� ���´�.
				else if (ret == 1)
					shutdown(SessionArray[wIndex]->m_Client_sock, SD_BOTH);


				--iSize;
			}



			// ------------------
			// Part 2. AUTH��� ���ǵ� ��Ŷ ó�� + Logout Flag ó��
			// ------------------
			int iIndex = MAX_USER - 1;

			// While�� �ѹ��� iIndex 1 ����.
			// �迭�� ���� �ں��� ó���Ѵ�. (ex. �ִ� ���� ���� 17000�̸�, 16999��° �迭���� ����)
			while (iIndex > 0)
			{
				cSession* NowSession = g_This->m_stSessionArray[iIndex];

				// �ش� ������ MODE_AUTH���� Ȯ�� ---------------------------
				if (NowSession->m_euMode == euSessionModeState::MODE_AUTH)
				{
					// LogOutFlag�� TRUE�̰�, SendFlag�� FALSE���
					if (NowSession->m_lLogoutFlag == TRUE && NowSession->m_lSendFlag == FALSE)
					{
						// Auth ��� ���� �� ����
						--g_lAuthModeUserCount;

						// �����ٰ� �˷��ش�.
						// ��� ���� �� �˷��ָ�, ���� �˷��ֱ� ����, GAME�ʿ��� Release�Ǿ� Release�� ���� �� ���ɼ�.
						NowSession->OnAuth_ClientLeave();

						// MODE_WAIT_LOGOUT���� ��� ����
						NowSession->m_euMode = euSessionModeState::MODE_WAIT_LOGOUT;					
					}

					// LogOutFlag�� FALSE�� ������ ���� ���� ó��
					else
					{
						// 1. CompleteRecvPacket ť�� ������ Ȯ��.
						int iQSize = NowSession->m_CRPacketQueue->GetNodeSize();

						// 2. ť�� ��尡 1�� �̻� ������, ��Ŷ ó��
						if (iQSize > 0)
						{
							// !! ��Ŷ ó�� Ƚ�� ������ �α� ����, PACKET_WORK_COUNT���� ����� ���� �� ������, PACKET_WORK_COUNT�� ���� !!
							if (iQSize > PACKET_WORK_COUNT)
								iQSize = PACKET_WORK_COUNT;

							CProtocolBuff_Net* NowPacket;

							while (iQSize > 0)
							{
								// ��� ����
								if (NowSession->m_CRPacketQueue->Dequeue(NowPacket) == -1)
									cMMOServer_Dump->Crash();

								// ��Ŷ ó��
								NowSession->OnAuth_Packet(NowPacket);

								// ��Ŷ ���۷��� ī��Ʈ 1 ����
								CProtocolBuff_Net::Free(NowPacket);

								// ��Ŷ 1�� ó�������� ���� �� ����.
								--iQSize;
							}
						}
					}
				}				
				

				--iIndex;
			}



			// ------------------
			// Part 3. OnAuth_Update
			// ------------------
			g_This->OnAuth_Update();



			// ------------------
			// Part 4. AUTH���� GAME���� ��� ��ȯ
			// ------------------
			iIndex = MAX_USER - 1;
			while (iIndex > 0)
			{
				cSession* NowSession = g_This->m_stSessionArray[iIndex];

				// �ش� ������ MODE_AUTH���� Ȯ�� ---------------------------
				if (NowSession->m_euMode == euSessionModeState::MODE_AUTH)
				{
					// Auth TO Game �÷��� Ȯ��
					// AUTH���� GAME���� ��� ��ȯ
					if (NowSession->m_lAuthToGameFlag == TRUE)
					{
						// Auth ��� ���� �� ����
						--g_lAuthModeUserCount;

						// �����ٰ� �˷��ش�.
						// ��� ���� �� �˷��ָ�, OnAuth_ClientLeave�� ȣ��Ǳ⵵ ����, GAME�ʿ��� ���� OnGame_ClinetJoin �� ���ɼ�
						NowSession->OnAuth_ClientLeave(true);

						// MODE_AUTH_TO_GAME���� ��� ����
						NowSession->m_euMode = euSessionModeState::MODE_AUTH_TO_GAME;
					}
				}

				--iIndex;
			}		
					   	
		}

		return 0;		
	}

	// Game ������
	UINT	WINAPI	CMMOServer::GameThread(LPVOID lParam)
	{
		CMMOServer* g_This = (CMMOServer*)lParam;

		// ------- �ʿ��� ������ ���ڷ� �޾Ƶα� -------

		// 1�����ӿ�, 1���� ������ �� ���� ��Ŷ�� ó���� ���ΰ�
		const int PACKET_WORK_COUNT = (int)CMMOServer::euDEFINE::eu_GAME_PACKET_COUNT;

		// �ִ� ���� ���� ����, ���÷� �޾Ƶα�
		int MAX_USER = g_This->m_iMaxJoinUser;

		// Sleep ���ڷ� �޾Ƶα�
		const int SLEEP_VALUE = (int)CMMOServer::euDEFINE::eu_GAME_SLEEP;

		// 1������ ����, AUTH_IN_GAME���� GAME���� ����� �� �ִ� ���� ��
		const int NEWUSER_PACKET_COUNT = (int)CMMOServer::euDEFINE::eu_GAME_NEWUSER_JOIN_COUNT;

		// ���� �̺�Ʈ
		HANDLE* ExitEvent = &g_This->m_hGameExitEvent;

		// ���� ���� �迭
		cSession** SessionArray = g_This->m_stSessionArray;

		while (1)
		{
			// ------------------
			// SLEEP_VALUE��ŭ �ڴٰ� �Ͼ��.
			// ------------------	
			DWORD Ret = WaitForSingleObject(*ExitEvent, SLEEP_VALUE);

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
			int iIndex = MAX_USER - 1;
			int iModeChangeCount = NEWUSER_PACKET_COUNT;			
			while (iIndex > 0)
			{
				cSession* NowSession = SessionArray[iIndex];

				// ������ MODE_AUTH_TO_GAME ������� Ȯ�� -----------------
				if (NowSession->m_euMode == euSessionModeState::MODE_AUTH_TO_GAME)
				{
					// Game ��� ���� �� ����
					++g_lGameModeUserCount;

					// �´ٸ�, OnGame_ClientJoint ȣ��
					NowSession->OnGame_ClientJoin();

					// ��� ����
					NowSession->m_euMode = euSessionModeState::MODE_GAME;

					// ��� ü���� ���� ���� 0�� �Ǹ� �̹� �����ӿ� �����ȯ�� ����.
					// ���⼭ �ʹ� ��ü�Ǹ� �ٸ� �������� ������ �� ���ɼ�
					--iModeChangeCount;
					if (iModeChangeCount == 0)
						break;
				}

				--iIndex;
			}



			// ------------------
			// Part 2. Game ��� ���ǵ� ��Ŷ ó�� + Logout Flag ó��
			// ------------------
			iIndex = MAX_USER - 1;
			while (iIndex > 0)
			{
				cSession* NowSession = SessionArray[iIndex];

				// ������ MODE_GAME ������� Ȯ�� -----------------
				if (NowSession->m_euMode == euSessionModeState::MODE_GAME)
				{
					// LogOutFlag�� TRUE�̰�, SendFlag�� FALSE���
					if (NowSession->m_lLogoutFlag == TRUE && NowSession->m_lSendFlag == FALSE)
					{
						// Game ��� ���� �� ����
						--g_lGameModeUserCount;

						// Game��忡�� �������� �˷��ش�.
						NowSession->OnGame_ClientLeave();

						// �´ٸ�, MODE_WAIT_LOGOUT���� ��� ����
						NowSession->m_euMode = euSessionModeState::MODE_WAIT_LOGOUT;
					}

					// LogOutFlag�� FALSE�� ������ ���� ���� ó��
					else
					{
						// 1. CompleteRecvPacket ť�� ������ Ȯ��.
						int iQSize = NowSession->m_CRPacketQueue->GetNodeSize();

						// 2. ť�� ��尡 1�� �̻� ������, ��Ŷ ó��
						if (iQSize > 0)
						{
							// !! ��Ŷ ó�� Ƚ�� ������ �α� ����, PACKET_WORK_COUNT���� ����� ���� �� ������, PACKET_WORK_COUNT�� ���� !!
							if (iQSize > PACKET_WORK_COUNT)
								iQSize = PACKET_WORK_COUNT;

							CProtocolBuff_Net* NowPacket;

							while (iQSize > 0)
							{
								// ��� ����
								if (NowSession->m_CRPacketQueue->Dequeue(NowPacket) == -1)
									cMMOServer_Dump->Crash();								

								// ��Ŷ ó��
								NowSession->OnGame_Packet(NowPacket);

								// ��Ŷ ���۷��� ī��Ʈ 1 ����
								CProtocolBuff_Net::Free(NowPacket);

								// ��Ŷ 1�� ó�������� ���� �� ����.
								--iQSize;
							}
						}
					}
					
				}

				--iIndex;
			}
			

			// ------------------
			// Part 3. Game ����� ������Ʈ ó��
			// ------------------
			g_This->OnGame_Update();			


			// ------------------
			// Part 4. Release ����
			// ------------------
			iIndex = MAX_USER - 1;
			while (iIndex > 0)
			{
				cSession* NowSession = SessionArray[iIndex];

				// ������ MODE_WAIT_LOGOUT ������� Ȯ�� -----------------
				if (NowSession->m_euMode == euSessionModeState::MODE_WAIT_LOGOUT)
				{
					// Release
					// ���ο��� OnGame_ClientRelease ȣ��
					g_This->InDisconnect(NowSession);
					
				}

				--iIndex;
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

		// ���� �̺�Ʈ
		HANDLE* ExitEvent = &g_This->m_hSendExitEvent;

		// ���� ���� �迭
		cSession** SessionArray = g_This->m_stSessionArray;

		while (1)
		{
			// ------------------
			// 1�̸� �����帶�� 1ȸ�� �Ͼ��.
			// ------------------	
			DWORD Ret = WaitForSingleObject(*ExitEvent, 1);

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
				cSession* NowSession = SessionArray[iArrayIndex];

				// ------------------
				// send ���� �������� üũ
				// ------------------
				// SendFlag(1������)�� TRUE(2������)�� ����.
				// ���⼭ TRUE�� ���ϵǴ� ����, �̹� NowSession->m_SendFlag�� 1(���� ��)��°�.
				// ��, Release�� Ż ����.
				if (InterlockedExchange(&NowSession->m_lSendFlag, TRUE) == TRUE)
				{
					++iArrayIndex;
					continue;
				}			

				// SendFlag�� FALSE�� TRUE�� �ٲ۴�.
				/*if (NowSession->m_lSendFlag == FALSE)
					NowSession->m_lSendFlag = TRUE;*/


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

				if (NowSession->m_iWSASendCount > 0)
					cMMOServer_Dump->Crash();

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
						// ���� ���� ���·� ����.
						// WSASend�� �����ϸ� �Ϸ������� �ȶ߱� ������, SendFlag�� �ٲ� ���� ����.
						NowSession->m_lSendFlag = FALSE;

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
			m_hSendHandle[i] = (HANDLE)_beginthreadex(NULL, 0, SendThread, this, 0, NULL);
			if (m_hSendHandle == NULL)
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
			m_hAuthHandle[i] = (HANDLE)_beginthreadex(NULL, 0, AuthThread, this, 0, NULL);
			if (m_hAuthHandle == NULL)
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
			m_hGameHandle[i] = (HANDLE)_beginthreadex(NULL, 0, GameThread, this, 0, NULL);
			if (m_hGameHandle == NULL)
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


		// ���� ���� ����
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

		// SO_KEEPALIVE �ɼ� ����
		BOOL bEnable = TRUE;
		retval = setsockopt(m_soListen_sock, SOL_SOCKET, SO_KEEPALIVE, (char*)&bEnable, sizeof(bEnable));
		if (retval == SOCKET_ERROR)
		{
			// ������ ����, �� ���� ����
			m_iOSErrorCode = WSAGetLastError();
			m_iMyErrorCode = euError::NETWORK_LIB_ERROR__SOCKOPT_FAIL;

			// ���� �ڵ� ��ȯ �� �������� ����.
			ExitFunc(m_iW_ThreadCount);

			// �α� ��� (�α� ���� : ����)
			cMMOServer_Log->LogSave(L"MMOServer", CSystemLog::en_LogLevel::LEVEL_ERROR, L"Start() --> setsockopt() KeepAlive apply Error : NetError(%d), OSError(%d)",
				(int)m_iMyErrorCode, m_iOSErrorCode);

			// false ����
			return false;
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
		static int iIndex = 0;	

		if(m_stSessionArray == nullptr)
			m_stSessionArray = new cSession*[Max];

		m_stSessionArray[iIndex] = pSession;
		m_stSessionArray[iIndex]->m_bCode = HeadCode;
		m_stSessionArray[iIndex]->m_bXORCode_1 = XORCode1;
		m_stSessionArray[iIndex]->m_bXORCode_2 = XORCode2;		

		++iIndex;
	}
	   	  
}