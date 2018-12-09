// Main.cpp : 이 파일에는 'main' 함수가 포함됩니다. 거기서 프로그램 실행이 시작되고 종료됩니다.
//

#include "pch.h"
#include <iostream>
#include "Heap.h"

using namespace std;

int main()
{
	Heap<char> he;
	he.Init();

	he.Insert('A', 1);
	he.Insert('B', 2);
	he.Insert('C', 3);

	char Data;
	he.Delete(&Data);
	cout << Data << endl;

	he.Insert('A', 1);
	he.Insert('B', 2);
	he.Insert('C', 3);

	he.Delete(&Data);
	cout << Data << endl;

	while (he.Delete(&Data))
	{
		cout << Data << endl;
	}

	return 0;
}