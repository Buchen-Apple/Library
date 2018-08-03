#include "stdafx.h"

#pragma comment(lib,"ws2_32")
#include <Ws2tcpip.h>

#include <process.h>
#include <Windows.h>
#include <strsafe.h>

#include "NetworkLib.h"
#include "RingBuff\RingBuff.h"
#include "Log\Log.h"
#include "CrashDump\CrashDump.h"


<<<<<<< HEAD
LONG g_llPacketAllocCount = 0;
=======
#include <mmsystem.h>
#pragma comment(lib,"winmm.lib")

>>>>>>> parent of 4721bdb... 218-08-03 7차과제 ver 04


namespace Library_Jingyu
{
	// �α� ���� �������� �ϳ� �ޱ�.
	CSystemLog* cNetLibLog = CSystemLog::GetInstance();	

	// ���� ���� ���� �ϳ� �ޱ�
	CCrashDump* cNetDump = CCrashDump::GetInstance();


	// ��� ������
	#define dfNETWORK_PACKET_HEADER_SIZE	2

	// �� ���� ������ �� �ִ� WSABUF�� ī��Ʈ
	#define dfSENDPOST_MAX_WSABUF			100
<<<<<<< HEAD
	
	void PacketAllocCountAdd()
	{
		InterlockedIncrement(&g_llPacketAllocCount);
	}

	LONG PacketAllocCountSub()
	{
		return InterlockedDecrement(&g_llPacketAllocCount);
	}

	LONG PacketAllocCount_Get()
	{
		return g_llPacketAllocCount;
	}
=======
>>>>>>> parent of 4721bdb... 218-08-03 7차과제 ver 04


	// ------------------------------
	// enum�� ����ü
	// ------------------------------
	// enum class
	enum class CLanServer::euError : int
	{
		NETWORK_LIB_ERROR__NORMAL = 0,					// ���� ���� �⺻ ����
		NETWORK_LIB_ERROR__WINSTARTUP_FAIL,				// ���� �ʱ�ȭ �ϴٰ� ������
		NETWORK_LIB_ERROR__CREATE_IOCP_PORT,			// IPCP ����ٰ� ������
		NETWORK_LIB_ERROR__W_THREAD_CREATE_FAIL,		// ��Ŀ������ ����ٰ� ���� 
		NETWORK_LIB_ERROR__A_THREAD_CREATE_FAIL,		// ����Ʈ ������ ����ٰ� ���� 
		NETWORK_LIB_ERROR__CREATE_SOCKET_FAIL,			// ���� ���� ���� 
		NETWORK_LIB_ERROR__BINDING_FAIL,				// ���ε� ����
		NETWORK_LIB_ERROR__LISTEN_FAIL,					// ���� ����
		NETWORK_LIB_ERROR__SOCKOPT_FAIL,				// ���� �ɼ� ���� ����
		NETWORK_LIB_ERROR__IOCP_ERROR,					// IOCP ��ü ����
		NETWORK_LIB_ERROR__NOT_FIND_CLINET,				// map �˻� ���� �Ҷ� Ŭ���̾�Ʈ�� ��ã�����.
		NETWORK_LIB_ERROR__SEND_QUEUE_SIZE_FULL	,		// Enqueue����� ���� ����
		NETWORK_LIB_ERROR__QUEUE_DEQUEUE_EMPTY,			// Dequeue ��, ť�� ����ִ� ����. Peek�� �õ��ϴµ� ť�� ����� ��Ȳ�� ����
		NETWORK_LIB_ERROR__WSASEND_FAIL,				// SendPost���� WSASend ����
		NETWORK_LIB_ERROR__WSAENOBUFS,					// WSASend, WSARecv�� ���ۻ����� ����
		NETWORK_LIB_ERROR__EMPTY_RECV_BUFF,				// Recv �Ϸ������� �Դµ�, ���ú� ���۰� ����ִٰ� ������ ����.
		NETWORK_LIB_ERROR__A_THREAD_ABNORMAL_EXIT,		// ����Ʈ ������ ������ ����. ���� accept()�Լ����� �̻��� ������ ���°�.
		NETWORK_LIB_ERROR__W_THREAD_ABNORMAL_EXIT,		// ��Ŀ ������ ������ ����. 
		NETWORK_LIB_ERROR__WFSO_ERROR,					// WaitForSingleObject ����.
		NETWORK_LIB_ERROR__IOCP_IO_FAIL					// IOCP���� I/O ���� ����. �� ����, ���� Ƚ���� I/O�� ��õ��Ѵ�.
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
		
		// I/O�� �ܺ����� ����(�������� ���� ���������..)���� �������� �� �� 5ȸ���� ��õ� �غ���. 
		// �� �õ��� Ƚ���� ������ ����. ������ �� ���� 1�� �����ϴٰ�, 5���Ǹ� �� �̻� ���ӽõ� ����
		// I/O�� �����ϸ� �� ��� 0�̵ȴ�.
		//LONG	m_lReyryCount;

		// �ش� �ε��� �迭�� ��������� üũ
		// 1�̸� �����, 0�̸� ����� �ƴ�
		LONG m_lUseFlag;

		// �ش� �迭�� ���� �ε���
		ULONGLONG m_lIndex;

		// ����, WSASend�� �� ���� �����͸� �����ߴ°�. (����Ʈ �ƴ�! ī��Ʈ. ����!)
		long m_iWSASendCount;
		
		// SendPost���� ���带 �� �������� �����͵� �迭
		CProtocolBuff* m_cpbufSendPayload[dfSENDPOST_MAX_WSABUF];

		CRingBuff m_RecvQueue;
		CRingBuff m_SendQueue;

		OVERLAPPED m_overRecvOverlapped;
		OVERLAPPED m_overSendOverlapped;

		// ������
		stSession()
		{			
			m_lIOCount = 0;
			m_lSendFlag = 0;
			m_lIndex = 0;
			m_lUseFlag = FALSE;
			m_iWSASendCount = 0;
		}

		void Reset_Func()
		{
			// -- SendFlag, I/Oī��Ʈ, SendCount �ʱ�ȭ
			m_lIOCount = 0;
			m_lSendFlag = 0;

			// ť �ʱ�ȭ
			m_RecvQueue.ClearBuffer();
			m_SendQueue.ClearBuffer();
		}

