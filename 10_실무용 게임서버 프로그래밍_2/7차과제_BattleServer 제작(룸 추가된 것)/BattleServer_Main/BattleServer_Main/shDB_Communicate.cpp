#include "pch.h"
#include "shDB_Communicate.h"

#include <process.h>


// -----------------------
// shDB�� ����ϴ� Ŭ����
// -----------------------
namespace Library_Jingyu
{

	// --------------
	// ������
	// --------------

	// DB_Read ������
	UINT WINAPI shDB_Communicate::DB_ReadThread(LPVOID lParam)
	{
		shDB_Communicate* gThis = (shDB_Communicate*)lParam;
		
		// --------------------
		// ���÷� �޾Ƶα�
		// --------------------

		// IOCP �ڵ�
		HANDLE hIOCP = gThis->m_hDB_Read;

		// Read�� �Ϸ�� DB_WORK* �� �����صδ� ������ ť �޾Ƶα�
		CLF_Queue<DB_WORK*> *m_pComQueue = gThis->m_pDB_ReadComplete_Queue;


		// --------------------
		// ���� ����
		// --------------------
		HTTP_Exchange m_HTTP_Post((TCHAR*)_T("10.0.0.1"), 80);	

		DWORD APIType;
		DB_WORK* pWork;
		OVERLAPPED* overlapped;
		
		while (1)
		{
			// ������ �ʱ�ȭ
			APIType = 0;
			pWork = nullptr;
			overlapped = nullptr;

			// ��ȣ ��ٸ���
			GetQueuedCompletionStatus(hIOCP, &APIType, (PULONG_PTR)&pWork, &overlapped, INFINITE);

			// ���� ��ȣ�� ��� ó��
			if (APIType == en_PHP_TYPE::EXIT)
				break;


			// 1. APIType Ȯ��
			switch (APIType)
			{

				// Seelct_account.php
			case SELECT_ACCOUNT:
			{
				DB_WORK_LOGIN* NowWork = (DB_WORK_LOGIN*)pWork;

				// DB�� ���� ������ Ȯ��.
				TCHAR Body[1000];				

				ZeroMemory(NowWork->m_tcResponse, sizeof(NowWork->m_tcResponse));
				ZeroMemory(Body, sizeof(Body));

				// 1. Body �����
				swprintf_s(Body, _Mycountof(Body), L"{\"accountno\" : %lld}", NowWork->AccountNo);

				// 2. http ��� �� ��� ���
				int TryCount = 5;
				while (m_HTTP_Post.HTTP_ReqANDRes((TCHAR*)_T("Contents/Select_account.php"), Body, NowWork->m_tcResponse) == false)
				{
					TryCount--;

					if(TryCount == 0)
						gThis->m_Dump->Crash();
				}					

				// 3. Read �Ϸ� ť�� �ֱ�
				m_pComQueue->Enqueue(pWork);
			}
				break;


				// Seelct_contents.php
			case SELECT_CONTENTS:
			{
				DB_WORK_LOGIN_CONTENTS* NowWork = (DB_WORK_LOGIN_CONTENTS*)pWork;

				// DB�� ���� ������ Ȯ��.
				TCHAR Body[1000];

				ZeroMemory(NowWork->m_tcResponse, sizeof(NowWork->m_tcResponse));
				ZeroMemory(Body, sizeof(Body));

				// 1. Body �����
				swprintf_s(Body, _Mycountof(Body), L"{\"accountno\" : %lld}", NowWork->AccountNo);

				// 2. http ��� �� ��� ���
				int TryCount = 5;
				while (m_HTTP_Post.HTTP_ReqANDRes((TCHAR*)_T("Contents/Select_contents.php"), Body, NowWork->m_tcResponse) == false)
				{
					TryCount--;

					if (TryCount == 0)
						gThis->m_Dump->Crash();
				}

				// 3. Read �Ϸ� ť�� �ֱ�
				m_pComQueue->Enqueue(pWork);

			}
				break;


				// ���� Ÿ���̸� ũ����
			default:
				gThis->m_Dump->Crash();
				break;
			}
		}

		return 0;
	}

