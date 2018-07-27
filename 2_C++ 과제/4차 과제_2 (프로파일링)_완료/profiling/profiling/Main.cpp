#include <iostream>
#include <windows.h>
//#include "Profiling_Class.h"

// Profiling_Class.h�� ����Ǿ� ���� �ʴٸ�, �Ʒ� ��ũ�ε��� �������� ����. 
#ifndef __PROFILING_CLASS_H__
#define BEGIN(STR) 
#define END(STR)
#define FREQUENCY_SET()
#define PROFILING_SHOW()
#define PROFILING_FILE_SAVE()
#define RESET()
#else
#define BEGIN(STR)				BEGIN(STR)
#define END(STR)				END(STR)
#define FREQUENCY_SET()			FREQUENCY_SET()
#define PROFILING_SHOW()		PROFILING_SHOW()
#define PROFILING_FILE_SAVE()	PROFILING_FILE_SAVE()
#define RESET()					RESET()

#endif // !__PROFILING_CLASS_H__

using namespace std;

// �׽�Ʈ�� �Լ�
void Func1();
void Func2();
void Func3();
void Func4();
void Func5();

int main()
{	
	FREQUENCY_SET(); // ���ļ� ���ϱ�	
	PROFILING_SHOW(); // �������ϸ� ��ü����
	cout << "1�� ��" << endl;

	for (int i = 0; i < 1257; ++i)
	{
		Func1();
		Func2();
		Func3();
		Func4();
		Func5();
		if (i == 3)
		{
			RESET(); // �������ϸ� ����
		}
	}
	PROFILING_SHOW(); // �������ϸ� ��ü����

	PROFILING_FILE_SAVE(); // �������ϸ� ���� ����
	return 0;
}

// �׽�Ʈ�� �Լ�
void Func1()
{	
	BEGIN("Func1");
	puts("Func1()\n");
	END("Func1");
}
void Func2()
{
	BEGIN("Func2");
	printf("Func2()\n");
	END("Func2");
}
void Func3()
{
	BEGIN("Func3");
	cout << "Func3()" << endl;
	END("Func3");
}
void Func4()
{
	BEGIN("Func4");
	Sleep(1);
	END("Func4");
}
void Func5()
{
	BEGIN("Func5");
	char* test = new char;
	delete test;
	END("Func5");
}