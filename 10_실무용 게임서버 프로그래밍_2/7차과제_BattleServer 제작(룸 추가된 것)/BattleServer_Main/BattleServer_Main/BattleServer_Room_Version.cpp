#include "pch.h"
#include "BattleServer_Room_Version.h"
#include "Parser\Parser_Class.h"
#include "Protocol_Set\CommonProtocol_2.h"		// ���� �Ϲ����� ���� �ʿ�
#include "Protocol_Set/ContentsData.h"
#include "Log\Log.h"
#include "CPUUsage\CPUUsage.h"
#include "PDHClass\PDHCheck.h"

#include "rapidjson\document.h"
#include "rapidjson\writer.h"
#include "rapidjson\stringbuffer.h"

#include <process.h>
#include <strsafe.h>
#include <cmath>
#include <algorithm>
#include <list>

using namespace rapidjson;
using namespace std;

// -----------------------
// shDB�� ����ϴ� Ŭ����
// -----------------------
namespace Library_Jingyu
{
	// ������
	CCrashDump* g_BattleServer_Room_Dump = CCrashDump::GetInstance();

	// �α׿�
	CSystemLog* g_BattleServer_RoomLog = CSystemLog::GetInstance();

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
		HTTP_Exchange m_HTTP_Post((TCHAR*)_T("10.10.10.1"), 80);

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
				TCHAR Body[1000] = { 0, };

				ZeroMemory(NowWork->m_tcResponse, sizeof(NowWork->m_tcResponse));

				// 1. Body �����
				swprintf_s(Body, _Mycountof(Body), L"{\"accountno\" : %lld}", NowWork->AccountNo);