	// DB_Write ������
	UINT WINAPI shDB_Communicate::DB_WriteThread(LPVOID lParam)
	{
		shDB_Communicate* gThis = (shDB_Communicate*)lParam;

		// --------------------
		// ���÷� �޾Ƶα�
		// --------------------

		// ������ ����� �ϰ� ť
		CNormalQueue<DB_WORK*> *pWorkerQueue = gThis->m_pDB_Wirte_Start_Queue;

		// �Ϸ�� �۾� ���� ť
		CNormalQueue<DB_WORK*> *pEndQueue = gThis->m_pDB_Wirte_End_Queue;


		// --------------
		// ���� ����
		// --------------

		// �� ��Ű���, ����� �̺�Ʈ �޾Ƶα�
		// [���� ��ȣ, ���ϱ� ��ȣ] �������
		HANDLE hEvent[2] = { gThis->m_hDBWrite_Exit_Event , gThis->m_hDBWrite_Event };

		DB_WORK* pWork;

		HTTP_Exchange m_HTTP_Post((TCHAR*)_T("10.0.0.1"), 80);
		
		while (1)
		{
			// �̺�Ʈ ���
			DWORD Check = WaitForMultipleObjects(2, hEvent, FALSE, INFINITE);

			// �̻��� ��ȣ���
			if (Check == WAIT_FAILED)
			{
				DWORD Error = GetLastError();
				printf("DB_WriteThread Exit Error!!! (%d) \n", Error);
				
				gThis->m_Dump->Crash();
			}

			// ���� ��ȣ�� �Դٸ������Ʈ ������ ����.
			else if (Check == WAIT_OBJECT_0)
				break;


			// ----------------- �ϰ� ������ ���Ѵ�.
			// �ϰ� ���� ���������� �޾Ƶд�.
			// �ش� �ϰ���ŭ�� ó���ϰ� �ڷ�����.
			// ó�� ���� �ϰ���, ������ ���� ó���Ѵ�.
			int Size = pWorkerQueue->GetNodeSize();

			while (Size > 0)
			{
				Size--;

				// 1. ť���� �ϰ� 1�� ������	
				if (pWorkerQueue->Dequeue(pWork) == -1)
					gThis->m_Dump->Crash();

				DB_WORK_CONTENT_UPDATE* NowWork = (DB_WORK_CONTENT_UPDATE*)pWork;

				// 2. �ϰ� Ÿ�Կ� ���� ���� ó��
				switch (NowWork->m_wWorkType)
				{
					// �÷��� ī��Ʈ ����
				case eu_DB_AFTER_TYPE::eu_PLAYCOUNT_UPDATE:
				{
					// DB�� ���� ����
					TCHAR Body[1000];

					ZeroMemory(NowWork->m_tcResponse, sizeof(NowWork->m_tcResponse));
					ZeroMemory(Body, sizeof(Body));

					// 1. Body �����
					swprintf_s(Body, _Mycountof(Body), L"{\"accountno\" : %lld, \"playcount\" : \"%d\"}",	NowWork->AccountNo, NowWork->m_iCount);

					// 2. HTTP ���
					int TryCount = 5;
					while (m_HTTP_Post.HTTP_ReqANDRes((TCHAR*)_T("Contents/Update_contents.php"), Body, NowWork->m_tcResponse) == false)
					{
						TryCount--;

						if (TryCount == 0)
							gThis->m_Dump->Crash();
					}

					// 3. Write �Ϸ� ť�� �ֱ�
					pEndQueue->Enqueue(pWork);
				}
					break;

					// �÷��� �ð� ����
				case eu_DB_AFTER_TYPE::eu_PLAYTIME_UPDATE:
				{
					// DB�� ���� ����
					TCHAR Body[1000];

					ZeroMemory(NowWork->m_tcResponse, sizeof(NowWork->m_tcResponse));
					ZeroMemory(Body, sizeof(Body));

					// 1. Body �����
					swprintf_s(Body, _Mycountof(Body), L"{\"accountno\" : %lld, \"playtime\" : \"%d\"}", NowWork->AccountNo, NowWork->m_iCount);

					// 2. HTTP ���
					int TryCount = 5;
					while (m_HTTP_Post.HTTP_ReqANDRes((TCHAR*)_T("Contents/Update_contents.php"), Body, NowWork->m_tcResponse) == false)
					{
						TryCount--;

						if (TryCount == 0)
							gThis->m_Dump->Crash();
					}

					// 3. Write �Ϸ� ť�� �ֱ�
					pEndQueue->Enqueue(pWork);
				}
					break;

					// ų ī��Ʈ ����
				case eu_DB_AFTER_TYPE::eu_KILL_UPDATE:
				{
					// DB�� ���� ����
					TCHAR Body[1000];

					ZeroMemory(NowWork->m_tcResponse, sizeof(NowWork->m_tcResponse));
					ZeroMemory(Body, sizeof(Body));

					// 1. Body �����
					swprintf_s(Body, _Mycountof(Body), L"{\"accountno\" : %lld, \"kill\" : \"%d\"}", NowWork->AccountNo, NowWork->m_iCount);

					// 2. HTTP ���
					int TryCount = 5;
					while (m_HTTP_Post.HTTP_ReqANDRes((TCHAR*)_T("Contents/Update_contents.php"), Body, NowWork->m_tcResponse) == false)
					{
						TryCount--;

						if (TryCount == 0)
							gThis->m_Dump->Crash();
					}

					// 3. Write �Ϸ� ť�� �ֱ�
					pEndQueue->Enqueue(pWork);

				}
					break;

					// ��� ī��Ʈ ����
				case eu_DB_AFTER_TYPE::eu_DIE_UPDATE:
				{
					// DB�� ���� ����
					TCHAR Body[1000];

					ZeroMemory(NowWork->m_tcResponse, sizeof(NowWork->m_tcResponse));
					ZeroMemory(Body, sizeof(Body));

					// 1. Body �����
					swprintf_s(Body, _Mycountof(Body), L"{\"accountno\" : %lld, \"die\" : \"%d\"}", NowWork->AccountNo, NowWork->m_iCount);

					// 2. HTTP ���
					int TryCount = 5;
					while (m_HTTP_Post.HTTP_ReqANDRes((TCHAR*)_T("Contents/Update_contents.php"), Body, NowWork->m_tcResponse) == false)
					{
						TryCount--;

						if (TryCount == 0)
							gThis->m_Dump->Crash();
					}

					// 3. Write �Ϸ� ť�� �ֱ�
					pEndQueue->Enqueue(pWork);

				}
					break;

					// �¸� ī��Ʈ ����
				case eu_DB_AFTER_TYPE::eu_WIN_UPDATE:
				{
					// DB�� ���� ����
					TCHAR Body[1000];

					ZeroMemory(NowWork->m_tcResponse, sizeof(NowWork->m_tcResponse));
					ZeroMemory(Body, sizeof(Body));

					// 1. Body �����
					swprintf_s(Body, _Mycountof(Body), L"{\"accountno\" : %lld, \"win\" : \"%d\"}", NowWork->AccountNo, NowWork->m_iCount);

					// 2. HTTP ���
					int TryCount = 5;
					while (m_HTTP_Post.HTTP_ReqANDRes((TCHAR*)_T("Contents/Update_contents.php"), Body, NowWork->m_tcResponse) == false)
					{
						TryCount--;

						if (TryCount == 0)
							gThis->m_Dump->Crash();
					}

					// 3. Write �Ϸ� ť�� �ֱ�
					pEndQueue->Enqueue(pWork);
				}
					break;

				default:
					gThis->m_Dump->Crash();
				}			
				
				}

		}

		return 0;
	}



