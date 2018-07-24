#include "stdafx.h"
#include "CrashDump.h"

#pragma comment(lib, "DbgHelp.lib")
#include <DbgHelp.h>
#include <crtdbg.h>
#include <csignal>

namespace Library_Jingyu
{
	long CCrashDump::_DumpCount;

	// �̱����� ���� �����ڴ� Private
	// ������
	CCrashDump::CCrashDump(void)
	{
		_DumpCount = 0;

		// --------------
		// CRT ���� �ڵ鸵
		// --------------
		_invalid_parameter_handler oldHandler, newHandler;
		newHandler = myInvalidParameterHandler;

		// CRT �Լ��� nullptr ���� �־��� �� 
		oldHandler = _set_invalid_parameter_handler(newHandler);

		// CRT ���� �޽��� ��� �ߴ�. �ٷ� ������ �����.
		_CrtSetReportMode(_CRT_WARN, 0);	// ��� �޽��� (Warning) ��� �ߴ�.
		_CrtSetReportMode(_CRT_ASSERT, 0);	// ��� ���� ��� �ߴ� (��ȼ� ���� : ����� false�� ��� ���� �߻���Ŵ)
		_CrtSetReportMode(_CRT_ERROR, 0);	// Error �޽��� ��� �ߴ�.

											// CRT ������ �߻��ϸ�, ��ŷ�� �Լ� ���(�Լ�������)
		_CrtSetReportHook(_custom_Report_hook);


		// --------------
		// pure virtual function called ���� �ڵ鷯�� ����� ���� �Լ��� ��ȸ
		// �ش� ������, ������/�Ҹ��ڿ��� virtual �Լ��� ȣ������ �� ���� ������ �߻��ϴ� ����.
		// --------------
		_set_purecall_handler(myPurecallHandler);

		// abort() �Լ��� ȣ������ �� ����â�� ���� �ʵ��� �Ѵ�.
		// abort() : �ش� �Լ��� ȣ��Ǹ�, ������ ���Ḧ �߱��ϰ�, ȣ��Ʈ ȯ�濡 ��� ����. exit()�� ���������� ���α׷� ���� ��, ���� �����ϰ� ���� ������ ����.
		// �׸���, Signal ��ȣ�� �߻���Ų��.
		_set_abort_behavior(0, _WRITE_ABORT_MSG | _CALL_REPORTFAULT);

		// ������ ��ȣ(1�� ����)�� �� ��, ������ �Լ�(2�� ����)�� ȣ��ȴ�.
		signal(SIGABRT, signalHandler);		// ������ ����
		signal(SIGINT, signalHandler);		// Ctrl+C ��ȣ
		signal(SIGILL, signalHandler);		// �߸��� ���
		signal(SIGFPE, signalHandler);		// �ε� �Ҽ��� ����
		signal(SIGSEGV, signalHandler);		// �߸��� ����ҿ� ������
		signal(SIGTERM, signalHandler);		// ���� ��û.


		SetHandlerDump();
	}


	// �̱��� ���
	CCrashDump* CCrashDump::GetInstance()
	{
		static CCrashDump cCrashDump;
		return &cCrashDump;
	}
	

	// -----------------
	// static �Լ���
	// -----------------
	// ������ Crash ����
	void CCrashDump::Crash(void)
	{
		fputs("Crush!!\n", stdout);
		int *p = nullptr;
		*p = 0;
	}

