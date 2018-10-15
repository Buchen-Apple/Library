/*
������ ����� Network Library
*/

#include "pch.h"

#pragma comment(lib,"ws2_32")
#include <Ws2tcpip.h>

#include <process.h>
#include <strsafe.h>

#include "NetworkLib_NetServer.h"
#include "CrashDump\CrashDump.h"
#include "RingBuff\RingBuff.h"
#include "Log\Log.h"
#include "LockFree_Queue\LockFree_Queue.h"

ULONGLONG g_ullAcceptTotal;
LONG	  g_lAcceptTPS;
LONG	g_lSendPostTPS;

LONG g_lSemCount;


namespace Library_Jingyu
{

#define _MyCountof(_array)		sizeof(_array) / (sizeof(_array[0]))

	// �α� ���� �������� �ϳ� �ޱ�.
	CSystemLog* cNetLibLog = CSystemLog::GetInstance();

	// ���� ���� ���� �ϳ� �ޱ�
	CCrashDump* cNetDump = CCrashDump::GetInstance();

	// ��� ������
#define dfNETWORK_PACKET_HEADER_SIZE_NETSERVER	5

	// �� ���� ������ �� �ִ� WSABUF�� ī��Ʈ
#define dfSENDPOST_MAX_WSABUF			300


	// ------------------------------
	// enum�� ����ü
	// ------------------------------
	// ��� ����ü
#pragma pack(push, 1)
	struct CNetServer::stProtocolHead
	{
		BYTE	m_Code;
		WORD	m_Len;
		BYTE	m_RandXORCode;
		BYTE	m_Checksum;
	};
#pragma pack(pop)
	

	// ���� ����ü
	struct CNetServer::stSession
	{
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

		// �ش� �ε��� �迭�� ������ �Ǿ����� üũ
		// TRUE�̸� ������ �Ǿ���.(����� �ƴ�), FALSE�̸� ������ �ȵ���.(�����)
		LONG m_lReleaseFlag;

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

		// PQCS overlapped ����ü
		OVERLAPPED m_overPQCSOverlapped;

		// ��Ŀ���� PQCS�� �õ��ߴ��� üũ�ϴ� Flag
		// TRUE�� ��Ŀ���� PQCS �õ���. FALSE�� �ƴ�.
		LONG m_lPQCSFlag;

		// ������ ��Ŷ �����
		void* m_LastPacket = nullptr;

		// ������ 
		stSession()
		{
			m_SendQueue = new CLF_Queue<CProtocolBuff_Net*>(0, false);
			m_lIOCount = 0;
			m_lReleaseFlag = TRUE;
			m_lSendFlag = FALSE;
			m_lPQCSFlag = FALSE;
			m_iWSASendCount = 0;
		}

		// �Ҹ���
		~stSession()
		{
			delete m_SendQueue;
		}

	};

	

	// -----------------------------
	// ������ ȣ�� �ϴ� �Լ�
	// -----------------------------

