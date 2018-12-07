// Main.cpp : 이 파일에는 'main' 함수가 포함됩니다. 거기서 프로그램 실행이 시작되고 종료됩니다.
//

#include "pch.h"
#include <iostream>
#include "NodeList.h"

using namespace std;

int main()
{
	NodeList list;
	list.ListInit();

	// 데이터 3개 넣기
	list.ListInsert(10);
	list.ListInsert(40);
	list.ListInsert(20);
	list.ListInsert(30);

	// 데이터 전체 조회
	int Data;
	cout << "Total NodeCount : " << list.ListCount() << endl;
	if (list.ListFirst(&Data))
	{
		cout << Data << endl;

		while (list.ListNext(&Data))
		{
			cout << Data << endl;
		}
	}
	cout << endl;

	// 데이터 40 삭제
	cout << "Delete 40 -->";
	if (list.ListFirst(&Data))
	{
		if (Data == 40)
		{
			list.ListRemove();
			cout << "Delete OK" << endl;;
		}

		else
		{
			while (list.ListNext(&Data))
			{
				if (Data == 40)
				{
					list.ListRemove();
					cout << "Delete OK" << endl;;
					break;
				}
			}
		}
	}
	cout << endl;

	// 데이터 전체 조회
	cout << "Total NodeCount : " << list.ListCount() << endl;
	if (list.ListFirst(&Data))
	{
		cout << Data << endl;

		while (list.ListNext(&Data))
		{
			cout << Data << endl;
		}
	}
	cout << endl;
    
	return 0;
}