		// �Ҹ���
		// �ʿ������
		//~stSession() {}

	
	};

	// �̻�� �ε��� ���� ����ü(����)
	struct CLanServer::stEmptyStack
	{
		// ������ Last-In-First_Out
		int m_iTop;
		int m_iMax;

		ULONGLONG* m_iArray;

		// �ε��� ���
		//
		// ��ȯ�� ULONGLONG ---> �� ��ȯ������ ����Ʈ ���� �� ���� �־, Ȥ�� �𸣴� unsigned�� �����Ѵ�.
		// 0�̻�(0����) : �ε��� ���� ��ȯ
		// 10000000(õ��) : �� �ε��� ����.
		ULONGLONG Pop()
		{
			// �ε��� ����� üũ ----------
			// �� �ε����� ������ 18,000,000,000,000,000,000 
			// ���� Max�� ������ ������ ��� ������ ���, �� �ε����� ����.
			if (m_iTop == 0)
				return 10000000;

			// �� �ε����� ������, m_iTop�� ���� �� �ε��� ����.
			m_iTop--;			

			return m_iArray[m_iTop];
		}

		// �ε��� �ֱ�
		//
		// ��ȯ�� bool
		// true : �ε��� �������� ��
		// false : �ε��� ���� ���� (�̹� Max��ŭ �� ��)
		bool Push(ULONGLONG PushIndex)
		{
			// �ε����� ��á�� üũ ---------
			// �ε����� ��á���� false ����
			if (m_iTop == m_iMax)
				return false;

			// �ε����� ������ �ʾ�����, �߰�
			m_iArray[m_iTop] = PushIndex;
			m_iTop++;

			return true;
		}

		stEmptyStack(int Max)
		{
			m_iTop = Max;
			m_iMax = Max;

			m_iArray = new ULONGLONG[Max];

			// ���� ���� ��, ��� �� �ε���
			for (int i = 0; i < Max; ++i)
				m_iArray[i] = i;
		}

		~stEmptyStack()
		{
			delete[] m_iArray;
		}

	};



	// -----------------------------
	// ������ ȣ�� �ϴ� �Լ�
	// -----------------------------
	// ���� ����
	// [���� IP(���ε� �� IP), ��Ʈ, ��Ŀ������ ��, ����Ʈ ������ ��, TCP_NODELAY ��� ����(true�� ���), �ִ� ������ ��] �Է¹���.
	//
	// return false : ���� �߻� ��. �����ڵ� ���� �� false ����
	// return true : ����
	bool CLanServer::Start(const TCHAR* bindIP, USHORT port, int WorkerThreadCount, int AcceptThreadCount, bool Nodelay, int MaxConnect)
	{		

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
			cNetLibLog->LogSave(L"LanServer", CSystemLog::en_LogLevel::LEVEL_ERROR, L"Start() --> WSAStartup() Error : NetError(%d), OSError(%d)", 
				(int)m_iMyErrorCode, m_iOSErrorCode);
		
			// false ����
			return false;
		}

		// ����� �Ϸ� ��Ʈ ����
		m_hIOCPHandle = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, NULL, 0);
		if (m_hIOCPHandle == NULL)
		{
			// ������ ����, �� ���� ����
			m_iOSErrorCode = WSAGetLastError();
			m_iMyErrorCode = euError::NETWORK_LIB_ERROR__CREATE_IOCP_PORT;

			// ���� �ڵ� ��ȯ �� �������� ����.
			ExitFunc(m_iW_ThreadCount);

			// �α� ��� (�α� ���� : ����)
			cNetLibLog->LogSave(L"LanServer", CSystemLog::en_LogLevel::LEVEL_ERROR, L"Start() --> CreateIoCompletionPort() Error : NetError(%d), OSError(%d)",
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
				cNetLibLog->LogSave(L"LanServer", CSystemLog::en_LogLevel::LEVEL_ERROR, L"Start() --> WorkerThread Create Error : NetError(%d), OSError(%d)",
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
			cNetLibLog->LogSave(L"LanServer", CSystemLog::en_LogLevel::LEVEL_ERROR, L"Start() --> socket() Error : NetError(%d), OSError(%d)",
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
			cNetLibLog->LogSave(L"LanServer", CSystemLog::en_LogLevel::LEVEL_ERROR, L"Start() --> bind() Error : NetError(%d), OSError(%d)",
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
			cNetLibLog->LogSave(L"LanServer", CSystemLog::en_LogLevel::LEVEL_ERROR, L"Start() --> listen() Error : NetError(%d), OSError(%d)",
				(int)m_iMyErrorCode, m_iOSErrorCode);

			// false ����
			return false;
		}

		// ����� ���� ������ �۽Ź��� ũ�⸦ 0���� ����. �׷��� ���������� �񵿱� ��������� ����
		// ���� ���ϸ� �ٲٸ� ��� Ŭ�� �۽Ź��� ũ��� 0�̵ȴ�.
		int optval = 0;
		retval = setsockopt(m_soListen_sock, SOL_SOCKET, SO_SNDBUF, (char*)&optval, sizeof(optval));
		if (optval == SOCKET_ERROR)
		{
			// ������ ����, �� ���� ����
			m_iOSErrorCode = WSAGetLastError();
			m_iMyErrorCode = euError::NETWORK_LIB_ERROR__SOCKOPT_FAIL;

			// ���� �ڵ� ��ȯ �� �������� ����.
			ExitFunc(m_iW_ThreadCount);

			// �α� ��� (�α� ���� : ����)
			cNetLibLog->LogSave(L"LanServer", CSystemLog::en_LogLevel::LEVEL_ERROR, L"Start() --> setsockopt() SendBuff Size Change Error : NetError(%d), OSError(%d)",
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
				cNetLibLog->LogSave(L"LanServer", CSystemLog::en_LogLevel::LEVEL_ERROR, L"Start() --> setsockopt() Nodelay apply Error : NetError(%d), OSError(%d)",
					(int)m_iMyErrorCode, m_iOSErrorCode);

				// false ����
				return false;
			}
		}

		// �ִ� ���� ���� ���� �� ����
		m_iMaxJoinUser = MaxConnect;

		// ���� �迭 �����Ҵ�, �̻�� ���� ���� ���� �����Ҵ�.
		m_stSessionArray = new stSession[MaxConnect];
		m_stEmptyIndexStack = new stEmptyStack(MaxConnect);

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
				cNetLibLog->LogSave(L"LanServer", CSystemLog::en_LogLevel::LEVEL_ERROR, L"Start() --> Accept Thread Create Error : NetError(%d), OSError(%d)",
					(int)m_iMyErrorCode, m_iOSErrorCode);

				// false ����
				return false;
			}
		}

		

		// ���� ������ !!
		m_bServerLife = true;		

		// ���� ���� �α� ���		
		cNetLibLog->LogSave(L"LanServer", CSystemLog::en_LogLevel::LEVEL_SYSTEM, L"ServerOpen...");

		return true;
	}

	// ���� ��ž.
	void CLanServer::Stop()
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
			cNetLibLog->LogSave(L"LanServer", CSystemLog::en_LogLevel::LEVEL_ERROR, L"Stop() --> Accept Thread EXIT Error : NetError(%d), OSError(%d)",
				(int)m_iMyErrorCode, m_iOSErrorCode);

			// ���� �߻� �Լ� ȣ��
			OnError((int)euError::NETWORK_LIB_ERROR__WFSO_ERROR, L"Stop() --> Accept Thread EXIT Error");
		}

		// 2. ��� �������� Shutdown
		// ��� �������� �˴ٿ� ����
		for (int i = 0; i < m_iMaxJoinUser; ++i)
		{
			if(m_stSessionArray[i].m_lUseFlag == TRUE)
				shutdown(m_stSessionArray[i].m_Client_sock, SD_BOTH);				
		}

		// ��� ������ ����Ǿ����� üũ
		while (1)
		{
			if (m_ullJoinUserCount == 0)
				break;

			Sleep(1);
		}

		// 3. ��Ŀ ������ ����
		for (int i = 0; i<m_iW_ThreadCount; ++i)
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
			cNetLibLog->LogSave(L"LanServer", CSystemLog::en_LogLevel::LEVEL_ERROR, L"Stop() --> Worker Thread EXIT Error : NetError(%d), OSError(%d)",
				(int)m_iMyErrorCode, m_iOSErrorCode);

			// ���� �߻� �Լ� ȣ��
			OnError((int)euError::NETWORK_LIB_ERROR__WFSO_ERROR, L"Stop() --> Worker Thread EXIT Error");
		}

		// 4. ���� ���ҽ� ��ȯ
		// 1) ����Ʈ ������ �ڵ� ��ȯ
		for(int i=0; i<m_iA_ThreadCount; ++i)
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
		delete[] m_stEmptyIndexStack;

		// 5. ���� ������ �ƴ� ���·� ����
		m_bServerLife = false;

		// 6. ���� ���� �ʱ�ȭ
		Reset();

		// 7. ���� ���� �α� ���		
