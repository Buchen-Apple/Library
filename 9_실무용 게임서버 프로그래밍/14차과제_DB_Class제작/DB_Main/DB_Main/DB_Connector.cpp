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

		// 2. ������ ���� ������, char ���·� ��ȯ�� ����
		int len = (int)_tcslen(m_wcDBIP);
		WideCharToMultiByte(CP_UTF8, 0, m_wcDBIP, (int)_tcslen(m_wcDBIP), m_cDBIP, len, NULL, NULL);

		len = (int)_tcslen(m_wcDBUser);
		WideCharToMultiByte(CP_UTF8, 0, m_wcDBUser, (int)_tcslen(m_wcDBUser), m_cDBUser, len, NULL, NULL);

		len = (int)_tcslen(m_wcDBPassword);
		WideCharToMultiByte(CP_UTF8, 0, m_wcDBPassword, (int)_tcslen(m_wcDBPassword), m_cDBPassword, len, NULL, NULL);

		len = (int)_tcslen(m_wcDBName);
		WideCharToMultiByte(CP_UTF8, 0, m_wcDBName, (int)_tcslen(m_wcDBName), m_cDBName, len, NULL, NULL);

		// 3. DB�� ���� �õ�
		// �� 5ȸ ���� �õ�.
		// 5ȸ���� ���� ���ϸ� Crash
		int iConnectCount = 0;
		while (1)
		{
			m_pMySQL = mysql_real_connect(&m_MySQL, m_cDBIP, m_cDBUser, m_cDBPassword, m_cDBName, m_iDBPort, NULL, 0);
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
		// 1. ���� ������ utf-8�� ��ȯ. ���ÿ� utf-8 ����� ��������� ����
		int len = (int)_tcslen(szStringFormat);
		WideCharToMultiByte(CP_UTF8, 0, szStringFormat, (int)_tcslen(szStringFormat), m_cQueryUTF8, len, NULL, NULL);

		// 2. �����ڵ� ����� ��������� ����. ���� �α׸� ��ų� �ϴ� ���� �Ҷ��� �����ڵ尡 �ʿ�
		if (StringCbCopy(m_wcQuery, eQUERY_MAX_LEN, szStringFormat) != S_OK)
		{
			// ī�� ���� �� ���� ���� ��, false ����
			return false;
		}

		// 3. ���� ������
		int Error = mysql_query(m_pMySQL, m_cQueryUTF8);

		// ������ ����Ÿ� 5ȸ���� �翬�� �õ�
		if (Error == CR_SOCKET_CREATE_ERROR ||
			Error == CR_CONNECTION_ERROR ||
			Error == CR_CONN_HOST_ERROR ||
			Error == CR_SERVER_HANDSHAKE_ERR ||
			Error == CR_SERVER_GONE_ERROR ||
			Error == CR_INVALID_CONN_HANDLE ||
			Error == CR_SERVER_LOST)
		{
		}

		int Count = 0;
		while (Count < 5)
		{
			// DB ����
			m_pMySQL = mysql_real_connect(&m_MySQL, m_cDBIP, m_cDBUser, m_cDBPassword, m_cDBName, m_iDBPort, NULL, 0);
			if (m_pMySQL != NULL)
				break;

			Count++;
			Sleep(0);
		}

		// 5ȸ ���� �õ��ߴµ��� ���� ���и�, ���� ��� ���� false.
		if (m_pMySQL == NULL)
		{
			printf("DBQuery()1. Mysql connection error : %s(%d)\n", mysql_error(&m_MySQL), mysql_errno(&m_MySQL));
			return false;
		}


		// 4. ��� �޾Ƶα�
		// �ۿ��� Query_Save() �Լ��� ȣ���ؼ� ���.
		m_pSqlResult = mysql_store_result(m_pMySQL);

		return true;
	}

	// DBWriter �������� Save ���� ����
	// ������� �������� ����.
	// �����⸸�ϰ� ��� ���� �ʿ� ������ ���
	bool	CDBConnector::Query_Save(WCHAR *szStringFormat, ...)	
	{
		// 1. ���� ������ utf-8�� ��ȯ. ���ÿ� utf-8 ����� ��������� ����
		int len = (int)_tcslen(szStringFormat);
		WideCharToMultiByte(CP_UTF8, 0, szStringFormat, (int)_tcslen(szStringFormat), m_cQueryUTF8, len, NULL, NULL);

		// 2. �����ڵ� ����� ��������� ����. ���� �α׸� ��ų� �ϴ� ���� �Ҷ��� �����ڵ尡 �ʿ�
		if (StringCbCopy(m_wcQuery, eQUERY_MAX_LEN, szStringFormat) != S_OK)
		{
			// ī�� ���� �� ���� ���� ��, false ����
			return false;
		}

		// 3. ���� ������
		int Error = mysql_query(m_pMySQL, m_cQueryUTF8);

		// ������ ����Ÿ� 5ȸ���� �翬�� �õ�
		if (Error == CR_SOCKET_CREATE_ERROR ||
			Error == CR_CONNECTION_ERROR ||
			Error == CR_CONN_HOST_ERROR ||
			Error == CR_SERVER_HANDSHAKE_ERR ||
			Error == CR_SERVER_GONE_ERROR ||
			Error == CR_INVALID_CONN_HANDLE ||
			Error == CR_SERVER_LOST)
		{
		}

		int Count = 0;
		while (Count < 5)
		{
			// DB ����
			m_pMySQL = mysql_real_connect(&m_MySQL, m_cDBIP, m_cDBUser, m_cDBPassword, m_cDBName, m_iDBPort, NULL, 0);
			if (m_pMySQL != NULL)
				break;

			Count++;
			Sleep(0);
		}

		// 5ȸ ���� �õ��ߴµ��� ���� ���и�, ���� ��� ���� false.
		if (m_pMySQL == NULL)
		{
			printf("DBQuery()1. Mysql connection error : %s(%d)\n", mysql_error(&m_MySQL), mysql_errno(&m_MySQL));
			return false;
		}


		// 4. ��� ���� ��, �ٷ� result �Ѵ�.		
		m_pSqlResult = mysql_store_result(m_pMySQL);
		mysql_free_result(m_pSqlResult);

		return true;

	}
															

	//////////////////////////////////////////////////////////////////////
	// ������ ���� �ڿ� ��� �̾ƿ���.
	//
	// ����� ���ٸ� NULL ����.
	//////////////////////////////////////////////////////////////////////
	MYSQL_ROW	CDBConnector::FetchRow()
	{
		// 1 �� ������ �ѱ��.
		mysql_fetch_row(m_pSqlResult);
	}

	//////////////////////////////////////////////////////////////////////
	// �� ������ ���� ��� ��� ��� �� ����.
	//////////////////////////////////////////////////////////////////////
	void	CDBConnector::FreeResult()
	{
		mysql_free_result(m_pSqlResult);
	}


	//////////////////////////////////////////////////////////////////////
	// mysql �� LastError �� �ɹ������� �����Ѵ�.
	//////////////////////////////////////////////////////////////////////
	void	CDBConnector::SaveLastError()
	{

	}

}