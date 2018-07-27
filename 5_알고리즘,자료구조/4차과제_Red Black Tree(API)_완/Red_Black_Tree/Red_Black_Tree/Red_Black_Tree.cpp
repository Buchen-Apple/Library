#include "stdafx.h"
#include <stdio.h>
#include <stdlib.h>
#include "Red_Black_Tree.h"

// ������
CRBT::CRBT()
{
	// �� ��� ����
	m_pNilNode.eNodeColor = BLACK;
	m_pNilNode.stpLeftChild = NULL;
	m_pNilNode.stpRightChild = NULL;
	m_pNilNode.stpParent = NULL;

	// RootNode�� �⺻ NULL�� ����
	m_pRootNode = NULL;
}

// ���� �Լ�
bool CRBT::Insert(BData data)
{
	// RootNode�� NULL�̶��, ���� ��� ����̴�, data�� ��Ʈ��带 �����ϰ� return�Ѵ�.
	if (m_pRootNode == NULL)
	{
		// RootNode ����
		m_pRootNode = (BTreeNode*)malloc(sizeof(BTreeNode));

		m_pRootNode->iNodeData = data;
		m_pRootNode->stpLeftChild = &m_pNilNode;
		m_pRootNode->stpRightChild = &m_pNilNode;
		m_pRootNode->stpParent = NULL;
		m_pRootNode->eNodeColor = BLACK;

		return true;
	}

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
			if (pSerchNode == &m_pNilNode)
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
			if (pSerchNode == &m_pNilNode)
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
	pSerchNode->stpLeftChild = &m_pNilNode;
	pSerchNode->stpRightChild = &m_pNilNode;
	pSerchNode->stpParent = pParentNode;
	pSerchNode->eNodeColor = NODE_COLOR::RED;

	// 5. �θ�� ����
	if (bPostionFalg == false)
		pParentNode->stpLeftChild = pSerchNode;
	else
		pParentNode->stpRightChild = pSerchNode;

	// 6. ���� �߰��� ����� �θ� Red���, ���� �ذ��� ���� �Լ� ȣ��
	if (pSerchNode->stpParent->eNodeColor == RED)
		InsertColorFixup(pSerchNode);

	return true;
}

