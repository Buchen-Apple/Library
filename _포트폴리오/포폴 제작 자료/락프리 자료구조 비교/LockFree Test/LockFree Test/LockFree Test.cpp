// LockFree Test.cpp : 이 파일에는 'main' 함수가 포함됩니다. 거기서 프로그램 실행이 시작되고 종료됩니다.
//

#include "pch.h"
#include <iostream>
#include "LockFree_Queue/LockFree_Queue.h"
#include "profiling/Profiling_Class.h"
#include "NormalTemplate_Queue/Normal_Queue_Template.h"

#include <process.h>

#define LOOP_COUNT	1000000
#define FLAG 1	// 0이면 크리티컬 섹션, 1이면 락프리, 2면 SRW


using namespace Library_Jingyu;

class CTest
{
	int a = 10;
};

Profiling profile;

UINT WINAPI EnqThread(LPVOID lParam);
UINT WINAPI DeqThread(LPVOID lParam);

HANDLE Event, DeqEvent;

CRITICAL_SECTION cs;
SRWLOCK srwl;

CNormalQueue<CTest> g_Normal;
CLF_Queue<CTest> g_LockFreeQ(0);

#define _Mycountof(Array)	sizeof(Array) / sizeof(TCHAR)

int main()
{

	InitializeCriticalSection(&cs);
	InitializeSRWLock(&srwl);

	FREQUENCY_SET();





	Event = CreateEvent(NULL, TRUE, FALSE, NULL);

	HANDLE hThread[4];

	// Enq만 하는 스레드 4개
	// 스레드 1개당 100만개씩 Enq
	hThread[0] = (HANDLE)_beginthreadex(0, 0, EnqThread, (void*)1, 0, 0);
	hThread[1] = (HANDLE)_beginthreadex(0, 0, EnqThread, (void*)2, 0, 0);
	hThread[2] = (HANDLE)_beginthreadex(0, 0, EnqThread, (void*)3, 0, 0);
	hThread[3] = (HANDLE)_beginthreadex(0, 0, EnqThread, (void*)4, 0, 0);

	printf("Enq Start!!!!\n");
	BEGIN("Enq");

	SetEvent(Event);

	WaitForMultipleObjects(4, hThread, TRUE, INFINITE);

	END("Enq");
	printf("Enq End!\n");





	DeqEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	
	HANDLE hThread_2[4];

	// Deq만 하는 스레드 4개
	// 스레드 1개당 100만개씩 Deq
	hThread_2[0] = (HANDLE)_beginthreadex(0, 0, DeqThread, (void*)1, 0, 0);
	hThread_2[1] = (HANDLE)_beginthreadex(0, 0, DeqThread, (void*)2, 0, 0);
	hThread_2[2] = (HANDLE)_beginthreadex(0, 0, DeqThread, (void*)3, 0, 0);
	hThread_2[3] = (HANDLE)_beginthreadex(0, 0, DeqThread, (void*)4, 0, 0);

	printf("Deq Start!!!!\n");
	BEGIN("Deq");

	SetEvent(Event);

	WaitForMultipleObjects(4, hThread_2, TRUE, INFINITE);

	END("Deq");
	printf("Deq End!\n");







	printf("File Save Start!!!\n");
	PROFILING_FILE_SAVE();
	printf("File Save OK\n");



	DeleteCriticalSection(&cs);
	return 0;
}


UINT WINAPI EnqThread(LPVOID lParam)
{
	WaitForSingleObject(Event, INFINITE);
	
	int i = 0;
	while (i < LOOP_COUNT)
	{
		CTest Test;

#if(FLAG == 0)

		EnterCriticalSection(&cs);		
		g_Normal.Enqueue(Test);
		LeaveCriticalSection(&cs);

#elif(FLAG == 1)
		g_LockFreeQ.Enqueue(Test);

#elif(FLAG == 2)
		AcquireSRWLockExclusive(&srwl);
		g_Normal.Enqueue(Test);
		ReleaseSRWLockExclusive(&srwl);

#endif

		++i;
	}

	return 0;
}

UINT WINAPI DeqThread(LPVOID lParam)
{
	WaitForSingleObject(Event, INFINITE);

	int i = 0;
	while (i < LOOP_COUNT)
	{
		CTest Test;

#if(FLAG == 0)
		EnterCriticalSection(&cs);		
		int Ret = g_Normal.Dequeue(Test);
		LeaveCriticalSection(&cs);

#elif(FLAG == 1)
		int Ret = g_LockFreeQ.Dequeue(Test);

#elif(FLAG == 2)
		AcquireSRWLockExclusive(&srwl);
		int Ret = g_Normal.Dequeue(Test);
		ReleaseSRWLockExclusive(&srwl);

#endif

		if (Ret == 0)
			++i;
	}

	return 0;
}