	// ���� ����
	// [���� IP(���ε� �� IP), ��Ʈ, ��Ŀ������ ��, Ȱ��ȭ��ų ��Ŀ������ ��, ����Ʈ ������ ��, TCP_NODELAY ��� ����(true�� ���), �ִ� ������ ��, ��Ŷ Code, XOR 1���ڵ�, XOR 2���ڵ�] �Է¹���.
	//
	// return false : ���� �߻� ��. �����ڵ� ���� �� false ����
	// return true : ����
	bool CNetServer::Start(const TCHAR* bindIP, USHORT port, int WorkerThreadCount, int ActiveWThreadCount, int AcceptThreadCount, bool Nodelay, int MaxConnect,
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
			cNetLibLog->LogSave(L"NetServer", CSystemLog::en_LogLevel::LEVEL_ERROR, L"Start() --> WSAStartup() Error : NetError(%d), OSError(%d)",
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
			cNetLibLog->LogSave(L"NetServer", CSystemLog::en_LogLevel::LEVEL_ERROR, L"Start() --> CreateIoCompletionPort() Error : NetError(%d), OSError(%d)",
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
				cNetLibLog->LogSave(L"NetServer", CSystemLog::en_LogLevel::LEVEL_ERROR, L"Start() --> WorkerThread Create Error : NetError(%d), OSError(%d)",
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
			cNetLibLog->LogSave(L"NetServer", CSystemLog::en_LogLevel::LEVEL_ERROR, L"Start() --> socket() Error : NetError(%d), OSError(%d)",
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
			cNetLibLog->LogSave(L"NetServer", CSystemLog::en_LogLevel::LEVEL_ERROR, L"Start() --> bind() Error : NetError(%d), OSError(%d)",
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
			cNetLibLog->LogSave(L"NetServer", CSystemLog::en_LogLevel::LEVEL_ERROR, L"Start() --> listen() Error : NetError(%d), OSError(%d)",
				(int)m_iMyErrorCode, m_iOSErrorCode);

			// false ����
			return false;
		}

		// ����� ���� ������ �۽Ź��� ũ�⸦ 0���� ����. �׷��� ���������� �񵿱� ��������� ����
		// ���� ���ϸ� �ٲٸ� ��� Ŭ�� �۽Ź��� ũ��� 0�̵ȴ�.
		//int optval = 0;
		//retval = setsockopt(m_soListen_sock, SOL_SOCKET, SO_SNDBUF, (char*)&optval, sizeof(optval));
		//if (optval == SOCKET_ERROR)
		//{
		//	// ������ ����, �� ���� ����
		//	m_iOSErrorCode = WSAGetLastError();
		//	m_iMyErrorCode = euError::NETWORK_LIB_ERROR__SOCKOPT_FAIL;

		//	// ���� �ڵ� ��ȯ �� �������� ����.
		//	ExitFunc(m_iW_ThreadCount);

		//	// �α� ��� (�α� ���� : ����)
		//	cNetLibLog->LogSave(L"NetServer", CSystemLog::en_LogLevel::LEVEL_ERROR, L"Start() --> setsockopt() SendBuff Size Change Error : NetError(%d), OSError(%d)",
		//		(int)m_iMyErrorCode, m_iOSErrorCode);

		//	// false ����
		//	return false;
		//}

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
				cNetLibLog->LogSave(L"NetServer", CSystemLog::en_LogLevel::LEVEL_ERROR, L"Start() --> setsockopt() Nodelay apply Error : NetError(%d), OSError(%d)",
					(int)m_iMyErrorCode, m_iOSErrorCode);

				// false ����
				return false;
			}
		}

		// �ִ� ���� ���� ���� �� ����
		m_iMaxJoinUser = MaxConnect;

		// ���� �迭 �����Ҵ�
		m_stSessionArray = new stSession[MaxConnect];

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
				cNetLibLog->LogSave(L"NetServer", CSystemLog::en_LogLevel::LEVEL_ERROR, L"Start() --> Accept Thread Create Error : NetError(%d), OSError(%d)",
					(int)m_iMyErrorCode, m_iOSErrorCode);

				// false ����
				return false;
			}
		}



		// ���� ������ !!
		m_bServerLife = true;

		// ���� ���� �α� ���		
		// �̰�, ��ӹ޴� �ʿ��� ��°ɷ� ����. �ݼ��� ��ü�� ���������� �۵����� ����.
		//cNetLibLog->LogSave(L"NetServer", CSystemLog::en_LogLevel::LEVEL_SYSTEM, L"ServerOpen...");

		return true;
	}

	// ���� ��ž.
	void CNetServer::Stop()
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
			cNetLibLog->LogSave(L"NetServer", CSystemLog::en_LogLevel::LEVEL_ERROR, L"Stop() --> Accept Thread EXIT Error : NetError(%d), OSError(%d)",
				(int)m_iMyErrorCode, m_iOSErrorCode);

