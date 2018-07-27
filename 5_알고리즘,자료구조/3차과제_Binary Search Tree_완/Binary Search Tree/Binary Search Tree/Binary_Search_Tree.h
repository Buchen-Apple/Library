#ifndef __BINARY_SEARCH_TREE_H__
#define __BINARY_SEARCH_TREE_H__

typedef int BData;

class CBST
{
	struct BTreeNode
	{
		BData iNodeData;
		BTreeNode* stpLeftChild;
		BTreeNode* stpRightChild;
	};

	BTreeNode* m_pRootNode;

public:
	// ������
	CBST(BData data = 10);

	// ���� �Լ�
	bool Insert(BData data);

	// ���� �Լ�
	bool Delete(BData data);

	// ��ȸ�ϸ� ��� �� ��� �Լ�
	void Traversal();

	// ���� �˻�
	void InorderTraversal(BTreeNode* pSearchNode);

	// ��Ʈ ����� �� ��ȯ
	BData GetRootNodeData();


};

#endif // !__BINARY_SEARCH_TREE_H__

