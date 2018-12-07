// Main.cpp : 이 파일에는 'main' 함수가 포함됩니다. 거기서 프로그램 실행이 시작되고 종료됩니다.
//

#include "pch.h"
#include <iostream>
#include "ArrayQueue.h"

using namespace std;


int main()
{
	arrayQueue queue;
	queue.Init();

	// 데이터 넣기
	queue.Enqueue(10);
	queue.Enqueue(20);
	queue.Enqueue(30);
	queue.Enqueue(40);

	// 데이터 빼면서 출력
	int Data;
	while(queue.Dequeue(&Data))
	{
		cout << Data << endl;
	}
	cout << endl;

	return 0;
}
