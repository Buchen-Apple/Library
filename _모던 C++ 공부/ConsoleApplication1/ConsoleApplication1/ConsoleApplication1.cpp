// ConsoleApplication1.cpp : 이 파일에는 'main' 함수가 포함됩니다. 거기서 프로그램 실행이 시작되고 종료됩니다.
//

#include "pch.h"
#include <iostream>
#include <windows.h>
#include <initializer_list>

using std::initializer_list;
using std::cout;
using std::endl;




class MyClass
{
	int a;
	int b;
	int c;

public:
	void Func() const
	{
		cout << "TTT\n";
	}
	void Func2()
	{
		cout << "aaaaa\n";
	}
};



int main()
{
	MyClass Base;

	Base.Func();
	Base.Func2();
	
	const MyClass Sub = Base;

	Sub.Func();
	Sub.Func2();
		 
	return 0;
}

