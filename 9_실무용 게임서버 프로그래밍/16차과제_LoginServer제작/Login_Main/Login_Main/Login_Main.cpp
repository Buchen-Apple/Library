// Login_Main.cpp : 이 파일에는 'main' 함수가 포함됩니다. 거기서 프로그램 실행이 시작되고 종료됩니다.
//

#include "pch.h"
#include "LoginServer.h"
#include <Windows.h>


// 출력용 코드
LONG g_lAllocNodeCount;

LONG g_lUpdateStructCount;
LONG g_lUpdateStruct_PlayerCount;

extern ULONGLONG g_ullAcceptTotal;
extern LONG	  g_lAcceptTPS;
extern LONG	g_lSendPostTPS;
LONG	  g_lUpdateTPS;


using namespace Library_Jingyu;

int _tmain()
{
	CLogin_NetServer Test;
	

	return 0;
}
