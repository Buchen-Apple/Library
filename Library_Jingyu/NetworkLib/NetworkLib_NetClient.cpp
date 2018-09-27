/*
������ ����� Network Library
*/

#include "pch.h"

#pragma comment(lib,"ws2_32")
#include <Ws2tcpip.h>

#include <process.h>
#include <strsafe.h>

#include "NetworkLib_NetClient.h"
#include "CrashDump\CrashDump.h"
#include "Log\Log.h"

LONG	g_lSendPostTPS;


namespace Library_Jingyu
{

#define _MyCountof(_array)		sizeof(_array) / (sizeof(_array[0]))

	// �α� ���� �������� �ϳ� �ޱ�.
	CSystemLog* cNetClientLibLog = CSystemLog::GetInstance();

	// ���� ���� ���� �ϳ� �ޱ�
	CCrashDump* cNetClientDump = CCrashDump::GetInstance();

	// ��� ������
#define dfNETWORK_PACKET_HEADER_SIZE_NETSERVER	5

	// �� ���� ������ �� �ִ� WSABUF�� ī��Ʈ
#define dfSENDPOST_MAX_WSABUF			300


	// ------------------------------
	// enum�� ����ü
	// ------------------------------
	// ��� ����ü
#pragma pack(push, 1)
	struct CNetClient::stProtocolHead
	{
		BYTE	m_Code;
		WORD	m_Len;
		BYTE	m_RandXORCode;
		BYTE	m_Checksum;
	};
#pragma pack(pop)
		

	// -----------------------------
	// ������ ȣ�� �ϴ� �Լ�
	// -----------------------------

	// ���� ����
	// [���� IP(���ε� �� IP), ��Ʈ, ��Ŀ������ ��, Ȱ��ȭ��ų ��Ŀ������ ��, ����Ʈ ������ ��, TCP_NODELAY ��� ����(true�� ���), �ִ� ������ ��, ��Ŷ Code, XOR 1���ڵ�, XOR 2���ڵ�] �Է¹���.
	//
	// return false : ���� �߻� ��. �����ڵ� ���� �� false ����
	// return true : ����
	bool CNetClient::Start(const TCHAR* ConnectIP, USHORT port, int WorkerThreadCount, int ActiveWThreadCount, bool Nodelay, BYTE Code, BYTE XORCode1, BYTE XORCode2)
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

		// ������ ������ IP�� Port �����صα�
		StringCchCopy(m_tcServerIP, _MyCountof(m_tcServerIP), ConnectIP);
		m_wPort = port;

		// ���� �ʱ�ȭ
		WSADATA wsa;
		int retval = WSAStartup(MAKEWORD(2, 2), &wsa);
		if (retval != 0)
		{
			// ������ ����, �� ���� ����
			m_iOSErrorCode = retval;
			m_iMyErrorCode = euError::NETWORK_LIB_ERROR__WINSTARTUP_FAIL;

			// ���� �ڵ� ��ȯ �� �������� ����.
			ExitFunc(WorkerThreadCount);

			// �α� ��� (�α� ���� : ����)
			cNetClientLibLog->LogSave(L"NetClient", CSystemLog::en_LogLevel::LEVEL_ERROR, L"Start() --> WSAStartup() Error : NetError(%d), OSError(%d)",
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
			ExitFunc(WorkerThreadCount);

			// �α� ��� (�α� ���� : ����)
			cNetClientLibLog->LogSave(L"NetClient", CSystemLog::en_LogLevel::LEVEL_ERROR, L"Start() --> CreateIoCompletionPort() Error : NetError(%d), OSError(%d)",
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
				cNetClientLibLog->LogSave(L"NetClient", CSystemLog::en_LogLevel::LEVEL_ERROR, L"Start() --> WorkerThread Create Error : NetError(%d), OSError(%d)",
					(int)m_iMyErrorCode, m_iOSErrorCode);

				// false ����
				return false;
			}
		}

		// ����
		ConnectFunc();

		// ���� ���� �α� ���		
		// �̰�, ��ӹ޴� �ʿ��� ��°ɷ� ����. �ݼ��� ��ü�� ���������� �۵����� ����.
		//cNetClientLibLog->LogSave(L"NetClient", CSystemLog::en_LogLevel::LEVEL_SYSTEM, L"ServerOpen...");

		return true;
	}

