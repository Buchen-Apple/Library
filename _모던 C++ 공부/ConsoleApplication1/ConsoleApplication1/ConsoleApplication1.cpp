// ConsoleApplication1.cpp : 이 파일에는 'main' 함수가 포함됩니다. 거기서 프로그램 실행이 시작되고 종료됩니다.
//

#include "pch.h"
#include <iostream>

using std::cout;

int main()
{
	int someInt = 10;
	bool somebool1 = (bool)someInt;
	bool somebool2 = bool(someInt);
	bool somebool3 = static_cast<bool>(someInt);
	return 0;
}