	// ---------------
	// ��� �Լ�. �ܺο��� ȣ�� ����
	// ---------------

	// DB�� Read�� ���� ���� �� ȣ��Ǵ� �Լ�
	// ���ڷ� ���� ����ü�� ������ Ȯ���� ���� ó��
	// 
	// Parameter : DB_WORK*, APIType
	// return : ����
	void shDB_Communicate::DBReadFunc(DB_WORK* Protocol, WORD APIType)
	{
		// Read�� �����忡�� �ϰ� ���� (PQCS)
		// �̰� ��
		PostQueuedCompletionStatus(m_hDB_Read, APIType, (ULONG_PTR)Protocol, 0);
	}	

	// DB�� Write �� ���� ���� �� ȣ��Ǵ� �Լ�.
	// ���ڷ� ���� ����ü�� ������ Ȯ���� ���� ó��
	//
	// Parameter : DB_WORK*
	// return : ����
	void shDB_Communicate::DBWriteFunc(DB_WORK* Protocol)
	{
		// Wirte�� �����忡�� �ϰ� ���� (Normal Q ���)
		m_pDB_Wirte_Start_Queue->Enqueue(Protocol);

		// Write ������ �����
		SetEvent(m_hDBWrite_Event);
	}


	// ---------------
	// �����ڿ� �Ҹ���
	// ---------------

