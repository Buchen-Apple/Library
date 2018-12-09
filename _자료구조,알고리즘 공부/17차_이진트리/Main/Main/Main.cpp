// Main.cpp : 이 파일에는 'main' 함수가 포함됩니다. 거기서 프로그램 실행이 시작되고 종료됩니다.
//

#include "pch.h"
#include <iostream>
#include "BinaryTree_List.h"

using namespace std;

void Action(BTData data)
{
	cout << data << " ";
}


int main()
{
	BinaryTree_List BTtool;

	BTNode* bt1 = BTtool.MakeBTreeNode();
	BTNode* bt2 = BTtool.MakeBTreeNode();
	BTNode* bt3 = BTtool.MakeBTreeNode();
	BTNode* bt4 = BTtool.MakeBTreeNode();

	BTtool.SetData(bt1, 1);
	BTtool.SetData(bt2, 2);
	BTtool.SetData(bt3, 3);
	BTtool.SetData(bt4, 4);

	BTtool.SetLeftSubTree(bt1, bt2);
	BTtool.SetRightSubTree(bt1, bt3);
	BTtool.SetLeftSubTree(bt2, bt4);

	BTtool.InorderTraverse(bt1, Action);
	cout << endl;

	BTtool.PreorderTraverse(bt1, Action);
	cout << endl;
	
	BTtool.PostorderTraverse(bt1, Action);
	cout << endl;

	return 0;
}