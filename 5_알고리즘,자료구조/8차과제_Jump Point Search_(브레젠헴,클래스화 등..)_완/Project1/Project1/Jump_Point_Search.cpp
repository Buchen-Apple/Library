#include "stdafx.h"
#include "Jump_Point_Search.h"
#include "Bresenham_Line.h"
#include <math.h>
#include <time.h>

#define CheckTile(X, Y)		\
	((&stMapArrayCopy[Y * m_iWidth] + X)->m_eTileType == OBSTACLE || X < 0 || X >= m_iWidth || Y < 0 || Y >= m_iHeight || iTempMaxSearchCount >= m_iMaxSearchCount)

// ������
CFindSearch::CFindSearch(Map* Copy, int Width, int Height, int Radius, int MaxSearchCount)
{
	// ���� ����ġ, �밢�� ����ġ �� ����
	m_dStraight_Line = 1;
	m_dDiagonal_Line = 2;

	m_iWidth = Width;
	m_iHeight = Height;
	m_iRadius = Radius;
	m_iMaxSearchCount = MaxSearchCount;

	StartNode = nullptr;
	FinNode = nullptr;

	// ���� �纻�� �����Ѵ�.
	stMapArrayCopy = Copy;

	// Jump��忡 ĥ�� �� �̸� ����
	Color1 = RGB(232, 217, 255);	// ���� �迭
	Color2 = RGB(250, 244, 192);	// ��� �迭
	Color3 = RGB(234, 234, 234);	// ȸ�� �迭
	Color4 = RGB(255, 180, 180);	// ���� �迭
	Color5 = RGB(119, 255, 112);	// �ʷ� �迭
	Color6 = RGB(188, 229, 92);		// �ʷ� �迭2
	Color7 = RGB(255, 178, 245);	// ��ȫ �迭
	Color8 = RGB(255, 166, 72);		// ��ȫ �迭
}

// �Ҹ���
CFindSearch::~CFindSearch()
{
	// �ϴ°� ����.
	DeleteObject(m_hMyBitmap);
	DeleteObject(m_hOldBitmap);
	DeleteDC(m_hMemDC);
}

// ������ �ڵ��� ���޹ް� MemDC���� �����ϴ� �Լ�
void CFindSearch::DCSet(HWND hWnd)
{
	// ������ �̹� ȣ��� ���� ������, ������ �Ҵ�Ǿ��� MemDC ���� Delete�����ش�.
	if (m_hWnd != NULL)
	{
		DeleteObject(m_hMyBitmap);
		DeleteObject(m_hOldBitmap);
		DeleteDC(m_hMemDC);
	}

	m_hWnd = hWnd;

	HDC hdc = GetDC(m_hWnd);
	m_hMemDC = CreateCompatibleDC(hdc);
	GetClientRect(m_hWnd, &rt);

	m_hMyBitmap = CreateCompatibleBitmap(hdc, rt.right, rt.bottom);
	m_hOldBitmap = (HBITMAP)SelectObject(m_hMemDC, m_hMyBitmap);

	FillRect(m_hMemDC, &rt, (HBRUSH)GetStockObject(WHITE_BRUSH));
}

// ������, ������ �����ϱ�, ����/���� ����Ʈ �ʱ�ȭ�ϱ�, ������ �ڵ� ����ϱ�.(�׸��׸��� ����)
void CFindSearch::Init(int StartX, int StartY, int FinX, int FinY)
{
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
	StartNode->dir = NONE;

	// �������� G,H,F ����
	FinNode->m_fG = 0;
	FinNode->m_fH = 0;
	FinNode->m_fF = 0;
	FinNode->stpParent = nullptr;
	FinNode->dir = NONE;

	// ���� ����Ʈ, ���� ����Ʈ �ʱ�ȭ
	m_CloseList.Clear();
	m_OpenList.Clear();
}

// ���� ��, �������� ���¸���Ʈ�� �ִ´�.
void CFindSearch::StartNodeInsert()
{
	m_OpenList.Insert(StartNode);		
}

// ȣ��� �� ���� Jump Point Search ����
void CFindSearch::Jump_Point_Search_While()
{
	if (m_OpenList.GetCount() == 0)
	{
		// ����Ʈ Ŭ����, Ÿ�̸� ���� �� ó��
		LastClear();
		return;
	}

	// 2-1. ���¸���Ʈ�� ������� �ʴٸ�, ���� ����Ʈ���� ��� �ϳ� ����.
	stFindSearchNode* Node = m_OpenList.GetListNode();

	// 2-2. ���� ��尡 ��������� break;
	if (Node->m_iX == FinNode->m_iX &&
		Node->m_iY == FinNode->m_iY)
	{
		// Node�� �������̴�, ������� �θ� Ÿ���鼭 ���� �ߴ´�.
		Line(Node);		
		
		// ����Ʈ Ŭ����, Ÿ�̸� ���� �� ó��
		int x = StartNode->m_iX, y = StartNode->m_iY, x2 = FinNode->m_iX, y2 = FinNode->m_iY;
		LastClear();

		// �ܺη� ��ȯ�ϴ� �뵵�� ����ϴ�, ���̺귯���� �Լ�(��θ� ��)�� �ٽ� ȣ���ؼ� ������ �ٽ� �׷���. ��߳����ʴ���!
		POINT point[100];
		int Count = PathGet(x, y, x2, y2, point, true);
		Out_Line(point, Count, RGB(255, 187, 0));

		return;
	}

	// 2-3. ������ ��尡 �ƴϸ� ���� ��带 CloseList�� �ֱ�
	m_CloseList.Insert(Node);

	if ((&stMapArrayCopy[Node->m_iY * m_iWidth] + Node->m_iX)->m_eTileType == OPEN_LIST)
		(&stMapArrayCopy[Node->m_iY * m_iWidth] + Node->m_iX)->m_eTileType = CLOSE_LIST;

	// 2-4. �׸��� ��� ���� �Լ� ȣ��
	// �� �ȿ��� ��Ȳ�� ���� Ÿ�� ����.
	CreateNodeCheck(Node);
}

