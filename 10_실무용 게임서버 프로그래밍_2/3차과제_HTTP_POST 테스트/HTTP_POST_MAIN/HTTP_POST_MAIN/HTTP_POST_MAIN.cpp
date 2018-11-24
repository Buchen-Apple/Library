#include "pch.h"
#include <iostream>
#include "Http_Exchange/HTTP_Exchange.h"
#include "CrashDump/CrashDump.h"

#include "rapidjson\document.h"
#include "rapidjson\writer.h"
#include "rapidjson\stringbuffer.h"

#include <process.h>

using namespace Library_Jingyu;
using namespace rapidjson;

#define _Mycountof(Array)	sizeof(Array) / sizeof(TCHAR)


#define SERVER_IP	(TCHAR*)_T("10.0.0.2")
#define SERVER_PORT	80
#define FLAG	0	// 0이면 새로 안만든다.
#define SLEEP(x)	//Sleep(x)


UINT WINAPI DBTestThread(LPVOID lParam);

CCrashDump* g_Dump = CCrashDump::GetInstance();

int g_SleepValue;

LONG g_TPS;

HTTP_Exchange HTTP_Post(SERVER_IP, SERVER_PORT);

namespace Library_Jingyu
{
	LONG g_lNET_BUFF_SIZE = 512;
}

int _tmain()
{	
	int ThreadNum;
	printf("Thread : ");
	scanf_s("%d", &ThreadNum);

	printf("Sleep : ");
	scanf_s("%d", &g_SleepValue);

	if (FLAG == 1)
	{
		int TempEmail = 123;
		TCHAR tcToken[65] = L"sakljflksajfklsafnkscksjlfasf12345adflsadlfjkefafkj%kldjsklf121";

		// 계정을 스레드 수 만큼 생성 (Create.php)
		for (int i = 0; i < ThreadNum; ++i)
		{
			TCHAR RequestBody[2000];
			TCHAR Body[1000];

			// 1. 계정 생성
			ZeroMemory(RequestBody, sizeof(RequestBody));

			swprintf_s(Body, _Mycountof(Body), L"{\"email\" : \"%d@naver.com\"}", TempEmail + i);
			if (HTTP_Post.HTTP_ReqANDRes((TCHAR*)_T("Contents/Create.php"), Body, RequestBody) == false)
				g_Dump->Crash();

			// Json데이터 파싱하기 (이미 UTF-16을 넣는중)
			GenericDocument<UTF16<>> Doc;
			Doc.Parse(RequestBody);

			int Result = 0;
			Result = Doc[_T("result")].GetInt();

			// 결과가 1이 아니면 Crash()
			if (Result != 1)
				g_Dump->Crash();

			// 2. 세션 키 Update
			ZeroMemory(RequestBody, sizeof(RequestBody));
			ZeroMemory(Body, sizeof(Body));

			swprintf_s(Body, _Mycountof(Body), L"{\"email\" : \"%d@naver.com\", \"sessionKey\" : \"%s\"}", TempEmail + i, tcToken);
			if (HTTP_Post.HTTP_ReqANDRes((TCHAR*)_T("Contents/Update_account.php"), Body, RequestBody) == false)
				g_Dump->Crash();

			// Json데이터 파싱하기 (이미 UTF-16을 넣는중)
			Doc.Parse(RequestBody);

			Result;
			Result = Doc[_T("result")].GetInt();

			// 결과가 1이 아니면 Crash()
			if (Result != 1)
				g_Dump->Crash();


			printf("Create OK. %d\n", i + 1);
		}
	}	

	// 스레드 n개 생성
	HANDLE *hThread = new HANDLE[ThreadNum];
	for (int i = 0; i < ThreadNum; ++i)
	{
		hThread[i] = (HANDLE)_beginthreadex(0, 0, DBTestThread, (LPVOID)(i+1), 0, 0);
	}

	while (1)
	{
		Sleep(1000);
		LONG Temp = InterlockedExchange(&g_TPS, 0);
		printf("TPS : %d\n", Temp);
	}

	return 0;
}


