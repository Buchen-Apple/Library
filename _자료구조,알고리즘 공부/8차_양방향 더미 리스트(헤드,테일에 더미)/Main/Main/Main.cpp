// Main.cpp : 이 파일에는 'main' 함수가 포함됩니다. 거기서 프로그램 실행이 시작되고 종료됩니다.
//

#include "pch.h"
#include <iostream>
#include "DLinkedList_Dummy.h"

using namespace std;

void InsertData(int Data);
void Traversal();
void DeleteData(int Data);

DLinkedList_Dummy<int> list;

int main()
{
	list.Init();

	InsertData(10);
	InsertData(20);
	InsertData(30);
	InsertData(40);

	// 순회
	Traversal();

	// 삭제
	DeleteData(10);

	// 순회
	Traversal();

	return 0;
}

void InsertData(int Data)
{
	list.Insert_back(Data);
}

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

void DeleteData(int Data)
{
	cout << "Delete " << Data << " -->";
	int DeleteData;
	if (list.First(&DeleteData))
	{
		if (DeleteData == Data)
		{
			cout << "Delete OK" << endl;
			list.Remove();
		}

		else
		{
			while (list.Next(&DeleteData))
			{
				if (DeleteData == Data)
				{
					cout << "Delete OK" << endl;
					list.Remove();
					break;
				}
			}
		}
	}
	cout << endl;
}