// ���ڷ� ���� ��带 ��������, ���� ����
void CFindSearch::CreateNodeCheck(stFindSearchNode* Node, bool ShowCheck)
{
	// �̹� ����� �÷� ����
	COLORREF colorRGB;
	switch (m_iColorKey)
	{
	case 1:
		colorRGB = Color1;
		m_iColorKey++;
		break;

	case 2:
		colorRGB = Color2;
		m_iColorKey++;
		break;

	case 3:
		colorRGB = Color3;
		m_iColorKey++;
		break;

	case 4:
		colorRGB = Color4;
		m_iColorKey++;
		break;

	case 5:
		colorRGB = Color5;
		m_iColorKey++;
		break;

	case 6:
		colorRGB = Color6;
		m_iColorKey++;
		break;

	case 7:
		colorRGB = Color7;
		m_iColorKey++;
		break;

	case 8:
		colorRGB = Color8;
		m_iColorKey = 1;
		break;

	default:
		break;
	}

	// 1. ���ڷ� ���� ��尡 ������ �����, 8�������� Jump�ϸ鼭 ��� ���� ���� üũ
	if (Node->stpParent == nullptr)
	{
		// LL
		CheckCreateJump(Node->m_iX - 1, Node->m_iY, Node, colorRGB, LL);

		// RR
		CheckCreateJump(Node->m_iX + 1, Node->m_iY, Node, colorRGB, RR);

		// UU
		CheckCreateJump(Node->m_iX, Node->m_iY - 1, Node, colorRGB, UU);

		// DD
		CheckCreateJump(Node->m_iX, Node->m_iY + 1, Node, colorRGB, DD);

		// LU
		CheckCreateJump(Node->m_iX - 1, Node->m_iY - 1, Node, colorRGB, LU);

		// LD
		CheckCreateJump(Node->m_iX - 1, Node->m_iY + 1, Node, colorRGB, LD);

		// RU
		CheckCreateJump(Node->m_iX + 1, Node->m_iY - 1, Node, colorRGB, RU);

		// RD
		CheckCreateJump(Node->m_iX + 1, Node->m_iY + 1, Node, colorRGB, RD);
	}

	// �װ� �ƴ϶��, ���� / �밢���� ���� ��� ���� ���� üũ
	else
	{
		// ��
		if (Node->dir == UU)
		{
			// �� ����
			CheckCreateJump(Node->m_iX, Node->m_iY - 1, Node, colorRGB, Node->dir);

			// �»� üũ
			if ((&stMapArrayCopy[Node->m_iY * m_iWidth] + (Node->m_iX - 1))->m_eTileType == OBSTACLE &&
				(&stMapArrayCopy[(Node->m_iY - 1) * m_iWidth] + (Node->m_iX - 1))->m_eTileType != OBSTACLE)
				CheckCreateJump(Node->m_iX - 1, Node->m_iY - 1, Node, colorRGB, LU);

			// ��� üũ
			if ((&stMapArrayCopy[Node->m_iY * m_iWidth] + (Node->m_iX + 1))->m_eTileType == OBSTACLE &&
				(&stMapArrayCopy[(Node->m_iY - 1) * m_iWidth] + (Node->m_iX + 1))->m_eTileType != OBSTACLE)
				CheckCreateJump(Node->m_iX + 1, Node->m_iY - 1, Node, colorRGB, RU);
		}

		// ��
		else if (Node->dir == DD)
		{
			// �� ����
			CheckCreateJump(Node->m_iX, Node->m_iY + 1, Node, colorRGB, Node->dir);

			// ���� üũ
			if ((&stMapArrayCopy[Node->m_iY * m_iWidth] + (Node->m_iX - 1))->m_eTileType == OBSTACLE &&
				(&stMapArrayCopy[(Node->m_iY + 1) * m_iWidth] + (Node->m_iX - 1))->m_eTileType != OBSTACLE)
				CheckCreateJump(Node->m_iX - 1, Node->m_iY + 1, Node, colorRGB, LD);

			// ���� üũ
			if ((&stMapArrayCopy[Node->m_iY * m_iWidth] + (Node->m_iX + 1))->m_eTileType == OBSTACLE &&
				(&stMapArrayCopy[(Node->m_iY + 1) * m_iWidth] + (Node->m_iX + 1))->m_eTileType != OBSTACLE)
				CheckCreateJump(Node->m_iX + 1, Node->m_iY + 1, Node, colorRGB, RD);
		}

		// ��
		else if (Node->dir == LL)
		{
			// �� ����
			CheckCreateJump(Node->m_iX - 1, Node->m_iY, Node, colorRGB, Node->dir);

			// �»� üũ
			if ((&stMapArrayCopy[(Node->m_iY - 1) * m_iWidth] + Node->m_iX)->m_eTileType == OBSTACLE &&
				(&stMapArrayCopy[(Node->m_iY - 1) * m_iWidth] + (Node->m_iX - 1))->m_eTileType != OBSTACLE)
				CheckCreateJump(Node->m_iX - 1, Node->m_iY - 1, Node, colorRGB, LU);

			// ���� üũ
			if ((&stMapArrayCopy[(Node->m_iY + 1) * m_iWidth] + Node->m_iX)->m_eTileType == OBSTACLE &&
				(&stMapArrayCopy[(Node->m_iY + 1) * m_iWidth] + (Node->m_iX - 1))->m_eTileType != OBSTACLE)
				CheckCreateJump(Node->m_iX - 1, Node->m_iY + 1, Node, colorRGB, LD);
		}

		// ��
		else if (Node->dir == RR)
		{
			// �� ����
			CheckCreateJump(Node->m_iX + 1, Node->m_iY, Node, colorRGB, Node->dir);

			// ��� üũ

			if ((&stMapArrayCopy[(Node->m_iY - 1) * m_iWidth] + Node->m_iX)->m_eTileType == OBSTACLE &&
				(&stMapArrayCopy[(Node->m_iY - 1) * m_iWidth] + (Node->m_iX + 1))->m_eTileType != OBSTACLE)
				CheckCreateJump(Node->m_iX + 1, Node->m_iY - 1, Node, colorRGB, RU);

			// ���� üũ
			if ((&stMapArrayCopy[(Node->m_iY + 1) * m_iWidth] + Node->m_iX)->m_eTileType == OBSTACLE &&
				(&stMapArrayCopy[(Node->m_iY + 1) * m_iWidth] + (Node->m_iX + 1))->m_eTileType != OBSTACLE)
				CheckCreateJump(Node->m_iX + 1, Node->m_iY + 1, Node, colorRGB, RD);
		}

		// �»�
		else if (Node->dir == LU)
		{
			// ���� ����
			CheckCreateJump(Node->m_iX - 1, Node->m_iY, Node, colorRGB, LL);

			// ���� ����
			CheckCreateJump(Node->m_iX, Node->m_iY - 1, Node, colorRGB, UU);

			// �»� �밢��
			CheckCreateJump(Node->m_iX - 1, Node->m_iY - 1, Node, colorRGB, Node->dir);

			// ��� �밢��
			if ((&stMapArrayCopy[Node->m_iY * m_iWidth] + (Node->m_iX + 1))->m_eTileType == OBSTACLE &&
				(&stMapArrayCopy[(Node->m_iY - 1) * m_iWidth] + (Node->m_iX + 1))->m_eTileType != OBSTACLE)
				CheckCreateJump(Node->m_iX + 1, Node->m_iY - 1, Node, colorRGB, RU);

			// ���� �밢�� 
			if ((&stMapArrayCopy[(Node->m_iY + 1)* m_iWidth] + Node->m_iX)->m_eTileType == OBSTACLE &&
				(&stMapArrayCopy[(Node->m_iY + 1)* m_iWidth] + (Node->m_iX - 1))->m_eTileType != OBSTACLE)
				CheckCreateJump(Node->m_iX - 1, Node->m_iY + 1, Node, colorRGB, LD);
		}

		// ����
		else if (Node->dir == LD)
		{
			// ���� ����
			CheckCreateJump(Node->m_iX - 1, Node->m_iY, Node, colorRGB, LL);

			// �Ʒ��� ����
			CheckCreateJump(Node->m_iX, Node->m_iY + 1, Node, colorRGB, DD);

			// ���� �밢��
			CheckCreateJump(Node->m_iX - 1, Node->m_iY + 1, Node, colorRGB, Node->dir);

			// �»� �밢��

			if ((&stMapArrayCopy[(Node->m_iY - 1)* m_iWidth] + Node->m_iX)->m_eTileType == OBSTACLE &&
				(&stMapArrayCopy[(Node->m_iY - 1)* m_iWidth] + (Node->m_iX - 1))->m_eTileType != OBSTACLE)
				CheckCreateJump(Node->m_iX - 1, Node->m_iY - 1, Node, colorRGB, LU);

			// ���� �밢�� 
			if ((&stMapArrayCopy[Node->m_iY* m_iWidth] + (Node->m_iX + 1))->m_eTileType == OBSTACLE &&
				(&stMapArrayCopy[(Node->m_iY + 1)* m_iWidth] + (Node->m_iX + 1))->m_eTileType != OBSTACLE)
				CheckCreateJump(Node->m_iX + 1, Node->m_iY + 1, Node, colorRGB, RD);
		}

		// ���
		else if (Node->dir == RU)
		{
			// ������ ����
			CheckCreateJump(Node->m_iX + 1, Node->m_iY, Node, colorRGB, RR);

			// ���� ����
			CheckCreateJump(Node->m_iX, Node->m_iY - 1, Node, colorRGB, UU);

			// ��� �밢��
			CheckCreateJump(Node->m_iX + 1, Node->m_iY - 1, Node, colorRGB, Node->dir);

			// �»� �밢��
			if ((&stMapArrayCopy[Node->m_iY* m_iWidth] + (Node->m_iX - 1))->m_eTileType == OBSTACLE &&
				(&stMapArrayCopy[(Node->m_iY - 1) * m_iWidth] + (Node->m_iX - 1))->m_eTileType != OBSTACLE)
				CheckCreateJump(Node->m_iX - 1, Node->m_iY - 1, Node, colorRGB, LU);

			// ���� �밢�� 
			if ((&stMapArrayCopy[(Node->m_iY + 1) * m_iWidth] + Node->m_iX)->m_eTileType == OBSTACLE &&
				(&stMapArrayCopy[(Node->m_iY + 1) * m_iWidth] + (Node->m_iX + 1))->m_eTileType != OBSTACLE)
				CheckCreateJump(Node->m_iX + 1, Node->m_iY + 1, Node, colorRGB, RD);
		}

		// ����
		else if (Node->dir == RD)
		{
			// ������ ����
			CheckCreateJump(Node->m_iX + 1, Node->m_iY, Node, colorRGB, RR);

			// �Ʒ��� ����
			CheckCreateJump(Node->m_iX, Node->m_iY + 1, Node, colorRGB, DD);

			// ���� �밢��
			CheckCreateJump(Node->m_iX + 1, Node->m_iY + 1, Node, colorRGB, Node->dir);

			// ��� �밢��
			if ((&stMapArrayCopy[(Node->m_iY - 1) * m_iWidth] + Node->m_iX)->m_eTileType == OBSTACLE &&
				(&stMapArrayCopy[(Node->m_iY - 1) * m_iWidth] + (Node->m_iX + 1))->m_eTileType != OBSTACLE)
				CheckCreateJump(Node->m_iX + 1, Node->m_iY - 1, Node, colorRGB, RU);

			// ���� �밢�� 
			if ((&stMapArrayCopy[Node->m_iY * m_iWidth] + (Node->m_iX - 1))->m_eTileType == OBSTACLE &&
				(&stMapArrayCopy[(Node->m_iY + 1) * m_iWidth] + (Node->m_iX - 1))->m_eTileType != OBSTACLE)
				CheckCreateJump(Node->m_iX - 1, Node->m_iY + 1, Node, colorRGB, LD);
		}
	}
	if(ShowCheck)
		GridShow();
}

