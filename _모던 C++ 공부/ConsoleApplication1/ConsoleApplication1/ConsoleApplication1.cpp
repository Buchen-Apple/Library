// ConsoleApplication1.cpp : 이 파일에는 'main' 함수가 포함됩니다. 거기서 프로그램 실행이 시작되고 종료됩니다.
//

#include "pch.h"
#include <iostream>
#include <windows.h>
#include <initializer_list>
#include <string>
#include <vector>

using std::initializer_list;
using std::cout;
using std::endl;
using std::vector;
using std::string;

class AT
{
public:
	AT()
	{
		cout << "AT constructor\n";
	}
};

void incr(AT&& temp)
{
	cout << "aaaa\n";
}



int main()
{
	AT a;
	incr(a);
		 
	return 0;
}

