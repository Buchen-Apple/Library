// Project1.cpp: 콘솔 응용 프로그램의 진입점을 정의합니다.
//

#include "stdafx.h"
#include <Windows.h>
#include <process.h>


unsigned int WINAPI	OutputThreadFunc(LPVOID lpParam);
unsigned int WINAPI	CountThreadFunc(LPVOID lpParam);

typedef struct _SynchString
{
	TCHAR string[100];
	HANDLE	hEvent;
	HANDLE	hMutex;
} SyncString;

SyncString gSyncString;

int _tmain()
{
	HANDLE	hThreads[2];
	UINT	dwThreadIDs[2];

	// 2번인자가 TRUE면 수동 리셋 모드, FALSE면 자동 리셋 모드 (책)
	// 해보니까 2번인자가 TRUE면 자동 리셋 모드, FALSE면 수동 리셋 모드
	gSyncString.hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

	gSyncString.hMutex = CreateMutex(NULL, FALSE, NULL);

	if (gSyncString.hEvent == NULL || gSyncString.hMutex == NULL)
	{
		_fputts(L"Kernel Object Error\n", stdout);
		return -1;
	}

	hThreads[0] = (HANDLE)_beginthreadex(NULL, 0, OutputThreadFunc, NULL, 0, &dwThreadIDs[0]);
	hThreads[1] = (HANDLE)_beginthreadex(NULL, 0, CountThreadFunc, NULL, 0, &dwThreadIDs[1]);

	if (hThreads[0] == 0 || hThreads[1] == 0)
	{
		_fputts(L"Thread Create Error\n", stdout);
		return -1;
	}

	_fputts(L"Insert String : ", stdout);

	_fgetts(gSyncString.string, 30, stdin);

	SetEvent(gSyncString.hEvent);

	WaitForMultipleObjects(2, hThreads, TRUE, INFINITE);

	CloseHandle(gSyncString.hEvent);
	CloseHandle(gSyncString.hMutex);
	CloseHandle(hThreads[0]);
	CloseHandle(hThreads[1]);

    return 0;
}

unsigned int WINAPI	OutputThreadFunc(LPVOID lpParam)
{
	WaitForSingleObject(gSyncString.hEvent, INFINITE);	
	//WaitForSingleObject(gSyncString.hMutex, INFINITE);

	//_fputts(L"OutputThreadFunc\n", stdout);
	_fputts(L"Output String : ", stdout);
	_fputts(gSyncString.string, stdout);
	
	//ReleaseMutex(gSyncString.hMutex);
	SetEvent(gSyncString.hEvent);
	return 0;
}

unsigned int WINAPI	CountThreadFunc(LPVOID lpParam)
{
	WaitForSingleObject(gSyncString.hEvent, INFINITE);
	//WaitForSingleObject(gSyncString.hMutex, INFINITE);
	
	//_fputts(L"CountThreadFunc\n", stdout);
	_tprintf(L"OutPut String Length : %d \n", _tcslen(gSyncString.string) - 1);	

	//ReleaseMutex(gSyncString.hMutex);
	SetEvent(gSyncString.hEvent);
	return 0;
}