// ���ڷ� ���� ��带 ��������, ��� �������� �̵��� ������ ���� ��, ����� ���� ��� ���� �Լ����� ȣ��.
void CFindSearch::CheckCreateJump(int X, int Y, stFindSearchNode* parent, COLORREF colorRGB, Direction dir)
{
	int OutX = 0, OutY = 0;
	double G = 0, OutG = 0;

	switch (dir)
	{
	case CFindSearch::LL:
		if (JumpLL(X, Y, &OutX, &OutY, G, &OutG, colorRGB, dir) == true)
		{
			OutG += parent->m_fG;
			CreateNode(OutX, OutY, parent, OutG, dir);
		}

		break;

	case CFindSearch::LU:
		if (JumpLU(X, Y, &OutX, &OutY, G, &OutG, colorRGB, dir) == true)
		{
			OutG += parent->m_fG;
			CreateNode(OutX, OutY, parent, OutG, dir);
		}

		break;

	case CFindSearch::LD:
		if (JumpLD(X, Y, &OutX, &OutY, G, &OutG, colorRGB, dir) == true)
		{
			OutG += parent->m_fG;
			CreateNode(OutX, OutY, parent, OutG, dir);
		}

		break;

	case CFindSearch::RR:
		if (JumpRR(X, Y, &OutX, &OutY, G, &OutG, colorRGB, dir) == true)
		{
			OutG += parent->m_fG;
			CreateNode(OutX, OutY, parent, OutG, dir);
		}

		break;

	case CFindSearch::RU:
		if (JumpRU(X, Y, &OutX, &OutY, G, &OutG, colorRGB, dir) == true)
		{
			OutG += parent->m_fG;
			CreateNode(OutX, OutY, parent, OutG, dir);
		}

		break;

	case CFindSearch::RD:
		if (JumpRD(X, Y, &OutX, &OutY, G, &OutG, colorRGB, dir) == true)
		{
			OutG += parent->m_fG;
			CreateNode(OutX, OutY, parent, OutG, dir);
		}

		break;

	case CFindSearch::UU:
		if (JumpUU(X, Y, &OutX, &OutY, G, &OutG, colorRGB, dir) == true)
		{
			OutG += parent->m_fG;
			CreateNode(OutX, OutY, parent, OutG, dir);		
		}				

		break;

	case CFindSearch::DD:
		if (JumpDD(X, Y, &OutX, &OutY, G, &OutG, colorRGB, dir) == true)
		{
			OutG += parent->m_fG;
			CreateNode(OutX, OutY, parent, OutG, dir);
		}

		break;
	}
}

// �� ����
bool CFindSearch::JumpUU(int X, int Y, int* OutX, int* OutY, double G, double* OutG, COLORREF colorRGB, Direction dir)
{	
	int iTempMaxSearchCount = 0;

	while (1)
	{
		// 1. �ش� ��ǥ�� ���� ��� ���� ���� ��ǥ���� üũ (�� ������, ���ع�����)
		if (CheckTile(X, Y))
			return false;

		G += m_dStraight_Line;

		// ���� ��尡 ���������� üũ
		if (FinNode->m_iX == X && FinNode->m_iY == Y)
		{
			*OutX = X;
			*OutY = Y;
			*OutG = G;
			break;
		}

		// �������� �ƴ϶��, ���� ��尡 �ڳ����� üũ
		else if (((&stMapArrayCopy[Y * m_iWidth] + (X - 1))->m_eTileType == OBSTACLE && (&stMapArrayCopy[(Y - 1) * m_iWidth] + (X - 1))->m_eTileType != OBSTACLE) ||
			((&stMapArrayCopy[Y * m_iWidth] + (X + 1))->m_eTileType == OBSTACLE && (&stMapArrayCopy[(Y - 1) * m_iWidth] + (X + 1))->m_eTileType != OBSTACLE))
		{
			*OutX = X;
			*OutY = Y;
			*OutG = G;
			break;
		}

		// �ڳʰ� �ƴ϶��, �ش� ��ġ�� Ÿ���� JUMP�� �ٲ۴�. Ȯ���ߴܴ� �ǹ̷�! 
		// �ٵ� ������/������/���¸���Ʈ/��������Ʈ�� ������ �ȹٲ۴�. 
		// �׵��� �����ϰ� ���� �����Ǿ�� �Ѵ�.
		if ((&stMapArrayCopy[Y * m_iWidth] + X)->m_eTileType != START && (&stMapArrayCopy[Y * m_iWidth] + X)->m_eTileType != FIN &&
			(&stMapArrayCopy[Y * m_iWidth] + X)->m_eTileType != OPEN_LIST && (&stMapArrayCopy[Y * m_iWidth] + X)->m_eTileType != CLOSE_LIST)
		{
			(&stMapArrayCopy[Y * m_iWidth] + X)->m_eTileType = JUMP;
			(&stMapArrayCopy[Y * m_iWidth] + X)->m_coColorRGB = colorRGB;
		}
		
		Y--;
		iTempMaxSearchCount++;
	}
	

	return true;
}

