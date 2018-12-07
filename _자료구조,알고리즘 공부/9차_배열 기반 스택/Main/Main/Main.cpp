// Main.cpp : 이 파일에는 'main' 함수가 포함됩니다. 거기서 프로그램 실행이 시작되고 종료됩니다.
//

#include "pch.h"
#include <iostream>
#include "ArrayStack.h"

using namespace std;

int main()
{
	arrayStack stack;

	stack.Init();

	// 데이터 넣기
	stack.Push(10);
	stack.Push(20);
	stack.Push(30);
	stack.Push(40);

	// 데이터 하나씩 빼면서 확인
	cout << "Size : " << stack.GetNodeSize() << endl;
	int Data;
	while (stack.Pop(&Data))
	{
		cout << Data << endl;
	}
	cout << endl;

	return 0;
}