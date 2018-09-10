#include "pch.h"
#include "DB_Connector.h"
#include "CrashDump\CrashDump.h"
#include <strsafe.h>


namespace Library_Jingyu
{

#define 
	CCrashDump* g_DBDump = CCrashDump::GetInstance();

	//////////////////////////////////////////////////////////////////////
	// ������
	// 
	// Parameter : ������ DB IP, ����� �̸�, ��й�ȣ, DB �̸�, ��Ʈ
	//////////////////////////////////////////////////////////////////////
	CDBConnector::CDBConnector(WCHAR *DBIP, WCHAR *User, WCHAR *Password, WCHAR *DBName, int DBPort)
	{
		// -----------------------------------
		// ���ڰ����� ��������� ����
		// �����ϴ� �����ϸ� ũ����.
		// ���⼭ �����ϴ°� �׳� �ڵ�Ǽ��̴� �����ȵ�.
		// -----------------------------------

		// 1. IP
		if (StringCchCopy(m_wcDBIP, 16, DBIP) != S_OK)
			g_DBDump->Crash();

		// 2. ����� �̸�
		if (StringCchCopy(m_wcDBUser, 64, User) != S_OK)
			g_DBDump->Crash();

		// 3. ��й�ȣ
		if (StringCchCopy(m_wcDBPassword, 64, Password) != S_OK)
			g_DBDump->Crash();

		// 4. DB �̸�
		if (StringCchCopy(m_wcDBName, 64, DBName) != S_OK)
			g_DBDump->Crash();

		// 5. ��Ʈ
		m_iDBPort = DBPort;

		// -----------------------------------
		// DB ���� ��ü �����͸� nullptr�� �ʱ�ȭ.
		// �� �����͸� �̿��� DB ���� ���� �Ǵ�.
		// -----------------------------------
		m_pMySQL == nullptr;
		
	}

	// �Ҹ���
	CDBConnector::~CDBConnector()
	{
		Disconnect();
	}

	//////////////////////////////////////////////////////////////////////
	// MySQL DB ����
	// �����ڿ��� ���޹��� DB������ �̿���, DB�� ����
	// 
	// Parameter : ����
	// return : ���� ���� �� true, 
	//		  : ���� ���� �� false
	//////////////////////////////////////////////////////////////////////
	bool	CDBConnector::Connect()
	{
		// 1. �ʱ�ȭ
		mysql_init(&m_MySQL);

		// 2. ������ ���� ������, char ���·� ��ȯ
		char IP[32] = { 0, };
		char User[32] = { 0, };
		char Password[32] = { 0, };
		char Name[32] = { 0, };

		int len = (int)_tcslen(m_wcDBIP);
		WideCharToMultiByte(CP_UTF8, 0, m_wcDBIP, (int)_tcslen(m_wcDBIP), IP, len, NULL, NULL);

		len = (int)_tcslen(m_wcDBUser);
		WideCharToMultiByte(CP_UTF8, 0, m_wcDBUser, (int)_tcslen(m_wcDBUser), User, len, NULL, NULL);

		len = (int)_tcslen(m_wcDBPassword);
		WideCharToMultiByte(CP_UTF8, 0, m_wcDBPassword, (int)_tcslen(m_wcDBPassword), Password, len, NULL, NULL);

		len = (int)_tcslen(m_wcDBName);
		WideCharToMultiByte(CP_UTF8, 0, m_wcDBName, (int)_tcslen(m_wcDBName), Name, len, NULL, NULL);

		// 3. DB�� ���� �õ�
		// �� 5ȸ ���� �õ�.
		// 5ȸ���� ���� ���ϸ� Crash
		int iConnectCount = 0;
		while (1)
		{
			m_pMySQL = mysql_real_connect(&m_MySQL, IP, User, Password, Name, m_iDBPort, NULL, 0);
			if (m_pMySQL != NULL)
				break;

			if (iConnectCount >= 5)
				g_DBDump->Crash();

			iConnectCount++;
		}

		// ����Ʈ ������ utf8�� ����
		mysql_set_character_set(m_pMySQL, "utf8");
		
	}

	//////////////////////////////////////////////////////////////////////
	// MySQL DB ����
	//////////////////////////////////////////////////////////////////////
	bool	CDBConnector::Disconnect()
	{
		// ��� ����Ǿ� ���� ���, ���� ����
		if(m_pMySQL != nullptr)
			mysql_close(m_pMySQL);
	}


	//////////////////////////////////////////////////////////////////////
	// ���� ������ ����� �ӽ� ����
	//
	//////////////////////////////////////////////////////////////////////
	bool	CDBConnector::Query(WCHAR *szStringFormat, ...)
	{

	}

	// DBWriter �������� Save ���� ����
	// ������� �������� ����.
	bool	CDBConnector::Query_Save(WCHAR *szStringFormat, ...)	
	{

	}
															

	//////////////////////////////////////////////////////////////////////
	// ������ ���� �ڿ� ��� �̾ƿ���.
	//
	// ����� ���ٸ� NULL ����.
	//////////////////////////////////////////////////////////////////////
	MYSQL_ROW	CDBConnector::FetchRow()
	{

	}

	//////////////////////////////////////////////////////////////////////
	// �� ������ ���� ��� ��� ��� �� ����.
	//////////////////////////////////////////////////////////////////////
	void	CDBConnector::FreeResult()
	{

	}


	//////////////////////////////////////////////////////////////////////
	// mysql �� LastError �� �ɹ������� �����Ѵ�.
	//////////////////////////////////////////////////////////////////////
	void	CDBConnector::SaveLastError()
	{

	}

}