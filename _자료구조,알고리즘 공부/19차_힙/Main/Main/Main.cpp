// Main.cpp : 이 파일에는 'main' 함수가 포함됩니다. 거기서 프로그램 실행이 시작되고 종료됩니다.
//

#include "pch.h"
#include <iostream>
#include "Heap.h"

using namespace std;

// 첫 번째 인자의 우선순위가 더 높을경우, 0보다 큰 값
// 두 번째 인자의 우선순위가 더 높을경우, 0보다 작은값
// 둘이 우선순위가 같으면 0
int Comp(char d1, char d2)
{
	if (d1 < d2)
		return 1;

	else if (d1 > d2)
		return -1;

	else
		return 0;
}

int main()
{
	Heap<char> he;
	he.Init(Comp);

	he.Insert('A');
	he.Insert('B');
	he.Insert('C');

	char Data;
	he.Delete(&Data);
	cout << Data << endl;

	he.Insert('A');
	he.Insert('B');
	he.Insert('C');

	he.Delete(&Data);
	cout << Data << endl;

	while (he.Delete(&Data))
	{
		cout << Data << endl;
	}

	return 0;
}