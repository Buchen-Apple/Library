// Main.cpp : 이 파일에는 'main' 함수가 포함됩니다. 거기서 프로그램 실행이 시작되고 종료됩니다.
//

#include "pch.h"
#include <iostream>
#include "ExpressionTree.h"

using namespace std;

int main()
{
	char exp[] = "(1+2)*7";

	ExpressionTree ExpTree;

	BTNode* Root = ExpTree.CreateExpTree(exp);
	int Result = ExpTree.ExpTreeResult(Root);

	ExpTree.ShowPreOrder(Root);
	cout << endl;

	ExpTree.ShowInOrder(Root);
	cout << endl;

	ExpTree.ShowPostOrder(Root);
	cout << endl;

	return 0;
}
