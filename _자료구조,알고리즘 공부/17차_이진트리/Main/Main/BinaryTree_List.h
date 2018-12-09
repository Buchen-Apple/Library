#ifndef __BINARY_TREE_LIST_H__
#define __BINARY_TREE_LIST_H__

typedef int BTData;

// ���� Ʈ���� ���
class BTNode
{
	friend class BinaryTree_List;

	BTData m_data;
	BTNode* m_pLeft;
	BTNode* m_pRight;

	BTNode();	
};

// Ʈ�� Ŭ����
// �ܺο��� ������ ��带 �̿��ϴ� �Լ����� ���ǵǾ� �ִ�.
class BinaryTree_List
{
	typedef void Action(BTData data);

	// ������. ������ȸ
	void NodeDelete(BTNode* pNode);

public:
	// ����Ʈ���� ��� ����
	BTNode* MakeBTreeNode(void);

	// ���ڷ� ���� ����� data ���
	BTData GetData(BTNode* pNode);

	// ���ڷ� ���� ����� data ����
	void SetData(BTNode* pNode, BTData Data);

	// ���ڷ� ���� ����� Left ����Ʈ�� ��Ʈ ���
	BTNode* GetLeftSubTree(BTNode* pNode);

	// ���ڷ� ���� ����� Right ����Ʈ�� ��Ʈ ���
	BTNode* GetRightSubTree(BTNode* pNode);

	// 1�� ������ Left����Ʈ����, 2������ ����
	void SetLeftSubTree(BTNode* pMain, BTNode* pLeftSub);

	// 1�� ������ Right����Ʈ����, 2������ ����
	void SetRightSubTree(BTNode* pMain, BTNode* pRightSub);

public:
	// ------------- ��ȸ ---------------

	// ���� ��ȸ
	void PreorderTraverse(BTNode* pNode, Action function);

	// ���� ��ȸ
	void InorderTraverse(BTNode* pNode, Action function);

	// ���� ��ȸ
	void PostorderTraverse(BTNode* pNode, Action function);
};

#endif // !__BINARY_TREE_LIST_H__
