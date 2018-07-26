
// *********************************************
// ���� �α׿� �ý��� �α׸� ����� �� �ִ� �������
// ���� �α� : DB�� ����
// �ý��� �α� : ���� ��� ���� �� ���Ͽ� ����
// *********************************************

#ifndef __LOG_H__
#define __LOG_H__

#include "Mysql\include\mysql.h"
#include "Mysql\include\errmsg.h"

#pragma comment(lib, "Mysql/lib/vs14/mysqlclient.lib")



namespace Library_Jingyu
{
	#define _Mycountof(_array)  sizeof(_array) / sizeof(TCHAR)
	

	// �̱��� ���� ��, ���丮 ���� define
	#define SYSLOG_SET_DIRECTORY(_dir)								\
	CSystemLog* cLog = CSystemLog::GetInstance();					\
	cLog->SetDirectory(L"Test")										\

	// LogLevel ���� define
	#define SYSLOG_SET_LEVEL(_level)								\
	cLog->SetLogLeve(_level)										\

	// �ý��� �α� ����� (�Ϲ�)
	#define LOG(_type, _level, _format, ...)						\
	cLog->LogSave(_type, _level, _format, ## __VA_ARGS__)			\

	// �ý��� �α� ����� (16����)
	// ������ BYTE���·� Buff �޾ƾ���.
	#define LOG_HEX(_type, _level, _desc, _buff)					\
	cLog->LogHexSave(_type, _level, _desc, _buff, sizeof(_buff))	\


	// --------------------------------------
	// ���� �α� ���� �Լ�
	// --------------------------------------

	// ���� �α׿� Ŀ��Ʈ�ϴ� �Լ�
	//
	// return��
	// 0 : ġ���������� ������ ���� ����. ���� �������� �ƴ�
	// 1 : ���������� �α� �� ����
	// 2 : �̰� ������ ��� ���� ����!
	int Log_GameLogConnect(MYSQL *GameLog_conn, MYSQL *GameLog_connection);

	// ���� Log ���� �Լ�
	// DB�� �����Ѵ�.
	// server / type / code / accountno / param1 ~ 4 / paramString �Է¹ޱ� ����
	//
	// return��
	// 0 : ġ���������� ������ ���� ����. ���� �������� �ƴԤ�
	// 1 : ���������� �α� �� ����
	// 2 : �̰� ������ ��� ���� ����!
	int Log_GameLogSave(MYSQL *conn, MYSQL *connection, int iServer, int iType, int iCode, int iAccountNo,
		const char* paramString = "0", int iParam1 = 0, int iParam2 = 0, int iParam3 = 0, int iParam4 = 0);

	// �ѹ� �������� ���ȴµ�, ���̺��� ���ٸ�, ���̺� ���� �� ������ �ٽ� ������ �Լ�.
	//
	// return��
	// mysql_query�� ���ϰ��� �״�� �����Ѵ�.
	int CreateTableAndQuery(MYSQL *conn, MYSQL *connection, char* cTableName);








	// --------------------------------------
	// �ý��� �α� Ŭ����
	// �ý��� �α״� "����"�� �����Ѵ�.
	// --------------------------------------
	class CSystemLog
	{
	public:
		// �α� ����
		// ex)LEVEL_DEBUG���� ���õȴٸ�, LEVEL_DEBUG���� ū ������ �αװ� ����
		enum en_LogLevel
		{
			LEVEL_DEBUG = 0, LEVEL_WARNING, LEVEL_ERROR, LEVEL_SYSTEM
		};

	private:
		// ������
		// �̱����� ���� Private���� ����.
		CSystemLog();

		// �α� ���� ����, ���丮 ����
		en_LogLevel m_iLogLevel;
		TCHAR m_tcDirectory[1024];

		// Log()�Լ����� fopen �� ���� �����Ҷ� ����ϴ� ��.
		SRWLOCK m_lSrwlock;

		// �α� ������ �� ���� �����ϰ� 1�� �����ϴ� ī��Ʈ.
		// �α׿� �Բ� ��µȴ�.
		ULONGLONG m_ullLogCount;

	private:
		// ������ ���Ͽ� �ؽ��z�� �����ϴ� �Լ�
		bool FileSave(TCHAR* FileName, TCHAR* SaveText);

		// 16������ 10������ ��ȯ
		void HexToDecimal(char* Desc, BYTE* Source, int Bufflen);
		
	public:
		// �̱��� ���
		static CSystemLog* GetInstance();

		// ���丮 ���� ��, ���丮 �����ϴ� �Լ�
		bool SetDirectory(const TCHAR* directory);

		// �α� ������ �����ϴ� �Լ�
		// ���⼭ ���õ� �������� ���� ū �α׸� ����
		// ex) m_iLogLevel�� LEVEL_DEBUG���� ���õȴٸ�, LEVEL_DEBUG���� ū �αװ� ����
		void SetLogLeve(en_LogLevel level);

		// Log ���� �Լ�. �������� ���
		//
		// return ��
		// true : �α� ���������� ����� or �α� ������ ���� �α׸� ����õ���. ������ �ƴϴϱ� �� ȭ�鿡 ��¸� �ϰ� true ����
		// false : �α� ������ ������ ���� �� �α� ������ �ȵ� ��Ȳ�� false ��ȯ.
		bool LogSave(const TCHAR* type, en_LogLevel level, const TCHAR* stringFormat, ...);

		// Log�� Hex�� �����ϴ� �Լ�.
		// ��Ŷ �������� �� ��Ŷ�� ����� �뵵�̸�, ������ BYTE���·� Buff�޴´ٴ� ����!!
		//
		// return ��
		// true : �α� ���������� ����� or �α� ������ ���� �α׸� ����õ���. ������ �ƴϴϱ� �� ȭ�鿡 ��¸� �ϰ� true ����
		// false : �α� ������ ������ ���� �� �α� ������ �ȵ� ��Ȳ�� false ��ȯ.
		bool LogHexSave(const TCHAR* type, en_LogLevel level, const TCHAR* PacketDesc, BYTE* Buff, int Bufflen);

	};
}


#endif // !__LOG_H__