<<<<<<< HEAD
		_tprintf_s(L"PacketCount : [%d]\n", g_llPacketAllocCount);

		//g_llPacketAllocCount = 0;
=======
>>>>>>> parent of 4721bdb... 218-08-03 7차과제 ver 04
		cNetLibLog->LogSave(L"LanServer", CSystemLog::en_LogLevel::LEVEL_SYSTEM, L"ServerClose...");
	}

	// �ܺο���, � �����͸� ������ ������ ȣ���ϴ� �Լ�.
	// SendPacket�� �׳� �ƹ����� �ϸ� �ȴ�.
	// �ش� ������ SendQ�� �־�״ٰ� ���� �Ǹ� ������.
	//
	// return true : SendQ�� ���������� ������ ����.
	// return false : SendQ�� ������ �ֱ� ���� or ���ϴ� ���� ��ã��
	bool CLanServer::SendPacket(ULONGLONG ClinetID, CProtocolBuff* payloadBuff)
	{
		// 1. ClinetID�� ������ Index �˾ƿ���
		ULONGLONG wArrayIndex = GetSessionIndex(ClinetID);

		// 2. �̻�� �迭�̸� ���� �߸��� ���̴� false ����
		if (m_stSessionArray[wArrayIndex].m_lUseFlag == FALSE)
		{
			// ������ ȣ���� �Լ���, ���� Ȯ���� �����ϱ� ������ OnError�Լ� ȣ�� ����.
			m_iMyErrorCode = euError::NETWORK_LIB_ERROR__NOT_FIND_CLINET;

			// �α� ��� (�α� ���� : ����)
			// �ʿ��ϸ� ��´�. ���� �Ʒ��ִ°� ���ſ� ������
			//cNetLibLog->LogSave(L"LanServer", CSystemLog::en_LogLevel::LEVEL_ERROR, L"SendPacket() --> Not Fine Clinet :  NetError(%d)",(int)m_iMyErrorCode);

			return false;
		}		
	
		// 3. ����� �־, ��Ŷ �ϼ��ϱ�
		SetProtocolBuff_HeaderSet(payloadBuff);

		// 4. ��ť. ��Ŷ�� "�ּ�"�� ��ť�Ѵ�(8����Ʈ)
		void* payloadBuffAddr = payloadBuff;

		int EnqueueCheck = m_stSessionArray[wArrayIndex].m_SendQueue.Enqueue((char*)&payloadBuffAddr, sizeof(void*));
		if (EnqueueCheck == -1)
		{
			// ������ ȣ���� �Լ���, ���� Ȯ���� �����ϱ� ������ OnError�Լ� ȣ�� ����.
			m_iMyErrorCode = euError::NETWORK_LIB_ERROR__SEND_QUEUE_SIZE_FULL;

			// �α� ��� (�α� ���� : ����)
			cNetLibLog->LogSave(L"LanServer", CSystemLog::en_LogLevel::LEVEL_ERROR, L"SendPacket() --> Send Queue Full Clinet : [%s : %d] NetError(%d)",
				m_stSessionArray[wArrayIndex].m_IP, m_stSessionArray[wArrayIndex].m_prot, (int)m_iMyErrorCode);

			// �ش� ������ ������ ���´�.
			shutdown(m_stSessionArray[wArrayIndex].m_Client_sock, SD_BOTH);
			
			return false;
		}		

		// 4. SendPost�õ�
		bool Check = SendPost(&m_stSessionArray[wArrayIndex]);		

		return Check;
	}

	// ������ ������ ���� �� ȣ���ϴ� �Լ�. �ܺ� ���� ���.
	// ���̺귯������ ������!��� ��û�ϴ� �� ��
	//
	// return true : �ش� �������� �˴ٿ� �� ����.
	// return false : ���������� ���� ������ ���������Ϸ��� ��.
	bool CLanServer::Disconnect(ULONGLONG ClinetID)
	{
		// 1. ClinetID�� ������ Index �˾ƿ���
		ULONGLONG wArrayIndex = GetSessionIndex(ClinetID);

		// 2. ���� ã�� ������ �ƴϰų�, �̻�� �迭�̸� ���� �߸��� ���̴� false ����
		if (m_stSessionArray[wArrayIndex].m_lIndex != wArrayIndex || m_stSessionArray[wArrayIndex].m_lUseFlag == FALSE)
		{
			// �� ���� ����. (������ ������ ����)
			m_iMyErrorCode = euError::NETWORK_LIB_ERROR__NOT_FIND_CLINET;

			// �α� ��� (�α� ���� : ����)
			cNetLibLog->LogSave(L"LanServer", CSystemLog::en_LogLevel::LEVEL_ERROR, L"SendPacket() --> Not Fine Clinet : NetError(%d)", (int)m_iMyErrorCode);

			return false;
		}

		// ������ϴ� ������ �˴ٿ� ������.
		// ���� �ڿ������� I/Oī��Ʈ�� ���ҵǾ ��Ŀ��Ʈ�ȴ�.
		shutdown(m_stSessionArray[wArrayIndex].m_Client_sock, SD_BOTH);

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
		// �α� �ʱ�ȭ
		cNetLibLog->SetDirectory(L"LanServer");
		cNetLibLog->SetLogLeve(CSystemLog::en_LogLevel::LEVEL_DEBUG);

		// ���� �������� false�� ���� 
		m_bServerLife = false;

		// SRWLock �ʱ�ȭ
		InitializeSRWLock(&m_srwSession_stack_srwl);
	}

	// �Ҹ���
	CLanServer::~CLanServer()
	{
		// ������ �������̾�����, ���¸� false �� �ٲٰ�, ���� �������� ����
		if (m_bServerLife == true )
		{
			m_bServerLife = false;
			Stop();
		}
	}

	// ������ �������� ��Ű�� �Լ�
	void CLanServer::InDisconnect(stSession* DeleteSession)
