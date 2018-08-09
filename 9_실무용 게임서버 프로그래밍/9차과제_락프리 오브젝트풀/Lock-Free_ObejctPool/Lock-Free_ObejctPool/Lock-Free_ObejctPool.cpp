// Lock-Free_ObejctPool.cpp: 콘솔 응용 프로그램의 진입점을 정의합니다.
//

#include "stdafx.h"
#include <stdio.h>
#include "ObjectPool\Object_Pool_LockFreeVersion.h"
#include "CrashDump\CrashDump.h"
#include <Windows.h>
#include <process.h>

#include <mmsystem.h>
#pragma comment(lib,"winmm.lib")

using namespace Library_Jingyu;

#define ALLOC_COUNT	100000

class CTest
{
public:
	ULONGLONG Addr = 0x0000000055555555;
	LONG Count = 0;
};

CMemoryPool<CTest> g_MPool(0, false);
CCrashDump* g_CDump = CCrashDump::GetInstance();




///////  메모리풀 테스트 툴  //////////////////////////////////////////////////////////////////////////////////////////

// 여러개의 스레드에서 일정수량의 Alloc 과 Free 를 반복적으로 함
// 모든 데이터는 0x0000000055555555 으로 초기화 되어 있음.

// 0. Alloc (스레드당 10000 개 x 10 개 스레드 총 10만개)
// 1. 0x0000000055555555 이 맞는지 확인.
// 2. Interlocked + 1 (Data + 1 / Count + 1)
// 3. 약간대기
// 4. 여전히 0x0000000055555556 이 맞는지 (Count == 1) 확인.
// 5. Interlocked - 1 (Data - 1 / Count - 1)
// 6. 약간대기
// 7. 0x0000000055555555 이 맞는지 (Count == 0) 확인.
// 8. Free
// 반복.

// 테스트 목적
//
// - 할당된 메모리를 또 할당 하는가 ?
// - 잘못된 메모리를 할당 하는가 ?

// TestThread
UINT	WINAPI	TestThread(LPVOID lParam);

int _tmain(void)
{
	timeBeginPeriod(1);

	// 1. 시작 전에 내가 Alloc으로 100000(십만)개의 메모리풀 만든 후 프리한다.
	// Alloc()
	//CTest* cAllocArray[ALLOC_COUNT];

	//for (int i = 0; i < ALLOC_COUNT; ++i)
	//{
	//	cAllocArray[i] = g_MPool.Alloc();
	//}		

	//// Free()
	//for (int i = 0; i < ALLOC_COUNT; ++i)
	//{
	//	g_MPool.Free(cAllocArray[i]);
	//}	

	// 2. 스레드 생성
	// 10개의 스레드 x 스레드당 10000개의 블럭 사용
	HANDLE hThread[10];
	for (int i = 0; i < 10; ++i)
	{
		hThread[i] = (HANDLE)_beginthreadex(NULL, 0, TestThread, 0, 0, NULL);
	}

	// 3. 메인스레드는 1초에 1회, 정보 출력
	// [유저가 사용중인 블록 수 / 총 블록 수] 
	while (1)
	{
		Sleep(1000);

		printf("User UseCount : %d		AllocCount : %d\n", g_MPool.GetUseCount(), g_MPool.GetAllocCount());
	}

	timeEndPeriod(1);
	return 0;
}


// TestThread
UINT	WINAPI	TestThread(LPVOID lParam)
{
	while (1)
	{
		CTest* cTestArray[10000];
		ULONGLONG TempAddr[10000] = { 0 ,};
		LONG TempCount[10000] = { 0 , };

		// 1. Alloc (1만개)
		for (int i = 0; i < 10000; ++i)
		{
			cTestArray[i] = g_MPool.Alloc();
			if(cTestArray[i] == nullptr)
				g_CDump->Crash();
		}
	

		/*
		// 2. Addr 비교 (0x0000000055555555가 맞는지)
		for (int i = 0; i < 10000; ++i)
		{			
			if (cTestArray[i]->Addr != 0x0000000055555555)
				g_CDump->Crash();

			// 3. 인터락으로 Addr, Count ++
			TempAddr[i] = InterlockedIncrement(&cTestArray[i]->Addr);
			TempCount[i] = InterlockedIncrement(&cTestArray[i]->Count);
		}

		// 4. 잠시 대기 (Sleep)
		//Sleep(0);

		// 5. Addr과 Count 다시 비교
		for (int i = 0; i < 10000; ++i)
		{
			if(cTestArray[i]->Addr != TempAddr[i])
				g_CDump->Crash();

			else if(cTestArray[i]->Count != TempCount[i])
				g_CDump->Crash();

			// 6. 인터락으로 Addr, Count --
			TempAddr[i] = InterlockedDecrement(&cTestArray[i]->Addr);
			TempCount[i] = InterlockedDecrement(&cTestArray[i]->Count);
		}
		

		// 7. 잠시 대기 (Sleep)
		//Sleep(0);

		// 8. Addr과 Count가 0x0000000055555555, 0이 맞는지 확인
		for (int i = 0; i < 10000; ++i)
		{
			if (cTestArray[i]->Addr != TempAddr[i])
				g_CDump->Crash();

			else if (cTestArray[i]->Count != TempCount[i])
				g_CDump->Crash();

			
		}
		*/

		// 9. Free
		for (int i = 0; i < 10000; ++i)
		{			
			if (g_MPool.Free(cTestArray[i]) == false)
				g_CDump->Crash();
		}
	}
	return 0;
}