			// ���� �߻� �Լ� ȣ��
			OnError((int)euError::NETWORK_LIB_ERROR__WFSO_ERROR, L"Stop() --> Accept Thread EXIT Error");
		}

		// 2. ��� �������� Shutdown
		// ��� �������� �˴ٿ� ����
		for (int i = 0; i < m_iMaxJoinUser; ++i)
		{
			if (m_stSessionArray[i].m_lReleaseFlag == FALSE)
			{
				shutdown(m_stSessionArray[i].m_Client_sock, SD_BOTH);
			}
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
			cNetLibLog->LogSave(L"NetServer", CSystemLog::en_LogLevel::LEVEL_ERROR, L"Stop() --> Worker Thread EXIT Error : NetError(%d), OSError(%d)",
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
		cNetLibLog->LogSave(L"NetServer", CSystemLog::en_LogLevel::LEVEL_SYSTEM, L"ServerStop...");
	}

	// �ܺο���, � �����͸� ������ ������ ȣ���ϴ� �Լ�.
	// SendPacket�� �׳� �ƹ����� �ϸ� �ȴ�.
	// �ش� ������ SendQ�� �־�״ٰ� ���� �Ǹ� ������.
	//
	// Parameter : SessionID, SendBuff, LastFlag(����Ʈ FALSE)
	// return : ����
	void CNetServer::SendPacket(ULONGLONG SessionID, CProtocolBuff_Net* payloadBuff, LONG LastFlag)
	{
		// ���� �� �ɱ�(�� �ƴ����� ��ó�� �����) ----------------
		stSession* NowSession = GetSessionLOCK(SessionID);
		if (NowSession == nullptr)
		{
			// �� ����, ť���� ���� ���߱� ������, ���ڷ� ���� ����ȭ���۸� Free�Ѵ�.
			// ref ī��Ʈ�� 0�� �Ǹ� �޸�Ǯ�� ��ȯ
			CProtocolBuff_Net::Free(payloadBuff);
			return;
		}

		// 1. m_LastPacket�� nullptr�� �ƴ϶��, ������ ���� ����. ť�� ���� �ʴ´�.
		if (NowSession->m_LastPacket != nullptr)
		{
			CProtocolBuff_Net::Free(payloadBuff);
			return;
		}

		// 2. ���ڷ� ���� Flag�� true���, ������ ��Ŷ�� �ּҸ� ����
		if (LastFlag == TRUE)
			NowSession->m_LastPacket = payloadBuff;		

		// 3. ����� �־� ��Ŷ �ϼ�
		payloadBuff->Encode(m_bCode, m_bXORCode_1, m_bXORCode_2);

		// 4. ��ť. ��Ŷ�� "�ּ�"�� ��ť�Ѵ�(8����Ʈ)
		// ����ȭ ���� ���۷��� ī��Ʈ 1 ����
		payloadBuff->Add();
		NowSession->m_SendQueue->Enqueue(payloadBuff);

		// 5. ����ȭ ���� ���۷��� ī��Ʈ 1 ����. 0 �Ǹ� �޸�Ǯ�� ��ȯ
		CProtocolBuff_Net::Free(payloadBuff);

		// 6. PQCS���� ���Ͷ����� �ϳ� ����. ���⼭ ������Ų ���� ��Ŀ���� ����
		InterlockedIncrement(&NowSession->m_lIOCount);
		
		// 7. PQCS
		PostQueuedCompletionStatus(m_hIOCPHandle, 0, (ULONG_PTR)NowSession, &NowSession->m_overPQCSOverlapped);

		// 8. ���� �� ����(�� �ƴ����� ��ó�� ���) ----------------------
		// ���⼭ false�� ���ϵǸ� �̹� �ٸ������� �����Ǿ���� �ߴµ� SendPacket�� I/Oī��Ʈ�� �ø����� ���� �������� ���� ��������.
		// �ٵ� ���� ���ϰ� ���� �ʰ� ����
		GetSessionUnLOCK(NowSession);				   		
	}

	// ������ ������ ���� �� ȣ���ϴ� �Լ�. �ܺ� ���� ���.
	// ���̺귯������ ������!��� ��û�ϴ� �� ��
	//
	// Parameter : SessionID
	// return : ����
	void CNetServer::Disconnect(ULONGLONG SessionID)
	{
		// 1. ���� �� 
		stSession* DeleteSession = GetSessionLOCK(SessionID);
		if (DeleteSession == nullptr)
			return;

		// 2. ������ϴ� ������ �˴ٿ� ������.
		// ���� �ڿ������� I/Oī��Ʈ�� ���ҵǾ ��Ŀ��Ʈ�ȴ�.
		shutdown(DeleteSession->m_Client_sock, SD_BOTH);

		// 3. ���� �� ����
		// ���⼭ ������ ������, ���������� ������ ������ ���� �ֱ� ������ (shutdown ��������!) falseüũ ���Ѵ�.
		GetSessionUnLOCK(DeleteSession);
	}

	///// ���� �����Լ��� /////
	// ������ ���� ���
	int CNetServer::WinGetLastError()
	{
		return m_iOSErrorCode;
	}

	// �� ���� ���
	int CNetServer::NetLibGetLastError()
	{
		return (int)m_iMyErrorCode;
	}

	// ���� ������ �� ���
	ULONGLONG CNetServer::GetClientCount()
	{
		return m_ullJoinUserCount;
	}

	// ���� �������� ���
	// return true : ������
	// return false : ������ �ƴ�
	bool CNetServer::GetServerState()
	{
		return m_bServerLife;
	}

	// �̻�� ���� ���� ������ ��� ���
	LONG CNetServer::GetStackNodeCount()
	{
		return m_stEmptyIndexStack->GetInNode();
	}






	// -----------------------------
	// Lib ���ο����� ����ϴ� �Լ�
	// -----------------------------
	// ������
	CNetServer::CNetServer()
	{
		// ���� �������� false�� ���� 
		m_bServerLife = false;
	}

	// �Ҹ���
	CNetServer::~CNetServer()
	{
		// ������ �������̾�����, ���� �������� ����
		if (m_bServerLife == true)
			Stop();
	}

	// ������ �������� ��Ű�� �Լ�
	void CNetServer::InDisconnect(stSession* DeleteSession)
	{
		// ReleaseFlag�� I/Oī��Ʈ �� �� 0�̶�� 'ReleaseFlag'�� TRUE�� �ٲ۴�!
		// I/Oī��Ʈ�� �ȹٲ۴�.
		// 
		// FALSE�� ���ϵǴ� ����, �̹� DeleteSession->m_lReleaseFlag�� I/OCount�� 0�̾��ٴ� �ǹ�. 
		// ������ Ÿ���Ѵ�.
		if (InterlockedCompareExchange64((LONG64*)&DeleteSession->m_lReleaseFlag, TRUE, FALSE) == FALSE)
		{
			// �������ʿ� �˷��ֱ� ���� ���� ID �޾Ƶ�.
			// �ش� ������ ���ÿ� �ݳ��� ������ �������� �˷��ֱ� ������ �̸� �޾Ƶд�.
			ULONGLONG sessionID = DeleteSession->m_ullSessionID;

			DeleteSession->m_ullSessionID = 0xffffffffffffffff;

			// �ش� ������ 'Send ����ȭ ����(Send�ߴ� ����ȭ ���� ����. ���� �Ϸ����� ������ ����ȭ���۵�)'�� �ִ� �����͸� Free�Ѵ�.
			for (int i = 0; i < DeleteSession->m_iWSASendCount; ++i)
				CProtocolBuff_Net::Free(DeleteSession->m_PacketArray[i]);

			// SendCount �ʱ�ȭ
			DeleteSession->m_iWSASendCount = 0;			

			int UseSize = DeleteSession->m_SendQueue->GetInNode();

			CProtocolBuff_Net* Payload;
			int i = 0;
			while (i < UseSize)
			{
				// ��ť ��, ����ȭ ���� �޸�Ǯ�� Free�Ѵ�.
				if (DeleteSession->m_SendQueue->Dequeue(Payload) == -1)
					cNetDump->Crash();

				CProtocolBuff_Net::Free(Payload);

				i++;
			}			

			// ť �ʱ�ȭ
			DeleteSession->m_RecvQueue.ClearBuffer();			

			// Ŭ���� ����
			closesocket(DeleteSession->m_Client_sock);

			// ���� �� ���� �� ����
			InterlockedDecrement(&m_ullJoinUserCount);

			// ������ �ʿ� ����� ���� �˷��� 
			// �������� ����� ���� ����Ű�� �̿��� ����Ѵ�. �׷��� ���ڷ� ����Ű�� �Ѱ��ش�.
			OnClientLeave(sessionID);

			// �̻�� �ε��� ���ÿ� �ݳ�
			m_stEmptyIndexStack->Push(DeleteSession->m_lIndex);			
		}
		
		return;	
	}

	// Start���� ������ �� �� ȣ���ϴ� �Լ�.
	// 1. ����� �Ϸ���Ʈ �ڵ� ��ȯ
	// 2. ��Ŀ������ ����, �ڵ��ȯ, �ڵ� �迭 ��������
	// 3. ����Ʈ ������ ����, �ڵ� ��ȯ
	// 4. �������� �ݱ�
	// 5. ���� ����
	void CNetServer::ExitFunc(int w_ThreadCount)
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

	// ��Ŀ������
	UINT WINAPI	CNetServer::WorkerThread(LPVOID lParam)
	{
		DWORD cbTransferred;
		stSession* stNowSession;
		OVERLAPPED* overlapped;

		CNetServer* g_This = (CNetServer*)lParam;

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
			// PQCS ��û ����
			// -----------------
			if (&stNowSession->m_overPQCSOverlapped == overlapped)
			{
				// 1. PQCS Flag üũ
				// ���⼭ TRUE�� ���ϵǴ� ����, �̹� stNowSession->m_lPQCSFlag�� 1(PQCS��)�̾��ٴ� ��.
				if (InterlockedExchange(&stNowSession->m_lPQCSFlag, TRUE) == TRUE)
				{
					// �׷��ٸ� �ٽ� PQCS�� �Ǵ�.
					PostQueuedCompletionStatus(g_This->m_hIOCPHandle, 0, (ULONG_PTR)stNowSession, &stNowSession->m_overPQCSOverlapped);
					continue;
				}

				//  2. SendPost()
				g_This->SendPost(stNowSession);

				// 3. PQCSFlag �ǵ���
				stNowSession->m_lPQCSFlag = FALSE;

				// 4. SendPacket���� �������״� I/O ī��Ʈ ����
				if (InterlockedDecrement(&stNowSession->m_lIOCount) == 0)
					g_This->InDisconnect(stNowSession);					

				continue;
			}


			// -----------------
			// Recv ����
			// -----------------
			// WSArecv()�� �Ϸ�� ���, ���� �����Ͱ� 0�� �ƴϸ� ���� ó��
			else if (&stNowSession->m_overRecvOverlapped == overlapped && cbTransferred > 0)
			{
				// rear �̵�
				stNowSession->m_RecvQueue.MoveWritePos(cbTransferred);

				// 1. ������ ��, ��Ŷ ó��
				g_This->RecvProc(stNowSession);

				// 2. ���ú� �ٽ� �ɱ�.
				// ���ú� ť�� ��á���� ���� ���´�.
				if (g_This->RecvPost(stNowSession) == 1)
				{
					// I/O ī��Ʈ ���ҽ��״µ� 0�̵Ǹ� shutdown�����͵� ���� InDisconnect.
					if (InterlockedDecrement(&stNowSession->m_lIOCount) == 0)
					{
						g_This->InDisconnect(stNowSession);
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
				InterlockedAdd(&g_lSendPostTPS, stNowSession->m_iWSASendCount);

				// 1. ���� �Ϸ�ƴٰ� �������� �˷���
				g_This->OnSend(stNowSession->m_ullSessionID, cbTransferred);		

				// 2. ������ ���� ������ ��� ����
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
						// I/O ī��Ʈ ���ҽ��״µ� 0�̵Ǹ� shutdown�����͵� ���� InDisconnect.
						if (InterlockedDecrement(&stNowSession->m_lIOCount) == 0)
						{
							g_This->InDisconnect(stNowSession);
							continue;
						}

						// 0�� �ƴ϶�� ���������� ���� ���´�.
						shutdown(stNowSession->m_Client_sock, SD_BOTH);
						continue;
					}
				}

				// 3. ������ ���� ������ �ƴ� ��� ����
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
												

				// 4. ���� ���� ���·� ����
				stNowSession->m_lSendFlag = FALSE;				

				// 5. �ٽ� ���� �õ�
				g_This->SendPost(stNowSession);
			}

			// -----------------
			// I/Oī��Ʈ ���� �� ���� ó��
			// -----------------
			// I/Oī��Ʈ ���� ��, 0�̶������ ����
			if(InterlockedDecrement(&stNowSession->m_lIOCount) == 0)
				g_This->InDisconnect(stNowSession);
		}
		return 0;
	}

	// Accept ������
	UINT WINAPI	CNetServer::AcceptThread(LPVOID lParam)
	{
		// --------------------------
		// Accept ��Ʈ
		// --------------------------
		SOCKET client_sock;
		SOCKADDR_IN	clientaddr;
		int addrlen = sizeof(clientaddr);

		CNetServer* g_This = (CNetServer*)lParam;

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
					
			InterlockedIncrement(&g_lAcceptTPS); // �׽�Ʈ��!!

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

			g_ullAcceptTotal++;	// �׽�Ʈ��!!

			// ------------------
			// IP�� ��Ʈ �˾ƿ���.
			// ------------------
			InetNtop(AF_INET, &clientaddr.sin_addr, tcTempIP, 30);
			port = ntohs(clientaddr.sin_port);




			// ------------------
			// ���� ����, IP���� �Ǵ��ؼ� ���� �߰� �۾��� �ʿ��� ��찡 ���� ���� ������ ȣ��
			// ------------------		
			// false�� ���Ӱź�, 
			// true�� ���� ��� ����. true���� �Ұ� ������ OnConnectionRequest�Լ� ���ڷ� ������ ������.
			if (g_This->OnConnectionRequest(tcTempIP, port) == false)
				continue;




			// ------------------
			// ���� ����ü ���� �� ����
			// ------------------
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

			// -- LastPacket �ʱ�ȭ
			g_This->m_stSessionArray[iIndex].m_LastPacket = nullptr;
			
			// -- ����
			g_This->m_stSessionArray[iIndex].m_Client_sock = client_sock;						

			// -- IP�� Port
			StringCchCopy(g_This->m_stSessionArray[iIndex].m_IP, _MyCountof(g_This->m_stSessionArray[iIndex].m_IP), tcTempIP);
			g_This->m_stSessionArray[iIndex].m_prot = port;

			// -- SendFlag �ʱ�ȭ
			g_This->m_stSessionArray[iIndex].m_lSendFlag = FALSE;

			// 3) �ش� ����, ��������� ����
			// ������ ��� �������� ������ ���� ���·� ����.
			g_This->m_stSessionArray[iIndex].m_lReleaseFlag = FALSE;


			// ------------------
			// IOCP ����
			// ------------------
			// ���ϰ� IOCP ����
			if (CreateIoCompletionPort((HANDLE)client_sock, g_This->m_hIOCPHandle, (ULONG_PTR)&g_This->m_stSessionArray[iIndex], 0) == NULL)
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



			// ------------------
			// ��� ���������� �Ϸ�Ǿ����� ���� �� ó�� �Լ� ȣ��.
			// ------------------
			g_This->OnClientJoin(MixKey);



			// ------------------
			// �񵿱� ����� ����
			// ------------------
			// ���ú� ���۰� ��á���� 1����
			int ret = g_This->RecvPost(&g_This->m_stSessionArray[iIndex]);

			// �������״�, I/Oī��Ʈ --. 0�̶�� ����ó��
			if(InterlockedDecrement(&g_This->m_stSessionArray[iIndex].m_lIOCount) == 0)
				g_This->InDisconnect(&g_This->m_stSessionArray[iIndex]);		

			// I/Oī��Ʈ ���ҽ��״µ� 0�� �ƴ϶��, ret üũ.
			// ret�� 1�̶�� ���� ���´�.
			else if (ret == 1)
				shutdown(g_This->m_stSessionArray[iIndex].m_Client_sock, SD_BOTH);
		}

		return 0;
	}

	// SendPacket, Disconnect �� �ܺο��� ȣ���ϴ� �Լ�����, ���Ŵ� �Լ�.
	// ���� ���� �ƴ����� ��ó�� ���.
	//
	// Parameter : SessionID
	// return : ���������� ���� ã���� ��, �ش� ���� ������
	//		  : I/Oī��Ʈ�� 0�̵Ǿ� ������ ������, nullptr
	CNetServer::stSession* 	CNetServer::GetSessionLOCK(ULONGLONG SessionID)
	{
		// 1. SessionID�� ���� �˾ƿ���	
		stSession* retSession = &m_stSessionArray[(WORD)SessionID];

		// 2. I/O ī��Ʈ 1 ����.	
		if (InterlockedIncrement(&retSession->m_lIOCount) == 1)
		{
			// I/O ī��Ʈ�� 1�̶�� �ٽ� --
			// ������ ���� 0�̸鼭, inDIsconnect ȣ��
			if(InterlockedDecrement(&retSession->m_lIOCount) == 0)
				InDisconnect(retSession);			

			return nullptr;
		}	

		// 3. Release Flag üũ
		if (retSession->m_lReleaseFlag == TRUE)
		{
			if (InterlockedDecrement(&retSession->m_lIOCount) == 0)
				InDisconnect(retSession);

			return nullptr;
		}

		// 4. ���� ���ϴ� ������ �´��� üũ
		if (retSession->m_ullSessionID != SessionID)
		{
			// �ƴ϶�� I/O ī��Ʈ 1 ����
			// ������ ���� 0�̸鼭, inDIsconnect ȣ��
			if (InterlockedDecrement(&retSession->m_lIOCount) == 0)
				InDisconnect(retSession);

			return nullptr;
		}								

		// 5. ���������� �ִ� ������, �����ϰ� ó���� ������� �ش� ������ ������ ��ȯ
		return retSession;
	}

	// SendPacket, Disconnect �� �ܺο��� ȣ���ϴ� �Լ�����, �� �����ϴ� �Լ�
	// ���� ���� �ƴ����� ��ó�� ���.
	//
	// parameter : ���� ������
	// return : ���������� ���� ��, true
	//		  : I/Oī��Ʈ�� 0�̵Ǿ� ������ ������, false
	bool 	CNetServer::GetSessionUnLOCK(stSession* NowSession)
	{
		// 1. I/O ī��Ʈ 1 ����
		if (InterlockedDecrement(&NowSession->m_lIOCount) == 0)
		{
			InDisconnect(NowSession);
			return false;
		}

		return true;
	}
	   

	// ���� �������� ���½�Ų��.
	// Stop() �Լ����� ���.
	void CNetServer::Reset()
	{
		m_iW_ThreadCount = 0;
		m_iA_ThreadCount = 0;
		m_hWorkerHandle = nullptr;
		m_hIOCPHandle = 0;
		m_soListen_sock = 0;
		m_iMaxJoinUser = 0;
		m_ullJoinUserCount = 0;
	}






	// ------------
	// Lib ���ο����� ����ϴ� ���ú� ���� �Լ���
	// ------------
	// RecvProc �Լ�. ť�� ���� üũ �� PacketProc���� �ѱ��.
	void CNetServer::RecvProc(stSession* NowSession)
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
				cNetDump->Crash();

			// 3. ����� �ڵ� Ȯ��. �� ���� �´���
			if(Header.m_Code != m_bCode)
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
				cNetDump->Crash();
			

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

			// 11. Recv���� �������� ��� Ÿ�Կ� ���� �б�ó��.
			OnRecv(NowSession->m_ullSessionID, PayloadBuff);

			CProtocolBuff_Net::Free(PayloadBuff);
		}

		return;
	}

	// RecvPost �Լ�. �񵿱� ����� ����
	//
	// return 0 : ���������� WSARecv() �Ϸ�
	// return 1 : RecvQ�� ���� ����
	// return 2 : I/O ī��Ʈ�� 0�̵Ǿ� ������ ����
	int CNetServer::RecvPost(stSession* NowSession)
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
					InDisconnect(NowSession);
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
					cNetLibLog->LogSave(L"NetServer", CSystemLog::en_LogLevel::LEVEL_ERROR, L"WSARecv --> %s : NetError(%d), OSError(%d)",
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






	// ------------
	// Lib ���ο����� ����ϴ� ���� ���� �Լ���
	// ------------
	// ���� ������ ������ WSASend() �ϱ�
	//
	// return 0 : ���������� WSASend() �Ϸ� or WSASend�� ���������� ����� ������ �ƴ�. or UseSize�� 0�̶� �Ұ� ����.
	// return 1 : SendFlag�� TRUE(���� �̹� ������)��.
	// return 2 : I/Oī��Ʈ�� 0�̵Ǿ ����� ����
	int CNetServer::SendPost(stSession* NowSession)
	{
		while (1)
		{	
			// ------------------
			// send ���� �������� üũ
			// ------------------
			// 1. SendFlag(1������)�� �� TRUE(2������)�� ����.
			// ���⼭ TRUE�� ���ϵǴ� ����, �̹� NowSession->m_SendFlag�� 1(���� ��)�̾��ٴ� ��.
			if (InterlockedExchange(&NowSession->m_lSendFlag, TRUE) == TRUE)
				return 1;
			
			// 2. SendBuff�� �����Ͱ� �ִ��� Ȯ��
			// ���⼭ ���� UseSize�� ���� ������ �̴�. 
			int UseSize = NowSession->m_SendQueue->GetInNode();
			if (UseSize == 0)
			{
				// WSASend �Ȱɾ��� ������, ���� ���� ���·� �ٽ� ����.
				NowSession->m_lSendFlag = FALSE;

				// 3. ��¥�� ����� ������ �ٽ��ѹ� üũ. �� Ǯ�� �Դµ�, ���ؽ�Ʈ ����Ī �Ͼ�� �ٸ� �����尡 �ǵ���� ���ɼ�
				// ������ ������ ���� �ö󰡼� �ѹ� �� �õ�
				if (NowSession->m_SendQueue->GetInNode() > 0)
					continue;

				break;
			}			


			// ------------------
			// Send �غ��ϱ�
			// ------------------
			// 1. WSABUF ����.
			WSABUF wsabuf[dfSENDPOST_MAX_WSABUF];

			if (UseSize > dfSENDPOST_MAX_WSABUF)
				UseSize = dfSENDPOST_MAX_WSABUF;

			int i = 0;
			while (i < UseSize)
			{
				if (NowSession->m_SendQueue->Dequeue(NowSession->m_PacketArray[i]) == -1)
					cNetDump->Crash();	
				

				// WSABUF�� �����ϱ�
				wsabuf[i].buf = NowSession->m_PacketArray[i]->GetBufferPtr();
				wsabuf[i].len = NowSession->m_PacketArray[i]->GetUseSize();

				i++;
			}			

			NowSession->m_iWSASendCount = UseSize;

			// 4. Overlapped ����ü �ʱ�ȭ
			ZeroMemory(&NowSession->m_overSendOverlapped, sizeof(NowSession->m_overSendOverlapped));

			// 5. WSASend()
			DWORD SendBytes = 0, flags = 0;
			InterlockedIncrement(&NowSession->m_lIOCount);

			// 6. ���� ó��
			if (WSASend(NowSession->m_Client_sock, wsabuf, UseSize, &SendBytes, flags, &NowSession->m_overSendOverlapped, NULL) == SOCKET_ERROR)
			{
				int Error = WSAGetLastError();

				// �񵿱� ������� ���۵Ȱ� �ƴ϶��
				if (Error != WSA_IO_PENDING)
				{
					if(InterlockedDecrement(&NowSession->m_lIOCount) == 0)
					{
						InDisconnect(NowSession);
						return 2;
					}
					
					// ������ ���� �����̶��
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
						cNetLibLog->LogSave(L"NetServer", CSystemLog::en_LogLevel::LEVEL_ERROR, L"WSASend --> %s : NetError(%d), OSError(%d)",
							tcErrorString, (int)m_iMyErrorCode, m_iOSErrorCode);
						
						// ���� �Լ� ȣ��
						OnError((int)euError::NETWORK_LIB_ERROR__WSAENOBUFS, tcErrorString);

						// ���´�.
						shutdown(NowSession->m_Client_sock, SD_BOTH);
					}
				}
			}
			break;
		}

		return 0;
	}

}
