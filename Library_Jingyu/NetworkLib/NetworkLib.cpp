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



namespace Library_Jingyu
{
	// �α� ���� �������� �ϳ� �ޱ�.
	CSystemLog* cNetLibLog = CSystemLog::GetInstance();	

	// ���� ���� ���� �ϳ� �ޱ�
	CCrashDump* cNetDump = CCrashDump::GetInstance();


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
		NETWORK_LIB_ERROR__W_THREAD_CREATE_FAIL,		// ��Ŀ������ ����ٰ� ���� 
		NETWORK_LIB_ERROR__A_THREAD_CREATE_FAIL,		// ����Ʈ ������ ����ٰ� ���� 
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
		NETWORK_LIB_ERROR__W_THREAD_ABONORMAL_EXIT,		// ��Ŀ ������ ������ ����. 
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
		LONG	m_lReyryCount;

		// �� ����ü ����ȭ ��ü.
		CRITICAL_SECTION m_csStruct_Lock;
		
		CRingBuff m_RecvQueue;
		CRingBuff m_SendQueue;

		OVERLAPPED m_overRecvOverlapped;
		OVERLAPPED m_overSendOverlapped;

		// ������
		stSession()
		{
			InitializeCriticalSection(&m_csStruct_Lock);
			m_lIOCount = 0;
			m_lSendFlag = 0;
			m_lReyryCount = 0;
		}

		// �Ҹ���
		~stSession()
		{
			DeleteCriticalSection(&m_csStruct_Lock);
		}

		// Critical_Section �� �ɱ�
		void LockSession_CS_Func()
		{
			EnterCriticalSection(&m_csStruct_Lock);
		}

		// Critical_Section �� Ǯ��
		void UnlockSession_CS_Func()
		{
			LeaveCriticalSection(&m_csStruct_Lock);
		}

#define	Struct_Lock()	LockSession_CS_Func()
#define Struct_Unlock()	UnlockSession_CS_Func()
	
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

		// �ִ� ���� ���� ���� �� ����
		m_iMaxJoinUser = MaxConnect;

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

		// ���ϰ��� [WAIT_OBJECT_0 ~ WAIT_OBJECT_0 + m_iW_ThreadCount - 1] �� �ƴ϶��, ���� ������ �߻��� ��. ���� ��� �� �����Ѵ�.
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
		Lock_Exclusive_Map();

		map<ULONGLONG, stSession*>::iterator itor = map_Session.begin();
		for (; itor != map_Session.end(); ++itor)
			shutdown(itor->second->m_Client_sock, SD_BOTH);		

		Unlock_Exclusive_Map();

		// ��� ������ ����Ǿ����� üũ
		while (1)
		{
			if (map_Session.size() == 0)
				break;

			Sleep(1);
		}

		// 3. ��Ŀ ������ ����
		for (int i = 0; i<m_iW_ThreadCount; ++i)
			PostQueuedCompletionStatus(m_hIOCPHandle, 0, 0, 0);

		// ��Ŀ������ ���� ���
		retval = WaitForMultipleObjects(m_iW_ThreadCount, m_hWorkerHandle, TRUE, INFINITE);

		// ���ϰ��� [WAIT_OBJECT_0 ~ WAIT_OBJECT_0 + m_iW_ThreadCount - 1] �� �ƴ϶��, ���� ������ �߻��� ��. ���� ��� �� �����Ѵ�.
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
			m_iMyErrorCode = euError::NETWORK_LIB_ERROR__W_THREAD_ABONORMAL_EXIT;

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

		// 5. ���� ������ �ƴ� ���·� ����
		m_bServerLife = false;

		// 6. ���� ���� �ʱ�ȭ
		Reset();