// �� ����
bool CFindSearch::JumpDD(int X, int Y, int* OutX, int* OutY, double G, double* OutG, COLORREF colorRGB, Direction dir)
{
	int iTempMaxSearchCount = 0;

	while (1)
	{
		// 1. �ش� ��ǥ�� ���� ��� ���� ���� ��ǥ���� üũ (�� ������, ���ع�����)
		if (CheckTile(X, Y))
			return false;

		G += m_dStraight_Line;

		// ���� ��尡 ���������� üũ
		if (FinNode->m_iX == X && FinNode->m_iY == Y)
		{
			*OutX = X;
			*OutY = Y;
			*OutG = G;
			break;
		}

		// ���� ��尡 �ڳ����� üũ
		else if (((&stMapArrayCopy[Y * m_iWidth] + (X - 1))->m_eTileType == OBSTACLE && (&stMapArrayCopy[(Y + 1) * m_iWidth] + (X - 1))->m_eTileType != OBSTACLE) ||
			((&stMapArrayCopy[Y *  m_iWidth] + (X + 1))->m_eTileType == OBSTACLE && (&stMapArrayCopy[(Y + 1) * m_iWidth] + (X + 1))->m_eTileType != OBSTACLE))
		{
			*OutX = X;
			*OutY = Y;
			*OutG = G;
			break;
		}

		if ((&stMapArrayCopy[Y * m_iWidth] + X)->m_eTileType != START && (&stMapArrayCopy[Y * m_iWidth] + X)->m_eTileType != FIN &&
			(&stMapArrayCopy[Y * m_iWidth] + X)->m_eTileType != OPEN_LIST && (&stMapArrayCopy[Y * m_iWidth] + X)->m_eTileType != CLOSE_LIST)
		{
			(&stMapArrayCopy[Y * m_iWidth] + X)->m_eTileType = JUMP;
			(&stMapArrayCopy[Y * m_iWidth] + X)->m_coColorRGB = colorRGB;
		}

		Y++;
		iTempMaxSearchCount++;

	}
	
	return true;
}

// �� ����
bool CFindSearch::JumpLL(int X, int Y, int* OutX, int* OutY, double G, double* OutG, COLORREF colorRGB, Direction dir)
{
	int iTempMaxSearchCount = 0;

	while (1)
	{
		// 1. �ش� ��ǥ�� ���� ��� ���� ���� ��ǥ���� üũ (�� ������, ���ع�����)
		if (CheckTile(X, Y))
			return false;

		G += m_dStraight_Line;

		// ���� ��尡 ���������� üũ
		if (FinNode->m_iX == X && FinNode->m_iY == Y)
		{
			*OutX = X;
			*OutY = Y;
			*OutG = G;
			break;
		}

		// ���� ��尡 �ڳ����� üũ
		else if (((&stMapArrayCopy[(Y - 1) *  m_iWidth] + X)->m_eTileType == OBSTACLE && (&stMapArrayCopy[(Y - 1) *  m_iWidth] + (X - 1))->m_eTileType != OBSTACLE) ||
			((&stMapArrayCopy[(Y + 1) *  m_iWidth] + X)->m_eTileType == OBSTACLE && (&stMapArrayCopy[(Y + 1) *  m_iWidth] + (X - 1))->m_eTileType != OBSTACLE))
		{
			*OutX = X;
			*OutY = Y;
			*OutG = G;
			break;
		}

		if ((&stMapArrayCopy[Y *  m_iWidth] + X)->m_eTileType != START && (&stMapArrayCopy[Y *  m_iWidth] + X)->m_eTileType != FIN &&
			(&stMapArrayCopy[Y *  m_iWidth] + X)->m_eTileType != OPEN_LIST && (&stMapArrayCopy[Y *  m_iWidth] + X)->m_eTileType != CLOSE_LIST)
		{
			(&stMapArrayCopy[Y *  m_iWidth] + X)->m_eTileType = JUMP;
			(&stMapArrayCopy[Y *  m_iWidth] + X)->m_coColorRGB = colorRGB;
		}

		X--;
		iTempMaxSearchCount++;
	}

	return true;
}

// �� ����
bool CFindSearch::JumpRR(int X, int Y, int* OutX, int* OutY, double G, double* OutG, COLORREF colorRGB, Direction dir)
{
	int iTempMaxSearchCount = 0;

	while (1)
	{
		// 1. �ش� ��ǥ�� ���� ��� ���� ���� ��ǥ���� üũ (�� ������, ���ع�����)
		if (CheckTile(X, Y))
			return false;

		G += m_dStraight_Line;

		// ���� ��尡 ���������� üũ
		if (FinNode->m_iX == X && FinNode->m_iY == Y)
		{
			*OutX = X;
			*OutY = Y;
			*OutG = G;
			break;
		}

		// ���� ��尡 �ڳ����� üũ
		else if (((&stMapArrayCopy[(Y - 1) *  m_iWidth] + X)->m_eTileType == OBSTACLE && (&stMapArrayCopy[(Y - 1) *  m_iWidth] + (X + 1))->m_eTileType != OBSTACLE) ||
			((&stMapArrayCopy[(Y + 1) *  m_iWidth] + X)->m_eTileType == OBSTACLE && (&stMapArrayCopy[(Y + 1) *  m_iWidth] + (X + 1))->m_eTileType != OBSTACLE))
		{
			*OutX = X;
			*OutY = Y;
			*OutG = G;
			break;
		}

		if ((&stMapArrayCopy[Y *  m_iWidth] + X)->m_eTileType != START && (&stMapArrayCopy[Y *  m_iWidth] + X)->m_eTileType != FIN &&
			(&stMapArrayCopy[Y *  m_iWidth] + X)->m_eTileType != OPEN_LIST && (&stMapArrayCopy[Y *  m_iWidth] + X)->m_eTileType != CLOSE_LIST)
		{
			(&stMapArrayCopy[Y *  m_iWidth] + X)->m_eTileType = JUMP;
			(&stMapArrayCopy[Y *  m_iWidth] + X)->m_coColorRGB = colorRGB;
		}

		X++;
		iTempMaxSearchCount++;

	}

	return true;
}

