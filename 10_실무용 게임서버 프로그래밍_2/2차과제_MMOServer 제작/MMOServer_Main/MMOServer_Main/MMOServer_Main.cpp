// MMOServer_Main.cpp : 이 파일에는 'main' 함수가 포함됩니다. 거기서 프로그램 실행이 시작되고 종료됩니다.
//

#include "pch.h"

#include "CGameServer.h"

using namespace Library_Jingyu;

int _tmain()
{
	CGameServer Test;

	Test.GameServerStart();

	while (1)
	{
		Sleep(1000);
		printf("aaaa\n");
	}

	return 0;
}

