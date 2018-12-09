#ifndef __EXPRESSION_TREE_H__
#define __EXPRESSION_TREE_H__

#include "Convertor_Cal.h"
#include "BinaryTree_List.h"

// ��¿� �Լ�
void ShowAction(BTData data);

class ExpressionTree
{
	// ǥ��� ��ȯ Ŭ����
	Convertor_Cal m_ConvClass;

	// ����Ʈ�� ����
	BinaryTree_List m_BTtool;

public:
	// ����Ʈ�� ����
	BTNode* CreateExpTree(char exp[]);

	// ����Ʈ�� ���
	int ExpTreeResult(BTNode* Root);

	// ���� ǥ��� ��� ���
	void ShowPreOrder(BTNode* Root);

	// ���� ǥ��� ��� ���
	void ShowInOrder(BTNode* Root);

	// ���� ǥ��� ��� ���
	void ShowPostOrder(BTNode* Root);
};



#endif // !__EXPRESSION_TREE_H__


