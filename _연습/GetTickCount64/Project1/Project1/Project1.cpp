// Project1.cpp: 콘솔 응용 프로그램의 진입점을 정의합니다.
//

#include "stdafx.h"
#include <Windows.h>


int _tmain()
{
	ULONGLONG StartTime = GetTickCount64();

	while (1)
	{
		ULONGLONG test = GetTickCount64();
		if (test - StartTime >= 1000)
		{
			printf("%lld\n", test - StartTime);

			StartTime = GetTickCount64();
		}
	}
    return 0;
}

