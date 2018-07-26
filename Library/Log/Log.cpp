#include "stdafx.h"
#include "Log.h"
#include <strsafe.h>
#include <direct.h>		//_mkdir or _wmkdir

namespace Library_Jingyu
{

	// --------------------------------------
	// ���� �α� DB�� Ŀ��Ʈ �� �� �ʿ��� ������.
	// --------------------------------------
	#define GAMELOG_IP			"127.0.0.1"
	#define	GAMELOG_USER		"root"
	#define GAMELOG_PASSWORD	"034689"
	#define GAMELOG_DBNAME		"test_05"
	#define GAMELOG_DBPORT		3306

	#define QUERY_SIZE	1024
	#define SYSTEM_LOG_SIZE	1024


	// --------------------------------------
	// ���� �α� ���� �Լ�
	// --------------------------------------
	// ���� �α׿� Ŀ��Ʈ�ϴ� �Լ�
	//
	// return��
	// 0 : ġ���������� ������ ���� ����. ���� �������� �ƴ�
	// 1 : ���������� �α� �� ����
	// 2 : �̰� ������ ��� ���� ����!
	int Log_GameLogConnect(MYSQL *GameLog_conn, MYSQL *GameLog_connection)
	{
		// �ʱ�ȭ
		mysql_init(GameLog_conn);

		// ���� �α׿� DB ����
		GameLog_connection = mysql_real_connect(GameLog_conn, GAMELOG_IP, GAMELOG_USER, GAMELOG_PASSWORD, GAMELOG_DBNAME, GAMELOG_DBPORT, NULL, 0);
		if (GameLog_connection == NULL)
		{
			printf("Mysql connection error : %s(%d)\n", mysql_error(GameLog_conn), mysql_errno(GameLog_conn));
			return 0;
		}

		//�ѱۻ���������߰�.
		mysql_set_character_set(GameLog_connection, "utf8");

		return 1;
	}


