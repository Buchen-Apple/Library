#include "BinaryTree.h"
#include <stdlib.h>
#include <stdio.h>


////////////////////////////////////
// ����Ʈ�� ����ü ���� �Լ�
////////////////////////////////////
BTreeNode* MakeBTreeNode(void)
{
	BTreeNode * Node = (BTreeNode*)malloc(sizeof(BTreeNode));
	Node->m_Left = NULL;
	Node->m_Right = NULL;

	return Node;
}

////////////////////////////////////
// ����Ʈ�� ����ü�� ������ ��� �Լ�
////////////////////////////////////
BTData GetData(BTreeNode* bt)
{
	return bt->m_data;
}

////////////////////////////////////
// ����Ʈ�� ����ü�� ������ ���� �Լ�
////////////////////////////////////
void SetData(BTreeNode* bt, BTData data)
{
	bt->m_data = data;
}

////////////////////////////////////
// ���޵� ����Ʈ����, ���� ����Ʈ���� ��Ʈ��带 ��� �Լ�
////////////////////////////////////
BTreeNode* GetLeftSubTree(BTreeNode* bt)
{
	return bt->m_Left;
}

////////////////////////////////////
// ���޵� ����Ʈ����, ������ ����Ʈ���� ��Ʈ��带 ��� �Լ�
////////////////////////////////////
BTreeNode* GetRightSubTree(BTreeNode* bt)
{
	return bt->m_Right;
}

////////////////////////////////////
// ���ʿ� ����Ʈ�� ���� �Լ�.
// main�� ���ʿ� sub ����
////////////////////////////////////
void MakeLeftSubTree(BTreeNode* main, BTreeNode* sub)
{
	if (main->m_Left != NULL)
		DeleteTree(main->m_Left);

	main->m_Left = sub;
}

////////////////////////////////////
// �����ʿ� ����Ʈ�� ���� �Լ�.
// main�� �����ʿ� sub ����
////////////////////////////////////
void MakeRightSubTree(BTreeNode* main, BTreeNode* sub)
{
	if (main->m_Right != NULL)
		DeleteTree(main->m_Right);

	main->m_Right = sub;
}

////////////////////////////////////
// ���� ��ȸ �Լ�
////////////////////////////////////
void FirstTraverse(BTreeNode* bt, VisitFuncPtr action)
{
	if (bt == NULL)
		return;

	action(GetData(bt));
	FirstTraverse(bt->m_Left, action);
	FirstTraverse(bt->m_Right, action);
}


////////////////////////////////////
// ���� ��ȸ �Լ�
////////////////////////////////////
void SecondTraverse(BTreeNode* bt, VisitFuncPtr action)
{
	if (bt == NULL)
		return;

	if(bt->m_Left != NULL && bt->m_Right != NULL)
		printf(" ( ");

	SecondTraverse(bt->m_Left, action);		
	action(GetData(bt));
	SecondTraverse(bt->m_Right, action);	

	if (bt->m_Left != NULL && bt->m_Right != NULL)
		printf(" ) ");
}

////////////////////////////////////
// ���� ��ȸ �Լ�
////////////////////////////////////
void EndTraverse(BTreeNode* bt, VisitFuncPtr action)
{
	if (bt == NULL)
		return;

	EndTraverse(bt->m_Left, action);
	EndTraverse(bt->m_Right, action);
	action(GetData(bt));
}

////////////////////////////////////
// Ʈ�� �Ҹ� �Լ�
// ���޵� ��Ʈ ����� ��,�� ��� Ʈ���� �Ҹ��Ų��.
// ���� ��ȸ�� �ؾ� ��� Ʈ���� �Ҹ�ȴ�.
////////////////////////////////////
void DeleteTree(BTreeNode* bt)
{
	if (bt == NULL)
		return;

	DeleteTree(bt->m_Left);
	DeleteTree(bt->m_Right);
	free(bt);
}

