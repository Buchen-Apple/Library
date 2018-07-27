#include "stdafx.h"
#include "A_Search.h"
#include <math.h>

// ������
CFindSearch::CFindSearch(Map (*Copy)[MAP_WIDTH])
{	
	StartNode = nullptr;
	FinNode = nullptr;

	// ���� �纻�� �����Ѵ�.
	stMapArrayCopy = Copy;
}

// �Ҹ���
CFindSearch::~CFindSearch()
{
	// �ϴ°� ����.
		
}

// ������, ������, MemDC �����ϱ�
void CFindSearch::Init(int StartX, int StartY, int FinX, int FinY, HWND hWnd)
{
	this->hWnd = hWnd;

	StartNode = new stFindSearchNode[sizeof(stFindSearchNode)];
	FinNode = new stFindSearchNode[sizeof(stFindSearchNode)];

	// x,y��ǥ ����
	StartNode->m_iX = StartX;
	StartNode->m_iY = StartY;
	FinNode->m_iX = FinX;
	FinNode->m_iY = FinY;

	// �������� G,H,F ����
	StartNode->m_fG = 0;
	StartNode->m_fH = fabs((FinX - StartX) + fabs(FinY - StartY));
	StartNode->m_fF = StartNode->m_fG + StartNode->m_fH;
	StartNode->stpParent = nullptr;

	// �������� G,H,F ����
	FinNode->m_fG = 0;
	FinNode->m_fH = 0;
	FinNode->m_fF = 0;
	FinNode->stpParent = nullptr;
}

// �ݺ��� ���鼭 ���̽�Ÿ ����
void CFindSearch::A_Search_While()
{
	// 1. ���� ��, �������� ���¸���Ʈ�� �ִ´�.
	m_OpenList.Insert(StartNode);

	// 2. �׸��� �ݺ� ����
	// ���� ����Ʈ�� �� �� ���� �ݺ�.
	while (m_OpenList.GetCount() != 0)
	{
		// 2-1. ���� ����Ʈ�� ������� �ʴٸ�, ���� ����Ʈ���� ��� �ϳ� ����.
		stFindSearchNode* Node = m_OpenList.GetListNode();

		// 2-2. ���� ��尡 ��������� break;
		if (Node->m_iX == FinNode->m_iX &&
			Node->m_iY == FinNode->m_iY)
		{
			// Node�� �������̴�, ������� �θ� Ÿ���鼭 ���� �ߴ´�.
			Show(Node);			
			break;
		}

		// 2-3. ������ ��尡 �ƴϸ� ���� ��带 CloseList�� �ֱ�.
		m_CloseList.Insert(Node);

		if(stMapArrayCopy[Node->m_iY][Node->m_iX].m_eTileType == OPEN_LIST)
			stMapArrayCopy[Node->m_iY][Node->m_iX].m_eTileType = CLOSE_LIST;

		// 2-4. �׸��� ��� �����Լ� 8�� ȣ��(�����¿�,�밢������ �� 8��)
		// ��
		CreateNode(Node->m_iX, Node->m_iY - 1, Node);

		// ��
		CreateNode(Node->m_iX, Node->m_iY + 1, Node);

		// ��
		CreateNode(Node->m_iX - 1, Node->m_iY, Node);

		// ��
		CreateNode(Node->m_iX + 1, Node->m_iY, Node);


		// �»�
		CreateNode(Node->m_iX - 1, Node->m_iY - 1, Node, 1.0);

		// ����
		CreateNode(Node->m_iX - 1, Node->m_iY + 1, Node, 1.0);

		// ���
		CreateNode(Node->m_iX + 1, Node->m_iY - 1, Node, 1.0);

		// ����
		CreateNode(Node->m_iX + 1, Node->m_iY + 1, Node, 1.0);
	}

	// �� ������, Open����Ʈ�� Close����Ʈ �ʱ�ȭ
	m_CloseList.Clear();
	m_OpenList.Clear();

	// FinNode�� �Ҹ� �ȵ����� �Ҹ��Ų��.
	delete[] FinNode;

}