// �»� ����
bool CFindSearch::JumpLU(int X, int Y, int* OutX, int* OutY, double G, double* OutG, COLORREF colorRGB, Direction dir)
{
	int iTempMaxSearchCount = 0;

	while (1)
	{
		// 1. �ش� ��ǥ�� ���� ��� ���� ���� ��ǥ���� üũ (�� ������, ���ع�����)
		if (CheckTile(X, Y))
			return false;

		G += m_dDiagonal_Line;
		// ���� ��尡 ���������� üũ
		if (FinNode->m_iX == X && FinNode->m_iY == Y)
		{
			*OutX = X;
			*OutY = Y;
			*OutG = G;
			return true;
		}

		// ���� ��ġ�� �ڳ����� üũ
		else if (((&stMapArrayCopy[Y *  m_iWidth] + (X + 1))->m_eTileType == OBSTACLE && (&stMapArrayCopy[(Y - 1) *  m_iWidth] + (X + 1))->m_eTileType != OBSTACLE) ||
			((&stMapArrayCopy[(Y + 1) *  m_iWidth] + X)->m_eTileType == OBSTACLE && (&stMapArrayCopy[(Y + 1) *  m_iWidth] + (X - 1))->m_eTileType != OBSTACLE))
		{
			*OutX = X;
			*OutY = Y;
			*OutG = G;
			break;
		}

		// ���� ���� �Ÿ��� �ڳʰ� �ִ��� üũ
		else if (JumpLL(X - 1, Y, OutX, OutY, G, OutG, colorRGB, LL) == true)
		{
			*OutX = X;
			*OutY = Y;
			*OutG = G;
			break;
		}

		// �� �����Ÿ��� �ڳʰ� �ִ��� üũ
		else if (JumpUU(X, Y - 1, OutX, OutY, G, OutG, colorRGB, UU) == true)
		{
			*OutX = X;
			*OutY = Y;
			*OutG = G;
			break;
		}

		if ((&stMapArrayCopy[Y *  m_iWidth] + X)->m_eTileType != START && (&stMapArrayCopy[Y *  m_iWidth] + X)->m_eTileType != FIN &&
			(&stMapArrayCopy[Y *  m_iWidth] + X)->m_eTileType != OPEN_LIST && (&stMapArrayCopy[Y *  m_iWidth] + X)->m_eTileType != CLOSE_LIST)
		{
			(&stMapArrayCopy[Y *  m_iWidth] + X)->m_eTileType = JUMP;
			(&stMapArrayCopy[Y *  m_iWidth] + X)->m_coColorRGB = colorRGB;
		}

		X--;
		Y--;
		iTempMaxSearchCount++;
	}
	return true;
}

// ���� ����
bool CFindSearch::JumpLD(int X, int Y, int* OutX, int* OutY, double G, double* OutG, COLORREF colorRGB, Direction dir)
{
	int iTempMaxSearchCount = 0;

	while (1)
	{
		// 1. �ش� ��ǥ�� ���� ��� ���� ���� ��ǥ���� üũ (�� ������, ���ع�����)
		if (CheckTile(X, Y))
			return false;

		G += m_dDiagonal_Line;
		// ���� ��尡 ���������� üũ
		if (FinNode->m_iX == X && FinNode->m_iY == Y)
		{
			*OutX = X;
			*OutY = Y;
			*OutG = G;
			break;
		}

		// ���� ��ġ�� �ڳ����� üũ
		else if (((&stMapArrayCopy[(Y - 1) *  m_iWidth] + X)->m_eTileType == OBSTACLE && (&stMapArrayCopy[(Y - 1) *  m_iWidth] + (X - 1))->m_eTileType != OBSTACLE) ||
			((&stMapArrayCopy[Y *  m_iWidth] + (X + 1))->m_eTileType == OBSTACLE && (&stMapArrayCopy[(Y + 1) *  m_iWidth] + (X + 1))->m_eTileType != OBSTACLE))
		{
			*OutX = X;
			*OutY = Y;
			*OutG = G;
			break;
		}

		// ���� ���� �Ÿ��� �ڳʰ� �ִ��� üũ
		else if (JumpLL(X - 1, Y, OutX, OutY, G, OutG, colorRGB, LL) == true)
		{
			*OutX = X;
			*OutY = Y;
			*OutG = G;
			break;
		}

		// �Ʒ� �����Ÿ��� �ڳʰ� �ִ��� üũ
		else if (JumpDD(X, Y + 1, OutX, OutY, G, OutG, colorRGB, DD) == true)
		{
			*OutX = X;
			*OutY = Y;
			*OutG = G;
			break;
		}

		if ((&stMapArrayCopy[Y *  m_iWidth] + X)->m_eTileType != START && (&stMapArrayCopy[Y *  m_iWidth] + X)->m_eTileType != FIN &&
			(&stMapArrayCopy[Y *  m_iWidth] + X)->m_eTileType != OPEN_LIST && (&stMapArrayCopy[Y *  m_iWidth] + X)->m_eTileType != CLOSE_LIST)
		{
			(&stMapArrayCopy[Y *  m_iWidth] + X)->m_eTileType = JUMP;
			(&stMapArrayCopy[Y *  m_iWidth] + X)->m_coColorRGB = colorRGB;
		}

		X--;
		Y++;
		iTempMaxSearchCount++;
	}

	return true;
}

// ��� ����
bool CFindSearch::JumpRU(int X, int Y, int* OutX, int* OutY, double G, double* OutG, COLORREF colorRGB, Direction dir)
{
	int iTempMaxSearchCount = 0;

	while (1)
	{
		// 1. �ش� ��ǥ�� ���� ��� ���� ���� ��ǥ���� üũ (�� ������, ���ع�����)
		if (CheckTile(X, Y))
			return false;

		G += m_dDiagonal_Line;
		// ���� ��尡 ���������� üũ
		if (FinNode->m_iX == X && FinNode->m_iY == Y)
		{
			*OutX = X;
			*OutY = Y;
			*OutG = G;
			break;
		}

		// ���� ��ġ�� �ڳ����� üũ
		else if (((&stMapArrayCopy[Y *  m_iWidth] + (X - 1))->m_eTileType == OBSTACLE && (&stMapArrayCopy[(Y - 1) *  m_iWidth] + (X - 1))->m_eTileType != OBSTACLE) ||
			((&stMapArrayCopy[(Y + 1) *  m_iWidth] + X)->m_eTileType == OBSTACLE && (&stMapArrayCopy[(Y + 1) *  m_iWidth] + (X + 1))->m_eTileType != OBSTACLE))
		{
			*OutX = X;
			*OutY = Y;
			*OutG = G;
			break;
		}

		// ������ ���� �Ÿ��� �ڳʰ� �ִ��� üũ
		else if (JumpRR(X + 1, Y, OutX, OutY, G, OutG, colorRGB, RR) == true)
		{
			*OutX = X;
			*OutY = Y;
			*OutG = G;
			break;
		}

		// �� �����Ÿ��� �ڳʰ� �ִ��� üũ
		else if (JumpUU(X, Y - 1, OutX, OutY, G, OutG, colorRGB, UU) == true)
		{
			*OutX = X;
			*OutY = Y;
			*OutG = G;
			break;
		}

		if ((&stMapArrayCopy[Y *  m_iWidth] + X)->m_eTileType != START && (&stMapArrayCopy[Y *  m_iWidth] + X)->m_eTileType != FIN &&
			(&stMapArrayCopy[Y *  m_iWidth] + X)->m_eTileType != OPEN_LIST && (&stMapArrayCopy[Y *  m_iWidth] + X)->m_eTileType != CLOSE_LIST)
		{
			(&stMapArrayCopy[Y *  m_iWidth] + X)->m_eTileType = JUMP;
			(&stMapArrayCopy[Y *  m_iWidth] + X)->m_coColorRGB = colorRGB;
		}

		X++;
		Y--;
		iTempMaxSearchCount++;
	}

	return true;
}