		// 7. ���� ���� �α� ���		
		cNetLibLog->LogSave(L"LanServer", CSystemLog::en_LogLevel::LEVEL_SYSTEM, L"ServerClose...");
	}

	// �ܺο���, � �����͸� ������ ������ ȣ���ϴ� �Լ�.
	// SendPacket�� �׳� �ƹ����� �ϸ� �ȴ�.
	// �ش� ������ SendQ�� �־�״ٰ� ���� �Ǹ� ������.
	//
	// return true : SendQ�� ���������� ������ ����.
	// return true : SendQ�� ������ �ֱ� ����.
	bool CLanServer::SendPacket(ULONGLONG ClinetID, CProtocolBuff* payloadBuff)
	{
		// 1. ClinetID�� ���Ǳ���ü �˾ƿ���
		Lock_Shared_Map(); // �� �� ----------- 
		stSession* NowSession = FineSessionPtr(ClinetID);
		if (NowSession == nullptr)
		{
			// ������ ȣ���� �Լ���, ���� Ȯ���� �����ϱ� ������ OnError�Լ� ȣ�� ����.
			m_iMyErrorCode = euError::NETWORK_LIB_ERROR__NOT_FIND_CLINET;

			// �α� ��� (�α� ���� : ����)
			// ---> �ϴ� ���� �����׽�Ʈ �߿��� �ʹ� ���� �߻��ϴ� ����.
			//cNetLibLog->LogSave(L"LanServer", CSystemLog::en_LogLevel::LEVEL_ERROR, L"SendPacket() --> Not Fine Clinet :  NetError(%d)",(int)m_iMyErrorCode);

			Unlock_Shared_Map(); // �� ��� ----------- 

			return false;
		}

		NowSession->Struct_Lock(); // ���� �� ----------- 
		Unlock_Shared_Map(); // �� ��� ----------- 		
	
		// 2. ��� ����� (���̷ε� ����� ����)
		WORD Header = payloadBuff->GetUseSize();
	
		// 3. �ֱ� (���, ���̷ε带 2�� ��ť�ϴ°�..��� �ȵɱ�)

		// ��� ��ť
		int EnqueueCheck = NowSession->m_SendQueue.Enqueue((char*)&Header, dfNETWORK_PACKET_HEADER_SIZE);
		if (EnqueueCheck == -1)
		{
			// �ش� ������ ������ ���´�.
			shutdown(NowSession->m_Client_sock, SD_BOTH);

			// ������ ȣ���� �Լ���, ���� Ȯ���� �����ϱ� ������ OnError�Լ� ȣ�� ����.
			m_iMyErrorCode = euError::NETWORK_LIB_ERROR__SEND_QUEUE_SIZE_FULL;

			// �α� ��� (�α� ���� : ����)
			cNetLibLog->LogSave(L"LanServer", CSystemLog::en_LogLevel::LEVEL_ERROR, L"SendPacket() --> Send Queue Full Clinet : [%s : %d] NetError(%d)",
				NowSession->m_IP, NowSession->m_prot, (int)m_iMyErrorCode);
			
			NowSession->Struct_Unlock(); // ���� ��� ----------- 
			return false;
		}

		// ���̷ε� ��ť
		EnqueueCheck = NowSession->m_SendQueue.Enqueue(payloadBuff->GetBufferPtr(), Header);
		if (EnqueueCheck == -1)
		{
			 // �ش� ������ ������ ���´�.
			shutdown(NowSession->m_Client_sock, SD_BOTH);

			// ������ ȣ���� �Լ���, ���� Ȯ���� �����ϱ� ������ OnError�Լ� ȣ�� ����.
			m_iMyErrorCode = euError::NETWORK_LIB_ERROR__SEND_QUEUE_SIZE_FULL;	

			// �α� ��� (�α� ���� : ����)
			cNetLibLog->LogSave(L"LanServer", CSystemLog::en_LogLevel::LEVEL_ERROR, L"SendPacket() --> Send Queue Full Clinet : [%s : %d] NetError(%d)",
				NowSession->m_IP, NowSession->m_prot, (int)m_iMyErrorCode);

			NowSession->Struct_Unlock(); // ���� ��� ----------- 
			return false;
		}

		// 4. SendPost�õ�
		bool Check = SendPost(NowSession);
		if(Check == true)
			NowSession->Struct_Unlock(); // ���� ��� ----------- 
		

		return Check;
	}

	// ������ ������ ���� �� ȣ���ϴ� �Լ�. �ܺ� ���� ���.
	// ���̺귯������ ������!��� ��û�ϴ� �� ��
	//
	// return true : �ش� �������� �˴ٿ� �� ����.
	// return false : ���������� ���� ������ ���������Ϸ��� ��.
	bool CLanServer::Disconnect(ULONGLONG ClinetID)
	{
		// ���� ã�´�.
		Lock_Shared_Map(); // �� �� ---------------------
		stSession* NowSession = FineSessionPtr(ClinetID);		

		// ���� ��ã������ �����ڵ� ����� false ����
		if (NowSession == nullptr)
		{
			Unlock_Shared_Map(); // �� �� ---------------------

			// �� ���� ����. (������ ������ ����)
			m_iMyErrorCode = euError::NETWORK_LIB_ERROR__NOT_FIND_CLINET;

			// �α� ��� (�α� ���� : ����)
			cNetLibLog->LogSave(L"LanServer", CSystemLog::en_LogLevel::LEVEL_ERROR, L"SendPacket() --> Not Fine Clinet : NetError(%d)", (int)m_iMyErrorCode);

			return false;
		}

		Unlock_Shared_Map(); // �� �� ---------------------

		// ������ϴ� ������ �˴ٿ� ������.
		// ���� �ڿ������� I/Oī��Ʈ�� ���ҵǾ ��Ŀ��Ʈ�ȴ�.
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
		// �α� �ʱ�ȭ
		cNetLibLog->SetDirectory(L"LanServer");
		cNetLibLog->SetLogLeve(CSystemLog::en_LogLevel::LEVEL_ERROR);

		// ���� �������� false�� ���� 
		m_bServerLife = false;

		// SRWLock �ʱ�ȭ
		InitializeSRWLock(&m_srwSession_map_srwl);
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
	{
		ULONGLONG sessionID = DeleteSession->m_ullSessionID;
	
		// map���� ���ܽ�Ű��
		Lock_Exclusive_Map();  // �� �� -----------------

		// ����, ���� ������� �̹� �����Ȱɷ� ġ��, �����Ѵ�.
		if (map_Session.erase(sessionID) == 0)
		{
			Unlock_Exclusive_Map(); // �� ��� -----------------
			return;
		}

		Unlock_Exclusive_Map(); // �� ��� -----------------

		DeleteSession->Struct_Lock();  // ���� �� -----------------
		DeleteSession->Struct_Unlock(); // ���� ��� -----------------
		
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

			/*
			 overlapped�� nullptr�� �ƴϸ鼭 ���ϰ��� false��, I/O�� ���� ������ �� ������ GetLastError �ϰ� ���� Ƚ�� I/O ���û. (�������� ���� ���� ������ ���� ����?)
			 �ٵ�.. �̰� �ǹ̾��°Ű���. �� üũ ���ϴ°� �´°Ű��⵵ �ϰ�.
			 ���� -> ���� -> �ٽ� ������ �ݺ��ϴ� ���ϱ� ������ ���� Ŭ�� ���� ERROR_NETNAME_DELETED(�ش� ��Ʈ��ũ�� �� �̻� ����Ҽ� ����)�̰� ��� �ߴµ�,
			 �ϴ� �� ������ �����Ѵٰ� �ĵ�, �̹� ���� ���� �������� ������ ��ȸ�� �ִ°� �³� �𸣰ڴ�. ���������� �������� �׳� ����ɷ� ó��?
			 Ŭ�󿡰� �˷��ִ� �뵵��(����ǥ�ÿ�)���� ���ܵ־��ұ�? �����...
			*/
			//else if(retval == false && GetLastError() != ERROR_NETNAME_DELETED)
			//{
			//	// ������ ����, �� ���� ����
			//	g_This->m_iOSErrorCode = GetLastError();
			//	g_This->m_iMyErrorCode = euError::NETWORK_LIB_ERROR__IOCP_IO_FAIL;

			//	// ���п� ���� ������ �õ�
			//	// m_lReyryCount�� 5�� �ƴ� ���� �õ��Ѵ�.
			//	if (NowSession->m_lReyryCount != 5)
			//	{
			//		InterlockedIncrement(&NowSession->m_lReyryCount);

			//		// Recv�� �������� ���, ���ú긦 �ٽ� �ɾ��.
			//		if (&NowSession->m_overRecvOverlapped == overlapped)
			//			g_This->RecvPost(NowSession);

			//		// Send�� �������� ���, ���带 �ٽ� �ɾ��.
			//		else if (&NowSession->m_overSendOverlapped == overlapped)
			//			g_This->SendPost(NowSession);
			//	}

			//  // �α� ��� (�α� ���� : ����)
			//  cNetLibLog->LogSave(L"LanServer", CSystemLog::en_LogLevel::LEVEL_ERROR, L"IOCP_IO_Fail --> GQCS return Error : [%s : %d] NetError(%d), OSError(%d)",
			//	NowSession->m_IP, NowSession->m_prot, (int)g_This->m_iMyErrorCode, g_This->m_iOSErrorCode);

			//	// ���� �߻� �Լ� ȣ��
			//	g_This->OnError((int)euError::NETWORK_LIB_ERROR__IOCP_IO_FAIL, L"IOCP_IO_Fail");				
			//}

			// -----------------
			// Recv ����
			// -----------------
			// WSArecv()�� �Ϸ�� ���, ���� �����Ͱ� 0�� �ƴϸ� ���� ó��
			if (&stNowSession->m_overRecvOverlapped == overlapped && cbTransferred > 0)
			{
				// m_lReyryCount���� 0���� ����
				//InterlockedExchange(&NowSession->m_lReyryCount, 0);

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

				// m_lReyryCount���� 0���� ����
				//InterlockedExchange(&NowSession->m_lReyryCount, 0);

				// 1. ���� �Ϸ�ƴٰ� �������� �˷���
				g_This->OnSend(stNowSession->m_ullSessionID, cbTransferred);

				// 2. front �̵�	
				stNowSession->m_SendQueue.RemoveData(cbTransferred);

				// 3. ���� ���� ���·� ����
				//stNowSession->m_lSendFlag = 0;
				InterlockedExchange(&stNowSession->m_lSendFlag, FALSE);

				// 3. �ٽ� ���� �õ�. false�� ���ϵǸ� ����� �����̴� �ٽ� ���� �ö󰣴�.
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
				g_This->m_iMyErrorCode = euError::NETWORK_LIB_ERROR__A_THREAD_ABONORMAL_EXIT;

				// �α� ��� (�α� ���� : ����)
				cNetLibLog->LogSave(L"LanServer", CSystemLog::en_LogLevel::LEVEL_ERROR, L"accpet(). Abonormal_exit : NetError(%d), OSError(%d)",
					(int)g_This->m_iMyErrorCode, g_This->m_iOSErrorCode);

				// ���� �߻� �Լ� ȣ��
				g_This->OnError((int)euError::NETWORK_LIB_ERROR__A_THREAD_ABONORMAL_EXIT, L"accpet(). Abonormal_exit");

				break;
			}	

			// ------------------
			// �ִ� ������ �� �̻� ���� �Ұ�
			// ------------------
			if (g_This->m_iMaxJoinUser <= g_This->m_ullJoinUserCount)
			{
				closesocket(client_sock);
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
			stSession* NewSession = new stSession;
			StringCchCopy(NewSession->m_IP, _MyCountof(NewSession->m_IP), tcTempIP);
			NewSession->m_prot = port;
			NewSession->m_Client_sock = client_sock;
			//NewSession->m_ullSessionID = InterlockedIncrement(&ullUniqueSessionID);
			NewSession->m_ullSessionID = ++ullUniqueSessionID;
						

			// ------------------
			// map ��� ��, ������ �� �߰�
			// ------------------
			// ���õ� ����ü�� map�� ���
			g_This->Lock_Exclusive_Map();
			g_This->map_Session.insert(pair<ULONGLONG, stSession*>(NewSession->m_ullSessionID, NewSession));
			g_This->Unlock_Exclusive_Map();
						

			// ------------------
			// IOCP ����
			// ------------------
			// ���ϰ� IOCP ����
			if (CreateIoCompletionPort((HANDLE)client_sock, g_This->m_hIOCPHandle, (ULONG_PTR)NewSession, 0) == NULL)
			{
				// ������ ����, �� ���� ����
				g_This->m_iOSErrorCode = WSAGetLastError();
				g_This->m_iMyErrorCode = euError::NETWORK_LIB_ERROR__A_THREAD_ABONORMAL_EXIT;

				// �α� ��� (�α� ���� : ����)
				cNetLibLog->LogSave(L"LanServer", CSystemLog::en_LogLevel::LEVEL_ERROR, L"accpet(). Abonormal_exit : NetError(%d), OSError(%d)",
					(int)g_This->m_iMyErrorCode, g_This->m_iOSErrorCode);

				// ���� �߻� �Լ� ȣ��
				g_This->OnError((int)euError::NETWORK_LIB_ERROR__A_THREAD_ABONORMAL_EXIT, L"accpet(). Abonormal_exit");

				break;
			}
					


			// ------------------
			// ��� ���������� �Ϸ�Ǿ����� ���� �� ó�� �Լ� ȣ��.
			// ------------------
			NewSession->m_lIOCount++; // I/Oī��Ʈ ++. �������� 1���� ����. ���� recv,send �� ��͵� �Ȱɾ��� ������ �׳� ++�ص� ����!
			g_This->OnClientJoin(NewSession->m_ullSessionID);	

			// ������ �� ����. disconnect������ ���Ǵ� �����̱� ������ ���Ͷ� ���
			InterlockedIncrement(&g_This->m_ullJoinUserCount);

			// ------------------
			// �񵿱� ����� ����
			// ------------------
			// ��ȯ���� false���, �� �ȿ��� ����� ������. �ٵ� �ȹ���
			g_This->RecvPost(NewSession);

			// I/Oī��Ʈ --. 0�̶�� ����ó��
			if (InterlockedDecrement(&NewSession->m_lIOCount) == 0)
				g_This->InDisconnect(NewSession);
			
		}

		return 0;
	}

	// map�� Exclusive �� �ɱ�
	void CLanServer::LockMap_Exclusive_Func()
	{
		AcquireSRWLockExclusive(&m_srwSession_map_srwl);
	}

	// map�� Exclusive �� Ǯ��
	void CLanServer::UnlockMap_Exclusive_Func()
	{
		ReleaseSRWLockExclusive(&m_srwSession_map_srwl);
	}

	// map�� Shared �� �ɱ�, �� Ǯ��
	void CLanServer::LockMap_Shared_Func()
	{
		AcquireSRWLockShared(&m_srwSession_map_srwl);
	}

	// map�� Shared �� Ǯ��
	void CLanServer::UnlockMap_Shared_Func()
	{
		ReleaseSRWLockShared(&m_srwSession_map_srwl);
	}
	
	// ClinetID�� Session����ü ã�� �Լ�
	CLanServer::stSession* CLanServer::FineSessionPtr(ULONGLONG ClinetID)
	{
		map <ULONGLONG, stSession*>::iterator iter;

		iter = map_Session.find(ClinetID);
		if (iter == map_Session.end())
		{
			return nullptr;
		}

		stSession* NowSession = iter->second;
		return NowSession;
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
		InterlockedExchange(&m_ullJoinUserCount, 0);
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

			int DequeueSize = NowSession->m_RecvQueue.Dequeue(PayloadBuff.GetBufferPtr(), Header_PaylaodSize);
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
			// ���⼭ ���� UseSize�� ���� ������ �̴�. �Ʒ����� ���� �����Ҷ��� ����Ѵ�.
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

			// 6. ���� ó��
			if (WSASend(NowSession->m_Client_sock, wsabuf, wsabufCount, &SendBytes, flags, &NowSession->m_overSendOverlapped, NULL) == SOCKET_ERROR)
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
						cNetLibLog->LogSave(L"LanServer", CSystemLog::en_LogLevel::LEVEL_ERROR, L"WSASend --> %s : NetError(%d), OSError(%d)",
							tcErrorString, (int)m_iMyErrorCode, m_iOSErrorCode);

						// ���� �Լ� ȣ��
						OnError((int)euError::NETWORK_LIB_ERROR__WSAENOBUFS, tcErrorString);
					}

					// ���۸� üũ
					//else if (Error != WSAESHUTDOWN && Error != WSAECONNRESET && Error != WSAECONNABORTED && Error != WSAEMFILE)
					//{
					//	// �ϴ� ������ϴ� �˴ٿ� ȣ��
					//	shutdown(NowSession->m_Client_sock, SD_BOTH);

					//	// �� ����, �����쿡�� ����
					//	m_iOSErrorCode = Error;
					//	m_iMyErrorCode = euError::NETWORK_LIB_ERROR__WSASEND_FAIL;

					//	// ���� ��Ʈ�� �����
					//	TCHAR tcErrorString[300];
					//	StringCchPrintf(tcErrorString, 300, _T("WSASendFail... UserID : %d, [%s:%d]"),
					//		NowSession->m_ullSessionID, NowSession->m_IP, NowSession->m_prot);

					//	// ���� �Լ� ȣ��
					//	OnError((int)euError::NETWORK_LIB_ERROR__WSASEND_FAIL, tcErrorString);
					//}		
				}
			}
			break;
		}

		return true;
	}

}