// ��� ���� �Լ�
void CFindSearch::CreateNode(int iX, int iY, stFindSearchNode* parent, double fWeight)
{
	// 1. �ش� ��ǥ�� ���� ��� ���� ���� ��ǥ���� üũ (�� ������, ���ع�����) 
	if (stMapArrayCopy[iY][iX].m_eTileType == OBSTACLE ||
		iX < 0 || iX >= MAP_WIDTH || iY < 0 || iY >= MAP_HEIGHT)
		return;

	// 2. ���� ������ ��ġ���, �ش� ��ǥ�� ��尡 Open ����Ʈ�� �ִ��� üũ
	stFindSearchNode* SearchNode = m_OpenList.Search(iX, iY);
	if (SearchNode != nullptr)
	{
		// �ִٸ�, �ش� ����� G���� ���� ������ ����� G�� üũ.
		// �ش� ����� G���� �� ũ�ٸ�, 
		if (SearchNode->m_fG > (parent->m_fG + 1 + fWeight))
		{
			// parent�� �θ� ����
			SearchNode->stpParent = parent;

			// G�� �����(parent�� G�� + 1)
			SearchNode->m_fG = parent->m_fG + 1 + fWeight;

			// F�� �����.
			SearchNode->m_fF = (SearchNode->m_fG + SearchNode->m_fH);

			// ���ĵ� �ٽ��Ѵ�.
			m_OpenList.Sort();
		}

		return;
	}

	// 3. Close�� ����Ʈ�� �ִ����� üũ
	SearchNode = m_CloseList.Search(iX, iY);
	if (SearchNode != nullptr)
	{
		// �ִٸ�, �ش� ����� G���� ���� ������ ����� G�� üũ.
		// �ش� ����� G���� �� ũ�ٸ�, 
		if (SearchNode->m_fG > (parent->m_fG + 1 + fWeight))
		{
			// parent�� �θ� ����
			SearchNode->stpParent = parent;

			// G�� �����(parent�� G�� + 1)
			SearchNode->m_fG = parent->m_fG + 1 + fWeight;

			// F�� �����.
			SearchNode->m_fF = (SearchNode->m_fG + SearchNode->m_fH);

			// ���ĵ� �ٽ��Ѵ�.
			m_OpenList.Sort();
		}

		return;
	}

	// 4. �� �ٿ� ������, ��� ���� ��, Open�� �ִ´�.
	stFindSearchNode* NewNode = new stFindSearchNode[sizeof(stFindSearchNode)];

	NewNode->stpParent = parent;

	NewNode->m_iX = iX;
	NewNode->m_iY = iY;

	NewNode->m_fG = parent->m_fG + 1 + fWeight;
	NewNode->m_fH = fabs(FinNode->m_iX - iX) + fabs(FinNode->m_iY - iY);
	NewNode->m_fF = NewNode->m_fG + NewNode->m_fH;

	if(stMapArrayCopy[iY][iX].m_eTileType == NONE)
		stMapArrayCopy[iY][iX].m_eTileType = OPEN_LIST;

	m_OpenList.Insert(NewNode);	

	// 5. ���ĵ� �ٽ��Ѵ�.
	m_OpenList.Sort();

	// 6. ����Ѵ�.
	Show();

}

