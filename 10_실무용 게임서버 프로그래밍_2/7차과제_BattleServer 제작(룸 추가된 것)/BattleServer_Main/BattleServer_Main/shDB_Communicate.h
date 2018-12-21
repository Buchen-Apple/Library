#ifndef __SHDB_COMMUNICATE_H__
#define __SHDB_COMMUNICATE_H__

#include "ProtocolBuff/ProtocolBuff(Net)_ObjectPool.h"
#include "ObjectPool/Object_Pool_LockFreeVersion.h"
#include "LockFree_Queue/LockFree_Queue.h"
#include "CrashDump/CrashDump.h"
#include "Http_Exchange/HTTP_Exchange.h"
#include "NormalTemplate_Queue\Normal_Queue_Template.h"

// --------------------------------------------
// shDB ���ο��� �޸�Ǯ�� �����Ǵ� ����ü��
// --------------------------------------------
namespace Library_Jingyu
{
	// Auth �����忡�� �α��� ������ ���� ��, ����Ű üũ�� ���� ��������
	struct DB_WORK_LOGIN
	{
		WORD m_wWorkType;
		WORD m_wAPIType;
		INT64 m_i64UniqueKey;

		// Player ������
		LPVOID pPointer;
		TCHAR m_tcResponse[420];

		INT64 AccountNo;
	};

	// Auth �����忡�� �α��� ��û�� ���� ��, ���� ���� ������ �����ϱ� ���� ��������.
	struct DB_WORK_LOGIN_CONTENTS
	{
		WORD m_wWorkType;
		WORD m_wAPIType;
		INT64 m_i64UniqueKey;

		// Player ������
		LPVOID pPointer;	
		TCHAR m_tcResponse[200];

		INT64 AccountNo;
	};

	// ContentDB�� ���� ������ Write�� �� ���
	struct DB_WORK_CONTENT_UPDATE
	{
		WORD m_wWorkType;

		// Update�� res�� ������ ª��. ���� result : 1��
		TCHAR m_tcResponse[30];

		int		m_iCount;	// ������ ī��Ʈ		

		INT64 AccountNo;
	};

	// shDB�� � API�� ȣ���� ������. API Type
	enum en_PHP_TYPE
	{
		// Seelct_account.php
		SELECT_ACCOUNT = 1,

		// Seelct_contents.php
		SELECT_CONTENTS,

		// Update_account.php
		UPDATE_ACCOUN,

		// Updated_contents.php
		UPDATE_CONTENTS,

		// DB_Read ������ ���� ��ȣ
		EXIT = 9999
	};

	// DB ��û�� ���� ��ó���� ����, Type
	enum eu_DB_AFTER_TYPE
	{
		// �α��� ��Ŷ�� ���� ���� ó��
		eu_LOGIN_AUTH = 0,

		// �α��� ��Ŷ�� ���� ���� ��������
		eu_LOGIN_INFO,

		// PlayCount ����
		eu_PLAYCOUNT_UPDATE,

		// PlayTime ����
		eu_PLAYTIME_UPDATE,

		// Kill ����
		eu_KILL_UPDATE,

		// Die ����
		eu_DIE_UPDATE,

		// Win ����
		eu_WIN_UPDATE
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

		// ����
		CCrashDump* m_Dump;

		// DB_Read�� ����� �Ϸ���Ʈ �ڵ�
		HANDLE m_hDB_Read;

		// DB_Read�� ��Ŀ ������ �ڵ�
		HANDLE* m_hDB_Read_Thread;

		// ---------------		

		// DB Write �����忡�� �Ͻ�Ű�� �� ť(Normal ť)
		CNormalQueue<DB_WORK*>* m_pDB_Wirte_Start_Queue;		

		// DB Write�� ������� �Ͻ�Ű�� ���̺�Ʈ.
		HANDLE m_hDBWrite_Event;

		// DB Write�� ������ ����� �̺�Ʈ
		HANDLE m_hDBWrite_Exit_Event;

		// DB_Write�� ������ �ڵ�
		HANDLE m_hDB_Write_Thread;


	public:
		// ---------------
		// �ܺο��� ���� ������ ��� ����
		// ---------------		

		// DB_WORK�� �����ϴ� �޸� Ǯ
		CMemoryPoolTLS<DB_WORK>* m_pDB_Work_Pool;

		// Read�� �Ϸ�� DB_WORK* �� �����صδ� ������ ť
		CLF_Queue<DB_WORK*> *m_pDB_ReadComplete_Queue;

		// DB Write ��������, �Ϸ�� �ϰ� ���� ť(Normal ť)
		CNormalQueue<DB_WORK*>* m_pDB_Wirte_End_Queue;


	private:
		// --------------
		// ������
		// --------------

		// DB_Read ������
		static UINT WINAPI DB_ReadThread(LPVOID lParam);

		// DB_Write ������
		static UINT WINAPI DB_WriteThread(LPVOID lParam);
	

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

		// DB�� Write �� ���� ���� �� ȣ��Ǵ� �Լ�.
		// ���ڷ� ���� ����ü�� ������ Ȯ���� ���� ó��
		//
		// Parameter : DB_WORK*
		// return : ����
		void DBWriteFunc(DB_WORK* Protocol);


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
