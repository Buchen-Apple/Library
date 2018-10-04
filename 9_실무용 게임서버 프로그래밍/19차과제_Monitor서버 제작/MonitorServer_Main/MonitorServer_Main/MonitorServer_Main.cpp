#include "pch.h"
#include "MonitorServer\MonitorServer.h"

#include <mmsystem.h>
#pragma comment(lib, "winmm.lib")

using namespace Library_Jingyu;

int _tmain()
{
	timeBeginPeriod(1);

	CNet_Monitor_Server Test;

	if (Test.ServerStart() == false)
	{
		printf("fail..\n");
		return 0;
	}

	Sleep(INFINITE);


	timeEndPeriod(1);

	return 0;
}

