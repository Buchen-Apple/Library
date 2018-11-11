#ifndef __SHDB_COMMUNICATE_H__
#define __SHDB_COMMUNICATE_H__

#include "ProtocolBuff/ProtocolBuff(Net)_ObjectPool.h"
#include "ObjectPool/Object_Pool_LockFreeVersion.h"
#include "LockFree_Queue/LockFree_Queue.h"
#include "CrashDump/CrashDump.h"
#include "Http_Exchange/HTTP_Exchange.h"

// --------------------------------------------
// shDB ���ο��� �޸�Ǯ�� �����Ǵ� ����ü��
// --------------------------------------------
namespace Library_Jingyu
{
	// Auth �����忡�� �α��� ������ ���� ��, ����Ű üũ�� ���� ��������
	struct DB_WORK_LOGIN
	{
		WORD m_wWorkType;
		INT64 m_i64UniqueKey;
		LPVOID pPointer;
		CProtocolBuff_Net* m_pBuff;
		TCHAR m_tcRequest[200];

		// --- �Ʒ����Ͱ� ������ �� ���� ����.
		INT64 AccountNo;
	};

	// shDB�� � API�� ȣ���� ������.
	enum en_PHP_TYPE
	{
		// Seelct_account.php
		SELECT_ACCOUNT = 1,

		// DB_Read ������ ���� ��ȣ
		EXIT = 9999
	};

	
}


// -----------------------
// shDB�� ����ϴ� Ŭ����
// -----------------------
namespace Library_Jingyu
{
	// �޸�Ǯ���� ������ ����ü.
	// ���� ũ�Ⱑ ū �� 1���� �����Ѵ�.
#define DB_WORK	DB_WORK_LOGIN

	class shDB_Communicate
	{
		// ---------------
		// ��� ����
		// ---------------			

		// DB_Read�� ����� �Ϸ���Ʈ �ڵ�
		HANDLE m_hDB_Read;

		// DB_Read�� ��Ŀ ������ �ڵ�
		HANDLE* m_hDB_Read_Thread;

		// ����
		CCrashDump* m_Dump;	

	public:
		// ---------------
		// �ܺο��� ���� ������ ��� ����
		// ---------------		

		// DB_WORK�� �����ϴ� �޸� Ǯ
		CMemoryPoolTLS<DB_WORK>* m_pDB_Work_Pool;

		// Read�� �Ϸ�� DB_WORK* �� �����صδ� ������ ť
		CLF_Queue<DB_WORK*> *m_pDB_ReadComplete_Queue;


	private:
		// --------------
		// ������
		// --------------

		// DB_Read ������
		static UINT WINAPI DB_ReadThread(LPVOID lParam);


	private:
		// ---------------
		// API Ÿ�Կ� ���� ������
		// ---------------

		// Select_Account.php�� ������ ������
		TCHAR m_tcSELECT_ACCOUNT[30] = L"{\"accountno\" : %lld}";



	public:
		// ---------------
		// ��� �Լ�. �ܺο��� ȣ�� ����
		// ---------------

		// DB�� Read�� ���� ���� �� ȣ��Ǵ� �Լ�
		// ���ڷ� ���� ����ü�� ������ Ȯ���� ���� ó��
		// 
		// Parameter : DB_WORK*, APIType
		// return : ����
		void DBReadFunc(DB_WORK* Protocol, WORD APIType);


	public:
		// ---------------
		// �����ڿ� �Ҹ���
		// ---------------

		// ������
		shDB_Communicate();

		// �Ҹ���
		virtual ~shDB_Communicate();
	};
}


#endif // !__SHDB_COMMUNICATE_H__
