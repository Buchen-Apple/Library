// Project1.cpp: 콘솔 응용 프로그램의 진입점을 정의합니다.
//

#include "stdafx.h"
#include <Windows.h>

int _tmain()
{
	HANDLE hTimer = NULL;
	LARGE_INTEGER	liDueTime;

	liDueTime.QuadPart = -100000000;

	// FALSE면 자동으로 Non-Signaled로 변경.
	// TRUE면 수동으로 Non-Signaled로 변경해야 함
	hTimer = CreateWaitableTimer(NULL, FALSE, L"WaitableTimer");

	if (hTimer == false)
	{
		_tprintf(L"CreateWaitableTimer Error : %d\n", GetLastError());
		return -1;
	}

	_fputts(L"Waiting for 10 Second...\n", stdout);

	SetWaitableTimer(hTimer, &liDueTime, 5000, NULL, NULL, FALSE);

	int i = 0;
	while (1)
	{
		WaitForSingleObject(hTimer, INFINITE);
		i++;
		_tprintf(L"Timer was signaled! %d \n", i);
	}

	CloseHandle(hTimer);
    return 0;
}