	// ���� Log ���� �Լ�
	// DB�� �����Ѵ�.
	// server / type / code / accountno / param1 ~ 4 / paramString �Է¹ޱ� ����
	//
	// return��
	// 0 : ġ���������� ������ ���� ����. ���� �������� �ƴ�
	// 1 : ���������� �α� �� ����
	// 2 : �̰� ������ ��� ���� ����!
	int Log_GameLogSave(MYSQL *conn, MYSQL *connection, int iServer, int iType, int iCode, int iAccountNo,
		const char* paramString, int iParam1, int iParam2, int iParam3, int iParam4)
	{
		// 1. ���̺� �̸� ���� 
		// ���̺��� �� ������ �����ȴ�.
		// ������ �ȿ� ������ ���̺� �̸��� �����ϱ� ������ �Ź� �����Ѵ�.
		char cTableName[200];

		SYSTEMTIME	stNowTime;
		GetLocalTime(&stNowTime);
		StringCbPrintfA(cTableName, 200, "gamelog_%d%02d", stNowTime.wYear, stNowTime.wMonth);

		// 2. ������ �����
		char cQuery[QUERY_SIZE];
		StringCbPrintfA(cQuery, QUERY_SIZE, "INSERT INTO `%s`(`server`, `type`, `code`, `accountno`, `param1`, `param2`, `param3`, `param4`, `paramstring`) "
			"VALUES (%d, %d, %d, %d, %d, %d, %d, %d,'%s')",
			cTableName, iServer, iType, iCode, iAccountNo, iParam1, iParam2, iParam3, iParam4, paramString);


		// 3. ������ ������ ������
		int Error = mysql_query(conn, cQuery);

		// 4. ���� ���� �� ���� ó��
		if (Error != 0)
		{
			// case 1. ������ ������ ----> 5ȸ���� �翬�� �õ�
			if (Error == CR_SOCKET_CREATE_ERROR ||
				Error == CR_CONNECTION_ERROR ||
				Error == CR_CONN_HOST_ERROR ||
				Error == CR_SERVER_HANDSHAKE_ERR ||
				Error == CR_SERVER_GONE_ERROR ||
				Error == CR_INVALID_CONN_HANDLE ||
				Error == CR_SERVER_LOST)
			{

				int Count = 0;
				while (Count < 5)
				{
					// DB �翬��
					connection = mysql_real_connect(conn, GAMELOG_IP, GAMELOG_USER, GAMELOG_PASSWORD, GAMELOG_DBNAME, GAMELOG_DBPORT, NULL, 0);
					if (connection != NULL)
						break;

					Count++;
				}

				// 5ȸ ���� �õ��ߴµ��� ���� ���и�, ���� ��� ���� ����
				if (connection == NULL)
				{
					// ���� �ý��� �α� ���ܾ���. ��µ� �ϰ�!
					printf("Log_GameLogSave()1. Mysql connection error : %s(%d)\n", mysql_error(conn), mysql_errno(conn));
					return 2;
				}

				// ������ ���������� ���� �ٽ� ��������.
				else
				{
					// �� ������ ���ٸ�,
					if (mysql_query(connection, cQuery) != 0)
					{
						// ���̺��� ��� �� ������ ���, ���̺� ������ �ٽ� ���� ��������.
						// �׷��� ������ ������, �ý��� �α� ����� ���� ����
						if (mysql_errno(conn) == 1146)
						{
							if (CreateTableAndQuery(conn, connection, cTableName) != 0)
							{
								// ���� �ý��� �α� ���ܾ���. ��µ� �ϰ�!
								printf("Log_GameLogSave()2. Mysql query error : %s(%d)\n", mysql_error(conn), mysql_errno(conn));
								return 2;
							}
						}

						else
						{
							// �װ� �ƴ϶�� �ý��� �α� ����� ���� ����.
							// ���� �ý��� �α� ���ܾ���. ��µ� �ϰ�!
							printf("Log_GameLogSave()3. Mysql query error : %s(%d)\n", mysql_error(conn), mysql_errno(conn));
							return 2;
						}

					}
				}
			}

			// case 2. �� ���� ���� ���̺��� ���� ----> ���̺� ���� ���� ���� ��, ������ �ٽ� ����
			else if (mysql_errno(conn) == 1146)
			{
				if (CreateTableAndQuery(conn, connection, cTableName) != 0)
				{
					// ���̺� ���� ���� �����ٰ� ����������, ���� �ý��� �α� ���ܾ���. ��µ� �ϰ�!
					printf("Log_GameLogSave()4. Mysql query error : %s(%d)\n", mysql_error(conn), mysql_errno(conn));
					return 2;
				}

				// ���̺� ���� ������ �� ������, ���� ������ �ٽ� ������.
				// ���⼭�� ������ ������ �׳� ���� �����Ŵ...
				if (mysql_query(conn, cQuery) != 0)
				{
					// ���� �ý��� �α� ���ܾ���.��µ� �ϰ�!
					printf("Log_GameLogSave()5. Mysql query error : %s(%d)\n", mysql_error(conn), mysql_errno(conn));
					return 2;
				}

			}

			// case 3. ���� ������ �������  ----> �ý��� �α� ����
			else
			{
				// ���� �ý��� �α� ���ܾ���.��µ� �ϰ�!
				printf("Log_GameLogSave()6. Mysql query error : %s(%d)\n", mysql_error(conn), mysql_errno(conn));
				return 2;
			}

		}

		return 1;
	}


	// �ѹ� �������� ���ȴµ�, ���̺��� ���ٸ�, ���̺� ���� �� ������ �ٽ� ������ �Լ�.
	//
	// return��
	// mysql_query�� ���ϰ��� �״�� �����Ѵ�.
	int CreateTableAndQuery(MYSQL *conn, MYSQL *connection, char* cTableName)
	{
		// ���̺� ���� ���� ����
		char TableCreate[QUERY_SIZE];
		StringCbPrintfA(TableCreate, 200, "CREATE TABLE %s LIKE gamelog_template", cTableName);

		// ���̺� ���� ���� ������
		int Error = mysql_query(connection, TableCreate);

		return Error;
	}











