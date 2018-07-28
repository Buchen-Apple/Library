#include "stdafx.h"

#pragma comment(lib,"ws2_32")
#include <Ws2tcpip.h>

#include <process.h>
#include <Windows.h>
#include <strsafe.h>

#include "NetworkLib.h"
#include "RingBuff\RingBuff.h"



namespace Library_Jingyu
{
	// ��� ������
	#define dfNETWORK_PACKET_HEADER_SIZE	2


	// ------------------------------
	// enum�� ����ü
	// ------------------------------
	// enum class
	enum class CLanServer::euError : int
	{
		NETWORK_LIB_ERROR__NORMAL = 0,					// ���� ���� �⺻ ����
		NETWORK_LIB_ERROR__WINSTARTUP_FAIL,				// ���� �ʱ�ȭ �ϴٰ� ������
		NETWORK_LIB_ERROR__CREATE_IOCP_PORT,			// IPCP ����ٰ� ������
		NETWORK_LIB_ERROR__W_THREAD_FAIL,				// ��Ŀ������ ����ٰ� ���� 
		NETWORK_LIB_ERROR__A_THREAD_FAIL,				// ����Ʈ ������ ����ٰ� ���� 
		NETWORK_LIB_ERROR__L_THREAD_FAIL,				// ������üũ ������ ����ٰ� ���� 
		NETWORK_LIB_ERROR__CREATE_SOCKET_FAIL,			// ���� ���� ���� 
		NETWORK_LIB_ERROR__BINDING_FAIL,				// ���ε� ����
		NETWORK_LIB_ERROR__LISTEN_FAIL,					// ���� ����
		NETWORK_LIB_ERROR__SOCKOPT_FAIL,				// ���� �ɼ� ���� ����
		NETWORK_LIB_ERROR__IOCP_ERROR,					// IOCP ��ü ����
		NETWORK_LIB_ERROR__NOT_FIND_CLINET,				// map �˻� ���� �Ҷ� Ŭ���̾�Ʈ�� ��ã�����.
		NETWORK_LIB_ERROR__SEND_QUEUE_SIZE_FULL	,		// Enqueue����� ���� ����
		NETWORK_LIB_ERROR__WSASEND_FAIL,				// SendPost���� WSASend ����
		NETWORK_LIB_ERROR__WSAENOBUFS,					// WSASend, WSARecv�� ���ۻ����� ����
		NETWORK_LIB_ERROR__EMPTY_RECV_BUFF,				// Recv �Ϸ������� �Դµ�, ���ú� ���۰� ����ִٰ� ������ ����.
		NETWORK_LIB_ERROR__A_THREAD_ABONORMAL_EXIT,		// ����Ʈ ������ ������ ����. ���� accept()�Լ����� �̻��� ������ ���°�.
		NETWORK_LIB_ERROR__DISCONNECT_WAIT_CLINET		// ���� ���� ������� Ŭ���̾�Ʈ���� send�� �ϰų� �� ���
	};

	// ���� ����ü
	struct CLanServer::stSession
	{
		SOCKET m_Client_sock;

		TCHAR m_IP[30];
		USHORT m_prot;

		ULONGLONG m_ullSessionID;

		LONG	m_lIOCount;

		// Send���� �������� üũ. 1�̸� Send��, 0�̸� Send�� �ƴ�
		LONG	m_lSendFlag;

		// shutdown �������� üũ. shutdown ���� ���¸� ������ �־ ������ �ȵ�.
		bool m_bShutdownState;

		// �� ����ü ����ȭ ��ü.
		SRWLOCK m_srwStruct_srwl;

		OVERLAPPED m_overRecvOverlapped;
		OVERLAPPED m_overSendOverlapped;

		CRingBuff m_RecvQueue;
		CRingBuff m_SendQueue;

		// ������
		stSession()
		{
			InitializeSRWLock(&m_srwStruct_srwl);
			m_lIOCount = 0;
			m_lSendFlag = 0;
			m_bShutdownState = false;
		}

		// �� �ɱ�
		void LockSession_Func()
		{
			AcquireSRWLockExclusive(&m_srwStruct_srwl);
		}

		// �� Ǯ��
		void UnlockSession_Func()
		{
			ReleaseSRWLockExclusive(&m_srwStruct_srwl);
		}


	#define	Struct_Lock()	LockSession_Func()
	#define Struct_Unlock()	UnlockSession_Func()
	
	};




	// -----------------------------
	// ������ ȣ�� �ϴ� �Լ�
	// -----------------------------
	// ���� ����
	// [���� IP(���ε� �� IP), ��Ʈ, ��Ŀ������ ��, TCP_NODELAY ��� ����(true�� ���), �ִ� ������ ��] �Է¹���.
	// return false : ���� �߻� ��. �����ڵ� ���� �� false ����
	// return true : ����
	bool CLanServer::Start(const TCHAR* bindIP, USHORT port, int WorkerThreadCount, bool Nodelay, int MaxConnect)
	{
		// ���� ���� �ʱ�ȭ �Լ�
		Init();

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
		
			// false ����
			return false;
		}


