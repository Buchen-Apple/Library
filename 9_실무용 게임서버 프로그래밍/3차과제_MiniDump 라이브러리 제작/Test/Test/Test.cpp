// Test.cpp: 콘솔 응용 프로그램의 진입점을 정의합니다.
//

#include "stdafx.h"
#include "CrashDump.h"
#include <process.h>


using namespace Library_Jingyu;

// 싱글톤으로 하나 얻음
CCrashDump* Test = CCrashDump::GetInstance();

UINT	WINAPI	TestFunc(LPVOID lParam);

class CDerived;

class CBase
{
public:
	CBase(CDerived *derived) : m_pDerived(derived) {};
	~CBase();
	virtual void function(void) = 0;

	CDerived * m_pDerived;
};

class CDerived : public CBase
{
public:
	CDerived() : CBase(this) {};   // C4355
	virtual void function(void) {};
};

CBase::~CBase()
{
	m_pDerived->function();
}





int _tmain()
{
	// CRT myInvalidParameterHandler 예외 테스트
	memcpy_s(nullptr, 0, nullptr, 100);

	printf("aaaa\n");
	// -----------------
	// Signal 신호 테스트
	// -----------------
	// SIGABRT(비정상 종료)
	// abort();



	// SIGINT(ctrl + c)
	//int abc = 0;
	//scanf_s("%d", &abc);



	// SIGILL(잘못된 명령)



	// SIGFPE(부동 소수점 오류)	



	// SIGSEGV(잘못된 저장소 엑세스)
	//int* abc = nullptr;
	//*abc = 0;



	// SIGTERM(종료 요청)
	//HANDLE Thread = (HANDLE)_beginthreadex(NULL, 0, TestFunc, 0, 0, NULL);	
	//TerminateThread(Thread, 0);
	//CloseHandle(Thread);
	//exit(EXIT_SUCCESS);
	//ExitProcess(0);
	//TerminateProcess(GetCurrentProcess(), 0);
	


	// -----------------
	// PureCall 확인
	// -----------------
	//CDerived myDerived;

    return 0;
}

UINT	WINAPI	TestFunc(LPVOID lParam)
{
	return 0;
}