	// ������
	shDB_Communicate::shDB_Communicate()
	{

		// DB Write�� ������� �Ͻ�Ű�� ���̺�Ʈ �����
		// �ڵ� ���� �̺�Ʈ.
		m_hDBWrite_Event = CreateEvent(NULL, FALSE, FALSE, NULL);

		// DB Write�� ������ ����� �̺�Ʈ �����
		// �ڵ� ���� �̺�Ʈ.
		m_hDBWrite_Exit_Event = CreateEvent(NULL, FALSE, FALSE, NULL);

		// ���� �ޱ�
		m_Dump = CCrashDump::GetInstance();

		// DB_WORK �޸�Ǯ �����Ҵ�
		m_pDB_Work_Pool = new CMemoryPoolTLS<DB_WORK>(0, false);
		
		// Read �Ϸ� ������ ť �����Ҵ�
		m_pDB_ReadComplete_Queue = new CLF_Queue<DB_WORK*>(0);
		
		// Write �Ϸ� ��� ť �����Ҵ�.
		m_pDB_Wirte_End_Queue = new CNormalQueue<DB_WORK*>();

		// Write �����忡�� �Ͻ�Ű��� ť �����Ҵ�.
		m_pDB_Wirte_Start_Queue = new CNormalQueue<DB_WORK*>();

		// DB_Read�� ����� �Ϸ���Ʈ ����
		// 4���� ������ ����, 2���� ������ Ȱ��ȭ
		int Create = 4;
		int Active = 2;		

		m_hDB_Read = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, NULL, Active);
		if (m_hDB_Read == NULL)
			m_Dump->Crash();

		// DB_Read�� ��Ŀ ������ ����
		m_hDB_Read_Thread = new HANDLE[Create];
		for (int i = 0; i < Create; ++i)
		{
			m_hDB_Read_Thread[i] = (HANDLE)_beginthreadex(0, 0, DB_ReadThread, this, 0, 0);
			if(m_hDB_Read_Thread[i] == NULL)
				m_Dump->Crash();
		}

		// DB_Write�� ������
		m_hDB_Write_Thread = (HANDLE)_beginthreadex(0, 0, DB_WriteThread, this, 0, 0);
		if (m_hDB_Write_Thread == NULL)
			m_Dump->Crash();
	}

	// �Ҹ���
	shDB_Communicate::~shDB_Communicate()
	{
		// 1. DB_ReadThread ����
		for(int i=0; i<4; ++i)
			PostQueuedCompletionStatus(m_hDB_Read, en_PHP_TYPE::EXIT, 0, 0);

		WaitForMultipleObjects(4, m_hDB_Read_Thread, TRUE, INFINITE);		

		// 2. ����� �Ϸ���Ʈ �ݱ�
		CloseHandle(m_hDB_Read);

		// 3. Read �Ϸ� ������ ť ����
		DB_WORK* Delete;
		while (1)
		{
			// ������
			if (m_pDB_ReadComplete_Queue->Dequeue(Delete) == -1)
				break;

			// ��ȯ
			m_pDB_Work_Pool->Free(Delete);
		}

		// 4. DB_WriteThread ����
		SetEvent(m_hDBWrite_Exit_Event);

		WaitForSingleObject(m_hDB_Write_Thread, INFINITE);

		// 5. Write �Ͻ�Ű�� �� ť ����
		while (1)
		{
			// ������
			if (m_pDB_Wirte_Start_Queue->Dequeue(Delete) == -1)
				break;

			// ��ȯ
			m_pDB_Work_Pool->Free(Delete);
		}

		// 6. Write �� �Ϸ�� ť ����
		while (1)
		{
			// ������
			if (m_pDB_Wirte_End_Queue->Dequeue(Delete) == -1)
				break;

			// ��ȯ
			m_pDB_Work_Pool->Free(Delete);
		}


		// 7. ���� ��������

		// ��Ŀ ������ �ڵ� ��������
		delete[] m_hDB_Read_Thread;

		// DB_WORK �޸�Ǯ ��������
		delete m_pDB_Work_Pool;

		// Read �Ϸ� ������ ť ��������
		delete m_pDB_ReadComplete_Queue;

		// Write �Ϸ� ��� ť ��������
		delete m_pDB_Wirte_End_Queue;

		// Write �����忡�� �Ͻ�Ű��� ť ��������
		delete m_pDB_Wirte_Start_Queue;
	}
}