	// --------------------------------------
	// �ý��� �α� Ŭ����
	// �ý��� �α״� "����"�� �����Ѵ�.
	// --------------------------------------
	// ������
	// �̱����� ���� Private���� ����.
	CSystemLog::CSystemLog()
	{
		// SRWLOCK �ʱ�ȭ
		InitializeSRWLock(&m_lSrwlock);

		// �α� ī��Ʈ 0���� �ʱ�ȭ
		m_ullLogCount = 0;
	}

	// �̱��� ���
	CSystemLog* CSystemLog::GetInstance()
	{
		static CSystemLog cCSystemLog;
		return &cCSystemLog;
	}

	// ���丮 ���� ��, ���丮 �����ϴ� �Լ�
	//
	// return ��
	// false : ���丮 ������ ������ ��, ȭ�鿡 ���� ��� �� false ����
	// true : ���丮 ���� ����
	bool CSystemLog::SetDirectory(const TCHAR* directory)
	{
		// 1. ���丮 �����ϱ�
		if (_tmkdir(directory) == -1)
		{
			// �̹� ���丮�� ������ �־ �����ѰŸ�, ���� �ƴ�
			// �װ� �ƴ϶�� ������ ȭ�鿡 ���.
			if (errno != EEXIST)
			{
				perror("���� ���� ���� - ������ �̹� �ְų� ����Ȯ��\n");
				printf("errorno : %d", errno);
				return false;
			}
		}

		// 2. ������ ���丮 �����ϱ�.
		// ������ ���� ������ �� ��ο� ����ȴ�.
		_tcscpy_s(m_tcDirectory, _Mycountof(m_tcDirectory), directory);

		return true;
	}

	// �α� ������ �����ϴ� �Լ�
	// ���⼭ ���õ� �������� ���� ū �α׸� ����
	// ex) m_iLogLevel�� LEVEL_DEBUG���� ���õȴٸ�, LEVEL_DEBUG���� ū �αװ� ����
	void CSystemLog::SetLogLeve(en_LogLevel level)
	{
		m_iLogLevel = level;
	}

	// ������ ���Ͽ� �ؽ��z�� �����ϴ� �Լ�
	bool CSystemLog::FileSave(TCHAR* FileName, TCHAR* SaveText)
	{
		// 5. ���� ����.
		// �� �ɰ� �Ѵ�. -------------
		AcquireSRWLockExclusive(&m_lSrwlock);

		FILE *fp;
		if (_tfopen_s(&fp, FileName, _T("at")) != 0)
		{
			// ���� ���� ���� �� �޽��� ���.
			printf("CSystemLog. _tfopen_s() error. (%d)", errno);

			ReleaseSRWLockExclusive(&m_lSrwlock);
			return false;
		}

		if (_fputts(SaveText, fp) == EOF)
		{
			// ���� ���� ���� �� �޽��� ���
			printf("CSystemLog. _fputts() error. (%d)", errno);

			ReleaseSRWLockExclusive(&m_lSrwlock);
			return false;
		}

		fclose(fp);

		ReleaseSRWLockExclusive(&m_lSrwlock);
		// �� Ǭ��. ----------------

		return true;
	}