<<<<<<< HEAD
	{	
=======
	{

		int TempCount = DeleteSession->m_iWSASendCount;
		DeleteSession->m_iWSASendCount = 0;  // ���� ī��Ʈ 0���� ����.
>>>>>>> parent of 4721bdb... 218-08-03 7차과제 ver 04

		ULONGLONG sessionID = DeleteSession->m_ullSessionID;

		OnClientLeave(sessionID);

		int TempCount = DeleteSession->m_iWSASendCount;
		InterlockedExchange(&DeleteSession->m_iWSASendCount, 0);  // ���� ī��Ʈ 0���� ����.		

		// ���� ī��Ʈ�� ������ ��� �������� �Ѵ�.
		if (TempCount > 0)
		{
<<<<<<< HEAD
<<<<<<< HEAD
			for (int i = 0; i < dfSENDPOST_MAX_WSABUF; ++i)
=======
			for (int i = 0; i < TempCount; ++i)
>>>>>>> parent of 8d940d8... 성준씨버전 ver 1
			{
				delete DeleteSession->m_cpbufSendPayload[i];
				PacketAllocCountSub();
			}
		}			

		// ���� ť�� �����Ͱ� ������ �������� �Ѵ�.
		CProtocolBuff* Payload[1024];
		int UseSize = DeleteSession->m_SendQueue.GetUseSize();
		int DequeueSize = DeleteSession->m_SendQueue.Dequeue((char*)Payload, UseSize);

		DequeueSize = DequeueSize / 8;

		if (DequeueSize != UseSize / 8)
		{
			// ���� �α� ���
			// �α� ���(�α� ���� : ����)
			cNetLibLog->LogSave(L"LanServer", CSystemLog::en_LogLevel::LEVEL_ERROR, L"InDisconnect(). DequeueSize, UseSize Not SAME! : Rear(%d), Front(%d)",
				DequeueSize, UseSize / 8);
		}

		// ���� Dequeue��ŭ ���鼭 �����Ѵ�.
		for (int i = 0; i < DequeueSize; ++i)
		{
			delete Payload[i];
			if (PacketAllocCountSub() != 0)
			{
				// ���� �α� ���
				// �α� ���(�α� ���� : ����)
				//cNetLibLog->LogSave(L"LanServer", CSystemLog::en_LogLevel::LEVEL_ERROR, L"InDisconnect(). PacketAllocCount Not 0 : %d",
				//	g_llPacketAllocCount);
			}
		}



		//int UseSize = DeleteSession->m_SendQueue.GetUseSize();

		//while (UseSize > 0)
		//{
		//	int Temp = UseSize;

		//	if (Temp > 8000)
		//		Temp = 8000;

		//	// UseSize ������ ��ŭ ��ť
		//	CProtocolBuff* Payload[1000];
		//	int DequeueSize = DeleteSession->m_SendQueue.Dequeue((char*)Payload, Temp);

		//	UseSize -= DequeueSize;

		//	DequeueSize = DequeueSize / 8;

		//	// ���� Dequeue��ŭ ���鼭 �����Ѵ�.
		//	for (int i = 0; i < DequeueSize; ++i)
		//	{
		//		delete Payload[i];
		//		PacketAllocCountSub();
		//	}
		//}

		int* RearValue = (int*)DeleteSession->m_SendQueue.GetWriteBufferPtr();
		int* FrontValue = (int*)DeleteSession->m_SendQueue.GetReadBufferPtr();

		if (*RearValue != *FrontValue)
		{
			// ���� �α� ���
			// �α� ���(�α� ���� : ����)
			cNetLibLog->LogSave(L"LanServer", CSystemLog::en_LogLevel::LEVEL_ERROR, L"InDisconnect(). Rear, Front Not SAME! : Rear(%d), Front(%d)",
				RearValue, FrontValue);
		}

		// ����
		DeleteSession->Reset_Func();

		// Ŭ���� ����
		closesocket(DeleteSession->m_Client_sock);		
=======
			for (int i = 0; i < TempCount; ++i)
				delete DeleteSession->m_cpbufSendPayload[i];			
		}	

		DeleteSession->m_lUseFlag = FALSE;
>>>>>>> parent of 4721bdb... 218-08-03 7차과제 ver 04

		DeleteSession->m_lUseFlag = FALSE;

		// �̻�� �ε��� ���ÿ� �ݳ�
		Lock_Exclusive_Stack();
		if (m_stEmptyIndexStack->Push(DeleteSession->m_lIndex) == false)
		{
			// ���� �α� ���
			// �α� ���(�α� ���� : �����)
			cNetLibLog->LogSave(L"LanServer", CSystemLog::en_LogLevel::LEVEL_DEBUG, L"InDisconnect(). Stack_Index_Full! : NowJoinUser(%d), MaxUser(%d)",
				m_ullJoinUserCount, m_iMaxJoinUser);
		}
		Unlock_Exclusive_Stack();

		// Ŭ���� ����
		closesocket(DeleteSession->m_Client_sock);

		// ���� �� ����
		InterlockedDecrement(&m_ullJoinUserCount);	

		
		return;
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
		DWORD cbTransferred;
		stSession* stNowSession;
		OVERLAPPED* overlapped;

		CLanServer* g_This = (CLanServer*)lParam;

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

			// GQCS ��� �� �Լ�ȣ��
			g_This->OnWorkerThreadBegin();

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

				// �α� ��� (�α� ���� : ����)
				cNetLibLog->LogSave(L"LanServer", CSystemLog::en_LogLevel::LEVEL_ERROR, L"IOCP_Error --> GQCS return Error : NetError(%d), OSError(%d)",
					(int)g_This->m_iMyErrorCode, g_This->m_iOSErrorCode);


				// ���� �߻� �Լ� ȣ��
				g_This->OnError((int)euError::NETWORK_LIB_ERROR__IOCP_ERROR, L"IOCP_Error");

				break;
			}

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

				// 2. ���ú� �ٽ� �ɱ�. false�� ���ϵǸ� ����� �����̴� �ٽ� ���� �ö󰣴�.
				if (g_This->RecvPost(stNowSession) == false)
					continue;
			}

			// -----------------
			// Send ����
			// -----------------
			// WSAsend()�� �Ϸ�� ���, ���� �����Ͱ� 0�� �ƴϸ� ����ó��
			else if (&stNowSession->m_overSendOverlapped == overlapped && cbTransferred > 0)
			{
				// 1. ���� �Ϸ�ƴٰ� �������� �˷���
				g_This->OnSend(stNowSession->m_ullSessionID, cbTransferred);			

				// 2. ���´� ����ȭ���� ����
				int TempCount = stNowSession->m_iWSASendCount;
				InterlockedExchange(&stNowSession->m_iWSASendCount, 0);  // ���� ī��Ʈ 0���� ����.

<<<<<<< HEAD
<<<<<<< HEAD
				for (int i = 0; i < dfSENDPOST_MAX_WSABUF; ++i)
=======
				for (int i = 0; i < TempCount; ++i)
>>>>>>> parent of 8d940d8... 성준씨버전 ver 1
				{
					delete stNowSession->m_cpbufSendPayload[i];
					PacketAllocCountSub();
				}
=======
				for (int i = 0; i < TempCount; ++i)
					delete stNowSession->m_cpbufSendPayload[i];		
>>>>>>> parent of 4721bdb... 218-08-03 7차과제 ver 04

				// 4. ���� ���� ���·� ����
				InterlockedExchange(&stNowSession->m_lSendFlag, FALSE);

				// 5. �ٽ� ���� �õ�. false�� ���ϵǸ� ����� �����̴� �ٽ� ���� �ö󰣴�.
				if (g_This->SendPost(stNowSession) == false)
					continue;				
			}

			// -----------------
			// I/Oī��Ʈ ���� �� ���� ó��
			// -----------------
			// I/Oī��Ʈ ���� ��, 0�̶������ ����
			if (InterlockedDecrement(&stNowSession->m_lIOCount) == 0)
				g_This->InDisconnect(stNowSession);

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

		// ������ ������ �� ���� 1�� �����ϴ� ������ Ű.
		ULONGLONG ullUniqueSessionID = 0;

		while (1)
		{
			ZeroMemory(&clientaddr, sizeof(clientaddr));
			addrlen = sizeof(clientaddr);
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

				// �α� ��� (�α� ���� : ����)
				cNetLibLog->LogSave(L"LanServer", CSystemLog::en_LogLevel::LEVEL_ERROR, L"accpet(). Abnormal_exit : NetError(%d), OSError(%d)",
					(int)g_This->m_iMyErrorCode, g_This->m_iOSErrorCode);

				// ���� �߻� �Լ� ȣ��
				g_This->OnError((int)euError::NETWORK_LIB_ERROR__A_THREAD_ABNORMAL_EXIT, L"accpet(). Abnormal_exit");

				break;
			}	

			// ------------------
			// �ִ� ������ �� �̻� ���� �Ұ�
			// ------------------
			if (g_This->m_iMaxJoinUser <= g_This->m_ullJoinUserCount)
			{
				closesocket(client_sock);
<<<<<<< HEAD

				// ���� �α� ��� (�α� ���� : �����)
				cNetLibLog->LogSave(L"LanServer", CSystemLog::en_LogLevel::LEVEL_DEBUG, L"accpet(). User_Max... : NowJoinUser(%d), MaxUser(%d)",
					g_This->m_ullJoinUserCount, g_This->m_iMaxJoinUser);

=======
>>>>>>> parent of 4721bdb... 218-08-03 7차과제 ver 04
				continue;
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
			// false�� ���Ӱź�, 
			// true�� ���� ��� ����. true���� �Ұ� ������ OnConnectionRequest�Լ� ���ڷ� ������ ������.
			if (g_This->OnConnectionRequest(tcTempIP, port) == false)
				continue;




			// ------------------
			// ���� ����ü ���� �� ����
			// ------------------
			// 1) �̻�� �ε��� �˾ƿ���
			g_This->Lock_Exclusive_Stack(); // �̻�� �ε��� ���� �� -----------
			ULONGLONG iIndex = g_This->m_stEmptyIndexStack->Pop();
			if (iIndex == 10000000)
			{
				g_This->Unlock_Exclusive_Stack(); // �̻�� �ε��� ���� �� ���� -----------

				// ���� �α� ���
				// �α� ���(�α� ���� : �����)
				cNetLibLog->LogSave(L"LanServer", CSystemLog::en_LogLevel::LEVEL_DEBUG, L"accpet(). Stack_Index_Empty... : NowJoinUser(%d), MaxUser(%d)",
					g_This->m_ullJoinUserCount, g_This->m_iMaxJoinUser);

				break;
			}
			g_This->Unlock_Exclusive_Stack(); // �̻�� �ε��� ���� �� ���� -----------

			// 2) �ش� ���� �迭, ��������� ����
			g_This->m_stSessionArray[iIndex].m_lUseFlag = TRUE;

			// 3) �ʱ�ȭ�ϱ�
			// -- ����
			g_This->m_stSessionArray[iIndex].m_Client_sock = client_sock;

			// -- ����Ű(�ͽ� Ű)�� �ε���
			ULONGLONG MixKey = InterlockedIncrement(&ullUniqueSessionID) | (iIndex << 48);
			g_This->m_stSessionArray[iIndex].m_ullSessionID = MixKey;
			g_This->m_stSessionArray[iIndex].m_lIndex = iIndex;

			// -- IP�� Port
			StringCchCopy(g_This->m_stSessionArray[iIndex].m_IP, _MyCountof(g_This->m_stSessionArray[iIndex].m_IP), tcTempIP);
			g_This->m_stSessionArray[iIndex].m_prot = port;
			
			// -- SendFlag, I/Oī��Ʈ, ť �ʱ�ȭ �ʱ�ȭ
			g_This->m_stSessionArray[iIndex].Reset_Func();			
							



			// ------------------
			// IOCP ����
			// ------------------
			// ���ϰ� IOCP ����
			if (CreateIoCompletionPort((HANDLE)client_sock, g_This->m_hIOCPHandle, (ULONG_PTR)&g_This->m_stSessionArray[iIndex], 0) == NULL)
			{
				// ������ ����, �� ���� ����
				g_This->m_iOSErrorCode = WSAGetLastError();
				g_This->m_iMyErrorCode = euError::NETWORK_LIB_ERROR__A_THREAD_ABNORMAL_EXIT;

				// �α� ��� (�α� ���� : ����)
				cNetLibLog->LogSave(L"LanServer", CSystemLog::en_LogLevel::LEVEL_ERROR, L"accpet(). Abonormal_exit : NetError(%d), OSError(%d)",
					(int)g_This->m_iMyErrorCode, g_This->m_iOSErrorCode);

				// ���� �߻� �Լ� ȣ��
				g_This->OnError((int)euError::NETWORK_LIB_ERROR__A_THREAD_ABNORMAL_EXIT, L"accpet(). Abonormal_exit");

				break;
			}
					
			// ������ �� ����. disconnect������ ���Ǵ� �����̱� ������ ���Ͷ� ���
			InterlockedIncrement(&g_This->m_ullJoinUserCount);



			// ------------------
			// ��� ���������� �Ϸ�Ǿ����� ���� �� ó�� �Լ� ȣ��.
			// ------------------
			g_This->m_stSessionArray[iIndex].m_lIOCount++; // I/Oī��Ʈ ++. �������� 1���� ����. ���� recv,send �� ��͵� �Ȱɾ��� ������ �׳� ++�ص� ����!
			g_This->OnClientJoin(MixKey);

			

			// ------------------
			// �񵿱� ����� ����
			// ------------------
			// ��ȯ���� false���, �� �ȿ��� ����� ������. �ٵ� �ȹ���
			g_This->RecvPost(&g_This->m_stSessionArray[iIndex]);

			// I/Oī��Ʈ --. 0�̶�� ����ó��
			if (InterlockedDecrement(&g_This->m_stSessionArray[iIndex].m_lIOCount) == 0)
				g_This->InDisconnect(&g_This->m_stSessionArray[iIndex]);
			
		}

		return 0;
	}

	// �̻�� ���� ���� ���ÿ� Exclusive �� �ɱ�, �� Ǯ��
	void CLanServer::LockStack_Exclusive_Func()
	{
		AcquireSRWLockExclusive(&m_srwSession_stack_srwl);
	}

	void CLanServer::UnlockStack_Exclusive_Func()
	{
		ReleaseSRWLockExclusive(&m_srwSession_stack_srwl);
	}

	// �̻�� ���� ���� ���ÿ� Shared �� �ɱ�, �� Ǯ��
	void CLanServer::LockStack_Shared_Func()
	{
		AcquireSRWLockShared(&m_srwSession_stack_srwl);
	}

	void CLanServer::UnlockStack_Shared_Func()
	{
		ReleaseSRWLockShared(&m_srwSession_stack_srwl);
	}



	// ���յ� Ű�� �Է¹�����, Index �����ϴ� �Լ�
	WORD CLanServer::GetSessionIndex(ULONGLONG MixKey)
	{
		return MixKey >> 48;		
	}

	// ���յ� Ű�� �Է¹�����, ��¥ ����Ű�� �����ϴ� �Լ�.
	ULONGLONG CLanServer::GetRealSessionKey(ULONGLONG MixKey)
	{
		return MixKey & 0x0000ffffffffffff;
	}

	// CProtocolBuff�� ��� �ִ� �Լ�
	void CLanServer::SetProtocolBuff_HeaderSet(CProtocolBuff* Packet)
	{
		WORD wHeader = Packet->GetUseSize() - dfNETWORK_PACKET_HEADER_SIZE;
		memcpy_s(&Packet->GetBufferPtr()[0], dfNETWORK_PACKET_HEADER_SIZE, &wHeader, dfNETWORK_PACKET_HEADER_SIZE);
	}



	// ���� �������� ���½�Ų��.
	void CLanServer::Reset()
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
	void CLanServer::RecvProc(stSession* NowSession)
	{
		// -----------------
		// Recv ť ���� ó��
		// -----------------

		while (1)
		{
			// 1. RecvBuff�� �ּ����� ����� �ִ��� üũ. (���� = ��� ������� ���ų� �ʰ�. ��, �ϴ� �����ŭ�� ũ�Ⱑ �ִ��� üũ)	
			WORD Header_PaylaodSize;

			// RecvBuff�� ��� ���� ���� ũ�Ⱑ ��� ũ�⺸�� �۴ٸ�, �Ϸ� ��Ŷ�� ���ٴ� ���̴� while�� ����.
			int UseSize = NowSession->m_RecvQueue.GetUseSize();
			if (UseSize < dfNETWORK_PACKET_HEADER_SIZE)
			{
				break;
			}

			// 2. ����� Peek���� Ȯ���Ѵ�.  Peek �ȿ�����, ��� �ؼ����� len��ŭ �д´�. 
			// ���۰� ��������� ���� ����.
			if (NowSession->m_RecvQueue.Peek((char*)&Header_PaylaodSize, dfNETWORK_PACKET_HEADER_SIZE) == -1)
			{
				// �ϴ� ������ϴ� �˴ٿ� ȣ��
				shutdown(NowSession->m_Client_sock, SD_BOTH);

				// �� ���� ����. ������ ������ ����.
				m_iMyErrorCode = euError::NETWORK_LIB_ERROR__EMPTY_RECV_BUFF;

				// ���� ��Ʈ�� �����
				TCHAR tcErrorString[300];
				StringCchPrintf(tcErrorString, 300, _T("RecvRingBuff_Empry.UserID : %d, [%s:%d]"), 
					NowSession->m_ullSessionID, NowSession->m_IP, NowSession->m_prot);

				// �α� ��� (�α� ���� : ����)
				cNetLibLog->LogSave(L"LanServer", CSystemLog::en_LogLevel::LEVEL_ERROR, L"%s, NetError(%d)",
					tcErrorString, (int)m_iMyErrorCode);

				// ���� �Լ� ȣ��
				OnError((int)euError::NETWORK_LIB_ERROR__EMPTY_RECV_BUFF, tcErrorString);
			
				// ������ ���� �����̴� ���� �ƹ��͵� ���ϰ� ����
				return;
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
			PayloadBuff.MoveReadPos(dfNETWORK_PACKET_HEADER_SIZE);

			int DequeueSize = NowSession->m_RecvQueue.Dequeue(&PayloadBuff.GetBufferPtr()[dfNETWORK_PACKET_HEADER_SIZE], Header_PaylaodSize);
			// ���۰� ��������� ���� ����
			if (DequeueSize == -1)
			{
				// �ϴ� ������ϴ� �˴ٿ� ȣ��
				shutdown(NowSession->m_Client_sock, SD_BOTH);

				// �� ���� ����. ������ ������ ����.
				m_iMyErrorCode = euError::NETWORK_LIB_ERROR__EMPTY_RECV_BUFF;

				// ���� ��Ʈ�� �����
				TCHAR tcErrorString[300];
				StringCchPrintf(tcErrorString, 300, _T("RecvRingBuff_Empry.UserID : %d, [%s:%d]"),
					NowSession->m_ullSessionID, NowSession->m_IP, NowSession->m_prot);

				// ���� �Լ� ȣ��
				OnError((int)euError::NETWORK_LIB_ERROR__EMPTY_RECV_BUFF, tcErrorString);

				// ������ ���� �����̴� ���� �ƹ��͵� ���ϰ� ����
				return;
			}
			PayloadBuff.MoveWritePos(DequeueSize);

			// 9. ����� ����ִ� Ÿ�Կ� ���� �б�ó��.
			OnRecv(NowSession->m_ullSessionID, &PayloadBuff);

		}

		return;
	}

	// RecvPost �Լ�. �񵿱� ����� ����
	//
	// return true : ���������� WSARecv() �Ϸ� or ��¶�� ����� ������ �ƴ�
	// return false : I/Oī��Ʈ�� 0�̵Ǿ ����� ������
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

		// 4. ���� ó��
		if (WSARecv(NowSession->m_Client_sock, wsabuf, wsabufCount, &recvBytes, &flags, &NowSession->m_overRecvOverlapped, NULL) == SOCKET_ERROR)
		{
			int Error = WSAGetLastError();

			// �񵿱� ������� ���۵Ȱ� �ƴ϶��
			if (Error != WSA_IO_PENDING)
			{
				// I/Oī��Ʈ 1����.I/O ī��Ʈ�� 0�̶�� ���� ����.
				if (InterlockedDecrement(&NowSession->m_lIOCount) == 0)
				{
					InDisconnect(NowSession);
					return false;
				}

				// ������ ���� �����̶��, I/Oī��Ʈ ������ ���� �ƴ϶� ������Ѵ�.
				if (Error == WSAENOBUFS)
				{			
					// �ϴ� ������ϴ� �˴ٿ� ȣ��
					shutdown(NowSession->m_Client_sock, SD_BOTH);

					// �� ����, �����쿡�� ����
					m_iOSErrorCode = Error;
					m_iMyErrorCode = euError::NETWORK_LIB_ERROR__WSAENOBUFS;

					// ���� ��Ʈ�� �����
					TCHAR tcErrorString[300];
					StringCchPrintf(tcErrorString, 300, _T("WSANOBUFS. UserID : %d, [%s:%d]"),
						NowSession->m_ullSessionID, NowSession->m_IP, NowSession->m_prot);

					// �α� ��� (�α� ���� : ����)
					cNetLibLog->LogSave(L"LanServer", CSystemLog::en_LogLevel::LEVEL_ERROR, L"WSARecv --> %s : NetError(%d), OSError(%d)",
						tcErrorString, (int)m_iMyErrorCode, m_iOSErrorCode);

					// ���� �Լ� ȣ��
					OnError((int)euError::NETWORK_LIB_ERROR__WSAENOBUFS, tcErrorString);
				}
			}
		}

		return true;
	}






	// ------------
	// Lib ���ο����� ����ϴ� ���� ���� �Լ���
	// ------------
	// ���� ������ ������ WSASend() �ϱ�
	//
	// return true : ���������� WSASend() �Ϸ� or ��¶�� ����� ������ �ƴ�
	// return false : I/Oī��Ʈ�� 0�̵Ǿ ����� ������
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
				return true;

			// 2. SendBuff�� �����Ͱ� �ִ��� Ȯ��
			// ���⼭ ���� UseSize�� ���� ������ �̴�. 
			int UseSize = NowSession->m_SendQueue.GetUseSize();
			if (UseSize == 0)
			{
				// WSASend �Ȱɾ��� ������, ���� ���� ���·� �ٽ� ����.
				NowSession->m_lSendFlag = 0;

				// 3. ��¥�� ����� ������ �ٽ��ѹ� üũ. �� Ǯ�� �Դµ�, ���ؽ�Ʈ ����Ī �Ͼ�� �ٸ� �����尡 �ǵ���� ���ɼ�
				// ������ ������ ���� �ö󰡼� �ѹ� �� �õ�
				if (NowSession->m_SendQueue.GetUseSize() > 0)
					continue;

				break;
			}

			// ------------------
			// Send �غ��ϱ�
			// ------------------
			// 1. WSABUF ����.
			WSABUF wsabuf[dfSENDPOST_MAX_WSABUF];		

			if (UseSize > dfSENDPOST_MAX_WSABUF * 8)
				UseSize = dfSENDPOST_MAX_WSABUF * 8;

