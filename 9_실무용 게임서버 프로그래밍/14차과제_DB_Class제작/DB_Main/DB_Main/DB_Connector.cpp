#include "pch.h"
#include "DB_Connector.h"
#include "CrashDump\CrashDump.h"
#include "Log\Log.h"
#include <strsafe.h>


namespace Library_Jingyu
{
#define _MyCountof(_array)		sizeof(_array) / (sizeof(_array[0]))

	CCrashDump* g_DBDump = CCrashDump::GetInstance();
	CSystemLog* g_DBLog = CSystemLog::GetInstance();

	//////////////////////////////////////////////////////////////////////
	// ������
	// 
	// Parameter : ������ DB IP, ����� �̸�, ��й�ȣ, DB �̸�, ��Ʈ
	//////////////////////////////////////////////////////////////////////
	CDBConnector::CDBConnector(WCHAR *DBIP, WCHAR *User, WCHAR *Password, WCHAR *DBName, int DBPort)
	{
		// -----------------------------------
		// ���ڰ��� ��������� ����
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
		m_pMySQL = nullptr;
		
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

			// ��������� ���� ����
			SaveLastError();
			
			// ī��Ʈ ����
			iConnectCount++;

			// �α׿� ����
			g_DBLog->LogSave(L"DB_Connector", CSystemLog::en_LogLevel::LEVEL_ERROR, 
				L"Connect() --> Connect Fail... (Count : %d)", iConnectCount);

			if (iConnectCount >= 5)
				g_DBDump->Crash();	

			// �ٷ� �õ��ϸ� ������ ���ɼ��� �ֱ� ������ Sleep(0)�� �Ѵ�.
			Sleep(0);
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
		if (m_pMySQL != nullptr)
			mysql_close(m_pMySQL);

		return true;
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
		if(StringCbCopy(m_wcQuery, eQUERY_MAX_LEN, szStringFormat) != S_OK)
			g_DBDump->Crash();

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
			int Count = 0;
			while (1)
			{
				// DB ����
				m_pMySQL = mysql_real_connect(&m_MySQL, m_cDBIP, m_cDBUser, m_cDBPassword, m_cDBName, m_iDBPort, NULL, 0);
				if (m_pMySQL != NULL)
					break;

				// ��������� ���� ����
				SaveLastError();
				
				// ī��Ʈ 1 ����
				Count++;

				// ���� �� �α� ����
				g_DBLog->LogSave(L"DB_Connector", CSystemLog::en_LogLevel::LEVEL_ERROR,
					L"Query() --> Connect Fail... (Count : %d)", Count);

				// 5��° �������� ���� ����. �� �̻� ������ ������ ���� �ȵ�.
				if (Count >= 5)
					g_DBDump->Crash();	

				// �ٷ� �õ��ϸ� ������ ���ɼ��� �ֱ� ������ Sleep(0)�� �Ѵ�.
				Sleep(0);
			}
		}

		// 4. ��� �޾Ƶα�
		// �ۿ��� FetchRow() �Լ��� ȣ���ؼ� ���.
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
			int Count = 0;
			while (1)
			{
				// DB ����
				m_pMySQL = mysql_real_connect(&m_MySQL, m_cDBIP, m_cDBUser, m_cDBPassword, m_cDBName, m_iDBPort, NULL, 0);
				if (m_pMySQL != NULL)
					break;

				// ��������� ���� ����
				SaveLastError();

				// ī��Ʈ 1 ����
				Count++;

				// ���� �� �α� ����
				g_DBLog->LogSave(L"DB_Connector", CSystemLog::en_LogLevel::LEVEL_ERROR,
					L"Query() --> Connect Fail... (Count : %d)", Count);

				// 5��° �������� ���� ����. �� �̻� ������ ������ ���� �ȵ�.
				if (Count >= 5)
					g_DBDump->Crash();

				// �ٷ� �õ��ϸ� ������ ���ɼ��� �ֱ� ������ Sleep(0)�� �Ѵ�.
				Sleep(0);
			}
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
		// �� ������ �ѱ��.
		// ����� ������ �ڵ����� NULL�� ���ϵȴ�.
		return mysql_fetch_row(m_pSqlResult);
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
		// ���� �ѹ� ����
		m_iLastError = mysql_errno(&m_MySQL);