	// Log ���� �Լ�. �������� ���
	//
	// return ��
	// true : �α� ���������� ����� or �α� ������ ���� �α׸� ����õ���. ������ �ƴϴϱ� �� ȭ�鿡 ��¸� �ϰ� true ����
	// false : �α� ������ ������ ���� �� �α� ������ �ȵ� ��Ȳ�� false ��ȯ.
	bool CSystemLog::LogSave(const TCHAR* type, en_LogLevel level, const TCHAR* stringFormat, ...)
	{
		// 1. �Է¹��� �α� ���� ��Ʈ�� ����
		TCHAR tcLogLevel[10];

		switch (level)
		{
		case CSystemLog::LEVEL_DEBUG:
			_tcscpy_s(tcLogLevel, _Mycountof(tcLogLevel), _T("DEBUG"));
			break;
		case CSystemLog::LEVEL_WARNING:
			_tcscpy_s(tcLogLevel, _Mycountof(tcLogLevel), _T("WARNING"));
			break;
		case CSystemLog::LEVEL_ERROR:
			_tcscpy_s(tcLogLevel, _Mycountof(tcLogLevel), _T("ERROR"));
			break;
		case CSystemLog::LEVEL_SYSTEM:
			_tcscpy_s(tcLogLevel, _Mycountof(tcLogLevel), _T("SYSTEM"));
			break;
		default:
			printf("'level' Parameter Error!!!\n");
			return false;
		}

		// 2. �ð� ���ؿ���. �� �Լ������� ��� �� �ð����� �ۼ��Ѵ�.
		SYSTEMTIME	stNowTime;
		GetLocalTime(&stNowTime);

		// 3. ���� �̸� �����
		// [���_Ÿ��.txt]
		TCHAR tcFileName[100];
		HRESULT retval = StringCbPrintf(tcFileName, 100, _T("%s\\%d%02d_%s.txt"), m_tcDirectory, stNowTime.wYear, stNowTime.wMonth, type);

		// StringCbPrintf ����ó��. S_OK�� �ƴϸ� ������ ����
		// �������� �������ٰ� �α� ����.
		if (retval != S_OK)
		{
			// ���� �̸��� ������� �α� ���� ����. �׳� ����ϰ� ��.
			printf("LOG_ERROR_FileName. LogSave Error!!\n");

			return false;

		}

		// 4. ���������� ���� ��Ʈ�� 1���� �̾Ƴ���.
		TCHAR tcContent[SYSTEM_LOG_SIZE];

		va_list vlist;
		va_start(vlist, stringFormat);
		retval = StringCbVPrintf(tcContent, SYSTEM_LOG_SIZE, stringFormat, vlist);
		va_end(vlist);

		// ���Ͷ����� �α� ī��Ʈ 1 ����
		ULONGLONG Count = InterlockedIncrement(&m_ullLogCount);

		// StringCbVPrintf() ���� ó��. S_OK�� �ƴϸ� ������ ����
		// �������� �������ٰ� �α� ����.
		if (retval != S_OK)
		{
			TCHAR tcLogSaveingError[SYSTEM_LOG_SIZE];

			// ���� �αװ� �ϼ����� �ʾұ� ������.. ���� ��ǥ���� tcContent�� ����غ���.
			// tcContent�� �ʹ� �� ���� ������ �߶󳽴�.
			// �� 700���� ����
			_tcsncpy_s(tcContent, SYSTEM_LOG_SIZE, tcContent, 700);

			StringCbPrintf(tcLogSaveingError, SYSTEM_LOG_SIZE, _T("[%s] [%d-%02d-%02d %02d:%02d:%02d / %s / %09u] %s-->%s\n"),
				_T("LOG_ERROR_Content"), stNowTime.wYear, stNowTime.wMonth, stNowTime.wDay, stNowTime.wHour, stNowTime.wMinute, stNowTime.wSecond, tcLogLevel, Count, type, tcContent);

			// ���Ͽ� ����
			FileSave(tcFileName, tcLogSaveingError);

			printf("LOG_ERROR_Content. LogSave Error!!\n");

			return false;

		}


		// 4. �Է¹��� Ÿ�� + �������� ���� �̿��� ���� ���Ͽ� ������ �α� ��Ʈ��, ȭ�鿡 ����� ��Ʈ���� �����.
		TCHAR tcSaveBuff[SYSTEM_LOG_SIZE];
		TCHAR tcConsoleBuff[SYSTEM_LOG_SIZE];

		// ����� �α� ��Ʈ�� ����� ---------------------
		retval = StringCbPrintf(tcSaveBuff, SYSTEM_LOG_SIZE, _T("[%s] [%d-%02d-%02d %02d:%02d:%02d / %s / %09u] %s\n"),
			type, stNowTime.wYear, stNowTime.wMonth, stNowTime.wDay, stNowTime.wHour, stNowTime.wMinute, stNowTime.wSecond, tcLogLevel, Count, tcContent);

		// StringCbPrintf() ���� ó��. S_OK�� �ƴϸ� ������ ����
		// �������� �������ٰ� �α� ����.
		if (retval != S_OK)
		{
			TCHAR tcLogSaveingError[SYSTEM_LOG_SIZE];

			// tcContent�� �ʹ� �� ���� ������ �߶󳽴�.
			// �� 700���� ����
			_tcsncpy_s(tcContent, SYSTEM_LOG_SIZE, tcContent, 700);

			StringCbPrintf(tcLogSaveingError, SYSTEM_LOG_SIZE, _T("[%s] [%d-%02d-%02d %02d:%02d:%02d / %s / %09u] %s-->%s\n"),
				_T("LOG_ERROR_SaveBuff"), stNowTime.wYear, stNowTime.wMonth, stNowTime.wDay, stNowTime.wHour, stNowTime.wMinute, stNowTime.wSecond, tcLogLevel, Count, type, tcContent);

			// ���Ͽ� ����
			FileSave(tcFileName, tcLogSaveingError);

			printf("LOG_ERROR_SaveBuff. LogSave Error!!\n");

			return false;
		}

		// ȭ�� ��¿� ��Ʈ�� ����� ---------------------
		retval = StringCbPrintf(tcConsoleBuff, SYSTEM_LOG_SIZE, _T("[%s] [%02d:%02d:%02d / %s ] %s\n"),
			type, stNowTime.wHour, stNowTime.wMinute, stNowTime.wSecond, tcLogLevel, tcContent);

		// StringCbPrintf() ���� ó��. S_OK�� �ƴϸ� ������ ����
		// �������� �������ٰ� �α� ����.
		if (retval != S_OK)
		{
			TCHAR tcLogSaveingError[SYSTEM_LOG_SIZE];

			// tcContent�� �ʹ� �� ���� ������ �߶󳽴�.
			// �� 700���� ����
			_tcsncpy_s(tcContent, SYSTEM_LOG_SIZE, tcContent, 700);

			StringCbPrintf(tcLogSaveingError, SYSTEM_LOG_SIZE, _T("[%s] [%d-%02d-%02d %02d:%02d:%02d / %s / %09u] %s-->%s\n"),
				_T("LOG_ERROR_ConsoleBuff"), stNowTime.wYear, stNowTime.wMonth, stNowTime.wDay, stNowTime.wHour, stNowTime.wMinute, stNowTime.wSecond, tcLogLevel, Count, type, tcContent);

			// ���Ͽ� ����
			FileSave(tcFileName, tcLogSaveingError);

			printf("LOG_ERROR_ConsoleBuff. LogSave Error!!\n");

			return false;
		}

		// 5. ȭ�鿡 ����Ѵ�.
		_tprintf(_T("%s"), tcConsoleBuff);


		// 6. ���Ͽ� ����
		// �̹��� �Է¹��� �α� ������, ������ �α� �������� ũ�ų� ���ٸ�, ���Ͽ� ����
		if (level >= m_iLogLevel)
			FileSave(tcFileName, tcSaveBuff);

		return true;

	}