<<<<<<< HEAD
<<<<<<< HEAD
			// 2. �� ���� 100���� ������(�� 800����Ʈ)�� �������� �õ�
=======
			// 2. �� ���� 100���� ������(�� 800����Ʈ)�� �������� �õ�			
>>>>>>> parent of 8d940d8... 성준씨버전 ver 1
			int wsabufByte = (NowSession->m_SendQueue.Dequeue((char*)NowSession->m_cpbufSendPayload, UseSize));
			if (wsabufByte == -1)
=======
			// 3. �� ���� 100���� ������(�� 800����Ʈ)�� �������� �õ�			
			int wsabufCount = (NowSession->m_SendQueue.Dequeue((char*)NowSession->m_cpbufSendPayload, UseSize));
			if (wsabufCount == -1)
>>>>>>> parent of 4721bdb... 218-08-03 7차과제 ver 04
			{
				// ť�� �� �������. ������ �ִٰ� �Դµ� ���⼭ ���°��� ��¥ �����ȵǴ� ����!
				// �� ��������
				m_iMyErrorCode = euError::NETWORK_LIB_ERROR__QUEUE_DEQUEUE_EMPTY;

				// ���� ��Ʈ�� �����
				TCHAR tcErrorString[300];
				StringCchPrintf(tcErrorString, 300, _T("QUEUE_PEEK_EMPTY. UserID : %d, [%s:%d]"),
					NowSession->m_ullSessionID, NowSession->m_IP, NowSession->m_prot);

				// �α� ��� (�α� ���� : ����)
				cNetLibLog->LogSave(L"LanServer", CSystemLog::en_LogLevel::LEVEL_ERROR, L"WSASend --> %s : NetError(%d)",
					tcErrorString, (int)m_iMyErrorCode);

				// ���´�.
				shutdown(NowSession->m_Client_sock, SD_BOTH);

				// ���� Flag 0���� ���� (���� ���ɻ���)
				InterlockedExchange(&NowSession->m_lSendFlag, FALSE);

				// ���� �Լ� ȣ��
				OnError((int)euError::NETWORK_LIB_ERROR__QUEUE_DEQUEUE_EMPTY, tcErrorString);

				return false;
			}
