#include <stdio.h>
#include <stdlib.h>
#include "AVL_Tree.h"

// ������
CAVLTree::CAVLTree()
{
	// RootNode�� �⺻ NULL�� ����
	m_pRootNode = NULL;
}

// ���� �Լ�
void CAVLTree::Insert(BTreeNode** pNode, TreeData data)
{
	// RootNode�� NULL�̶��, ���� ��� ����̴�, data�� ��Ʈ��带 �����ϰ� return�Ѵ�.
	if (m_pRootNode == NULL)
	{
		// RootNode ����
		m_pRootNode = (BTreeNode*)malloc(sizeof(BTreeNode));

		m_pRootNode->iNodeData = data;
		m_pRootNode->stpLeftChild = NULL;
		m_pRootNode->stpRightChild = NULL;

		*pNode = m_pRootNode;

		return;
	}

	// RootNode�� NULL�� �ƴ϶��, ������� ����

	// 1. �� ��ġ�� ã������ ��� ���� �� ����.
	// **pNode�� &(p->Left / p->Right)�̴�.
	// *pNode�� p->Left / p->Right�̴�.
	// pNode�� p�̴�.
	if (*pNode == NULL)
	{
		BTreeNode* pNewNode = (BTreeNode*)malloc(sizeof(BTreeNode));
		pNewNode->iNodeData = data;
		pNewNode->stpLeftChild = NULL;
		pNewNode->stpRightChild = NULL;
		*pNode = pNewNode;

		return;
	}
	
	// 2. ���� �۴ٸ�, �������� Insert(&���->��) ���.
	else if ((*pNode)->iNodeData > data)
	{
		Insert((&(*pNode)->stpLeftChild), data);

		// �׸��� ���뷱��
		*pNode = (BTreeNode*)Rebalance(*pNode);
	}

	// 3. ���� ũ�ٸ�, ���������� Insert(&���->����) ���.
	else if ((*pNode)->iNodeData < data)
	{
		Insert((&(*pNode)->stpRightChild), data);

		// �׸��� ���뷱��
		*pNode = (BTreeNode*)Rebalance(*pNode);
	}
}