	// �������� �����
	LONG WINAPI CCrashDump::MyExceptionFilter(__in PEXCEPTION_POINTERS pExceptionPointer)
	{
		// -----------------
		// ���Ͽ� ī��Ʈ�� ���̱� ���� DumpCount ����.
		// -----------------		
		long DumpCount = InterlockedIncrement(&_DumpCount);

		// -----------------
		// ���� ��¥�� �ð��� ���ؿ´�.
		// -----------------
		SYSTEMTIME	stNowTime;
		GetLocalTime(&stNowTime);

		// -----------------
		// �������� �̸� �����
		// -----------------
		TCHAR tcFileName[MAX_PATH];
		swprintf_s(tcFileName, MAX_PATH, _T("Dump_%d%02d%02d_%02d.%02d.%02d._%d.dmp"), stNowTime.wYear, stNowTime.wMonth, stNowTime.wDay,
			stNowTime.wHour, stNowTime.wMinute, stNowTime.wSecond, DumpCount);

		// -----------------
		// ���� ���� �������̶�� ȭ�鿡 ����ϱ�
		// -----------------
		_tprintf_s(_T("\n\n\n!!! Crash Error !!!  %d.%d.%d / %d:%d:%d \n"), stNowTime.wYear, stNowTime.wMonth, stNowTime.wDay,
			stNowTime.wHour, stNowTime.wMinute, stNowTime.wSecond);

		_tprintf_s(_T("Now Save dump File...\n"));

		// -----------------
		// �������� �����ϱ�
		// -----------------
		// ���� ����
		HANDLE hDumpFile = CreateFile(tcFileName, GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

		// ���������� ���������� �������� �����ϱ�
		if (hDumpFile != INVALID_HANDLE_VALUE)
		{
			// ���� ���� ����
			_MINIDUMP_EXCEPTION_INFORMATION DumpExceptionInfo;
			DumpExceptionInfo.ThreadId = GetCurrentThreadId();
			DumpExceptionInfo.ExceptionPointers = pExceptionPointer;
			DumpExceptionInfo.ClientPointers = TRUE;

			// ����
			MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), hDumpFile, MiniDumpWithFullMemory, &DumpExceptionInfo, NULL, NULL);

			// �ڵ� �ݱ�
			CloseHandle(hDumpFile);

			// ���� �� �ƴٰ� ���
			_tprintf_s(_T("CrashDump Save Finish ! \n"));
		}

		// �ش� ���� �����ؾ� ���α׷��� ��� ����ȴ�.
		return EXCEPTION_EXECUTE_HANDLER;

	}

	// �������� �����ϴ� �Լ� ����ϱ�.
	void CCrashDump::SetHandlerDump()
	{
		// ���� ���α׷���, ���μ����� �� �������� �ֻ��� ���� ó���⸦ ��ü�� �� �ִ�.
		// �� �Լ��� ȣ���� ��, ����� ���� �ʴ� ���μ������� ���ܰ� �߻��ϰ�, ó������ ���� ���� ���Ϳ��� ���ܰ� �߻��ϸ�
		// �ش� ���ʹ� ���ڷ� ���޵� 'MyExceptionFilter' ���� �Լ��� ȣ���Ѵ�.
		// MyExceptionFilter()�Լ��� ����, �������� �����Ѵ�.
		SetUnhandledExceptionFilter(MyExceptionFilter);

		// C ��Ÿ�� ���̺귯�� ������ �����ڵ鷯 ����� ���� ���� API ��ŷ
		// �ٵ� vs 2017������ �̰� �ȸ���...
		//static CAPIHook apiHook("kernel32.dll", "SetUnhandledExceptionFilter", (PROC)RedirectedSetUnhandlerExceptionFilter, true);
	}

	


	// --------------
	// ���� �ڵ鸵 �Ǿ��� �� ȣ��Ǵ� �Լ���
	// -------------
	// CRT �Լ��� ���ڸ� �߸��־��� �� (nullptr��..) ȣ�� (CRT�� C RungTime�� ����)
	void CCrashDump::myInvalidParameterHandler(const TCHAR* expression, const TCHAR* function, const TCHAR* filfile, UINT line, UINT_PTR pReserved)
	{
		fputs("myInvalidParameterHandler()!!\n", stdout);
		Crash();
	}

	// �� �� CRT �Լ� ���� �߻� �� ȣ��
	int CCrashDump::_custom_Report_hook(int ireposttype, char* message, int* returnvalue)
	{
		fputs("_custom_Report_hook()!!\n", stdout);
		Crash();
		return true;
	}

	// pure virtual function called ���� �߻� �� ȣ��
	void CCrashDump::myPurecallHandler(void)
	{
		fputs("myPurecallHandler()!!\n", stdout);
		Crash();
	}
	
	// ���� �ñ׳� �߻� �� ȣ��.
	void CCrashDump::signalHandler(int Error)
	{
		printf("signalHandler()!! %d\n", Error);
		Crash();
	}
}