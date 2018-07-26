#ifndef __CRASH_DUMP_H__
#define __CRASH_DUMP_H__

#include <Windows.h>

namespace Library_Jingyu
{

	class CCrashDump
	{
		// �̱����� ���� �����ڴ� Private
		// ������
		CCrashDump(void);

	public:
		static long _DumpCount;

	public:
		// �̱��� ���
		static CCrashDump* GetInstance();


		// -----------------
		// static �Լ���
		// -----------------
		// ������ Crash ����
		static void Crash(void);

		// �������� �����
		static LONG WINAPI MyExceptionFilter(__in PEXCEPTION_POINTERS pExceptionPointer);

		// C ��Ÿ�� ���̺귯�� ������ ���� �ڵ鷯 ����� ����, �ش� �Լ��� ȣ��ǵ��� �ϱ� ���Ѱ�.
		// �ٵ� 2017������ �ȸ���..
		// static LONG WINAPI RedirectedSetUnhandlerExceptionFilter(EXCEPTION_POINTERS *exceptionInfo);

		// �������� �����ϴ� �Լ� ����ϱ�.
		static void SetHandlerDump();


		// --------------
		// ���� �ڵ鸵 �Ǿ��� �� ȣ��Ǵ� �Լ���
		// -------------
		// CRT �Լ��� ���ڸ� �߸��־��� �� (nullptr��..) ȣ�� (CRT�� C RungTime�� ����)
		static void myInvalidParameterHandler(const TCHAR* expression, const TCHAR* function, const TCHAR* filfile, UINT line, UINT_PTR pReserved);

		// �� �� CRT �Լ� ���� �߻� �� ȣ��
		static int _custom_Report_hook(int ireposttype, char* message, int* returnvalue);

		// pure virtual function called ���� �߻� �� ȣ��
		static void myPurecallHandler(void);

		// ���� �ñ׳� �߻� �� ȣ��.
		static void signalHandler(int Error);
	};
	

}



#endif // !__CRASH_DUMP_H__