// ���� �Լ�
bool CRBT::Delete(BData data)
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
		if (pNowNode == &m_pNilNode)
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
		// 1-1-1. ���� �θ��� ���� �ڽ��̶��, �θ��� ������ Nil�� ����.
		if (bPostionFalg == false)
			pParentNode->stpLeftChild = &m_pNilNode;

		// 1-1-2. ���� �θ��� ������ �ڽ��̶��, �θ��� �������� Nil�� ����.
		else
			pParentNode->stpRightChild = &m_pNilNode;

		// �����Ϸ��� ��尡 ��Ʈ����� Ʈ���� ������ �Ҹ�� ��.
		if (pParentNode == m_pRootNode)
			m_pRootNode = NULL;

		// 2. �׸���, ���� ��������
		free(pNowNode);
	}

	// 1-2. �� ���ʿ� ��� �ڽ��� �ִٸ�, �ڽ� �� 1���� �����Ѵ�.
	// ���� ���� : �� ��� ���� �ڽ� �� ���� ���� ū �� or �� ��� ������ �ڽ� �� ���� ���� ���� ��
	// �� �� ����, ���� �ڽ� �� ���� ���� ū ������ �ϱ�� ��.
	else if (pNowNode->stpLeftChild != &m_pNilNode && pNowNode->stpRightChild != &m_pNilNode)
	{
		// ���� ���� ������, pBigNode���� pNowNode�� ���� ��尡 ����ִ�.
		BTreeNode* pBigNode = pNowNode->stpLeftChild;
		BTreeNode* pBigRightNode = pBigNode->stpRightChild;
		BTreeNode* pBigParentNode = pNowNode;

		// 1. ���� �ڽ� �� ���� ���� ū ��� ã��.
		// ������ �ڽ��� NULL�� ���� �� ���� ã�´�.
		while (1)
		{
			// ������ �ڽ��� Nil�̸� ��ġ�� ã�� ��.
			if (pBigRightNode == &m_pNilNode)
				break;

			pBigParentNode = pBigNode;
			pBigNode = pBigRightNode;
			pBigRightNode = pBigRightNode->stpRightChild;
		}

		// 2. pBigParentNode�� pBigNode�� ������ ���´�.
		// 2-1. pBigNode�� �θ��� ���� �ڽ��� ��� (��, �����Ϸ��� ����� ���ʿ� �ڽ��� 1���ۿ� ���� ���)
		if (pBigParentNode->stpLeftChild == pBigNode)
		{
			pBigParentNode->stpLeftChild = &m_pNilNode;
		}

		// 2-2. pBigNode�� �θ��� ������ �ڽ��� ��� (��κ��� ���)
		else if (pBigParentNode->stpRightChild == pBigNode)
		{
			pBigParentNode->stpRightChild = &m_pNilNode;
		}

		// 3. pBigNode���� pNowNode�� ���� ��忡�� ���� ū ���� ����ִ� ��尡 ��ġ�Ѵ�.
		// ���� pBigNode�� ������ pNowNode�� ����. (�����Ϳ� �÷�)
		pNowNode->iNodeData = pBigNode->iNodeData;
		pNowNode->eNodeColor = pBigNode->eNodeColor;

		// 4. �׸���, pBigNode�� ��������
		free(pBigNode);
	}

	// 1-3. �� ���ʿ� �ڽ��� �ִٸ�, �θ�� �� ���� �ڽ��� ����
	else if (pNowNode->stpLeftChild != &m_pNilNode)
	{
		// 1-3-1. ����(�����Ϸ��� ���)�� Root�����.		
		if (pNowNode == m_pRootNode)
		{
			BTreeNode* pNowNodeLeft = pNowNode->stpLeftChild;

			// �� ���� �ڽ��� ���� ��Ʈ��忡 �����ϰ� 
			pNowNode->iNodeData = pNowNodeLeft->iNodeData;
			pNowNode->eNodeColor = pNowNodeLeft->eNodeColor;

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
	else if (pNowNode->stpRightChild != &m_pNilNode)
	{
		// 1-4-1. ����(�����Ϸ��� ���)�� Root�����.		
		if (pNowNode == m_pRootNode)
		{
			BTreeNode* pNowNodeRight = pNowNode->stpRightChild;

			// �� ������ �ڽ��� ���� ��Ʈ��忡 �����ϰ� 
			pNowNode->iNodeData = pNowNodeRight->iNodeData;
			pNowNode->eNodeColor = pNowNodeRight->eNodeColor;

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
void CRBT::Traversal(HDC hdc, RECT rt)
{
	// ��ȸ�ϸ鼭 ���� ����, ���� ��´�.
	// ���� ���� ��ġ�� ��� ���� �ϴ� ��Ʈ�� ��ġ ����.
	int x = rt.right /2;
	int y = rt.top + 30;

	// ���� ��ȸ�ϸ� ����Ʈ�ϱ�.
	preorderTraversal(m_pRootNode, hdc, x, y);
}

// ���� ��ȸ�ϸ� ����Ʈ �ϱ�
void CRBT::preorderTraversal(BTreeNode* pSearchNode, HDC hdc, double x, double y)
{
	// ���� ����. ���� ��尡 ���̰ų� �γ��� ����.
	if (pSearchNode == &m_pNilNode)
		return;

	// 1. �� ������ �� ����.
	HBRUSH MyBrush, OldBrush;
	if (pSearchNode->eNodeColor == BLACK)
	{
		MyBrush = CreateSolidBrush(RGB(0, 0, 0));
		OldBrush = (HBRUSH)SelectObject(hdc, MyBrush);
	}
	else
	{
		MyBrush = CreateSolidBrush(RGB(255, 0, 0));
		OldBrush = (HBRUSH)SelectObject(hdc, MyBrush);
	}

	// 2. ���ڷ� ���� x,y �������� ���� ���� ���
	Ellipse(hdc, x - 20, y - 20, x + 20, y + 20);
	TCHAR string[5];
	_itow_s(pSearchNode->iNodeData, string, _countof(string), 10);
	TextOut(hdc, x - 5, y - 5, string, _tcslen(string));
	SelectObject(hdc, OldBrush);
	DeleteObject(MyBrush);
	
	// 3. ���� �����, ������ ������ �󸶳� ������ üũ
	// �� ���� ��尡 �ҳ�尡 �ƴϰ�, 
	// ���� �����, ������ ��嵵 �ҳ�尡 �ƴ϶�� ����, ������ ����� ���� üũ ����

	// ���� ����� ����.
	int LeftRightDepth = 1;

	// ���� ����� ����
	int LeftRightHeight;
	if (pSearchNode->stpLeftChild != &m_pNilNode && pSearchNode->stpLeftChild->stpRightChild != &m_pNilNode)
	{
		LeftRightHeight = SetHeight(pSearchNode->stpLeftChild->stpRightChild);

		for (int i = 0; i < LeftRightHeight; ++i)
			LeftRightDepth *= 2;
	}	

	// 4. ��(pSearchNode) ���� �������� ���߱�. �� ������ Nil����� �� �ȱߴ´�.
	double dX = x;
	double dY = y + 20;
	if (pSearchNode->stpLeftChild != &m_pNilNode)
	{
		MoveToEx(hdc, x, dY, NULL);

		double yMul50 = 50 / LeftRightDepth;

		for (int i = 0; i < LeftRightDepth; ++i)
		{
			dX -= 50;
			dY += yMul50;
			LineTo(hdc, dX, dY);
		}

	}
	preorderTraversal(pSearchNode->stpLeftChild, hdc, dX, dY);

	// 5. ������ �����, ���� ������ �󸶳� ������ üũ
	// �� ������ ��尡 �ҳ�尡 �ƴϰ�, 
	// ������ �����, ���� ��嵵 �ҳ�尡 �ƴ϶�� ���� üũ ����

	// ���� ����� ����.
	int RightLeftDepth = 1;

	// ���� ����� ����
	int RIghtLeftHeight;
	if (pSearchNode->stpRightChild != &m_pNilNode && pSearchNode->stpRightChild->stpLeftChild != &m_pNilNode)
	{
		RIghtLeftHeight = SetHeight(pSearchNode->stpRightChild->stpLeftChild);

		for (int i = 0; i < RIghtLeftHeight; ++i)
			RightLeftDepth *= 2;
	}

	// 6. ��(pSearchNode) ���� ���������� ���߱�. �� �������� Nil����� �� �ȱߴ´�.
	dX = x;
	dY = y + 20;

	if (pSearchNode->stpRightChild != &m_pNilNode)
	{
		MoveToEx(hdc, x, dY, NULL);

		double yMul50 = 50 / RightLeftDepth;

		for (int i = 0; i < RightLeftDepth; ++i)
		{
			dX += 50;
			dY += yMul50;
			LineTo(hdc, dX, dY);
		}
	}
	
	preorderTraversal(pSearchNode->stpRightChild, hdc, dX, dY);
}

// ��ȸ��
void CRBT::LeftRotation(BTreeNode* pRotateNode)
{
	// ���ڷ� ���޹��� ��带 �������� ��ȸ�� ��Ų��.
	// �� ����, ����, ���� ������ ��尡 Nil ����� ��ȸ�� �Ұ�
	if (pRotateNode->stpRightChild == &m_pNilNode)
		return;

	BTreeNode* pRotateNode_Parent = pRotateNode->stpParent;
	BTreeNode* pRotateNode_Right = pRotateNode->stpRightChild;
	BTreeNode* pRotateNode_Righ_Left = pRotateNode_Right->stpLeftChild;

	// 1. pRotateNode�� ����������, pRotateNode�� �������� ���� �ڽ��� ����Ŵ.
	pRotateNode->stpRightChild = pRotateNode_Righ_Left;

	// 2. pRotateNode�� �������� ���� �ڽ��� �θ��, pRotateNode�� ����Ŵ
	pRotateNode_Righ_Left->stpParent = pRotateNode;

	// 3. pRotateNode�� ������ �ڽ��� ��������, pRotateNode�� ����Ŵ
	pRotateNode_Right->stpLeftChild = pRotateNode;

	// 4. pRotateNode�� ������ �ڽ��� �θ��, pRotateNode�� �θ� ��Ű��
	pRotateNode_Right->stpParent = pRotateNode->stpParent;

	// 5. �θ� NULL�� �ƴ϶��, �θ�� pRotateNode�� �ڽ� �� ���� ����
	if (pRotateNode_Parent != NULL)
	{
		// 5-1. ����, pRotateNode�� �θ��� ���� �ڽ��̸�, pRotateNode�� �θ� ��������, pRotateNode�� ������ �ڽ��� ����Ŵ
		if (pRotateNode == pRotateNode_Parent->stpLeftChild)
			pRotateNode_Parent->stpLeftChild = pRotateNode_Right;

		// 5-2. ����, pRotateNode�� �θ��� ������ �ڽ��̸�, pRotateNode�� �θ� ����������, pRotateNode�� ������ �ڽ��� ����Ŵ
		else if (pRotateNode == pRotateNode_Parent->stpRightChild)
			pRotateNode_Parent->stpRightChild = pRotateNode_Right;
	}

	// 6. �θ� NULL�̶��, ȸ���ϴ� ��尡 ��Ʈ����°�. �� ����, ��Ʈ ��带 �ٽ� �����ؾ��Ѵ�. ȸ������ ���� ��Ʈ��尡 �����
	else
		m_pRootNode = pRotateNode_Right;

	// 7. pRotateNode�� �θ��,  pRotateNode�� ������ �ڽ��� ��Ű��
	pRotateNode->stpParent = pRotateNode_Right;

}

// ��ȸ��
void CRBT::RightRotation(BTreeNode* pRotateNode)
{
	// ���ڷ� ���޹��� ��带 �������� ��ȸ�� ��Ų��.
	// �� ����, ����, ���� ���� ��尡 Nil ����� ��ȸ�� �Ұ�
	if (pRotateNode->stpLeftChild == &m_pNilNode)
		return;

	BTreeNode* pRotateNode_Parent = pRotateNode->stpParent;
	BTreeNode* pRotateNode_Left = pRotateNode->stpLeftChild;
	BTreeNode* pRotateNode_Left_Right = pRotateNode_Left->stpRightChild;

	// 1. pRotateNode�� ��������, pRotateNode�� ������ ������ �ڽ��� ��Ű��.
	pRotateNode->stpLeftChild = pRotateNode_Left_Right;

	// 2. pRotateNode�� ������ ������ �ڽ��� �θ��, pRotateNode�� ��Ű��
	pRotateNode_Left_Right->stpParent = pRotateNode;

	// 3. pRotateNode�� ���� �ڽ��� ����������, pRotateNode�� ��Ű��
	pRotateNode_Left->stpRightChild = pRotateNode;

	// 4. pRotateNode�� ���� �ڽ��� �θ��, pRotateNode�� �θ� ��Ű��
	pRotateNode_Left->stpParent = pRotateNode->stpParent;

	// 5. �θ� NULL�� �ƴ϶��, �θ�� pRotateNode�� �ڽ� �� ���� ����
	if (pRotateNode_Parent != NULL)
	{
		// 5-1. ����, pRotateNode�� �θ��� ���� �ڽ��̸�, pRotateNode�� �θ� ��������, pRotateNode�� ���� �ڽ��� ����Ŵ
		if (pRotateNode == pRotateNode_Parent->stpLeftChild)
			pRotateNode_Parent->stpLeftChild = pRotateNode_Left;

		// 5-2. ����, pRotateNode�� �θ��� ���� �ڽ��̸�, pRotateNode�� �θ� ����������, pRotateNode�� ���� �ڽ��� ����Ŵ
		else if (pRotateNode == pRotateNode_Parent->stpRightChild)
			pRotateNode_Parent->stpRightChild = pRotateNode_Left;
	}

	// 6. �θ� NULL�̶��, ȸ���ϴ� ��尡 ��Ʈ����°�. �� ����, ��Ʈ ��带 �ٽ� �����ؾ��Ѵ�. ȸ������ ���� ��Ʈ��尡 �����
	else
		m_pRootNode = pRotateNode_Left;	

	// 7. pRotateNode�� �θ��,  pRotateNode�� ���� �ڽ��� ��Ű��
	pRotateNode->stpParent = pRotateNode_Left;

}

// Insert �� �� ���߱� ó��
void CRBT::InsertColorFixup(BTreeNode* pZNode)
{
	// Insert �� �ش� �Լ��� ȣ��Ǿ�����, pZNode�� �θ� Red�� ����.
	// ���̽��� ���� ó���Ѵ�.
	
	while (1)
	{
		// �������� 1. pZNode�� ��Ʈ ��尡 �� ��� while�� ����.
		if (pZNode == m_pRootNode)
			break;

		// �������� 1. pZNode�� �θ� Red�� �ƴ϶�� ����.
		if (pZNode->stpParent->eNodeColor != RED)
			break;

		// ���̽� 1,2,3��, �� �θ� �Ҿƹ����� ���� �ڽ��� ���.
		if (pZNode->stpParent == pZNode->stpParent->stpParent->stpLeftChild)
		{
			// �� �θ� ����
			BTreeNode* pZNode_stpParent = pZNode->stpParent;

			// �Ҿƹ��� ����
			BTreeNode* pZNode_grandpa = pZNode_stpParent->stpParent;

			// ������, �� �Ҿƹ����� ������ �ڽ��̴�.
			BTreeNode* pZNode_uncle = pZNode_grandpa->stpRightChild;

			// case 1. pZNode�� ������ Red�� ���.
			// �θ�� ������ black���� ����. �׸��� �Ҿƹ����� Red�� ������ ��, �Ҿƹ����� zNode�� �����Ѵ�.
			// ���� ���� �ذ� �ȵ�. �ٽ� while���� ���鼭 ���̽� üũ
			if (pZNode_uncle->eNodeColor == RED)
			{
				pZNode_stpParent->eNodeColor = BLACK;
				pZNode_uncle->eNodeColor = BLACK;

				pZNode_grandpa->eNodeColor = RED;
				pZNode = pZNode_grandpa;
			}

			// pZNode�� ������ Black�� ���
			else if (pZNode_uncle->eNodeColor == BLACK)
			{
				// case 2. ������ Black�̸鼭, ����(pZNode) �θ��� ������ �ڽ��� ���
				// �� �θ� �������� Left�����̼�. �׸��� �θ� zNode�� �����Ѵ�. �׷��� �ٷ� Case3�̵ȴ�.
				if (pZNode == pZNode_stpParent->stpRightChild)
				{
					LeftRotation(pZNode_stpParent);
					pZNode = pZNode_stpParent;
				}					

				// case 3. ������ Black�̸鼭, ����(pZNode) �θ��� ���� �ڽ��� ���
				// �θ� Black, �Ҿƹ����� Red�� ����.
				// �Ҿƹ����� �������� Right�����̼�. �׷��� ���� �ذ�!
				pZNode->stpParent->eNodeColor = BLACK;
				pZNode->stpParent->stpParent->eNodeColor = RED;
				RightRotation(pZNode->stpParent->stpParent);

				break;
			}

		}

		// ���̽� 4,5,6��, �� �θ� �Ҿƹ����� ������ �ڽ��� ���.
		else if (pZNode->stpParent == pZNode->stpParent->stpParent->stpRightChild)
		{
			// �� �θ� ����
			BTreeNode* pZNode_stpParent = pZNode->stpParent;

			// �Ҿƹ��� ����
			BTreeNode* pZNode_grandpa = pZNode_stpParent->stpParent;

			// ������, �� �Ҿƹ����� ���� �ڽ��̴�.
			BTreeNode* pZNode_uncle = pZNode_grandpa->stpLeftChild;

			// case 4. pZNode�� ������ Red�� ���.
			// �θ�� ������ black���� ����. �׸��� �Ҿƹ����� Red�� ������ ��, �Ҿƹ����� zNode�� �����Ѵ�.
			// ���� ���� �ذ� �ȵ�. �ٽ� while���� ���鼭 ���̽� üũ
			if (pZNode_uncle->eNodeColor == RED)
			{
				pZNode_stpParent->eNodeColor = BLACK;
				pZNode_uncle->eNodeColor = BLACK;

				pZNode_grandpa->eNodeColor = RED;
				pZNode = pZNode_grandpa;
			}

			// pZNode�� ������ Black�� ���
			else if (pZNode_uncle->eNodeColor == BLACK)
			{
				// case 5. ������ Black�̸鼭, ����(pZNode) �θ��� ���� �ڽ��� ���
				// �� �θ� �������� Right�����̼�. �׸��� �θ� zNode�� �����Ѵ�. �׷��� �ٷ� Case3�̵ȴ�.
				if (pZNode == pZNode_stpParent->stpLeftChild)
				{
					RightRotation(pZNode_stpParent);
					pZNode = pZNode_stpParent;
				}

				// case 6. ������ Black�̸鼭, ����(pZNode) �θ��� ������ �ڽ��� ���
				// �θ� Black, �Ҿƹ����� Red�� ����.
				// �Ҿƹ����� �������� LEft�����̼�. �׷��� ���� �ذ�!
				pZNode->stpParent->eNodeColor = BLACK;
				pZNode->stpParent->stpParent->eNodeColor = RED;
				LeftRotation(pZNode->stpParent->stpParent);

				break;
			}
		}
	}

	// ���̽� ó�� ��, ������ ��Ʈ��带 BLACK���� ����.
	m_pRootNode->eNodeColor = BLACK;
}

// ���� ���ϴ� �Լ�
int  CRBT::SetHeight(BTreeNode* pHeightNode)
{
	if (pHeightNode == &m_pNilNode)
		return 0;

	else
	{
		int left_h = SetHeight(pHeightNode->stpLeftChild);
		int right_h = SetHeight(pHeightNode->stpRightChild);
		return 1 + (left_h > right_h ? left_h : right_h);
	}
	
}