// ���� �Լ�
void CAVLTree::Delete(TreeData data)
{
	// ������ ��带 ã�´�.
	BTreeNode* pParentNode;
	BTreeNode* pNowNode = m_pRootNode;
	while (1)
	{
		// 1. �θ� ��� ����
		pParentNode = pNowNode;

		// 2. �� ��ġ ����
		// 2-1. [�θ� ��� data > ã������ data]���, ���� �θ��� ���� ��尡 �ȴ�.
		if (pParentNode->iNodeData > data)
			pNowNode = pParentNode->stpLeftChild;

		// 2-2. [�θ� ��� data < ã������ data]���, ���� �θ��� ������ ��尡 �ȴ�.
		else if (pParentNode->iNodeData < data)
			pNowNode = pParentNode->stpRightChild;

		// 2-3. [�θ� ��� data == ã������ data]���. �� ����, ��Ʈ��带 �����ϴ� ��츸 �ش�.
		else
			break;

		// ���� ����. pNowNode�� NULL�̸�, ���� ���(�ٻ�� ���)���� ��� �˻��ߴµ��� ���ϴ� ���� ���� ��. 
		if (pNowNode == NULL)
			return;

		// 3. ���� ���� �����Ǿ�� �ϴ� ������� üũ
		else if (pNowNode->iNodeData == data)
			break;
	}

	// ���� �̿� �����Ǵ� ���� ���� ��带 ã�´�.
	BTreeNode* pBigNode;
	BTreeNode* pBigChildNode;
	BTreeNode* pBigParentNode = NULL;
	int iCaseCheck = 0;

	// case 1. �� �ڽ��� 0���� ���
	if (pNowNode->stpLeftChild == NULL && pNowNode->stpRightChild == NULL)
	{
		// �׳� ���� �����Ѵ�.
		pBigNode = pNowNode;

		iCaseCheck = 1;
	}

	// case 2. ���� �� �ڽ��� �ִ� ���
	// case 3. �� ���� �ڽ��� ���� ���
	else if ((pNowNode->stpLeftChild != NULL && pNowNode->stpRightChild != NULL) || pNowNode->stpLeftChild != NULL)
	{
		// ���� �����, ���� ū ���� ã�´�.
		// ����, pBigNode���� pNowNode�� ���� ��尡 ����ִ�.
		pBigNode = pNowNode->stpLeftChild;
		pBigChildNode = pBigNode->stpRightChild;
		pBigParentNode = pNowNode;

		// 1. ���� �ڽ� �� ���� ���� ū ��� ã��.
		// ������ �ڽ��� NULL�� ���� �� ���� ã�´�.
		while (1)
		{
			// ������ �ڽ��� Nil�̸� ��ġ�� ã�� ��.
			if (pBigChildNode == NULL)
				break;

			pBigParentNode = pBigNode;
			pBigNode = pBigChildNode;
			pBigChildNode = pBigChildNode->stpRightChild;
		}
		iCaseCheck = 2;
	}

	// case 4. �� �����ʿ� �ڽ��� ���� ���
	else if (pNowNode->stpRightChild != NULL)
	{
		// ������ �����, ���� ���� ���� ã�´�.
		// ����, pBigNode���� pNowNode�� ������ ��尡 ����ִ�.
		pBigNode = pNowNode->stpRightChild;
		pBigChildNode = pBigNode->stpLeftChild;
		pBigParentNode = pNowNode;

		// 1. ������ �ڽ� �� ���� ���� ���� ��� ã��.
		// ���� �ڽ��� NULL�� ���� �� ���� ã�´�.
		while (1)
		{
			// ���� �ڽ��� Nil�̸� ��ġ�� ã�� ��.
			if (pBigChildNode == NULL)
				break;

			pBigParentNode = pBigNode;
			pBigNode = pBigChildNode;
			pBigChildNode = pBigChildNode->stpLeftChild;
		}

		iCaseCheck = 4;
	}

	// ���� ����, ���� �� �ֺ� ��� ����, �׸��� ���뷱��
	int iTempData = pBigNode->iNodeData;
	DeleteSupport(&m_pRootNode, pBigParentNode, pBigNode->iNodeData, iCaseCheck);
}

// ����, ��� ���� ��, ���뷱���ϴ� ����Լ�
void CAVLTree::DeleteSupport(BTreeNode** pNode, BTreeNode* pSubTree, TreeData data, int iCaseCheck)
{
	// 1. data ��ġ�� ã������ ���̽�(iCaseCheck)�� ���� ���� ����.
	// ������ �ǹ�.
	// **pNode�� &(p->Left / p->Right)�̴�.
	// *pNode�� p->Left / p->Right�̴�.
	// pNode�� p�̴�.
	if ((*pNode)->iNodeData == data)
	{
		switch (iCaseCheck)
		{
			// case 1, 2, 3, 4�� Delete�Լ� ����
			case 1:
			{
				BTreeNode* DeleteNode = *pNode;

				// 1. �����Ϸ��� ��尡 ��Ʈ����� ��Ʈ��带 NULL�� ����
				// �׸��� �� ���� ����
				if (DeleteNode == m_pRootNode)
					m_pRootNode = NULL;

				// 2. ��Ʈ��尡 �ƴ϶�� �� �θ� ���� ����Ű�� ������(Left or Right) NULL�� �����.
				else
					*pNode = NULL;

				// 3. ���� ��������.
				free(DeleteNode);
			}
			break;

			case 2:
			case 3:
			{				
				BTreeNode* DeleteNode = *pNode;

				// 1. �� ������ �ڽ��� NULL�� �ƴ϶��, �θ�� �� ������ �ڽ��� ����Ŵ.
				if (DeleteNode->stpRightChild != NULL)
					*pNode = DeleteNode->stpRightChild;

				// 2. �� ������ �ڽ��� NULL�̶��, �θ�� �� ���� �ڽ��� ����Ŵ
				else
					*pNode = DeleteNode->stpLeftChild;				

				// 3. �� ����
				pSubTree->iNodeData = data;

				// 4. ���� ��������.
				free(DeleteNode);				
			}

			break;

			case 4:
			{
				BTreeNode* DeleteNode = *pNode;

				// 1. �� ���� �ڽ��� NULL�� �ƴ϶��, �θ�� �� ���� �ڽ��� ����Ŵ.
				if (DeleteNode->stpLeftChild != NULL)
					*pNode = DeleteNode->stpLeftChild;

				// 2. �� ���� �ڽ��� NULL�̶��, �θ�� �� ������ �ڽ��� ����Ŵ
				else
					*pNode = DeleteNode->stpRightChild;

				// 3. �� ����
				pSubTree->iNodeData = data;				

				// 4. ���� ��������.
				free(DeleteNode);				
			}
			break;

		}

		return;
	}

	// 2. ���� �۴ٸ�, �������� Insert(&���->��) ���.
	else if ((*pNode)->iNodeData > data)
	{
		DeleteSupport( &(*pNode)->stpLeftChild, pSubTree, data, iCaseCheck);

		// �׸��� ���뷱��
		*pNode = (BTreeNode*)Rebalance(*pNode);
	}

	// 3. ���� ũ�ٸ�, ���������� Insert(&���->����) ���.
	else if ((*pNode)->iNodeData < data)
	{
		DeleteSupport( &(*pNode)->stpRightChild, pSubTree, data, iCaseCheck);

		// �׸��� ���뷱��
		*pNode = (BTreeNode*)Rebalance(*pNode);
	}

}

