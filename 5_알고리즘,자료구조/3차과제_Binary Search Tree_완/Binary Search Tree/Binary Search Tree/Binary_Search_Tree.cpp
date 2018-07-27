#include <stdio.h>
#include <stdlib.h>
#include "Binary_Search_Tree.h"

// ������
CBST::CBST(BData data)
{
	m_pRootNode = (BTreeNode*)malloc(sizeof(BTreeNode));

	m_pRootNode->iNodeData = data;
	m_pRootNode->stpLeftChild = NULL;
	m_pRootNode->stpRightChild = NULL;
}

// ���� �Լ�
bool CBST::Insert(BData data)
{
	BTreeNode* pSerchNode = m_pRootNode;
	BTreeNode* pParentNode;
	bool bPostionFalg = false;
	// bPostionFalg == false : ���� �ڽĿ� ����
	// bPostionFalg == true	 : ������ �ڽĿ� ����

	// �� ��ġ ã��
	while (1)
	{
		// 1. pParentNode�� pSerchNode�� �־ �θ� ��ġ ����.
		pParentNode = pSerchNode;

		// 2-1. [�������� data < �θ��� data]���, pSerchNode�� �θ��� ���� �ڽ� ��ġ�� pSerchNode�� ����
		if (pParentNode->iNodeData > data)
		{
			pSerchNode = pParentNode->stpLeftChild;
			// 3. �ڽ��� NULL�̶�� �� ��ġ�� ã�� ��
			if (pSerchNode == NULL)
			{
				// bPostionFalg�� false�� �ϰ� ������.
				bPostionFalg = false;
				break;
			}
		}

		// 2-2. [�������� data > �θ��� data]���, pSerchNode�� �θ��� ������ �ڽ� ��ġ�� pSerchNode�� ����
		else if (pParentNode->iNodeData < data)
		{
			pSerchNode = pParentNode->stpRightChild;

			// 3. �ڽ��� NULL�̶�� �� ��ġ�� ã�� ��
			if (pSerchNode == NULL)
			{
				// bPostionFalg�� true�� �ϰ� ������.
				bPostionFalg = true;
				break;
			}
		}

		// 2-3. [�θ��� data == �������� data]���, Ʈ���� ������.
		else
		{
			printf("Insert(). ���� / �ߺ��� �������Դϴ�. �õ��� �� : %d\n", data);
			return false;
		}
	}

	// 4. ��ġ�� ã������, ������ ����
	pSerchNode = (BTreeNode*)malloc(sizeof(BTreeNode));
	pSerchNode->iNodeData = data;
	pSerchNode->stpLeftChild = NULL;
	pSerchNode->stpRightChild = NULL;

	// 5. �θ�� ����
	if (bPostionFalg == false)
		pParentNode->stpLeftChild = pSerchNode;
	else
		pParentNode->stpRightChild = pSerchNode;

	return true;
}

