#include "pch.h"
#include "ExpressionTree.h"
#include "windows.h"
#include <iostream>
#include <stdio.h>

using namespace std;

// ��¿� �Լ�
void ShowAction(BTData data)
{
	// �ǿ����ڿ� �����ڴ� �ٸ��� ��µǾ�� �Ѵ�.
	if (0 <= data && data <= 9)
		cout << data << " ";

	else
		printf("%c ", data);
}


// ����Ʈ�� ����
BTNode* ExpressionTree::CreateExpTree(char exp[])
{
	// ���ڷ� ���� exp�� ���� ǥ������� ��ȯ
	m_ConvClass.ConvToRPNExp(exp);

	// ���� ǥ������� ��ȯ�� ���ڸ� ����Ʈ���� ��ȯ
	listStack<BTNode*> stack;
	stack.Init();

	int expLen = strlen(exp);
	int TempIndex = 0;
	int i = 0;

	// �ϳ��� ���鼭 Ȯ���Ѵ�.
	while (i < expLen)
	{
		// ���ο� ��� ����
		BTNode* NewNode = m_BTtool.MakeBTreeNode();

		// �ǿ��������� Ȯ��
		if (isdigit(exp[i]))
		{
			// �� �����ڶ�� ���ڸ� ������ ������ ��忡 ����
			m_BTtool.SetData(NewNode, exp[i] - '0');
		}

		// �������� ���
		else
		{
			BTNode* PopNode;

			// ���ÿ� �ִ� 2���� �ǿ����ڸ� ������ ��, ���ο� ����� ��/�쿡 ����
			// ���� Ƣ��� ���� �����ʿ� ����
			stack.Pop(&PopNode);
			m_BTtool.SetRightSubTree(NewNode, PopNode);

			stack.Pop(&PopNode);
			m_BTtool.SetLeftSubTree(NewNode, PopNode);

			m_BTtool.SetData(NewNode, exp[i]);
		}

		// ���ο� ��带 ���ÿ� Push
		stack.Push(NewNode);

		++i;
	}

	// ��Ʈ ��ȯ
	BTNode* Root;

	stack.Pop(&Root);
	return Root;
}

// ����Ʈ�� ���
int ExpressionTree::ExpTreeResult(BTNode* Root)
{
	// �� �̻� ��尡 ���� ��� data ����
	if (m_BTtool.GetLeftSubTree(Root) == nullptr &&
		m_BTtool.GetRightSubTree(Root) == nullptr)
	{
		return m_BTtool.GetData(Root);
	}

	int Result = 0;

	// �ش� ����� ����, �������� �˾ƿ´�.
	int op1 = ExpTreeResult(m_BTtool.GetLeftSubTree(Root));
	int op2 = ExpTreeResult(m_BTtool.GetRightSubTree(Root));

	// ���� ����� ���꿡 ���� ���
	switch (m_BTtool.GetData(Root))
	{
	case '+':
		Result = op1 + op2;
		break;

	case '-':
		Result = op1 - op2;
		break;

	case '*':
		Result = op1 * op2;
		break;

	case '/':
		Result = op1 / op2;
		break;
	}

	return Result;
}

// ���� ǥ��� ��� ���
void ExpressionTree::ShowPreOrder(BTNode* Root)
{
	m_BTtool.PreorderTraverse(Root, ShowAction);
}

// ���� ǥ��� ��� ���
void ExpressionTree::ShowInOrder(BTNode* Root)
{
	// Ż�� ����. ��尡 nullptr�̸� ���� �����Ѱ�.
	if (Root == nullptr)
		return;	

	int RootData = m_BTtool.GetData(Root);

	// �����ڶ�� ��ȣ ģ��
	if ('*' <= RootData && RootData <= '/')
		printf("(");

	// ���� ����
	ShowInOrder(m_BTtool.GetLeftSubTree(Root));	

	// ��Ʈ ����
	ShowAction(m_BTtool.GetData(Root));	

	// ������ ����
	ShowInOrder(m_BTtool.GetRightSubTree(Root));

	// �����ڶ�� ��ȣ ģ��
	if ('*' <= RootData && RootData <= '/')
		printf(") ");
}

// ���� ǥ��� ��� ���
void ExpressionTree::ShowPostOrder(BTNode* Root)
{
	m_BTtool.PostorderTraverse(Root, ShowAction);
}

