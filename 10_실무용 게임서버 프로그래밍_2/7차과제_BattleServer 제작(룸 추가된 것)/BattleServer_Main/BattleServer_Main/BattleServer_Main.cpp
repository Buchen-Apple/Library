// BattleServer_Main.cpp : 이 파일에는 'main' 함수가 포함됩니다. 거기서 프로그램 실행이 시작되고 종료됩니다.
//

#include "pch.h"
#include "BattleServer_Room_Version.h"

#include <mmsystem.h>
#pragma comment(lib, "winmm.lib")

using namespace Library_Jingyu;

int _tmain()
{
	timeBeginPeriod(1);

	CBattleServer_Room Test;

	timeEndPeriod(1);

	return 0;
}