// ����
bool CFindSearch::JumpRD(int X, int Y, int* OutX, int* OutY, double G, double* OutG, COLORREF colorRGB, Direction dir)
{
	int iTempMaxSearchCount = 0;

	while (1)
	{
		// 1. �ش� ��ǥ�� ���� ��� ���� ���� ��ǥ���� üũ (�� ������, ���ع�����)
		// true�� ���ϵǸ�, ��ֹ��̰ų� �� ���� ��ǥ�� ��.
		if (CheckTile(X, Y))
			return false;

		G += m_dDiagonal_Line;

		// ���� ��尡 ���������� üũ
		if (FinNode->m_iX == X && FinNode->m_iY == Y)
		{
			*OutX = X;
			*OutY = Y;
			*OutG = G;
			break;
		}

		// ���� ��ġ�� �ڳ����� üũ
		else if (((&stMapArrayCopy[Y *  m_iWidth] + (X - 1))->m_eTileType == OBSTACLE && (&stMapArrayCopy[(Y + 1) *  m_iWidth] + (X - 1))->m_eTileType != OBSTACLE) ||
			((&stMapArrayCopy[(Y - 1) *  m_iWidth] + X)->m_eTileType == OBSTACLE && (&stMapArrayCopy[(Y - 1) *  m_iWidth] + (X + 1))->m_eTileType != OBSTACLE))
		{
			*OutX = X;
			*OutY = Y;
			*OutG = G;
			break;
		}

		// ������ ���� �Ÿ��� �ڳʰ� �ִ��� üũ
		else if (JumpRR(X + 1, Y, OutX, OutY, G, OutG, colorRGB, RR) == true)
		{
			*OutX = X;
			*OutY = Y;
			*OutG = G;
			break;
		}

		// �Ʒ� �����Ÿ��� �ڳʰ� �ִ��� üũ
		else if (JumpDD(X, Y + 1, OutX, OutY, G, OutG, colorRGB, DD) == true)
		{
			*OutX = X;
			*OutY = Y;
			*OutG = G;
			break;
		}

		if ((&stMapArrayCopy[Y *  m_iWidth] + X)->m_eTileType != START && (&stMapArrayCopy[Y *  m_iWidth] + X)->m_eTileType != FIN &&
			(&stMapArrayCopy[Y *  m_iWidth] + X)->m_eTileType != OPEN_LIST && (&stMapArrayCopy[Y *  m_iWidth] + X)->m_eTileType != CLOSE_LIST)
		{
			(&stMapArrayCopy[Y *  m_iWidth] + X)->m_eTileType = JUMP;
			(&stMapArrayCopy[Y *  m_iWidth] + X)->m_coColorRGB = colorRGB;
		}

		X++;
		Y++;
		iTempMaxSearchCount++;
	}

	return true;
}

// ��� ���� �Լ�
void CFindSearch::CreateNode(int iX, int iY, stFindSearchNode* parent, double G, Direction dir)
{	
	// 1. �ش� ��ǥ�� ��尡 Open ����Ʈ�� �ִ��� üũ
	stFindSearchNode* SearchNode = m_OpenList.Search(iX, iY);
	if (SearchNode != nullptr)
	{
		// �ִٸ�, �ش� ����� G���� ���� ������ ����� G�� üũ.
		// �ش� ����� G���� �� ũ�ٸ�, 
		if (SearchNode->m_fG > G)
		{
			// parent�� �θ� ����
			SearchNode->stpParent = parent;

			// ���⵵ �缳��
			SearchNode->dir = dir;

			// G�� ����� (parent�� G�� + �θ���� �������� �Ÿ�)
			SearchNode->m_fG = G;

			// F�� �����.
			SearchNode->m_fF = (SearchNode->m_fG + SearchNode->m_fH);

			// ���ĵ� �ٽ��Ѵ�.
			m_OpenList.Sort();
		}
		return;
	}

	// 2. Close�� ����Ʈ�� �ִ����� üũ
	SearchNode = m_CloseList.Search(iX, iY);
	if (SearchNode != nullptr)
	{
		// �ִٸ�, �ش� ����� G���� ���� ������ ����� G�� üũ.
		// �ش� ����� G���� �� ũ�ٸ�, 
		if (SearchNode->m_fG > G)
		{
			// parent�� �θ� ����
			SearchNode->stpParent = parent;

			// ���⵵ �缳��
			SearchNode->dir = dir;

			// G�� ����� (parent�� G�� + �θ���� �������� �Ÿ�)
			SearchNode->m_fG = G;

			// F�� �����.
			SearchNode->m_fF = (SearchNode->m_fG + SearchNode->m_fH);

			// ���ĵ� �ٽ��Ѵ�.
			m_OpenList.Sort();
		}

		return;
	}

	// 3. �� �ٿ� ������, ��� ���� ��, Open�� �ִ´�.
	stFindSearchNode* NewNode = new stFindSearchNode[sizeof(stFindSearchNode)];

	NewNode->stpParent = parent;
	NewNode->dir = dir;

	NewNode->m_iX = iX;
	NewNode->m_iY = iY;

	NewNode->m_fG = G;
	NewNode->m_fH = fabs(FinNode->m_iX - iX) + fabs(FinNode->m_iY - iY);
	NewNode->m_fF = NewNode->m_fG + NewNode->m_fH;

	// �ش� ��尡 ����,����, ���ع���尡 �ƴϸ�, OPEN_LIST�� ����
	if ((&stMapArrayCopy[iY *  m_iWidth] + iX)->m_eTileType != START && (&stMapArrayCopy[iY *  m_iWidth] + iX)->m_eTileType != FIN && (&stMapArrayCopy[iY *  m_iWidth] + iX)->m_eTileType != OBSTACLE)
		(&stMapArrayCopy[iY *  m_iWidth] + iX)->m_eTileType = OPEN_LIST;

	m_OpenList.Insert(NewNode);

	// 4. ���ĵ� �ٽ��Ѵ�.
	m_OpenList.Sort();
}

