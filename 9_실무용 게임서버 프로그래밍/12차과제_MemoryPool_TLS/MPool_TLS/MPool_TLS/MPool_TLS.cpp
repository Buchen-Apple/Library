// MPool_TLS.cpp : 이 파일에는 'main' 함수가 포함됩니다. 거기서 프로그램 실행이 시작되고 종료됩니다.
//

#include "pch.h"
#include "ObjectPool\Object_Pool_LockFreeVersion.h"
#include "profiling\Profiling_Class.h"

#include <process.h>
#include <mmsystem.h>
#pragma comment(lib, "winmm.lib")


using namespace Library_Jingyu;

class CTest
{
public:
	ULONGLONG Addr = 0x0000000055555555;
	LONG Count = 0;
};


CMemoryPoolTLS<CTest>* g_MPool = new CMemoryPoolTLS<CTest>(5500, false);

#define COUNT	1000000


int _tmain()
{
	FREQUENCY_SET(); // 주파수 구하기	
	timeBeginPeriod(1);	
	
	int iCount = 0;

	BEGIN("new");
	while (iCount <= COUNT)
	{
		CTest* Test = g_MPool->Alloc();
		iCount++;
	}
	END("new");

	/*BEGIN("new");
	while (iCount <= COUNT)
	{
		CTest* Test = new CTest;
		iCount++;
	}
	END("new");*/



	
	PROFILING_FILE_SAVE(); // 프로파일링 파일 저장
	
	printf("End\n");
	timeEndPeriod(1);

	return 0;
}
