#include "pch.h"

#include <strsafe.h>

#include "DB_Connector.h"
#include "CrashDump\CrashDump.h"
#include "Log\Log.h"

#pragma comment(lib, "Mysql/lib/vs14/mysqlclient")


// �Ϲ� ����
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

		// Ŀ��Ʈ
		Connect();		
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
				L"Connect() --> Connect Fail... (Error : %d)(Count : %d)", m_iLastError, iConnectCount);

			if (iConnectCount >= 5)
				g_DBDump->Crash();	

			// �ٷ� �õ��ϸ� ������ ���ɼ��� �ֱ� ������ Sleep(0)�� �Ѵ�.
			Sleep(100);
		}

		// ����Ʈ ������ utf8�� ����
		mysql_set_character_set(m_pMySQL, "utf8");

		return true;		
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
	void	CDBConnector::Query(char *szStringFormat, va_list* vlist)
	{
		// 1. ���������� ���� ��Ʈ�� 1���� �̾Ƴ���.		
		HRESULT retval = StringCbVPrintfA(m_cQueryUTF8, eQUERY_MAX_LEN, szStringFormat, *vlist);

		// ���� ���� �� �̻��̸� �α���� ��.
		if (retval != S_OK)
		{

		}		

		// 2. ���� ������
		int Error, Count;
		while (1)
		{
			Error = mysql_query(m_pMySQL, m_cQueryUTF8);

			// ���� �� break;
			if (Error == 0)
				break;

			// ������ ����Ÿ� 5ȸ���� �翬�� �õ�
			if (Error == CR_SOCKET_CREATE_ERROR ||
				Error == CR_CONNECTION_ERROR ||
				Error == CR_CONN_HOST_ERROR ||
				Error == CR_SERVER_HANDSHAKE_ERR ||
				Error == CR_SERVER_GONE_ERROR ||
				Error == CR_INVALID_CONN_HANDLE ||
				Error == CR_SERVER_LOST)
			{
				Count = 0;
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
						L"Query() --> Connect Fail... (Error : %d)(Count : %d)", Error, Count);

					// 5��° �������� ���� ����. �� �̻� ������ ������ ���� �ȵ�.
					if (Count >= 5)
						g_DBDump->Crash();

					// �ٷ� �õ��ϸ� ������ ���ɼ��� �ֱ� ������ Sleep(0)�� �Ѵ�.
					Sleep(0);
				}
			}

		}		

		// 4. ��� �޾Ƶα�
		// �ۿ��� FetchRow() �Լ��� ȣ���ؼ� ���.
		m_pSqlResult = mysql_store_result(m_pMySQL);
	}

	// DBWriter �������� Save ���� ����
	// ������� �������� ����.
	// �����⸸�ϰ� ��� ���� �ʿ� ������ ���
	void	CDBConnector::Query_Save(char *szStringFormat, va_list* vlist)
	{
		// 1. ���������� ���� ��Ʈ�� 1���� �̾Ƴ���.		
		HRESULT retval = StringCbVPrintfA(m_cQueryUTF8, eQUERY_MAX_LEN, szStringFormat, *vlist);

		// ���� ���� �� �̻��̸� �α���� ��.
		if (retval != S_OK)
		{

		}	
		
		// 2. ���� ������
		int Error, Count;
		while (1)
		{			
			Error = mysql_query(m_pMySQL, m_cQueryUTF8);

			// ���� �� break;
			if (Error == 0)
				break;

			// ������ ����Ÿ� 5ȸ���� �翬�� �õ�
			if (Error == CR_SOCKET_CREATE_ERROR ||
				Error == CR_CONNECTION_ERROR ||
				Error == CR_CONN_HOST_ERROR ||
				Error == CR_SERVER_HANDSHAKE_ERR ||
				Error == CR_SERVER_GONE_ERROR ||
				Error == CR_INVALID_CONN_HANDLE ||
				Error == CR_SERVER_LOST)
			{
				Count = 0;
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
		}
		

		// 4. ��� ���� ��, �ٷ� result �Ѵ�.		
		m_pSqlResult = mysql_store_result(m_pMySQL);
		mysql_free_result(m_pSqlResult);
	}
															

	//////////////////////////////////////////////////////////////////////
	// ������ ���� �ڿ� ��� �̾ƿ���.
	//
	// ����� ���ٸ� NULL ����.
	//////////////////////////////////////////////////////////////////////
	MYSQL_ROW	CDBConnector::FetchRow()
	{
		// �� ������ �ѱ��.
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

		// ���������� ���ȴ� ���� ����
		int len = MultiByteToWideChar(CP_UTF8, 0, m_cQueryUTF8, (int)strlen(m_cQueryUTF8), NULL, NULL);
		MultiByteToWideChar(CP_UTF8, 0, m_cQueryUTF8, (int)strlen(m_cQueryUTF8), m_wcQuery, len);
	}	
}