<<<<<<< HEAD
<<<<<<< HEAD
=======
			NowSession->m_iWSASendCount = wsabufCount / 8;
>>>>>>> parent of 4721bdb... 218-08-03 7차과제 ver 04

			if (0 != wsabufByte % 8)
			{
				int a = 0;
			}

			int iMax = wsabufByte / 8;
			InterlockedExchange(&NowSession->m_iWSASendCount, iMax);
=======
			InterlockedExchange(&NowSession->m_iWSASendCount, wsabufByte / 8);
>>>>>>> parent of 8d940d8... 성준씨버전 ver 1

			// 3. ������ ���� ����Ʈ ��(����Ʈ �ƴ�! ����)��ŭ ���鼭 WSABUF����ü�� �Ҵ�
			for (int i = 0; i < NowSession->m_iWSASendCount; i++)
			{				
				wsabuf[i].buf = NowSession->m_cpbufSendPayload[i]->GetBufferPtr();
				wsabuf[i].len = NowSession->m_cpbufSendPayload[i]->GetUseSize();
			}

			// 4. Overlapped ����ü �ʱ�ȭ
			ZeroMemory(&NowSession->m_overSendOverlapped, sizeof(NowSession->m_overSendOverlapped));

			// 5. WSASend()
			DWORD SendBytes = 0, flags = 0;
			InterlockedIncrement(&NowSession->m_lIOCount);

			// 6. ���� ó��
			if (WSASend(NowSession->m_Client_sock, wsabuf, NowSession->m_iWSASendCount, &SendBytes, flags, &NowSession->m_overSendOverlapped, NULL) == SOCKET_ERROR)
			{
				int Error = WSAGetLastError();

				// �񵿱� ������� ���۵Ȱ� �ƴ϶��
				if (Error != WSA_IO_PENDING)
				{	
					// ���� Flag 0���� ���� (���� ���ɻ���)
					InterlockedExchange(&NowSession->m_lSendFlag, FALSE);

					// IOcount �ϳ� ����. I/O ī��Ʈ�� 0�̶�� ���� ����.
					if (InterlockedDecrement(&NowSession->m_lIOCount) == 0)
					{
						InDisconnect(NowSession);
						return false;
					}					

					// ������ ���� �����̶��
					if (Error == WSAENOBUFS)
					{
						// �� ����, �����쿡�� ����
						m_iOSErrorCode = Error;
						m_iMyErrorCode = euError::NETWORK_LIB_ERROR__WSAENOBUFS;

						// ���� ��Ʈ�� �����
						TCHAR tcErrorString[300];
						StringCchPrintf(tcErrorString, 300, _T("WSANOBUFS. UserID : %d, [%s:%d]"),
							NowSession->m_ullSessionID, NowSession->m_IP, NowSession->m_prot);

						// �α� ��� (�α� ���� : ����)
						cNetLibLog->LogSave(L"LanServer", CSystemLog::en_LogLevel::LEVEL_ERROR, L"WSASend --> %s : NetError(%d), OSError(%d)",
							tcErrorString, (int)m_iMyErrorCode, m_iOSErrorCode);

						// ���´�.
						shutdown(NowSession->m_Client_sock, SD_BOTH);

						// ���� �Լ� ȣ��
						OnError((int)euError::NETWORK_LIB_ERROR__WSAENOBUFS, tcErrorString);
					}
				}
			}
			break;
		}

		return true;
	}

}