	// 16������ 10������ ��ȯ
	// Source�� �ִ� ������ ��, Bufflen��ŭ�� 16������ ��ȯ �� Desc�� �����Ѵ�.
	void CSystemLog::HexToDecimal(char* Desc, BYTE* Source, int Bufflen)
	{
		BYTE* pin = Source;
		const char* hex = "0123456789ABCDEF";

		char* pout = Desc;

		for (int i = 0; i < Bufflen - 1; ++i)
		{
			*pout++ = hex[(*pin >> 4) & 0xF];
			*pout++ = hex[(*pin++) & 0xF];
			*pout++ = ':';
		}
		*pout++ = hex[(*pin >> 4) & 0xF];
		*pout++ = hex[(*pin++) & 0xF];
		*pout = 0;
	}

	// Log�� Hex�� �����ϴ� �Լ�.
	// ��Ŷ �������� �� ��Ŷ�� ����� �뵵�̸�, ������ BYTE���·� Buff�޴´ٴ� ����!!
	//
	// return ��
	// true : �α� ���������� ����� or �α� ������ ���� �α׸� ����õ���. ������ �ƴϴϱ� �� ȭ�鿡 ��¸� �ϰ� true ����
	// false : �α� ������ ������ ���� �� �α� ������ �ȵ� ��Ȳ�� false ��ȯ.
	bool CSystemLog::LogHexSave(const TCHAR* type, en_LogLevel level, const TCHAR* PacketDesc, BYTE* Buff, int Bufflen)
	{
		// 1. �Է¹��� �α� ���� ��Ʈ�� ����
		TCHAR tcLogLevel[10];

		switch (level)
		{
		case CSystemLog::LEVEL_DEBUG:
			_tcscpy_s(tcLogLevel, _Mycountof(tcLogLevel), _T("DEBUG"));
			break;
		case CSystemLog::LEVEL_WARNING:
			_tcscpy_s(tcLogLevel, _Mycountof(tcLogLevel), _T("WARNING"));
			break;
		case CSystemLog::LEVEL_ERROR:
			_tcscpy_s(tcLogLevel, _Mycountof(tcLogLevel), _T("ERROR"));
			break;
		case CSystemLog::LEVEL_SYSTEM:
			_tcscpy_s(tcLogLevel, _Mycountof(tcLogLevel), _T("SYSTEM"));
			break;
		default:
			printf("'level' Parameter Error!!!\n");
			return false;
		}

		// 2. �ð� ���ؿ���. �� �Լ������� ��� �� �ð����� �ۼ��Ѵ�.
		SYSTEMTIME	stNowTime;
		GetLocalTime(&stNowTime);

		// 3. ���� �̸� �����
		// [���_Ÿ��.txt]
		TCHAR tcFileName[100];
		HRESULT retval = StringCbPrintf(tcFileName, 100, _T("%s\\%d%02d_%s.txt"), m_tcDirectory, stNowTime.wYear, stNowTime.wMonth, type);

		// StringCbPrintf ����ó��. S_OK�� �ƴϸ� ������ ����
		// �������� �������ٰ� �α� ����.
		if (retval != S_OK)
		{
			// ���� �̸��� ������� �α� ���� ����. �׳� ����ϰ� ��.
			printf("LOG_ERROR_FileName. LogSave Error!!\n");

			return false;

		}


		// 4. �Է¹��� Buff�� 16������ ��ȯ�ϱ�.
		// 16������ 1����Ʈ�� ���� 2���� ǥ���ϱ� ������, �Է¹��� len�� 2�踸ŭ �����Ҵ�.
		// ����Ʈ ������ : �� ����ֱ� ���� Bufflen��ŭ �߰�.
		char* bContent = new char[Bufflen * 2 + Bufflen];
		HexToDecimal(bContent, Buff, Bufflen);

		// ��ȯ�� 16������ TCHAR ���·� ����
		//len = MultiByteToWideChar(CP_UTF8, 0, Json_Buff, (int)strlen(Json_Buff), NULL, NULL);
		int len = (int)strlen(bContent);

		TCHAR* tcHex = new TCHAR[Bufflen * 2 + Bufflen];
		MultiByteToWideChar(CP_UTF8, 0, bContent, (int)strlen(bContent), tcHex, len);
		tcHex[(Bufflen * 2 + Bufflen) - 1] = '\0';


		// 5. ��ȯ�� 16������ PacketDesc���ڸ� ���ļ� 1���� �α� ��Ʈ������ �����.
		TCHAR tcContent[SYSTEM_LOG_SIZE];
		retval = StringCbPrintf(tcContent, SYSTEM_LOG_SIZE, _T("%s : %s"), PacketDesc, tcHex);

		delete[] bContent;
		delete[] tcHex;



		// 6. ���� ���Ͽ� ������ �α� ��Ʈ��, ȭ�鿡 ����� ��Ʈ���� �����.
		TCHAR tcSaveBuff[SYSTEM_LOG_SIZE];
		TCHAR tcConsoleBuff[SYSTEM_LOG_SIZE];

		// ���Ͷ����� �α� ī��Ʈ 1 ����
		ULONGLONG Count = InterlockedIncrement(&m_ullLogCount);

		// ����� �α� ��Ʈ�� ����� ---------------------
		retval = StringCbPrintf(tcSaveBuff, SYSTEM_LOG_SIZE, _T("[%s] [%d-%02d-%02d %02d:%02d:%02d / %s / %09u] %s\n"),
			type, stNowTime.wYear, stNowTime.wMonth, stNowTime.wDay, stNowTime.wHour, stNowTime.wMinute, stNowTime.wSecond, tcLogLevel, Count, tcContent);

		// StringCbPrintf() ���� ó��. S_OK�� �ƴϸ� ������ ����
		// �������� �������ٰ� �α� ����.
		if (retval != S_OK)
		{
			TCHAR tcLogSaveingError[SYSTEM_LOG_SIZE];

			// tcSaveBuff�� �ʹ� �� ���� ������ �߶󳽴�.
			// �� 700���� ����
			_tcsncpy_s(tcSaveBuff, SYSTEM_LOG_SIZE, tcSaveBuff, 700);

			StringCbPrintf(tcLogSaveingError, SYSTEM_LOG_SIZE, _T("[%s] [%d-%02d-%02d %02d:%02d:%02d / %s / %09u] %s-->%s\n"),
				_T("LOG_ERROR_SaveBuff"), stNowTime.wYear, stNowTime.wMonth, stNowTime.wDay, stNowTime.wHour, stNowTime.wMinute, stNowTime.wSecond, tcLogLevel, Count, type, tcSaveBuff);

			// ���Ͽ� ����
			FileSave(tcFileName, tcLogSaveingError);

			printf("LOG_ERROR_SaveBuff. LogSave Error!!\n");

			return false;
		}

		// ȭ�� ��¿� ��Ʈ�� ����� ---------------------
		retval = StringCbPrintf(tcConsoleBuff, SYSTEM_LOG_SIZE, _T("[%s] [%02d:%02d:%02d / %s ] %s\n"),
			type, stNowTime.wHour, stNowTime.wMinute, stNowTime.wSecond, tcLogLevel, tcContent);

		// StringCbPrintf() ���� ó��. S_OK�� �ƴϸ� ������ ����
		// �������� �������ٰ� �α� ����.
		if (retval != S_OK)
		{
			TCHAR tcLogSaveingError[SYSTEM_LOG_SIZE];

			// tcConsoleBuff�� �ʹ� �� ���� ������ �߶󳽴�.
			// �� 700���� ����
			_tcsncpy_s(tcConsoleBuff, SYSTEM_LOG_SIZE, tcConsoleBuff, 700);

			StringCbPrintf(tcLogSaveingError, SYSTEM_LOG_SIZE, _T("[%s] [%d-%02d-%02d %02d:%02d:%02d / %s / %09u] %s-->%s\n"),
				_T("LOG_ERROR_ConsoleBuff"), stNowTime.wYear, stNowTime.wMonth, stNowTime.wDay, stNowTime.wHour, stNowTime.wMinute, stNowTime.wSecond, tcLogLevel, Count, type, tcConsoleBuff);

			// ���Ͽ� ����
			FileSave(tcFileName, tcLogSaveingError);

			printf("LOG_ERROR_ConsoleBuff. LogSave Error!!\n");

			return false;
		}



		// 7. ȭ�鿡 ����Ѵ�.
		_tprintf(_T("%s"), tcConsoleBuff);



		// 8. ���Ͽ� ����
		// �̹��� �Է¹��� �α� ������, ������ �α� �������� ũ�ų� ���ٸ�, ���Ͽ� ����
		if (level >= m_iLogLevel)
			FileSave(tcFileName, tcSaveBuff);

		return true;
	}
}