// Main�� Left��� ����
void CAVLTree::ChangeLeftSubTree(BTreeNode* pMain, BTreeNode* pChangeNode)
{
	pMain->stpLeftChild = pChangeNode;
}

// Main�� Right��� ����
void CAVLTree::ChangeRightSubTree(BTreeNode* pMain, BTreeNode* pChangeNode)
{
	pMain->stpRightChild = pChangeNode;
}

// LLȸ�� (��ȸ��)
char* CAVLTree::LLRotation(BTreeNode* pRotateNode)
{
	// pRotateNode�� ���� �ؼ�, ��ĭ ���� �Ʒ��� ��� ���� ȸ��
	BTreeNode* pNode = pRotateNode;
	BTreeNode* pChildNode = pNode->stpLeftChild;

	// 1. ����, �� �ڽ��̳�, �� ���� ��尡 NULL�̶�� ȸ�� �Ұ���.
	if (pNode == NULL || pChildNode == NULL)
		return nullptr;

	// 2. ���� ��������, �� ���� �ڽ���, ������ �ڽ��� ����Ų��.
	ChangeLeftSubTree(pNode, pChildNode->stpRightChild);

	// 3. �� ���� �ڽ��� ����������, ���� ����Ų��.
	ChangeRightSubTree(pChildNode, pNode);

	// 4. ����, ����(pRotateNode)�� ��Ʈ������, ���� �����ڽ��� ���Ӱ� ��Ʈ�� ����
	if (pRotateNode == m_pRootNode)
		m_pRootNode = pChildNode;

	// 5. ȸ�� �� ����� ��Ʈ ����� �ּҰ� ��ȯ
	return (char*)pChildNode;
}

// LRȸ�� (�κ� ��ȸ�� �� -> ��ȸ��)
char* CAVLTree::LRRotation(BTreeNode* pRotateNode)
{
	// �κ� ��ȸ�� �� ��ȸ��.
	// pRotateNode�� �θ�� �ؼ�, ��ĭ ���� �Ʒ��� ��� ���� ȸ��
	BTreeNode* pNode = pRotateNode;
	BTreeNode* pChildNode = pNode->stpLeftChild;

	// 1. ����, �� �ڽ��̳�, �� ���� ��尡 NULL�̶�� ȸ�� �Ұ���.
	if (pNode == NULL || pChildNode == NULL)
		return nullptr;

	// 2. �κ� ��ȸ��.
	ChangeLeftSubTree(pNode, (BTreeNode*)RRRotation(pChildNode));

	// 3. ��ȸ��
	return LLRotation(pNode);	
}

