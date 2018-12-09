// Main.cpp : 이 파일에는 'main' 함수가 포함됩니다. 거기서 프로그램 실행이 시작되고 종료됩니다.
//

#include "pch.h"
#include <iostream>
#include "ListQueue.h"

using namespace std;

int main()
{
	listQueue list;
	list.Init();

	list.Enqueue(10);
	list.Enqueue(20);
	list.Enqueue(30);
	list.Enqueue(40);

	int Data;
	while (list.Dequeue(&Data))
	{
		cout << Data << endl;
	}
	cout << endl;

	return 0;
}
