// ConsoleApplication1.cpp : 이 파일에는 'main' 함수가 포함됩니다. 거기서 프로그램 실행이 시작되고 종료됩니다.
//

#include "pch.h"
#include <iostream>
#include <time.h>
#include "ArrayQueue.h"

using namespace std;

#define CUS_COME_TERM	15

#define CHE_BUR	0
#define BUL_BUR	1
#define DUB_BUR	2

#define CHE_TERM	12
#define BUL_TERM	15
#define DUB_TERM	24

int main()
{
	int makeProc = 0;
	int cheOrder = 0, bulOrder = 0, dubOrder = 0;
	int sec;

	arrayQueue queue;
	queue.Init();

	srand(time(NULL));

	// 1회전은 1초로 가정
	for (sec = 0; sec < 3600; sec++)
	{
		// 고객이 주문할 시간이 되었다면.
		if (sec % CUS_COME_TERM == 0)
		{
			switch (rand() % 3)
			{
			case CHE_BUR:
				if (queue.Enqueue(BUL_TERM) == false)
				{
					cout << "Queue Memory Error!" << endl;
					return 0;
				}
				cheOrder++;
				break;

			case BUL_BUR:
				if(queue.Enqueue(BUL_TERM) == false)
				{
					cout << "Queue Memory Error!" << endl;
					return 0;
				}
				bulOrder++;
				break;

			case DUB_BUR:
				if(queue.Enqueue(DUB_TERM) == false)
				{
					cout << "Queue Memory Error!" << endl;
					return 0;
				}
				dubOrder++;
				break;

			default:
				break;
			}
		}

		// 햄버거 제작상황 체크
		if (makeProc <= 0 && queue.GetNodeSize() > 0)
			queue.Dequeue(&makeProc);

		makeProc--;
	}

	cout << "Report\n" << endl;
	cout << "1. Cheese Burer : " << cheOrder << endl;
	cout << "2. Bulgogi Burer : " << bulOrder << endl;
	cout << "3. Double Burer : " << dubOrder << endl;
	cout << "Waiting room size : " << ARRAY_SIZE << endl;
	
	return 0;
}
