// Main.cpp : 이 파일에는 'main' 함수가 포함됩니다. 거기서 프로그램 실행이 시작되고 종료됩니다.
//

#include "pch.h"
#include <iostream>
#include "Selection_Sort.h"

int main()
{
	SelectionSort<int> Ssort(10);

	Ssort.Insert(3);
	Ssort.Insert(4);
	Ssort.Insert(2);
	Ssort.Insert(1);
	Ssort.Insert(5);
	Ssort.Insert(7);
	Ssort.Insert(6);

	Ssort.Sort();

	return 0;
}