// �׸��� �׸���
void CFindSearch::GridShow()
{	
	HDC hdc = GetDC(m_hWnd);
	
	// �׸��� �׸���

	for (int i = 0; i < m_iHeight; ++i)
	{
		for (int j = 0; j < m_iWidth; ++j)
		{
			HBRUSH hMyBrush = NULL;
			HBRUSH hOldBrush = NULL;
			bool bBrushCheck = false;

			// �ش� �� Ÿ���� ���¿� ���� ���� �����Ѵ�.
			// ������� �ʷϻ�
			if ((&stMapArrayCopy[i *  m_iWidth] + j)->m_eTileType == START)
			{
				hMyBrush = CreateSolidBrush(RGB(0, 255, 0));
				hOldBrush = (HBRUSH)SelectObject(m_hMemDC, hMyBrush);
				bBrushCheck = true;
			}

			// �������� ������
			else if ((&stMapArrayCopy[i *  m_iWidth] + j)->m_eTileType == FIN)
			{
				hMyBrush = CreateSolidBrush(RGB(255, 0, 0));
				hOldBrush = (HBRUSH)SelectObject(m_hMemDC, hMyBrush);
				bBrushCheck = true;
			}

			// ���ع��� ȸ��
			else if ((&stMapArrayCopy[i *  m_iWidth] + j)->m_eTileType == OBSTACLE)

			{
				hMyBrush = CreateSolidBrush(RGB(80, 80, 80));
				hOldBrush = (HBRUSH)SelectObject(m_hMemDC, hMyBrush);
				bBrushCheck = true;
			}

			// OPEN_LIST�� �Ķ���
			else if ((&stMapArrayCopy[i *  m_iWidth] + j)->m_eTileType == OPEN_LIST)
			{
				hMyBrush = CreateSolidBrush(RGB(0, 0, 255));
				hOldBrush = (HBRUSH)SelectObject(m_hMemDC, hMyBrush);
				bBrushCheck = true;
			}

			// CLOSE_LIST�� ��ο� �Ķ���
			else if ((&stMapArrayCopy[i *  m_iWidth] + j)->m_eTileType == CLOSE_LIST)
			{
				hMyBrush = CreateSolidBrush(RGB(0, 0, 127));
				hOldBrush = (HBRUSH)SelectObject(m_hMemDC, hMyBrush);
				bBrushCheck = true;
			}

			// JUMP�� Ÿ�Ͽ� ������ �� ���
			else if ((&stMapArrayCopy[i *  m_iWidth] + j)->m_eTileType == JUMP)
			{
				hMyBrush = CreateSolidBrush((&stMapArrayCopy[i *  m_iWidth] + j)->m_coColorRGB);
				hOldBrush = (HBRUSH)SelectObject(m_hMemDC, hMyBrush);
				bBrushCheck = true;
			}

			// TEST�� Ÿ�Ͽ� ������ �� ��� ��, �Ӽ��� NONE���� ����
			else if ((&stMapArrayCopy[i *  m_iWidth] + j)->m_eTileType == TEST)
			{
				hMyBrush = CreateSolidBrush((&stMapArrayCopy[i *  m_iWidth] + j)->m_coColorRGB);
				hOldBrush = (HBRUSH)SelectObject(m_hMemDC, hMyBrush);
				bBrushCheck = true;
				(&stMapArrayCopy[i *  m_iWidth] + j)->m_eTileType = Tile_Type::NONE;
			}

			Rectangle(m_hMemDC, (&stMapArrayCopy[i *  m_iWidth] + j)->m_iMapX - m_iRadius, (&stMapArrayCopy[i *  m_iWidth] + j)->m_iMapY - m_iRadius, (&stMapArrayCopy[i *  m_iWidth] + j)->m_iMapX + m_iRadius, (&stMapArrayCopy[i *  m_iWidth] + j)->m_iMapY + m_iRadius);

			if (bBrushCheck == true)
			{
				SelectObject(m_hMemDC, hOldBrush);
				DeleteObject(hMyBrush);
				DeleteObject(hOldBrush);
			}
		}
	}

	BitBlt(hdc, rt.left, rt.top, rt.right, rt.bottom, m_hMemDC, rt.left, rt.top, SRCCOPY);

	ReleaseDC(m_hWnd, hdc);
}

// ���߱�
void CFindSearch::Line(stFindSearchNode* Node, COLORREF rgb)
{
	HDC hdc = GetDC(m_hWnd);

	MoveToEx(m_hMemDC, (&stMapArrayCopy[(Node->m_iY) *  m_iWidth] + Node->m_iX)->m_iMapX,
		(&stMapArrayCopy[(Node->m_iY) *  m_iWidth] + Node->m_iX)->m_iMapY, NULL);

	HPEN hMyPen, OldPen;

	if (rgb == -1)
	{
		hMyPen = CreatePen(PS_SOLID, 3, RGB(255, 0, 0));
		OldPen = (HPEN)SelectObject(m_hMemDC, hMyPen);
	}
	else
	{
		hMyPen = CreatePen(PS_SOLID, 3, rgb);
		OldPen = (HPEN)SelectObject(m_hMemDC, hMyPen);
	}

	stFindSearchNode* ParentNode = Node->stpParent;
	while (1)
	{
		if (ParentNode->m_iX == StartNode->m_iX && ParentNode->m_iY == StartNode->m_iY)
		{
			LineTo(m_hMemDC, (&stMapArrayCopy[(ParentNode->m_iY) *  m_iWidth] + ParentNode->m_iX)->m_iMapX,
				(&stMapArrayCopy[(ParentNode->m_iY) *  m_iWidth] + ParentNode->m_iX)->m_iMapY);
			break;
		}

		LineTo(m_hMemDC, (&stMapArrayCopy[(ParentNode->m_iY) *  m_iWidth] + ParentNode->m_iX)->m_iMapX,
			(&stMapArrayCopy[(ParentNode->m_iY) *  m_iWidth] + ParentNode->m_iX)->m_iMapY);

		ParentNode = ParentNode->stpParent;
	}

	SelectObject(m_hMemDC, OldPen);
	DeleteObject(hMyPen);
	DeleteObject(OldPen);

	BitBlt(hdc, rt.left, rt.top, rt.right, rt.bottom, m_hMemDC, rt.left, rt.top, SRCCOPY);

	ReleaseDC(m_hWnd, hdc);
}

// Jump_Point_Search_While()�� �Ϸ�� ��, ����/���� ����Ʈ, Ÿ�̸� �� �����ϱ�
void CFindSearch::LastClear()
{
	// 1. FinNode�� �Ҹ� �ȵ����� �Ҹ��Ų��.
	delete[] FinNode;

	// 2. Ÿ�̸ӵ� ������.
	KillTimer(m_hWnd, 1);

	m_iColorKey = 1;
}


////////////////////////////////
// �ܺη� X,Y�� ��ȯ�ϴ� �Լ�
///////////////////////////////
// �ۿ��� ������, ������� Map ����ü�� ������, ��θ� POINT����ü �ȿ� �־��ִ� �Լ�. 
// POINT����ü �迭 0������ parent�� �ϸ鼭 ���� �ȴ�.
// Use_Bresenham������ true�� ������ �극���� �˰������� ��ã�´�.
// �� �� �迭���� á���� �����Ѵ�.
int CFindSearch::PathGet(int StartX, int StartY, int FinX, int FinY, POINT* Outmap, bool Use_Bresenham)
{
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
	StartNode->dir = NONE;

	// �������� G,H,F ����
	FinNode->m_fG = 0;
	FinNode->m_fH = 0;
	FinNode->m_fF = 0;
	FinNode->stpParent = nullptr;
	FinNode->dir = NONE;

	// ���� ����Ʈ, ���� ����Ʈ �ʱ�ȭ
	m_CloseList.Clear();
	m_OpenList.Clear();

	// ���¸���Ʈ�� ���� ��ġ �ֱ�.
	m_OpenList.Insert(StartNode);

	// while�� ���鼭 ��� ���ϱ�.
	while (1)
	{
		if (m_OpenList.GetCount() == 0)
		{
			// 1. FinNode�� �Ҹ� �ȵ����� �Ҹ��Ų��.
			delete[] FinNode;
			m_iColorKey = 1;

			return 0;
		}

		// 2-1. ���¸���Ʈ�� ������� �ʴٸ�, ���� ����Ʈ���� ��� �ϳ� ����.
		stFindSearchNode* Node = m_OpenList.GetListNode();

		// 2-2. ���� ��尡 ��������� break;
		if (Node->m_iX == FinNode->m_iX &&
			Node->m_iY == FinNode->m_iY)
		{	
			// m_bUse_Bresenham����(�����ڿ��� ����)�� true��� �극���� �˰��� ����ϰڴٴ� ��.
			if (Use_Bresenham)
			{
				Bresenham_Line(Node);
			}

			// �迭 ä���
			int Array = 0;
			while (1)
			{
				Outmap[Array].x = Node->m_iX;
				Outmap[Array].y = Node->m_iY;

				if (Node->stpParent == nullptr)
					break;								

				Node = Node->stpParent;
				Array++;
			}

			// FinNode�� �Ҹ� �ȵ����� �Ҹ��Ų��.
			delete[] FinNode;
			m_iColorKey = 1;

			return Array;
		}

		// 2-3. ������ ��尡 �ƴϸ� ���� ��带 CloseList�� �ֱ�
		m_CloseList.Insert(Node);

		if ((&stMapArrayCopy[Node->m_iY * m_iWidth] + Node->m_iX)->m_eTileType == OPEN_LIST)
			(&stMapArrayCopy[Node->m_iY * m_iWidth] + Node->m_iX)->m_eTileType = CLOSE_LIST;

		// 2-4. �׸��� ��� ���� �Լ� ȣ��
		// �� �ȿ��� ��Ȳ�� ���� Ÿ�� ����.
		CreateNodeCheck(Node, false);
	}
}

