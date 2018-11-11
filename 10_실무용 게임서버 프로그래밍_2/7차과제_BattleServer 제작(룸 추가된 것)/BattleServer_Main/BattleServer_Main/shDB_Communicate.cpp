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
		HTTP_Exchange m_HTTP_Post((TCHAR*)_T("127.0.0.1"), 80);	

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
				// DB�� ���� ������ Ȯ��.
				TCHAR Body[1000];

				ZeroMemory(pWork->m_tcRequest, sizeof(pWork->m_tcRequest));
				ZeroMemory(Body, sizeof(Body));

				// 1. Body �����
				swprintf_s(Body, _Mycountof(Body), L"{\"accountno\" : %lld}", pWork->AccountNo);

				// 2. http ��� �� ��� ���
				if (m_HTTP_Post.HTTP_ReqANDRes((TCHAR*)_T("Contents/Select_account.php"), Body, pWork->m_tcRequest) == false)
					gThis->m_Dump->Crash();

				// 3. Read �Ϸ� ť�� �ֱ�
				m_pComQueue->Enqueue(pWork);
			}
				break;


			default:
				break;
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


	// ---------------
	// �����ڿ� �Ҹ���
	// ---------------

	// ������
	shDB_Communicate::shDB_Communicate()
	{
		// ���� �ޱ�
		m_Dump = CCrashDump::GetInstance();

		// DB_WORK �޸�Ǯ �����Ҵ�
		m_pDB_Work_Pool = new CMemoryPoolTLS<DB_WORK>(0, false);
		
		// Read �Ϸ� ������ ť �����Ҵ�
		m_pDB_ReadComplete_Queue = new CLF_Queue<DB_WORK*>(0);

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

			// ����ȭ ���� Free
			CProtocolBuff_Net::Free(Delete->m_pBuff);

			// ��ȯ
			m_pDB_Work_Pool->Free(Delete);
		}


		// 4. ���� ��������

		// ��Ŀ ������ �ڵ� ��������
		delete[] m_hDB_Read_Thread;

		// DB_WORK �޸�Ǯ ��������
		delete m_pDB_Work_Pool;

		// Read �Ϸ� ������ ť ��������
		delete m_pDB_ReadComplete_Queue;
	}
}