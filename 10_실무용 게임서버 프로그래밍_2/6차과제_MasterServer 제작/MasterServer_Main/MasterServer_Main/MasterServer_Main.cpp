#include "pch.h"
#include "MasterServer/MasterServer.h"

#include <conio.h>

#include <mmsystem.h>
#pragma comment(lib, "winmm.lib")

using namespace Library_Jingyu;

int _tmain()
{
	timeBeginPeriod(1);

	system("mode con: cols=80 lines=45");

	CMatchServer_Lan MServer;

	if (MServer.ServerStart() == false)
		return 0;

	printf("ok\n");

	while (1)
	{
		Sleep(1000);

		// 화면 출력
	}

	timeEndPeriod(1);
	return 0;   
}