// TLS ����
namespace Library_Jingyu
{
	// ������
	CBConnectorTLS::CBConnectorTLS(WCHAR *DBIP, WCHAR *User, WCHAR *Password, WCHAR *DBName, int DBPort)
	{
		// 1. DBConnector ������ ����
		m_stackConnector = new CLF_Stack< CDBConnector*>(false);

		// 2. TLSIndex�� ���´�.
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
	}


	// �Ҹ���
	// ���ÿ� �����ϰ� �ִ� DBConnector delete.
	// �ڵ����� DBConnector�� �Ҹ��� ȣ��
	CBConnectorTLS::~CBConnectorTLS()
	{
		LONG Count = m_stackConnector->GetInNode();

		// ���ÿ� ����ִ� DBConnector��ŭ ���鼭 delete
		while (Count > 0)
		{
			CDBConnector* retval = m_stackConnector->Pop();
			delete retval;
			Count--;
		}

	}

	//////////////////////////////////////////////////////////////////////
	// MySQL DB ����
	//
	// return : ���� �� true, ���� �� false.
	//////////////////////////////////////////////////////////////////////
	bool		CBConnectorTLS::Connect(void)
	{
		// 1. TLS���� DBConnector�� �����´�.
		CDBConnector* NowDB = (CDBConnector*)TlsGetValue(m_dwTLSIndex);

		// ����, ���� DBConnector�� �Ҵ����� ���� ���¶��, DBConnector����
		if (NowDB == nullptr)
		{
			NowDB = new CDBConnector(m_wcDBIP, m_wcDBUser, m_wcDBPassword, m_wcDBName, m_iDBPort);

			// ���ο� ��� ����
			m_stackConnector->Push(NowDB);

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
	void		CBConnectorTLS::Query(char *szStringFormat, ...)
	{
		// 1. TLS���� DBConnector�� �����´�.
		CDBConnector* NowDB = (CDBConnector*)TlsGetValue(m_dwTLSIndex);

		// ����, ���� DBConnector�� �Ҵ����� ���� ���¶��, DBConnector����
		if (NowDB == nullptr)
		{
			NowDB = new CDBConnector(m_wcDBIP, m_wcDBUser, m_wcDBPassword, m_wcDBName, m_iDBPort);

			// ���ο� ��� ����
			m_stackConnector->Push(NowDB);

			if (TlsSetValue(m_dwTLSIndex, NowDB) == 0)
			{
				DWORD Error = GetLastError();
				g_DBDump->Crash();
			}
		}

		va_list vlist;
		va_start(vlist, szStringFormat);

		// 2. ������ DBConnector�� Query �Լ� ȣ��
		NowDB->Query(szStringFormat, &vlist);

		// 3. �� �� �������� ���ҽ� ����
		va_end(vlist);
	}

	// DBWriter �������� Save ���� ����
	// ������� �������� ����.
	void		CBConnectorTLS::Query_Save(char *szStringFormat, ...)
	{
		// 1. TLS���� DBConnector�� �����´�.
		CDBConnector* NowDB = (CDBConnector*)TlsGetValue(m_dwTLSIndex);

		// ����, ���� DBConnector�� �Ҵ����� ���� ���¶��, DBConnector����
		if (NowDB == nullptr)
		{
			NowDB = new CDBConnector(m_wcDBIP, m_wcDBUser, m_wcDBPassword, m_wcDBName, m_iDBPort);

			// ���ο� ��� ����
			m_stackConnector->Push(NowDB);

			if (TlsSetValue(m_dwTLSIndex, NowDB) == 0)
			{
				DWORD Error = GetLastError();
				g_DBDump->Crash();
			}
		}

		va_list vlist;
		va_start(vlist, szStringFormat);

		// 2. ������ DBConnector�� Query_Save �Լ� ȣ�� ��, ��� �� ����
		NowDB->Query_Save(szStringFormat, &vlist);
		
		// 3. �� �� �������� ���ҽ� ����
		va_end(vlist);
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
		return NowDB->GetLastError();
	}

	WCHAR*		CBConnectorTLS::GetLastErrorMsg(void)
	{
		// 1. TLS���� DBConnector�� �����´�.
		CDBConnector* NowDB = (CDBConnector*)TlsGetValue(m_dwTLSIndex);

		// ����, ���� DBConnector�� �Ҵ����� ���� ���¶��, Crash
		if (NowDB == nullptr)
			g_DBDump->Crash();

		// 2. ������ DBConnector�� GetLastErrorMsg �Լ� ȣ�� �� ����� ����
		return NowDB->GetLastErrorMsg();
	}

}