				// 2. http ��� �� ��� ���
				int TryCount = 5;
				while (m_HTTP_Post.HTTP_ReqANDRes((TCHAR*)_T("select_account.php"), Body, NowWork->m_tcResponse) == false)
				{
					TryCount--;

					if (TryCount == 0)
						gThis->m_Dump->Crash();

					Sleep(100);
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
				TCHAR Body[1000] = { 0, };

				ZeroMemory(NowWork->m_tcResponse, sizeof(NowWork->m_tcResponse));

				// 1. Body �����
				swprintf_s(Body, _Mycountof(Body), L"{\"accountno\" : %lld}", NowWork->AccountNo);

				// 2. http ��� �� ��� ���
				int TryCount = 5;
				while (m_HTTP_Post.HTTP_ReqANDRes((TCHAR*)_T("select_contents.php"), Body, NowWork->m_tcResponse) == false)
				{
					TryCount--;

					if (TryCount == 0)
						gThis->m_Dump->Crash();

					Sleep(100);
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

		// DB_WORK�� �����ϴ� �޸�Ǯ
		CMemoryPoolTLS<DB_WORK>* pDBWorkPool = gThis->m_pDB_Work_Pool;


		// --------------
		// ���� ����
		// --------------

		// �� ��Ű���, ����� �̺�Ʈ �޾Ƶα�
		// [���� ��ȣ, ���ϱ� ��ȣ] �������
		HANDLE hEvent[2] = { gThis->m_hDBWrite_Exit_Event , gThis->m_hDBWrite_Event };

		DB_WORK* pWork;

		HTTP_Exchange m_HTTP_Post((TCHAR*)_T("10.10.10.1"), 80);

		// ��� üũ��
		LONG* TempDBWriteTPS = &gThis->m_lDBWriteTPS;

		// Writeī��Ʈ�� ���� �뵵�� AccountNo ������
		INT64 TempAccountNo;

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

			// 1. ť���� �ϰ� 1�� ������	
			while (pWorkerQueue->Dequeue(pWork) != -1)
			{
				// 2. �ϰ� Ÿ�Կ� ���� ���� ó��
				switch (pWork->m_wWorkType)
				{
					// �÷��� ī��Ʈ ����
				case eu_DB_AFTER_TYPE::eu_PLAYCOUNT_UPDATE:
				{
					DB_WORK_CONTENT_UPDATE* NowWork = (DB_WORK_CONTENT_UPDATE*)pWork;

					// DB�� ���� ����
					TCHAR Body[1000] = { 0, };

					ZeroMemory(NowWork->m_tcResponse, sizeof(NowWork->m_tcResponse));

					// 1. Body �����
					swprintf_s(Body, _Mycountof(Body), L"{\"accountno\" : %lld, \"playcount\" : \"%d\"}", NowWork->AccountNo, NowWork->m_iCount);

					// 2. HTTP ���
					int TryCount = 5;
					while (m_HTTP_Post.HTTP_ReqANDRes((TCHAR*)_T("update_contents.php"), Body, NowWork->m_tcResponse) == false)
					{
						TryCount--;

						if (TryCount == 0)
							gThis->m_Dump->Crash();

						Sleep(100);
					}

					// 3. Json������ �Ľ��ϱ� (UTF-16)
					GenericDocument<UTF16<>> Doc;
					Doc.Parse(NowWork->m_tcResponse);

					int iResult = Doc[_T("result")].GetInt();

					// 4. DB ��û ��� Ȯ��
					// ����� 1�� �ƴ϶�� Crash.
					// Write�� ������ �����Ѵٴ� ����
					if (iResult != 1)
						g_BattleServer_Room_Dump->Crash();

					// 5. AccountNo �޾Ƶα�.
					// �Ʒ����� ���ҽ�Ų��.
					TempAccountNo = NowWork->AccountNo;
				}
				break;

				// ų ī��Ʈ ����
				case eu_DB_AFTER_TYPE::eu_KILL_UPDATE:
				{
					DB_WORK_CONTENT_UPDATE* NowWork = (DB_WORK_CONTENT_UPDATE*)pWork;

					// DB�� ���� ����
					TCHAR Body[1000] = { 0, };

					ZeroMemory(NowWork->m_tcResponse, sizeof(NowWork->m_tcResponse));

					// 1. Body �����
					swprintf_s(Body, _Mycountof(Body), L"{\"accountno\" : %lld, \"kill\" : \"%d\"}", NowWork->AccountNo, NowWork->m_iCount);

					// 2. HTTP ���
					int TryCount = 5;
					while (m_HTTP_Post.HTTP_ReqANDRes((TCHAR*)_T("update_contents.php"), Body, NowWork->m_tcResponse) == false)
					{
						TryCount--;

						if (TryCount == 0)
							gThis->m_Dump->Crash();

						Sleep(100);
					}

					// 3. Json������ �Ľ��ϱ� (UTF-16)
					GenericDocument<UTF16<>> Doc;
					Doc.Parse(NowWork->m_tcResponse);

					int iResult = Doc[_T("result")].GetInt();

					// 4. DB ��û ��� Ȯ��
					// ����� 1�� �ƴ϶�� Crash.
					// Write�� ������ �����Ѵٴ� ����
					if (iResult != 1)
						g_BattleServer_Room_Dump->Crash();

					// 5. AccountNo �޾Ƶα�.
					// �Ʒ����� ���ҽ�Ų��.
					TempAccountNo = NowWork->AccountNo;

				}
				break;

				// ��� ī��Ʈ, �÷��� Ÿ�� ����
				case eu_DB_AFTER_TYPE::eu_DIE_UPDATE:
				{
					DB_WORK_CONTENT_UPDATE_2* NowWork = (DB_WORK_CONTENT_UPDATE_2*)pWork;

					// DB�� ���� ����
					TCHAR Body[1000] = { 0, };

					ZeroMemory(NowWork->m_tcResponse, sizeof(NowWork->m_tcResponse));

					// 1. Body �����
					swprintf_s(Body, _Mycountof(Body), L"{\"accountno\" : %lld, \"die\" : \"%d\", \"playtime\" : \"%d\"}", NowWork->AccountNo, NowWork->m_iCount1, NowWork->m_iCount2);

					// 2. HTTP ���
					int TryCount = 5;
					while (m_HTTP_Post.HTTP_ReqANDRes((TCHAR*)_T("update_contents.php"), Body, NowWork->m_tcResponse) == false)
					{
						TryCount--;

						if (TryCount == 0)
							gThis->m_Dump->Crash();

						Sleep(100);
					}

					// 3. Json������ �Ľ��ϱ� (UTF-16)
					GenericDocument<UTF16<>> Doc;
					Doc.Parse(NowWork->m_tcResponse);

					int iResult = Doc[_T("result")].GetInt();

					// 4. DB ��û ��� Ȯ��
					// ����� 1�� �ƴ϶�� Crash.
					// Write�� ������ �����Ѵٴ� ����
					if (iResult != 1)
						g_BattleServer_Room_Dump->Crash();

					// 5. AccountNo �޾Ƶα�.
					// �Ʒ����� ���ҽ�Ų��.
					TempAccountNo = NowWork->AccountNo;

				}
				break;

				// �¸� ī��Ʈ, �÷���Ÿ�� ����
				case eu_DB_AFTER_TYPE::eu_WIN_UPDATE:
				{
					DB_WORK_CONTENT_UPDATE_2* NowWork = (DB_WORK_CONTENT_UPDATE_2*)pWork;

					// DB�� ���� ����
					TCHAR Body[1000] = { 0, };

					ZeroMemory(NowWork->m_tcResponse, sizeof(NowWork->m_tcResponse));

					// 1. Body �����
					swprintf_s(Body, _Mycountof(Body), L"{\"accountno\" : %lld, \"win\" : \"%d\", \"playtime\" : \"%d\"}", NowWork->AccountNo, NowWork->m_iCount1, NowWork->m_iCount2);

					// 2. HTTP ���
					int TryCount = 5;
					while (m_HTTP_Post.HTTP_ReqANDRes((TCHAR*)_T("update_contents.php"), Body, NowWork->m_tcResponse) == false)
					{
						TryCount--;

						if (TryCount == 0)
							gThis->m_Dump->Crash();

						Sleep(100);
					}

					// 3. Json������ �Ľ��ϱ� (UTF-16)
					GenericDocument<UTF16<>> Doc;
					Doc.Parse(NowWork->m_tcResponse);

					int iResult = Doc[_T("result")].GetInt();

					// 4. DB ��û ��� Ȯ��
					// ����� 1�� �ƴ϶�� Crash.
					// Write�� ������ �����Ѵٴ� ����
					if (iResult != 1)
						g_BattleServer_Room_Dump->Crash();

					// 5. AccountNo �޾Ƶα�.
					// �Ʒ����� ���ҽ�Ų��.
					TempAccountNo = NowWork->AccountNo;
				}
				break;

				default:
					gThis->m_Dump->Crash();
				}						


				// 3. DBWrite TPS ����
				InterlockedIncrement(TempDBWriteTPS);

				// 4. DBWrite ī��Ʈ1 ����
				//gThis->m_pBattleServer->MinDBWriteCountFunc(TempAccountNo);

				// 5. DB_WORK ��ȯ
				pDBWorkPool->Free(pWork);

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

		InterlockedIncrement(&m_lDBWriteCountTPS);
	}

	// Battle���� this�� �޾Ƶд�.
	void shDB_Communicate::ParentSet(CBattleServer_Room* Parent)
	{
		m_pBattleServer = Parent;
	}




	// ---------------
	// �����ڿ� �Ҹ���
	// ---------------

	// ������
	shDB_Communicate::shDB_Communicate()
	{
		m_lDBWriteTPS = 0;
		m_lDBWriteCountTPS = 0;

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

		// Write �����忡�� �Ͻ�Ű��� ť �����Ҵ�.
		m_pDB_Wirte_Start_Queue = new CNormalQueue<DB_WORK*>();

		// DB_Read�� ����� �Ϸ���Ʈ ����
		// 30���� ������ ����, 2���� ������ Ȱ��ȭ
		int Create = 30;
		int Active = 2;

		m_hDB_Read = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, NULL, Active);
		if (m_hDB_Read == NULL)
			m_Dump->Crash();

		// DB_Read�� ��Ŀ ������ ����
		m_hDB_Read_Thread = new HANDLE[Create];
		for (int i = 0; i < Create; ++i)
		{
			m_hDB_Read_Thread[i] = (HANDLE)_beginthreadex(0, 0, DB_ReadThread, this, 0, 0);
			if (m_hDB_Read_Thread[i] == NULL)
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
		for (int i = 0; i < 10; ++i)
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


		// 6. ���� ��������

		// ��Ŀ ������ �ڵ� ��������
		delete[] m_hDB_Read_Thread;

		// DB_WORK �޸�Ǯ ��������
		delete m_pDB_Work_Pool;

		// Read �Ϸ� ������ ť ��������
		delete m_pDB_ReadComplete_Queue;

		// Write �����忡�� �Ͻ�Ű��� ť ��������
		delete m_pDB_Wirte_Start_Queue;
	}
}

// ------------------
// CGameSession�� �Լ�
// (CBattleServer_Room�� �̳�Ŭ����)
// ------------------
namespace Library_Jingyu
{
	// -----------------------
	// �����ڿ� �Ҹ���
	// -----------------------

	// ������
	CBattleServer_Room::CGameSession::CGameSession()
		:CMMOServer::cSession()
	{
		// ClientKey �ʱⰪ ����
		m_int64ClientKey = -1;

		// �ڷᱸ���� �� �÷���
		m_bStructFlag = false;

		// �α��� ��Ŷ �Ϸ� ó��
		m_bLoginFlag = false;

		// �� ����� ���� üũ �÷���
		m_bLogoutFlag = false;

		// �������� �� �ʱ� ��ȣ
		m_iRoomNo = -1;

		// �α��� HTTP ��û Ƚ��
		m_lLoginHTTPCount = 0;

		// ���� ����
		m_bAliveFlag = false;	

		// ������ ���� ���� ���� Ȯ��
		m_bLastDBWriteFlag = false;

		// ���ʴ� �α׾ƿ� ����
		m_euModeType = eu_PLATER_MODE::LOG_OUT;

		m_euDebugMode = eu_USER_TYPE_DEBUG::LOGOUT;
	}

	// �Ҹ���
	CBattleServer_Room::CGameSession::~CGameSession()
	{
		// �Ұ� ����
	}



	// -----------------------
	// �ܺο��� ȣ�� ������ �Լ�
	// (����)
	// -----------------------

	// ���� ���� ����
	//
	// Parameter : ����
	// return : ���� ������ �� true, ��� ������ �� false
	bool CBattleServer_Room::CGameSession::GetAliveState()
	{
		return m_bAliveFlag;
	}


	// -----------------------
	// �ܺο��� ȣ�� ������ �Լ�
	// (����)
	// -----------------------	

	// ĳ���� ���� �� ���õǴ� ��
	//
	// Parameter : ������ X, Y��ǥ, ĳ���� ������ �ð�(DWORD)
	// return : ���� 
	void CBattleServer_Room::CGameSession::StartSet(float PosX, float PosY, DWORD NowTime)
	{
		// X,Y ��ǥ
		m_fPosX = PosX;
		m_fPosY = PosY;

		// ù HP
		m_iHP = g_Data_HP;

		// �Ѿ� ��
		m_iBullet = g_Data_Cartridge_Bullet;

		// źâ ��. ���ʴ� 0
		m_iCartridge = 0;

		// ��� ��. ���ʴ� 0
		m_iHelmetCount = 0;

		// ���� ���� �ð�.
		// �и� ������� ����.
		m_dwGameStartTime = NowTime;
	}

	// ���� ���� ����
	//
	// Parameter : ������ ���� ����(true�� ����, false�� ���)
	// return : ����
	void CBattleServer_Room::CGameSession::AliveSet(bool Flag)
	{
		m_bAliveFlag = Flag;
	}

	// ���� ������ ó��
	// !! HP�� 0�� �Ǹ� �ڵ����� ������°� �ȴ� !!
	//
	// Parameter : ������ ���� ������
	// return : ���� �� ���� HP
	int CBattleServer_Room::CGameSession::Damage(int Damage)
	{
		// ������ HP����
		int HP = m_iHP - Damage;
		if (HP < 0)
			HP = 0;

		m_iHP = HP;

		// ���� �� �������� 0�̶�� ���� ����� ��
		if (HP == 0)
			m_bAliveFlag = false;		

		// ���� HP ����
		return HP;
	}




	// -----------------------
	// ���� ����
	// -----------------------	

	// ������ �¸� ī��Ʈ 1 ����
	// !! �¸� ī��Ʈ�� �÷��� �ð��� ���� ���� !!
	// !! ���ο����� �¸�ī��Ʈ ����, �÷���Ÿ�� ���� ��, DB�� ������� �Ѵ�. !!
	// 
	// Parameter : ����
	// return : ����
	void CBattleServer_Room::CGameSession::Record_Win_Add()
	{
		// ���� ī��Ʈ ��, �¸� ī��Ʈ ����
		++m_iRecord_Win;

		// �¸� ī��Ʈ DB�� ����.
		// �¸��ڴ�, ���� �÷��� Ÿ�� ���� ����. ���⼭ ����
		m_iRecord_PlayTime = m_iRecord_PlayTime + ((timeGetTime() - m_dwGameStartTime) / 1000);

		DB_WORK_CONTENT_UPDATE_2* WriteWork = (DB_WORK_CONTENT_UPDATE_2*)m_pParent->m_shDB_Communicate.m_pDB_Work_Pool->Alloc();

		WriteWork->m_wWorkType = eu_DB_AFTER_TYPE::eu_WIN_UPDATE;
		WriteWork->m_iCount1 = m_iRecord_Win;
		WriteWork->m_iCount2 = m_iRecord_PlayTime;

		WriteWork->AccountNo = m_Int64AccountNo;

		// Write �ϱ� ����, DBWriteī��Ʈ �÷�����.
		//m_pParent->AddDBWriteCountFunc(m_Int64AccountNo);

		// DBWrite �õ�
		m_pParent->m_shDB_Communicate.DBWriteFunc((DB_WORK*)WriteWork);
	}
	
	// ������ ��� ī��Ʈ 1 ����
	// !! ��� ī��Ʈ�� �÷��� �ð��� ���� ���� !!
	// !! ���ο����� ��� ī��Ʈ ����, �÷���Ÿ�� ���� ��, DB�� ������� �Ѵ�. !!
	// 
	// Parameter : ����
	// return : ����
	void  CBattleServer_Room::CGameSession::Recored_Die_Add()
	{
		// Dieī��Ʈ ����.
		++m_iRecord_Die;

		// �÷��� Ÿ�� ����
		m_iRecord_PlayTime = m_iRecord_PlayTime + ((timeGetTime() - m_dwGameStartTime) / 1000);
		
		// DBWrite ����ü ���� (Dieī��Ʈ + �÷��� Ÿ��)
		DB_WORK_CONTENT_UPDATE_2* DieWrite = (DB_WORK_CONTENT_UPDATE_2*)m_pParent->m_shDB_Communicate.m_pDB_Work_Pool->Alloc();

		DieWrite->m_wWorkType = eu_DB_AFTER_TYPE::eu_DIE_UPDATE;
		DieWrite->m_iCount1 = m_iRecord_Die;
		DieWrite->m_iCount2 = m_iRecord_PlayTime;

		DieWrite->AccountNo = m_Int64AccountNo;

		// ��û�ϱ�
		//m_pParent->AddDBWriteCountFunc(m_Int64AccountNo);
		m_pParent->m_shDB_Communicate.DBWriteFunc((DB_WORK*)DieWrite);
	}

	// ������ ų ī��Ʈ 1 ����
	// !! ���ο��� ų ī��Ʈ ���� ��, DB�� ������� �Ѵ� !!
	//
	// Parameter : ����
	// return : ����
	void CBattleServer_Room::CGameSession::Record_Kill_Add()
	{
		// ų ī��Ʈ ����
		++m_iRecord_Kill;

		// DBWrite ����ü ����
		DB_WORK_CONTENT_UPDATE* KillWrite = (DB_WORK_CONTENT_UPDATE*)m_pParent->m_shDB_Communicate.m_pDB_Work_Pool->Alloc();

		KillWrite->m_wWorkType = eu_DB_AFTER_TYPE::eu_KILL_UPDATE;
		KillWrite->m_iCount = m_iRecord_Kill;

		KillWrite->AccountNo = m_Int64AccountNo;

		// ��û�ϱ�
		//m_pParent->AddDBWriteCountFunc(m_Int64AccountNo);
		m_pParent->m_shDB_Communicate.DBWriteFunc((DB_WORK*)KillWrite);
	}

	// ������ �÷��� Ƚ�� 1 ����
	// !! ���ο��� �÷��� Ƚ�� ���� ��, DB�� ������� �Ѵ� !!
	//
	// Parameter : ����
	// return : ����
	void CBattleServer_Room::CGameSession::Record_PlayCount_Add()
	{
		// ���� ��, �÷��� Ƚ�� ����. �׸��� DB�� ����
		++m_iRecord_PlayCount;

		DB_WORK_CONTENT_UPDATE* CountWrite = (DB_WORK_CONTENT_UPDATE*)m_pParent->m_shDB_Communicate.m_pDB_Work_Pool->Alloc();

		CountWrite->m_wWorkType = eu_DB_AFTER_TYPE::eu_PLAYCOUNT_UPDATE;
		CountWrite->m_iCount = m_iRecord_PlayCount;

		CountWrite->AccountNo = m_Int64AccountNo;

		// Write �ϱ� ����, WriteCount ����.
		//m_pParent->AddDBWriteCountFunc(m_Int64AccountNo);

		// DBWrite
		m_pParent->m_shDB_Communicate.DBWriteFunc((DB_WORK*)CountWrite);
	}




	// -----------------
	// �����Լ�
	// -----------------

	// --------------- AUTH ���� �Լ�

	// ������ Auth ���� �����
	//
	// Parameter : ����
	// return : ����
	void CBattleServer_Room::CGameSession::OnAuth_ClientJoin()
	{
		// ������ ������ ��� ����
		if (m_euModeType != LOG_OUT)
			g_BattleServer_Room_Dump->Crash();

		m_euModeType = eu_PLATER_MODE::AUTH;

		m_euDebugMode = eu_USER_TYPE_DEBUG::CONNECT;
	}

	// ������ Auth ��忡�� ����
	//
	// Parameter : Game���� ����Ȱ����� �˷��ִ� Flag. ����Ʈ false(������� ����)
	// return : ����
	void CBattleServer_Room::CGameSession::OnAuth_ClientLeave(bool bGame)
	{
		// ��� ������ ���
		if (bGame == true)
		{
			// ��� ����
			if (m_euModeType != eu_PLATER_MODE::AUTH)
				g_BattleServer_Room_Dump->Crash();

			m_euModeType = eu_PLATER_MODE::GAME;

			m_euDebugMode = eu_USER_TYPE_DEBUG::AUTO_TO_GAME;
		}

		// ���� ���� ������ ���
		else
		{
			// ��� ����
			if (m_euModeType != eu_PLATER_MODE::AUTH)
				g_BattleServer_Room_Dump->Crash();

			m_euModeType = eu_PLATER_MODE::LOG_OUT;
			m_euDebugMode = eu_USER_TYPE_DEBUG::LOGOUT;

			// ClientKey �ʱⰪ���� ����.
			// !! �̰� ���ϸ�, Release�Ǵ� �߿� HTTP������ �� ��� !!
			// !! Auth_Update���� �̹� ����ť�� ��� ������ �������� �� SendPacket ���ɼ� !!
			// !! �� ���, ���� ������ �������� Send�� �� �� ���� !!
			UINT64 TempClientKey = m_int64ClientKey;
			m_int64ClientKey = -1;

			// �ش� ������ �ִ� �� �ȿ��� ���� ����
			// -1�� �ƴϸ� �뿡 �ִ°�.
			if (m_iRoomNo != -1)
			{
				AcquireSRWLockShared(&m_pParent->m_Room_Umap_srwl);		// ----- Room �ڷᱸ�� Shard �� 

				// 1. �� �˻�
				auto FindRoom = m_pParent->m_Room_Umap.find(m_iRoomNo);

				// ���� ������ Crash
				if (FindRoom == m_pParent->m_Room_Umap.end())
					g_BattleServer_Room_Dump->Crash();

				stRoom* NowRoom = FindRoom->second;

				// �� ���°� Play�� Crash. ���� �ȵ�
				// Auth��忡�� ����Ǵ� ������ �ִ� ����, ������ ���/�غ� ���̾�� ��				
				if (NowRoom->m_iRoomState == eu_ROOM_STATE::PLAY_ROOM)
					g_BattleServer_Room_Dump->Crash();	

				// ������� ���� Wait Ȥ�� Ready������ ��.
				// Auth �����常 ���� ����. �� Ǯ� ����
				ReleaseSRWLockShared(&m_pParent->m_Room_Umap_srwl);		// ----- Room �ڷᱸ�� Shard ��� 
				


				// 2. ���� �� �ȿ� �ִ� ���� �� ����
				--NowRoom->m_iJoinUserCount;

				// ���� ��, �� ���� ���� ���� 0���� ������ ���� �ȵ�.
				if (NowRoom->m_iJoinUserCount < 0)				
					g_BattleServer_Room_Dump->Crash();


				// 3. �� ���� �ڷᱸ������ ���� ����
				if (NowRoom->Erase(this) == false)
					g_BattleServer_Room_Dump->Crash();			


				// 4. �� ���� ��� ��������, �濡�� ������ �����ٰ� �˷���
				// BroadCast�ص� �ȴ�. ������ �� �ȿ� ���� ���� ������ ����.
				CProtocolBuff_Net* SendBuff = CProtocolBuff_Net::Alloc();

				WORD Type = en_PACKET_CS_GAME_RES_REMOVE_USER;

				SendBuff->PutData((char*)&Type, 2);
				SendBuff->PutData((char*)&m_iRoomNo, 4);
				SendBuff->PutData((char*)&m_Int64AccountNo, 8);

				// ���⼭, false�� ���ϵ� �� ����. (�� ���� �ڷᱸ���� ������ 0��)
				// ������ ���¿��� ��� ������ ���� �� �ֱ� ������ ������ �Ⱥ���.
				// ������ return �ȹ���
				NowRoom->SendPacket_BroadCast(SendBuff);


				// 5. ������ �������� �ش� ���� �����ٰ� ��Ŷ ������� ��.
				// Ready������ ������, �̹� �����Ϳ��� ������� ������ ������ �ȵ�.
				if(NowRoom->m_iRoomState == eu_ROOM_STATE::WAIT_ROOM)
					m_pParent->m_Master_LanClient->Packet_RoomLeave_Req(m_iRoomNo, TempClientKey);

				// 6. �� Ű -1�� �ʱ�ȭ
				m_iRoomNo = -1;
			}			

			// 7. m_bStructFlag�� true���, �ڷᱸ���� �� ����
			// �ڷᱸ������ �����Ѵ�.
			if (m_bStructFlag == true)
			{
				if (m_pParent->EraseAccountNoFunc(m_Int64AccountNo) == false)
					g_BattleServer_Room_Dump->Crash();

				// DBWrite���� ���� �õ�.
				// m_bStructFlag�� true���, DBWrite Ƚ�� �ڷᱸ���� ���� ���� ����.
				//if (m_pParent->MinDBWriteCountFunc(m_Int64AccountNo) == false)
					//g_BattleServer_Room_Dump->Crash();

				m_bStructFlag = false;
			}
		}

	}

	// Auth ����� �������� packet�� ��
	//
	// Parameter : ��Ŷ (CProtocolBuff_Net*)
	// return : ����
	void CBattleServer_Room::CGameSession::OnAuth_Packet(CProtocolBuff_Net* Packet)
	{
		// ��Ŷ ó��
		try
		{
			// 1. Ÿ�� ����
			WORD Type;
			Packet->GetData((char*)&Type, 2);

			// 2. Ÿ�Կ� ���� �б� ó��
			switch (Type)
			{
				// �α��� ��û
			case en_PACKET_CS_GAME_REQ_LOGIN:
				Auth_LoginPacket(Packet);
				break;

				// �� ���� ��û
			case en_PACKET_CS_GAME_REQ_ENTER_ROOM:
				Auth_RoomEnterPacket(Packet);
				break;

				// ��Ʈ��Ʈ
			case en_PACKET_CS_GAME_REQ_HEARTBEAT:
				break;

				// �� �ܿ��� ��������.
			default:
				throw CException(_T("OnAuth_Packet() --> Type Error!!"));
				break;
			}
		}
		catch (CException& exc)
		{
			// �α� ��� (�α� ���� : ����)
			g_BattleServer_RoomLog->LogSave(false, L"CBattleServer_Room", CSystemLog::en_LogLevel::LEVEL_ERROR, L"%s",
				(TCHAR*)exc.GetExceptionText());

			// ���� ���� ��û
			Disconnect();
		}
	}

	// Auth ����� ������ ��Ʈ��Ʈ�� ����
	//
	// Parameter : ����
	// return : ����
	void CBattleServer_Room::CGameSession::OnAuth_HeartBeat(DWORD DelayTime)
	{
		// ���� �α� ���
		// �α� ��� (�α� ���� : ����)
		g_BattleServer_RoomLog->LogSave(false, L"CBattleServer_Room", CSystemLog::en_LogLevel::LEVEL_ERROR,
			L"Auth HeartBeat!! AccoutnNo : %lld, Time : %d, State : %d", m_Int64AccountNo, DelayTime, m_euDebugMode);
	}




	// --------------- GAME ���� �Լ�

	// ������ Game���� �����
	//
	// Parameter : ����
	// return : ����
	void CBattleServer_Room::CGameSession::OnGame_ClientJoin()
	{
		// �α��� ���� üũ
		if (m_bLoginFlag == false)
			g_BattleServer_Room_Dump->Crash();

		AcquireSRWLockShared(&m_pParent->m_Room_Umap_srwl);		// ----- Room Umap Shared ��

		// 1. ������ �ִ� �� �˾ƿ���
		auto FindRoom = m_pParent->m_Room_Umap.find(m_iRoomNo);

		// ������ ���� �ȵ�.
		if (FindRoom == m_pParent->m_Room_Umap.end())
			g_BattleServer_Room_Dump->Crash();

		stRoom* NowRoom = FindRoom->second;

		// �� ��尡 Play�� �ƴϸ� ���� �ȵ�.
		if(NowRoom->m_iRoomState != eu_ROOM_STATE::PLAY_ROOM)
			g_BattleServer_Room_Dump->Crash();

		// �� Ǭ��
		ReleaseSRWLockShared(&m_pParent->m_Room_Umap_srwl);		// ----- Room Umap Shared ���		

		// 2. ���� ���� ��ȯ�� ���� ī��Ʈ ����.

		// ���� ��, �̹� JoinUser�� ���ٸ� ���� �ȵ�. �� ���� �濡 �� ���°�. ũ����
		if(NowRoom->m_iGameModeUser == NowRoom->m_iJoinUserCount)
			g_BattleServer_Room_Dump->Crash();

		++NowRoom->m_iGameModeUser;

		m_euDebugMode = eu_USER_TYPE_DEBUG::INGAME;

		// 3. ���� ��, JoinUser�� �������ٸ� �� ���� ��� �������� [�� ĳ���� ����] �� [�ٸ� ���� ĳ���� ����]�� ������.
		// OnGame_ClientJoin ���� ȣ���ϴ°� �ƴ϶�, ��� �����ߴ��� Ȯ�� �� ������. ���ÿ� ĳ���� ������ �ϱ� ���ؼ�.
		if (NowRoom->m_iGameModeUser == NowRoom->m_iJoinUserCount)
			NowRoom->CreateCharacter();


		// �÷��� Ƚ�� ���� ����
		Record_PlayCount_Add();	
	}

	// ������ Game��忡�� ����
	//
	// Parameter : ����
	// return : ����
	void CBattleServer_Room::CGameSession::OnGame_ClientLeave()
	{
		// ��� ����
		if (m_euModeType != eu_PLATER_MODE::GAME)
			g_BattleServer_Room_Dump->Crash();

		m_euModeType = eu_PLATER_MODE::LOG_OUT;
		m_euDebugMode = eu_USER_TYPE_DEBUG::LOGOUT;

		// ClientKey �ʱⰪ���� ����.
		// !! �̰� ���ϸ�, Release�Ǵ� �߿� HTTP������ �� ��� !!
		// !! Game_Update���� �̹� ����ť�� ��� ������ �������� �� SendPacket ���ɼ� !!
		// !! �� ���, ���� ������ �������� Send�� �� �� ���� !!
		m_int64ClientKey = -1;


		// �ش� ������ �ִ� �� �ȿ��� ���� ����
		// -1�� ���� ���� �ȵ�. ������ �濡 �־�� ��
		if(m_iRoomNo == -1)
			g_BattleServer_Room_Dump->Crash();


		AcquireSRWLockShared(&m_pParent->m_Room_Umap_srwl);		// ----- Room �ڷᱸ�� Shard �� 

		// 1. �� �˻�
		auto FindRoom = m_pParent->m_Room_Umap.find(m_iRoomNo);

		// ���� ������ Crash
		if (FindRoom == m_pParent->m_Room_Umap.end())
			g_BattleServer_Room_Dump->Crash();

		stRoom* NowRoom = FindRoom->second;

		// �� ���°� Play�� �ƴϸ� Crash. ���� �ȵ�
		// Game��忡�� ����Ǵ� ������ �ִ� ����, ������ �÷��� ���̾�� ��
		if (NowRoom->m_iRoomState != eu_ROOM_STATE::PLAY_ROOM)
			g_BattleServer_Room_Dump->Crash();

		// ������� ���� �� ��尡 PLAY_ROOM Ȯ��.
		// ��, Game������ �����ϴ� �� Ȯ��. �� Ǭ��.
		// �� Ǯ���µ� ���� ������ ���ɼ��� ���� ����. 
		// �� ������ Game �����尡 �ϰ�, �ش� �Լ��� Game �����忡�� ȣ��ȴ�. 
		ReleaseSRWLockShared(&m_pParent->m_Room_Umap_srwl);		// ----- Room �ڷᱸ�� Shard ��� 


		// 2. ���� �� �ȿ� �ִ� ���� �� ����
		--NowRoom->m_iJoinUserCount;		

		// ���� ��, �� ���� ���� ���� 0���� ������ ���� �ȵ�. Crash
		if (NowRoom->m_iJoinUserCount < 0)
			g_BattleServer_Room_Dump->Crash();


		// 3. ������ ������ ������ �������� ���, ������ ���� ��, Game��� ���� �� ����.
		// ������ ���� ���� �¸����θ� �Ǵ��ؾ� �ϱ� ������ 
		// ������ ���ӿ��� �����ų� HP�� 0�̵Ǿ� ����� ��� ���ҽ��Ѿ� ��
		if (m_bAliveFlag == true)
		{
			--NowRoom->m_iAliveUserCount;
			--NowRoom->m_iGameModeUser;
			
			// ���� ��, 0���� ������ ���� �ȵ�.
			if (NowRoom->m_iAliveUserCount < 0 || NowRoom->m_iGameModeUser < 0)
				g_BattleServer_Room_Dump->Crash();

			m_bAliveFlag = false;
		}



		// 4. �� ���� �ڷᱸ������ ���� ����
		if (NowRoom->Erase(this) == false)
			g_BattleServer_Room_Dump->Crash();



		// 5. �� ���� ��� ��������, �濡�� ������ �����ٰ� �˷���
		// BroadCast�ص� �ȴ�. ������ �� �ȿ� ���� ���� ������ ����.
		CProtocolBuff_Net* SendBuff = CProtocolBuff_Net::Alloc();

		WORD Type = en_PACKET_CS_GAME_RES_LEAVE_USER;

		SendBuff->PutData((char*)&Type, 2);
		SendBuff->PutData((char*)&m_iRoomNo, 4);
		SendBuff->PutData((char*)&m_Int64AccountNo, 8);

		// ���⼭, false�� ���ϵ� �� ����. (�� ���� �ڷᱸ���� ������ 0��)
		// ������ ���¿��� ��� ������ ���� �� �ֱ� ������ ������ �Ⱥ���.
		// ������ return �ȹ���
		NowRoom->SendPacket_BroadCast(SendBuff);

		m_iRoomNo = -1;



		// 6. �÷��� Ÿ�� ������ ���� ���ߴٸ�, ������ ����������� �ʾҴµ� ������ ����.
		// ����� ������ �Ǵ��Ѵ�.
		if (m_bLastDBWriteFlag == false)
		{
			// ������ ���ī��Ʈ 1 ����
			Recored_Die_Add();		
		}

		// 7. m_bStructFlag�� true���, �ڷᱸ���� �� ����
		// �ڷᱸ������ �����Ѵ�.
		if (m_bStructFlag == true)
		{
			if (m_pParent->EraseAccountNoFunc(m_Int64AccountNo) == false)
				g_BattleServer_Room_Dump->Crash();

			// DBWrite���� ���� �õ�.
			// m_bStructFlag�� true���, DBWrite Ƚ�� �ڷᱸ���� ���� ���� ����.
			//if (m_pParent->MinDBWriteCountFunc(m_Int64AccountNo) == false)
				//g_BattleServer_Room_Dump->Crash();

			m_bStructFlag = false;
		}
	}

	// Game ����� �������� packet�� ��
	//
	// Parameter : ��Ŷ (CProtocolBuff_Net*)
	// return : ����
	void CBattleServer_Room::CGameSession::OnGame_Packet(CProtocolBuff_Net* Packet)
	{
		// Ÿ�Կ� ���� ��Ŷ ó��
		try
		{
			// 1. Ÿ�� ����
			WORD Type;
			Packet->GetData((char*)&Type, 2);

			// 2. Ÿ�Կ� ���� �б⹮ ó��
			switch (Type)
			{	
				// ĳ���� �̵�
			case en_PACKET_CS_GAME_REQ_MOVE_PLAYER:
				Game_MovePacket(Packet);
				break;

				// Fire1 �߻� (�� �߻�)
			case en_PACKET_CS_GAME_REQ_FIRE1:
				Game_Frie_1_Packet(Packet);
				break;

				// HitDamage
			case en_PACKET_CS_GAME_REQ_HIT_DAMAGE:
				Game_HitDamage_Packet(Packet);
				break;

				// Fire 2 ���� (������)
			case en_PACKET_CS_GAME_REQ_FIRE2:
				Game_Fire_2_Packet(Packet);
				break;

				// KickDamage
			case en_PACKET_CS_GAME_REQ_KICK_DAMAGE:
				Game_KickDamage_Packet(Packet);
				break;

				// ������
			case en_PACKET_CS_GAME_REQ_RELOAD:
				Game_Reload_Packet(Packet);
				break;

				// �޵�Ŷ ������ ȹ��
			case en_PACKET_CS_GAME_REQ_MEDKIT_GET:
				Game_GetItem_Packet(Packet, MEDKIT);
				break;

				// źâ ������ ȹ��
			case en_PACKET_CS_GAME_REQ_CARTRIDGE_GET:
				Game_GetItem_Packet(Packet, CARTRIDGE);
				break;

				// ��� ������ ȹ��
			case en_PACKET_CS_GAME_REQ_HELMET_GET:
				Game_GetItem_Packet(Packet, HELMET);
				break;				

				// ĳ���� HitPoint ����
			case en_PACKET_CS_GAME_REQ_HIT_POINT:
				Game_HitPointPacket(Packet);
				break;

				// ��Ʈ��Ʈ
			case en_PACKET_CS_GAME_REQ_HEARTBEAT:
				break;

				// �� �ܿ��� ���� ����
			default:
				throw CException(_T("OnGame_Packet() --> Type Error!!"));
				break;
			}

		}
		catch (CException& exc)
		{
			// �α� ��� (�α� ���� : ����)
			g_BattleServer_RoomLog->LogSave(false, L"CBattleServer_Room", CSystemLog::en_LogLevel::LEVEL_ERROR, L"%s",
				(TCHAR*)exc.GetExceptionText());

			// ���� ���� ��û
			Disconnect();
		}
	}

	// Game ����� ������ ��Ʈ��Ʈ�� ����
	//
	// Parameter : ����
	// return : ����
	void CBattleServer_Room::CGameSession::OnGame_HeartBeat(DWORD DelayTime)
	{
		// ���� �α� ���
		// �α� ��� (�α� ���� : ����)
		g_BattleServer_RoomLog->LogSave(false, L"CBattleServer_Room", CSystemLog::en_LogLevel::LEVEL_ERROR,
			L"Game HeartBeat!! AccoutnNo : %lld, Time : %d, State : %d", m_Int64AccountNo, DelayTime, m_euDebugMode);
	}



	// --------------- Release ���� �Լ�

	// Release�� ����.
	//
	// Parameter : ����
	// return : ����
	void CBattleServer_Room::CGameSession::OnGame_ClientRelease()
	{
		// ��尡 LOG_OUT�� �ƴϸ� ũ����
		if (m_euModeType != eu_PLATER_MODE::LOG_OUT)
			g_BattleServer_Room_Dump->Crash();
		
		m_bLoginFlag = false;	
		m_bLogoutFlag = false;
		m_lLoginHTTPCount = 0;		
		m_bLastDBWriteFlag = false;		
	}

	// GQCS���� 121���� �߻� �� ȣ��Ǵ� �Լ�
	//
	// Parameter : ����
	// return : ����
	void CBattleServer_Room::CGameSession::OnSemaphore()
	{
		// ���� �α� ���
		// �α� ��� (�α� ���� : ����)
		g_BattleServer_RoomLog->LogSave(false, L"CBattleServer_Room", CSystemLog::en_LogLevel::LEVEL_ERROR,
			L"Semaphore!! AccoutnNo : %lld", m_Int64AccountNo);
	}



	// -----------------
	// Auth��� ��Ŷ ó�� �Լ�
	// -----------------

	// �α��� ��û 
	//
	// Parameter : CProtocolBuff_Net*
	// return : ����
	void CBattleServer_Room::CGameSession::Auth_LoginPacket(CProtocolBuff_Net* Packet)
	{
		// 1. ���� ClientKey�� �ʱⰪ�� �ƴϸ� Crash
		if (m_int64ClientKey != -1)
			g_BattleServer_Room_Dump->Crash();

		// 2. AccountNo ������
		INT64 AccountNo;
		Packet->GetData((char*)&AccountNo, 8);

		// 3. ���� DB�� Write������
		/*
		if (m_pParent->InsertDBWriteCountFunc(AccountNo) == false)
		{
			InterlockedIncrement(&m_pParent->m_OverlapLoginCount_DB);

			// �ߺ� �α��� ��Ŷ ������
			CProtocolBuff_Net* SendBuff = CProtocolBuff_Net::Alloc();

			// Ÿ��
			WORD Type = en_PACKET_CS_GAME_RES_LOGIN;
			SendBuff->PutData((char*)&Type, 2);

			// AccountNo
			SendBuff->PutData((char*)&AccountNo, 8);

			// Result
			BYTE Result = 6;		// �ߺ� �α���
			SendBuff->PutData((char*)&Result, 1);

			// SendPacket. ������ ����
			SendPacket(SendBuff, TRUE);

			// �ߺ� �α��� ó���� ������ ���� ���� ��û
			m_pParent->DisconnectAccountNoFunc(AccountNo);

			return;
		}
		*/
		
		// 4. ������ ������ ��, ����Ű, AccountNo, Ŭ���̾�ƮŰ�� ����� ����
		Packet->GetData(m_cSessionKey, 64);
		char ConnectToken[32];
		UINT Ver_Code;

		Packet->GetData(ConnectToken, 32);
		Packet->GetData((char*)&Ver_Code, 4);

		INT64 ClinetKey;
		Packet->GetData((char*)&ClinetKey, 8);

		m_Int64AccountNo = AccountNo;
		m_int64ClientKey = ClinetKey;


		// 5. ��Ʋ���� ���� ��ū ��

		// �� �ɰ� Ȯ��.
		AcquireSRWLockShared(&m_pParent->m_ServerEnterToken_srwl);	// ----- shasred ��

		// "����" ��ū�� ���� ��
		if (memcmp(ConnectToken, m_pParent->m_cConnectToken_Now, 32) != 0)
		{
			// �ٸ��ٸ� "����" ��ū�� ��
			if (memcmp(ConnectToken, m_pParent->m_cConnectToken_Before, 32) != 0)
			{
				ReleaseSRWLockShared(&m_pParent->m_ServerEnterToken_srwl);	// ----- shasred ���

				// ���� �α� ���
				// �α� ��� (�α� ���� : ����)
				g_BattleServer_RoomLog->LogSave(false, L"CBattleServer_Room", CSystemLog::en_LogLevel::LEVEL_ERROR,
					L"Auth_LoginPacket_AUTH()--> BattleEnterToken Error!! AccoutnNo : %lld", m_Int64AccountNo);

				InterlockedIncrement(&m_pParent->m_lBattleEnterTokenError);

				// �׷��� �ٸ��ٸ� �̻��� ������ �Ǵ�.
				// ��Ʋ���� ���� ��ū�� �ٸ��ٴ� ��Ŷ ������.
				CProtocolBuff_Net* SendBuff = CProtocolBuff_Net::Alloc();

				WORD Type = en_PACKET_CS_GAME_RES_LOGIN;
				BYTE Result = 3;	// ����� ����Ű ������ ���� ���. ���� �����ɱ�?

				SendBuff->PutData((char*)&Type, 2);
				SendBuff->PutData((char*)&AccountNo, 8);
				SendBuff->PutData((char*)&Result, 1);

				// SendPacket
				SendPacket(SendBuff);

				return;
			}
		}
		
		ReleaseSRWLockShared(&m_pParent->m_ServerEnterToken_srwl);	// ----- shasred ���

		// 6. ���� �� 
		if (m_pParent->m_uiVer_Code != Ver_Code)
		{
			// ���� �α� ���
			// �α� ��� (�α� ���� : ����)
			g_BattleServer_RoomLog->LogSave(false, L"CBattleServer_Room", CSystemLog::en_LogLevel::LEVEL_ERROR,
				L"Auth_LoginPacket_AUTH()--> VerCode Error!! AccoutnNo : %lld, Code : %d", m_Int64AccountNo, Ver_Code);

			// ������ �ٸ���� Result 5(���� ����)�� ������.
			InterlockedIncrement(&m_pParent->m_lVerError);

			WORD Type = en_PACKET_CS_MATCH_RES_LOGIN;
			BYTE Result = 5;

			CProtocolBuff_Net* SendBuff = CProtocolBuff_Net::Alloc();

			SendBuff->PutData((char*)&Type, 2);
			SendBuff->PutData((char*)&AccountNo, 8);
			SendBuff->PutData((char*)&Result, 1);

			// SendPacket
			SendPacket(SendBuff);
			return;
		}
		
		// 7. AccountNo �ڷᱸ���� �߰�.	
		// �̹� ������(false ����) �ߺ� �α������� ó��
		// ���� �������Դ� ���� ��Ŷ, ���� ���� ������ DIsconnect.
		if (m_pParent->InsertAccountNoFunc(AccountNo, this) == false)
		{	
			InterlockedIncrement(&m_pParent->m_OverlapLoginCount);

			// ���� �α� ���
			// �α� ��� (�α� ���� : ����)
			g_BattleServer_RoomLog->LogSave(false, L"CBattleServer_Room", CSystemLog::en_LogLevel::LEVEL_ERROR,
				L"Auth_LoginPacket_AUTH()--> Overlapped Login!! AccoutnNo : %lld", m_Int64AccountNo);

			// �ߺ� �α��� ��Ŷ ������
			CProtocolBuff_Net* SendBuff = CProtocolBuff_Net::Alloc();

			// Ÿ��
			WORD Type = en_PACKET_CS_GAME_RES_LOGIN;
			SendBuff->PutData((char*)&Type, 2);

			// AccountNo
			SendBuff->PutData((char*)&AccountNo, 8);

			// Result
			BYTE Result = 6;		// �ߺ� �α���
			SendBuff->PutData((char*)&Result, 1);				

			// SendPacket. ������ ����
			SendPacket(SendBuff, TRUE);

			// �ߺ� �α��� ó���� ������ ���� ���� ��û
			m_pParent->DisconnectAccountNoFunc(AccountNo);

			return;
		}

		// �ڷᱸ���� �� �÷��� ����
		m_bStructFlag = true;

		m_euDebugMode = eu_USER_TYPE_DEBUG::LOGIN_SEND;

		// 8. �α��� ����ó���� ���� HTTP ���
		// ��� ��, �ش� ��Ŷ�� ���� ��ó���� Auth_Update���� �ѱ��.
		DB_WORK_LOGIN* Send_A = (DB_WORK_LOGIN*)m_pParent->m_shDB_Communicate.m_pDB_Work_Pool->Alloc();

		Send_A->m_wWorkType = eu_DB_AFTER_TYPE::eu_LOGIN_AUTH;
		Send_A->m_i64UniqueKey = ClinetKey;
		Send_A->pPointer = this;

		Send_A->AccountNo = AccountNo;

		// Select_Account.php ��û
		m_pParent->m_shDB_Communicate.DBReadFunc((DB_WORK*)Send_A, en_PHP_TYPE::SELECT_ACCOUNT);


		// 9. ���� ���� ������ ���� HTTP ���
		// ��� ��, �ش� ��Ŷ�� ���� ��ó���� Auth_Update���� �ѱ��.
		DB_WORK_LOGIN_CONTENTS* Send_B = (DB_WORK_LOGIN_CONTENTS*)m_pParent->m_shDB_Communicate.m_pDB_Work_Pool->Alloc();
	
		Send_B->m_wWorkType = eu_DB_AFTER_TYPE::eu_LOGIN_INFO;
		Send_B->m_i64UniqueKey = ClinetKey;
		Send_B->pPointer = this;

		Send_B->AccountNo = AccountNo;

		// Select_Contents.php ��û
		m_pParent->m_shDB_Communicate.DBReadFunc((DB_WORK*)Send_B, en_PHP_TYPE::SELECT_CONTENTS);
	}
	
	// �� ���� ��û
	// 
	// Parameter : CProtocolBuff_Net*
	// return : ����
	void CBattleServer_Room::CGameSession::Auth_RoomEnterPacket(CProtocolBuff_Net* Packet)
	{
		// �α��� ���� üũ
		if(m_bLoginFlag == false)
			g_BattleServer_Room_Dump->Crash();

		// 1. ������
		INT64 AccountNo;
		int RoomNo;
		char EnterToken[32];

		Packet->GetData((char*)&AccountNo, 8);
		Packet->GetData((char*)&RoomNo, 4);
		Packet->GetData(EnterToken, 32);		

		// 2. ���� üũ
		if (AccountNo != m_Int64AccountNo)
			g_BattleServer_Room_Dump->Crash();
		
		AcquireSRWLockShared(&m_pParent->m_Room_Umap_srwl);	// ----- Room umap Shared ��

		
		// 3. �����ϰ��� �ϴ� �� �˻�
		auto ret = m_pParent->m_Room_Umap.find(RoomNo);

		// 4. �� ���翩�� üũ
		if (ret == m_pParent->m_Room_Umap.end())
		{
			ReleaseSRWLockShared(&m_pParent->m_Room_Umap_srwl);	// ----- Room umap Shared ���

			// ���� �α� ���
			// �α� ��� (�α� ���� : ����)
			g_BattleServer_RoomLog->LogSave(false, L"CBattleServer_Room", CSystemLog::en_LogLevel::LEVEL_ERROR,
				L"Enter Room --> Not Find Room !! AccoutnNo : %lld, RoomNo : %d", m_Int64AccountNo, RoomNo);

			// ���� ������, ���� ����
			CProtocolBuff_Net* SendBuff = CProtocolBuff_Net::Alloc();

			WORD Type = en_PACKET_CS_GAME_RES_ENTER_ROOM;
			BYTE Result = 4;
			BYTE MaxUser = 0;	// �� ���� ��Ŷ���� �ǹ̰� ������.. �׳� �ƹ��ų� ����.

			SendBuff->PutData((char*)&Type, 2);
			SendBuff->PutData((char*)&AccountNo, 8);
			SendBuff->PutData((char*)&RoomNo, 4);
			SendBuff->PutData((char*)&MaxUser, 1);
			SendBuff->PutData((char*)&Result, 1);

			SendPacket(SendBuff);

			return;
		}

		stRoom* NowRoom = ret->second;

		// Wait������ ����, Auth�����忡���� ����.
		// ������, ������� ������ Auth�� ���� Ȯ��.
		// �� Ǯ� �����ϴ�.
		ReleaseSRWLockShared(&m_pParent->m_Room_Umap_srwl);	// ----- Room umap Shared ���

		// 5. ���� ������ �ƴ� ��� ���� 3 ����
		if (NowRoom->m_iRoomState != eu_ROOM_STATE::WAIT_ROOM)
		{
			// ���� �α� ���
		// �α� ��� (�α� ���� : ����)
			g_BattleServer_RoomLog->LogSave(false, L"CBattleServer_Room", CSystemLog::en_LogLevel::LEVEL_ERROR,
				L"Enter Room --> Not Wait Room !! AccoutnNo : %lld, RoomNo : %d", m_Int64AccountNo, RoomNo);

			// ������ �ƴϸ�, ���� ����
			CProtocolBuff_Net* SendBuff = CProtocolBuff_Net::Alloc();

			WORD Type = en_PACKET_CS_GAME_RES_ENTER_ROOM;
			BYTE Result = 3;
			BYTE MaxUser = 0;	// ���� �ƴ� ��Ŷ���� �ǹ̰� ������.. �׳� �ƹ��ų� ����.

			SendBuff->PutData((char*)&Type, 2);
			SendBuff->PutData((char*)&AccountNo, 8);
			SendBuff->PutData((char*)&RoomNo, 4);
			SendBuff->PutData((char*)&MaxUser, 1);
			SendBuff->PutData((char*)&Result, 1);

			SendPacket(SendBuff);

			return;
		}			
		
		// 6. �� �ο��� üũ
		if (NowRoom->m_iJoinUserCount == NowRoom->m_iMaxJoinCount)
		{
			BYTE MaxUser = NowRoom->m_iJoinUserCount;	

			// ���� �α� ���
			// �α� ��� (�α� ���� : ����)
			g_BattleServer_RoomLog->LogSave(false, L"CBattleServer_Room", CSystemLog::en_LogLevel::LEVEL_ERROR,
				L"Enter Room --> Full Room !! AccoutnNo : %lld, RoomNo : %d", m_Int64AccountNo, RoomNo);

			// �̹� �ִ� �ο������, ���� ����			
			CProtocolBuff_Net* SendBuff = CProtocolBuff_Net::Alloc();

			WORD Type = en_PACKET_CS_GAME_RES_ENTER_ROOM;
			BYTE Result = 5;

			SendBuff->PutData((char*)&Type, 2);
			SendBuff->PutData((char*)&AccountNo, 8);
			SendBuff->PutData((char*)&RoomNo, 4);
			SendBuff->PutData((char*)&MaxUser, 1);
			SendBuff->PutData((char*)&Result, 1);

			SendPacket(SendBuff);

			return;
		}

		// 7. �� ��ū üũ
		if (memcmp(EnterToken, NowRoom->m_cEnterToken, 32) != 0)
		{
			BYTE MaxUser = NowRoom->m_iJoinUserCount;

			// ���� �α� ���
			// �α� ��� (�α� ���� : ����)
			g_BattleServer_RoomLog->LogSave(false, L"CBattleServer_Room", CSystemLog::en_LogLevel::LEVEL_ERROR,
				L"Enter Room --> Room Token Error !! AccoutnNo : %lld, RoomNo : %d", m_Int64AccountNo, RoomNo);

			InterlockedIncrement(&m_pParent->m_lRoomEnterTokenError);		

			// ��ū�� �ٸ���, ���� ����
			CProtocolBuff_Net* SendBuff = CProtocolBuff_Net::Alloc();

			WORD Type = en_PACKET_CS_GAME_RES_ENTER_ROOM;
			BYTE Result = 2;

			SendBuff->PutData((char*)&Type, 2);
			SendBuff->PutData((char*)&AccountNo, 8);
			SendBuff->PutData((char*)&RoomNo, 4);
			SendBuff->PutData((char*)&MaxUser, 1);
			SendBuff->PutData((char*)&Result, 1);

			SendPacket(SendBuff);

			return;
		}


		// 8. ������� ���� ����
		// �뿡 �ο� �߰�
		++NowRoom->m_iJoinUserCount;
		NowRoom->Insert(this);

		// �÷��̾�� �� �Ҵ�
		m_iRoomNo = NowRoom->m_iRoomNo;

		m_euDebugMode = eu_USER_TYPE_DEBUG::ROOMENTER;

		// 9. �������� �� ���� ���� ������
		CProtocolBuff_Net* SendBuff = CProtocolBuff_Net::Alloc();

		WORD Type = en_PACKET_CS_GAME_RES_ENTER_ROOM;
		BYTE Result = 1;

		SendBuff->PutData((char*)&Type, 2);
		SendBuff->PutData((char*)&AccountNo, 8);
		SendBuff->PutData((char*)&RoomNo, 4);
		SendBuff->PutData((char*)&NowRoom->m_iMaxJoinCount, 1);
		SendBuff->PutData((char*)&Result, 1);

		SendPacket(SendBuff);


		// 10. �濡 �ִ� ��� �������� "���� �߰���" ��Ŷ ������
		NowRoom->Send_AddUser(this);	


		// 11. �� �ο����� Ǯ ���� �Ǿ��� ���
		if (NowRoom->m_iJoinUserCount == NowRoom->m_iMaxJoinCount)
		{
			// 1) �����Ϳ��� �� ���� ��Ŷ ������
			m_pParent->m_Master_LanClient->Packet_RoomClose_Req(RoomNo);

			// 2) ��� �������� ī��Ʈ�ٿ� ��Ŷ ������
			CProtocolBuff_Net* SendBuff = CProtocolBuff_Net::Alloc();

			WORD Type = en_PACKET_CS_GAME_RES_PLAY_READY;

			SendBuff->PutData((char*)&Type, 2);
			SendBuff->PutData((char*)&RoomNo, 4);
			SendBuff->PutData((char*)&m_pParent->m_stConst.m_bCountDownSec, 1);

			if (NowRoom->SendPacket_BroadCast(SendBuff) == false)
				g_BattleServer_Room_Dump->Crash();

			// 3) �� ī��Ʈ�ٿ� ���� ����
			// �� �ð� + �������� ���� ī��Ʈ�ٿ�(10��)�� �Ǹ� �� ��带 Play�� �����Ű��
			// ������ AUTH_TO_GAME���� �����Ų��.
			NowRoom->m_dwCountDown = timeGetTime();

			InterlockedDecrement(&m_pParent->m_lNowWaitRoomCount);
			InterlockedIncrement(&m_pParent->m_lReadyRoomCount);

			// 4) ���� ���¸� Ready�� ����
			NowRoom->m_iRoomState = eu_ROOM_STATE::READY_ROOM;			
		}		
	}



	// -----------------
	// Game��� ��Ŷ ó�� �Լ�
	// -----------------

	// ������ �̵��� �� ������ ��Ŷ.
	//
	// Parameter : CProtocolBuff_Net*
	// return : ����
	void CBattleServer_Room::CGameSession::Game_MovePacket(CProtocolBuff_Net* Packet)
	{
		// �α��� ���� üũ
		if (m_bLoginFlag == false)
			g_BattleServer_Room_Dump->Crash();

		// ���� ���� üũ
		// ���� ������ �̵� ��û�̸� �׳� ����.
		if (m_bAliveFlag == false)
			return;

		// 1. �����ִ� �� �˾ƿ���
		AcquireSRWLockShared(&m_pParent->m_Room_Umap_srwl);		// ----- Room Umap Shared ��

		auto FindRoom = m_pParent->m_Room_Umap.find(m_iRoomNo);

		// ������ ũ����
		if (FindRoom == m_pParent->m_Room_Umap.end())
			g_BattleServer_Room_Dump->Crash();

		stRoom* NowRoom = FindRoom->second;

		// �÷��̸�尡 �ƴϸ� ũ����
		if (NowRoom->m_iRoomState != eu_ROOM_STATE::PLAY_ROOM)
			g_BattleServer_Room_Dump->Crash();

		ReleaseSRWLockShared(&m_pParent->m_Room_Umap_srwl);		// ----- Room Umap Shared ���


		// 2. ������
		float MoveTargetX;
		float MoveTargetY;
		float MoveTargetZ;

		float HitPointX;
		float HitPointY;
		float HitPointZ;

		Packet->GetData((char*)&MoveTargetX, 4);
		Packet->GetData((char*)&MoveTargetY, 4);
		Packet->GetData((char*)&MoveTargetZ, 4);

		Packet->GetData((char*)&HitPointX, 4);
		Packet->GetData((char*)&HitPointY, 4);
		Packet->GetData((char*)&HitPointZ, 4);


		// 2. ���� ����
		// Hit ����Ʈ�� ������ �ʿ� ����. �׳� �����̸� �Ѵ�.
		// MoveTargetX,Z�� �����Ѵ�. Z�� ���� ���� ����.
		// ��, ������ X,Y�� �����ϸ� �ȴ�.
		m_fPosX = MoveTargetX;
		m_fPosY = MoveTargetZ;



		// 3. �� ���� ��� �������� ���� ��Ŷ Send�ϱ�(���� ����)
		CProtocolBuff_Net* SendBuff = CProtocolBuff_Net::Alloc();
		WORD Type = en_PACKET_CS_GAME_RES_MOVE_PLAYER;

		SendBuff->PutData((char*)&Type, 2);
		SendBuff->PutData((char*)&m_Int64AccountNo, 8);

		SendBuff->PutData((char*)&MoveTargetX, 4);
		SendBuff->PutData((char*)&MoveTargetY, 4);
		SendBuff->PutData((char*)&MoveTargetZ, 4);

		SendBuff->PutData((char*)&HitPointX, 4);
		SendBuff->PutData((char*)&HitPointY, 4);
		SendBuff->PutData((char*)&HitPointZ, 4);

		NowRoom->SendPacket_BroadCast(SendBuff, m_Int64AccountNo);
	}
	
	// HitPoint ����
	//
	// Parameter : CProtocolBuff_Net*
	// return : ����
	void CBattleServer_Room::CGameSession::Game_HitPointPacket(CProtocolBuff_Net* Packet)
	{
		// �α��� ���� üũ
		if (m_bLoginFlag == false)
			g_BattleServer_Room_Dump->Crash();

		// ���� ���� üũ
		// ���� ������ ��Ʈ ����Ʈ ������ ����
		if (m_bAliveFlag == false)
			return;


		// 1. �����ִ� �� �˾ƿ���
		AcquireSRWLockShared(&m_pParent->m_Room_Umap_srwl);		// ----- Room Umap Shared ��

		auto FindRoom = m_pParent->m_Room_Umap.find(m_iRoomNo);

		// ������ ũ����
		if (FindRoom == m_pParent->m_Room_Umap.end())
			g_BattleServer_Room_Dump->Crash();

		stRoom* NowRoom = FindRoom->second;

		// �÷��̸�尡 �ƴϸ� ũ����
		if (NowRoom->m_iRoomState != eu_ROOM_STATE::PLAY_ROOM)
			g_BattleServer_Room_Dump->Crash();

		ReleaseSRWLockShared(&m_pParent->m_Room_Umap_srwl);		// ----- Room Umap Shared ���



		// 2. ������
		float HitPointX;
		float HitPointY;
		float HitPointZ;

		Packet->GetData((char*)&HitPointX, 4);
		Packet->GetData((char*)&HitPointY, 4);
		Packet->GetData((char*)&HitPointZ, 4);



		// 3. �ش� �� ���� �������� HitPoint ���� ��Ŷ ������.
		// �ڱ� �ڽ� ����

		CProtocolBuff_Net* SendBuff = CProtocolBuff_Net::Alloc();

		WORD Type = en_PACKET_CS_GAME_RES_HIT_POINT;

		SendBuff->PutData((char*)&Type, 2);

		SendBuff->PutData((char*)&m_Int64AccountNo, 8);

		SendBuff->PutData((char*)&HitPointX, 4);
		SendBuff->PutData((char*)&HitPointY, 4);
		SendBuff->PutData((char*)&HitPointZ, 4);

		NowRoom->SendPacket_BroadCast(SendBuff, m_Int64AccountNo);		
	}

	// Fire 1 ��Ŷ (�� �߻�)
	//
	// Parameter : CProtocolBuff_Net*
	// return : ����
	void CBattleServer_Room::CGameSession::Game_Frie_1_Packet(CProtocolBuff_Net* Packet)
	{
		// �α��� ���� üũ
		if (m_bLoginFlag == false)
			g_BattleServer_Room_Dump->Crash();

		// ���� ���� üũ
		// ���� ������ ���� ��û�� ����
		if (m_bAliveFlag == false)
			return;


		// 1. �����ִ� �� �˾ƿ���
		AcquireSRWLockShared(&m_pParent->m_Room_Umap_srwl);		// ----- Room Umap Shared ��

		auto FindRoom = m_pParent->m_Room_Umap.find(m_iRoomNo);

		// ������ ũ����
		if (FindRoom == m_pParent->m_Room_Umap.end())
			g_BattleServer_Room_Dump->Crash();

		stRoom* NowRoom = FindRoom->second;

		// �÷��̸�尡 �ƴϸ� ũ����
		if (NowRoom->m_iRoomState != eu_ROOM_STATE::PLAY_ROOM)
			g_BattleServer_Room_Dump->Crash();

		ReleaseSRWLockShared(&m_pParent->m_Room_Umap_srwl);		// ----- Room Umap Shared ���



		// 2. ������
		float HitPointX;
		float HitPointY;
		float HitPointZ;

		Packet->GetData((char*)&HitPointX, 4);
		Packet->GetData((char*)&HitPointY, 4);
		Packet->GetData((char*)&HitPointZ, 4);




		// 3. �Ѿ� ���� 0���� ��� ��Ŷ ������
		if (m_iBullet == 0)
			return;

		// �Ѿ� �� ����
		--m_iBullet;		




		// 4. �� ���� �����鿡��(�ڱ��ڽ��� ����) Fire 1�� ���� ������Ŷ ������
		CProtocolBuff_Net* SendBuff = CProtocolBuff_Net::Alloc();

		WORD Type = en_PACKET_CS_GAME_RES_FIRE1;

		SendBuff->PutData((char*)&Type, 2);
		SendBuff->PutData((char*)&m_Int64AccountNo, 8);

		SendBuff->PutData((char*)&HitPointX, 4);
		SendBuff->PutData((char*)&HitPointY, 4);
		SendBuff->PutData((char*)&HitPointZ, 4);

		NowRoom->SendPacket_BroadCast(SendBuff, m_Int64AccountNo);



		// 5. ��Ŷ�� ���� �ð� ����
		// ��Ŷ�� ��ε�ĳ��Ʈ �ϴ� �ð��� 100m/s�� �������� �ϱ� ���� ���⼭ �ð� ����
		m_dwFire1_StartTime = timeGetTime();
	}

	// HitDamage
	//
	// Parameter : CProtocolBuff_Net*
	// return : ����
	void CBattleServer_Room::CGameSession::Game_HitDamage_Packet(CProtocolBuff_Net* Packet)
	{
		// �α��� ���� üũ
		if (m_bLoginFlag == false)
			g_BattleServer_Room_Dump->Crash();

		// ���� ���� üũ
		// ���� ������ ���� ������ ��Ŷ�� ����
		if (m_bAliveFlag == false)
			return;


		// 1. �����ִ� �� �˾ƿ���
		AcquireSRWLockShared(&m_pParent->m_Room_Umap_srwl);		// ----- Room Umap Shared ��

		auto FindRoom = m_pParent->m_Room_Umap.find(m_iRoomNo);

		// ������ ũ����
		if (FindRoom == m_pParent->m_Room_Umap.end())
			g_BattleServer_Room_Dump->Crash();

		stRoom* NowRoom = FindRoom->second;

		// �÷��̸�尡 �ƴϸ� ũ����
		if (NowRoom->m_iRoomState != eu_ROOM_STATE::PLAY_ROOM)
			g_BattleServer_Room_Dump->Crash();

		ReleaseSRWLockShared(&m_pParent->m_Room_Umap_srwl);		// ----- Room Umap Shared ���



		// 2. Fire_1 ��Ŷ�� ���� �ð� üũ
		DWORD FireTime = m_dwFire1_StartTime;

		// ���� �ð��� 0���� �ʱ�ȭ.
		m_dwFire1_StartTime = 0;

		// 100m/s �̳��� �Դ��� üũ
		if ((timeGetTime() - FireTime) > 100)
		{
			// 100m/s�� ���� �Ŀ� �Դٸ� ��Ŷ ����
			return;
		}



		// 3. 100 m/s �̳��� �Դٸ� ������
		INT64 TargetAccountNo; // ������ AccountNo

		Packet->GetData((char*)&TargetAccountNo, 8);


		// 4. �ش� �濡 ������ AccountNo�� �ֳ� üũ
		CGameSession* Target = NowRoom->Find(TargetAccountNo);

		if (Target == nullptr)
		{
			// ���ٸ� ��Ŷ ����
			// ���� ���ɼ� ����. 100 m/s �̳��� �̹� ������ ���ɼ�

			//�α� �����.
			g_BattleServer_RoomLog->LogSave(false, L"CBattleServer_Room", CSystemLog::en_LogLevel::LEVEL_DEBUG,
				L"Game_HitDamage_Packet() --> Target Not Find (Attack : %lld, Target : %lld)", m_Int64AccountNo, TargetAccountNo);

			return;
		}

		// 5. Ÿ���� ��� ���¶�� �׳� ����
		if (Target->GetAliveState() == false)
			return;

		
		// 6. Ÿ�ٰ� �Ÿ� ���
		// ��Ÿ��� ���� (a2 + b2 = c2)
		// �� ������ �Ÿ� ���� (c)
		float a = (m_fPosX - Target->m_fPosX);
		float b = (m_fPosY - Target->m_fPosY);

		float c = sqrtf((a*a) + (b*b));

		// ��� ���, Ÿ���� ���� ���� ���ٸ�, ��Ŷ �Ⱥ���
		// if(c >= 17) �� ����
		if (isgreaterequal(c, 17))
			return;


		// 7. hp ���� ó��
		// ������� ���� ������ ����� �ִ°�. Ÿ�ٿ��� ���� ������ ���		
		int MinusDamage = m_pParent->GunDamage(c);

		// �������� 0�� ���� �� �ִ�. HP�� ��������̱� ������, 0.67...�� ���͵� 0��.
		// 0�� ���� ��Ŷ ����.
		if (MinusDamage == 0)
			return;

		// �������� 0���� ���� �� ����. ������ �Ÿ� ���� �ִٰ� Ȯ���߱� ������.
		else if (MinusDamage < 0)
			g_BattleServer_Room_Dump->Crash();		

		// Ÿ�ٿ��� ����� �ִ� ���, ��丸 1 ����
		int TargetHP, HelmetCount;
		BYTE HelmetHit;
		if (Target->m_iHelmetCount > 0)
		{
			Target->m_iHelmetCount--;

			// Send�� ���� ����
			HelmetCount = Target->m_iHelmetCount;
			TargetHP = Target->m_iHP;
			HelmetHit = 1;
		}

		// ����� ���� ���, ������ ����
		else
		{
			// Ÿ�ٿ��� ������ ����
			TargetHP = Target->Damage(MinusDamage);					

			// Send�� ���� ����
			HelmetCount = 0;
			HelmetHit = 0;
		}
		


		// 8. �� ���� ��� �������� ������Ŷ ������ (�ڱ� �ڽ� ����)
		CProtocolBuff_Net* SendBuff = CProtocolBuff_Net::Alloc();

		WORD Type = en_PACKET_CS_GAME_RES_HIT_DAMAGE;

		SendBuff->PutData((char*)&Type, 2);

		SendBuff->PutData((char*)&m_Int64AccountNo, 8);
		SendBuff->PutData((char*)&TargetAccountNo, 8);
		SendBuff->PutData((char*)&TargetHP, 4);

		SendBuff->PutData((char*)&HelmetHit, 1);
		SendBuff->PutData((char*)&HelmetCount, 4);

		NowRoom->SendPacket_BroadCast(SendBuff);


		// 9. Ÿ���� ����ߴٸ� ���� ��� ���� ����
		if (Target->GetAliveState() == false)
		{
			NowRoom->Player_Die(this, Target);
		}
	}

	// Frie 2 ��Ŷ (�� ����)
	//
	// Parameter : CProtocolBuff_Net*
	// return : ����
	void CBattleServer_Room::CGameSession::Game_Fire_2_Packet(CProtocolBuff_Net* Packet)
	{
		// �α��� ���� üũ
		if (m_bLoginFlag == false)
			g_BattleServer_Room_Dump->Crash();

		// ���� ���� üũ
		// ���� ������ ���� ��û�� ����
		if (m_bAliveFlag == false)
			return;


		// 1. �����ִ� �� �˾ƿ���
		AcquireSRWLockShared(&m_pParent->m_Room_Umap_srwl);		// ----- Room Umap Shared ��

		auto FindRoom = m_pParent->m_Room_Umap.find(m_iRoomNo);

		// ������ ũ����
		if (FindRoom == m_pParent->m_Room_Umap.end())
			g_BattleServer_Room_Dump->Crash();

		stRoom* NowRoom = FindRoom->second;

		// �÷��̸�尡 �ƴϸ� ũ����
		if (NowRoom->m_iRoomState != eu_ROOM_STATE::PLAY_ROOM)
			g_BattleServer_Room_Dump->Crash();

		ReleaseSRWLockShared(&m_pParent->m_Room_Umap_srwl);		// ----- Room Umap Shared ���



		// 2. ������
		float HitPointX;
		float HitPointY;
		float HitPointZ;

		Packet->GetData((char*)&HitPointX, 4);
		Packet->GetData((char*)&HitPointY, 4);
		Packet->GetData((char*)&HitPointZ, 4);



		// 3. �� ���� �����鿡��(�ڱ��ڽ��� ����) Fire 2�� ���� ������Ŷ ������
		CProtocolBuff_Net* SendBuff = CProtocolBuff_Net::Alloc();

		WORD Type = en_PACKET_CS_GAME_RES_FIRE2;

		SendBuff->PutData((char*)&Type, 2);
		SendBuff->PutData((char*)&m_Int64AccountNo, 8);

		SendBuff->PutData((char*)&HitPointX, 4);
		SendBuff->PutData((char*)&HitPointY, 4);
		SendBuff->PutData((char*)&HitPointZ, 4);

		NowRoom->SendPacket_BroadCast(SendBuff, m_Int64AccountNo);
	}

	// KickDamage
	//
	// Parameter : CProtocolBuff_Net*
	// return : ����
	void CBattleServer_Room::CGameSession::Game_KickDamage_Packet(CProtocolBuff_Net* Packet)
	{
		// �α��� ���� üũ
		if (m_bLoginFlag == false)
			g_BattleServer_Room_Dump->Crash();

		// ���� ���� üũ
		// ���� ������ ���� ��û�� ����
		if (m_bAliveFlag == false)
			return;


		// 1. �����ִ� �� �˾ƿ���
		AcquireSRWLockShared(&m_pParent->m_Room_Umap_srwl);		// ----- Room Umap Shared ��

		auto FindRoom = m_pParent->m_Room_Umap.find(m_iRoomNo);

		// ������ ũ����
		if (FindRoom == m_pParent->m_Room_Umap.end())
			g_BattleServer_Room_Dump->Crash();

		stRoom* NowRoom = FindRoom->second;

		// �÷��̸�尡 �ƴϸ� ũ����
		if (NowRoom->m_iRoomState != eu_ROOM_STATE::PLAY_ROOM)
			g_BattleServer_Room_Dump->Crash();

		ReleaseSRWLockShared(&m_pParent->m_Room_Umap_srwl);		// ----- Room Umap Shared ���


		// 2. ���� �ִٸ�, ������
		INT64 TargetAccountNo; // ������ AccountNo

		Packet->GetData((char*)&TargetAccountNo, 8);


		// 3. �ش� �濡 ������ AccountNo�� �ֳ� üũ
		CGameSession* Target = NowRoom->Find(TargetAccountNo);

		if (Target == nullptr)
		{
			// ���ٸ� ��Ŷ ����
			// ���� ���ɼ� ����. 100 m/s �̳��� �̹� ������ ���ɼ�

			//�α� �����.
			g_BattleServer_RoomLog->LogSave(false, L"CBattleServer_Room", CSystemLog::en_LogLevel::LEVEL_DEBUG,
				L"Game_KickDamage_Packet() --> Target Not Find (Attack : %lld, Target : %lld)", m_Int64AccountNo, TargetAccountNo);

			return;
		}

		// 4. Ÿ���� ��� ���¶�� �׳� ����
		if (Target->GetAliveState() == false)
			return;


		// 5. �����ڿ� �ǰ����� �Ÿ� ���ϱ�.

		// ��Ÿ��� ���� (a2 + b2 = c2)
		// �� ������ �Ÿ� ���� (c)
		float a = (m_fPosX - Target->m_fPosX);
		float b = (m_fPosY - Target->m_fPosY);

		float c = sqrtf((a*a) + (b*b));

		// �� ���� �Ÿ��� 2���� �ִٸ�, ���� ����
		// c > 2�� ����
		if (isgreater(c, 2))
			return; 


		// 6. Ÿ�ٿ��� ������ ������ ����
		int TargetHP = Target->Damage(g_Data_KickDamage);


		// HP ���� ��Ŷ ������
		CProtocolBuff_Net* SendBuff = CProtocolBuff_Net::Alloc();

		WORD Type = en_PACKET_CS_GAME_RES_KICK_DAMAGE;

		SendBuff->PutData((char*)&Type, 2);

		SendBuff->PutData((char*)&m_Int64AccountNo, 8);
		SendBuff->PutData((char*)&TargetAccountNo, 8);
		SendBuff->PutData((char*)&TargetHP, 4);

		NowRoom->SendPacket_BroadCast(SendBuff);


		// 7. �ǰ��ڰ� ����ߴٸ� �ش� ���� ��� ��Ŷ�� �� ��ü�� �Ѹ���.
		if (Target->GetAliveState() == false)
		{
			NowRoom->Player_Die(this, Target);
		}
	}

	// Reload Request
	//
	// Parameter : CProtocolBuff_Net*
	// return : ����
	void CBattleServer_Room::CGameSession::Game_Reload_Packet(CProtocolBuff_Net* Packet)
	{
		// �α��� ���� üũ
		if (m_bLoginFlag == false)
			g_BattleServer_Room_Dump->Crash();

		// ���� ���� üũ
		// ���� ������ ������ ��û�� ����
		if (m_bAliveFlag == false)
			return;

		// 1. �����ִ� �� �˾ƿ���
		AcquireSRWLockShared(&m_pParent->m_Room_Umap_srwl);		// ----- Room Umap Shared ��

		auto FindRoom = m_pParent->m_Room_Umap.find(m_iRoomNo);

		// ������ ũ����
		if (FindRoom == m_pParent->m_Room_Umap.end())
			g_BattleServer_Room_Dump->Crash();

		stRoom* NowRoom = FindRoom->second;

		// �÷��̸�尡 �ƴϸ� ũ����
		if (NowRoom->m_iRoomState != eu_ROOM_STATE::PLAY_ROOM)
			g_BattleServer_Room_Dump->Crash();

		ReleaseSRWLockShared(&m_pParent->m_Room_Umap_srwl);		// ----- Room Umap Shared ���


		// 2. �� źâ �� Ȯ��
		//	-------- źâ�� ���ٸ� ��Ŷ ����.
		if (m_iCartridge == 0)
			return;

		//	-------- źâ�� �ִٸ� źâ -1 ��  �Ѿ��� g_Data_Cartridge_Bullet �� ����
		else
		{
			m_iCartridge = m_iCartridge - 1;
			m_iBullet = g_Data_Cartridge_Bullet;
		}

		// 3. �� ���� ��� ����(�ڱ��ڽ� ����)���� ��Ŷ ������
		CProtocolBuff_Net* SendBuff = CProtocolBuff_Net::Alloc();

		WORD Type = en_PACKET_CS_GAME_RES_RELOAD;

		SendBuff->PutData((char*)&Type, 2);

		SendBuff->PutData((char*)&m_Int64AccountNo, 8);
		SendBuff->PutData((char*)&m_iBullet, 4);
		SendBuff->PutData((char*)&m_iCartridge, 4);

		NowRoom->SendPacket_BroadCast(SendBuff);
	}

	// ������ ȹ�� ��û
	//
	// Parameter : CProtocolBuff_Net*, �������� Type
	// return : ����
	void CBattleServer_Room::CGameSession::Game_GetItem_Packet(CProtocolBuff_Net* Packet, int Type)
	{
		// �α��� ���� üũ
		if (m_bLoginFlag == false)
			g_BattleServer_Room_Dump->Crash();

		// ���� ���� üũ
		// ���� ������ ������ ȹ�� ��û�� ����
		if (m_bAliveFlag == false)
			return;

		// 1. �����ִ� �� �˾ƿ���
		AcquireSRWLockShared(&m_pParent->m_Room_Umap_srwl);		// ----- Room Umap Shared ��

		auto FindRoom = m_pParent->m_Room_Umap.find(m_iRoomNo);

		// ������ ũ����
		if (FindRoom == m_pParent->m_Room_Umap.end())
			g_BattleServer_Room_Dump->Crash();

		stRoom* NowRoom = FindRoom->second;

		// �÷��̸�尡 �ƴϸ� ũ����
		if (NowRoom->m_iRoomState != eu_ROOM_STATE::PLAY_ROOM)
			g_BattleServer_Room_Dump->Crash();

		ReleaseSRWLockShared(&m_pParent->m_Room_Umap_srwl);		// ----- Room Umap Shared ���


		// 2. ������
		UINT ItemID;
		Packet->GetData((char*)&ItemID, 4);

		// 3. ������ �˻�
		stRoomItem* NowItem = NowRoom->Item_Find(ItemID);

		// ���� �������̸� �����Ѵ�.
		if (NowItem == nullptr)
			return;

		// 4. �����۰� ������ �Ÿ� üũ.
		// +3.0f ~ -3.0f �������� ����Ѵ�. (���� ��ǥ��, ������ �̵� �����̱� ������)		
		float PosX = NowItem->m_fPosX;
		float PosY = NowItem->m_fPosY;

		// islessequal(1������, 2������) ����
		// 1������ <= 2������ : true
		// 1������ > 2������ : false
		if (islessequal(fabs(PosX - m_fPosX), m_pParent->m_stConst.m_fGetItem_Correction) == false ||
			islessequal(fabs(PosY - m_fPosY), m_pParent->m_stConst.m_fGetItem_Correction) == false)
		{
			return;
		}

		// 5. �̹��� ������ ȹ���� ��������, ������ �� ����(4�� ����)�� ������ ���������� üũ
		// �̰� ���ϸ�, ������ ����ϸ鼭 ���� �����۰� ��ǥ�� ��ĥ ��� ���й�� ����
		if (NowItem->m_bItemArea == 2)
		{
			// ������ �� �����̶��, �������� ȹ���� �ð� ����(��, �������� �Ҹ��� �ð�)
			int i = 0;
			while (i < 4)
			{
				if (g_Data_ItemPoint_Playzone[i][0] == PosX &&
					g_Data_ItemPoint_Playzone[i][1] == PosY)
				{
					NowRoom->m_dwItemCreateTick[i] = timeGetTime();
					break;
				}

				++i;
			}
		}

		// 6. �Ÿ��� ������ ���������� ȹ���� ������.
		// ������ �ڷᱸ������ ������ ����
		if (NowRoom->Item_Erase(NowItem) == false)
			g_BattleServer_Room_Dump->Crash();

		// 7. �����ۿ� ���� ȿ�� ���� �� ��Ŷ ������
		switch (Type)
		{
			// źâ 
		case CARTRIDGE:
		{
			// źâ +1
			m_iCartridge++;

			// ��� ��Ŷ ������ (�ڱ��ڽ� ����. ��ε�ĳ����)
			CProtocolBuff_Net* SendBuff = CProtocolBuff_Net::Alloc();

			WORD Type = en_PACKET_CS_GAME_RES_CARTRIDGE_GET;

			SendBuff->PutData((char*)&Type, 2);

			SendBuff->PutData((char*)&m_Int64AccountNo, 8);
			SendBuff->PutData((char*)&ItemID, 4);
			SendBuff->PutData((char*)&m_iCartridge, 4);

			if (NowRoom->SendPacket_BroadCast(SendBuff) == false)
				g_BattleServer_Room_Dump->Crash();
		}
			break;

			// ���
		case HELMET:
		{
			// ���� ��� ���� g_Data_HelmetDefensive��ŭ ����
			m_iHelmetCount = m_iHelmetCount + g_Data_HelmetDefensive;

			// ��� ��Ŷ ������ (�ڱ��ڽ� ����. ��ε�ĳ����)
			CProtocolBuff_Net* SendBuff = CProtocolBuff_Net::Alloc();

			WORD Type = en_PACKET_CS_GAME_RES_HELMET_GET;

			SendBuff->PutData((char*)&Type, 2);

			SendBuff->PutData((char*)&m_Int64AccountNo, 8);
			SendBuff->PutData((char*)&ItemID, 4);
			SendBuff->PutData((char*)&m_iHelmetCount, 4);

			if (NowRoom->SendPacket_BroadCast(SendBuff) == false)
				g_BattleServer_Room_Dump->Crash();
		}
			break;

			// �޵�Ŷ
		case MEDKIT:			
		{
			// ������ hp ȸ��
			// g_Data_HP /2 ��ŭ ȸ��
			m_iHP = m_iHP + (g_Data_HP / 2);

			// �ִ� hp �̻� ȸ�� �Ұ���.
			if (m_iHP > g_Data_HP)
				m_iHP = g_Data_HP;			

			// ��� ��Ŷ ������ (�ڱ��ڽ� ����. ��ε�ĳ����)
			CProtocolBuff_Net* SendBuff = CProtocolBuff_Net::Alloc();

			WORD Type = en_PACKET_CS_GAME_RES_MEDKIT_GET;

			SendBuff->PutData((char*)&Type, 2);

			SendBuff->PutData((char*)&m_Int64AccountNo, 8);
			SendBuff->PutData((char*)&ItemID, 4);
			SendBuff->PutData((char*)&m_iHP, 4);

			if (NowRoom->SendPacket_BroadCast(SendBuff) == false)
				g_BattleServer_Room_Dump->Crash();
		}
			break;

		default:
			g_BattleServer_Room_Dump->Crash();
		}		
	}	
}

// ------------------
// stRoom�� �Լ�
// (CBattleServer_Room�� �̳�Ŭ����)
// ------------------
namespace Library_Jingyu
{

	// ------------
	// ��� �Լ�
	// ------------

	// ������
	CBattleServer_Room::stRoom::stRoom()
	{
		// �̸� �޸� ���� ��Ƶα�
		m_JoinUser_Vector.reserve(10);
		m_RoomItem_umap.reserve(30);		

		m_iJoinUserCount = 0;
	}
	
	// �Ҹ���
	CBattleServer_Room::stRoom::~stRoom()
	{
	}

	// ------------
	// ��ε� ĳ��Ʈ
	// ------------

	// �ڷᱸ�� ���� ��� �������� ���ڷ� ���� ��Ŷ ������
	//
	// Parameter : CProtocolBuff_Net*
	// return : �ڷᱸ�� ���� ������ 0���� ��� false
	//		  : �� �ܿ��� true
	bool CBattleServer_Room::stRoom::SendPacket_BroadCast(CProtocolBuff_Net* SendBuff)
	{
		// 1. �ڷᱸ�� ���� ���� �� �ޱ�
		size_t Size = m_JoinUser_Vector.size();

		// 2. ���� ���� 0���� ���, return false
		if (Size == 0)
		{
			CProtocolBuff_Net::Free(SendBuff);
			return false;
		}

		// !! while�� ���� ���� ī��Ʈ��, ���� �� ��ŭ ���� !!
		// �����ʿ���, �Ϸ� ������ ���� Free�� �ϱ� ������ Add�ؾ� �Ѵ�.
		SendBuff->Add((int)Size);

		// 3. ���� �� ��ŭ ���鼭 ��Ŷ ����.
		size_t Index = 0;

		while (Index < Size)
		{
			m_JoinUser_Vector[Index]->m_euDebugMode = eu_USER_TYPE_DEBUG::COUNTDOWN;
			m_JoinUser_Vector[Index]->SendPacket(SendBuff);
			++Index;
		}

		// 4. ��Ŷ Free
		// !! ������ ���뿡�� Free ������, ���۷��� ī��Ʈ�� �ο� �� ��ŭ Add �߱� ������ 1���� �� ������ ����. !!	
		CProtocolBuff_Net::Free(SendBuff);

		return true;
	}

	// �ڷᱸ�� ���� ��� �������� ���ڷ� ���� ��Ŷ ������
	// ���ڷ� ���� AccountNo�� �����ϰ� ������
	//
	// Parameter : CProtocolBuff_Net*, AccountNo(��Ŷ �Ⱥ��� ����)
	// return : �ڷᱸ�� ���� ������ 0���� ��� false
	//		  : �� �ܿ��� true
	bool CBattleServer_Room::stRoom::SendPacket_BroadCast(CProtocolBuff_Net* SendBuff, INT64 AccountNo)
	{
		// 1. �ڷᱸ�� ���� ���� �� �ޱ�
		size_t Size = m_JoinUser_Vector.size();

		// 2. ���� ���� 0���� ���, return false
		if (Size == 0)
		{
			CProtocolBuff_Net::Free(SendBuff);
			return false;
		}

		// !! while�� ���� ���� ī��Ʈ��, ���� �� - 1 ��ŭ ���� !!
		// �����ʿ���, �Ϸ� ������ ���� Free�� �ϱ� ������ Add�ؾ� �Ѵ�.
		// �ڱ� �ڽſ��Դ� �Ⱥ����� ������, ���� ������ ���� ���� ++ �Ѵ�
		SendBuff->Add((int)Size - 1);

		// 3. ���� �� ��ŭ ���鼭 ��Ŷ ����.
		size_t Index = 0;

		while (Index < Size)
		{
			// �ڱ� �ڽ��� �ƴ� ��쿡�� ������.
			if(m_JoinUser_Vector[Index]->m_Int64AccountNo != AccountNo)
				m_JoinUser_Vector[Index]->SendPacket(SendBuff);

			++Index;
		}

		// 4. ��Ŷ Free
		// !! ������ ���뿡�� Free ������, ���۷��� ī��Ʈ�� �ο� �� ��ŭ Add �߱� ������ 1���� �� ������ ����. !!	
		// ����, ���� ���� 1���̾��� ���. ���۷��� ī��Ʈ�� ������Ű�� �ʾ� �״�� 1������ 
		// ������ Send�� ���߱� ������ ���⼭ ���ҽ��Ѿ� ��.
		CProtocolBuff_Net::Free(SendBuff);

		return true;
	}



	// ------------
	// �Ϲ� ����Լ�
	// ------------

	// �� ���� ��� ������ Auth_To_Game���� ����
	//
	// Parameter : ����
	// return : ����
	void CBattleServer_Room::stRoom::ModeChange()
	{
		// 1. �ڷᱸ�� ���� ���� �� �ޱ�
		size_t Size = m_JoinUser_Vector.size();

		// 2. ���� ���� 0���� ���ɼ� ����.
		// ī��Ʈ �ٿ� �߿� ��� ������ ���� ���.
		// ������ �Ⱥ���.
		if (Size == 0)
			return;

		// 3. 0���� �ƴ϶��, ���ο� �ִ� ��� ������ AUTH_TO_GAME���� ����
		size_t Index = 0;

		while (Index < Size)
		{
			m_JoinUser_Vector[Index]->SetMode_GAME();
			++Index;
		}
	}

	// �� ���� ��� ������ �������·� ����
	//
	// Parameter : ����
	// return : �ڷᱸ�� ���� ������ 0���� ��� false
	//		  : �� �ܿ��� true
	bool CBattleServer_Room::stRoom::AliveFlag_True()
	{
		// 1. �ڷᱸ�� ���� ���� �� �ޱ�
		size_t Size = m_JoinUser_Vector.size();

		// 2. ���� ���� 0���̰ų� 0������ ���, return false
		if (Size <= 0)
			return false;

		// 3. ���� �� ��ŭ ���鼭 ���� �÷��� ����
		size_t Index = 0;

		while (Index < Size)
		{
			m_JoinUser_Vector[Index]->AliveSet(true);
			++Index;
		}

		return true;
	}

	// �� ���� �����鿡�� ���� ���� ��Ŷ ������
	//
	// Parameter : ����
	// return : ����
	void CBattleServer_Room::stRoom::GameOver()
	{
		// 1. �� �ȿ� ������ 0���� ��� Crash
		if (m_iJoinUserCount == 0)
			g_BattleServer_Room_Dump->Crash();

		// 2. �� �ο��� üũ ������ vector ���� ����� �޶� Crash
		size_t Size = m_JoinUser_Vector.size();

		if(Size == 0 || Size != m_iJoinUserCount)
			g_BattleServer_Room_Dump->Crash();		
			   

		// 3. �¸��ڿ��� ���� �¸���Ŷ �����
		CProtocolBuff_Net* winPacket = CProtocolBuff_Net::Alloc();

		WORD Type = en_PACKET_CS_GAME_RES_WINNER;
		winPacket->PutData((char*)&Type, 2);


		// 4. �й��ڿ��� ���� �й� ��Ŷ �����
		CProtocolBuff_Net* LosePacket = CProtocolBuff_Net::Alloc();

		Type = en_PACKET_CS_GAME_RES_GAMEOVER;
		LosePacket->PutData((char*)&Type, 2);


		// 5. ��ȸ�ϸ鼭 ��Ŷ ������
		size_t Index = 0;
		int WinUserCount = 0;

		while (Index < Size)
		{
			// ������ �¸����� ��� (������)
			if (m_JoinUser_Vector[Index]->m_bAliveFlag == true)
			{
				CGameSession* NowPlayer = m_JoinUser_Vector[Index];	

				// �� ���ӿ� �¸��ڴ� 1��.
				// WinUserCount�� 1�ε� ���� ���Դٸ�, �¸��ڰ� 1�� �̻��� �Ǿ��ٴ� �ǹ�.
				// ���� �ȵ�.
				if(WinUserCount == 1)
					g_BattleServer_Room_Dump->Crash();			

				// ������ �¸�ī��Ʈ 1 ����
				NowPlayer->Record_Win_Add();

				// ���� �����(OnGame_ClientLeave)���� �����ؾ� �ϱ� ������, LastDBWriteFlag�� �ϳ� �ΰ� ���� �߳� ���߳� üũ�Ѵ�.
				NowPlayer->m_bLastDBWriteFlag = true;

				// �¸� ��Ŷ ������
				NowPlayer->SendPacket(winPacket);

				// �¸���Ŷ ���� �� ����
				++WinUserCount;			
			}

			// ������ �й����� ��� (�����)
			else
			{
				// ���۷��� ī��Ʈ ����.
				LosePacket->Add();

				// �й� ��Ŷ ������
				m_JoinUser_Vector[Index]->SendPacket(LosePacket);
			}

			++Index;
		}

		// 6. �й� ��Ŷ�� ���۷��� ī��Ʈ ����.
		CProtocolBuff_Net::Free(LosePacket);
	}

	// �� ���� �����鿡�� �˴ٿ� ������
	//
	// Parameter : ����
	// return : ����
	void CBattleServer_Room::stRoom::Shutdown_All()
	{
		// 1. �� �ȿ� ������ 0���̸� ���� �ȵ�. �ۿ��� �̹� 0�� �ƴ϶�°��� �˰� �Ա� ������.
		if (m_iJoinUserCount <= 0)
			g_BattleServer_Room_Dump->Crash();

		// 2. ���� ���� ������ ������ �ٸ��� ũ����
		size_t Size = m_JoinUser_Vector.size();
		if (m_iJoinUserCount != Size)
			g_BattleServer_Room_Dump->Crash();

		// 4. ��ȸ�ϸ� Shutdown
		size_t Index = 0;

		while (Index < Size)
		{
			m_JoinUser_Vector[Index]->Disconnect();

			++Index;
		}	
	}
	
	// �� ���� ��� �����鿡�� �� ������Ŷ�� �ٸ� ���� ���� ��Ŷ ������
	// 
	// Parameter : ����
	// return : ����
	void CBattleServer_Room::stRoom::CreateCharacter()
	{
		// 1. �� �ȿ� ������ 0���̸� ���� �ȵ�. �ۿ��� �̹� 0�� �ƴ϶�°��� �˰� �Ա� ������.
		if (m_iJoinUserCount <= 0)
			g_BattleServer_Room_Dump->Crash();

		// 2. ���� ���� ������ ������ �ٸ��� ũ����
		size_t Size = m_JoinUser_Vector.size();
		if (m_iJoinUserCount != Size)
			g_BattleServer_Room_Dump->Crash();

		// 4. ĳ���͵��� �ʱ� ���� ����
		// OnGame_ClientJoin���� �����ϴ°� �ƴ϶� ���⼭ �����ϴ� ������, ���� ��ġ�� � ������ ���� ��
		// �� ������ 1, 2, 3, 4 ������� ��������� �ϱ� ������.
		// ���� OnGame_ClientJoin���� �ϰԵǸ� � ������ �� ������ �ߴ��� ��..
		size_t Index = 0;

		DWORD NowTime = timeGetTime();

		// �ʱ� ������ ������ ��ǥ�� ����
		int SourceIndex = rand() % m_iJoinUserCount;

		while (Index < Size)
		{
			m_JoinUser_Vector[Index]->m_euDebugMode = eu_USER_TYPE_DEBUG::GAME_START;

			// ĳ���� �ʱ� ���� ����
			m_JoinUser_Vector[Index]->StartSet(g_Data_Position[SourceIndex][0], 
												g_Data_Position[SourceIndex][1], NowTime);

			// �̹��� ������ ��ǥ��, �ο��� ��� ������ ��ġ��� �ٽ� 0���� ���ư���.
			if (SourceIndex == m_iJoinUserCount)
				SourceIndex = 0;

			else
				SourceIndex++;		

			++Index;
		}

		// 5. ���� m_dwReaZoneTime��  m_dwTick����.
		m_dwReaZoneTime = NowTime;
		m_dwTick = NowTime;

		// 6. ��ȸ�ϸ� ��Ŷ ����.		
		int NowUserIndex = 0;
		while (NowUserIndex < Size)
		{
			size_t Index = 0;

			while (Index < Size)
			{
				// �� ĳ������ ���, �� ĳ���� ���� ��Ŷ ����
				if (NowUserIndex == Index)
				{	
					// ��Ŷ �����
					CProtocolBuff_Net* MySendBuff = CProtocolBuff_Net::Alloc();

					WORD Type = en_PACKET_CS_GAME_RES_CREATE_MY_CHARACTER;

					MySendBuff->PutData((char*)&Type, 2);
					MySendBuff->PutData((char*)&m_JoinUser_Vector[NowUserIndex]->m_fPosX, 4);
					MySendBuff->PutData((char*)&m_JoinUser_Vector[NowUserIndex]->m_fPosY, 4);
					MySendBuff->PutData((char*)&m_JoinUser_Vector[NowUserIndex]->m_iHP, 4);
					MySendBuff->PutData((char*)&m_JoinUser_Vector[NowUserIndex]->m_iBullet, 4);

					// ��Ŷ ������	
					m_JoinUser_Vector[NowUserIndex]->SendPacket(MySendBuff);
				}


				// �ٸ� ��� ĳ������ ���, �ٸ� ��� ĳ���� ���� ��Ŷ ����.
				else
				{
					// ��Ŷ �����
					CProtocolBuff_Net* OtherSendBuff = CProtocolBuff_Net::Alloc();

					WORD Type = en_PACKET_CS_GAME_RES_CREATE_OTHER_CHARACTER;

					OtherSendBuff->PutData((char*)&Type, 2);
					OtherSendBuff->PutData((char*)&m_JoinUser_Vector[Index]->m_Int64AccountNo, 8);
					OtherSendBuff->PutData((char*)m_JoinUser_Vector[Index]->m_tcNickName, 40);
					OtherSendBuff->PutData((char*)&m_JoinUser_Vector[Index]->m_fPosX, 4);
					OtherSendBuff->PutData((char*)&m_JoinUser_Vector[Index]->m_fPosY, 4);
					OtherSendBuff->PutData((char*)&m_JoinUser_Vector[Index]->m_iHP, 4);
					OtherSendBuff->PutData((char*)&m_JoinUser_Vector[Index]->m_iBullet, 4);

					// ��Ŷ ������	
					m_JoinUser_Vector[NowUserIndex]->SendPacket(OtherSendBuff);
				}

				++Index;
			}

			++NowUserIndex;
		}		
	}
	
	// �¸��ڿ��� ���� ������
	//
	// Parameter : ����
	// return : ����
	void CBattleServer_Room::stRoom::WInRecodeSend()
	{
		// 1. �� �ȿ� ������ 0���̸� ���� �ȵ�. �ۿ��� �̹� 0�� �ƴ϶�°��� �˰� �Ա� ������.
		if (m_iJoinUserCount <= 0)
			g_BattleServer_Room_Dump->Crash();

		// 2. ���� ���� ������ ������ �ٸ��� ũ����
		size_t Size = m_JoinUser_Vector.size();
		if (m_iJoinUserCount != Size)
			g_BattleServer_Room_Dump->Crash();

		// 4. �¸��ڿ��� ���� ����
		size_t Index = 0;

		while (Index < Size)
		{
			// �ش� ������ ���������� üũ. �����ڸ� �¸�����
			CGameSession* NowPlayer = m_JoinUser_Vector[Index];

			if (NowPlayer->GetAliveState() == true)
			{
				// 1. ���� ��Ŷ ���� ������.
				CProtocolBuff_Net* SendBuff = CProtocolBuff_Net::Alloc();

				WORD Type = en_PACKET_CS_GAME_RES_RECORD;

				SendBuff->PutData((char*)&Type, 2);

				SendBuff->PutData((char*)&NowPlayer->m_iRecord_PlayCount, 4);
				SendBuff->PutData((char*)&NowPlayer->m_iRecord_PlayTime, 4);
				SendBuff->PutData((char*)&NowPlayer->m_iRecord_Kill, 4);
				SendBuff->PutData((char*)&NowPlayer->m_iRecord_Die, 4);
				SendBuff->PutData((char*)&NowPlayer->m_iRecord_Win, 4);

				NowPlayer->SendPacket(SendBuff);
				break;
			}

			Index++;
		}
	}

	// �ش� �濡, ������ ���� (���� ���� ���� �� ����)
	// ���� ��, �� ���� �������� ������ ���� ��Ŷ ����
	//
	// Parameter : ����
	// return : ����
	void CBattleServer_Room::stRoom::StartCreateItem()
	{
		// 1. �� �ȿ� ������ 0���̸� ���� �ȵ�. �ۿ��� �̹� 0�� �ƴ϶�°��� �˰� �Ա� ������.
		if (m_iJoinUserCount <= 0)
			g_BattleServer_Room_Dump->Crash();

		// 2. ���� ���� ������ ������ �ٸ��� ũ����
		size_t Size = m_JoinUser_Vector.size();
		if (m_iJoinUserCount != Size)
			g_BattleServer_Room_Dump->Crash();

		// 3. ������ ������ 21���� ������ ����
		// ������ ���ÿ� �� ���� ��� �������� �ش� ������ ���� ��Ŷ ������.
		int i = 0;
		while (i < 16)
		{
			// ���������� źâ, ��� �� 1�� ����.
			int ItemType = rand() % 2;			

			// ������ 1�� ����
			CreateItem(g_Data_ItemPoint_Redzone[i][0], g_Data_ItemPoint_Redzone[i][1], ItemType, 1);
			
			++i;
		}

		// 4. ������ �ƴ� ������ 4���� ������ ����
		// ��������.
		i = 0;
		while (i < 4)
		{
			// ������ ���� �������� �޵�Ŷ, źâ, ��� �� 1�� ����.
			int ItemType = rand() % 3;

			// ������ 1�� ����
			CreateItem(g_Data_ItemPoint_Playzone[i][0], g_Data_ItemPoint_Playzone[i][1], ItemType, 2);

			++i;
		}
	}

	// ������ ����� ��ġ�� ������ ����
	// ���� ��, �� ���� �������� ������ ���� ��Ŷ ����
	//
	// Parameter : CGameSession* (����� ����)
	// return : ����
	void CBattleServer_Room::stRoom::PlayerDieCreateItem(CGameSession* DiePlayer)
	{
		// 1. źâ�� ������ źâ ����
		if (DiePlayer->m_iCartridge > 0)
		{
			// �̹��� �������� ������ ��ǥ
			// ��ǥ ���� (-1, 0, +1) ��ġ �� 1���� ����
			// rand() % 3�� �ϸ� (0, 1, 2) �� 1���� ����.
			// ���⼭ -1�� �ϸ� (-1, 0, 1) �� 1���� ������ �ȴ�.
			int Add = (rand() % 3) - 1;

			// źâ 1�� ����
			CreateItem(DiePlayer->m_fPosX + Add, DiePlayer->m_fPosY + Add, eu_ITEM_TYPE::CARTRIDGE, 0);
		}

		// 2. źâ�� ���� ���� �߰���, �޵�Ŷ �Ǵ� ��� �� 1�� ���� 
		
		// 1�̸� ���, 2�� �޵�Ŷ
		int Type = (rand() % 2) + 1;

		// �̹��� �������� ������ ��ǥ
		int Add = (rand() % 3) - 1;
		float itemX = DiePlayer->m_fPosX + Add;
		float itemY = DiePlayer->m_fPosY + Add;

		// ��� or �޵�Ŷ ����
		CreateItem(DiePlayer->m_fPosX + Add, DiePlayer->m_fPosY + Add, Type, 0);
	}
	
	// ��ǥ�� ���� ��ġ�� ������ 1�� ����
	//
	// Parameter : ������ XY��ǥ, ������ Type
	// return : ����
	void CBattleServer_Room::stRoom::CreateItem(float PosX, float PosY, int Type, BYTE Area)
	{
		// ������ID ++
		++m_uiItemID;

		// �� ����, ������ �ڷᱸ���� �߰�
		stRoomItem* Item = m_pBattleServer->m_Item_Pool->Alloc();
		Item->m_uiID = m_uiItemID;
		Item->m_fPosX = PosX;
		Item->m_fPosY = PosY;
		Item->m_euType = (eu_ITEM_TYPE)Type;
		Item->m_bItemArea = Area;

		Item_Insert(m_uiItemID, Item);

		CProtocolBuff_Net* SendBuff = CProtocolBuff_Net::Alloc();

		// źâ�� ���
		if (Type == CARTRIDGE)
		{
			WORD Type = en_PACKET_CS_GAME_RES_CARTRIDGE_CREATE;
			SendBuff->PutData((char*)&Type, 2);
		}

		// ����� ���
		else if (Type == HELMET)
		{
			WORD Type = en_PACKET_CS_GAME_RES_HELMET_CREATE;
			SendBuff->PutData((char*)&Type, 2);			
		}

		// �޵�Ŷ�� ���
		else if(Type == MEDKIT)
		{
			WORD Type = en_PACKET_CS_GAME_RES_MEDKIT_CREATE;
			SendBuff->PutData((char*)&Type, 2);			
		}

		// �� ������ �ƴϸ� ũ����
		else
			g_BattleServer_Room_Dump->Crash();

		// ������ ID
		SendBuff->PutData((char*)&m_uiItemID, 4);

		// ������ ��ǥ
		SendBuff->PutData((char*)&PosX, 4);
		SendBuff->PutData((char*)&PosY, 4);

		// SendPacket_��ε�ĳ����
		if (SendPacket_BroadCast(SendBuff) == false)
			g_BattleServer_Room_Dump->Crash(); 		 
	}
	
	// �� ���� ��� �������� "���� �߰���" ��Ŷ ������
	//
	// Parameter : �̹��� ������ ���� CGameSession*
	// return : ����
	void CBattleServer_Room::stRoom::Send_AddUser(CGameSession* NowPlayer)
	{
		// 1. �� �ȿ� ������ 0���̸� ���� �ȵ�. �ۿ��� �̹� 0�� �ƴ϶�°��� �˰� �Ա� ������.
		if (m_iJoinUserCount <= 0)
			g_BattleServer_Room_Dump->Crash();

		// 2. ���� ���� ������ ������ �ٸ��� ũ����
		size_t Size = m_JoinUser_Vector.size();
		if (m_iJoinUserCount != Size)
			g_BattleServer_Room_Dump->Crash();

		WORD Type = en_PACKET_CS_GAME_RES_ADD_USER;

		// 4. ������(���ڷ� ���� ����. �̹��� ������ ����), ���� �߰��� ��Ŷ ������ (�� ���� ��� ����)
		// �ڱ� �ڽ� ����. ��� ����.
		size_t Index = 0;
		while (Index < Size)
		{
			CProtocolBuff_Net* SendBuff = CProtocolBuff_Net::Alloc();

			CGameSession* NowSession = m_JoinUser_Vector[Index];

			SendBuff->PutData((char*)&Type, 2);
			SendBuff->PutData((char*)&m_iRoomNo, 4);
			SendBuff->PutData((char*)&NowSession->m_Int64AccountNo, 8);
			SendBuff->PutData((char*)NowSession->m_tcNickName, 40);

			SendBuff->PutData((char*)&NowSession->m_iRecord_PlayCount, 4);
			SendBuff->PutData((char*)&NowSession->m_iRecord_PlayTime, 4);
			SendBuff->PutData((char*)&NowSession->m_iRecord_Kill, 4);
			SendBuff->PutData((char*)&NowSession->m_iRecord_Die, 4);
			SendBuff->PutData((char*)&NowSession->m_iRecord_Win, 4);

			NowPlayer->SendPacket(SendBuff);

			++Index;
		}

		// 5. �濡 �ִ� �� ���� ��������, ���� "���� �߰���" ��Ŷ ������
		// �ٸ�, �濡 ������ 1���� ���(��, �̹��� ������ �� ȥ��)�� �׳� ����.
		if (Size == 1)
			return;
		
		CProtocolBuff_Net* SendBuff = CProtocolBuff_Net::Alloc();

		// ���� ��Ŷ �����α�.
		SendBuff->PutData((char*)&Type, 2);
		SendBuff->PutData((char*)&m_iRoomNo, 4);
		SendBuff->PutData((char*)&NowPlayer->m_Int64AccountNo, 8);
		SendBuff->PutData((char*)NowPlayer->m_tcNickName, 40);

		SendBuff->PutData((char*)&NowPlayer->m_iRecord_PlayCount, 4);
		SendBuff->PutData((char*)&NowPlayer->m_iRecord_PlayTime, 4);
		SendBuff->PutData((char*)&NowPlayer->m_iRecord_Kill, 4);
		SendBuff->PutData((char*)&NowPlayer->m_iRecord_Die, 4);
		SendBuff->PutData((char*)&NowPlayer->m_iRecord_Win, 4);

		// ���۷��� ī��Ʈ ����
		// �� �ڽ��� ������ �����̴� 1 ����.
		// Alloc�ϸ鼭 1 �ö����� 1 ����.
		// ��, 3���� ���� Alloc���� 1 �ö󰡰�, Add�� (3-2 = 1) �ö󰣴�. --> RefCount 2���ȴ�. �� ����.
		SendBuff->Add((int)Size - 2);
		
		Index = 0;
		INT64 TempAccountNo = NowPlayer->m_Int64AccountNo;
		while (Index < Size)
		{
			// 4������ �ڱ� �ڽſ��� ��������, ���⼭�� ���ܽ��Ѿ� ��.
			if (TempAccountNo != m_JoinUser_Vector[Index]->m_Int64AccountNo)
			{
				// ���� �ƴѰ� Ȯ���̴�, ��Ŷ ������
				m_JoinUser_Vector[Index]->SendPacket(SendBuff);
			}

			++Index;
		}
	}

	// �������� ��������, �� ���� ���� ��� �� ó�� �Լ�
	//
	// Parameter : ������(CGameSession*), �����(CGameSession*) 
	// return : ����
	void CBattleServer_Room::stRoom::Player_Die(CGameSession* AttackPlayer, CGameSession* DiePlayer)
	{
		// 1. ������, �������� AccountNo ���÷� �޾Ƶα�
		INT64 AtkAccountNo = AttackPlayer->m_Int64AccountNo;
		INT64 DieAccountNo = DiePlayer->m_Int64AccountNo;


		// 2. �����ڸ� ������·� ����
		DiePlayer->AliveSet(false);


		// 3. ���� ���� ���� ���� �̹� 0�̾����� ���� ����. 
		// ��� ������ �׾��µ� �� �׾��ٰ� �� ��.
		if (m_iAliveUserCount == 0)
			g_BattleServer_Room_Dump->Crash();


		// 4. ���� ���� ���� ��, GameMode ���� �� ���� ī��Ʈ 1 ����
		--m_iAliveUserCount;
		--m_iGameModeUser;



		// 5. ��Ŷ ������
		CProtocolBuff_Net* SendBuff = CProtocolBuff_Net::Alloc();

		WORD Type = en_PACKET_CS_GAME_RES_DIE;

		SendBuff->PutData((char*)&Type, 2);
		SendBuff->PutData((char*)&DieAccountNo, 8);

		SendPacket_BroadCast(SendBuff);


		// 6. ������ ����� ��ġ�� �ű� ������ ����
		PlayerDieCreateItem(DiePlayer);



		// 7. �������� Kill ī��Ʈ 1 ����
		AttackPlayer->Record_Kill_Add();



		// 8. ������ ���ī��Ʈ 1 ����
		DiePlayer->Recored_Die_Add();	
		
		// ���� �����(OnGame_ClientLeave)���� �����ؾ� �ϱ� ������, LastDBWriteFlag�� �ϳ� �ΰ� ���� �߳� ���߳� üũ�Ѵ�.
		DiePlayer->m_bLastDBWriteFlag = true;



		// 9. ����ڿ��� ���� ���� ������
		CProtocolBuff_Net* DieSendBuff = CProtocolBuff_Net::Alloc();
		Type = en_PACKET_CS_GAME_RES_RECORD;

		DieSendBuff->PutData((char*)&Type, 2);
		DieSendBuff->PutData((char*)&DiePlayer->m_iRecord_PlayCount, 4);
		DieSendBuff->PutData((char*)&DiePlayer->m_iRecord_PlayTime, 4);
		DieSendBuff->PutData((char*)&DiePlayer->m_iRecord_Kill, 4);
		DieSendBuff->PutData((char*)&DiePlayer->m_iRecord_Die, 4);
		DieSendBuff->PutData((char*)&DiePlayer->m_iRecord_Win, 4);

		DiePlayer->SendPacket(DieSendBuff);
	}


	// ------------
	// �ڷᱸ�� �Լ�
	// ------------

	// �ڷᱸ���� Insert
	//
	// Parameter : �߰��ϰ��� �ϴ� CGameSession*
	// return : ����
	void CBattleServer_Room::stRoom::Insert(CGameSession* InsertPlayer)
	{
		m_JoinUser_Vector.push_back(InsertPlayer);
	}

	// �ڷᱸ���� ������ �ֳ� üũ
	//
	// Parameter : AccountNo
	// return : ã�� ������ CGameSession*
	//		  : ������ ���� �� nullptr
	CBattleServer_Room::CGameSession* CBattleServer_Room::stRoom::Find(INT64 AccountNo)
	{
		size_t Size = m_JoinUser_Vector.size();

		// 1. �ڷᱸ�� �ȿ� ������ 0�̶�� Crash
		if (Size == 0)
			g_BattleServer_Room_Dump->Crash();


		// 2. �� �ȿ� �ִ� ���� ���� �ٸ��ٸ� Crash
		if (m_iJoinUserCount != Size)
			g_BattleServer_Room_Dump->Crash();


		// 3. ��ȸ�ϸ鼭 �ֳ� ���� �˻�
		size_t Index = 0;
		while (Index < Size)
		{
			if (m_JoinUser_Vector[Index]->m_Int64AccountNo == AccountNo)
				return m_JoinUser_Vector[Index];

			++Index;
		}

		// 4. ������� ���� ���°�. 
		return nullptr;
	}

	// �ڷᱸ������ Erase
	//
	// Parameter : �����ϰ��� �ϴ� CGameSession*
	// return : ���� �� true
	//		  : ���� ��  false
	bool CBattleServer_Room::stRoom::Erase(CGameSession* InsertPlayer)
	{
		size_t Size = m_JoinUser_Vector.size();
		bool Flag = false;

		// 1. �ڷᱸ�� �ȿ� ������ 0�̶�� return false
		if (Size == 0)
			return false;

		// 2. �ڷᱸ�� �ȿ� ������ 1���̰ų�, ã���� �ϴ� ������ �������� �ִٸ� �ٷ� ����
		if (Size == 1 || m_JoinUser_Vector[Size - 1] == InsertPlayer)
		{
			Flag = true;
			m_JoinUser_Vector.pop_back();
		}

		// 3. �ƴ϶�� Swap �Ѵ�
		else
		{
			size_t Index = 0;
			while (Index < Size)
			{
				// ���� ã���� �ϴ� ������ ã�Ҵٸ�
				if (m_JoinUser_Vector[Index] == InsertPlayer)
				{
					Flag = true;

					CGameSession* Temp = m_JoinUser_Vector[Size - 1];
					m_JoinUser_Vector[Size - 1] = m_JoinUser_Vector[Index];
					m_JoinUser_Vector[Index] = Temp;

					m_JoinUser_Vector.pop_back();

					break;
				}

				++Index;
			}
		}

		// 4. ����, ���� ���ߴٸ� return false
		if (Flag == false)
			return false;

		return true;
	}
	
	// ������ �ڷᱸ���� Insert
	//
	// Parameter : ItemID, stRoomItem*
	// return : ����
	void CBattleServer_Room::stRoom::Item_Insert(UINT ID, stRoomItem* InsertItem)
	{
		// Insert �õ�
		auto ret = m_RoomItem_umap.insert(make_pair(ID, InsertItem));

		// �ߺ��� ��� Crash
		if(ret.second == false)
			g_BattleServer_Room_Dump->Crash();		
	}

	// ������ �ڷᱸ���� �������� �ֳ� üũ
	//
	// Parameter : itemID
	// return : ã�� �������� stRoomItem*
	//		  : �������� ���� �� nullptr
	CBattleServer_Room::stRoomItem* CBattleServer_Room::stRoom::Item_Find(UINT ID)
	{
		// ������ �˻�
		auto itor = m_RoomItem_umap.find(ID);

		if (itor == m_RoomItem_umap.end())
			return nullptr;

		return itor->second;
	}

	// ������ �ڷᱸ������ Erase
	//
	// Parameter : �����ϰ��� �ϴ� stRoomItem*
	// return : ���� �� true
	//		  : ���� ��  false
	bool CBattleServer_Room::stRoom::Item_Erase(stRoomItem* InsertPlayer)
	{
		// �˻�
		auto itor = m_RoomItem_umap.find(InsertPlayer->m_uiID);

		// ������ return false
		if (itor == m_RoomItem_umap.end())
			return false;

		// ã������ Erase
		m_RoomItem_umap.erase(itor);

		// stRoomItem* ��ȯ
		m_pBattleServer->m_Item_Pool->Free(InsertPlayer);

		return true;
	}



	// ------------
	// ������ ���� �Լ�
	// ------------

	// ������ ���
	// �� �Լ���, ������ ��� ������ �� ������ ȣ��ȴ�.
	// �̹� �ۿ���, ȣ�� ���� �� üũ �� �� �Լ� ȣ��.
	//
	// Parameter : ��� �ð�(BYTE). 
	// return : ����
	void CBattleServer_Room::stRoom::RedZone_Warning(BYTE AlertTimeSec)
	{
		// 1. �� �ȿ� ������ 0���̸� ���� �ȵ�. �ۿ��� �̹� 0�� �ƴ϶�°��� �˰� �Ա� ������.
		if (m_iJoinUserCount <= 0)
			g_BattleServer_Room_Dump->Crash();

		// 2. ���� ���� ������ ������ �ٸ��� ũ����
		size_t Size = m_JoinUser_Vector.size();
		if (m_iJoinUserCount != Size)
			g_BattleServer_Room_Dump->Crash();

		// 3. ������ �������̶��, ������ ������ ó�� ����
		if (m_iRedZoneCount + 1 == m_pBattleServer->m_stConst.m_iRedZoneActiveLimit)
		{
			// �� ���� ��� �������� ������ ������ ��� ��Ŷ ����.
			CProtocolBuff_Net* SendBUff = CProtocolBuff_Net::Alloc();
			WORD Type = en_PACKET_CS_GAME_RES_REDZONE_ALERT_FINAL;

			SendBUff->PutData((char*)&Type, 2);
			SendBUff->PutData((char*)&AlertTimeSec, 1);
			SendBUff->PutData((char*)&m_bLastRedZoneSafeType, 1);

			if (SendPacket_BroadCast(SendBUff) == false)
				g_BattleServer_Room_Dump->Crash();
		}

		// 4. ������ �������� �ƴ϶��, �Ϲ� ó��
		else
		{
			// 20�� �ڿ� Ȱ��ȭ �� �������� Ÿ���� �˾Ƴ���.
			int RedZoneType = m_arrayRedZone[m_iRedZoneCount];

			// �������� ���� Ÿ�� ����
			WORD Type;
			switch (RedZoneType)
			{
				// ����
			case eu_REDZONE_TYPE::LEFT:
				Type = en_PACKET_CS_GAME_RES_REDZONE_ALERT_LEFT;
				break;

				// ������
			case eu_REDZONE_TYPE::RIGHT:
				Type = en_PACKET_CS_GAME_RES_REDZONE_ALERT_RIGHT;
				break;

				// ��
			case eu_REDZONE_TYPE::TOP:
				Type = en_PACKET_CS_GAME_RES_REDZONE_ALERT_TOP;
				break;

				// �Ʒ�
			case eu_REDZONE_TYPE::BOTTOM:
				Type = en_PACKET_CS_GAME_RES_REDZONE_ALERT_BOTTOM;
				break;

			default:
				g_BattleServer_Room_Dump->Crash();
				break;
			}

			// �� ���� ��� �������� ������ ��� ��Ŷ ������
			CProtocolBuff_Net* SendBUff = CProtocolBuff_Net::Alloc();

			SendBUff->PutData((char*)&Type, 2);
			SendBUff->PutData((char*)&AlertTimeSec, 1);

			if (SendPacket_BroadCast(SendBUff) == false)
				g_BattleServer_Room_Dump->Crash();
		}

	}

	// ������ Ȱ��ȭ
	// �� �Լ���, �������� Ȱ��ȭ �� �������� ȣ��ȴ�
	// ��, Ȱ��ȭ ���� ������ �ۿ��� ���� ������ �� �Լ� ȣ��
	//
	// Parameter : ����
	// return : ����
	void CBattleServer_Room::stRoom::RedZone_Active()
	{
		// 1. �� �ȿ� ������ 0���̸� ���� �ȵ�. �ۿ��� �̹� 0�� �ƴ϶�°��� �˰� �Ա� ������.
		if (m_iJoinUserCount <= 0)
			g_BattleServer_Room_Dump->Crash();

		// 2. ���� ���� ������ ������ �ٸ��� ũ����
		size_t Size = m_JoinUser_Vector.size();
		if (m_iJoinUserCount != Size)
			g_BattleServer_Room_Dump->Crash();

		// 3. �̹��� Ȱ����ų �������� �˾Ƴ���
		int RedZoneType = m_arrayRedZone[m_iRedZoneCount];
		m_iRedZoneCount++;

		// 4. ������ �������� ���
		if (m_iRedZoneCount == m_pBattleServer->m_stConst.m_iRedZoneActiveLimit)
		{			
			float (*LastSafe)[2] = m_pBattleServer->m_arrayLastRedZoneSafeRange[m_bLastRedZoneSafeType - 1];

			// �������� ����
			m_fSafePos[0][0] = LastSafe[0][0];
			m_fSafePos[0][1] = LastSafe[0][1];
			m_fSafePos[1][0] = LastSafe[1][0];
			m_fSafePos[1][1] = LastSafe[1][1];

			// �� ���� ��� �������� ������ ������ Ȱ��ȭ ��Ŷ ����
			CProtocolBuff_Net* SendBUff = CProtocolBuff_Net::Alloc();

			WORD Type = en_PACKET_CS_GAME_RES_REDZONE_ACTIVE_FINAL;

			SendBUff->PutData((char*)&Type, 2);
			SendBUff->PutData((char*)&m_bLastRedZoneSafeType, 1);

			if (SendPacket_BroadCast(SendBUff) == false)
				g_BattleServer_Room_Dump->Crash();
		}

		// 5. ������ �������� �ƴ� ���
		else
		{
			// Ȱ��ȭ ��ų �������� ���� �������븦 �����Ѵ�.
			WORD Type;
			switch (RedZoneType)
			{
				// ����
			case eu_REDZONE_TYPE::LEFT:
				m_fSafePos[0][1] = m_pBattleServer->m_arrayRedZoneRange[RedZoneType][1][1];
				Type = en_PACKET_CS_GAME_RES_REDZONE_ACTIVE_LEFT;
				break;

				// ������
			case eu_REDZONE_TYPE::RIGHT:
				m_fSafePos[1][1] = m_pBattleServer->m_arrayRedZoneRange[RedZoneType][0][1];
				Type = en_PACKET_CS_GAME_RES_REDZONE_ACTIVE_RIGHT;
				break;

				// ��
			case eu_REDZONE_TYPE::TOP:
				m_fSafePos[0][0] = m_pBattleServer->m_arrayRedZoneRange[RedZoneType][1][0];
				Type = en_PACKET_CS_GAME_RES_REDZONE_ACTIVE_TOP;
				break;

				// �Ʒ�
			case eu_REDZONE_TYPE::BOTTOM:
				m_fSafePos[1][0] = m_pBattleServer->m_arrayRedZoneRange[RedZoneType][0][0];
				Type = en_PACKET_CS_GAME_RES_REDZONE_ACTIVE_BOTTOM;
				break;

			default:
				g_BattleServer_Room_Dump->Crash();
				break;
			}

			// �� ���� ��� �������� ������ Ȱ��ȭ ��Ŷ ������
			CProtocolBuff_Net* SendBUff = CProtocolBuff_Net::Alloc();

			SendBUff->PutData((char*)&Type, 2);

			if (SendPacket_BroadCast(SendBUff) == false)
				g_BattleServer_Room_Dump->Crash();
		}
	}

	// ������ ������ üũ
	// �� �Լ���, �������� �������� ��� �� ������ ȣ��
	// ��, ���� ������ �ۿ��� ���� ������ �� �Լ� ȣ��
	//
	// Parameter : ����
	// return : ����
	void CBattleServer_Room::stRoom::RedZone_Damage()
	{
		// 1. �� �ȿ� ������ 0���̸� ���� �ȵ�. �ۿ��� �̹� 0�� �ƴ϶�°��� �˰� �Ա� ������.
		if (m_iJoinUserCount <= 0)
			g_BattleServer_Room_Dump->Crash();

		// 2. ���� ���� ������ ������ �ٸ��� ũ����
		size_t Size = m_JoinUser_Vector.size();
		if (m_iJoinUserCount != Size)
			g_BattleServer_Room_Dump->Crash();

		// 3. �� ���� ��� ������ Ȯ��.
		// ��ǥ�� �������밡 �´��� üũ.
		// �������밡 �ƴ϶�� �������� ���� ����.
		size_t Index = 0;
		while (Index < Size)
		{
			// ������ ������, ��ǥ üũ
			if (m_JoinUser_Vector[Index]->GetAliveState() == true)
			{
				CGameSession* NowPlayer = m_JoinUser_Vector[Index];
				
				// �÷��̾� X��
				// - safe[0]�� X���� �۰ų�
				// - safe[1]�� X���� ũ�ų�
				//
				// �÷��̾� Y��
				// - safe[0]�� Y���� �۰ų�
				// - safe[1]�� Y���� ũ�ų�
				if (isless(NowPlayer->m_fPosX, m_fSafePos[0][0]) ||
					isgreater(NowPlayer->m_fPosX, m_fSafePos[1][0]) ||
					isless(NowPlayer->m_fPosY, m_fSafePos[0][1]) ||
					isgreater(NowPlayer->m_fPosY, m_fSafePos[1][1]) )
				{
					// ������ ������ ����
					int AfterHP = NowPlayer->Damage(1);					

					// ������ ��Ŷ ����.
					CProtocolBuff_Net* SendBuff = CProtocolBuff_Net::Alloc();
					WORD Type = en_PACKET_CS_GAME_RES_REDZONE_DAMAGE;

					SendBuff->PutData((char*)&Type, 2);
					SendBuff->PutData((char*)&NowPlayer->m_Int64AccountNo, 8);
					SendBuff->PutData((char*)&AfterHP, 4);

					// ��� �������� ����(�ڱ��ڽ� ����)
					if (SendPacket_BroadCast(SendBuff) == false)
						g_BattleServer_Room_Dump->Crash();

					// ������ ����ߴٸ�
					if (NowPlayer->GetAliveState() == false)
					{
						INT64 AccountNo = NowPlayer->m_Int64AccountNo;

						// ���� ���� ���� ���� �̹� 0�̾����� ���� ����. 
						// ��� ������ �׾��µ� �� �׾��ٰ� �� ��.
						if (m_iAliveUserCount == 0)
							g_BattleServer_Room_Dump->Crash();

						// ���� ���� ���� ��, GameMode ���� ī��Ʈ 1 ����
						--m_iAliveUserCount;
						--m_iGameModeUser;

						// ��Ŷ ������
						CProtocolBuff_Net* SendBuff = CProtocolBuff_Net::Alloc();

						WORD Type = en_PACKET_CS_GAME_RES_DIE;

						SendBuff->PutData((char*)&Type, 2);
						SendBuff->PutData((char*)&AccountNo, 8);

						if (SendPacket_BroadCast(SendBuff) == false)
							g_BattleServer_Room_Dump->Crash();

						// ������ ����� ��ġ�� �ű� ������ ����
						PlayerDieCreateItem(NowPlayer);

						// Dieī��Ʈ 1 ����
						NowPlayer->Recored_Die_Add();

						// ���� �����(OnGame_ClientLeave)���� �����ؾ� �ϱ� ������, LastDBWriteFlag�� �ϳ� �ΰ� ���� �߳� ���߳� üũ�Ѵ�.
						NowPlayer->m_bLastDBWriteFlag = true;

						// ����ڿ��� ���� ���� ������
						CProtocolBuff_Net* DieSendBuff = CProtocolBuff_Net::Alloc();
						Type = en_PACKET_CS_GAME_RES_RECORD;

						DieSendBuff->PutData((char*)&Type, 2);
						DieSendBuff->PutData((char*)&NowPlayer->m_iRecord_PlayCount, 4);
						DieSendBuff->PutData((char*)&NowPlayer->m_iRecord_PlayTime, 4);
						DieSendBuff->PutData((char*)&NowPlayer->m_iRecord_Kill, 4);
						DieSendBuff->PutData((char*)&NowPlayer->m_iRecord_Die, 4);
						DieSendBuff->PutData((char*)&NowPlayer->m_iRecord_Win, 4);

						NowPlayer->SendPacket(DieSendBuff);
					}
				}
			}

			++Index;
		}

	}

}

// ----------------------------------------
// 
// MMOServer�� �̿��� ��Ʋ ����. �� ����
//
// ----------------------------------------
namespace Library_Jingyu
{
	// Net ����ȭ ���� 1���� ũ�� (Byte)
	LONG g_lNET_BUFF_SIZE = 512;
		   	  


	// -----------------------
	// �ܺο��� ��� ������ �Լ�
	// -----------------------

	// Start
	// ���������� CMMOServer�� Start, ���� ���ñ��� �Ѵ�.
	//
	// Parameter : ����
	// return : ���� �� false
	bool CBattleServer_Room::ServerStart()
	{
		// ī��Ʈ �� �ʱ�ȭ
		m_lNowWaitRoomCount = 0;
		m_lNowTotalRoomCount = 0;
		m_lGlobal_RoomNo = 0;

		m_lBattleEnterTokenError = 0;
		m_lRoomEnterTokenError = 0;
		m_lQuery_Result_Not_Find = 0;
		m_lTempError = 0;
		m_lTokenError = 0;
		m_lVerError = 0;
		m_OverlapLoginCount = 0;
		m_OverlapLoginCount_DB = 0;
		m_lReadyRoomCount = 0;
		m_lPlayRoomCount = 0;
		m_lAuthFPS = 0;
		m_lGameFPS = 0;

		m_shDB_Communicate.ParentSet(this);

		// 1. ���� ����
		m_cGameSession = new CGameSession[m_stConfig.MaxJoinUser];

		int i = 0;
		while (i < m_stConfig.MaxJoinUser)
		{
			// GameServer�� ������ ����
			m_cGameSession[i].m_pParent = this;

			// ������ ���� ����
			SetSession(&m_cGameSession[i], m_stConfig.MaxJoinUser);
			++i;
		}

		// 2. ����͸� ������ ����Ǵ�, �� Ŭ���̾�Ʈ ����
		if (m_Monitor_LanClient->ClientStart(m_stConfig.MonitorServerIP, m_stConfig.MonitorServerPort, m_stConfig.MonitorClientCreateWorker,
			m_stConfig.MonitorClientActiveWorker, m_stConfig.MonitorClientNodelay) == false)
			return false;

		// 3. ä�� ��Ŭ��� ����Ǵ� ������ ����
		if (m_Chat_LanServer->ServerStart(m_stConfig.ChatLanServerIP, m_stConfig.ChatPort, m_stConfig.ChatCreateWorker, m_stConfig.ChatActiveWorker,
			m_stConfig.ChatCreateAccept, m_stConfig.ChatNodelay, m_stConfig.ChatMaxJoinUser) == false)
			return false;

		// 4. ������ ������ ����Ǵ�, �� Ŭ���̾�Ʈ ����
		if (m_Master_LanClient->ClientStart(m_stConfig.MasterServerIP, m_stConfig.MasterServerPort, m_stConfig.MasterClientCreateWorker,
			m_stConfig.MasterClientActiveWorker, m_stConfig.MasterClientNodelay) == false)
			return false;

		// 4. Battle ���� ����
		if (Start(m_stConfig.BindIP, m_stConfig.Port, m_stConfig.CreateWorker, m_stConfig.ActiveWorker, m_stConfig.CreateAccept,
			m_stConfig.Nodelay, m_stConfig.MaxJoinUser, m_stConfig.HeadCode, m_stConfig.XORCode) == false)
			return false;

		// ���� ���� �α� ���		
		g_BattleServer_RoomLog->LogSave(true, L"CBattleServer_Room", CSystemLog::en_LogLevel::LEVEL_SYSTEM, L"ServerOpen...");

		return true;

	}

	// Stop
	// ���������� Stop ����
	//
	// Parameter : ����
	// return : ����
	void CBattleServer_Room::ServerStop()
	{
		// 1. ����͸� Ŭ�� ����
		if (m_Monitor_LanClient->GetClinetState() == true)
			m_Monitor_LanClient->ClientStop();

		// 2. ������ Lan Ŭ�� ����
		if (m_Master_LanClient->GetClinetState() == true)
			m_Master_LanClient->ClientStop();

		// 3. ���� ����
		if (GetServerState() == true)
			Stop();

		// 3. ���� ����
		delete[] m_cGameSession;
	}

	// ��¿� �Լ�
	//
	// Parameter : ����
	// return : ����
	void CBattleServer_Room::ShowPrintf()
	{
		// �ش� ���μ����� ��뷮 üũ�� Ŭ����
		static CCpuUsage_Process ProcessUsage;

		// CPU ����� üũ Ŭ���� (�ϵ����)
		static CCpuUsage_Processor CProcessorCPU;

		// ȭ�� ����� �� ����
		/*
		Total SessionNum : 					- MMOServer �� ���Ǽ�
		AuthMode SessionNum :				- Auth ����� ���� ��
		GameMode SessionNum (Auth + Game) :	- Game ����� ���� �� (Auth + Game��� ���� ��)

		PacketPool_Net : 		- �ܺο��� ��� ���� Net ����ȭ ������ ��
		Accept Socket Queue :	- Accept Socket Queue ���� �ϰ� ��
		HeartBeat Flag	:		- ��Ʈ��Ʈ �÷���. 1�̸� ��Ʈ��Ʈ ��

		Accept Total :		- Accept ��ü ī��Ʈ (accept ���Ͻ� +1)
		Accept TPS :		- �ʴ� Accept ó�� Ƚ��
		Auth FPS :			- �ʴ� Auth ������ ó�� Ƚ��
		Game FPS :			- �ʴ� Game ������ ó�� Ƚ��

		Send TPS:			- �ʴ� Send�Ϸ� Ƚ��. (�Ϸ��������� ����)
		Recv TPS:			- �ʴ� Recv�Ϸ� Ƚ��. (��Ŷ 1���� �ϼ��Ǿ��� �� ����. RecvProc���� ��Ŷ�� �ֱ� ���� 1�� ����)

		NetBuff_ChunkAlloc_Count : - Net ����ȭ ���� �� Alloc�� ûũ �� (�ۿ��� ������� ûũ ��)
		Player_ChunkAlloc_Count :	- �÷��̾� ����ȭ ���� �� Alloc�� ûũ �� (�ۿ��� ������� ûũ ��)
		ASQPool_ChunkAlloc_Count : - Accept Socket Queue�� ���� �ϰ� �� Alloc�� ûũ �� (�ۿ��� ������� ûũ ��)

		------------------ Room -------------------
		WaitRoom :			- ���� ��
		ReadyRoom :			- �غ�� ��
		PlayRoom :			- �÷��̹� ��
		TotalRoom :			- �� �� ��
		TotalRoom_Pool :	- �� umap�� �ִ� ī��Ʈ.	

		Room_ChunkAlloc_Count : - �Ҵ���� �� ûũ ��(�ۿ��� ������� ��)

		------------------ DBWrite -------------------
		Node Alloc Count :	(Add : )	- DBWrite �����忡�� �Ͻ�Ű�� ť ���� ������. - Add�� �ʴ� ť�� ���� �������� ��.
		DBWrite TPS :	- DBWrite�� TPS

		------------------ Error -------------------
		Battle_EnterTokenError:		- ��Ʋ���� ���� ��ū ����
		Room_EnterTokenError:		- �� ���� ��ū ����
		Login_Query_Not_Find :		- auth�� �α��� ��Ŷ����, DB ������ -10�� ����� ��.
		Login_Query_Temp :			- auth�� �α��� ��Ŷ����, DB ���� �� -10�� �ƴ� ������ ��.
		Login_UserTokenError :		- Auth�� �α��� ��Ŷ����, ���� ��ū�� �ٸ�
		Login_VerError :			- auth�� �α��� ��Ŷ����, ������ ���� ������ �ٸ�
		Login_Duplicate :			- �ߺ� �α���
		Login_DBWrite :				- DBWrite���ε� �� ���� ���
		SemCount :					- ��������(121)���� �� 1 ����

		---------- Battle LanServer(Chat) ---------
		SessionNum :				- ��Ʋ ������ (ä�ü���)�� ������ ���� ��
		PacketPool_Lan :			- ������� ����ȭ���� �� ����. ��Ż.

		Lan_BuffChunkAlloc_Count :	- ������� ����ȭ ������ ûũ ��(�ۿ��� ������� ��)

		-------------- LanClient -------------------
		Monitor Connect :			- ����� ������ ����Ǵ� �� Ŭ��. ���� ����
		Master Connect :			- ������ ������ ����Ǵ� �� Ŭ��. ���ӿ���

		----------------------------------------------------
		CPU usage [T:%.1f%% U:%.1f%% K:%.1f%%] [BattleServer:%.1f%% U:%.1f%% K:%.1f%%] - ���μ���, ���μ��� ��뷮.

		*/

		// ��� ����, ���μ���/���μ��� ��뷮 ����
		ProcessUsage.UpdateCpuTime();
		CProcessorCPU.UpdateCpuTime();

		LONG AuthUser = GetAuthModeUserCount();
		LONG GameUser = GetGameModeUserCount();
		LONG TempDBWriteTPS = InterlockedExchange(&m_shDB_Communicate.m_lDBWriteTPS, 0);
		LONG TempDBWriteCountTPS = InterlockedExchange(&m_shDB_Communicate.m_lDBWriteCountTPS, 0);
		m_lAuthFPS = GetAuthFPS();
		m_lGameFPS = GetGameFPS();

		printf("================== Battle Server ==================\n"
			"Total SessionNum : %lld\n"
			"AuthMode SessionNum : %d\n"
			"GameMode SessionNum : %d (Auth + Game : %d)\n\n"

			"PacketPool_Net : %d\n"
			"Accept Socket Queue : %d\n"
			"HeartBeat Flag : %d\n\n"

			"Accept Total : %lld\n"
			"Accept TPS : %d\n"
			"Send TPS : %d\n"
			"Recv TPS : %d\n\n"

			"Auth FPS : %d\n"
			"Game FPS : %d\n\n"

			"NetBuff_ChunkAlloc_Count : %d (Out : %d)\n"
			"ASQPool_ChunkAlloc_Count : %d (Out : %d)\n\n"

			"------------------ Room -------------------\n"
			"WaitRoom : %d\n"
			"ReadyRoom : %d\n"
			"PlayRoom : %d\n"
			"TotalRoom : %d\n"
			"TotalRoom_Pool : %lld\n\n"

			"Room_ChunkAlloc_Count : %d (Out : %d)\n\n"

			"------------------DBWrite------------------\n"
			"Node Alloc Count : %d (Add : %d)\n"
			"DBWrite TPS : %d\n\n"

			"------------------ Error -------------------\n"
			"Battle_EnterTokenError : %d\n"
			"Room_EnterTokenError : %d\n"
			"Login_Query_Not_Find : %d\n"
			"Login_Query_Temp : %d\n"
			"Login_UserTokenError : %d\n"
			"Login_VerError : %d\n"
			"Login_Overlap_DB : %d\n"
			"Login_Overlap : %d\n"
			"SemCount : %d\n"
			"HeartBeat_Count : %d\n\n"

			"---------- Battle LanServer(Chat) ---------\n"
			"SessionNum : %lld\n"
			"PacketPool_Lan : %d\n\n"

			"Lan_BuffChunkAlloc_Count : %d (Out : %d)\n\n"

			"-------------- LanClient -----------\n"
			"Monitor Connect : %d\n"
			"Master Connect : %d\n\n"

			"========================================================\n\n"
			"CPU usage [T:%.1f%% U:%.1f%% K:%.1f%%] [BattleServer:%.1f%% U:%.1f%% K:%.1f%%]\n",

			// ----------- ���� ������
			GetClientCount(),
			AuthUser,
			GameUser, AuthUser + GameUser,

			CProtocolBuff_Net::GetNodeCount(),
			GetASQ_Count(),
			GetHeartBeatFlag(),

			GetAccpetTotal(),
			GetAccpetTPS(),
			GetSendTPS(),
			GetRecvTPS(),

			m_lAuthFPS,
			m_lGameFPS,

			CProtocolBuff_Net::GetChunkCount(), CProtocolBuff_Net::GetOutChunkCount(),
			GetChunkCount(), GetOutChunkCount(),

			m_lNowWaitRoomCount,
			m_lReadyRoomCount,
			m_lPlayRoomCount,
			m_lNowTotalRoomCount,		
			m_Room_Umap.size(),
			m_Room_Pool->GetAllocChunkCount(), m_Room_Pool->GetOutChunkCount(),

			m_shDB_Communicate.m_pDB_Wirte_Start_Queue->GetNodeSize(), TempDBWriteCountTPS,
			TempDBWriteTPS,

			// ----------- ����
			m_lBattleEnterTokenError,
			m_lRoomEnterTokenError,
			m_lQuery_Result_Not_Find,
			m_lTempError,
			m_lTokenError,
			m_lVerError,
			m_OverlapLoginCount_DB,
			m_OverlapLoginCount,
			GetSemCount(),
			GetHeartBeatCount(),

			// ----------- ��Ʋ ������
			m_Chat_LanServer->GetClientCount(),
			CProtocolBuff_Lan::GetNodeCount(),
			CProtocolBuff_Lan::GetChunkCount(), CProtocolBuff_Lan::GetOutChunkCount(),


			// ----------- ��Ŭ�� (������, �����)
			m_Monitor_LanClient->GetClinetState(),
			m_Master_LanClient->GetClinetState(),

			// ----------- ���μ��� ��뷮
			CProcessorCPU.ProcessorTotal(), CProcessorCPU.ProcessorUser(), CProcessorCPU.ProcessorKernel(),

			// ----------- ���μ��� ��뷮 
			ProcessUsage.ProcessTotal(), ProcessUsage.ProcessUser(), ProcessUsage.ProcessKernel()
		);
	}

	   


	// -----------------------
	// ���ο����� ����ϴ� �Լ�
	// -----------------------

	// ���Ͽ��� Config ���� �о����
	// 
	// Parameter : config ����ü
	// return : ���������� ���� �� true
	//		  : �� �ܿ��� false
	bool CBattleServer_Room::SetFile(stConfigFile* pConfig)
	{
		Parser Parser;

		// ���� �ε�
		try
		{
			Parser.LoadFile(_T("MMOGameServer_Config.ini"));
		}
		catch (int expn)
		{
			if (expn == 1)
			{
				printf("File Open Fail...\n");
				return false;
			}
			else if (expn == 2)
			{
				printf("FileR Read Fail...\n");
				return false;
			}
		}



		////////////////////////////////////////////////////////
		// BATTLESERVER config �о����
		////////////////////////////////////////////////////////

		// ���� ���� -------------------------
		if (Parser.AreaCheck(_T("BATTLESERVER")) == false)
			return false;

		// IP
		if (Parser.GetValue_String(_T("BindIP"), pConfig->BindIP) == false)
			return false;

		// Port
		if (Parser.GetValue_Int(_T("Port"), &pConfig->Port) == false)
			return false;

		// ���� ��Ŀ ��
		if (Parser.GetValue_Int(_T("CreateWorker"), &pConfig->CreateWorker) == false)
			return false;

		// Ȱ��ȭ ��Ŀ ��
		if (Parser.GetValue_Int(_T("ActiveWorker"), &pConfig->ActiveWorker) == false)
			return false;

		// ���� ����Ʈ
		if (Parser.GetValue_Int(_T("CreateAccept"), &pConfig->CreateAccept) == false)
			return false;

		// ��� �ڵ�
		if (Parser.GetValue_Int(_T("HeadCode"), &pConfig->HeadCode) == false)
			return false;

		// xorcode
		if (Parser.GetValue_Int(_T("XorCode"), &pConfig->XORCode) == false)
			return false;

		// Nodelay
		if (Parser.GetValue_Int(_T("Nodelay"), &pConfig->Nodelay) == false)
			return false;

		// �ִ� ���� ���� ���� ��
		if (Parser.GetValue_Int(_T("MaxJoinUser"), &pConfig->MaxJoinUser) == false)
			return false;

		// �α� ����
		if (Parser.GetValue_Int(_T("LogLevel"), &pConfig->LogLevel) == false)
			return false;




		////////////////////////////////////////////////////////
		// �⺻ config �о����
		////////////////////////////////////////////////////////

		// ���� ���� -------------------------
		if (Parser.AreaCheck(_T("CONFIG")) == false)
			return false;

		// VerCode
		if (Parser.GetValue_Int(_T("VerCode"), &m_uiVer_Code) == false)
			return false;

		// BattleNetServer�� IP
		if (Parser.GetValue_String(_T("BattleNetServerIP"), m_Master_LanClient->m_tcBattleNetServerIP) == false)
			return false;

		// MasterNetServer�� Port ����.
		m_Master_LanClient->m_iBattleNetServerPort = pConfig->Port;		

		// ������ ������ ���� �� ��� ��ū
		TCHAR m_tcMasterToken[33];
		if (Parser.GetValue_String(_T("MasterEnterToken"), m_tcMasterToken) == false)
			return false;

		// ��� ���� char������ ����� ������ ��ȯ�ؼ� ������ �־���Ѵ�.
		int len = WideCharToMultiByte(CP_UTF8, 0, m_tcMasterToken, lstrlenW(m_tcMasterToken), 0, 0, 0, 0);
		WideCharToMultiByte(CP_UTF8, 0, m_tcMasterToken, lstrlenW(m_tcMasterToken), m_Master_LanClient->m_cMasterToken, len, 0, 0);



		
		////////////////////////////////////////////////////////
		// ä�� LanServer Config �о����
		////////////////////////////////////////////////////////

		// ���� ���� -------------------------
		if (Parser.AreaCheck(_T("CHATLANSERVER")) == false)
			return false;

		// IP
		if (Parser.GetValue_String(_T("ChatBindIP"), pConfig->ChatLanServerIP) == false)
			return false;

		// Port
		if (Parser.GetValue_Int(_T("ChatPort"), &pConfig->ChatPort) == false)
			return false;

		// ���� ��Ŀ ��
		if (Parser.GetValue_Int(_T("ChatCreateWorker"), &pConfig->ChatCreateWorker) == false)
			return false;

		// Ȱ��ȭ ��Ŀ ��
		if (Parser.GetValue_Int(_T("ChatActiveWorker"), &pConfig->ChatActiveWorker) == false)
			return false;

		// ���� ����Ʈ
		if (Parser.GetValue_Int(_T("ChatCreateAccept"), &pConfig->ChatCreateAccept) == false)
			return false;

		// Nodelay
		if (Parser.GetValue_Int(_T("ChatNodelay"), &pConfig->ChatNodelay) == false)
			return false;

		// �ִ� ���� ���� ���� ��
		if (Parser.GetValue_Int(_T("ChatMaxJoinUser"), &pConfig->ChatMaxJoinUser) == false)
			return false;

		// �α� ����
		if (Parser.GetValue_Int(_T("ChatLogLevel"), &pConfig->ChatLogLevel) == false)
			return false;






		////////////////////////////////////////////////////////
		// ������ LanClient config �о����
		////////////////////////////////////////////////////////

		// ���� ���� -------------------------
		if (Parser.AreaCheck(_T("MASTERLANCLIENT")) == false)
			return false;

		// IP
		if (Parser.GetValue_String(_T("MasterServerIP"), pConfig->MasterServerIP) == false)
			return false;

		// Port
		if (Parser.GetValue_Int(_T("MasterServerPort"), &pConfig->MasterServerPort) == false)
			return false;

		// ���� ��Ŀ ��
		if (Parser.GetValue_Int(_T("MasterClientCreateWorker"), &pConfig->MasterClientCreateWorker) == false)
			return false;

		// Ȱ��ȭ ��Ŀ ��
		if (Parser.GetValue_Int(_T("MasterClientActiveWorker"), &pConfig->MasterClientActiveWorker) == false)
			return false;

		// Nodelay
		if (Parser.GetValue_Int(_T("MasterClientNodelay"), &pConfig->MasterClientNodelay) == false)
			return false;




		////////////////////////////////////////////////////////
		// ����͸� LanClient config �о����
		////////////////////////////////////////////////////////

		// ���� ���� -------------------------
		if (Parser.AreaCheck(_T("MONITORLANCLIENT")) == false)
			return false;

		// IP
		if (Parser.GetValue_String(_T("MonitorServerIP"), pConfig->MonitorServerIP) == false)
			return false;

		// Port
		if (Parser.GetValue_Int(_T("MonitorServerPort"), &pConfig->MonitorServerPort) == false)
			return false;

		// ���� ��Ŀ ��
		if (Parser.GetValue_Int(_T("MonitorClientCreateWorker"), &pConfig->MonitorClientCreateWorker) == false)
			return false;

		// Ȱ��ȭ ��Ŀ ��
		if (Parser.GetValue_Int(_T("MonitorClientActiveWorker"), &pConfig->MonitorClientActiveWorker) == false)
			return false;


		// Nodelay
		if (Parser.GetValue_Int(_T("MonitorClientNodelay"), &pConfig->MonitorClientNodelay) == false)
			return false;


		return true;
	}

	// �Ѿ� ������ ��� �� ���Ǵ� �Լ� (������ ���ÿ��� ������ ����.)
	// ���� ���ҽ��Ѿ� �ϴ� HP�� ����Ѵ�.
	//
	// Parameter : �����ڿ� �������� �Ÿ�
	// return : ���ҽ��Ѿ��ϴ� HP
	int CBattleServer_Room::GunDamage(float Range)
	{
		// ���� ������ ���� ���� ������ ���

		// �Ÿ��� 2���� ª�ٸ� 100%������
		// Range <= 2 �� ����
		if (islessequal(Range, 2))
			return g_Data_HitDamage;

		// �װ� �ƴ϶�� ������ ���� ���
		// 0~2������ ������ �Ÿ��� ����Ѵ�.
		// ��, 0~17���� �Ÿ��� üũ�ϱ� ������, ��� 2~17�̸�
		// �̴� ��, ���� ��궧�� 0~15���� �ִ°����� ����ؾ� �Ѵ�.
		return (int)(g_Data_HitDamage - (m_fFire1_Damage * (Range - 2)));
	}
		
	// �� ���� �Լ�
	//
	// Parameter : ����
	// return : ����
	void CBattleServer_Room::RoomCreate()
	{
		// 1. ���� �� �� �� ����
		InterlockedIncrement(&m_lNowTotalRoomCount);

		// 2. ���� �� ����.
		InterlockedIncrement(&m_lNowWaitRoomCount);

		// 3. �� Alloc
		stRoom* NowRoom = m_Room_Pool->Alloc();

		// ����, �� �ȿ� ���� ���� 0���� �ƴϸ� Crash
		if (NowRoom->m_iJoinUserCount != 0 || NowRoom->m_JoinUser_Vector.size() != 0)
			g_BattleServer_Room_Dump->Crash();


		// 4. �⺻ ����
		LONG RoomNo = InterlockedIncrement(&m_lGlobal_RoomNo);
		NowRoom->m_iBattleServerNo = m_iServerNo;
		NowRoom->m_iRoomNo = RoomNo;
		NowRoom->m_iRoomState = eu_ROOM_STATE::WAIT_ROOM;
		NowRoom->m_iGameModeUser = 0;
		NowRoom->m_bGameEndFlag = false;
		NowRoom->m_dwGameEndMSec = 0;
		NowRoom->m_bShutdownFlag = false;
		NowRoom->m_uiItemID = 0;
		NowRoom->m_pBattleServer = this;
		NowRoom->m_bRedZoneWarningFlag = false;

		// 10�ʸ��� ������ ������ų ���� ��� 0���� �ʱ�ȭ
		int i = 0;
		while (i < 4)
		{
			NowRoom->m_dwItemCreateTick[i] = 0;
			++i;
		}

		// 5. ������ ����
		// ������ ���� ���� ����
		int RedIndex = rand() % 24;

		i = 0;
		while (i < 4)
		{
			NowRoom->m_arrayRedZone[i] = m_arrayRedZoneCreate[RedIndex][i];
			++i;
		}

		// �� �� ������ ���� ����
		NowRoom->m_dwReaZoneTime = 0;
		NowRoom->m_dwTick = 0;
		NowRoom->m_iRedZoneCount = 0;

		// ��Ʈ ������ Ÿ��.
		// 1 ~ 4������ ��.
		NowRoom->m_bLastRedZoneSafeType = (rand() % 4) + 1;


		// 6. ���� �������� ��ǥ ����
		NowRoom->m_fSafePos[0][0] = 0;
		NowRoom->m_fSafePos[0][1] = 0;

		NowRoom->m_fSafePos[1][0] = 153;
		NowRoom->m_fSafePos[1][1] = 170;


		// 7. �� ���� ������ Ŭ�����Ű��
		auto itor_begin = NowRoom->m_RoomItem_umap.begin();
		auto itor_end = NowRoom->m_RoomItem_umap.end();

		while (itor_begin != itor_end)
		{
			// ������ ����ü ��ȯ
			m_Item_Pool->Free(itor_begin->second);

			// Erase
			itor_begin = NowRoom->m_RoomItem_umap.erase(itor_begin);
		}


		// 8. �� ���� ��ū ����				
		WORD Index = rand() % 64;	// 0 ~ 63 �� �ε��� ��󳻱�				
		memcpy_s(NowRoom->m_cEnterToken, 32, m_cRoomEnterToken[Index], 32);


		// 9. �� ���� �ڷᱸ���� �߰�
		if (InsertRoomFunc(RoomNo, NowRoom) == false)
			g_BattleServer_Room_Dump->Crash();


		// 10. ä�� �������� [�ű� ���� ���� �˸�] ��Ŷ ����
		// �� Ŭ�� ���� ������.
		// ä�� ������ ���� �����Ϳ��Ե� ������.
		m_Chat_LanServer->Packet_NewRoomCreate_Req(NowRoom);
	}
	   
	// HTTP ��� �� ��ó��
	//
	// Parameter : ����
	// return : ����
	void CBattleServer_Room::AuthLoop_HTTP()
	{
		// 1. Q ������ Ȯ��
		int iQSize = m_shDB_Communicate.m_pDB_ReadComplete_Queue->GetInNode();

		// �� �����ӿ� �ִ� m_iHTTP_MAX���� ��ó��.
		if (iQSize > m_stConst.m_iHTTP_MAX)
			iQSize = m_stConst.m_iHTTP_MAX;

		// 2. ������ ���� ����
		DB_WORK* DBData;
		while (iQSize > 0)
		{
			// �ϰ� ��ť		
			if (m_shDB_Communicate.m_pDB_ReadComplete_Queue->Dequeue(DBData) == -1)
				g_BattleServer_Room_Dump->Crash();

			try
			{
				// �ϰ� Ÿ�Կ� ���� �ϰ� ó��
				switch (DBData->m_wWorkType)
				{
					// �α��� ��Ŷ�� ���� ����ó��
				case eu_DB_AFTER_TYPE::eu_LOGIN_AUTH:
					Auth_LoginPacket_AUTH((DB_WORK_LOGIN*)DBData);
					break;

					// �α��� ��Ŷ�� ���� ���� ������ ó��
				case eu_DB_AFTER_TYPE::eu_LOGIN_INFO:
					Auth_LoginPacket_Info((DB_WORK_LOGIN_CONTENTS*)DBData);
					break;

					// ���� �ϰ� Ÿ���̸� ����.
				default:
					TCHAR str[200];
					StringCchPrintf(str, 200, _T("OnAuth_Update(). HTTP Type Error. Type : %d"), DBData->m_wWorkType);

					throw CException(str);
				}

			}
			catch (CException& exc)
			{
				// �α� ��� (�α� ���� : ����)
				g_BattleServer_RoomLog->LogSave(false, L"CBattleServer_Room", CSystemLog::en_LogLevel::LEVEL_ERROR, L"%s",
					(TCHAR*)exc.GetExceptionText());

				// ����
				g_BattleServer_Room_Dump->Crash();
			}

			// DB_WORK ��ȯ
			m_shDB_Communicate.m_pDB_Work_Pool->Free(DBData);

			--iQSize;
		}
	}
	
	// ������ ���� Ȥ�� ä�ü����� �׾��� ���
	// Auth �����忡�� ȣ��ȴ�.
	//
	// Parameter : ����
	// return : ���� 
	void CBattleServer_Room::AuthLoop_ServerDie()
	{
		// Wait����� �游 �����ϱ� ������, Shared �� ����
		AcquireSRWLockShared(&m_Room_Umap_srwl);		// ----- �� Shared ��

			// ���� �ϳ� �̻� �ִٸ�
		if (m_Room_Umap.size() > 0)
		{
			auto itor_Now = m_Room_Umap.begin();
			auto itor_End = m_Room_Umap.end();

			// �� ��ȸ
			while (itor_Now != itor_End)
			{
				// wait����� ���� ���
				if (itor_Now->second->m_iRoomState == eu_ROOM_STATE::WAIT_ROOM)
				{
					stRoom* NowRoom = itor_Now->second;

					// ���� �� ����, ���� �� �� ����
					InterlockedDecrement(&m_lNowWaitRoomCount);
					InterlockedIncrement(&m_lReadyRoomCount);

					// �����Ϳ��� �� ���� ��Ŷ ������
					m_Master_LanClient->Packet_RoomClose_Req(NowRoom->m_iRoomNo);

					// �� ��带 Ready�� ����
					NowRoom->m_iRoomState = eu_ROOM_STATE::READY_ROOM;

					// ������ �ִ� ���� ��� �� ���� ��� ���� Shutdown
					if (NowRoom->m_iJoinUserCount > 0)
						NowRoom->Shutdown_All();
				}

				++itor_Now;
			}

		}

		ReleaseSRWLockShared(&m_Room_Umap_srwl);		// ----- �� Shared ���
	}
	
	// �� ���� ó��
	// ���� game���� �����ϱ� ���..
	// Auth �����忡�� �������� ȣ��ȴ�.
	//
	// Parameter : ����
	// return : ����
	void CBattleServer_Room::AuthLoop_RoomLogic()
	{
		// �� �����ӿ� �ִ� m_iLoopRoomModeChange���� �� ��� ����
		int ModeChangeCount = m_stConst.m_iLoopRoomModeChange;
		DWORD CmpTime = timeGetTime();

		AcquireSRWLockShared(&m_Room_Umap_srwl);	// ----- Room umap Shared ��

		auto itor_Now = m_Room_Umap.begin();
		auto itor_End = m_Room_Umap.end();

		// �� �ڷᱸ���� ó������ ������ ��ȸ
		while (itor_Now != itor_End)
		{
			// ����, �̹� �����ӿ�, ������ ��ŭ�� �� ��� ������ �߻��ߴٸ� �׸��Ѵ�.
			if (ModeChangeCount == 0)
				break;

			stRoom* NowRoom = itor_Now->second;

			// �ش� ���� �غ���� ���
			if (NowRoom->m_iRoomState == eu_ROOM_STATE::READY_ROOM)
			{
				// ���� ���� 0���̸� �ٷ� ����
				// Auth��忡�� �� ������ Ÿ�� ����, 
				// 1. �߰��� ä�� or ������ ������ �׾ ���� �ı��ؾ� �� ���.
				// 2. Ready������ ��� ������ ���� ���
				// Play������ �Ѱܼ� �ڿ������� ����ǵ��� ��
				if (NowRoom->m_iJoinUserCount == 0)
				{
					InterlockedDecrement(&m_lReadyRoomCount);
					InterlockedIncrement(&m_lPlayRoomCount);

					// �� ���¸� Play�� ����
					// Game������� �� üũ ������ ���� ���� �ο����� 0���� üũ�Ѵ�.
					// ������, �Ѿ�� �ڵ����� �ı��� ��.
					NowRoom->m_iRoomState = eu_ROOM_STATE::PLAY_ROOM;

					--ModeChangeCount;
				}

				// ���� ����� ���� �ƴ� ��� ����.
				else
				{
					// ī��Ʈ �ٿ��� �Ϸ�Ǿ����� üũ
					if ((NowRoom->m_dwCountDown + m_stConst.m_iCountDownMSec) <= CmpTime)
					{
						// 1. ������ ���� �� ����
						if (NowRoom->m_iJoinUserCount != NowRoom->m_JoinUser_Vector.size())
							g_BattleServer_Room_Dump->Crash();

						NowRoom->m_iAliveUserCount = NowRoom->m_iJoinUserCount;

						// 2. ������ 0���� �ƴҶ��� ���� ����
						if (NowRoom->m_iJoinUserCount > 0)
						{
							// �� �� ��� ������ ���� �÷��� ����
							// false�� ���ϵ� �� ����. �ڷᱸ�� ���� ������ 0���� �� �ֱ� ������.
							// ������ ���ϰ� �ȹ޴´�.
							NowRoom->AliveFlag_True();

							// �� ���� ��� ��������, ��Ʋ���� ���� �÷��� ���� ��Ŷ ������
							CProtocolBuff_Net* SendBuff = CProtocolBuff_Net::Alloc();

							WORD Type = en_PACKET_CS_GAME_RES_PLAY_START;

							SendBuff->PutData((char*)&Type, 2);
							SendBuff->PutData((char*)&NowRoom->m_iRoomNo, 4);

							// ���⼭�� false�� ���ϵ� �� ����(�ڷᱸ�� ���� ������ 0���� �� ����)
							// ī��Ʈ�ٿ��� ������ ���� ��� ������ ���� ���ɼ�.
							// �׷��� ���ϰ� �ȹ޴´�.
							NowRoom->SendPacket_BroadCast(SendBuff);

							// ������ ���� �� stRoom�� ������ �ڷᱸ���� �߰�.
							// �׸��� ��� �������� ������ ���� ��Ŷ ����
							NowRoom->StartCreateItem();
						}

						InterlockedDecrement(&m_lReadyRoomCount);
						InterlockedIncrement(&m_lPlayRoomCount);

						// 3. �� ���¸� Play�� ����
						NowRoom->m_iRoomState = eu_ROOM_STATE::PLAY_ROOM;

						// 4. ��� ������ AUTH_TO_GAME���� ����
						NowRoom->ModeChange();

						// 5. �̹� �����ӿ��� �� ��� ���� ���� ī��Ʈ ����
						--ModeChangeCount;
					}
				}
			}

			++itor_Now;
		}

		ReleaseSRWLockShared(&m_Room_Umap_srwl);	// ----- Room umap Shared ���
	}

	// ���Ӹ���� �ߺ� �α��� ���� ����
	// Game �����忡�� �������� ȣ��ȴ�.
	// 
	// Parameter : ����
	// return : ����
	void CBattleServer_Room::GameLoop_OverlapLogin()
	{
		AcquireSRWLockExclusive(&m_Overlap_list_srwl);		// forward_list �ڷᱸ�� Exclusive �� ------- 

		// ó������ ������ ��ȸ
		auto itor_listBegin = m_Overlap_list.begin();
		auto itor_listEnd = m_Overlap_list.end();

		while (itor_listBegin != itor_listEnd)
		{
			CGameSession* NowPlayer = itor_listBegin->second;

			// 1. ��� Ȯ��
			// Auth�� ���, �ƿ� �� �ڷᱸ���� �ȵ�����, Game�̾��ٰ� LOG_OUT�� �Ǵ� ���� OnGame_ClientLeave���� ó���ϱ� ������, 
			// ���⼭ Game�̶�� ���� ���� Game��� ������°� Ȯ��
			if (NowPlayer->m_euModeType == eu_PLATER_MODE::GAME)
			{
				// ClientKey ��
				// ���⼭ ClientKey�� �ٸ��ٴ� ����, ���� ������ �ٸ� ������ �ƴٴ� ��.
				// �� list�� ���� ����� ������ �̹� ������.		
				if (NowPlayer->m_int64ClientKey == itor_listBegin->first)
				{
					// shutdown ����
					NowPlayer->Disconnect();
				}
			}

			// 2. itor_listBegin�� Next�� �̵�
			// �׸��� m_Overlap_list�� ���� �� ������(�ݹ� Ȯ���� ������)�� pop �Ѵ�
			// �ٸ� ���������� list������ ���ŵǾ�� ��.
			itor_listBegin = next(itor_listBegin);
			m_Overlap_list.pop_front();
		}

		ReleaseSRWLockExclusive(&m_Overlap_list_srwl);		// forward_list �ڷᱸ�� Exclusive ��� ------- 

	}

	// ���� ����� �� üũ
	// Game �����忡�� �������� ȣ��ȴ�.
	//
	// Parameter : (out)int�� �迭(������ �� ��ȣ ������), (out)������ ���� ��
	void CBattleServer_Room::GameLoop_RoomLogic(int DeleteRoomNo[], int* Index)
	{
		AcquireSRWLockShared(&m_Room_Umap_srwl);	// ----- Room Umap Shared ��

		auto itor_Now = m_Room_Umap.begin();
		auto itor_End = m_Room_Umap.end();

		// Step 1. ������ �� üũ
		// Step 2. ���� ����� �� ó�� 	
		// Setp 3. ���� ���� üũ
		// Step 4. �����ڰ� 0���� ��쿡�� ���� ����.
		// Step 5. ���������� �÷��� ���� ���� ��� ó��.

		// ��� ���� ��ȸ�ϸ�, PLAY ����� �濡 ���� �۾� ����
		while (itor_Now != itor_End)
		{
			// �� ��� üũ. 
			if (itor_Now->second->m_iRoomState == eu_ROOM_STATE::PLAY_ROOM)
			{
				// ������� ���� ���� �����常 �����ϴ� ��.
				stRoom* NowRoom = itor_Now->second;

				// Step 1. ������ �� üũ
				// ������ �ο� ���� 0���� ��. (���� �� �ƴ�)
				// ���� ���� ��, ��� ������ �i�Ƴ��� �۾����� �Ϸ�� ��.
				if (NowRoom->m_iJoinUserCount == 0)
				{
					if ((*Index) < 100)
					{
						DeleteRoomNo[(*Index)] = NowRoom->m_iRoomNo;
						++(*Index);
					}
				}

				// Step 2. ���� ����� �� ó�� 
				else if (NowRoom->m_bGameEndFlag == true)
				{
					// �˴ٿ��� ���� �ʾ��� ��쿡�� ���� ����.
					// �̹� ���� �����ӿ� �˴ٿ��� ���ȴµ�, ���� ��� ������ ������� �ʾ�
					// �� �� ������ Ż ���ɼ��� �ֱ� ������.
					if (NowRoom->m_bShutdownFlag == false)
					{
						// ���� �ð��� �Ǿ����� üũ
						if ((timeGetTime() - NowRoom->m_dwGameEndMSec) >= m_stConst.m_iRoomCloseDelay)
						{
							// �˴ٿ� ���ȴٴ� �÷��� ����.
							NowRoom->m_bShutdownFlag = true;

							// ���� �濡 �ִ� �������� Shutdown
							NowRoom->Shutdown_All();
						}
					}
				}

				// Setp 3. ���� ���� üũ
				// �����ڰ� 1���̸鼭 ���Ӹ���� ���� ���� ������ ���� ������ ���, ���� ����.			
				else if (NowRoom->m_iAliveUserCount == 1)
				{
					// !! Auth ��忡�� 1���� ������ ��� ������ ���� �Ŀ� !!
					// !! ���� 1���� Game���� ����Ǳ� ���� OnGame_Update���� �ش� ������ ���� üũ�� ��� !!
					// !! �÷��� ī��Ʈ�� ������� ���� ���� �ִ�. !!
					// !! ������, Game��� ���� ���� AliveUser���� �������� �� Ȯ���Ѵ� !!
					if (NowRoom->m_iAliveUserCount == NowRoom->m_iGameModeUser)
					{
						// ���� ���� ��Ŷ ������
						// - ������ 1���� �¸� ��Ŷ
						// - ������ �����ڵ鿡�� �й� ��Ŷ
						NowRoom->GameOver();

						// �� �� �¸��ڿ��� ���� ������
						NowRoom->WInRecodeSend();

						// ���� �� ���� �÷��� ����
						NowRoom->m_bGameEndFlag = true;

						// �� ������ �ð� ����
						// �� �ı� üũ �뵵
						NowRoom->m_dwGameEndMSec = timeGetTime();
					}
				}

				// Step 4. �����ڰ� 0���� ��쿡�� ���� ����.
				// �� ���� �����ڵ��� ���� �����ӿ� hp�� 0�̵Ǿ� ���� ���.
				else if (NowRoom->m_iAliveUserCount == 0)
				{
					if (NowRoom->m_iGameModeUser != 0)
						g_BattleServer_Room_Dump->Crash();

					// ���� ���� ��Ŷ ������
					// - ��� �������� �й� ��Ŷ ����. �¸��� ����
					CProtocolBuff_Net* SendBuff = CProtocolBuff_Net::Alloc();

					WORD Type = en_PACKET_CS_GAME_RES_GAMEOVER;

					SendBuff->PutData((char*)&Type, 2);

					NowRoom->SendPacket_BroadCast(SendBuff);

					// �� �� �¸��� ����
					// NowRoom->WInRecodeSend();

					// ���� �� ���� �÷��� ����
					NowRoom->m_bGameEndFlag = true;

					// �� ������ �ð� ����
					// �� �ı� üũ �뵵
					NowRoom->m_dwGameEndMSec = timeGetTime();
				}

				// Step 5. ���������� �÷��� ���� ���� ���
				else
				{
					// ���� �ð� ���ص�
					DWORD NowTime = timeGetTime();

					// -- ������ ���� üũ
					// ���� ������ ���۵� ������ üũ
					// ���� ���¸� PLAY�� �Ѿ����, ���� ��� ������ Game���� �Ѿ���� �ʾ��� ��쿡��
					// �� ������ Ż �� ������, ������ ��� ������ Game���� �Ѿ�ͼ� ������ �����ߴ��� üũ
					if (NowRoom->m_dwReaZoneTime > 0)
					{
						// -- �������� Ȱ��ȭ �Ǿ �Ǵ��� üũ.
						// �̹�, ��� �������� Ȱ��ȭ �Ǿ��ٸ�, �� �̻� Ȱ��ȭ�Ǹ� �ȵ�.
						if (NowRoom->m_iRedZoneCount < m_stConst.m_iRedZoneActiveLimit)
						{
							// -- ������ ��� üũ						
							if (NowRoom->m_bRedZoneWarningFlag == false)
							{
								// ������ ��� �Ⱥ��� ���¶��, �ð� üũ �� �����Ŷ ����
								if ((NowTime - NowRoom->m_dwReaZoneTime) >= m_stConst.m_dwRedZoneWarningTime)
								{
									// ������ ��� �Լ� ȣ��
									NowRoom->RedZone_Warning((BYTE)(m_stConst.m_dwRedZoneWarningTime / 1000));

									// ������ ��� ���� �÷��׸� true�� �����.
									NowRoom->m_bRedZoneWarningFlag = true;
								}
							}

							// -- ������ �˶��� ���� ���¶�� ������ Ȱ��ȭ üũ
							else
							{
								// -- ������ Ȱ��ȭ üũ
								if ((NowTime - NowRoom->m_dwReaZoneTime) >= m_stConst.m_dwReaZoneActiveTime)
								{
									// ������ Ȱ��ȭ �Լ� ȣ��
									NowRoom->RedZone_Active();

									// ������ Ȱ��ȭ �ð� ����. �ٽ� 40�ʸ� ��ٷ�����.
									NowRoom->m_dwReaZoneTime = NowTime;

									// ������ ��� ���� �÷��� ����
									NowRoom->m_bRedZoneWarningFlag = false;

								}
							}
						}

						// -- ������ ������ üũ						
						// Ȱ��ȭ�� �������� �ϳ��� �ִ� ��쿡�� �ش� ���� ����
						if (NowRoom->m_iRedZoneCount > 0)
						{
							// 1�ʰ� �Ǿ��� üũ. �������� 1�ʴ����� üũ�ϱ� ������
							if ((NowTime - NowRoom->m_dwTick) >= 1000)
							{
								// ������ ƽ ������ üũ �Լ� ȣ��
								NowRoom->RedZone_Damage();

								// tick ����
								NowRoom->m_dwTick = NowTime;
							}
						}
					}

					// -- �� ���� ���� ������ ���� üũ
					// ������ �� ������ ������ ��������(�� 4�� ����) �������� ȹ�� �� 10�� �Ŀ� ������Ǿ�� �Ѵ�.
					// �̰� ���ϸ�, ���� �׽�Ʈ �� ������ ������ �ʴ´�.
					int i = 0;
					while (i < 4)
					{
						// �ش� ��ġ�� �������� ���� ��� �Ʒ� ����
						if (NowRoom->m_dwItemCreateTick[i] > 0)
						{
							// ���� �ð���, �ش� ��ġ�� �������� �Ҹ�� �ð����� ���� 10�ʰ� �Ǿ�����
							if ((NowTime - NowRoom->m_dwItemCreateTick[i]) >= 10000)
							{
								// �Ǿ��ٸ�, ������ �����Ǿ����� �ð��� 0���� �ʱ�ȭ
								NowRoom->m_dwItemCreateTick[i] = 0;

								// 10�ʰ� �Ǿ��ٸ�, �ش� ��ġ�� źâ ������ ����
								NowRoom->CreateItem(g_Data_ItemPoint_Playzone[i][0],
									g_Data_ItemPoint_Playzone[i][1], eu_ITEM_TYPE::CARTRIDGE, 2);
							}
						}

						++i;
					}
				}
			}

			++itor_Now;
		}

		ReleaseSRWLockShared(&m_Room_Umap_srwl);	// ----- Room Umap Shared ���
	}

	// ���� ����� �� ����
	// Game �����忡�� �������� ȣ��ȴ�.
	//
	// Parameter : (out)int�� �迭(������ �� ��ȣ ������), (out)������ ���� ��
	// return : ����
	void CBattleServer_Room::GameLoop_RoomDelete(int DeleteRoomNo[], int* Index)
	{
		int TempIndex = *Index;

		// Step 1. �� ����
		// Step 2. ä�ü����� �� ���� ��Ŷ ������

		// Index�� 0���� ũ�ٸ�, ������ ���� �ִ� ��. 		
		if (TempIndex > 0)
		{
			// Step 1. �� ����
			int Index_B = 0;

			AcquireSRWLockExclusive(&m_Room_Umap_srwl);	// ----- Room Umap Exclusive ��		

			while (Index_B < TempIndex)
			{
				// 1. �˻�
				auto FindRoom = m_Room_Umap.find(DeleteRoomNo[Index_B]);

				// ���⼭ ������ �ȵ�. �� ������ �� �������� �ۿ� ���ϱ� ������
				if (FindRoom == m_Room_Umap.end())
					g_BattleServer_Room_Dump->Crash();

				// 2. stRoom* ����
				m_Room_Pool->Free(FindRoom->second);

				// 3. Erase
				m_Room_Umap.erase(FindRoom);

				InterlockedDecrement(&m_lPlayRoomCount);
				InterlockedDecrement(&m_lNowTotalRoomCount);

				++Index_B;
			}

			ReleaseSRWLockExclusive(&m_Room_Umap_srwl);	// ----- Room Umap Exclusive ���


			// Step 2. ä�ü����� �� ���� ��Ŷ ������
			// ä�ü����� ����Ǿ� ���� ���
			if (m_Chat_LanServer->m_bLoginCheck == true)
			{
				Index_B = 0;

				// ä�ü��� ���� ID
				ULONGLONG ChatSessionID = m_Chat_LanServer->m_ullSessionID;

				while (Index_B < TempIndex)
				{
					UINT ReqSequence = InterlockedIncrement(&m_Chat_LanServer->m_uiReqSequence);

					// ��Ŷ �����
					CProtocolBuff_Lan* SendBuff = CProtocolBuff_Lan::Alloc();

					WORD Type = en_PACKET_CHAT_BAT_REQ_DESTROY_ROOM;

					SendBuff->PutData((char*)&Type, 2);
					SendBuff->PutData((char*)&m_iServerNo, 4);
					SendBuff->PutData((char*)&DeleteRoomNo[Index_B], 4);
					SendBuff->PutData((char*)&ReqSequence, 4);

					// ��Ŷ ������
					m_Chat_LanServer->SendPacket(ChatSessionID, SendBuff);

					++Index_B;
				}
			}

		}

	}


	// -----------------------
	// ��Ŷ ��ó�� �Լ�
	// -----------------------

	// Login ��Ŷ�� ���� ���� ó�� (��ū üũ ��..)
	//
	// Parameter : DB_WORK_LOGIN*
	// return : ����
	void CBattleServer_Room::Auth_LoginPacket_AUTH(DB_WORK_LOGIN* DBData)
	{
		// 1. ClientKey üũ
		CGameSession* NowPlayer = (CGameSession*)DBData->pPointer;

		// �ٸ��� �̹� ������ ������ �Ǵ�. ���ɼ� �ִ� ��Ȳ
		if (NowPlayer->m_int64ClientKey != DBData->m_i64UniqueKey)
			return;


		// 2. Json������ �Ľ��ϱ� (UTF-16)
		GenericDocument<UTF16<>> Doc;
		Doc.Parse(DBData->m_tcResponse);

		int iResult = Doc[_T("result")].GetInt();


		// 3. DB ��û ��� Ȯ��
		// ����� 1�� �ƴ϶��, 
		if (iResult != 1)
		{
			// �̹� ���� ��Ŷ�� �� �������� üũ
			if (NowPlayer->m_bLogoutFlag == true)
				return;

			// �ٸ� HTTP ��û�� ���� ��Ŷ�� �� ������ ���ϵ��� �÷��� ����.
			NowPlayer->m_bLogoutFlag = true;

			// ���� �α� ���
			// �α� ��� (�α� ���� : ����)
			g_BattleServer_RoomLog->LogSave(false, L"CBattleServer_Room", CSystemLog::en_LogLevel::LEVEL_ERROR,
				L"Auth_LoginPacket_AUTH()--> DB Result Error!! AccoutnNo : %lld, Error : %d", NowPlayer->m_Int64AccountNo, iResult);

			WORD Type = en_PACKET_CS_GAME_RES_LOGIN;
			BYTE Result = 2;			

			// ����� -10�� ��� (ȸ������ ��ü�� �ȵǾ� ����)
			if (iResult == -10)
				InterlockedIncrement(&m_lQuery_Result_Not_Find);

			// �� �� ��Ÿ ������ ���
			else
				InterlockedIncrement(&m_lTempError);

			CProtocolBuff_Net* SendBuff = CProtocolBuff_Net::Alloc();

			SendBuff->PutData((char*)&Type, 2);
			SendBuff->PutData((char*)&DBData->AccountNo, 8);
			SendBuff->PutData((char*)&Result, 1);

			// ������ ����
			NowPlayer->SendPacket(SendBuff, TRUE);
			return;
		}

		// 4. ��ūŰ ��
		const TCHAR* tDBToekn = Doc[_T("sessionkey")].GetString();

		char DBToken[64];
		int len = (int)_tcslen(tDBToekn);
		WideCharToMultiByte(CP_UTF8, 0, tDBToekn, (int)_tcslen(tDBToekn), DBToken, len, NULL, NULL);

		if (memcmp(DBToken, NowPlayer->m_cSessionKey, 64) != 0)
		{
			// �̹� ���� ��Ŷ�� �� �������� üũ
			if (NowPlayer->m_bLogoutFlag == true)
				return;

			// ���� �α� ���
			// �α� ��� (�α� ���� : ����)
			g_BattleServer_RoomLog->LogSave(false, L"CBattleServer_Room", CSystemLog::en_LogLevel::LEVEL_ERROR,
				L"Auth_LoginPacket_AUTH()--> SessionKey Error!! AccoutnNo : %lld", NowPlayer->m_Int64AccountNo);

			// �ٸ� HTTP ��û�� ���� ��Ŷ�� �� ������ ���ϵ��� �÷��� ����.
			NowPlayer->m_bLogoutFlag = true;

			// ��ū�� �ٸ���� Result 3(����Ű ����)�� ������.
			InterlockedIncrement(&m_lTokenError);

			WORD Type = en_PACKET_CS_GAME_RES_LOGIN;
			BYTE Result = 3;

			CProtocolBuff_Net* SendBuff = CProtocolBuff_Net::Alloc();

			SendBuff->PutData((char*)&Type, 2);
			SendBuff->PutData((char*)&DBData->AccountNo, 8);
			SendBuff->PutData((char*)&Result, 1);

			// ������ ����
			NowPlayer->SendPacket(SendBuff, TRUE);
			return;
		}
		

		// 5. �г��� ����
		const TCHAR* TempNick = Doc[_T("nick")].GetString();
		ZeroMemory(&NowPlayer->m_tcNickName, sizeof(NowPlayer->m_tcNickName));
		StringCchCopy(NowPlayer->m_tcNickName, _Mycountof(NowPlayer->m_tcNickName), TempNick);


		// 6. Contents ������ ���õƴٸ� ���� ��Ŷ ����
		NowPlayer->m_lLoginHTTPCount++;

		if (NowPlayer->m_lLoginHTTPCount == 2)
		{
			// �α��� ��Ŷ ó�� Flag ����
			NowPlayer->m_bLoginFlag = true;

			NowPlayer->m_euDebugMode = eu_USER_TYPE_DEBUG::LOGIN_OK;

			// ���� ���� ��Ŷ ����
			CProtocolBuff_Net* SendBuff = CProtocolBuff_Net::Alloc();

			WORD Type = en_PACKET_CS_GAME_RES_LOGIN;
			BYTE Result = 1;

			SendBuff->PutData((char*)&Type, 2);
			SendBuff->PutData((char*)&DBData->AccountNo, 8);
			SendBuff->PutData((char*)&Result, 1);

			NowPlayer->SendPacket(SendBuff);
		}
	}

	// Login ��Ŷ�� ���� Contents ���� ��������
	//
	// Parameter : DB_WORK_LOGIN*
	// return : ����
	void CBattleServer_Room::Auth_LoginPacket_Info(DB_WORK_LOGIN_CONTENTS* DBData)
	{
		// 1. ClientKey üũ
		CGameSession* NowPlayer = (CGameSession*)DBData->pPointer;

		// �ٸ��� �̹� ������ ������ �Ǵ�. ���ɼ� �ִ� ��Ȳ
		if (NowPlayer->m_int64ClientKey != DBData->m_i64UniqueKey)
			return;

		// 2. Json������ �Ľ��ϱ� (UTF-16)
		GenericDocument<UTF16<>> Doc;
		Doc.Parse(DBData->m_tcResponse);

		int iResult = Doc[_T("result")].GetInt();


		// 3. DB ��û ��� Ȯ��
		// ����� 1�� �ƴ϶��, 
		if (iResult != 1)
		{
			// �̹� ���� ��Ŷ�� �� �������� üũ
			if (NowPlayer->m_bLogoutFlag == true)
				return;

			// ���� �α� ���
			// �α� ��� (�α� ���� : ����)
			g_BattleServer_RoomLog->LogSave(false, L"CBattleServer_Room", CSystemLog::en_LogLevel::LEVEL_ERROR,
				L"Auth_LoginPacket_Info()--> DB Result Error!! AccoutnNo : %lld, Error : %d", NowPlayer->m_Int64AccountNo, iResult);

			// �ٸ� HTTP ��û�� ���� ��Ŷ�� �� ������ ���ϵ��� �÷��� ����.
			NowPlayer->m_bLogoutFlag = true;

			WORD Type = en_PACKET_CS_GAME_RES_LOGIN;
			BYTE Result = 2;

			// ����� -10�� ��� (ȸ������ ��ü�� �ȵǾ� ����)
			if (iResult == -10)
				InterlockedIncrement(&m_lQuery_Result_Not_Find);

			// �� �� ��Ÿ ������ ���
			else
				InterlockedIncrement(&m_lTempError);

			CProtocolBuff_Net* SendBuff = CProtocolBuff_Net::Alloc();

			SendBuff->PutData((char*)&Type, 2);
			SendBuff->PutData((char*)&DBData->AccountNo, 8);
			SendBuff->PutData((char*)&Result, 1);

			// ������ ����
			NowPlayer->SendPacket(SendBuff, TRUE);
			return;
		}

		// 4. ������� ���� �������� ��Ŷ ã�ƿ� ��.

		// ���� ����
		NowPlayer->m_iRecord_PlayCount = Doc[_T("playcount")].GetInt();	// �÷��� Ƚ��
		NowPlayer->m_iRecord_PlayTime = Doc[_T("playtime")].GetInt();	// �÷��� �ð� �ʴ���
		NowPlayer->m_iRecord_Kill = Doc[_T("kill")].GetInt();			// ���� Ƚ��
		NowPlayer->m_iRecord_Die = Doc[_T("die")].GetInt();				// ���� Ƚ��
		NowPlayer->m_iRecord_Win = Doc[_T("win")].GetInt();				// �����¸� Ƚ��

		// 5. ������ �α��� HTTP ��û�� �Ϸ�Ǿ��ٸ� ���� ��Ŷ ����
		NowPlayer->m_lLoginHTTPCount++;

		if (NowPlayer->m_lLoginHTTPCount == 2)
		{
			// �α��� ��Ŷ ó�� Flag ����
			NowPlayer->m_bLoginFlag = true;

			NowPlayer->m_euDebugMode = eu_USER_TYPE_DEBUG::LOGIN_OK;

			// ���� ���� ��Ŷ ����
			CProtocolBuff_Net* SendBuff = CProtocolBuff_Net::Alloc();

			WORD Type = en_PACKET_CS_GAME_RES_LOGIN;
			BYTE Result = 1;

			SendBuff->PutData((char*)&Type, 2);
			SendBuff->PutData((char*)&DBData->AccountNo, 8);
			SendBuff->PutData((char*)&Result, 1);

			NowPlayer->SendPacket(SendBuff);
		}

	}
	   





	// -----------------------
	// AccountNo �ڷᱸ�� ���� �Լ�
	// -----------------------

	// AccountNo �ڷᱸ���� ������ �߰��ϴ� �Լ�
	//
	// Parameter : AccountNo, CGameSession*
	// return : ���� �� true
	//		  : ���� �� false
	bool CBattleServer_Room::InsertAccountNoFunc(INT64 AccountNo, CGameSession* InsertPlayer)
	{

		AcquireSRWLockExclusive(&m_AccountNo_Umap_srwl);		// AccountNo uamp Exclusive ��

		// 1. Insert
		auto ret = m_AccountNo_Umap.insert(make_pair(AccountNo, InsertPlayer));

		ReleaseSRWLockExclusive(&m_AccountNo_Umap_srwl);		// AccountNo uamp Exclusive ���

		// 2. �ߺ�Ű�� �� false ����
		if (ret.second == false)
			return false;

		return true;
	}

	// AccountNo �ڷᱸ������ ���� �˻� ��, �ش� �������� Disconenct �ϴ� �Լ�
	//
	// Parameter : AccountNo
	// return : ����
	void CBattleServer_Room::DisconnectAccountNoFunc(INT64 AccountNo)
	{
		AcquireSRWLockShared(&m_AccountNo_Umap_srwl);		// AccountNo uamp Shared ��

		// 1. �˻�
		auto ret = m_AccountNo_Umap.find(AccountNo);

		// 2. ���� ������ �� �׳� ����.
		// ���� �� �ɰ� ������ ����, �̹� ������ �������� ���� �ֱ� ������
		// �������� ��Ȳ���� �Ǵ�.
		if (ret == m_AccountNo_Umap.end())
		{
			ReleaseSRWLockShared(&m_AccountNo_Umap_srwl);		// AccountNo uamp Shared ���
			return;
		}

		// 3. Auth��� ��� Disconnect
		if (ret->second->m_euModeType == eu_PLATER_MODE::AUTH)
			ret->second->Disconnect();

		// Auth�� �ƴ϶��, Game����̰ų� LogOut�̴�, Game���� �̰���Ų��.
		else
			InsertOverlapFunc(ret->second->m_int64ClientKey, ret->second);

		ReleaseSRWLockShared(&m_AccountNo_Umap_srwl);		// AccountNo uamp Shared ���
	}


	// AccountNo �ڷᱸ������ ������ �����ϴ� �Լ�
	//
	// Parameter : AccountNo
	// return : ���� �� true
	//		  : ���� �� false
	bool CBattleServer_Room::EraseAccountNoFunc(INT64 AccountNo)
	{
		AcquireSRWLockExclusive(&m_AccountNo_Umap_srwl);		// AccountNo uamp Exclusive ��

		// 1. �˻�
		auto ret = m_AccountNo_Umap.find(AccountNo);

		// 2. ���� ������ �� false ����
		if (ret == m_AccountNo_Umap.end())
		{
			ReleaseSRWLockExclusive(&m_AccountNo_Umap_srwl);		// AccountNo uamp Exclusive ���
			return false;
		}

		// 3. �ִ� ������� Erase
		m_AccountNo_Umap.erase(ret);

		ReleaseSRWLockExclusive(&m_AccountNo_Umap_srwl);		// AccountNo uamp Shared ���

		return true;
	}







	// -----------------------
	// DBWrite ī��Ʈ �ڷᱸ�� ���� �Լ�
	// -----------------------

	// DBWrite ī��Ʈ ���� �ڷᱸ���� ������ �߰��ϴ� �Լ�
	//
	// Parameter : AccountNo
	// return : ���� �� true
	//		  : ����(Ű �ߺ�) �� false
	bool CBattleServer_Room::InsertDBWriteCountFunc(INT64 AccountNo)
	{
		AcquireSRWLockExclusive(&m_DBWrite_Umap_srwl);		// ----- DBWrite Umap Exclusive ��

		// 1. �߰�
		auto ret = m_DBWrite_Umap.insert(make_pair(AccountNo, 1));

		// �߰� ���� �� (�ߺ� Ű) return false
		if (ret.second == false)
		{
			ReleaseSRWLockExclusive(&m_DBWrite_Umap_srwl);		// ----- DBWrite Umap Exclusive ���
			return false;
		}

		ReleaseSRWLockExclusive(&m_DBWrite_Umap_srwl);		// ----- DBWrite Umap Exclusive ���
		return true;
	}

	// DBWrite ī��Ʈ ���� �ڷᱸ������ ������ �˻� ��, 
	// ī��Ʈ(Value)�� 1 �����ϴ� �Լ�
	//
	// Parameter : AccountNo
	// return : ����
	void CBattleServer_Room::AddDBWriteCountFunc(INT64 AccountNo)
	{
		AcquireSRWLockExclusive(&m_DBWrite_Umap_srwl);		// ----- DBWrite Umap Exclusive ��

		// 1. �˻�
		auto ret = m_DBWrite_Umap.find(AccountNo);

		// ������ ũ����
		if (ret == m_DBWrite_Umap.end())
			g_BattleServer_Room_Dump->Crash();

	
		// 2. Value�� �� 1 ����
		++ret->second;

		ReleaseSRWLockExclusive(&m_DBWrite_Umap_srwl);		// ----- DBWrite Umap Exclusive ���
	}

	// DBWrite ī��Ʈ ���� �ڷᱸ������ ������ �˻� ��, 
	// ī��Ʈ(Value)�� 1 ���ҽ�Ű�� �Լ�
	// ���� �� 0�̵Ǹ� Erase�Ѵ�.
	//
	// Parameter : AccountNo
	// return : ���� �� true
	//		  : �˻� ���� �� false
	bool CBattleServer_Room::MinDBWriteCountFunc(INT64 AccountNo)
	{
		AcquireSRWLockExclusive(&m_DBWrite_Umap_srwl);		// ----- DBWrite Umap Exclusive ��

		// 1. �˻�
		auto ret = m_DBWrite_Umap.find(AccountNo);

		// ������ return false
		if (ret == m_DBWrite_Umap.end())
		{
			ReleaseSRWLockExclusive(&m_DBWrite_Umap_srwl);		// ----- DBWrite Umap Exclusive ���
			return false;
		}

		// �̹� Value�� 0�̸� ũ����
		if(ret->second == 0)
			g_BattleServer_Room_Dump->Crash();


		// 2. Value�� �� 1 ����
		--ret->second;


		// 3. ���� �� 0�̸� Erase
		if (ret->second == 0)
			m_DBWrite_Umap.erase(ret);

		ReleaseSRWLockExclusive(&m_DBWrite_Umap_srwl);		// ----- DBWrite Umap Exclusive ���

		return true;
	}








	// ---------------------------------
	// Auth����� �� ���� �ڷᱸ�� ����
	// ---------------------------------

	// ���� Room �ڷᱸ���� Insert�ϴ� �Լ�
	//
	// Parameter : RoomNo, stRoom*
	// return : ���� �� true
	//		  : ����(�ߺ� Ű) �� false	
	bool CBattleServer_Room::InsertRoomFunc(int RoomNo, stRoom* InsertRoom)
	{
		AcquireSRWLockExclusive(&m_Room_Umap_srwl);		// ----- Room �ڷᱸ�� Exclusive �� 

		// 1. insert
		auto ret = m_Room_Umap.insert(make_pair(RoomNo, InsertRoom));

		ReleaseSRWLockExclusive(&m_Room_Umap_srwl);		// ----- Room �ڷᱸ�� Exclusive ��� 

		// 2. �ߺ�Ű��� false ����
		if (ret.second == false)
			return false;

		return true;
	}




	// -----------------------
	// �ߺ��α��� �ڷᱸ��(list) ���� �Լ�
	// -----------------------

	// �ߺ��α��� �ڷᱸ��(list)�� Insert
	//
	// Parameter : ClientKey, CGameSession*
	// return : ����
	void CBattleServer_Room::InsertOverlapFunc(INT64 ClientKey, CGameSession* InsertPlayer)
	{		
		AcquireSRWLockExclusive(&m_Overlap_list_srwl);		// Exclusive ��	-----------------

		// Push
		m_Overlap_list.push_front(make_pair(ClientKey, InsertPlayer));

		ReleaseSRWLockExclusive(&m_Overlap_list_srwl);		// Exclusive �� ����-------------
	}





	// -----------------------
	// �����Լ�
	// -----------------------

	// AuthThread���� 1Loop���� 1ȸ ȣ��.
	// 1�������� ���������� ó���ؾ� �ϴ� ���� �Ѵ�.
	// 
	// parameter : ����
	// return : ����
	void CBattleServer_Room::OnAuth_Update()
	{
		// ------------------- HTTP ��� ��, ��ó��
		AuthLoop_HTTP();


		// ------------------- �� ���� ó��
		AuthLoop_RoomLogic();		


		// ------------------- �����
		// !! ���� ���� ���� !!
		// - [���� ���� ���� ������� �� �� �� < �ִ� ������ �� �ִ� �� ��] ����
		// - [���� ���� �� < �ִ� ������ �� �ִ� ���� ��] ����

		// ������ ������ �α���, ä�� �� Ŭ�� �α��� ������ ���� ������.
		// �ϴ�, �����Ϳ��� ����Ǿ� �־�� ��Ʋ���� No�� �޴´�.
		// �׸��� ä�ü������Ե� ������ �� �Ŀ��� ���������� ���� �����Ǿ��ٰ� �� �� �ִ�.
		// ��, �� ���� ������ ������ �Ǿ�߸� ����.
		if (m_Master_LanClient->m_bLoginCheck == true &&
			m_Chat_LanServer->m_bLoginCheck == true)
		{
			// �� �����ӿ� �ִ� m_iLoopCreateRoomCount���� �� ����
			int LoopCount = m_stConst.m_iLoopCreateRoomCount;

			while (LoopCount > 0)
			{
				// 1. ���� ������� �� ��� üũ
				if (m_lNowTotalRoomCount < m_stConst.m_lMaxTotalRoomCount)
				{
					// 2. ���� ���� �� üũ
					if (m_lNowWaitRoomCount < m_stConst.m_lMaxWaitRoomCount)
					{
						// ������� ���� ����� ���� ����	
						// �� ���� �Լ� ȣ��
						RoomCreate();	
						--LoopCount;
					}

					// ���� ���� ���� �̹� max��� �� �ȸ���� ������.
					else
						break;
				}

				// ���� ������� �� ����� �̹� max��� �� �ȸ���� ������.
				else
					break;
			}
		}
		

		// -------------------- ������ ���� Ȥ�� ä�� ������ �׾��� ���

		// Wait������ ���� �ı��Ѵ�.
		//
		// !! ������ �������� �� �ı� ��Ŷ�� ������ ���� : �Ʒ� ���� !!
		// !! ä�� �������� �� �ı� ��Ŷ�� ������ ���� : Play���� ���� �ı��� ��. !!
		//
		// Play�� ���ӽ����忡�� �����ϸ�, �ش� ���� �ο��� 0���� �Ǹ� ���� �����Ѵ�.
		else if (m_Master_LanClient->m_bLoginCheck == false ||
			m_Chat_LanServer->m_bLoginCheck == false)
		{
			AuthLoop_ServerDie();
		}	


		// ------------------- ��ū ��߱�
		// ä�� �� ������ ���� ä�� ������ ��ū ����.
		// �̰� �Ϸ�Ǹ�, �˾Ƽ� ������ �������Ե� ��ū�� ���޵ȴ�.
		m_Chat_LanServer->Packet_TokenChange_Req();
	}

	// GameThread���� 1Loop���� 1ȸ ȣ��.
	// 1�������� ���������� ó���ؾ� �ϴ� ���� �Ѵ�.
	// 
	// parameter : ����
	// return : ����
	void CBattleServer_Room::OnGame_Update()
	{
		// ------------------- ���� ����� �ߺ� �α��� ���� ����
		GameLoop_OverlapLogin();
		

		// ------------------- �� ����

		// �̹� �����ӿ� ������ �� �޾Ƶα�.
		// �� �����ӿ� �ִ� 100���� �� ���� ����
		int DeleteRoomNo[100];
		int Index = 0;

		GameLoop_RoomLogic(DeleteRoomNo, &Index);


		// ------------------- �� ����
		GameLoop_RoomDelete(DeleteRoomNo, &Index);
	}

	// ���ο� ���� ���� ��, Auth���� ȣ��ȴ�.
	//
	// parameter : ������ ������ IP, Port
	// return false : Ŭ���̾�Ʈ ���� �ź�
	// return true : ���� ���
	bool CBattleServer_Room::OnConnectionRequest(TCHAR* IP, USHORT port)
	{
		return true;
	}

	// ��Ŀ �����尡 ��� �� ȣ��Ǵ� �Լ�.
	// GQCS �ٷ� �ϴܿ��� ȣ��
	// 
	// parameter : ����
	// return : ����
	void CBattleServer_Room::OnWorkerThreadBegin()
	{

	}

	// ��Ŀ �����尡 ���� �� ȣ��Ǵ� �Լ�
	// GQCS �ٷ� ������ ȣ��
	// 
	// parameter : ����
	// return : ����
	void CBattleServer_Room::OnWorkerThreadEnd()
	{

	}

	// ���� �߻� �� ȣ��Ǵ� �Լ�.
	//
	// parameter : ���� �ڵ�(���� ������ �����ڵ�� WinGetLastError() �Լ��� ��� ����. ���� ��� 0�� ���ϵ�)
	//			 : ���� �ڵ忡 ���� ��Ʈ��
	// return : ����
	void CBattleServer_Room::OnError(int error, const TCHAR* errorStr)
	{
		// �α� ��� (�α� ���� : ����)
		g_BattleServer_RoomLog->LogSave(false, L"CBattleServer_Room", CSystemLog::en_LogLevel::LEVEL_ERROR, L"%s (ErrorCode : %d)",
			errorStr, error);

		// ����, �������� ���� ����.
	}

	   	  

	// -----------------
	// �����ڿ� �Ҹ���
	// -----------------
	CBattleServer_Room::CBattleServer_Room()
		:CMMOServer()
	{	
		srand((UINT)time(NULL));	

		// �� ���� ��ū �����α�
		for (int i = 0; i < 64; ++i)
		{
			for (int h = 0; h < 32; ++h)
			{
				m_cRoomEnterToken[i][h] = (rand() % 128) + 1;
			}
		}	

		// ��Ʋ���� ���� ���� ��ū ����� �α�
		for (int i = 0; i < 32; ++i)
		{
			m_cConnectToken_Now[i] = (rand() % 128) + 1;
		}

		// ����͸� ������ ����ϱ� ���� LanClient �����Ҵ�
		m_Monitor_LanClient = new CGame_MinitorClient;
		m_Monitor_LanClient->ParentSet(this);

		// ������ ������ ����ϱ� ���� LanClient �����Ҵ�
		m_Master_LanClient = new CBattle_Master_LanClient;
		m_Master_LanClient->ParentSet(this);

		// ä�ü����� ����Ǵ� LanServer �����Ҵ�
		m_Chat_LanServer = new CBattle_Chat_LanServer;
		m_Chat_LanServer->SetMasterClient(m_Master_LanClient, this);


		// ------------------- Config���� ����		
		if (SetFile(&m_stConfig) == false)
			g_BattleServer_Room_Dump->Crash();

		// ------------------- �α� ������ ���� ����
		g_BattleServer_RoomLog->SetDirectory(L"CBattleServer_Room");
		g_BattleServer_RoomLog->SetLogLeve((CSystemLog::en_LogLevel)m_stConfig.LogLevel);
				


		// TLS �����Ҵ�
		m_Room_Pool = new CMemoryPoolTLS<stRoom>(0, false);	
		m_Item_Pool = new CMemoryPoolTLS<stRoomItem>(0, false);

		// reserve ����
		m_AccountNo_Umap.reserve(m_stConfig.MaxJoinUser);
		m_Room_Umap.reserve(m_stConst.m_lMaxTotalRoomCount);
		m_DBWrite_Umap.reserve(m_stConfig.MaxJoinUser);

		//SRW�� �ʱ�ȭ
		InitializeSRWLock(&m_AccountNo_Umap_srwl);
		InitializeSRWLock(&m_Room_Umap_srwl);
		InitializeSRWLock(&m_DBWrite_Umap_srwl);
		InitializeSRWLock(&m_Overlap_list_srwl);
		InitializeSRWLock(&m_ServerEnterToken_srwl);

		// ���� ������ ���Ǵ�, �Ÿ� 1�� ������.
		m_fFire1_Damage = g_Data_HitDamage / (float)15;	// �Ѿ� ������


		// next_permutation�� �̿��� ������ ���� ������ �̸� ������.
		// ����� ��, �̰� �� �ϳ��� �������� �̾Ƽ� �濡�� �Ҵ����ش�.
		vector<int> v(4);

		for (int i = 0; i < 4; ++i)
			v[i] = i;

		int h = 0;
		do 
		{
			for (int i = 0; i < 4; i++)
				m_arrayRedZoneCreate[h][i] = v[i];

			h++;

		} while (next_permutation(v.begin(), v.end()));

		// ������ Ÿ�Կ� ���� Ȱ�� ��ġ �̸� ������.
		m_arrayRedZoneRange[0][0][0] = 0;	// Left x1
		m_arrayRedZoneRange[0][0][1] = 0;	// Left y1
		m_arrayRedZoneRange[0][1][0] = 153;	// Left x2
		m_arrayRedZoneRange[0][1][1] = 50;	// Left y2

		m_arrayRedZoneRange[1][0][0] = 0;	// Right x1
		m_arrayRedZoneRange[1][0][1] = 115;	// Right y1
		m_arrayRedZoneRange[1][1][0] = 153;	// Right x2
		m_arrayRedZoneRange[1][1][1] = 170;	// Right y2

		m_arrayRedZoneRange[2][0][0] = 0;	// Top x1
		m_arrayRedZoneRange[2][0][1] = 0;	// Top y1
		m_arrayRedZoneRange[2][1][0] = 44;	// Top x2
		m_arrayRedZoneRange[2][1][1] = 170;	// Top y2

		m_arrayRedZoneRange[3][0][0] = 102;	// bottom x1
		m_arrayRedZoneRange[3][0][1] = 0;	// bottom y1
		m_arrayRedZoneRange[3][1][0] = 153;	// bottom x2
		m_arrayRedZoneRange[3][1][1] = 170;	// bottom y2

		// ������ ������ Ȱ�� �� ��������
		m_arrayLastRedZoneSafeRange[0][0][0] = 47;	//Level1 �������� x1
		m_arrayLastRedZoneSafeRange[0][0][1] = 51;	//Level1 �������� y1
		m_arrayLastRedZoneSafeRange[0][1][0] = 75;	//Level1 �������� x2
		m_arrayLastRedZoneSafeRange[0][1][1] = 84;	//Level1 �������� y2

		m_arrayLastRedZoneSafeRange[1][0][0] = 47;	//Level2 �������� x1
		m_arrayLastRedZoneSafeRange[1][0][1] = 82;	//Level2 �������� y1
		m_arrayLastRedZoneSafeRange[1][1][0] = 75;	//Level2 �������� x2
		m_arrayLastRedZoneSafeRange[1][1][1] = 112;	//Level2 �������� y2

		m_arrayLastRedZoneSafeRange[2][0][0] = 76;	//Level3 �������� x1
		m_arrayLastRedZoneSafeRange[2][0][1] = 51;	//Level3 �������� y1
		m_arrayLastRedZoneSafeRange[2][1][0] = 101;	//Level3 �������� x2
		m_arrayLastRedZoneSafeRange[2][1][1] = 85;	//Level3 �������� y2

		m_arrayLastRedZoneSafeRange[3][0][0] = 74;	//Level4 �������� x1
		m_arrayLastRedZoneSafeRange[3][0][1] = 84;	//Level4 �������� y1
		m_arrayLastRedZoneSafeRange[3][1][0] = 100;	//Level4 �������� x2
		m_arrayLastRedZoneSafeRange[3][1][1] = 114;	//Level4 �������� y2
	}

	CBattleServer_Room::~CBattleServer_Room()
	{
		// ����͸� ������ ����ϴ� LanClient ��������
		delete m_Monitor_LanClient;

		// ������ ������ ����ϴ� LanClient ���� ����
		delete m_Master_LanClient;

		// TLS ���� ����
		delete m_Room_Pool;
		delete m_Item_Pool;
	}

}

// ----------------------------------------
// 
// ������ ������ ����Ǵ� LanClient
//
// ----------------------------------------
namespace Library_Jingyu
{

	// -----------------------
	// ��Ŷ ó�� �Լ�
	// -----------------------

	// �����Ϳ��� ���� �α��� ��û ����
	//
	// Parameter : SessionID, CProtocolBuff_Lan*
	// return : ����
	void CBattle_Master_LanClient::Packet_Login_Res(ULONGLONG SessionID, CProtocolBuff_Lan* Payload)
	{
		// 1. Connect �������� Ȯ��
		if (SessionID != m_ullSessionID)
			g_BattleServer_Room_Dump->Crash();

		if(m_ullSessionID == 0xffffffffffffffff)
			g_BattleServer_Room_Dump->Crash();

		if (m_bLoginCheck == true)
			g_BattleServer_Room_Dump->Crash();

		// 2. ������
		int ServerNo;
		Payload->GetData((char*)&ServerNo, 4);


		// 3. ���� ��ȣ ����
		m_BattleServer_this->m_iServerNo = ServerNo;	


		// 4. ���� �α����̱� ������ ��Ʋ���� ���� ��ū ������
		UINT ReqSequence = uiReqSequence;

		CProtocolBuff_Lan* SendBuff = CProtocolBuff_Lan::Alloc();

		WORD Type = en_PACKET_BAT_MAS_REQ_CONNECT_TOKEN;

		SendBuff->PutData((char*)&Type, 2);
		SendBuff->PutData((char*)m_BattleServer_this->m_cConnectToken_Now, 32);
		SendBuff->PutData((char*)&uiReqSequence, 4);

		SendPacket(SessionID, SendBuff);

		
		// 5. ������ ���� �־��ٸ�, ������ ������ �� ���� ��Ŷ ����. (������, ���� ����� ��� �ı���.)
		// Shared�� �����ص� ������ ����!
		// 1. OnAuth_Update�� �� ���� üũ �κ���, Ready������ �游 �����ϱ� ������ ������ �� ���� ���ɼ� X
		// 2. ���������� OnGame_Update�� Play������ �游 ����.
		// 3. Ŭ���̾�Ʈ�� �� ���� ��Ŷ�� ��Ʋ�� ������ ���ؼ���, �ش� ���� �����Ϳ� �־�� ��. 
		// �ٵ�, �� ��Ŷ�� �����Ϳ��� ���� ������ �����̱� ������, ���⼭ ��Ŷ�� ������ ���� ���� ����.
		/*
		AcquireSRWLockShared(&m_BattleServer_this->m_Room_Umap_srwl);	// ----- �� Shared ��

		// ���� �����Ѵٸ�
		if (m_BattleServer_this->m_Room_Umap.size() > 0)
		{
			auto itor_Now = m_BattleServer_this->m_Room_Umap.begin();
			auto itor_End = m_BattleServer_this->m_Room_Umap.end();

			WORD Type = en_PACKET_BAT_MAS_REQ_CREATED_ROOM;

			// ��ȸ
			while (itor_Now != itor_End)
			{
				// ��� ������ ���̶��, �����Ϳ��� �� ���� ��Ŷ ����
				if (itor_Now->second->m_iRoomState == CBattleServer_Room::eu_ROOM_STATE::WAIT_ROOM)
				{
					CBattleServer_Room::stRoom* NowRoom = itor_Now->second;

					UINT ReqSequence = InterlockedIncrement(&uiReqSequence);

					// ��Ŷ �����
					CProtocolBuff_Lan* SendBuff = CProtocolBuff_Lan::Alloc();

					SendBuff->PutData((char*)&Type, 2);
					SendBuff->PutData((char*)&NowRoom->m_iBattleServerNo, 4);
					SendBuff->PutData((char*)&NowRoom->m_iRoomNo, 4);
					SendBuff->PutData((char*)&NowRoom->m_iMaxJoinCount, 4);
					SendBuff->PutData((char*)NowRoom->m_cEnterToken, 32);
					SendBuff->PutData((char*)&ReqSequence, 4);

					// ��Ŷ ������
					SendPacket(SessionID, SendBuff);
				}

				++itor_Now;
			}
		}

		ReleaseSRWLockShared(&m_BattleServer_this->m_Room_Umap_srwl);	// ----- �� Shared ���
		*/


		// 6. �α��� ���·� ����
		m_bLoginCheck = true;
	}

	// �ű� ���� ���� ����
	//
	// Parameter : SessionID, CProtocolBuff_Lan*
	// return : ����
	void CBattle_Master_LanClient::Packet_NewRoomCreate_Res(ULONGLONG SessionID, CProtocolBuff_Lan* Payload)
	{
		// ���� �ϴ°� ����..
	}

	// ��ū ����� ����
	//
	// Parameter : SessionID, CProtocolBuff_Lan*
	// return : ����
	void CBattle_Master_LanClient::Packet_TokenChange_Res(ULONGLONG SessionID, CProtocolBuff_Lan* Payload)
	{
		// ���� �ϴ°� ����..
	}

	// �� ���� ����
	//
	// Parameter : SessionID, CProtocolBuff_Lan*
	// return : ����
	void CBattle_Master_LanClient::Packet_RoomClose_Res(ULONGLONG SessionID, CProtocolBuff_Lan* Payload)
	{
		// ���� �ϴ°� ����..
	}

	// �� ���� ����
	//
	// Parameter : RoomNo, AccountNo
	// return : ����
	void CBattle_Master_LanClient::Packet_RoomLeave_Res(ULONGLONG SessionID, CProtocolBuff_Lan* Payload)
	{
		// ���� �ϴ°� ����..
	}




	// -----------------------
	// ä�� Lan ������ ȣ���ϴ� �Լ�
	// -----------------------

	// �����Ϳ���, �ű� ���� ���� ��Ŷ ������
	//
	// Parameter : RoomNo
	// return : ����
	void CBattle_Master_LanClient::Packet_NewRoomCreate_Req(int RoomNo)
	{
		// �����Ϳ� ������ ���� ���¶�� ��Ŷ �Ⱥ�����.
		if (m_bLoginCheck == false)
			return;

		if (m_ullSessionID == 0xffffffffffffffff)
			g_BattleServer_Room_Dump->Crash();


		// 1. �� �˻�
		AcquireSRWLockShared(&m_BattleServer_this->m_Room_Umap_srwl);	// ----- �� Shared ��

		auto FindRoom = m_BattleServer_this->m_Room_Umap.find(RoomNo);

		if (FindRoom == m_BattleServer_this->m_Room_Umap.end())
			g_BattleServer_Room_Dump->Crash();

		CBattleServer_Room::stRoom* NewRoom = FindRoom->second;


		// 2. ��Ŷ �����
		WORD Type = en_PACKET_BAT_MAS_REQ_CREATED_ROOM;
		UINT ReqSequence = InterlockedIncrement(&uiReqSequence);

		CProtocolBuff_Lan* SendBuff = CProtocolBuff_Lan::Alloc();

		SendBuff->PutData((char*)&Type, 2);
		SendBuff->PutData((char*)&NewRoom->m_iBattleServerNo, 4);
		SendBuff->PutData((char*)&RoomNo, 4);
		SendBuff->PutData((char*)&NewRoom->m_iMaxJoinCount, 4);
		SendBuff->PutData(NewRoom->m_cEnterToken, 32);

		SendBuff->PutData((char*)&ReqSequence, 4);

		ReleaseSRWLockShared(&m_BattleServer_this->m_Room_Umap_srwl);	// ----- �� Shared ���


		// 3. �����Ϳ��� Send�ϱ�
		SendPacket(m_ullSessionID, SendBuff);
	}

	// ��ū ��߱� �Լ�
	//
	// Parameter : ����
	// return : ����
	void CBattle_Master_LanClient::Packet_TokenChange_Req()
	{
		// �����Ϳ� ������ ���� ���¶�� ��Ŷ �Ⱥ�����.
		if (m_bLoginCheck == false)
			return;

		if (m_ullSessionID == 0xffffffffffffffff)
			g_BattleServer_Room_Dump->Crash();


		// 1. ��ū ��߱� ��Ŷ �����
		WORD Type = en_PACKET_BAT_MAS_REQ_CONNECT_TOKEN;
		UINT ReqSequence = InterlockedIncrement(&uiReqSequence);

		CProtocolBuff_Lan* SendBuff = CProtocolBuff_Lan::Alloc();

		SendBuff->PutData((char*)&Type, 2);
		SendBuff->PutData(m_BattleServer_this->m_cConnectToken_Now, 32);
		SendBuff->PutData((char*)&ReqSequence, 4);


		// 2. �����Ϳ��� ��Ŷ ������
		SendPacket(m_ullSessionID, SendBuff);
	}

	



	// -----------------------
	// Battle Net ������ ȣ���ϴ� �Լ�
	// -----------------------		
	
	// �����Ϳ���, �� ���� ��Ŷ ������
	//
	// Parameter : RoomNo
	// return : ����
	void CBattle_Master_LanClient::Packet_RoomClose_Req(int RoomNo)
	{
		// �����Ϳ� ������ ���� ���¶�� ��Ŷ �Ⱥ�����.
		if (m_bLoginCheck == false)
			return;

		if (m_ullSessionID == 0xffffffffffffffff)
			g_BattleServer_Room_Dump->Crash();

		// 1. ��Ŷ �����
		WORD Type = en_PACKET_BAT_MAS_REQ_CLOSED_ROOM;
		UINT ReqSequence = InterlockedIncrement(&uiReqSequence);

		CProtocolBuff_Lan* SendBuff = CProtocolBuff_Lan::Alloc();

		SendBuff->PutData((char*)&Type, 2);
		SendBuff->PutData((char*)&RoomNo, 4);
		SendBuff->PutData((char*)&ReqSequence, 4);

		SendPacket(m_ullSessionID, SendBuff);
	}

	// �����Ϳ���, �� ���� ��Ŷ ������
	//
	// Parameter : RoomNo, ClientKey
	// return : ����
	void CBattle_Master_LanClient::Packet_RoomLeave_Req(int RoomNo, INT64 ClientKey)
	{
		// �����Ϳ� ������ ���� ���¶�� ��Ŷ �Ⱥ�����.
		if (m_bLoginCheck == false)
			return;

		if (m_ullSessionID == 0xffffffffffffffff)
			g_BattleServer_Room_Dump->Crash();

		// 1. ��Ŷ �����
		WORD Type = en_PACKET_BAT_MAS_REQ_LEFT_USER;
		UINT ReqSequence = InterlockedIncrement(&uiReqSequence);

		CProtocolBuff_Lan* SendBuff = CProtocolBuff_Lan::Alloc();

		SendBuff->PutData((char*)&Type, 2);
		SendBuff->PutData((char*)&RoomNo, 4);
		SendBuff->PutData((char*)&ClientKey, 8);
		SendBuff->PutData((char*)&ReqSequence, 4);

		SendPacket(m_ullSessionID, SendBuff);
	}





	// -----------------------
	// �ܺο��� ��� ������ �Լ�
	// -----------------------

	// ���� �Լ�
	// ����������, ��ӹ��� CLanClient�� Startȣ��.
	//
	// Parameter : ������ ������ IP, ��Ʈ, ��Ŀ������ ��, Ȱ��ȭ��ų ��Ŀ������ ��, TCP_NODELAY ��� ����(true�� ���)
	// return : ���� �� true , ���� �� falsel 
	bool CBattle_Master_LanClient::ClientStart(TCHAR* ConnectIP, int Port, int CreateWorker, int ActiveWorker, int Nodelay)
	{
		// ������ ������ ����
		if (Start(ConnectIP, Port, CreateWorker, ActiveWorker, Nodelay) == false)
			return false;

		return true;
	}

	// ���� �Լ�
	// ����������, ��ӹ��� CLanClient�� Stopȣ��.
	// �߰���, ���ҽ� ���� ��
	//
	// Parameter : ����
	// return : ����
	void CBattle_Master_LanClient::ClientStop()
	{
		// ������ ������ ���� ����
		Stop();
	}

	// ��Ʋ������ this�� �Է¹޴� �Լ�
	// 
	// Parameter : ��Ʋ ������ this
	// return : ����
	void CBattle_Master_LanClient::ParentSet(CBattleServer_Room* ChatThis)
	{
		m_BattleServer_this = ChatThis;
	}




	// -----------------------
	// �����Լ�
	// -----------------------

	// ��ǥ ������ ���� ���� ��, ȣ��Ǵ� �Լ� (ConnectFunc���� ���� ���� �� ȣ��)
	//
	// parameter : ����Ű
	// return : ����
	void CBattle_Master_LanClient::OnConnect(ULONGLONG SessionID)
	{
		if(m_bLoginCheck == true)
			g_BattleServer_Room_Dump->Crash();

		if (m_ullSessionID != 0xffffffffffffffff)
			g_BattleServer_Room_Dump->Crash();

		m_ullSessionID = SessionID;
		
		while (1)
		{
			// ä�ü����� ����Ǿ��� üũ.
			// ���� �� ���¿���, ������ ������ �α��� ��Ŷ ����.
			if (m_BattleServer_this->m_Chat_LanServer->m_bLoginCheck == true)
			{
				// ������ ����(Lan)�� �α��� ��Ŷ ����
				CProtocolBuff_Lan* SendBuff = CProtocolBuff_Lan::Alloc();

				WORD Type = en_PACKET_BAT_MAS_REQ_SERVER_ON;

				SendBuff->PutData((char*)&Type, 2);

				SendBuff->PutData((char*)m_tcBattleNetServerIP, 32);
				SendBuff->PutData((char*)&m_iBattleNetServerPort, 2);
				SendBuff->PutData(m_BattleServer_this->m_cConnectToken_Now, 32);
				SendBuff->PutData(m_cMasterToken, 32);

				SendBuff->PutData((char*)m_tcChatNetServerIP, 32);
				SendBuff->PutData((char*)&m_iChatNetServerPort, 2);


				SendPacket(SessionID, SendBuff);

				break;
			}

			Sleep(1);
		}
		
	}

	// ��ǥ ������ ���� ���� �� ȣ��Ǵ� �Լ� (InDIsconnect �ȿ��� ȣ��)
	//
	// parameter : ����Ű
	// return : ����
	void CBattle_Master_LanClient::OnDisconnect(ULONGLONG SessionID)
	{
		// SessionID�� �ʱⰪ���� ����
		m_ullSessionID = 0xffffffffffffffff;

		// �� �α��� ���·� ����
		m_bLoginCheck = false;
	}

	// ��Ŷ ���� �Ϸ� �� ȣ��Ǵ� �Լ�.
	//
	// parameter : ���� ����Ű, CProtocolBuff_Lan*
	// return : ����
	void CBattle_Master_LanClient::OnRecv(ULONGLONG SessionID, CProtocolBuff_Lan* Payload)
	{
		// Type �и�
		WORD Type;
		Payload->GetData((char*)&Type, 2);


		// Ÿ�Կ� ���� �б� ó��
		try
		{
			switch (Type)
			{
				// �α��� ��û�� ���� ����
			case en_PACKET_BAT_MAS_RES_SERVER_ON:
				Packet_Login_Res(SessionID, Payload);
				break;
				
				// �ű� ���� ���� ����
			case en_PACKET_BAT_MAS_RES_CREATED_ROOM:
				Packet_NewRoomCreate_Res(SessionID, Payload);
				break;

				// ��ū ����� ����
			case en_PACKET_BAT_MAS_RES_CONNECT_TOKEN:
				Packet_TokenChange_Res(SessionID, Payload);
				break;

				// �� ���� ����
			case en_PACKET_BAT_MAS_RES_CLOSED_ROOM:
				Packet_RoomClose_Res(SessionID, Payload);
				break;

				// �� ���� ����
			case en_PACKET_BAT_MAS_RES_LEFT_USER:
				Packet_RoomLeave_Res(SessionID, Payload);
				break;

				// ���� Ÿ���̸� ũ����
			default:
				TCHAR str[100];
				StringCchPrintf(str, 100, _T("CBattle_Master_LanClient. OnRecv(). TypeError. SessionID : %lld, Type : %d"), SessionID, Type);

				throw CException(str);
				break;
			}

		}
		catch (CException& exc)
		{
			// �α� ��� (�α� ���� : ����)
			g_BattleServer_RoomLog->LogSave(false, L"CBattleServer_Room", CSystemLog::en_LogLevel::LEVEL_ERROR, L"%s",
				(TCHAR*)exc.GetExceptionText());

			g_BattleServer_Room_Dump->Crash();

			// ���� ���� ��û
			//Disconnect(SessionID);
		}
	}

	// ��Ŷ �۽� �Ϸ� �� ȣ��Ǵ� �Լ�
	//
	// parameter : ���� ����Ű, Send �� ������
	// return : ����
	void CBattle_Master_LanClient::OnSend(ULONGLONG SessionID, DWORD SendSize)
	{}

	// ��Ŀ �����尡 ��� �� ȣ��Ǵ� �Լ�.
	// GQCS �ٷ� �ϴܿ��� ȣ��
	// 
	// parameter : ����
	// return : ����
	void CBattle_Master_LanClient::OnWorkerThreadBegin()
	{}

	// ��Ŀ �����尡 ���� �� ȣ��Ǵ� �Լ�
	// GQCS �ٷ� ������ ȣ��
	// 
	// parameter : ����
	// return : ����
	void CBattle_Master_LanClient::OnWorkerThreadEnd()
	{}

	// ���� �߻� �� ȣ��Ǵ� �Լ�.
	//
	// parameter : ���� �ڵ�(���� ������ �����ڵ�� WinGetLastError() �Լ��� ��� ����. ���� ��� 0�� ���ϵ�)
	//			 : ���� �ڵ忡 ���� ��Ʈ��
	// return : ����
	void CBattle_Master_LanClient::OnError(int error, const TCHAR* errorStr)
	{}






	// -----------------------
	// �����ڿ� �Ҹ���
	// -----------------------

	// ������
	CBattle_Master_LanClient::CBattle_Master_LanClient()
	{
		// SessionID �ʱ�ȭ
		m_ullSessionID = 0xffffffffffffffff;

		// Req������ �ʱ�ȭ
		uiReqSequence = 0;

		// �α��� ���� false
		m_bLoginCheck = false;
	}

	// �Ҹ���
	CBattle_Master_LanClient::~CBattle_Master_LanClient()
	{
		// ���� ������ �Ǿ�������, ���� ����
		if (GetClinetState() == true)
			ClientStop();
	}
}

// ---------------
// CGame_MinitorClient
// CLanClienet�� ��ӹ޴� ����͸� Ŭ��
// ---------------
namespace Library_Jingyu
{
	// -----------------------
	// �����ڿ� �Ҹ���
	// -----------------------
	CGame_MinitorClient::CGame_MinitorClient()
		:CLanClient()
	{
		// ����͸� ���� �������� �����带 �����ų �̺�Ʈ ����
		//
		// �ڵ� ���� Event 
		// ���� ���� �� non-signalled ����
		// �̸� ���� Event	
		m_hMonitorThreadExitEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

		m_ullSessionID = 0xffffffffffffffff;
	}

	CGame_MinitorClient::~CGame_MinitorClient()
	{
		// ���� ������ �Ǿ�������, ���� ����
		if (GetClinetState() == true)
			ClientStop();

		// �̺�Ʈ ����
		CloseHandle(m_hMonitorThreadExitEvent);
	}



	// -----------------------
	// �ܺο��� ��� ������ �Լ�
	// -----------------------

	// ���� �Լ�
	// ����������, ��ӹ��� CLanClient�� Startȣ��.
	//
	// Parameter : ������ ������ IP, ��Ʈ, ��Ŀ������ ��, Ȱ��ȭ��ų ��Ŀ������ ��, TCP_NODELAY ��� ����(true�� ���)
	// return : ���� �� true , ���� �� falsel 
	bool CGame_MinitorClient::ClientStart(TCHAR* ConnectIP, int Port, int CreateWorker, int ActiveWorker, int Nodelay)
	{
		// ����͸� ������ ����
		if (Start(ConnectIP, Port, CreateWorker, ActiveWorker, Nodelay) == false)
			return false;

		// ����͸� ������ ���� ������ ������ ����
		m_hMonitorThread = (HANDLE)_beginthreadex(NULL, 0, MonitorThread, this, 0, NULL);

		return true;
	}

	// ���� �Լ�
	// ����������, ��ӹ��� CLanClient�� Stopȣ��.
	// �߰���, ���ҽ� ���� ��
	//
	// Parameter : ����
	// return : ����
	void CGame_MinitorClient::ClientStop()
	{
		// 1. ����͸� ���� �������� ������ ����
		SetEvent(m_hMonitorThreadExitEvent);

		// ���� ���
		if (WaitForSingleObject(m_hMonitorThread, INFINITE) == WAIT_FAILED)
		{
			DWORD Error = GetLastError();
			printf("MonitorThread Exit Error!!! (%d) \n", Error);
		}

		// 2. ������ �ڵ� ��ȯ
		CloseHandle(m_hMonitorThread);

		// 3. ����͸� ������ ���� ����
		Stop();
	}

	// ��Ʋ������ this�� �Է¹޴� �Լ�
	// 
	// Parameter : ��Ʋ ������ this
	// return : ����
	void CGame_MinitorClient::ParentSet(CBattleServer_Room* ChatThis)
	{
		m_BattleServer_this = ChatThis;
	}




	// -----------------------
	// ���ο����� ����ϴ� ��� �Լ�
	// -----------------------

	// ���� �ð����� ����͸� ������ ������ �����ϴ� ������
	UINT	WINAPI CGame_MinitorClient::MonitorThread(LPVOID lParam)
	{
		// this �޾Ƶα�
		CGame_MinitorClient* g_This = (CGame_MinitorClient*)lParam;

		// ���� ��ȣ �̺�Ʈ �޾Ƶα�
		HANDLE* hEvent = &g_This->m_hMonitorThreadExitEvent;

		// CPU ����� üũ Ŭ���� (��Ʋ���� ����Ʈ����)
		CCpuUsage_Process CProcessCPU;

		// CPU ����� üũ Ŭ���� (�ϵ����)
		CCpuUsage_Processor CProcessorCPU;

		// PDH�� Ŭ����
		CPDH	CPdh;

		while (1)
		{
			// 1�ʿ� �ѹ� ����� ������ ������.
			DWORD Check = WaitForSingleObject(*hEvent, 1000);

			// �̻��� ��ȣ���
			if (Check == WAIT_FAILED)
			{
				DWORD Error = GetLastError();
				printf("MoniterThread Exit Error!!! (%d) \n", Error);
				break;
			}

			// ����, ���� ��ȣ�� �Դٸ� ������ ����.
			else if (Check == WAIT_OBJECT_0)
				break;


			// �װ� �ƴ϶��, ���� �Ѵ�.
			// ����͸� ������ �������� ����!
			if (g_This->GetClinetState() == false)
				continue;


			// ���μ���, ���μ��� CPU �����, PDH ���� ����
			CProcessorCPU.UpdateCpuTime();
			CProcessCPU.UpdateCpuTime();
			CPdh.SetInfo();

			// ----------------------------------
			// �ϵ���� ���� ������ (���μ���)
			// ----------------------------------
			int TimeStamp = (int)(time(NULL));

			// 1. �ϵ���� CPU ���� ��ü
			g_This->InfoSend(dfMONITOR_DATA_TYPE_SERVER_CPU_TOTAL, (int)CProcessorCPU.ProcessorTotal(), TimeStamp);

			// 2. �ϵ���� ��밡�� �޸� (MByte)
			g_This->InfoSend(dfMONITOR_DATA_TYPE_SERVER_AVAILABLE_MEMORY, (int)CPdh.Get_AVA_Mem(), TimeStamp);

			// 3. �ϵ���� �̴��� ���� ����Ʈ (KByte)
			int iData = (int)(CPdh.Get_Net_Recv() / 1024);
			g_This->InfoSend(dfMONITOR_DATA_TYPE_SERVER_NETWORK_RECV, iData, TimeStamp);

			// 4. �ϵ���� �̴��� �۽� ����Ʈ (KByte)
			iData = (int)(CPdh.Get_Net_Send() / 1024);
			g_This->InfoSend(dfMONITOR_DATA_TYPE_SERVER_NETWORK_SEND, iData, TimeStamp);

			// 5. �ϵ���� �������� �޸� ��뷮 (MByte)
			iData = (int)(CPdh.Get_NonPaged_Mem() / 1024 / 1024);
			g_This->InfoSend(dfMONITOR_DATA_TYPE_SERVER_NONPAGED_MEMORY, iData, TimeStamp);



			// ----------------------------------
			// ��Ʋ���� ���� ������
			// ----------------------------------

			// ��Ʋ������ On�� ���, ���Ӽ��� ���� ������.
			if (g_This->m_BattleServer_this->GetServerState() == true)
			{
				// 1. ��Ʋ���� ON		
				g_This->InfoSend(dfMONITOR_DATA_TYPE_BATTLE_SERVER_ON, TRUE, TimeStamp);

				// 2. ��Ʋ���� CPU ���� (Ŀ�� + ����)
				g_This->InfoSend(dfMONITOR_DATA_TYPE_BATTLE_CPU, (int)CProcessCPU.ProcessTotal(), TimeStamp);

				// 3. ��Ʋ���� �޸� ���� Ŀ�� ��뷮 (Private) MByte
				int Data = (int)(CPdh.Get_UserCommit() / 1024 / 1024);
				g_This->InfoSend(dfMONITOR_DATA_TYPE_BATTLE_MEMORY_COMMIT, Data, TimeStamp);

				// 4. ��Ʋ���� ��ŶǮ ��뷮
				g_This->InfoSend(dfMONITOR_DATA_TYPE_BATTLE_PACKET_POOL, CProtocolBuff_Net::GetNodeCount() + CProtocolBuff_Lan::GetNodeCount(), TimeStamp);

				// 5. Auth ������ �ʴ� ���� ��
				g_This->InfoSend(dfMONITOR_DATA_TYPE_BATTLE_AUTH_FPS, g_This->m_BattleServer_this->m_lAuthFPS, TimeStamp);

				// 6. Game ������ �ʴ� ���� ��
				g_This->InfoSend(dfMONITOR_DATA_TYPE_BATTLE_GAME_FPS, g_This->m_BattleServer_this->m_lGameFPS, TimeStamp);

				// 7. ��Ʋ���� ���� ������ü
				g_This->InfoSend(dfMONITOR_DATA_TYPE_BATTLE_SESSION_ALL, (int)g_This->m_BattleServer_this->GetClientCount(), TimeStamp);

				// 8. Auth ������ ��� �ο�
				g_This->InfoSend(dfMONITOR_DATA_TYPE_BATTLE_SESSION_AUTH, g_This->m_BattleServer_this->GetAuthModeUserCount(), TimeStamp);

				// 9. Game ������ ��� �ο�
				g_This->InfoSend(dfMONITOR_DATA_TYPE_BATTLE_SESSION_GAME, g_This->m_BattleServer_this->GetGameModeUserCount(), TimeStamp);

				// 10. ���� ��
				g_This->InfoSend(dfMONITOR_DATA_TYPE_BATTLE_ROOM_WAIT, g_This->m_BattleServer_this->m_lNowWaitRoomCount, TimeStamp);

				// 11. �÷��� �� ��
				g_This->InfoSend(dfMONITOR_DATA_TYPE_BATTLE_ROOM_PLAY, g_This->m_BattleServer_this->m_lPlayRoomCount, TimeStamp);
			}

		}

		return 0;
	}

	// ����͸� ������ ������ ����
	//
	// Parameter : DataType(BYTE), DataValue(int), TimeStamp(int)
	// return : ����
	void CGame_MinitorClient::InfoSend(BYTE DataType, int DataValue, int TimeStamp)
	{
		WORD Type = en_PACKET_SS_MONITOR_DATA_UPDATE;

		CProtocolBuff_Lan* SendBuff = CProtocolBuff_Lan::Alloc();

		SendBuff->PutData((char*)&Type, 2);
		SendBuff->PutData((char*)&DataType, 1);
		SendBuff->PutData((char*)&DataValue, 4);
		SendBuff->PutData((char*)&TimeStamp, 4);

		SendPacket(m_ullSessionID, SendBuff);
	}





	// -----------------------
	// �����Լ�
	// -----------------------

	// ��ǥ ������ ���� ���� ��, ȣ��Ǵ� �Լ� (ConnectFunc���� ���� ���� �� ȣ��)
	//
	// parameter : ����Ű
	// return : ����
	void CGame_MinitorClient::OnConnect(ULONGLONG SessionID)
	{
		if (m_ullSessionID != 0xffffffffffffffff)
			g_BattleServer_Room_Dump->Crash();

		m_ullSessionID = SessionID;

		// ����͸� ����(Lan)�� �α��� ��Ŷ ����
		CProtocolBuff_Lan* SendBuff = CProtocolBuff_Lan::Alloc();

		WORD Type = en_PACKET_SS_MONITOR_LOGIN;
		int ServerNo = dfSERVER_NO;

		SendBuff->PutData((char*)&Type, 2);
		SendBuff->PutData((char*)&ServerNo, 4);

		SendPacket(SessionID, SendBuff);
	}

	// ��ǥ ������ ���� ���� �� ȣ��Ǵ� �Լ� (InDIsconnect �ȿ��� ȣ��)
	//
	// parameter : ����Ű
	// return : ����
	void CGame_MinitorClient::OnDisconnect(ULONGLONG SessionID)
	{
		m_ullSessionID = 0xffffffffffffffff;
	}

	// ��Ŷ ���� �Ϸ� �� ȣ��Ǵ� �Լ�.
	//
	// parameter : ���� ����Ű, CProtocolBuff_Lan*
	// return : ����
	void CGame_MinitorClient::OnRecv(ULONGLONG SessionID, CProtocolBuff_Lan* Payload)
	{}

	// ��Ŷ �۽� �Ϸ� �� ȣ��Ǵ� �Լ�
	//
	// parameter : ���� ����Ű, Send �� ������
	// return : ����
	void CGame_MinitorClient::OnSend(ULONGLONG SessionID, DWORD SendSize)
	{}

	// ��Ŀ �����尡 ��� �� ȣ��Ǵ� �Լ�.
	// GQCS �ٷ� �ϴܿ��� ȣ��
	// 
	// parameter : ����
	// return : ����
	void CGame_MinitorClient::OnWorkerThreadBegin()
	{}

	// ��Ŀ �����尡 ���� �� ȣ��Ǵ� �Լ�
	// GQCS �ٷ� ������ ȣ��
	// 
	// parameter : ����
	// return : ����
	void CGame_MinitorClient::OnWorkerThreadEnd()
	{}

	// ���� �߻� �� ȣ��Ǵ� �Լ�.
	//
	// parameter : ���� �ڵ�(���� ������ �����ڵ�� WinGetLastError() �Լ��� ��� ����. ���� ��� 0�� ���ϵ�)
	//			 : ���� �ڵ忡 ���� ��Ʈ��
	// return : ����
	void CGame_MinitorClient::OnError(int error, const TCHAR* errorStr)
	{}
}

// ---------------------------
//
// ä�ü����� ��Ŭ��� ����Ǵ� Lan ����
//
// ---------------------------
namespace Library_Jingyu
{
	// -----------------------
	// Battle Net ������ ȣ���ϴ� �Լ�
	// -----------------------

	// ä�� ��Ŭ�󿡰�, �ű� ���� ���� ��Ŷ ������
	//
	// Parameter : stRoom*
	// return : ����
	void CBattle_Chat_LanServer::Packet_NewRoomCreate_Req(CBattleServer_Room::stRoom* NewRoom)
	{
		// ������ ä�� ��Ŭ�� ���ٸ� ��Ŷ �Ⱥ�����.
		if (m_bLoginCheck == false)
			return;

		// ������ ä�� Ŭ�� ���ٸ�, �Ⱥ�����.
		if (m_ullSessionID == 0xffffffffffffffff)
			g_BattleServer_Room_Dump->Crash();


		// 1. ��Ŷ �����
		WORD Type = en_PACKET_CHAT_BAT_REQ_CREATED_ROOM;
		UINT ReqSequence = InterlockedIncrement(&m_uiReqSequence);

		CProtocolBuff_Lan* SendBuff = CProtocolBuff_Lan::Alloc();

		SendBuff->PutData((char*)&Type, 2);
		SendBuff->PutData((char*)&NewRoom->m_iBattleServerNo, 4);
		SendBuff->PutData((char*)&NewRoom->m_iRoomNo, 4);
		SendBuff->PutData((char*)&NewRoom->m_iMaxJoinCount, 4);
		SendBuff->PutData(NewRoom->m_cEnterToken, 32);

		SendBuff->PutData((char*)&ReqSequence, 4);

		// 2. ä�� Ŭ�󿡰� Send�ϱ�
		SendPacket(m_ullSessionID, SendBuff);
	}

	// ä�� ��Ŭ�󿡰�, ��ū �߱�
	//
	// Parameter : ����
	// return : ����
	void CBattle_Chat_LanServer::Packet_TokenChange_Req()
	{
		// ������ ä�� ��Ŭ�� ���ٸ� ��Ŷ �Ⱥ�����.
		if (m_bLoginCheck == false)
			return;

		// ������ ä�� Ŭ�� ���ٸ�, �Ⱥ�����.
		if (m_ullSessionID == 0xffffffffffffffff)
			g_BattleServer_Room_Dump->Crash();


		// ��ū ��߱� ������ �Ǿ����� Ȯ���ϱ�.
		DWORD NowTime = timeGetTime();

		// !! ���� �ȳ���������, Ȥ�� ������ ������ ��Ȳ�� �߻��� ���� ������ signed������ �޴´�. !!
		// !! �׸��� �װ� ���Ѵ� !!
		int CheckTime = NowTime - m_dwTokenSendTime;
		
		//if ((m_dwTokenSendTime + m_BattleServer->m_stConst.m_iTokenChangeSlice) <= NowTime)
		if (CheckTime > m_BattleServer->m_stConst.m_iTokenChangeSlice)
		{
			// 1. ��ū �߱� �ð� ����
			m_dwTokenSendTime = NowTime;

			// �� �ɱ�. �� �Ȱɸ� �����ϴ� �� ���� üũ���� �� ����.
			AcquireSRWLockExclusive(&m_BattleServer->m_ServerEnterToken_srwl);	// ----- Exclusive ��

			// 2. "����" ��ū�� "����" ��ū�� ����
			memcpy_s(m_BattleServer->m_cConnectToken_Before, 32, m_BattleServer->m_cConnectToken_Now, 32);

			// 3. "����" ��ū �ٽ� �����
			int i = 0;
			while (i < 32)
			{
				m_BattleServer->m_cConnectToken_Now[i] = (rand() % 128) + 1;

				++i;
			}

			ReleaseSRWLockExclusive(&m_BattleServer->m_ServerEnterToken_srwl);	// ----- Exclusive ���

			// 4. ��ū ��߱� ��Ŷ �����
			WORD Type = en_PACKET_CHAT_BAT_REQ_CONNECT_TOKEN;
			UINT ReqSequence = InterlockedIncrement(&m_uiReqSequence);

			CProtocolBuff_Lan* SendBuff = CProtocolBuff_Lan::Alloc();

			SendBuff->PutData((char*)&Type, 2);
			SendBuff->PutData(m_BattleServer->m_cConnectToken_Now, 32);
			SendBuff->PutData((char*)&ReqSequence, 4);


			// 5. ä�ÿ��� ��ū ��Ŷ ������
			SendPacket(m_ullSessionID, SendBuff);
		}
	}




	// ------------------------
	// �ܺο��� ȣ�� ������ �Լ�
	// ------------------------

	// ���� ����
	//
	// Parameter : IP, Port, ���� ��Ŀ, Ȱ��ȭ ��Ŀ, ���� ����Ʈ ������, �������, �ִ� ������ ��
	// return : ���� �� false
	bool CBattle_Chat_LanServer::ServerStart(TCHAR* IP, int Port, int CreateWorker, int ActiveWorker, int CreateAccept, int Nodelay, int MaxUser)
	{
		if (Start(IP, Port, CreateWorker, ActiveWorker, CreateAccept, Nodelay, MaxUser) == false)
			return false;

		return true;
	}

	// ���� ����
	//
	// Parameter : ����
	// return : ����
	void CBattle_Chat_LanServer::ServerStop()
	{
		Stop();
	}




	// -----------------------
	// ��Ŷ ó�� �Լ�
	// -----------------------
	
	// �ű� ���� ���� ��Ŷ ����.
	// �� �ȿ��� �����Ϳ��Ե� �����ش�.
	//
	// Parameter : CProtocolBuff_Lan*
	// return : ����
	void CBattle_Chat_LanServer::Packet_NewRoomCreate_Res(ULONGLONG SessionID, CProtocolBuff_Lan* Packet)
	{
		if (SessionID != m_ullSessionID)
			g_BattleServer_Room_Dump->Crash();

		if (m_ullSessionID == 0xffffffffffffffff)
			g_BattleServer_Room_Dump->Crash();

		if (m_bLoginCheck == false)
			g_BattleServer_Room_Dump->Crash();


		// 1. ������
		int RoomNo;
		Packet->GetData((char*)&RoomNo, 4);

		// 2. �����Ϳ��� �ű� �� ���� ��Ŷ ������
		m_pMasterClient->Packet_NewRoomCreate_Req(RoomNo);
	}
	
	// �� ������ ���� ����
	//
	// Parameter : SessinID, CProtocolBuff_Lan*
	// return : ����
	void CBattle_Chat_LanServer::Packet_RoomClose_Res(ULONGLONG SessionID, CProtocolBuff_Lan* Packet)
	{

	}

	// ��ū ����࿡ ���� ����
	// �� �ȿ��� �����Ϳ��Ե� �����ش�.
	//
	// Parameter : SessionID, CProtocolBuff_Lan*
	// return : ����
	void CBattle_Chat_LanServer::Packet_TokenChange_Res(ULONGLONG SessionID, CProtocolBuff_Lan* Packet)
	{
		if (SessionID != m_ullSessionID)
			g_BattleServer_Room_Dump->Crash();

		if (m_ullSessionID == 0xffffffffffffffff)
			g_BattleServer_Room_Dump->Crash();

		if (m_bLoginCheck == false)
			g_BattleServer_Room_Dump->Crash();


		// 1. ������ �������� ��ū ������
		m_pMasterClient->Packet_TokenChange_Req();
	}

	// �α��� ��û
	//
	// Parameter : SessinID, CProtocolBuff_Lan*
	// return : ����
	void CBattle_Chat_LanServer::Packet_Login(ULONGLONG SessionID, CProtocolBuff_Lan* Packet)
	{
		if (SessionID != m_ullSessionID)
			g_BattleServer_Room_Dump->Crash();

		if (m_ullSessionID == 0xffffffffffffffff)
			g_BattleServer_Room_Dump->Crash();

		if (m_bLoginCheck == true)
			g_BattleServer_Room_Dump->Crash();

		// ä�ü����� IP�� Port ��������
		Packet->GetData((char*)m_pMasterClient->m_tcChatNetServerIP, 32);
		Packet->GetData((char*)&m_pMasterClient->m_iChatNetServerPort, 2);		


		// ���� ��Ŷ ������
		CProtocolBuff_Lan* SendBuff = CProtocolBuff_Lan::Alloc();

		WORD Type = en_PACKET_CHAT_BAT_RES_SERVER_ON;

		SendBuff->PutData((char*)&Type, 2);

		SendPacket(SessionID, SendBuff);



		// ä�ü����� ��ū ���� ��Ŷ ������
		UINT ReqSequence = InterlockedIncrement(&m_uiReqSequence);
		CProtocolBuff_Lan* TokenBuff = CProtocolBuff_Lan::Alloc();

		Type = en_PACKET_CHAT_BAT_REQ_CONNECT_TOKEN;

		TokenBuff->PutData((char*)&Type, 2);
		TokenBuff->PutData((char*)m_BattleServer->m_cConnectToken_Now, 32);
		TokenBuff->PutData((char*)&ReqSequence, 4);

		SendPacket(SessionID, TokenBuff);


		// ��ū �߱޽ð� ����
		m_dwTokenSendTime = timeGetTime();

		// �α��� ���·� ����
		m_bLoginCheck = true;
	}






	// --------------------------
	// ���ο����� ����ϴ� �Լ�
	// --------------------------

	// ������ �� Ŭ��, ��Ʋ Net ���� ����
	//
	// Parameter : CBattle_Master_LanClient*
	// return : ����
	void CBattle_Chat_LanServer::SetMasterClient(CBattle_Master_LanClient* SetPoint, CBattleServer_Room* SetPoint2)
	{
		m_pMasterClient = SetPoint;
		m_BattleServer = SetPoint2;
	}



	   	 



	// -----------------------
	// �����Լ�
	// -----------------------

	// Accept ����, ȣ��ȴ�.
	//
	// parameter : ������ ������ IP, Port
	// return false : Ŭ���̾�Ʈ ���� �ź�
	// return true : ���� ���
	bool CBattle_Chat_LanServer::OnConnectionRequest(TCHAR* IP, USHORT port)
	{
		// �̹� ������ ������ ���� �� return false;
		// ä�� �� �������� �� Ŭ�� ���� ����
		if (m_ullSessionID != 0xffffffffffffffff)
			return false;

		return true;
	}

	// ���� �� ȣ��Ǵ� �Լ� (AcceptThread���� Accept �� ȣ��)
	//
	// parameter : ������ �������� �Ҵ�� ����Ű
	// return : ����
	void CBattle_Chat_LanServer::OnClientJoin(ULONGLONG SessionID)
	{
		if (m_ullSessionID != 0xffffffffffffffff)
			g_BattleServer_Room_Dump->Crash();

		if(m_bLoginCheck == true)
			g_BattleServer_Room_Dump->Crash();

		m_ullSessionID = SessionID;
	}

	// ���� ���� �� ȣ��Ǵ� �Լ� (InDIsconnect �ȿ��� ȣ��)
	//
	// parameter : ���� ����Ű
	// return : ����
	void CBattle_Chat_LanServer::OnClientLeave(ULONGLONG SessionID)
	{
		m_ullSessionID = 0xffffffffffffffff;
		m_bLoginCheck = false;
	}

	// ��Ŷ ���� �Ϸ� �� ȣ��Ǵ� �Լ�.
	//
	// parameter : ���� ����Ű, ���� ��Ŷ
	// return : ����
	void CBattle_Chat_LanServer::OnRecv(ULONGLONG SessionID, CProtocolBuff_Lan* Payload)
	{
		// Ÿ�� Ȯ��
		WORD Type;

		Payload->GetData((char*)&Type, 2);

		try
		{
			// Ÿ�Կ� ���� �б� ó��
			switch (Type)
			{
				// �ű� ���� ������ ���� ����
			case en_PACKET_CHAT_BAT_RES_CREATED_ROOM:
				Packet_NewRoomCreate_Res(SessionID, Payload);
				break;			

				// �� ������ ���� ����
			case en_PACKET_CHAT_BAT_RES_DESTROY_ROOM:
				break;

				// ���� ��ū ����࿡ ���� ����
			case en_PACKET_CHAT_BAT_RES_CONNECT_TOKEN:
				Packet_TokenChange_Res(SessionID, Payload);
				break;

				// �α��� 
			case en_PACKET_CHAT_BAT_REQ_SERVER_ON:
				Packet_Login(SessionID, Payload);
				break;

				// ���� Ÿ���̸� ũ����
			default:
				TCHAR str[100];
				StringCchPrintf(str, 100, _T("CBattle_Chat_LanServer. OnRecv(). TypeError. SessionID : %lld, Type : %d"), SessionID, Type);

				throw CException(str);
				break;
			}
		}

		catch (CException& exc)
		{
			// �α� ��� (�α� ���� : ����)
			g_BattleServer_RoomLog->LogSave(false, L"CBattleServer_Room", CSystemLog::en_LogLevel::LEVEL_ERROR, L"%s",
				(TCHAR*)exc.GetExceptionText());

			g_BattleServer_Room_Dump->Crash();

			// ���� ���� ��û
			//Disconnect(SessionID);
		}
	}
		

	// ��Ŷ �۽� �Ϸ� �� ȣ��Ǵ� �Լ�
	//
	// parameter : ���� ����Ű, Send �� ������
	// return : ����
	void CBattle_Chat_LanServer::OnSend(ULONGLONG SessionID, DWORD SendSize)
	{

	}

	// ��Ŀ �����尡 ��� �� ȣ��Ǵ� �Լ�.
	// GQCS �ٷ� �ϴܿ��� ȣ��
	// 
	// parameter : ����
	// return : ����
	void CBattle_Chat_LanServer::OnWorkerThreadBegin()
	{

	}

	// ��Ŀ �����尡 ���� �� ȣ��Ǵ� �Լ�
	// GQCS �ٷ� ������ ȣ��
	// 
	// parameter : ����
	// return : ����
	void CBattle_Chat_LanServer::OnWorkerThreadEnd()
	{

	}

	// ���� �߻� �� ȣ��Ǵ� �Լ�.
	//
	// parameter : ���� �ڵ�(���� ������ �����ڵ�� WinGetLastError() �Լ��� ��� ����. ���� ��� 0�� ���ϵ�)
	//			 : ���� �ڵ忡 ���� ��Ʈ��
	// return : ����
	void CBattle_Chat_LanServer::OnError(int error, const TCHAR* errorStr)
	{

	}




	// ---------------
	// �����ڿ� �Ҹ���
	// ----------------

	// ������
	CBattle_Chat_LanServer::CBattle_Chat_LanServer()
	{
		m_ullSessionID = 0xffffffffffffffff;
		m_bLoginCheck = false;
	}

	// �Ҹ���
	CBattle_Chat_LanServer::~CBattle_Chat_LanServer()
	{

	}

}