// RRȸ�� (��ȸ��)
char* CAVLTree::RRRotation(BTreeNode* pRotateNode)
{
	// pRotateNode�� �θ�� �ؼ�, ��ĭ ���� �Ʒ��� ��� ���� ȸ��
	BTreeNode* pNode = pRotateNode;
	BTreeNode* pChildNode = pNode->stpRightChild;

	// 1. ����, �� �ڽ��̳�, �� ���� ��尡 NULL�̶�� ȸ�� �Ұ���.
	if (pNode == NULL || pChildNode == NULL)
		return nullptr;

	// 2. ���� ����������, �� ������ �ڽ���, ���� �ڽ��� ����Ų��.
	ChangeRightSubTree(pNode, pChildNode->stpLeftChild);	

	// 3. �� ������ �ڽ��� ��������, ���� ����Ų��.
	ChangeLeftSubTree(pChildNode, pNode);

	// 4. ����, ����(pRotateNode)�� ��Ʈ������, ���� �����ڽ��� ���Ӱ� ��Ʈ�� ����
	if (pRotateNode == m_pRootNode)
		m_pRootNode = pChildNode;

	// 5. ȸ�� �� ����� ��Ʈ ����� �ּҰ� ��ȯ
	return (char*)pChildNode;
}

// RLȸ�� (�κ� ��ȸ�� �� -> ��ȸ��)
char* CAVLTree::RLRotation(BTreeNode* pRotateNode)
{
	// �κ� ��ȸ�� �� ��ȸ��.
	// pRotateNode�� �θ�� �ؼ�, ��ĭ ���� �Ʒ��� ��� ���� ȸ��
	BTreeNode* pNode = pRotateNode;
	BTreeNode* pChildNode = pNode->stpRightChild;

	// 1. ����, �� �ڽ��̳�, �� ���� ��尡 NULL�̶�� ȸ�� �Ұ���.
	if (pNode == NULL || pChildNode == NULL)
		return nullptr;

	// 2. �κ� ��ȸ��.
	ChangeRightSubTree(pNode, (BTreeNode*)LLRotation(pChildNode));

	// 3. ��ȸ��
	return RRRotation(pNode);
}

// ���� ���ϱ�
int CAVLTree::GetHeight(BTreeNode* pHeightNode)
{
	if (pHeightNode == NULL)
		return 0;

	int leftH = GetHeight(pHeightNode->stpLeftChild);
	int rightH = GetHeight(pHeightNode->stpRightChild);

	// ū ���� ���� ��ȯ.
	return 1 + (leftH > rightH ? leftH : rightH);
}

// ���� �μ� ���ϱ�
int CAVLTree::GetHeightDiff(BTreeNode* pDiffNode)
{
	if (pDiffNode == NULL)
		return 0;

	int leftSubH = GetHeight(pDiffNode->stpLeftChild);
	int rightSubH = GetHeight(pDiffNode->stpRightChild);

	// ���� �μ� ��� ��� ��ȯ.
	return leftSubH - rightSubH;
}

// ���뷱�� �ϱ�
char* CAVLTree::Rebalance(BTreeNode* pbalanceNode)
{
	// ���� �μ� ���
	int iDiff = GetHeightDiff(pbalanceNode);

	// ���� �μ��� +2 �̻��̸� LL���� �Ǵ� LR �����̴�.
	if (iDiff > 1)
	{
		// LL���¶��
		if (GetHeightDiff(pbalanceNode->stpLeftChild) > 0)
			pbalanceNode = (BTreeNode*)LLRotation(pbalanceNode);

		// LR���¶��
		else
			pbalanceNode = (BTreeNode*)LRRotation(pbalanceNode);
	}

	// ���� �μ��� -2 ���϶�� RR���� �Ǵ� RL �����̴�.
	if (iDiff < -1)
	{
		// RR���¶��
		if (GetHeightDiff(pbalanceNode->stpRightChild) < 0)
			pbalanceNode = (BTreeNode*)RRRotation(pbalanceNode);

		// RL���¶��
		else
			pbalanceNode = (BTreeNode*)RLRotation(pbalanceNode);
	}

	return (char*)pbalanceNode;
}

// ���� ��ȸ
void CAVLTree::InorderTraversal(BTreeNode* RootNode)
{
	if (RootNode == NULL)
		return;

	InorderTraversal(RootNode->stpLeftChild);

	if(RootNode == m_pRootNode)
		printf("(rt %d) ", RootNode->iNodeData);

	else
		printf("%d ", RootNode->iNodeData);

	InorderTraversal(RootNode->stpRightChild);
}