UINT WINAPI DBTestThread(LPVOID lParam)
{
	// 쿼리를 계속 날릴 accountno
	const UINT64 constAccountNo = (UINT64)lParam;

	// 쿼리를 날릴 http_post.
	//HTTP_Exchange HTTP_Post(SERVER_IP, SERVER_PORT);

	// 킬 수 카운팅 변수
	int KillCount = 0;

	TCHAR RequestBody[2000];
	TCHAR Body[1000];
	while (1)
	{	
		// 1. Update_account (닉네임 변경)
		ZeroMemory(RequestBody, sizeof(RequestBody));
		ZeroMemory(Body, sizeof(Body));

		swprintf_s(Body, _Mycountof(Body), L"{\"accountno\" : %lld, \"nick\" : \"%s\"}", constAccountNo, _T("ChangeNick"));
		if (HTTP_Post.HTTP_ReqANDRes((TCHAR*)_T("Contents/Update_account.php"), Body, RequestBody) == false)
			g_Dump->Crash();

		// Json데이터 파싱하기 (이미 UTF-16을 넣는중)
		GenericDocument<UTF16<>> Doc;
		Doc.Parse(RequestBody);

		int Result;
		Result = Doc[_T("result")].GetInt();

		// 결과가 1이 아니면 Crash()
		if (Result != 1)
			g_Dump->Crash();	

		//_tprintf(_T("Update_Account(1) : %s\n"), RequestBody);
		InterlockedIncrement(&g_TPS);

		SLEEP(g_SleepValue);



		// 2. Select_account (변경한 닉네임 잘 반영되었는가 체크)	
		ZeroMemory(RequestBody, sizeof(RequestBody));
		ZeroMemory(Body, sizeof(Body));

		swprintf_s(Body, _Mycountof(Body), L"{\"accountno\" : %lld}", constAccountNo);
		if (HTTP_Post.HTTP_ReqANDRes((TCHAR*)_T("Contents/Select_account.php"), Body, RequestBody) == false)
			g_Dump->Crash();

		// Json데이터 파싱하기 (이미 UTF-16을 넣는중)
		Doc.Parse(RequestBody);

		const TCHAR* NickName;
		NickName = Doc[_T("nick")].GetString();

		// 결과가 "ChangeNick"이 아니면 Crahs()
		if (_tcscmp(NickName, _T("ChangeNick")) != 0)
			g_Dump->Crash();

		//_tprintf(_T("Select_account(2) : %s\n"), RequestBody);
		InterlockedIncrement(&g_TPS);

		SLEEP(g_SleepValue);		



		
		// 3. Update_Content (킬 수 1 증가)
		ZeroMemory(RequestBody, sizeof(RequestBody));
		ZeroMemory(Body, sizeof(Body));

		KillCount++;
		swprintf_s(Body, _Mycountof(Body), L"{\"accountno\" : %lld, \"kill\" : \"%d\"}", constAccountNo, KillCount);
		if (HTTP_Post.HTTP_ReqANDRes((TCHAR*)_T("Contents/Update_contents.php"), Body, RequestBody) == false)
		{
			printf("fail...\n");
			g_Dump->Crash();
		}

		// Json데이터 파싱하기 (이미 UTF-16을 넣는중)
		Doc.Parse(RequestBody);

		Result = Doc[_T("result")].GetInt();

		// 결과가 1이 아니면 Crash()
		if (Result != 1)
			g_Dump->Crash();

		//_tprintf(_T("Update_contents(5) : %s\n"), RequestBody);
		InterlockedIncrement(&g_TPS);

		SLEEP(g_SleepValue);



		// 4. Select_Content (킬수 잘 반영되었나 확인)
		ZeroMemory(RequestBody, sizeof(RequestBody));
		ZeroMemory(Body, sizeof(Body));

		swprintf_s(Body, _Mycountof(Body), L"{\"accountno\" : %lld}", constAccountNo);
		if (HTTP_Post.HTTP_ReqANDRes((TCHAR*)_T("Contents/Select_contents.php"), Body, RequestBody) == false)
		{
			printf("fail...\n");
			g_Dump->Crash();
		}

		// Json데이터 파싱하기 (이미 UTF-16을 넣는중)
		Doc.Parse(RequestBody);

		int TempKill;
		TempKill = Doc[_T("kill")].GetInt();

		// 결과가 KillCount변수와 다르다면 Crash()
		if (TempKill != KillCount)
			g_Dump->Crash();

		//_tprintf(_T("Select_contents(6) : %s\n"), RequestBody);
		InterlockedIncrement(&g_TPS);

		SLEEP(g_SleepValue);
		Sleep(g_SleepValue);
	}

	return 0;
}