	// ���� ��ž.
	void CNetClient::Stop()
	{

		// 1. Ŭ�� ���� ���� ����
		shutdown(m_stSession.m_Client_sock, SD_BOTH);

		// ����Ǿ����� üũ
		while (1)
		{
			if (m_ullJoinUserCount == 0)
				break;

			Sleep(1);
		}		

		// 2. ��Ŀ ������ ����
		for (int i = 0; i < m_iW_ThreadCount; ++i)
			PostQueuedCompletionStatus(m_hIOCPHandle, 0, 0, 0);

		// ��Ŀ������ ���� ���
		DWORD retval = WaitForMultipleObjects(m_iW_ThreadCount, m_hWorkerHandle, TRUE, INFINITE);

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
			cNetClientLibLog->LogSave(L"NetClient", CSystemLog::en_LogLevel::LEVEL_ERROR, L"Stop() --> Worker Thread EXIT Error : NetError(%d), OSError(%d)",
				(int)m_iMyErrorCode, m_iOSErrorCode);

			// ���� �߻� �Լ� ȣ��
			OnError((int)euError::NETWORK_LIB_ERROR__W_THREAD_ABNORMAL_EXIT, L"Stop() --> Worker Thread EXIT Error");
		}

		// 4. ���� ���ҽ� ��ȯ

		// 1) ��Ŀ ������ �ڵ� ��ȯ
		for (int i = 0; i < m_iW_ThreadCount; ++i)
			CloseHandle(m_hWorkerHandle[i]);

		// 2) ��Ŀ ������ �迭 ��������
		delete[] m_hWorkerHandle;

		// 3) IOCP�ڵ� ��ȯ
		CloseHandle(m_hIOCPHandle);

		// 4) ���� ����
		WSACleanup();

		// 5. Ŭ�� ������ �ƴ� ���·� ����
		m_bClienetConnect = false;

