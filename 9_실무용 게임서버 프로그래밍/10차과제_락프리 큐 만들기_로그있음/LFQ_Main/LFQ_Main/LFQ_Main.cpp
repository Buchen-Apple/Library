// LFQ_Main.cpp: 콘솔 응용 프로그램의 진입점을 정의합니다.
//

#include "pch.h"
#include "LockFree_Queue.h"
#include "CrashDump\CrashDump.h"

#include <process.h>
#include <mmsystem.h>
#pragma comment(lib, "winmm.lib")

using namespace Library_Jingyu;

#define ALLOC_COUNT	4

#define THREAD_ALLOC_COUNT	1
#define TOTAL_THREAD		4

class CTest
{
public:
	ULONGLONG Addr = 0x0000000055555555;
	LONG Count = 0;
};

CCrashDump* g_CDump = CCrashDump::GetInstance();
CLF_Queue<CTest> LFQ;

LONG g_UseCount;

ULONGLONG ThreadCount[TOTAL_THREAD] = { 0, };

// TestThread
UINT	WINAPI	TestThread(LPVOID lParam);

int _tmain()
{
	system("mode con: cols=120 lines=20");

	timeBeginPeriod(1);

	// 1. 시작 전에 ALLOC_COUNT 개를 만들어서 Enqueue한다.
	for (int i = 0; i < ALLOC_COUNT; ++i)
	{
		CTest Test;
		LFQ.Enqueue(Test, TOTAL_THREAD);
	}

	// 2. 스레드 생성
	// TOTAL_THREAD개의 스레드. 스레드당 THREAD_ALLOC_COUNT개의 블럭 사용
	HANDLE hThread[10];
	for (int i = 0; i < TOTAL_THREAD; ++i)
	{
		hThread[i] = (HANDLE)_beginthreadex(NULL, 0, TestThread, (VOID*)i, 0, NULL);
	}

	// 3. 화면에 출력
	// [유저가 사용중인 블록 수 / 총 블록 수] 
	while (1)
	{
		Sleep(1000);
		printf("OutStackCount : %d, InStackCount : %d\n"
				"ThreadLoop : %lld / %lld / %lld / %lld\n\n", g_UseCount, LFQ.GetInNode(), ThreadCount[0], ThreadCount[1], ThreadCount[2], ThreadCount[3]);
	}


	timeEndPeriod(1);

	return 0;
}

// TestThread
UINT	WINAPI	TestThread(LPVOID lParam)
{
	CTest cTestArray[THREAD_ALLOC_COUNT];
	ULONGLONG TempAddr[THREAD_ALLOC_COUNT];
	LONG TempCount[THREAD_ALLOC_COUNT];

	int ID = (int)lParam;

	while (1)
	{
		cTestArray[0].Addr = NULL;
		cTestArray[0].Count = 0;

		// 1. Dequeue
		for (int i = 0; i < THREAD_ALLOC_COUNT; ++i)
		{
			int retval = LFQ.Dequeue(cTestArray[i], ID);
			if (retval == -1)
				g_CDump->Crash();

			InterlockedIncrement(&g_UseCount);
		}


		// 2. Addr 비교 (0x0000000055555555가 맞는지)
		for (int i = 0; i < THREAD_ALLOC_COUNT; ++i)
		{
			if (cTestArray[i].Addr != 0x0000000055555555)
				g_CDump->Crash();

			// 3. 인터락으로 Addr, Count ++
			TempAddr[i] = InterlockedIncrement(&cTestArray[i].Addr);
			TempCount[i] = InterlockedIncrement(&cTestArray[i].Count);
		}

		// 3. 잠시 대기 (Sleep)
		//Sleep(0);

		// 4. Addr과 Count 다시 비교
		for (int i = 0; i < THREAD_ALLOC_COUNT; ++i)
		{
			if (cTestArray[i].Addr != TempAddr[i])
				g_CDump->Crash();

			else if (cTestArray[i].Count != TempCount[i])
				g_CDump->Crash();

			// 6. 인터락으로 Addr, Count --
			TempAddr[i] = InterlockedDecrement(&cTestArray[i].Addr);
			TempCount[i] = InterlockedDecrement(&cTestArray[i].Count);
		}


		// 5. 잠시 대기 (Sleep)
		//Sleep(0);

		// 6. Addr과 Count가 0x0000000055555555, 0이 맞는지 확인
		for (int i = 0; i < THREAD_ALLOC_COUNT; ++i)
		{
			if (cTestArray[i].Addr != TempAddr[i])
				g_CDump->Crash();

			else if (cTestArray[i].Count != TempCount[i])
				g_CDump->Crash();
		}


		// 7. Enqueue
		for (int i = 0; i < THREAD_ALLOC_COUNT; ++i)
		{
			LFQ.Enqueue(cTestArray[i], ID);
			InterlockedDecrement(&g_UseCount);
		}

		ThreadCount[ID]++;
	}
	return 0;
}

