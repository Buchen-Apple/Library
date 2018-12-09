#include "pch.h"
#include "BinaryTree_List.h"

// ����Ʈ���� ���
BTNode::BTNode()
{
	m_pLeft = nullptr;
	m_pRight = nullptr;
}



// ������. ������ȸ
void BinaryTree_List::NodeDelete(BTNode* pNode)
{
	// �� �̻� ��尡 ������ ����
	if (pNode == nullptr)
		return;

	// ���� ��� ����
	NodeDelete(pNode->m_pLeft);

	// ������ ��� ����
	NodeDelete(pNode->m_pRight);

	// ���� ��� ����
	delete pNode;
}


// ����Ʈ���� ��� ����
BTNode* BinaryTree_List::MakeBTreeNode(void)
{
	return new BTNode;
}

// ���ڷ� ���� ����� data ���
BTData BinaryTree_List::GetData(BTNode* pNode)
{
	return pNode->m_data;
}

// ���ڷ� ���� ����� data ����
void BinaryTree_List::SetData(BTNode* pNode, BTData Data)
{
	pNode->m_data = Data;
}

// ���ڷ� ���� ����� Left ����Ʈ�� ��Ʈ ���
BTNode* BinaryTree_List::GetLeftSubTree(BTNode* pNode)
{
	return pNode->m_pLeft;
}

// ���ڷ� ���� ����� Right ����Ʈ�� ��Ʈ ���
BTNode* BinaryTree_List::GetRightSubTree(BTNode* pNode)
{
	return pNode->m_pRight;
}

// 1�� ������ Left����Ʈ����, 2������ ����
void BinaryTree_List::SetLeftSubTree(BTNode* pMain, BTNode* pLeftSub)
{
	if (pMain->m_pLeft != nullptr)
		NodeDelete(pMain->m_pLeft);

	pMain->m_pLeft = pLeftSub;
}

// 1�� ������ Right����Ʈ����, 2������ ����
void BinaryTree_List::SetRightSubTree(BTNode* pMain, BTNode* pRightSub)
{
	if (pMain->m_pRight != nullptr)
		NodeDelete(pMain->m_pRight);

	pMain->m_pRight = pRightSub;
}

// ���� ��ȸ
void BinaryTree_List::PreorderTraverse(BTNode* pNode, Action function)
{
	// Ż�� ����. ��尡 nullptr�̸� ���� �����Ѱ�.
	if (pNode == nullptr)
		return;

	// ��Ʈ ����
	function(pNode->m_data);

	// ���� ����
	PreorderTraverse(pNode->m_pLeft, function);

	// ������ ����
	PreorderTraverse(pNode->m_pRight, function);
}

// ���� ��ȸ
void BinaryTree_List::InorderTraverse(BTNode* pNode, Action function)
{
	// Ż�� ����. ��尡 nullptr�̸� ���� �����Ѱ�.
	if (pNode == nullptr)
		return;

	// ���� ����
	InorderTraverse(pNode->m_pLeft, function);

	// ��Ʈ ����
	function(pNode->m_data);

	// ������ ����
	InorderTraverse(pNode->m_pRight, function);
}

// ���� ��ȸ
void BinaryTree_List::PostorderTraverse(BTNode* pNode, Action function)
{
	// Ż�� ����. ��尡 nullptr�̸� ���� �����Ѱ�.
	if (pNode == nullptr)
		return;

	// ���� ����
	PostorderTraverse(pNode->m_pLeft, function);

	// ������ ����
	PostorderTraverse(pNode->m_pRight, function);

	// ��Ʈ ����
	function(pNode->m_data);
}