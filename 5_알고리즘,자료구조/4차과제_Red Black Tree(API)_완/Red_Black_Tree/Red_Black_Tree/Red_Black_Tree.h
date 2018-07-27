#ifndef __RED_BLAKC_TREE_H__
#define __RED_BLAKC_TREE_H__

typedef int BData;

class CRBT
{
	enum NODE_COLOR
	{
		BLACK = 0, RED
	};

	struct BTreeNode
	{
		BData iNodeData;
		BTreeNode* stpLeftChild;
		BTreeNode* stpRightChild;
		BTreeNode* stpParent;
		NODE_COLOR eNodeColor;
	};

	BTreeNode* m_pRootNode;
	BTreeNode m_pNilNode;
	int m_iHeight;

public:
	// ������
	CRBT();

	// ���� �Լ�
	bool Insert(BData data);

	// ���� �Լ�
	bool Delete(BData data);

	// ��ȸ�ϸ� ��� �� ��� �Լ�
	void Traversal(HDC hdc, RECT rt);

private:
	// ���� ��ȸ�ϸ� ��,����, �� �߱�
	void preorderTraversal(BTreeNode* pSearchNode, HDC hdc, double x, double y);

	// ��ȸ��
	void LeftRotation(BTreeNode* pRotateNode);

	// ��ȸ��
	void RightRotation(BTreeNode* pRotateNode);

	// Insert �� �� ���߱� ó��
	void InsertColorFixup(BTreeNode* pZNode);

	// ���� ���ϴ� �Լ�
	int SetHeight(BTreeNode* pHeightNode);
};

#endif // !__RED_BLAKC_TREE_H__

