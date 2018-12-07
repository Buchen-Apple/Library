// Main.cpp : 이 파일에는 'main' 함수가 포함됩니다. 거기서 프로그램 실행이 시작되고 종료됩니다.
//

#include "pch.h"
#include <iostream>
#include "DoubleLinkedList.h"

using namespace std;

// 데이터 넣기
void DataInsert(int Data);

// 전체 순회하며 출력
void Traversal();

// 특정 값 삭제
void DataDelete(int Data);

DLinkedList list;

int main()
{	
	list.Init();

	// 데이터 넣기
	DataInsert(10);
	DataInsert(20);
	DataInsert(30);
	DataInsert(40);

	// 순회
	Traversal();	

	// 삭제
	DataDelete(10);	

	// 순회
	Traversal();

	return 0;
}

// 데이터 넣기
void DataInsert(int Data)
{
	list.Insert(Data);
}

// 전체 순회하며 출력
void Traversal()
{
	cout << "Size : " << list.Size() << endl;
	int Data;
	if (list.First(&Data))
	{
		cout << Data << endl;

		while (list.Next(&Data))
		{
			cout << Data << endl;
		}
	}
	cout << endl;

}

// 특정 값 삭제
void DataDelete(int Data)
{
	cout << "Delete " << Data << " --> ";
	if (list.First(&Data))
	{
		if (Data == 20)
		{
			list.Remove();
			cout << "Delete OK" << endl;
		}

		else
		{
			while (list.Next(&Data))
			{
				if (Data == 20)
				{
					list.Remove();
					cout << "Delete OK" << endl;
					break;
				}
			}
		}
	}
	cout << endl;
}