		// ���� �޽��� ����
		StringCbPrintf(m_wcLastErrorMsg, _MyCountof(m_wcLastErrorMsg), 
			L"Mysql error : %s\n", mysql_error(&m_MySQL));
	}



	// --------------------------
	// --------------------------
	// --------------------------
	// --------------------------
	// --------------------------

	// DB TLS

	// ������
	CBConnectorTLS::CBConnectorTLS(WCHAR *DBIP, WCHAR *User, WCHAR *Password, WCHAR *DBName, int DBPort)
	{
		// 1. TLSIndex�� ���´�.
		m_dwTLSIndex = TlsAlloc();
		if (m_dwTLSIndex == TLS_OUT_OF_INDEXES)
			g_DBDump->Crash();

		// -----------------------------------
		// ���ڰ��� ��������� ����
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

		// ---------------------
		// �α� ����
		g_DBLog->SetDirectory(L"DB_Connector");
		g_DBLog->SetLogLeve((CSystemLog::en_LogLevel)CSystemLog::en_LogLevel::LEVEL_ERROR);
	}


	// �Ҹ���
	// ���ÿ� �����ϰ� �ִ� DBConnector delete.
	// �ڵ����� DBConnector�� �Ҹ��� ȣ��
	CBConnectorTLS::~CBConnectorTLS()
	{
		LONG Count = m_stackConnector.GetInNode();

		// ���ÿ� ����ִ� DBConnector��ŭ ���鼭 delete
		while (Count > 0)
		{
			CDBConnector* retval = m_stackConnector.Pop();
			delete retval;
			Count--;
		}
		
	}

	//////////////////////////////////////////////////////////////////////
	// MySQL DB ����
	//
	// return : ���� �� true, ���� �� false.
	// ���� ��, GetLastError��, GetLastErrorMsg�� �̿��� ���� Ȯ��
	//////////////////////////////////////////////////////////////////////
	bool		CBConnectorTLS::Connect(void)
	{
		// 1. TLS���� DBConnector�� �����´�.
		CDBConnector* NowDB = (CDBConnector*)TlsGetValue(m_dwTLSIndex);

		// ����, ���� DBConnector�� �Ҵ����� ���� ���¶��, DBConnector����
		if (NowDB == nullptr)
		{
			NowDB = new CDBConnector(m_wcDBIP, m_wcDBUser, m_wcDBPassword, m_wcDBName, m_iDBPort);

			if (TlsSetValue(m_dwTLSIndex, NowDB) == 0)
			{
				DWORD Error = GetLastError();
				g_DBDump->Crash();
			}

		}

		// 2. ������ DBConnector�� connect �Լ� ȣ�� ��, ��� �� ����
		return NowDB->Connect();
	}

	//////////////////////////////////////////////////////////////////////
	// MySQL DB ����
	//
	// return : ���������� ���� �� true
	//////////////////////////////////////////////////////////////////////
	bool		CBConnectorTLS::Disconnect(void)
	{
		// 1. TLS���� DBConnector�� �����´�.
		CDBConnector* NowDB = (CDBConnector*)TlsGetValue(m_dwTLSIndex);

		// ����, ���� DBConnector�� �Ҵ����� ���� ���¶��, Crash
		if (NowDB == nullptr)
			g_DBDump->Crash();

		// 2. ������ DBConnector�� Disconnect �Լ� ȣ�� ��, ��� �� ����
		return NowDB->Disconnect();
	}


	//////////////////////////////////////////////////////////////////////
	// ���� ������ ����� �ӽ� ����
	//
	// Parameter : WCHAR�� ���� �޽���
	//////////////////////////////////////////////////////////////////////
	bool		CBConnectorTLS::Query(WCHAR *szStringFormat, ...)
	{		
		// 1. TLS���� DBConnector�� �����´�.
		CDBConnector* NowDB = (CDBConnector*)TlsGetValue(m_dwTLSIndex);

		// ����, ���� DBConnector�� �Ҵ����� ���� ���¶��, Crash
		if (NowDB == nullptr)
			g_DBDump->Crash();

		 // 2. ������ DBConnector�� Query �Լ� ȣ�� ��, ��� �� ����
		return NowDB->Query(szStringFormat);
	}

	// DBWriter �������� Save ���� ����
	// ������� �������� ����.
	bool		CBConnectorTLS::Query_Save(WCHAR *szStringFormat, ...)
	{
		// 1. TLS���� DBConnector�� �����´�.
		CDBConnector* NowDB = (CDBConnector*)TlsGetValue(m_dwTLSIndex);

		// ����, ���� DBConnector�� �Ҵ����� ���� ���¶��, Crash
		if (NowDB == nullptr)
			g_DBDump->Crash();

		// 2. ������ DBConnector�� Query_Save �Լ� ȣ�� ��, ��� �� ����
		return NowDB->Query_Save(szStringFormat);
	}
		
															

	//////////////////////////////////////////////////////////////////////
	// ������ ���� �ڿ� ��� �̾ƿ���.
	// ����� ���ٸ� NULL ����.
	// 
	// return : result�� Row. Row�� ���� �� null ����
	//////////////////////////////////////////////////////////////////////
	MYSQL_ROW	CBConnectorTLS::FetchRow(void)
	{
		// 1. TLS���� DBConnector�� �����´�.
		CDBConnector* NowDB = (CDBConnector*)TlsGetValue(m_dwTLSIndex);

		// ����, ���� DBConnector�� �Ҵ����� ���� ���¶��, Crash
		if (NowDB == nullptr)
			g_DBDump->Crash();

		// 2. ������ DBConnector�� FetchRow �Լ� ȣ�� ��, ��� �� ����
		return NowDB->FetchRow();
	}

	//////////////////////////////////////////////////////////////////////
	// �� ������ ���� ��� ��� ��� �� ����.
	//////////////////////////////////////////////////////////////////////
	void		CBConnectorTLS::FreeResult(void)
	{
		// 1. TLS���� DBConnector�� �����´�.
		CDBConnector* NowDB = (CDBConnector*)TlsGetValue(m_dwTLSIndex);

		// ����, ���� DBConnector�� �Ҵ����� ���� ���¶��, Crash
		if (NowDB == nullptr)
			g_DBDump->Crash();

		// 2. ������ DBConnector�� FreeResult �Լ� ȣ��.
		NowDB->FreeResult();
	}


	//////////////////////////////////////////////////////////////////////
	// Error ���.
	//////////////////////////////////////////////////////////////////////
	int			CBConnectorTLS::GetLastError(void)
	{
		// 1. TLS���� DBConnector�� �����´�.
		CDBConnector* NowDB = (CDBConnector*)TlsGetValue(m_dwTLSIndex);

		// ����, ���� DBConnector�� �Ҵ����� ���� ���¶��, Crash
		if (NowDB == nullptr)
			g_DBDump->Crash();

		// 2. ������ DBConnector�� GetLastError �Լ� ȣ�� �� ����� ����
		NowDB->GetLastError();
	}

	WCHAR*		CBConnectorTLS::GetLastErrorMsg(void)
	{
		// 1. TLS���� DBConnector�� �����´�.
		CDBConnector* NowDB = (CDBConnector*)TlsGetValue(m_dwTLSIndex);

		// ����, ���� DBConnector�� �Ҵ����� ���� ���¶��, Crash
		if (NowDB == nullptr)
			g_DBDump->Crash();

		// 2. ������ DBConnector�� GetLastErrorMsg �Լ� ȣ�� �� ����� ����
		NowDB->GetLastErrorMsg();
	}

}