// �ܺο��� POINT���ڸ� �޾Ƽ� �� �߱�
void CFindSearch::Out_Line(POINT* p, int Count, COLORREF rgb)
{
	HDC hdc = GetDC(m_hWnd);

	MoveToEx(m_hMemDC, (&stMapArrayCopy[(p[0].y) *  m_iWidth] + p[0].x)->m_iMapX,
		(&stMapArrayCopy[(p[0].y) *  m_iWidth] + p[0].x)->m_iMapY, NULL);

	HPEN hMyPen, OldPen;

	if (rgb == -1)
	{
		hMyPen = CreatePen(PS_SOLID, 3, RGB(255, 0, 0));
		OldPen = (HPEN)SelectObject(m_hMemDC, hMyPen);
	}
	else
	{
		hMyPen = CreatePen(PS_SOLID, 3, rgb);
		OldPen = (HPEN)SelectObject(m_hMemDC, hMyPen);
	}

	for (int i = 1; i <= Count; ++i)
	{
		LineTo(m_hMemDC, (&stMapArrayCopy[(p[i].y) *  m_iWidth] + p[i].x)->m_iMapX,
			(&stMapArrayCopy[(p[i].y) *  m_iWidth] + p[i].x)->m_iMapY);
	}

	SelectObject(m_hMemDC, OldPen);
	DeleteObject(hMyPen);
	DeleteObject(OldPen);

	BitBlt(hdc, rt.left, rt.top, rt.right, rt.bottom, m_hMemDC, rt.left, rt.top, SRCCOPY);

	ReleaseDC(m_hWnd, hdc);
}


////////////////////////////////
// �극���� �˰��� ���� �Լ�
///////////////////////////////
// �극���� �˰��� �׽�Ʈ�� ��ĥ�ϱ�
void CFindSearch::Bresenham_Line_Test(int StartX, int StartY, int FinX, int FinY)
{
    FillRect(m_hMemDC, &rt, (HBRUSH)GetStockObject(WHITE_BRUSH));
	GridShow();

	CBresenhamLine test;
	test.Init(StartX, StartY, FinX, FinY);
	POINT p;

	HDC hdc = GetDC(m_hWnd);	

	while (test.GetNext(&p))
	{
		(&stMapArrayCopy[p.y *  m_iWidth] + p.x)->m_eTileType = TEST;
		(&stMapArrayCopy[p.y *  m_iWidth] + p.x)->m_coColorRGB = RGB(152,0,0);		
	}

	MoveToEx(m_hMemDC, (&stMapArrayCopy[StartY *  m_iWidth] + StartX)->m_iMapX, (&stMapArrayCopy[StartY *  m_iWidth] + StartX)->m_iMapY, NULL);
	LineTo(m_hMemDC, (&stMapArrayCopy[FinY *  m_iWidth] + FinX)->m_iMapX, (&stMapArrayCopy[FinY *  m_iWidth] + FinX)->m_iMapY);

	BitBlt(hdc, rt.left, rt.top, rt.right, rt.bottom, m_hMemDC, rt.left, rt.top, SRCCOPY);

	ReleaseDC(m_hWnd, hdc);
}

// �극���� ���� üũ
void CFindSearch::Bresenham_Line(stFindSearchNode* FinNode)
{
	// 1. �극���� ��ü ����
	CBresenhamLine Bresenham;

	// 2. ������ ��ü ����
	POINT p;

	// 3. while���� ���鼭, ������������, �극���� �˰����� �̿��� �ִܰ�� üũ.
	stFindSearchNode* NowNode = FinNode;
	stFindSearchNode* Now_Next = NowNode->stpParent;
	stFindSearchNode* Now_Next_Next = Now_Next->stpParent;	
	
	if (Now_Next_Next == nullptr)
		return;

	// 4. �극���� ��ü �ʱ�ȭ
	Bresenham.Init(NowNode->m_iX, NowNode->m_iY, Now_Next_Next->m_iX, Now_Next_Next->m_iY);

	// 5. �극���� �˰��� ����
	while (1)
	{
		// 1. Next ��� �˾ƿ���.
		Bresenham.GetNext(&p);

		// 2. Next ��ΰ�, ������ ���������, ��ǥ������ ������ ���̴� �Ʒ� ���� ����
		if (p.x == Now_Next_Next->m_iX && p.y == Now_Next_Next->m_iY)
		{
			// 2-1. NowNode�� �θ� Now_Next���� Now_Next_Next�� ���� (��, Now_Next�� ������)
			NowNode->stpParent = Now_Next_Next;

			// 2-2. Now_Next_Next�� NowNode�� ����
			//NowNode = Now_Next_Next;

			// 2-3. ����, NowNode�� ���� ���ų�, Now_Next_Next�� nullptre�̶��, �극���� �˰����� ���� ������.
			if (NowNode == StartNode || NowNode->stpParent->stpParent == nullptr)
				break;

			// 2-4. ���� ������ NowNode����, Now_Next / Now_Next_Next�� ������.
			Now_Next = NowNode->stpParent;
			Now_Next_Next = Now_Next->stpParent;		

			// 2-5. �극���� ��ü ���ʱ�ȭ
			Bresenham.Init(NowNode->m_iX, NowNode->m_iY, Now_Next_Next->m_iX, Now_Next_Next->m_iY);
		}

		// 3. Next ��ΰ�, ������ �������� �ƴ϶��, ���� ��ΰ� ��ֹ����� üũ
		else
		{
			// 3-1. ���� ��ΰ� ��ֹ��̸� Now_Next�� NowNode�� ���� �� �ٽ� ����.
			// ���� Now_Next, Now_Next_Next�ʹ� ���� ����. �극���� �˰������� ���� ����� Next�̴�.
			if ((&stMapArrayCopy[p.y *  m_iWidth] + p.x)->m_eTileType == OBSTACLE)
			{
				// 1. Now_Next�� NowNode�� ����
				NowNode = Now_Next;

				// 2. ����, NowNode�� ���� ���ų�, Now_Next_Next�� nullptre�̶��, �극���� �˰����� ���� ������.
				if (Now_Next == StartNode || NowNode->stpParent->stpParent == nullptr)
					break;

				// 3. ���� ������ NowNode����, Now_Next / Now_Next_Next�� ������.
				Now_Next = NowNode->stpParent;					
				Now_Next_Next = Now_Next->stpParent;

				// 2-5. �극���� ��ü ���ʱ�ȭ
				Bresenham.Init(NowNode->m_iX, NowNode->m_iY, Now_Next_Next->m_iX, Now_Next_Next->m_iY);
			}
		}
	}
}

