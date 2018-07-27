#ifndef __AVL_TREE_H__
#define __AVL_TREE_H__

typedef int	TreeData;

struct BTreeNode
{
	TreeData iNodeData;
	BTreeNode* stpLeftChild;
	BTreeNode* stpRightChild;
};

class CAVLTree
{
	/*struct BTreeNode
	{
		TreeData iNodeData;
		BTreeNode* stpLeftChild;
		BTreeNode* stpRightChild;
	};*/

	BTreeNode* m_pRootNode;

public:
	// ������
	CAVLTree();

	BTreeNode* GetRootNode()
	{
		return m_pRootNode;
	}

	// ���� �Լ�
	void Insert(BTreeNode** pNode, TreeData data);

	// ���� �Լ�
	void Delete(TreeData data);

	// �ش� ����� �� ��ȯ.
	TreeData GetData(BTreeNode* pNode)
	{
		return pNode->iNodeData;
	}

	// �ش� ����� ���ʳ�� �ּ� ��ȯ
	BTreeNode* GetLeftNode(BTreeNode* pNode)
	{
		return pNode->stpLeftChild;
	}

	//  �ش� ����� �����ʳ�� �ּ� ��ȯ
	BTreeNode* GetRightNode(BTreeNode* pNode)
	{
		return pNode->stpRightChild;
	}

	// ���� ��ȸ
	void InorderTraversal(BTreeNode* RootNode);

private:
	// �����ϸ� ���뷱���ϴ� ����Լ�
	void DeleteSupport(BTreeNode** pNode, BTreeNode* pSubTree, TreeData data, int iCaseCheck);

	// Main�� Left��� ����
	void ChangeLeftSubTree(BTreeNode* pMain, BTreeNode* pChangeNode);

	// Main�� Right��� ����
	void ChangeRightSubTree(BTreeNode* pMain, BTreeNode* pChangeNode);

	// LLȸ�� (��ȸ��)
	char* LLRotation(BTreeNode* pRotateNode);

	// LRȸ�� (��ȸ�� �� -> ��ȸ��)
	char* LRRotation(BTreeNode* pRotateNode);

	// RRȸ�� (��ȸ��)
	char* RRRotation(BTreeNode* pRotateNode);

	// RLȸ�� (��ȸ�� �� -> ��ȸ��)
	char* RLRotation(BTreeNode* pRotateNode);

	// ���� ���ϱ�
	int GetHeight(BTreeNode* pHeightNode);

	// ���� �μ� ���ϱ�
	int GetHeightDiff(BTreeNode* pDiffNode);

	// ���뷱�� �ϱ�
	char* Rebalance(BTreeNode* pbalanceNode);
};

#endif // !__AVL_TREE_H__
