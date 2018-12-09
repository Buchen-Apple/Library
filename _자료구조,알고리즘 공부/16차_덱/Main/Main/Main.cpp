// Main.cpp : 이 파일에는 'main' 함수가 포함됩니다. 거기서 프로그램 실행이 시작되고 종료됩니다.
//

#include "pch.h"
#include <iostream>
#include "Deque.h"

using namespace std;

int main()
{
	Deque deq;
	deq.Init();

	// 데이터 넣기 1차 
	deq.Enqueue_Head(3);
	deq.Enqueue_Head(2);
	deq.Enqueue_Head(1);

	deq.Enqueue_Tail(4);
	deq.Enqueue_Tail(5);
	deq.Enqueue_Tail(6);

	int Data;
	while (deq.Dequeue_Head(&Data))
	{
		cout << Data << " ";
	}
	cout << endl;


	// 데이터 넣기 2차
	deq.Enqueue_Head(3);
	deq.Enqueue_Head(2);
	deq.Enqueue_Head(1);

	deq.Enqueue_Tail(4);
	deq.Enqueue_Tail(5);
	deq.Enqueue_Tail(6);

	while (deq.Dequeue_Tail(&Data))
	{
		cout << Data << " ";
	}
	cout << endl;

	return 0;
}