// ���� �Լ�
bool CBST::Delete(BData data)
{	
	BTreeNode* pNowNode = m_pRootNode;
	BTreeNode* pParentNode;
	bool bPostionFalg = false;
	// bPostionFalg == false : pNowNode�� pParentNode�� ���� �ڽ��̴�.
	// bPostionFalg == true	 :  pNowNode�� pParentNode�� ������ �ڽ��̴�.

	// �����Ϸ��� ��� ã��.
	while (1)	
	{	
		// 1. �θ� ��� ����
		pParentNode = pNowNode;

		// 2. �� ��ġ ����
		// 2-1. [�θ� ��� data > ã������ data]���, ���� �θ��� ���� ��尡 �ȴ�.
		if (pParentNode->iNodeData > data)
		{
			pNowNode = pParentNode->stpLeftChild;
			bPostionFalg = false;
		}

		// 2-2. [�θ� ��� data < ã������ data]���, ���� �θ��� ������ ��尡 �ȴ�.
		else if (pParentNode->iNodeData < data)
		{
			pNowNode = pParentNode->stpRightChild;
			bPostionFalg = true;
		}

		// 2-3. [�θ� ��� data == ã������ data]���. �� ����, ��Ʈ��带 �����ϴ� ��츸 �ش�.
		else
			break;

		// ���� ����. pNowNode�� NULL�̸�, ���� ���(�ٻ�� ���)���� ��� �˻��ߴµ��� ���ϴ� ���� ���� ��. 
		if (pNowNode == NULL)
			return false;


		// 3. ���� ���� �����Ǿ�� �ϴ� ������� üũ
		else if (pNowNode->iNodeData == data)
			break;
	}

	// ������ ��带 ã��. (pNowNode)
	// ���� ���� ���� ����	

	// 1-1. �� �ڽ��� 0���̶��, �׳� ����
	if (pNowNode->stpLeftChild == NULL && pNowNode->stpRightChild == NULL)
	{
		// 1-1-1. ���� �θ��� ���� �ڽ��̶��, �θ��� ������ NULL�� ����.
		if (bPostionFalg == false)
			pParentNode->stpLeftChild = NULL;

		// 1-1-2. ���� �θ��� ������ �ڽ��̶��, �θ��� �������� NULL�� ����.
		else
			pParentNode->stpRightChild = NULL;

		// �� �ڽ��� 0���ε�, ������ ��尡 ��Ʈ����� Ʈ���� ������ �Ҹ�� ��.
		if (pParentNode == m_pRootNode)
			m_pRootNode->iNodeData = NULL;

		// 2. �׸���, ���� ��������
		free(pNowNode);
	}

	// 1-2. �� ���ʿ� ��� �ڽ��� �ִٸ�, �ڽ� �� 1���� �����Ѵ�.
	// ���� ���� : �� ��� ���� �ڽ� �� ���� ���� ū �� or �� ��� ������ �ڽ� �� ���� ���� ���� ��
	// �� �� ����, ���� �ڽ� �� ���� ���� ū ������ �ϱ�� ��.
	else if (pNowNode->stpLeftChild != NULL && pNowNode->stpRightChild != NULL)
	{
		// ���� ���� ������, pBigNode���� pNowNode�� ���� ��尡 ����ִ�.
		BTreeNode* pBigNode = pNowNode->stpLeftChild;
		BTreeNode* pBigRightNode = pBigNode->stpRightChild;
		BTreeNode* pBigParentNode = pNowNode;

		// 1. ���� �ڽ� �� ���� ���� ū ��� ã��.
		// ������ �ڽ��� NULL�� ���� �� ���� ã�´�.
		while (1)
		{
			// ������ �ڽ��� NULL�̸� ��ġ�� ã�� ��.
			if (pBigRightNode == NULL)
				break;

			pBigParentNode = pBigNode;
			pBigNode = pBigRightNode;
			pBigRightNode = pBigRightNode->stpRightChild;
		}

		// 2. pBigParentNode�� pBigNode�� ������ ���´�.
		// 2-1. pBigNode�� �θ��� ���� �ڽ��� ��� (��, �����Ϸ��� ����� ���ʿ� �ڽ��� 1���ۿ� ���� ���)
		if (pBigParentNode->stpLeftChild == pBigNode)
		{
			pBigParentNode->stpLeftChild = NULL;
		}

		// 2-2. pBigNode�� �θ��� ������ �ڽ��� ��� (��κ��� ���)
		else if (pBigParentNode->stpRightChild == pBigNode)
		{
			pBigParentNode->stpRightChild = NULL;
		}

		// 3. pBigNode���� pNowNode�� ���� ��忡�� ���� ū ���� ����ִ� ��尡 ��ġ�Ѵ�.
		// ���� pBigNode�� ���� pNowNode�� ������ ����.
		pNowNode->iNodeData = pBigNode->iNodeData;

		// 4. �׸���, pBigNode�� ��������
		free(pBigNode);
	}

	// 1-3. �� ���ʿ� �ڽ��� �ִٸ�, �θ�� �� ���� �ڽ��� ����
	else if (pNowNode->stpLeftChild != NULL)
	{	
		// 1-3-1. ����(�����Ϸ��� ���)�� Root�����.		
		if (pNowNode == m_pRootNode)
		{
			BTreeNode* pNowNodeLeft = pNowNode->stpLeftChild;

			// �� ���� �ڽ��� ���� ��Ʈ��忡 �����ϰ� 
			pNowNode->iNodeData = pNowNodeLeft->iNodeData;

			// ��Ʈ����� ������, ���� �ڽ��� ���� �ڽ��� ����Ų��.
			pNowNode->stpLeftChild = pNowNodeLeft->stpLeftChild;

			// 2. �׸���, ��Ʈ����� �ٷ� ���� ��带 ��������
			free(pNowNodeLeft);
		}

		// 1-3-2. ���� �θ��� ���� �ڽ��̶��, �θ��� ���ʿ� �� ���� �ڽ� ����
		else if (bPostionFalg == false)
		{
			pParentNode->stpLeftChild = pNowNode->stpLeftChild;

			// 2. �׸���, ���� ��������
			free(pNowNode);
		}

		// 1-3-3. ���� �θ��� ������ �ڽ��̶��, �θ��� �����ʿ� �� ���� �ڽ� ����
		else if (bPostionFalg == true)
		{
			pParentNode->stpRightChild = pNowNode->stpLeftChild;

			// 2. �׸���, ���� ��������
			free(pNowNode);
		}
	}

	// 1-4. �� �����ʿ� �ڽ��� �ִٸ�, �θ�� �� ������ �ڽ��� ����
	else if (pNowNode->stpRightChild != NULL)
	{
		// 1-4-1. ����(�����Ϸ��� ���)�� Root�����.		
		if (pNowNode == m_pRootNode)
		{
			BTreeNode* pNowNodeRight = pNowNode->stpRightChild;

			// �� ������ �ڽ��� ���� ��Ʈ��忡 �����ϰ� 
			pNowNode->iNodeData = pNowNodeRight->iNodeData;

			// ��Ʈ����� ��������, ������ �ڽ��� ������ �ڽ��� ����Ų��.
			pNowNode->stpRightChild = pNowNodeRight->stpRightChild;

			// 2. �׸���, ��Ʈ����� �ٷ� ������ ��带 ��������
			free(pNowNodeRight);
		}

		// 1-4-1. ���� �θ��� ���� �ڽ��̶��, �θ��� ���ʿ� �� ������ �ڽ� ����
		else if (bPostionFalg == false)
			pParentNode->stpLeftChild = pNowNode->stpRightChild;

		// 1-4-2. ���� �θ��� ������ �ڽ��̶��, �θ��� �����ʿ� �� ������ �ڽ� ����
		else if (bPostionFalg == true)
			pParentNode->stpRightChild = pNowNode->stpRightChild;

		// 2. �׸���, ���� ��������
		free(pNowNode);
	}

	return true;
}

// ��ȸ �Լ�
void CBST::Traversal()
{
	// ���� ��ȸ
	InorderTraversal(m_pRootNode);

}

// ���� �˻�
void CBST::InorderTraversal(BTreeNode* pSearchNode)
{
	if (pSearchNode == NULL)
		return;

	InorderTraversal(pSearchNode->stpLeftChild);
	printf("%d ", pSearchNode->iNodeData);
	InorderTraversal(pSearchNode->stpRightChild);
}

// ��Ʈ ����� �� ��ȯ
BData CBST::GetRootNodeData()
{
	return m_pRootNode->iNodeData;
}