// Main.cpp : 이 파일에는 'main' 함수가 포함됩니다. 거기서 프로그램 실행이 시작되고 종료됩니다.
//

#include "pch.h"
#include <iostream>
#include "BubbleSort.h"

using namespace std;

void ShowPirntf(int Data)
{
	cout << Data << endl;
}

int main()
{
	BubbleSort<int> Bubble(10, ShowPirntf);

	Bubble.Insert(3);
	Bubble.Insert(2);
	Bubble.Insert(4);
	Bubble.Insert(1);

	Bubble.Sort();

	Bubble.Action();

	return 0;
}
