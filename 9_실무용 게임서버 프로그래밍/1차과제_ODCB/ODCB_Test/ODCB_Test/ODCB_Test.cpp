// ODCB_Test.cpp: 콘솔 응용 프로그램의 진입점을 정의합니다.
//

#include "stdafx.h"
#include <Windows.h>
#include <sql.h>
#include <sqlext.h>
#include <locale.h>

SQLHENV		hEnv;		// 환경 핸들
SQLHDBC		hDbc;		// 연결 핸들
SQLHSTMT	hStmt;		// 명령 핸들

HRESULT		hResult;


#define _Mycountof(Array)		sizeof(Array) / sizeof(TCHAR)


enum {ENV = 1, DBC, STMT};


void ErrorFunc(int Type);

int _tmain()
{
	// 유니코드 스탠다드 출력 환경 셋팅
	_tsetlocale(LC_ALL, L"korean");	

	TCHAR			BuffID[40];
	TCHAR			BuffPass[40];
	TCHAR			BuffNick[40];

	SQLLEN			IBuffID, IBuffPass, IBuffNick, IBuffScore;



	// -------------------
	// 환경 핸들을 할당하고 버전 속성을 설정한다.
	// -------------------
	if (SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &hEnv) != SQL_SUCCESS)
	{
		ErrorFunc(ENV);
		return 0;
	}

	if (SQLSetEnvAttr(hEnv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, SQL_IS_INTEGER) != SQL_SUCCESS)
	{
		ErrorFunc(ENV);
		return 0;
	}






	// -------------------
	// 연결 핸들을 할당
	// -------------------
	if (SQLAllocHandle(SQL_HANDLE_DBC, hEnv, &hDbc) != SQL_SUCCESS)
	{
		// 연결 핸들 핸들에 대한 에러 처리
		ErrorFunc(DBC);
		return 0;
	}




	// -------------------
	// 접속한다
	// -------------------
	TCHAR szConnect[1024] = L"Driver={MySQL ODBC 8.0 Unicode Driver}; Server=127.0.0.1; Database=session_test; User=root; Password=034689; Option=3;";
	SQLTCHAR OutCon[1024];
	SQLSMALLINT	cbOutCon;

	if (SQLDriverConnect(hDbc, NULL, (SQLTCHAR*)szConnect,
		(SQLSMALLINT)_tcslen(szConnect), OutCon, _Mycountof(OutCon), &cbOutCon, SQL_DRIVER_NOPROMPT) != SQL_SUCCESS)
	{
		// 접속에 대한 에러처리
		ErrorFunc(DBC);
		return 0;
	}





	// -------------------
	// 명령핸들 할당
	// -------------------
	if (SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt) != SQL_SUCCESS)
	{
		// 접속에 대한 에러처리
		ErrorFunc(STMT);
		return 0;
	}





	// -------------------
	// 바인딩 하기
	// -------------------
	SQLBindCol(hStmt, 2, SQL_C_TCHAR, BuffID, _Mycountof(BuffID), &IBuffID);
	SQLBindCol(hStmt, 3, SQL_C_TCHAR, BuffPass, _Mycountof(BuffPass), &IBuffPass);
	SQLBindCol(hStmt, 4, SQL_C_TCHAR, BuffNick, _Mycountof(BuffNick), &IBuffNick);

	// 에러처리 한번 해보기
	ErrorFunc(STMT);






	// -------------------
	// 실행하기
	// -------------------
	SQLExecDirect(hStmt, (SQLTCHAR*)L"SELECT * FROM accounttbl", SQL_NTS);
	ErrorFunc(STMT);





	// -------------------
	// 결과 출력
	// -------------------
	while (SQLFetch(hStmt) != SQL_NO_DATA)
	{
		_tprintf(L"ID : %s, Pass : %s, Nick : %s\n", BuffID, BuffPass, BuffNick);
		ErrorFunc(STMT);
	}




	// -------------------
	// 마무리
	// -------------------
	SQLCloseCursor(hStmt);
	SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
	SQLDisconnect(hDbc);
	SQLFreeHandle(SQL_HANDLE_DBC, hDbc);
	SQLFreeHandle(SQL_HANDLE_ENV, hEnv);
	


    return 0;
}


void ErrorFunc(int Type)
{
	// SQLGetDiagRec()에서 사용할 인자들. 
	TCHAR			State[1024] = { 0, };
	TCHAR			Message[1024] = { 0, };
	SQLSMALLINT		MsgLen = 0;
	int				NativeError = 0;


	// 환경 핸들일 시
	if (Type == ENV)
		SQLGetDiagRec(SQL_HANDLE_ENV, &hEnv, 1, State, (SQLINTEGER*)&NativeError, Message, _Mycountof(Message), &MsgLen);

	// 연결 핸들일 시
	else if (Type == DBC)
		SQLGetDiagRec(SQL_HANDLE_DBC, &hDbc, 1, State, (SQLINTEGER*)&NativeError, Message, _Mycountof(Message), &MsgLen);
	
	
	// 명령 핸들일 시
	else if(Type == STMT)
		SQLGetDiagRec(SQL_HANDLE_STMT, &hStmt, 1, State, (SQLINTEGER*)&NativeError, Message, _Mycountof(Message), &MsgLen);

	// 그 어떤것도 아닐 시 
	else
	{
		_tprintf(L"ErrorFunc(). Not Find Type\n");
		return;
	}


	if(_tcscmp(Message, L"") != 0)
		_tprintf(L"%s\n", Message);


}
