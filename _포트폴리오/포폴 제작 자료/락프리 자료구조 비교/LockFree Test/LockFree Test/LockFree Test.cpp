// LockFree Test.cpp : 이 파일에는 'main' 함수가 포함됩니다. 거기서 프로그램 실행이 시작되고 종료됩니다.
//

#include "pch.h"
#include <iostream>
#include "LockFree_Queue/LockFree_Queue.h"
#include "profiling/Profiling_Class.h"
#include "NormalTemplate_Queue/Normal_Queue_Template.h"

#include <process.h>

#define LOOP_COUNT	100000
#define FLAG 1	// 0이면 크리티컬 섹션, 1이면 락프리, 2면 SRW
#define THREAD_COUNT	10


using namespace Library_Jingyu;
using namespace std;

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

	HANDLE hThread[THREAD_COUNT];

	// Enq만 하는 스레드
	// 스레드 1개당 100만개씩 Enq
	for (int i = 0; i < THREAD_COUNT; ++i)
	{
		hThread[i] = (HANDLE)_beginthreadex(0, 0, EnqThread, 0, 0, 0);
	}	

	printf("Enq Start!!!!\n");
	BEGIN("Enq");

	SetEvent(Event);

	WaitForMultipleObjects(THREAD_COUNT, hThread, TRUE, INFINITE);

	END("Enq");
	printf("Enq End!\n");





	DeqEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	
	HANDLE hThread_2[THREAD_COUNT];

	// Deq만 하는 스레드
	// 스레드 1개당 100만개씩 Deq
	for (int i = 0; i < THREAD_COUNT; ++i)
	{
		hThread_2[i] = (HANDLE)_beginthreadex(0, 0, DeqThread, 0, 0, 0);
	}

	printf("Deq Start!!!!\n");
	BEGIN("Deq");

	SetEvent(Event);

	WaitForMultipleObjects(THREAD_COUNT, hThread_2, TRUE, INFINITE);

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
	CTest Test;

	while (i < LOOP_COUNT)
	{		

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
	CTest Test;

	while (i < LOOP_COUNT)
	{
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
