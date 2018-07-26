// ConsoleApplication1.cpp: 콘솔 응용 프로그램의 진입점을 정의합니다.
//

#include "stdafx.h"
#include "Log\Log.h"

using namespace Library_Jingyu;

void Test()
{
	UCHAR buf[] = { 100, 124, 200, 255 };

	char str[12];

	UCHAR* pin = buf;
	const char* hex = "0123456789ABCDEF";

	char* pout = str;

	for (int i = 0; i < sizeof(buf) - 1; ++i)
	{
		*pout++ = hex[(*pin >> 4) & 0xF];
		*pout++ = hex[(*pin++) & 0xF];
		*pout++ = ':';
	}
	*pout++ = hex[(*pin >> 4) & 0xF];
	*pout++ = hex[(*pin++) & 0xF];
	*pout = 0;

	printf("%s\n", str);
	/*printf("%x\n", buf[0]);
	printf("%x\n", buf[1]);
	printf("%x\n", buf[2]);
	printf("%x\n", buf[3]);*/
}



int _tmain()
{	
	SYSLOG_SET_DIRECTORY(L"Test");
	SYSLOG_SET_LEVEL(CSystemLog::en_LogLevel::LEVEL_ERROR);
	
	// 시스템 로그 테스트(일반)
	const TCHAR* abcc = _T("진규....");
	LOG(L"Battle", CSystemLog::en_LogLevel::LEVEL_ERROR, L"Shop [%d, %s]", 18245, abcc);

	// 시스템 로그 테스트 (핵사)
	BYTE buf[] = { 0, 112, 100, 255, 123, 147, 157, 015, 15, 11 };
	LOG_HEX(L"Packet", CSystemLog::en_LogLevel::LEVEL_ERROR, L"LoginPacket", buf);
	



	// GameLogDB와 연결하기
	MYSQL GameLog_conn;
	MYSQL* GameLog_connection = nullptr;
	Log_GameLogConnect(&GameLog_conn, GameLog_connection);

	// GameLogDB에 로그 저장
	int iServer = 1;
	int iType = 20;
	int iCode = 100;
	int iaccountNo = 1;	
	const char* cParamString = "songsong";
	int iParam1 = 10;

	int Retval = Log_GameLogSave(&GameLog_conn, GameLog_connection, iServer, iType, iCode, iaccountNo, cParamString, iParam1);
	if (Retval == 2)
		return 0;


	// DB와 연결 끊기
	mysql_close(GameLog_connection);

    return 0;
}

