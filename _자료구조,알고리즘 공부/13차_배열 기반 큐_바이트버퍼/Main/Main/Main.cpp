// Main.cpp : 이 파일에는 'main' 함수가 포함됩니다. 거기서 프로그램 실행이 시작되고 종료됩니다.
//

#include "pch.h"
#include <iostream>
#include "arrayQueue_Byte.h"

using namespace std;

int main()
{
	arrayQueue_Byte queue;
	queue.Init();

	// 데이터 넣기
	int Data = 10;
	queue.Enqueue((char*)&Data, 4);

	queue.Dequeue((char*)&Data, 4);
	cout << Data << endl;

	Data = 20;
	queue.Enqueue((char*)&Data, 4);

	queue.Dequeue((char*)&Data, 4);
	cout << Data << endl;	

	return 0;
}
