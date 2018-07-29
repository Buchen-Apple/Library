#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "ListBaseStack.h"
#include "ExpressionTree.h"

// ���� Ʈ�� ����
BTreeNode* MakeExpTree(char exp[])
{
	Stack stack;
	BTreeNode* pnode;

	size_t explen = strlen(exp);
	int i;

	StackInit(&stack);

	for (i = 0; i < explen; ++i)
	{
		pnode = MakeBTreeNode();

		// ���ڶ��
		if (isdigit(exp[i]))
			SetData(pnode, exp[i] - '0');

		// �����ڶ��
		else
		{
			MakeRightSubTree(pnode, SPop(&stack));
			MakeLeftSubTree(pnode, SPop(&stack));
			SetData(pnode, exp[i]);
		}

		SPush(&stack, pnode);
	}

	return SPop(&stack);
}

// ���� Ʈ�� ���
int EvaluateExpTree(BTreeNode* bt)
{
	int op1, op2;

	if (GetLeftSubTree(bt) == NULL && GetRightSubTree(bt) == NULL)
		return GetData(bt);

	op1 = EvaluateExpTree(GetLeftSubTree(bt));
	op2 = EvaluateExpTree(GetRightSubTree(bt));

	switch (GetData(bt))
	{
	case '+':
		return op1 + op2;
	case '-':
		return op1 - op2;
	case '*':
		return op1 * op2;
	case '/':
		return op1 / op2;
	}

	return 0;
}

// �� ����Լ�
void ShowNodeData(int data)
{
	if (0 <= data && data <= 9)
		printf("%d ", data);
	else
		printf("%c ", data);
}

// ���� ǥ��� ��� ���
void ShowPreFixTypeExp(BTreeNode* bt)
{
	FirstTraverse(bt, ShowNodeData);
}

// ���� ǥ��� ��� ���
void ShowInFixTypeExp(BTreeNode* bt)
{	
	SecondTraverse(bt, ShowNodeData);	
}

// ���� ǥ��� ��� ���
void ShowPostFixTypeExp(BTreeNode* bt)
{
	EndTraverse(bt, ShowNodeData);
}