		// ����� �Ϸ���Ʈ ����
		// ����� �Ϸ� ��Ʈ ����
		m_hIOCPHandle = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
		if (m_hIOCPHandle == NULL)
		{
			// ������ ����, �� ���� ����
			m_iOSErrorCode = WSAGetLastError();
			m_iMyErrorCode = euError::NETWORK_LIB_ERROR__CREATE_IOCP_PORT;

			// ���� �ڵ� ��ȯ �� �������� ����.
			ExitFunc(m_iW_ThreadCount);
		
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
				m_iMyErrorCode = euError::NETWORK_LIB_ERROR__W_THREAD_FAIL;

				// ���� �ڵ� ��ȯ �� �������� ����.
				ExitFunc(i);
			
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

			// false ����
			return false;
		}

		// ����
		if (listen(m_soListen_sock, SOMAXCONN) == SOCKET_ERROR)
		{
			// ������ ����, �� ���� ����
			m_iOSErrorCode = WSAGetLastError();
			m_iMyErrorCode = euError::NETWORK_LIB_ERROR__LISTEN_FAIL;
		
			// ���� �ڵ� ��ȯ �� �������� ����.
			ExitFunc(m_iW_ThreadCount);

			// false ����
			return false;
		}

		// ����� ���� ������ �۽Ź��� ũ�⸦ 0���� ����. �׷��� ���������� �񵿱� ��������� ����
		// ���� ���ϸ� �ٲٸ� ��� Ŭ�� �۽Ź��� ũ��� 0�̵ȴ�.
		int optval = 0;
		if (setsockopt(m_soListen_sock, SOL_SOCKET, SO_SNDBUF, (char*)&optval, sizeof(optval)) == SOCKET_ERROR)
		{
			// ������ ����, �� ���� ����
			m_iOSErrorCode = WSAGetLastError();
			m_iMyErrorCode = euError::NETWORK_LIB_ERROR__SOCKOPT_FAIL;

			// ���� �ڵ� ��ȯ �� �������� ����.
			ExitFunc(m_iW_ThreadCount);

			// false ����
			return false;
		}

		// ���ڷ� ���� ������� �ɼ� ��� ���ο� ���� ���̱� �ɼ� ����
		// �̰� true�� ������� ����ϰڴٴ� ��(���̱� �������Ѿ���)
		if (Nodelay == true)
		{
			BOOL optval = TRUE;
			if (setsockopt(m_soListen_sock, IPPROTO_TCP, TCP_NODELAY, (char*)&optval, sizeof(optval)) == SOCKET_ERROR)
			{
				// ������ ����, �� ���� ����
				m_iOSErrorCode = WSAGetLastError();
				m_iMyErrorCode = euError::NETWORK_LIB_ERROR__SOCKOPT_FAIL;

				// ���� �ڵ� ��ȯ �� �������� ����.
				ExitFunc(m_iW_ThreadCount);

				// false ����
				return false;
			}
		}

		// ����Ʈ ������ ����
		m_hAcceptHandle = (HANDLE)_beginthreadex(NULL, 0, AcceptThread, this, 0, NULL);
		if (m_hAcceptHandle == NULL)
		{
			// ������ ����, �� ���� ����
			m_iOSErrorCode = errno;
			m_iMyErrorCode = euError::NETWORK_LIB_ERROR__A_THREAD_FAIL;

			// ���� �ڵ� ��ȯ �� �������� ����.
			ExitFunc(m_iW_ThreadCount);

			// false ����
			return false;
		}

		// ����Ʈ ������ ���� 1�� ����. �ʿ� �������� ������ Ȥ�� �𸣴� ī��Ʈ
		m_iA_ThreadCount = 1;

		// ���� ������ !!
		m_bServerLife = true;