		// 6. ���� ���� �ʱ�ȭ
		Reset();		
	}

	// �ܺο���, � �����͸� ������ ������ ȣ���ϴ� �Լ�.
	// SendPacket�� �׳� �ƹ����� �ϸ� �ȴ�.
	// �ش� ������ SendQ�� �־�״ٰ� ���� �Ǹ� ������.
	//
	// Parameter : SessionID, SendBuff, LastFlag(����Ʈ FALSE)
	// return : ����
	void CNetClient::SendPacket(ULONGLONG SessionID, CProtocolBuff_Net* payloadBuff, LONG LastFlag)
	{
		// 1. ���� �� �ɱ�(�� �ƴ����� ��ó�� �����)
		stSession* NowSession = GetSessionLOCK(SessionID);
		if (NowSession == nullptr)
		{
			// �� ����, ť���� ���� ���߱� ������, ���ڷ� ���� ����ȭ���۸� Free�Ѵ�.
			// ref ī��Ʈ�� 0�� �Ǹ� �޸�Ǯ�� ��ȯ
			CProtocolBuff_Net::Free(payloadBuff);
			return;
		}

		// 2. ���ڷ� ���� Flag�� true���, ������ ��Ŷ�� �ּҸ� ����
		if (LastFlag == TRUE)
			NowSession->m_LastPacket = payloadBuff;

		// 3. ����� �־, ��Ŷ �ϼ��ϱ�
		payloadBuff->Encode(m_bCode, m_bXORCode_1, m_bXORCode_2);

		// 4. ��ť. ��Ŷ�� "�ּ�"�� ��ť�Ѵ�(8����Ʈ)
		// ����ȭ ���� ���۷��� ī��Ʈ 1 ����
		payloadBuff->Add();
		NowSession->m_SendQueue->Enqueue(payloadBuff);

		// 5. ����ȭ ���� ���۷��� ī��Ʈ 1 ����. 0 �Ǹ� �޸�Ǯ�� ��ȯ
		CProtocolBuff_Net::Free(payloadBuff);	
				
		// 6. SendPost�õ�
		SendPost(NowSession);

		// 7. ���� �� ����(�� �ƴ����� ��ó�� ���)
		// ���⼭ false�� ���ϵǸ� �̹� �ٸ������� �����Ǿ���� �ߴµ� �� SendPacket�� I/Oī��Ʈ�� �ø����� ���� �������� ���� ��������.
		// �ٵ� ���� ���ϰ� ���� �ʰ� ����
		GetSessionUnLOCK(NowSession);
	}

	// ������ ������ ���� �� ȣ���ϴ� �Լ�. �ܺ� ���� ���.
	// ���̺귯������ ������!��� ��û�ϴ� �� ��
	//
	// Parameter : SessionID
	// return : ����
	void CNetClient::Disconnect(ULONGLONG SessionID)
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
	int CNetClient::WinGetLastError()
	{
		return m_iOSErrorCode;
	}

	// �� ���� ���
	int CNetClient::NetLibGetLastError()
	{
		return (int)m_iMyErrorCode;
	}

	// ���� ������ �� ���
	ULONGLONG CNetClient::GetClientCount()
	{
		return m_ullJoinUserCount;
	}

	// Ŭ�� ���������� Ȯ��
	// return true : ������
	// return false : ������ �ƴ�
	bool CNetClient::GetClinetState()
	{
		return m_bClienetConnect;
	}







	// -----------------------------
	// Lib ���ο����� ����ϴ� �Լ�
	// -----------------------------
	// ������
	CNetClient::CNetClient()
	{
		// Ŭ�� ������ ���·� ���� 
		m_bClienetConnect = false;
	}

	// �Ҹ���
	CNetClient::~CNetClient()
	{
		// Ŭ�� ���� ������ �������̶��, ���� ���� ����
		if (m_bClienetConnect == true)
		{
			m_bClienetConnect = false;
			Stop();
		}
	}

	// ������ �������� ��Ű�� �Լ�
	void CNetClient::InDisconnect(stSession* DeleteSession)
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
					cNetClientDump->Crash();

				CProtocolBuff_Net::Free(Payload);

				i++;
			}			

			// ť �ʱ�ȭ
			DeleteSession->m_RecvQueue.ClearBuffer();

			// SendFlag �ʱ�ȭ
			DeleteSession->m_lSendFlag = FALSE;

			// Ŭ���� ����
			closesocket(DeleteSession->m_Client_sock);		

			// ���� �� ���� �� ����
			InterlockedDecrement(&m_ullJoinUserCount);

			// ������ �� ������ ����Ǿ��ٰ� �˷���.
			// �������� ����� ���� ����Ű�� �̿��� ����Ѵ�. �׷��� ���ڷ� ����Ű�� �Ѱ��ش�.
			OnDisconnect(sessionID);
		}
		
		return;	
	}

	// Start���� ������ �� �� ȣ���ϴ� �Լ�.
	// 1. ����� �Ϸ���Ʈ �ڵ� ��ȯ
	// 2. ��Ŀ������ ����, �ڵ��ȯ, �ڵ� �迭 ��������
	// 3. ����Ʈ ������ ����, �ڵ� ��ȯ
	// 4. �������� �ݱ�
	// 5. ���� ����
	void CNetClient::ExitFunc(int w_ThreadCount)
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

		// 3. �������� �ݱ�
		if (m_soListen_sock != NULL)
			closesocket(m_soListen_sock);

		// 4. ���� ����
		// ���� �ʱ�ȭ ���� ���� ���¿��� WSAClenup() ȣ���ص� �ƹ� ���� ����
		WSACleanup();
	}

	// ��Ŀ������
	UINT WINAPI	CNetClient::WorkerThread(LPVOID lParam)
	{
		DWORD cbTransferred;
		stSession* stNowSession;
		OVERLAPPED* overlapped;

		CNetClient* g_This = (CNetClient*)lParam;

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
		

	// ������ ������ connect�ϴ� �Լ�
	bool CNetClient::ConnectFunc()
	{
		// RecvPost�� �ϴٰ� ������ ���, InDisconnect�� �Ǳ� ������ while������ �ݺ��Ѵ�.
		while (1)
		{
			// ------------------
			// ���� ����ü ���� �� ����
			// ------------------

			// 1) I/O ī��Ʈ ����
			// ���� ���
			InterlockedIncrement(&m_stSession.m_lIOCount);

			// 2) ���� �����ϱ�
			m_stSession.m_ullSessionID = InterlockedIncrement(&m_ullUniqueSessionID);


			// ------------------
			// ���� ���� �� ����
			// ------------------
			m_stSession.m_Client_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
			if (m_stSession.m_Client_sock == INVALID_SOCKET)
			{
				// ������ ����, �� ���� ����
				m_iOSErrorCode = WSAGetLastError();
				m_iMyErrorCode = euError::NETWORK_LIB_ERROR__CREATE_SOCKET_FAIL;

				// ���� �ڵ� ��ȯ �� �������� ����.
				ExitFunc(m_iW_ThreadCount);

				// �α� ��� (�α� ���� : ����)
				cNetClientLibLog->LogSave(L"LanClient", CSystemLog::en_LogLevel::LEVEL_ERROR, L"ConnectFunc() --> socket() Error : NetError(%d), OSError(%d)",
					(int)m_iMyErrorCode, m_iOSErrorCode);

				// false ����
				return false;

			}

			// ������ �۽Ź��� ũ�⸦ 0���� ����. �׷��� ���������� �񵿱� ��������� ����
			//int optval = 0;
			//int retval = setsockopt(m_stSession.m_Client_sock, SOL_SOCKET, SO_SNDBUF, (char*)&optval, sizeof(optval));
			//if (optval == SOCKET_ERROR)
			//{
			//	// ������ ����, �� ���� ����
			//	m_iOSErrorCode = WSAGetLastError();
			//	m_iMyErrorCode = euError::NETWORK_LIB_ERROR__SOCKOPT_FAIL;

			//	// ���� �ڵ� ��ȯ �� �������� ����.
			//	ExitFunc(m_iW_ThreadCount);

			//	// �α� ��� (�α� ���� : ����)
			//	cNetClientLibLog->LogSave(L"LanClient", CSystemLog::en_LogLevel::LEVEL_ERROR, L"ConnectFunc() --> setsockopt() SendBuff Size Change Error : NetError(%d), OSError(%d)",
			//		(int)m_iMyErrorCode, m_iOSErrorCode);

			//	// false ����
			//	return false;
			//}

			// ������� �ɼ� ��� ���ο� ���� ���̱� �ɼ� ���� (Start���� ���޹���)
			// true�� ������� ����ϰڴٴ� ��(���̱� �������Ѿ���)
			if (m_bNodelay == true)
			{
				BOOL optval = TRUE;
				int retval = setsockopt(m_soListen_sock, IPPROTO_TCP, TCP_NODELAY, (char*)&optval, sizeof(optval));

				if (retval == SOCKET_ERROR)
				{
					// ������ ����, �� ���� ����
					m_iOSErrorCode = WSAGetLastError();
					m_iMyErrorCode = euError::NETWORK_LIB_ERROR__SOCKOPT_FAIL;

					// ���� �ڵ� ��ȯ �� �������� ����.
					ExitFunc(m_iW_ThreadCount);

					// �α� ��� (�α� ���� : ����)
					cNetClientLibLog->LogSave(L"LanClient", CSystemLog::en_LogLevel::LEVEL_ERROR, L"ConnectFunc() --> setsockopt() Nodelay apply Error : NetError(%d), OSError(%d)",
						(int)m_iMyErrorCode, m_iOSErrorCode);

					// false ����
					return false;
				}
			}


			// ------------------
			// ���� connect
			// ------------------
			// ���� �������� ����
			ULONG on = 1;
			DWORD check = ioctlsocket(m_stSession.m_Client_sock, FIONBIO, &on);
			if (check == SOCKET_ERROR)
			{
				printf("NoneBlock Change Fail...\n");
				return 0;
			}

			SOCKADDR_IN clientaddr;
			ZeroMemory(&clientaddr, sizeof(clientaddr));
			clientaddr.sin_family = AF_INET;
			clientaddr.sin_port = htons(m_wPort);
			InetPton(AF_INET, m_tcServerIP, &clientaddr.sin_addr.S_un.S_addr);

			// connect �õ�
			while (1)
			{
				connect(m_stSession.m_Client_sock, (SOCKADDR*)&clientaddr, sizeof(clientaddr));

				DWORD Check = WSAGetLastError();

				// ������ �� ���
				if (Check == WSAEWOULDBLOCK)
				{
					// �����, ���ܼ�, ����
					FD_SET wset;
					FD_SET exset;
					wset.fd_count = 0;
					exset.fd_count = 0;

					wset.fd_array[wset.fd_count] = m_stSession.m_Client_sock;
					wset.fd_count++;

					exset.fd_array[exset.fd_count] = m_stSession.m_Client_sock;
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

					}

					// Ÿ�Ӿƿ� üũ
					else if (retval == 0)
					{
						printf("Select Timeout..\n");
						printf("%d\n", WSAGetLastError());

					}

					// ������ �ִٸ�, ���ܼ¿� ������ �Դ��� üũ
					else if (exset.fd_count > 0)
					{
						//���ܼ� �����̸� ������ ��.
						printf("Select ---> exset problem..\n");
					}

					// ����¿� ������ �Դ��� üũ
					// ������� ���� ���������� �ѹ� �� üũ�غ�
					else if (wset.fd_count > 0)
					{
						// �ٽ� ������ ������� ����
						ULONG on = 0;
						DWORD check = ioctlsocket(m_stSession.m_Client_sock, FIONBIO, &on);
						if (check == SOCKET_ERROR)
						{
							printf("NoneBlock Change Fail...\n");
							return false;
						}

						// ���������� �� ������ break;
						break;

					}

					// ���������� �ٽ� ����
					Sleep(0);
					continue;
				}

				// �̹� ������ �Ǿ��ٸ� break;
				else if (Check == WSAEISCONN)
					break;
			}

			// ------------------
			// IOCP ����
			// ------------------
			// ���ϰ� IOCP ����
			if (CreateIoCompletionPort((HANDLE)m_stSession.m_Client_sock, m_hIOCPHandle, (ULONG_PTR)&m_stSession, 0) == NULL)
			{
				// ������ ����, �� ���� ����
				m_iOSErrorCode = WSAGetLastError();
				m_iMyErrorCode = euError::NETWORK_LIB_ERROR__A_THREAD_ABNORMAL_EXIT;

				// �α� ��� (�α� ���� : ����)
				cNetClientLibLog->LogSave(L"LanClient", CSystemLog::en_LogLevel::LEVEL_ERROR, L"accpet(). Abonormal_exit : NetError(%d), OSError(%d)",
					(int)m_iMyErrorCode, m_iOSErrorCode);

				// ���� �߻� �Լ� ȣ��
				OnError((int)euError::NETWORK_LIB_ERROR__A_THREAD_ABNORMAL_EXIT, L"accpet(). Abonormal_exit");

				return false;
			}

			// ------------------
			// ������ ����
			// ------------------
			// ������ ��� �������� ������ ���� ���·� ����.
			m_stSession.m_lReleaseFlag = FALSE;

			// ������ ������ ���� �� ����
			InterlockedIncrement(&m_ullJoinUserCount);

			// ------------------
			// �񵿱� ����� ����
			// ------------------
			// ��ȯ���� false���, �� �ȿ��� ����� ������. �ٵ� �ȹ���
			RecvPost(&m_stSession);

			// �������״�, I/Oī��Ʈ --. 0�̶�� ����ó��
			if (InterlockedDecrement(&m_stSession.m_lIOCount) == 0)
			{
				InDisconnect(&m_stSession);

				// ���� ��, �ٽ� �����ϱ� ���� �Լ� ������� �̵�
				continue;
			}

			// I/Oī��Ʈ�� 0�̵Ǿ� ������ ���� �ƴ϶�� ���� Connect �� ��
			// ConClientJoin ȣ��
			OnConnect(m_stSession.m_ullSessionID);

			break;
		}

		m_bClienetConnect = true;

		return true;
	}


	// SendPacket, Disconnect �� �ܺο��� ȣ���ϴ� �Լ�����, ���Ŵ� �Լ�.
	// ���� ���� �ƴ����� ��ó�� ���.
	//
	// Parameter : SessionID
	// return : ���������� ���� ã���� ��, �ش� ���� ������
	//		  : I/Oī��Ʈ�� 0�̵Ǿ� ������ ������, nullptr
	CNetClient::stSession* 	CNetClient::GetSessionLOCK(ULONGLONG SessionID)
	{
		// 1. SessionID�� ���� �˾ƿ���	
		//stSession* retSession = &m_stSessionArray[(WORD)SessionID];

		// 2. I/O ī��Ʈ 1 ����.	
		if (InterlockedIncrement(&m_stSession.m_lIOCount) == 1)
		{
			// ������ ���� 0�̸鼭, inDIsconnect ȣ��
			if (InterlockedDecrement(&m_stSession.m_lIOCount) == 0)
			{
				InDisconnect(&m_stSession);
				ConnectFunc();
			}

			return nullptr;
		}	

		// 3. Release Flag üũ
		if (m_stSession.m_lReleaseFlag == TRUE)
		{
			// �ƴ϶�� I/O ī��Ʈ 1 ����
			// ������ ���� 0�̸鼭, inDIsconnect ȣ��
			if (InterlockedDecrement(&m_stSession.m_lIOCount) == 0)
			{
				InDisconnect(&m_stSession);
				ConnectFunc();
			}

			return nullptr;
		}

		// 4. ���� ���ϴ� ������ �´��� üũ
		if (m_stSession.m_ullSessionID != SessionID)
		{
			// �̹� Release�� �������, I/O ī��Ʈ 1 ����
			// ������ ���� 0�̸鼭, inDIsconnect ȣ��
			if (InterlockedDecrement(&m_stSession.m_lIOCount) == 0)
			{
				InDisconnect(&m_stSession);
				ConnectFunc();
			}

			return nullptr;
		}								

		// 5. ���������� �ִ� ������, �����ϰ� ó���� ������� �ش� ������ ������ ��ȯ
		return &m_stSession;
	}

	// SendPacket, Disconnect �� �ܺο��� ȣ���ϴ� �Լ�����, �� �����ϴ� �Լ�
	// ���� ���� �ƴ����� ��ó�� ���.
	//
	// parameter : ���� ������
	// return : ���������� ���� ��, true
	//		  : I/Oī��Ʈ�� 0�̵Ǿ� ������ ������, false
	bool 	CNetClient::GetSessionUnLOCK(stSession* NowSession)
	{
		// 1. I/O ī��Ʈ 1 ����
		if(InterlockedDecrement(&NowSession->m_lIOCount) == 0)
			InDisconnect(NowSession);		

		return true;
	}
	   

	// ���� �������� ���½�Ų��.
	// Stop() �Լ����� ���.
	void CNetClient::Reset()
	{
		m_iW_ThreadCount = 0;
		m_hWorkerHandle = nullptr;
		m_hIOCPHandle = 0;
		m_soListen_sock = 0;
		m_ullJoinUserCount = 0;
	}






	// ------------
	// Lib ���ο����� ����ϴ� ���ú� ���� �Լ���
	// ------------
	// RecvProc �Լ�. ť�� ���� üũ �� PacketProc���� �ѱ��.
	void CNetClient::RecvProc(stSession* NowSession)
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
				cNetClientDump->Crash();

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
				cNetClientDump->Crash();
			

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
	int CNetClient::RecvPost(stSession* NowSession)
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
					cNetClientLibLog->LogSave(L"NetClient", CSystemLog::en_LogLevel::LEVEL_ERROR, L"WSARecv --> %s : NetError(%d), OSError(%d)",
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
	// return true : ���������� WSASend() �Ϸ� or WSASend�� ���������� ����� ������ �ƴ�.
	// return false : I/Oī��Ʈ�� 0�̵Ǿ ����� ����
	bool CNetClient::SendPost(stSession* NowSession)
	{
		while (1)
		{	
			// ------------------
			// send ���� �������� üũ
			// ------------------
			// 1. SendFlag(1������)�� 0(3������)�� ���ٸ�, SendFlag(1������)�� 1(2������)���� ����
			// ���⼭ TRUE�� ���ϵǴ� ����, �̹� NowSession->m_SendFlag�� 1(���� ��)�̾��ٴ� ��.
			/*if (InterlockedCompareExchange(&NowSession->m_lSendFlag, TRUE, FALSE) == TRUE)
				break;	*/

			// 1. SendFlag(1������)�� �� TRUE(2������)�� ����.
			// ���⼭ TRUE�� ���ϵǴ� ����, �̹� NowSession->m_SendFlag�� 1(���� ��)�̾��ٴ� ��.
			if (InterlockedExchange(&NowSession->m_lSendFlag, TRUE) == TRUE)
				break;
			
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
					cNetClientDump->Crash();

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
						StringCchPrintf(tcErrorString, 300, _T("WSANOBUFS. UserID : %lld, [%s:%d]"),
							NowSession->m_ullSessionID, NowSession->m_IP, NowSession->m_prot);

						// �α� ��� (�α� ���� : ����)
						cNetClientLibLog->LogSave(L"NetClient", CSystemLog::en_LogLevel::LEVEL_ERROR, L"WSASend --> %s : NetError(%d), OSError(%d)",
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

		return true;
	}

}
