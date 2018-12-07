// Main.cpp : 이 파일에는 'main' 함수가 포함됩니다. 거기서 프로그램 실행이 시작되고 종료됩니다.
//

#include "pch.h"
#include <iostream>
#include "ListStack.h"

using namespace std;

int main()
{
	listStack stack;
	stack.Init();

	stack.Push(10);
	stack.Push(20);
	stack.Push(30);
	stack.Push(40);

	cout << "Size : " << stack.GetNodeSize() << endl;
	while (stack.IsEmpty() == false)
	{
		int Data;
		stack.Pop(&Data);
		cout << Data << endl;
	}
	cout << endl;

	return 0;
}