		return true;
	}

	// ���� ��ž.
	void CLanServer::Stop()
	{
		// 1. Accept ������ ����. �� �̻� ������ ������ �ȵǴ� Accept������ ���� ����
		// Accept ������� ���������� closesocket�ϸ� �ȴ�.
		closesocket(m_soListen_sock);
		DWORD retval = WaitForSingleObject(m_hAcceptHandle, INFINITE);
		if (retval != WAIT_OBJECT_0)
		{
			printf("����Ʈ ������ ����\n");		
		}

		// 2. ��� �������� Shutdown
		// ��� �������� �˴ٿ� ����
		Lock_Map();

		map<ULONGLONG, stSession*>::iterator itor = map_Session.begin();
		for (; itor != map_Session.end(); ++itor)
		{
			itor->second->Struct_Lock();
			itor->second->m_bShutdownState = true;
			shutdown(itor->second->m_Client_sock, SD_BOTH);
			itor->second->Struct_Unlock();
		}
		

		Unlock_Map();

		// ��� ������ ����Ǿ����� üũ
		while (1)
		{
			if (m_ullJoinUserCount == 0)
				break;

			Sleep(2);
		}

		// 3. ��Ŀ ������ ����
		for (int i = 0; i<m_iW_ThreadCount; ++i)
			PostQueuedCompletionStatus(m_hIOCPHandle, 0, 0, 0);

		// ��Ŀ������ ���� ���
		WaitForMultipleObjects(m_iW_ThreadCount, m_hWorkerHandle, TRUE, INFINITE);

		// 4. ���� ���ҽ� ��ȯ
		// 1) ����Ʈ ������ �ڵ� ��ȯ
		CloseHandle(m_hAcceptHandle);

		// 2) ��Ŀ ������ �ڵ� ��ȯ
		for (int i = 0; i < m_iW_ThreadCount; ++i)
			CloseHandle(m_hWorkerHandle[i]);

		// 3) ��Ŀ ������ �迭 ��������
		delete[] m_hWorkerHandle;

		// 4) IOCP�ڵ� ��ȯ
		CloseHandle(m_hIOCPHandle);

		// 5) ���� ����
		WSACleanup();

		// 5. ���� ������ �ƴ� ���·� ����
		m_bServerLife = false;
	}

	// �ܺο���, � �����͸� ������ ������ ȣ���ϴ� �Լ�.
	// SendPacket�� �׳� �ƹ����� �ϸ� �ȴ�.
	// �ش� ������ SendQ�� �־�״ٰ� ���� �Ǹ� ������.
	bool CLanServer::SendPacket(ULONGLONG ClinetID, CProtocolBuff* payloadBuff)
	{
		// 1. ClinetID�� ���Ǳ���ü �˾ƿ���
		stSession* NowSession = FineSessionPtr(ClinetID);
		if (NowSession == nullptr)
		{
			// ������ ȣ���� �Լ���, ���� Ȯ���� �����ϱ� ������ OnError�Լ� ȣ�� ����.
			m_iMyErrorCode = euError::NETWORK_LIB_ERROR__NOT_FIND_CLINET;
			return false;
		}
	
		// 2. shutdown���� �ƴ� ������ �ֱ� ����
		NowSession->Struct_Lock();
		bool Check = NowSession->m_bShutdownState;
		NowSession->Struct_Unlock();

		if (Check == true)
		{
			// ������ ȣ���� �Լ���, ���� Ȯ���� �����ϱ� ������ OnError�Լ� ȣ�� ����.
			m_iMyErrorCode = euError::NETWORK_LIB_ERROR__DISCONNECT_WAIT_CLINET;
			return false;
		}
	
		// 3. ��� ����� (���̷ε� ����� ����)
		WORD Header = payloadBuff->GetUseSize();
	
		// 4. �ֱ�
		NowSession->m_SendQueue.EnterLOCK();  // �� ---------------------------	

		// ��� ��ť
		int EnqueueCheck = NowSession->m_SendQueue.Enqueue((char*)&Header, dfNETWORK_PACKET_HEADER_SIZE);
		if (EnqueueCheck == -1)
		{
			NowSession->m_SendQueue.LeaveLOCK();  // �� ���� ---------------------------	

												  // ������ ȣ���� �Լ���, ���� Ȯ���� �����ϱ� ������ OnError�Լ� ȣ�� ����.
			m_iMyErrorCode = euError::NETWORK_LIB_ERROR__SEND_QUEUE_SIZE_FULL;
			return false;
		}

		// ���̷ε� ��ť
		EnqueueCheck = NowSession->m_SendQueue.Enqueue(payloadBuff->GetBufferPtr(), Header);
		if (EnqueueCheck == -1)
		{
			NowSession->m_SendQueue.LeaveLOCK();  // �� ���� ---------------------------	

			// ������ ȣ���� �Լ���, ���� Ȯ���� �����ϱ� ������ OnError�Լ� ȣ�� ����.
			m_iMyErrorCode = euError::NETWORK_LIB_ERROR__SEND_QUEUE_SIZE_FULL;		
			return false;
		}

		NowSession->m_SendQueue.LeaveLOCK();  // �� ���� ---------------------------	

		// 5. SendPost�õ�
		SendPost(NowSession);

		return true;
	}

	// ������ ������ ���� �� ȣ���ϴ� �Լ�. �ܺ� ���� ���.
	// ���̺귯������ ������!��� ��û�ϴ� �� ��
	bool CLanServer::Disconnect(ULONGLONG ClinetID)
	{
		// ���� ã�´�.
		stSession* NowSession = FineSessionPtr(ClinetID);

		// ���� ��ã������ �����ڵ� ����� false ����
		if (NowSession == nullptr)
		{
			// �� ���� ����. (������ ������ ����)
			m_iMyErrorCode = euError::NETWORK_LIB_ERROR__NOT_FIND_CLINET;
			return false;
		}

		// ������ϴ� ������ �˴ٿ� ������.
		// ���� �ڿ������� I/Oī��Ʈ�� ���ҵǾ ��Ŀ��Ʈ�ȴ�.
		NowSession->Struct_Lock();
		NowSession->m_bShutdownState = true;
		NowSession->Struct_Unlock();

		shutdown(NowSession->m_Client_sock, SD_BOTH);

		return true;
	}

	///// ���� �����Լ��� /////
	// ������ ���� ���
	int CLanServer::WinGetLastError()
	{
		return m_iOSErrorCode;
	}

	// �� ���� ���
	int CLanServer::NetLibGetLastError()
	{
		return (int)m_iMyErrorCode;
	}

	// ���� ������ �� ���
	ULONGLONG CLanServer::GetClientCount()
	{
		return m_ullJoinUserCount;
	}

	// ���� �������� ���
	// return true : ������
	// return false : ������ �ƴ�
	bool CLanServer::GetServerState()
	{
		return m_bServerLife;
	}







	// -----------------------------
	// Lib ���ο����� ����ϴ� �Լ�
	// -----------------------------
	// ������
	CLanServer::CLanServer()
	{
		// ���� �������� false�� ���� 
		m_bServerLife = false;

		InitializeSRWLock(&m_srwSession_map_srwl);
	}

	// �Ҹ���
	CLanServer::~CLanServer()
	{
		// ���� �������� false�� ����
		m_bServerLife = false;

		// ���� ���� ������ ������ Stop�Լ� ȣ��.
		if (m_ullJoinUserCount != 0)
		{
			Stop();
		}
	}

	// ������ �������� ��Ű�� �Լ�
	void CLanServer::InDisconnect(stSession* DeleteSession)
	{
		ULONGLONG sessionID = DeleteSession->m_ullSessionID;
	
		// map���� ���ܽ�Ű��
		size_t retval;

		Lock_Map();
		retval = map_Session.erase(sessionID);
		Unlock_Map();

		// ����, ���� ������� �׳� �����Ȱɷ� ġ��, �����Ѵ�.
		if (retval == 0)
		{
			// ���� �Լ� ȣ��
			OnClientLeave(sessionID);
			printf("��������!!\n");
			return;
		}	

		// Ŭ���� ����, ���� ��������	
		closesocket(DeleteSession->m_Client_sock);
		delete DeleteSession;

		// ���� �� ����
		InterlockedDecrement(&m_ullJoinUserCount);

		// ���� �Լ� ȣ��
		OnClientLeave(sessionID);
	}

	// Start���� ������ �� �� ȣ���ϴ� �Լ�.
	// 1. ����� �Ϸ���Ʈ �ڵ� ��ȯ
	// 2. ��Ŀ������ ����, �ڵ��ȯ, �ڵ� �迭 ��������
	// 3. ����Ʈ ������ ����, �ڵ� ��ȯ
	// 4. �������� �ݱ�
	// 5. ���� ����
	void CLanServer::ExitFunc(int w_ThreadCount)
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
	UINT WINAPI	CLanServer::WorkerThread(LPVOID lParam)
	{
		int retval;

		DWORD cbTransferred;
		stSession* NowSession;
		OVERLAPPED* overlapped;

		CLanServer* g_This = (CLanServer*)lParam;

		while (1)
		{
			// ������ �ʱ�ȭ
			cbTransferred = 0;
			NowSession = nullptr;
			overlapped = nullptr;

			// GQCS �Ϸ� �� �Լ� ȣ��
			g_This->OnWorkerThreadEnd();

			// �񵿱� ����� �Ϸ� ���
			// GQCS ���
			retval = GetQueuedCompletionStatus(g_This->m_hIOCPHandle, &cbTransferred, (PULONG_PTR)&NowSession, &overlapped, INFINITE);

			// GQCS ��� �� �Լ�ȣ��
			g_This->OnWorkerThreadBegin();

			// --------------
			// �Ϸ� üũ
			// --------------
			// overlapped�� nullptr�̶��, IOCP ������.
			if (overlapped == NULL)
			{
				// �� �� 0�̸� ������ ����.
				if (cbTransferred == 0 && NowSession == NULL)
				{
					// printf("��Ŀ ������ ��������\n");

					break;
				}
				// �װ� �ƴϸ� IOCP ���� �߻��� ��

				// ������ ����, �� ���� ����
				g_This->m_iOSErrorCode = WSAGetLastError();
				g_This->m_iMyErrorCode = euError::NETWORK_LIB_ERROR__IOCP_ERROR;

				// ���� �߻� �Լ� ȣ��
				g_This->OnError((int)euError::NETWORK_LIB_ERROR__IOCP_ERROR, L"IOCP_Error");

				break;
			}

			// overlapped�� nullptr�� �ƴ϶�� �� ��û ���� ���� üũ
			else
			{
				// ���� �ϴ°� ����.			
			}

			// -----------------
			// Recv ����
			// -----------------
			// WSArecv()�� �Ϸ�� ���, ���� �����Ͱ� 0�� �ƴϸ� ���� ó��
			if (&NowSession->m_overRecvOverlapped == overlapped && cbTransferred > 0)
			{
				// rear �̵�
				NowSession->m_RecvQueue.MoveWritePos(cbTransferred);

				// 1. ������ ��, ��Ŷ ó��
				g_This->RecvProc(NowSession);

				// 2. ���ú� �ٽ� �ɱ�
				g_This->RecvPost(NowSession);
			}

			// -----------------
			// Send ����
			// -----------------
			// WSAsend()�� �Ϸ�� ���, ���� �����Ͱ� 0�� �ƴϸ� ����ó��
			else if (&NowSession->m_overSendOverlapped == overlapped && cbTransferred > 0)
			{
				// 1. front �̵�	
				NowSession->m_SendQueue.EnterLOCK();		// �� -----------------------

				NowSession->m_SendQueue.RemoveData(cbTransferred);

				NowSession->m_SendQueue.LeaveLOCK();		// �� ���� ------------------

				// 2. ���� ���� ���·� ����
				NowSession->m_lSendFlag = 0;

				// 3. �ٽ� ���� �õ�
				g_This->SendPost(NowSession);

				// 4. ���� �Ϸ�ƴٰ� �������� �˷���
				g_This->OnSend(NowSession->m_ullSessionID, cbTransferred);
			}

			// -----------------
			// I/Oī��Ʈ ���� �� �� �� ó��
			// -----------------
			// I/Oī��Ʈ ���� ��, 0�̶������ ����
			long NowVal = InterlockedDecrement(&NowSession->m_lIOCount);
			if (NowVal == 0)
				g_This->InDisconnect(NowSession);
		}
		return 0;
	}

	// Accept ������
	UINT WINAPI	CLanServer::AcceptThread(LPVOID lParam)
	{
		// --------------------------
		// Accept ��Ʈ
		// --------------------------
		SOCKET client_sock;
		SOCKADDR_IN	clientaddr;
		int addrlen;

		CLanServer* g_This = (CLanServer*)lParam;

		while (true)
		{
			ZeroMemory(&clientaddr, sizeof(clientaddr));
			addrlen = sizeof(clientaddr);
			client_sock = accept(g_This->m_soListen_sock, (SOCKADDR*)&clientaddr, &addrlen);
			if (client_sock == INVALID_SOCKET)
			{
				int Error = WSAGetLastError();
				// 10004�� ������ ��������.
				if (Error == WSAEINTR)
				{
					// Accept ������ ���� ����
					break;
				}

				// �װ� �ƴ϶�� OnError �Լ� ȣ��
				// ������ ����, �� ���� ����
				g_This->m_iOSErrorCode = Error;
				g_This->m_iMyErrorCode = euError::NETWORK_LIB_ERROR__A_THREAD_ABONORMAL_EXIT;

				// ���� �߻� �Լ� ȣ��
				g_This->OnError((int)euError::NETWORK_LIB_ERROR__A_THREAD_ABONORMAL_EXIT, L"accpet(). Abonormal_exit");

				break;
			}

			// ------------------
			// IP�� ��Ʈ �˾ƿ���.
			// ------------------
			TCHAR tcTempIP[30];
			InetNtop(AF_INET, &clientaddr.sin_addr, tcTempIP, 30);
			USHORT port = ntohs(clientaddr.sin_port);

			// ------------------
			// ���� ����, IP���� �Ǵ��ؼ� ���� �߰� �۾��� �ʿ��� ��찡 ���� ���� ������ ȣ��
			// ------------------
			bool Check = g_This->OnConnectionRequest(tcTempIP, port);
		
			// false�� ���Ӱź�, 
			// true�� ���� ��� ����. true���� �Ұ� ������ OnConnectionRequest�Լ� ���ڷ� ������ ������.
			if (Check == false)
				continue;


			// ------------------
			// ���� ����ü ���� �� ����
			// ------------------
			stSession* NewSession = new stSession;
			StringCchCopy(NewSession->m_IP, _MyCountof(NewSession->m_IP), tcTempIP);
			NewSession->m_prot = port;
			NewSession->m_Client_sock = client_sock;
			NewSession->m_ullSessionID = g_This->m_ullSessionID++;

			// ------------------
			// map ��� �� IOCP ����
			// ------------------
			// ���õ� ����ü�� map�� ���
			g_This->Lock_Map();
			g_This->map_Session.insert(pair<ULONGLONG, stSession*>(NewSession->m_ullSessionID, NewSession));
			g_This->Unlock_Map();


			// ���ϰ� IOCP ����
			CreateIoCompletionPort((HANDLE)client_sock, g_This->m_hIOCPHandle, (ULONG_PTR)NewSession, 0);
		
			// ������ �� ����. disconnect������ ���Ǵ� �����̱� ������ ���Ͷ� ���
			InterlockedIncrement(&g_This->m_ullJoinUserCount);

			// ------------------
			// �񵿱� ����� ����
			// ------------------
			g_This->RecvPost_Accept(NewSession);

			// ------------------
			// ��� ���������� �Ϸ�Ǿ����� ���� �� ó�� �Լ� ȣ��.
			// ------------------
			g_This->OnClientJoin(NewSession->m_ullSessionID);	
		}

		return 0;
	}

	// map�� �� �ɱ�
	void CLanServer::LockMap_Func()
	{
		AcquireSRWLockExclusive(&m_srwSession_map_srwl);
	}

	// map�� �� Ǯ��
	void CLanServer::UnlockMap_Func()
	{
		ReleaseSRWLockExclusive(&m_srwSession_map_srwl);
	}

	// ClinetID�� Session����ü ã�� �Լ�
	CLanServer::stSession* CLanServer::FineSessionPtr(ULONGLONG ClinetID)
	{
		// �� ------------------
		Lock_Map();
		map <ULONGLONG, stSession*>::iterator iter;

		iter = map_Session.find(ClinetID);
		if (iter == map_Session.end())
		{
			// �� �� ------------------
			Unlock_Map();
			return nullptr;
		}

		stSession* NowSession = iter->second;

		Unlock_Map();
		// �� �� ------------------

		return NowSession;
	}

	// ���� �������� �ʱⰪ���� �ʱ�ȭ
	void CLanServer::Init()
	{	
		m_iW_ThreadCount = 0;
		m_iA_ThreadCount = 0;
		m_hWorkerHandle = nullptr;
		m_hIOCPHandle = 0;
		m_soListen_sock = 0;
		m_iOSErrorCode = 0;
		m_iMyErrorCode = (euError)0;
		m_ullSessionID = 0;
		m_ullJoinUserCount = 0;
	}






	// ------------
	// Lib ���ο����� ����ϴ� ���ú� ���� �Լ���
	// ------------
	// RecvProc �Լ�. ť�� ���� üũ �� PacketProc���� �ѱ��.
	bool CLanServer::RecvProc(stSession* NowSession)
	{
		// -----------------
		// Recv ť ���� ó��
		// -----------------

		while (1)
		{
			// 1. RecvBuff�� �ּ����� ����� �ִ��� üũ. (���� = ��� ������� ���ų� �ʰ�. ��, �ϴ� �����ŭ�� ũ�Ⱑ �ִ��� üũ)	
			WORD Header_PaylaodSize = 0;

			// RecvBuff�� ��� ���� ���� ũ�Ⱑ ��� ũ�⺸�� �۴ٸ�, �Ϸ� ��Ŷ�� ���ٴ� ���̴� while�� ����.
			int UseSize = NowSession->m_RecvQueue.GetUseSize();
			if (UseSize < dfNETWORK_PACKET_HEADER_SIZE)
			{
				break;
			}

			// 2. ����� Peek���� Ȯ���Ѵ�.  Peek �ȿ�����, ��� �ؼ����� len��ŭ �д´�. 
			// ���۰� ��������� ���� ����.
			int PeekSize = NowSession->m_RecvQueue.Peek((char*)&Header_PaylaodSize, dfNETWORK_PACKET_HEADER_SIZE);
			if (PeekSize == -1)
			{
				// �ϴ� ������ϴ� �˴ٿ� ȣ��
				NowSession->Struct_Lock();
				NowSession->m_bShutdownState = true;
				NowSession->Struct_Unlock();

				shutdown(NowSession->m_Client_sock, SD_BOTH);

				// �� ���� ����. ������ ������ ����.
				m_iMyErrorCode = euError::NETWORK_LIB_ERROR__EMPTY_RECV_BUFF;

				// ���� ��Ʈ�� �����
				TCHAR tcErrorString[300];
				StringCchPrintf(tcErrorString, 300, _T("RecvRingBuff_Empry.UserID : %d, [%s:%d]"), 
					NowSession->m_ullSessionID, NowSession->m_IP, NowSession->m_prot);

				// ���� �Լ� ȣ��
				OnError((int)euError::NETWORK_LIB_ERROR__EMPTY_RECV_BUFF, tcErrorString);
			
				return false;
			}

			// 3. �ϼ��� ��Ŷ�� �ִ��� Ȯ��. (�ϼ� ��Ŷ ������ = ��� ������ + ���̷ε� Size)
			// ��� ���, �ϼ� ��Ŷ ����� �ȵǸ� while�� ����.
			if (UseSize < (dfNETWORK_PACKET_HEADER_SIZE + Header_PaylaodSize))
			{
				break;
			}

			// 4. RecvBuff���� Peek�ߴ� ����� ����� (�̹� Peek������, �׳� Remove�Ѵ�)
			NowSession->m_RecvQueue.RemoveData(dfNETWORK_PACKET_HEADER_SIZE);

			// 5. RecvBuff���� ���̷ε� Size ��ŭ ���̷ε� ����ȭ ���۷� �̴´�. (��ť�̴�. Peek �ƴ�)
			CProtocolBuff PayloadBuff;

			int DequeueSize = NowSession->m_RecvQueue.Dequeue(PayloadBuff.GetBufferPtr(), Header_PaylaodSize);
			// ���۰� ��������� ���� ����
			if (DequeueSize == -1)
			{
				// �ϴ� ������ϴ� �˴ٿ� ȣ��
				NowSession->Struct_Lock();
				NowSession->m_bShutdownState = true;
				NowSession->Struct_Unlock();

				shutdown(NowSession->m_Client_sock, SD_BOTH);

				// �� ���� ����. ������ ������ ����.
				m_iMyErrorCode = euError::NETWORK_LIB_ERROR__EMPTY_RECV_BUFF;

				// ���� ��Ʈ�� �����
				TCHAR tcErrorString[300];
				StringCchPrintf(tcErrorString, 300, _T("RecvRingBuff_Empry.UserID : %d, [%s:%d]"),
					NowSession->m_ullSessionID, NowSession->m_IP, NowSession->m_prot);

				// ���� �Լ� ȣ��
				OnError((int)euError::NETWORK_LIB_ERROR__EMPTY_RECV_BUFF, tcErrorString);
				return false;
			}
			PayloadBuff.MoveWritePos(DequeueSize);

			// 9. ����� ����ִ� Ÿ�Կ� ���� �б�ó��.
			OnRecv(NowSession->m_ullSessionID, &PayloadBuff);

		}

		return true;
	}

	// Accept�� RecvProc�Լ�
	bool CLanServer::RecvPost_Accept(stSession* NowSession)
	{
		// ------------------
		// �񵿱� ����� ����
		// ------------------
		// 1. WSABUF ����.
		WSABUF wsabuf[2];
		int wsabufCount = 0;

		int FreeSize = NowSession->m_RecvQueue.GetFreeSize();
		int Size = NowSession->m_RecvQueue.GetNotBrokenPutSize();

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
		int retval = WSARecv(NowSession->m_Client_sock, wsabuf, wsabufCount, &recvBytes, &flags, &NowSession->m_overRecvOverlapped, NULL);


		// 4. ���� ó��
		if (retval == SOCKET_ERROR)
		{
			int Error = WSAGetLastError();

			// �񵿱� ������� ���۵Ȱ� �ƴ϶��
			if (Error != WSA_IO_PENDING)
			{
				// I/Oī��Ʈ 1����.
				long Nowval = InterlockedDecrement(&NowSession->m_lIOCount);

				// I/O ī��Ʈ�� 0�̶�� ���� ����.
				if (Nowval == 0)
					InDisconnect(NowSession);

				// ������ ���� �����̶��, �ش� ������ ����.
				if (Error == WSAENOBUFS)
				{				
					// �ϴ� ������ϴ� �˴ٿ� ȣ��
					NowSession->Struct_Lock();
					NowSession->m_bShutdownState = true;
					NowSession->Struct_Unlock();

					shutdown(NowSession->m_Client_sock, SD_BOTH);

					// �� ����, �����쿡�� ����
					m_iOSErrorCode = Error;
					m_iMyErrorCode = euError::NETWORK_LIB_ERROR__WSAENOBUFS;

					// ���� ��Ʈ�� �����
					TCHAR tcErrorString[300];
					StringCchPrintf(tcErrorString, 300, _T("WSANOBUFS. UserID : %d, [%s:%d]"),
						NowSession->m_ullSessionID, NowSession->m_IP, NowSession->m_prot);

					// ���� �Լ� ȣ��
					OnError((int)euError::NETWORK_LIB_ERROR__WSAENOBUFS, tcErrorString);
				}

				return false;
			}
		}

		return true;
	}

	// RecvPost �Լ�. �񵿱� ����� ����
	bool CLanServer::RecvPost(stSession* NowSession)
	{
		// ------------------
		// �񵿱� ����� ����
		// ------------------
		// 1. WSABUF ����.
		WSABUF wsabuf[2];
		int wsabufCount = 0;

		int FreeSize = NowSession->m_RecvQueue.GetFreeSize();
		int Size = NowSession->m_RecvQueue.GetNotBrokenPutSize();

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
		int retval = WSARecv(NowSession->m_Client_sock, wsabuf, wsabufCount, &recvBytes, &flags, &NowSession->m_overRecvOverlapped, NULL);

		// 4. ���� ó��
		if (retval == SOCKET_ERROR)
		{
			int Error = WSAGetLastError();

			// �񵿱� ������� ���۵Ȱ� �ƴ϶��
			if (Error != WSA_IO_PENDING)
			{
				InterlockedDecrement(&NowSession->m_lIOCount);

				// ������ ���� �����̶��
				if (Error == WSAENOBUFS)
				{			
					// �ϴ� ������ϴ� �˴ٿ� ȣ��
					NowSession->Struct_Lock();
					NowSession->m_bShutdownState = true;
					NowSession->Struct_Unlock();

					shutdown(NowSession->m_Client_sock, SD_BOTH);

					// �� ����, �����쿡�� ����
					m_iOSErrorCode = Error;
					m_iMyErrorCode = euError::NETWORK_LIB_ERROR__WSAENOBUFS;

					// ���� ��Ʈ�� �����
					TCHAR tcErrorString[300];
					StringCchPrintf(tcErrorString, 300, _T("WSANOBUFS. UserID : %d, [%s:%d]"),
						NowSession->m_ullSessionID, NowSession->m_IP, NowSession->m_prot);

					// ���� �Լ� ȣ��
					OnError((int)euError::NETWORK_LIB_ERROR__WSAENOBUFS, tcErrorString);
				}

				return false;
			}
		}

		return true;
	}






	// ------------
	// Lib ���ο����� ����ϴ� ���� ���� �Լ���
	// ------------
	// ���� ������ ������ WSASend() �ϱ�
	bool CLanServer::SendPost(stSession* NowSession)
	{
		while (1)
		{
			// ------------------
			// send ���� �������� üũ
			// ------------------
			// 1. SendFlag(1������)�� 0(3������)�� ���ٸ�, SendFlag(1������)�� 1(2������)���� ����
			// ���⼭ TRUE�� ���ϵǴ� ����, �̹� NowSession->m_SendFlag�� 1(���� ��)�̾��ٴ� ��.
			if (InterlockedCompareExchange(&NowSession->m_lSendFlag, TRUE, FALSE) == TRUE)
			{
				return true;
			}

			// 2. SendBuff�� �����Ͱ� �ִ��� Ȯ��
			// ���⼭ ���� UseSize�� ���� ������ �̴�. �Ʒ����� ���� �����Ҷ��� ����Ѵ�.
			int UseSize = NowSession->m_SendQueue.GetUseSize();
			if (UseSize == 0)
			{
				// WSASend �Ȱɾ��� ������, ���� ���� ���·� �ٽ� ����.
				NowSession->m_lSendFlag = 0;

				// 3. ��¥�� ����� ������ �ٽ��ѹ� üũ. �� Ǯ�� �Դµ�, ���ؽ�Ʈ ����Ī �Ͼ�� �ٸ� �����尡 �ǵ���� ���ɼ�
				// ������ ������ ���� �ö󰡼� �ѹ� �� �õ�
				if (NowSession->m_SendQueue.GetUseSize() > 0)
				{
					continue;
				}

				break;
			}

			// ------------------
			// Send �غ��ϱ�
			// ------------------
			// 1. WSABUF ����.
			WSABUF wsabuf[2];
			int wsabufCount = 0;

			// 2. BrokenSize ���ϱ�
			// !!!!���� ����!!! 
			// ���� BrokenSize��, UseSize���� ���� ���ؿ��� �帧�� ������!!! 
			// 
			// ------------
			// �ó�����...
			// [Front = 0, rear = 10]�� ��Ȳ�� BrokenSize�� ���ϸ� 10�� ���´�. (BrokenSize = 10)
			// �� ���̿� ���ؽ�Ʈ ����Ī�� �Ͼ�� �ٸ� �����尡 �����͸� �ִ´�. (Fornt = 0, rear = 20)
			// �ٽ� ���ؽ�Ʈ ����Ī���� �ش� ������� ���ƿ� �� UseSize�� ���ϸ� 20�� ���´� (Front = 0, rear = 20, BrokenSize = 10, UseSize = 20)
			// ------------
			// 
			// �� ��Ȳ����, BrokenSize�� UseSize�� ���ϸ�, ���� ��Ȳ�� �ƴѵ� ���۸� 2�� ����ϰԵȴ�.
			// �� ���, [0]�� ���ۿ� &Buff[0]�� ���� ����� 10, [1]�� ���ۿ��� &Buff[0]�� ���� ����� 10�� ���� ��Ȳ�� �߻��Ѵ�.
			// ������ �����Ͱ� ���� �ȴ�!! 
			// �� ���, ����ʿ����� ������ ���� ������?�� ������ �������� �ְ�.. Ȥ�� �׳� ���� �̻��ϰ� ó���Ǽ� ������ ���带 ���Ҽ��� �ְ� �׷���.
			// ��������!!!!!

			int BrokenSize = NowSession->m_SendQueue.GetNotBrokenGetSize();

			// 3. UseSize�� �� ũ�ٸ�, ���۰� ����ٴ� �Ҹ�. 2���� ��� ������.
			if (BrokenSize <  UseSize)
			{
				// fornt ��ġ�� ���۸� �����ͷ� ����(���� 1ĭ �տ� ���� ������, �� �ȿ��� 1ĭ �ձ��� �������)
				wsabuf[0].buf = NowSession->m_SendQueue.GetFrontBufferPtr();
				wsabuf[0].len = BrokenSize;

				wsabuf[1].buf = NowSession->m_SendQueue.GetBufferPtr();;
				wsabuf[1].len = UseSize - BrokenSize;
				wsabufCount = 2;
			}

			// 3-2. �װ� �ƴ϶��, WSABUF�� 1���� �����Ѵ�.
			else
			{
				// fornt ��ġ�� ���۸� �����ͷ� ����(���� 1ĭ �տ� ���� ������, �� �ȿ��� 1ĭ �ձ��� �������)
				wsabuf[0].buf = NowSession->m_SendQueue.GetFrontBufferPtr();
				wsabuf[0].len = BrokenSize;

				wsabufCount = 1;
			}

			// 4. Overlapped ����ü �ʱ�ȭ
			ZeroMemory(&NowSession->m_overSendOverlapped, sizeof(NowSession->m_overSendOverlapped));

			// 5. WSASend()
			DWORD SendBytes = 0, flags = 0;
			InterlockedIncrement(&NowSession->m_lIOCount);
			int retval = WSASend(NowSession->m_Client_sock, wsabuf, wsabufCount, &SendBytes, flags, &NowSession->m_overSendOverlapped, NULL);

			// 6. ���� ó��
			if (retval == SOCKET_ERROR)
			{
				int Error = WSAGetLastError();

				// �񵿱� ������� ���۵Ȱ� �ƴ϶��
				if (Error != WSA_IO_PENDING)
				{
					if (Error != WSAESHUTDOWN && Error != WSAECONNRESET && Error != WSAECONNABORTED)
					{
						// �ϴ� ������ϴ� �˴ٿ� ȣ��
						NowSession->Struct_Lock();
						NowSession->m_bShutdownState = true;
						NowSession->Struct_Unlock();

						shutdown(NowSession->m_Client_sock, SD_BOTH);

						// �� ����, �����쿡�� ����
						m_iOSErrorCode = Error;
						m_iMyErrorCode = euError::NETWORK_LIB_ERROR__WSASEND_FAIL;

						// ���� ��Ʈ�� �����
						TCHAR tcErrorString[300];
						StringCchPrintf(tcErrorString, 300, _T("WSASendFail... UserID : %d, [%s:%d]"),
							NowSession->m_ullSessionID, NowSession->m_IP, NowSession->m_prot);

						// ���� �Լ� ȣ��
						OnError((int)euError::NETWORK_LIB_ERROR__WSASEND_FAIL, tcErrorString);
					}

					// ������ ���� �����̶��
					else if (Error == WSAENOBUFS)
					{
						// �ϴ� ������ϴ� �˴ٿ� ȣ��
						NowSession->Struct_Lock();
						NowSession->m_bShutdownState = true;
						NowSession->Struct_Unlock();

						shutdown(NowSession->m_Client_sock, SD_BOTH);

						// �� ����, �����쿡�� ����
						m_iOSErrorCode = Error;
						m_iMyErrorCode = euError::NETWORK_LIB_ERROR__WSAENOBUFS;

						// ���� ��Ʈ�� �����
						TCHAR tcErrorString[300];
						StringCchPrintf(tcErrorString, 300, _T("WSANOBUFS. UserID : %d, [%s:%d]"),
							NowSession->m_ullSessionID, NowSession->m_IP, NowSession->m_prot);

						// ���� �Լ� ȣ��
						OnError((int)euError::NETWORK_LIB_ERROR__WSAENOBUFS, tcErrorString);
					}

					return false;
				}
			}
			break;
		}

		return true;
	}

}