// �׸���
void CFindSearch::Show(stFindSearchNode* Node)
{
	HDC hdc = GetDC(hWnd);
	HDC MemDC = CreateCompatibleDC(hdc);
	RECT rt;
	GetClientRect(hWnd, &rt);

	HBITMAP hMyBitmap = CreateCompatibleBitmap(hdc, rt.right, rt.bottom);
	HBITMAP hOldBitmap = (HBITMAP)SelectObject(MemDC, hMyBitmap);

	FillRect(MemDC, &rt, (HBRUSH)GetStockObject(WHITE_BRUSH));

	// TODO: ���⿡ hdc�� ����ϴ� �׸��� �ڵ带 �߰��մϴ�.
	int iRadius = MAP_TILE_RADIUS;

	for (int i = 0; i < MAP_HEIGHT; ++i)
	{
		for (int j = 0; j < MAP_WIDTH; ++j)
		{
			HBRUSH hMyBrush = NULL;
			HBRUSH hOldBrush = NULL;
			bool bBrushCheck = false;

			// �ش� �� Ÿ���� ���¿� ���� ���� �����Ѵ�.
			// ������� �ʷϻ�
			if (stMapArrayCopy[i][j].m_eTileType == START)
			{
				hMyBrush = CreateSolidBrush(RGB(0, 255, 0));
				hOldBrush = (HBRUSH)SelectObject(MemDC, hMyBrush);
				bBrushCheck = true;
			}

			// �������� ������
			else if (stMapArrayCopy[i][j].m_eTileType == FIN)
			{
				hMyBrush = CreateSolidBrush(RGB(255, 0, 0));
				hOldBrush = (HBRUSH)SelectObject(MemDC, hMyBrush);
				bBrushCheck = true;
			}

			// ���ع��� ȸ��
			else if (stMapArrayCopy[i][j].m_eTileType == OBSTACLE)
			
			{
				hMyBrush = CreateSolidBrush(RGB(125, 125, 125));
				hOldBrush = (HBRUSH)SelectObject(MemDC, hMyBrush);
				bBrushCheck = true;
			}

			// OPEN_LIST�� �Ķ���
			else if (stMapArrayCopy[i][j].m_eTileType == OPEN_LIST)
			{
				hMyBrush = CreateSolidBrush(RGB(0, 0, 255));
				hOldBrush = (HBRUSH)SelectObject(MemDC, hMyBrush);
				bBrushCheck = true;
			}

			// CLOSE_LIST�� ���� �Ķ���
			else if (stMapArrayCopy[i][j].m_eTileType == CLOSE_LIST)
			{
				hMyBrush = CreateSolidBrush(RGB(0, 0, 127));
				hOldBrush = (HBRUSH)SelectObject(MemDC, hMyBrush);
				bBrushCheck = true;
			}

			Rectangle(MemDC, stMapArrayCopy[i][j].m_iMapX - iRadius, stMapArrayCopy[i][j].m_iMapY - iRadius, stMapArrayCopy[i][j].m_iMapX + iRadius, stMapArrayCopy[i][j].m_iMapY + iRadius);

			if (bBrushCheck == true)
			{
				SelectObject(MemDC, hOldBrush);
				DeleteObject(hMyBrush);
				DeleteObject(hOldBrush);
			}
		}
	}

	// ���߱�
	if(Node != nullptr)
	{
		MoveToEx(MemDC, stMapArrayCopy[Node->m_iY][Node->m_iX].m_iMapX,
			stMapArrayCopy[Node->m_iY][Node->m_iX].m_iMapY, NULL);

		HPEN hMyPen = CreatePen(PS_SOLID, 1, RGB(255, 255, 0));
		HPEN OldPen = (HPEN)SelectObject(MemDC, hMyPen);

		stFindSearchNode* ParentNode = Node->stpParent;
		while (1)
		{
			if (ParentNode->m_iX == StartNode->m_iX && ParentNode->m_iY == StartNode->m_iY)
			{
				LineTo(MemDC, stMapArrayCopy[ParentNode->m_iY][ParentNode->m_iX].m_iMapX,
					stMapArrayCopy[ParentNode->m_iY][ParentNode->m_iX].m_iMapY);
				break;
			}

			LineTo(MemDC, stMapArrayCopy[ParentNode->m_iY][ParentNode->m_iX].m_iMapX,
				stMapArrayCopy[ParentNode->m_iY][ParentNode->m_iX].m_iMapY);

			ParentNode = ParentNode->stpParent;
		}

		DeleteObject(hMyPen);
		DeleteObject(OldPen);
	}

	BitBlt(hdc, rt.left, rt.top, rt.right, rt.bottom, MemDC, rt.left, rt.top, SRCCOPY);

	DeleteObject(hMyBitmap);
	DeleteObject(hOldBitmap);
	DeleteDC(MemDC);
}