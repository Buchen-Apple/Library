// ConsoleApplication1.cpp : 이 파일에는 'main' 함수가 포함됩니다. 거기서 프로그램 실행이 시작되고 종료됩니다.
//

#include "pch.h"
#include <iostream>
#include <process.h>
#include <windows.h>

#include "profiling/Profiling_Class.h"
#include "CrashDump/CrashDump.h"

using namespace Library_Jingyu;

#define SIZE 10000000
#define THREAD_COUNT	50

LONG AddValue;
LONG AddValue2;
SRWLOCK srwl;
HANDLE hEvent, hEvent2;

UINT InterlockThread(LPVOID lParam)
{
	WaitForSingleObject(hEvent, INFINITE);
	
	for (int i = 0; i < SIZE; ++i)
	{
		InterlockedIncrement(&AddValue);
	}	

	return 0;
}

UINT SRWLOCKThread(LPVOID lParam)
{
	WaitForSingleObject(hEvent2, INFINITE);
	
	for (int i = 0; i < SIZE; ++i)
	{
		AcquireSRWLockExclusive(&srwl);
		AddValue2++;
		ReleaseSRWLockExclusive(&srwl);
	}	

	return 0;
}

int main()
{
	Profiling pro;

	FREQUENCY_SET();

	hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	hEvent2 = CreateEvent(NULL, TRUE, FALSE, NULL);	

	// 인터락 스레드 생성
	HANDLE hThread[THREAD_COUNT];
	for (int i = 0; i < THREAD_COUNT; ++i)
		hThread[i] = (HANDLE)_beginthreadex(NULL, NULL, InterlockThread, NULL, NULL, NULL);

	BEGIN("Interlock");
	SetEvent(hEvent);
	WaitForMultipleObjects(THREAD_COUNT, hThread, TRUE, INFINITE);
	END("Interlock");

	// SRWlock 스레드 
	InitializeSRWLock(&srwl);
	HANDLE hThread2[THREAD_COUNT];
	for (int i = 0; i < THREAD_COUNT; ++i)
		hThread2[i] = (HANDLE)_beginthreadex(NULL, NULL, InterlockThread, NULL, NULL, NULL);
	
	BEGIN("srwlock");
	SetEvent(hEvent2);	
	WaitForMultipleObjects(THREAD_COUNT, hThread2, TRUE, INFINITE);	
	END("srwlock");

	// 파일로 저장
	PROFILING_FILE_SAVE();

	return 0;
}
