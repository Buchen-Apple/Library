#include "stdafx.h"
#include "Jump_Point_Search.h"
#include <math.h>
#include <time.h>

// ���ڷ� ���� ��带 ��������, ��� �������� �̵��� ������ ����. CheckCreateJump()��� �Լ��� ������� ��ũ�η� ����.
#define CheckCreateJump(X,Y,dir)	\
		if(Jump(X,Y,&OutX,&OutY,G,&OutG,colorRGB,dir))	\
		 { OutG += Node->m_fG; CreateNode(OutX, OutY, Node, OutG, dir); OutX = OutY; G = 0, OutG = Node->m_fG;}

// ������
CFindSearch::CFindSearch(Map(*Copy)[MAP_WIDTH])
{
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

//  ���� ��, �������� ���¸���Ʈ�� �ִ´�.
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
		Show(Node);
		
		// ����Ʈ Ŭ����, Ÿ�̸� ���� �� ó��
		LastClear();
		return;
	}

	// 2-3. ������ ��尡 �ƴϸ� ���� ��带 CloseList�� �ֱ�
	m_CloseList.Insert(Node);

	if (stMapArrayCopy[Node->m_iY][Node->m_iX].m_eTileType == OPEN_LIST)
		stMapArrayCopy[Node->m_iY][Node->m_iX].m_eTileType = CLOSE_LIST;

	// 2-4. �׸��� ��� ���� �Լ� ȣ��
	// �� �ȿ��� ��Ȳ�� ���� Ÿ�� ����.
	CreateNodeCheck(Node);
}

// ���ڷ� ���� ��带 ��������, ���� ����
void CFindSearch::CreateNodeCheck(stFindSearchNode* Node)
{
	// �̹� ����� �÷� ����
	COLORREF colorRGB;
	switch (ColorKey)
	{
	case 1:
		colorRGB = Color1;
		ColorKey++;
		break;

	case 2:
		colorRGB = Color2;
		ColorKey++;
		break;

	case 3:
		colorRGB = Color3;
		ColorKey++;
		break;

	case 4:
		colorRGB = Color4;
		ColorKey++;
		break;

	case 5:
		colorRGB = Color5;
		ColorKey++;
		break;

	case 6:
		colorRGB = Color6;
		ColorKey++;
		break;

	case 7:
		colorRGB = Color7;
		ColorKey++;
		break;

	case 8:
		colorRGB = Color8;
		ColorKey = 1;
		break;

	default:
		break;
	}	

	// CheckCreateJump ��ũ�� �Լ����� Jump�� CreateNode�� ȣ���ϴµ�, �� �� ����� ������ ����.
	int OutX = 0, OutY = 0;
	double G = 0, OutG = Node->m_fG;

	// 1. ���ڷ� ���� ��尡 ������ �����, 8�������� Jump�ϸ鼭 ��� ���� ���� üũ
	if (Node->stpParent == nullptr)
	{
		// LL
		CheckCreateJump(Node->m_iX - 1, Node->m_iY, LL);

		// RR
		CheckCreateJump(Node->m_iX + 1, Node->m_iY, RR);

		// UU
		CheckCreateJump(Node->m_iX, Node->m_iY - 1, UU);

		// DD
		CheckCreateJump(Node->m_iX, Node->m_iY + 1, DD);

		// LU
		CheckCreateJump(Node->m_iX -1, Node->m_iY -1, LU);

		// LD
		CheckCreateJump(Node->m_iX -1, Node->m_iY +1, LD);

		// RU
		CheckCreateJump(Node->m_iX +1, Node->m_iY -1, RU);

		// RD
		CheckCreateJump(Node->m_iX +1, Node->m_iY +1, RD);
	}

	// �װ� �ƴ϶��, ���� / �밢���� ���� ��� ���� ���� üũ
	else
	{
		// ��
		if (Node->dir == UU)
		{
			// �� ����
			CheckCreateJump(Node->m_iX, Node->m_iY - 1, Node->dir);

			// �»� üũ
			if (stMapArrayCopy[Node->m_iY][Node->m_iX - 1].m_eTileType == OBSTACLE && stMapArrayCopy[Node->m_iY - 1][Node->m_iX - 1].m_eTileType != OBSTACLE)
				CheckCreateJump(Node->m_iX - 1, Node->m_iY - 1, LU);

			// ��� üũ
			if (stMapArrayCopy[Node->m_iY][Node->m_iX + 1].m_eTileType == OBSTACLE && stMapArrayCopy[Node->m_iY - 1][Node->m_iX + 1].m_eTileType != OBSTACLE)
				CheckCreateJump(Node->m_iX + 1, Node->m_iY - 1, RU);
		}

		// ��
		else if (Node->dir == DD)
		{
			// �� ����
			CheckCreateJump(Node->m_iX, Node->m_iY + 1, Node->dir);

			// ���� üũ
			if (stMapArrayCopy[Node->m_iY][Node->m_iX - 1].m_eTileType == OBSTACLE && stMapArrayCopy[Node->m_iY + 1][Node->m_iX - 1].m_eTileType != OBSTACLE)
				CheckCreateJump(Node->m_iX - 1, Node->m_iY + 1, LD);

			// ���� üũ
			if (stMapArrayCopy[Node->m_iY][Node->m_iX + 1].m_eTileType == OBSTACLE && stMapArrayCopy[Node->m_iY + 1][Node->m_iX + 1].m_eTileType != OBSTACLE)
				CheckCreateJump(Node->m_iX + 1, Node->m_iY + 1, RD);
		}

		// ��
		else if (Node->dir == LL)
		{
			// �� ����
			CheckCreateJump(Node->m_iX - 1, Node->m_iY, Node->dir);

			// �»� üũ
			if (stMapArrayCopy[Node->m_iY -1][Node->m_iX].m_eTileType == OBSTACLE && stMapArrayCopy[Node->m_iY -1][Node->m_iX -1].m_eTileType != OBSTACLE)
				CheckCreateJump(Node->m_iX -1, Node->m_iY -1, LU);

			// ���� üũ
			if (stMapArrayCopy[Node->m_iY +1][Node->m_iX].m_eTileType == OBSTACLE && stMapArrayCopy[Node->m_iY +1][Node->m_iX -1].m_eTileType != OBSTACLE)
				CheckCreateJump(Node->m_iX -1, Node->m_iY +1, LD);
		}

		// ��
		else if (Node->dir == RR)
		{
			// �� ����
			CheckCreateJump(Node->m_iX + 1, Node->m_iY, Node->dir);

			// ��� üũ
			if (stMapArrayCopy[Node->m_iY - 1][Node->m_iX].m_eTileType == OBSTACLE && stMapArrayCopy[Node->m_iY - 1][Node->m_iX + 1].m_eTileType != OBSTACLE)
				CheckCreateJump(Node->m_iX + 1, Node->m_iY - 1, RU);

			// ���� üũ
			if (stMapArrayCopy[Node->m_iY + 1][Node->m_iX].m_eTileType == OBSTACLE && stMapArrayCopy[Node->m_iY + 1][Node->m_iX + 1].m_eTileType != OBSTACLE)
				CheckCreateJump(Node->m_iX + 1, Node->m_iY + 1, RD);
		}

		// �»�
		else if (Node->dir == LU)
		{
			// ���� ����
			CheckCreateJump(Node->m_iX - 1, Node->m_iY, LL);

			// ���� ����
			CheckCreateJump(Node->m_iX, Node->m_iY - 1, UU);

			// �»� �밢��
			CheckCreateJump(Node->m_iX - 1, Node->m_iY - 1, Node->dir);

			// ��� �밢��
			if (stMapArrayCopy[Node->m_iY][Node->m_iX + 1].m_eTileType == OBSTACLE && stMapArrayCopy[Node->m_iY - 1][Node->m_iX + 1].m_eTileType != OBSTACLE)
				CheckCreateJump(Node->m_iX + 1, Node->m_iY - 1, RU);

			// ���� �밢�� 
			if (stMapArrayCopy[Node->m_iY + 1][Node->m_iX].m_eTileType == OBSTACLE && stMapArrayCopy[Node->m_iY + 1][Node->m_iX - 1].m_eTileType != OBSTACLE)
				CheckCreateJump(Node->m_iX - 1, Node->m_iY + 1, LD);
		}

		// ����
		else if (Node->dir == LD)
		{
			// ���� ����
			CheckCreateJump(Node->m_iX - 1, Node->m_iY, LL);

			// �Ʒ��� ����
			CheckCreateJump(Node->m_iX, Node->m_iY + 1, DD);

			// ���� �밢��
			CheckCreateJump(Node->m_iX - 1, Node->m_iY + 1, Node->dir);

			// �»� �밢��
			if (stMapArrayCopy[Node->m_iY - 1][Node->m_iX].m_eTileType == OBSTACLE && stMapArrayCopy[Node->m_iY - 1][Node->m_iX - 1].m_eTileType != OBSTACLE)
				CheckCreateJump(Node->m_iX - 1, Node->m_iY - 1, LU);

			// ���� �밢�� 
			if (stMapArrayCopy[Node->m_iY][Node->m_iX + 1].m_eTileType == OBSTACLE && stMapArrayCopy[Node->m_iY + 1][Node->m_iX + 1].m_eTileType != OBSTACLE)
				CheckCreateJump(Node->m_iX + 1, Node->m_iY + 1, RD);
		}

		// ���
		else if (Node->dir == RU)
		{
			// ������ ����
			CheckCreateJump(Node->m_iX + 1, Node->m_iY, RR);

			// ���� ����
			CheckCreateJump(Node->m_iX, Node->m_iY - 1, UU);

			// ��� �밢��
			CheckCreateJump(Node->m_iX + 1, Node->m_iY - 1, Node->dir);

			// �»� �밢��
			if (stMapArrayCopy[Node->m_iY][Node->m_iX - 1].m_eTileType == OBSTACLE && stMapArrayCopy[Node->m_iY - 1][Node->m_iX - 1].m_eTileType != OBSTACLE)
				CheckCreateJump(Node->m_iX - 1, Node->m_iY - 1, LU);

			// ���� �밢�� 
			if (stMapArrayCopy[Node->m_iY + 1][Node->m_iX].m_eTileType == OBSTACLE && stMapArrayCopy[Node->m_iY + 1][Node->m_iX + 1].m_eTileType != OBSTACLE)
				CheckCreateJump(Node->m_iX + 1, Node->m_iY + 1, RD);
		}

		// ����
		else if (Node->dir == RD)
		{
			// ������ ����
			CheckCreateJump(Node->m_iX + 1, Node->m_iY, RR);

			// �Ʒ��� ����
			CheckCreateJump(Node->m_iX, Node->m_iY + 1, DD);

			// ���� �밢��
			CheckCreateJump(Node->m_iX + 1, Node->m_iY + 1, Node->dir);

			// ��� �밢��
			if (stMapArrayCopy[Node->m_iY - 1][Node->m_iX].m_eTileType == OBSTACLE && stMapArrayCopy[Node->m_iY - 1][Node->m_iX + 1].m_eTileType != OBSTACLE)
				CheckCreateJump(Node->m_iX + 1, Node->m_iY - 1, RU);

			// ���� �밢�� 
			if (stMapArrayCopy[Node->m_iY][Node->m_iX - 1].m_eTileType == OBSTACLE && stMapArrayCopy[Node->m_iY + 1][Node->m_iX - 1].m_eTileType != OBSTACLE)
				CheckCreateJump(Node->m_iX - 1, Node->m_iY + 1, LD);
		}			
	}	
	Show();
}

// ����
// �ش� ���⿡ ������ ��尡 �ִ��� üũ(���)
bool CFindSearch::Jump(int X, int Y, int* OutX, int* OutY, double G, double* OutG, COLORREF colorRGB, Direction dir)
{
	// ���� ����ġ, �밢�� ����ġ
	static double dStraight_Line = 1;
	static double dDiagonal_Line = 2;

	// 1. �ش� ��ǥ�� ���� ��� ���� ���� ��ǥ���� üũ (�� ������, ���ع�����)
	if (stMapArrayCopy[Y][X].m_eTileType == OBSTACLE ||
		X < 0 || X >= MAP_WIDTH || Y < 0 || Y >= MAP_HEIGHT)
		return false;

	// 2. ���⿡ ���� ����
	// ��	
	if (dir == UU)
	{
		G += dStraight_Line;

		// ���� ��尡 ���������� üũ
		if (FinNode->m_iX == X && FinNode->m_iY == Y)
		{
			*OutX = X;
			*OutY = Y;
			*OutG = G;
			return true;
		}

		// �������� �ƴ϶��, ���� ��尡 �ڳ����� üũ
		else if ((stMapArrayCopy[Y][X - 1].m_eTileType == OBSTACLE && stMapArrayCopy[Y - 1][X - 1].m_eTileType != OBSTACLE) ||
				(stMapArrayCopy[Y][X + 1].m_eTileType == OBSTACLE && stMapArrayCopy[Y - 1][X + 1].m_eTileType != OBSTACLE))
		{
			*OutX = X;
			*OutY = Y;
			*OutG = G;
			return true;
		}

		// �ڳʰ� �ƴ϶��, �ش� ��ġ�� Ÿ���� JUMP�� �ٲ۴�. Ȯ���ߴܴ� �ǹ̷�! 
		// �ٵ� ������/������/���¸���Ʈ/��������Ʈ�� ������ �ȹٲ۴�. 
		// �׵��� �����ϰ� ���� �����Ǿ�� �Ѵ�.
		if (stMapArrayCopy[Y][X].m_eTileType != START && stMapArrayCopy[Y][X].m_eTileType != FIN &&
			stMapArrayCopy[Y][X].m_eTileType != OPEN_LIST && stMapArrayCopy[Y][X].m_eTileType != CLOSE_LIST)
		{
			stMapArrayCopy[Y][X].m_eTileType = JUMP;
			stMapArrayCopy[Y][X].colorRGB = colorRGB;
		}

		Jump(X, Y - 1, OutX, OutY, G, OutG, colorRGB, dir);
	}

	// ��	
	else if (dir == DD)
	{
		G += dStraight_Line;

		// ���� ��尡 ���������� üũ
		if (FinNode->m_iX == X && FinNode->m_iY == Y)
		{
			*OutX = X;
			*OutY = Y;
			*OutG = G;
			return true;
		}

		// ���� ��尡 �ڳ����� üũ
		else if ((stMapArrayCopy[Y][X - 1].m_eTileType == OBSTACLE && stMapArrayCopy[Y + 1][X - 1].m_eTileType != OBSTACLE) ||
				(stMapArrayCopy[Y][X + 1].m_eTileType == OBSTACLE && stMapArrayCopy[Y + 1][X + 1].m_eTileType != OBSTACLE))
		{
			*OutX = X;
			*OutY = Y;
			*OutG = G;
			return true;
		}

		if (stMapArrayCopy[Y][X].m_eTileType != START && stMapArrayCopy[Y][X].m_eTileType != FIN &&
			stMapArrayCopy[Y][X].m_eTileType != OPEN_LIST && stMapArrayCopy[Y][X].m_eTileType != CLOSE_LIST)
		{
			stMapArrayCopy[Y][X].m_eTileType = JUMP;
			stMapArrayCopy[Y][X].colorRGB = colorRGB;
		}
		Jump(X, Y + 1, OutX, OutY, G, OutG, colorRGB, dir);
	}

	//��	
	else if (dir == LL)
	{
		G += dStraight_Line;

		// ���� ��尡 ���������� üũ
		if (FinNode->m_iX == X && FinNode->m_iY == Y)
		{
			*OutX = X;
			*OutY = Y;
			*OutG = G;
			return true;
		}

		// ���� ��尡 �ڳ����� üũ
		else if ((stMapArrayCopy[Y - 1][X].m_eTileType == OBSTACLE && stMapArrayCopy[Y - 1][X - 1].m_eTileType != OBSTACLE) ||
				(stMapArrayCopy[Y + 1][X].m_eTileType == OBSTACLE && stMapArrayCopy[Y + 1][X - 1].m_eTileType != OBSTACLE))
		{
			*OutX = X;
			*OutY = Y;
			*OutG = G;
			return true;
		}

		if (stMapArrayCopy[Y][X].m_eTileType != START && stMapArrayCopy[Y][X].m_eTileType != FIN &&
			stMapArrayCopy[Y][X].m_eTileType != OPEN_LIST && stMapArrayCopy[Y][X].m_eTileType != CLOSE_LIST)
		{
			stMapArrayCopy[Y][X].m_eTileType = JUMP;
			stMapArrayCopy[Y][X].colorRGB = colorRGB;
		}

		Jump(X - 1, Y, OutX, OutY, G, OutG, colorRGB, dir);
	}

	// ��	
	else if (dir == RR)
	{
		G += dStraight_Line;

		// ���� ��尡 ���������� üũ
		if (FinNode->m_iX == X && FinNode->m_iY == Y)
		{
			*OutX = X;
			*OutY = Y;
			*OutG = G;
			return true;
		}

		// ���� ��尡 �ڳ����� üũ
		else if ((stMapArrayCopy[Y - 1][X].m_eTileType == OBSTACLE && stMapArrayCopy[Y - 1][X + 1].m_eTileType != OBSTACLE) ||
				(stMapArrayCopy[Y + 1][X].m_eTileType == OBSTACLE && stMapArrayCopy[Y + 1][X + 1].m_eTileType != OBSTACLE))
		{
			*OutX = X;
			*OutY = Y;
			*OutG = G;
			return true;
		}

		if (stMapArrayCopy[Y][X].m_eTileType != START && stMapArrayCopy[Y][X].m_eTileType != FIN &&
			stMapArrayCopy[Y][X].m_eTileType != OPEN_LIST && stMapArrayCopy[Y][X].m_eTileType != CLOSE_LIST)
		{
			stMapArrayCopy[Y][X].m_eTileType = JUMP;
			stMapArrayCopy[Y][X].colorRGB = colorRGB;
		}

		Jump(X + 1, Y, OutX, OutY, G, OutG, colorRGB, dir);
	}

	// �»�
	else if (dir == LU)
	{
		G += dDiagonal_Line;
		// ���� ��尡 ���������� üũ
		if (FinNode->m_iX == X && FinNode->m_iY == Y)
		{
			*OutX = X;
			*OutY = Y;
			*OutG = G;
			return true;
		}

		// ���� ��ġ�� �ڳ����� üũ
		else if ((stMapArrayCopy[Y][X + 1].m_eTileType == OBSTACLE && stMapArrayCopy[Y - 1][X + 1].m_eTileType != OBSTACLE) ||
			(stMapArrayCopy[Y + 1][X].m_eTileType == OBSTACLE && stMapArrayCopy[Y + 1][X - 1].m_eTileType != OBSTACLE))
		{
			*OutX = X;
			*OutY = Y;
			*OutG = G;
			return true;
		}

		// ���� ���� �Ÿ��� �ڳʰ� �ִ��� üũ
		else if (Jump(X - 1, Y, OutX, OutY, G, OutG, colorRGB, LL) == true)
		{
			*OutX = X;
			*OutY = Y;
			*OutG = G;
			return true;
		}

		// �� �����Ÿ��� �ڳʰ� �ִ��� üũ
		else if (Jump(X, Y - 1, OutX, OutY, G, OutG, colorRGB, UU) == true)
		{
			*OutX = X;
			*OutY = Y;
			*OutG = G;
			return true;
		}

		if (stMapArrayCopy[Y][X].m_eTileType != START && stMapArrayCopy[Y][X].m_eTileType != FIN &&
			stMapArrayCopy[Y][X].m_eTileType != OPEN_LIST && stMapArrayCopy[Y][X].m_eTileType != CLOSE_LIST)
		{
			stMapArrayCopy[Y][X].m_eTileType = JUMP;
			stMapArrayCopy[Y][X].colorRGB = colorRGB;
		}
		Jump(X - 1, Y - 1, OutX, OutY, G, OutG, colorRGB, dir);
	}

	// ����
	else if (dir == LD)
	{
		G += dDiagonal_Line;
		// ���� ��尡 ���������� üũ
		if (FinNode->m_iX == X && FinNode->m_iY == Y)
		{
			*OutX = X;
			*OutY = Y;
			*OutG = G;
			return true;
		}

		// ���� ��ġ�� �ڳ����� üũ
		else if ((stMapArrayCopy[Y - 1][X].m_eTileType == OBSTACLE && stMapArrayCopy[Y - 1][X - 1].m_eTileType != OBSTACLE) ||
			(stMapArrayCopy[Y][X + 1].m_eTileType == OBSTACLE && stMapArrayCopy[Y + 1][X + 1].m_eTileType != OBSTACLE))
		{
			*OutX = X;
			*OutY = Y;
			*OutG = G;
			return true;
		}

		// ���� ���� �Ÿ��� �ڳʰ� �ִ��� üũ
		else if (Jump(X - 1, Y, OutX, OutY, G, OutG, colorRGB, LL) == true)
		{
			*OutX = X;
			*OutY = Y;
			*OutG = G;
			return true;
		}

		// �Ʒ� �����Ÿ��� �ڳʰ� �ִ��� üũ
		else if (Jump(X, Y + 1, OutX, OutY, G, OutG, colorRGB, DD) == true)
		{
			*OutX = X;
			*OutY = Y;
			*OutG = G;
			return true;
		}

		if (stMapArrayCopy[Y][X].m_eTileType != START && stMapArrayCopy[Y][X].m_eTileType != FIN &&
			stMapArrayCopy[Y][X].m_eTileType != OPEN_LIST && stMapArrayCopy[Y][X].m_eTileType != CLOSE_LIST)
		{
			stMapArrayCopy[Y][X].m_eTileType = JUMP;
			stMapArrayCopy[Y][X].colorRGB = colorRGB;
		}

		Jump(X - 1, Y + 1, OutX, OutY, G, OutG, colorRGB, dir);
	}

	// ���
	else if (dir == RU)
	{
		G += dDiagonal_Line;
		// ���� ��尡 ���������� üũ
		if (FinNode->m_iX == X && FinNode->m_iY == Y)
		{
			*OutX = X;
			*OutY = Y;
			*OutG = G;
			return true;
		}

		// ���� ��ġ�� �ڳ����� üũ
		else if ((stMapArrayCopy[Y][X - 1].m_eTileType == OBSTACLE && stMapArrayCopy[Y - 1][X - 1].m_eTileType != OBSTACLE) ||
			(stMapArrayCopy[Y + 1][X].m_eTileType == OBSTACLE && stMapArrayCopy[Y + 1][X + 1].m_eTileType != OBSTACLE))
		{
			*OutX = X;
			*OutY = Y;
			*OutG = G;
			return true;
		}

		// ������ ���� �Ÿ��� �ڳʰ� �ִ��� üũ
		else if (Jump(X + 1, Y, OutX, OutY, G, OutG, colorRGB, RR) == true)
		{
			*OutX = X;
			*OutY = Y;
			*OutG = G;
			return true;
		}

		// �� �����Ÿ��� �ڳʰ� �ִ��� üũ
		else if (Jump(X, Y - 1, OutX, OutY, G, OutG, colorRGB, UU) == true)
		{
			*OutX = X;
			*OutY = Y;
			*OutG = G;
			return true;
		}

		if (stMapArrayCopy[Y][X].m_eTileType != START && stMapArrayCopy[Y][X].m_eTileType != FIN &&
			stMapArrayCopy[Y][X].m_eTileType != OPEN_LIST && stMapArrayCopy[Y][X].m_eTileType != CLOSE_LIST)
		{
			stMapArrayCopy[Y][X].m_eTileType = JUMP;
			stMapArrayCopy[Y][X].colorRGB = colorRGB;
		}

		Jump(X + 1, Y - 1, OutX, OutY, G, OutG, colorRGB, dir);
	}

	// ����
	else if (dir == RD)
	{
		G += dDiagonal_Line;

		// ���� ��尡 ���������� üũ
		if (FinNode->m_iX == X && FinNode->m_iY == Y)
		{
			*OutX = X;
			*OutY = Y;
			*OutG = G;
			return true;
		}

		// ���� ��ġ�� �ڳ����� üũ
		else if ((stMapArrayCopy[Y][X - 1].m_eTileType == OBSTACLE && stMapArrayCopy[Y + 1][X - 1].m_eTileType != OBSTACLE) ||
			(stMapArrayCopy[Y - 1][X].m_eTileType == OBSTACLE && stMapArrayCopy[Y - 1][X + 1].m_eTileType != OBSTACLE))
		{
			*OutX = X;
			*OutY = Y;
			*OutG = G;
			return true;
		}

		// ������ ���� �Ÿ��� �ڳʰ� �ִ��� üũ
		else if (Jump(X + 1, Y, OutX, OutY, G, OutG, colorRGB, RR) == true)
		{
			*OutX = X;
			*OutY = Y;
			*OutG = G;
			return true;
		}

		// �Ʒ� �����Ÿ��� �ڳʰ� �ִ��� üũ
		else if (Jump(X, Y + 1, OutX, OutY, G, OutG, colorRGB, DD) == true)
		{
			*OutX = X;
			*OutY = Y;
			*OutG = G;
			return true;
		}

		if (stMapArrayCopy[Y][X].m_eTileType != START && stMapArrayCopy[Y][X].m_eTileType != FIN &&
			stMapArrayCopy[Y][X].m_eTileType != OPEN_LIST && stMapArrayCopy[Y][X].m_eTileType != CLOSE_LIST)
		{
			stMapArrayCopy[Y][X].m_eTileType = JUMP;
			stMapArrayCopy[Y][X].colorRGB = colorRGB;
		}

		Jump(X + 1, Y + 1, OutX, OutY, G, OutG, colorRGB, dir);
	}
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
		//if (SearchNode->m_fG > (parent->m_fG + (fabs(parent->m_iX - iX) + fabs(parent->m_iY - iY))))
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
	if (stMapArrayCopy[iY][iX].m_eTileType != START && stMapArrayCopy[iY][iX].m_eTileType != FIN && stMapArrayCopy[iY][iX].m_eTileType != OBSTACLE)
		stMapArrayCopy[iY][iX].m_eTileType = OPEN_LIST;

	m_OpenList.Insert(NewNode);

	// 4. ���ĵ� �ٽ��Ѵ�.
	m_OpenList.Sort();
}

// �׸���
void CFindSearch::Show(stFindSearchNode* Node, int StartX, int StartY, int FinX, int FinY)
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
				hMyBrush = CreateSolidBrush(RGB(80, 80, 80));
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

			// CLOSE_LIST�� ��ο� �Ķ���
			else if (stMapArrayCopy[i][j].m_eTileType == CLOSE_LIST)
			{
				hMyBrush = CreateSolidBrush(RGB(0, 0, 127));
				hOldBrush = (HBRUSH)SelectObject(MemDC, hMyBrush);
				bBrushCheck = true;
			}

			// JUMP�� Ÿ�Ͽ� ������ �� ���
			else if (stMapArrayCopy[i][j].m_eTileType == JUMP)
			{
				hMyBrush = CreateSolidBrush(stMapArrayCopy[i][j].colorRGB);
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
	if (Node != nullptr)
	{
		MoveToEx(MemDC, stMapArrayCopy[Node->m_iY][Node->m_iX].m_iMapX,
			stMapArrayCopy[Node->m_iY][Node->m_iX].m_iMapY, NULL);

		HPEN hMyPen = CreatePen(PS_SOLID, 3, RGB(255, 0, 0));
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

	 //�극���� �˰���.
	if (StartX != -1)
	{
		// ���� X,Y��ġ ����
		int X = StartX, Y = StartY;

		// ����, Y Ȥ�� X�� ������ų ���� ����.
		int Error = 0;

		// X,Y�� ������ų ��.
		int AddX, AddY;

		// ���������� ������������ ���̿� ����.
		int Width = FinX - StartX;
		int Height = FinY - StartY;

		//  AddX, AddY ����
		if (Width < 0)
		{
			Width = -Width;
			AddX = -1;
		}
		else
			AddX = 1;

		if (Height < 0)
		{
			Height = -Height;
			AddY = -1;
		}
		else
			AddY = 1;

		
		// ���̰� ���̺���, ũ�ų� ���� ���
		if (Width >= Height)
		{		
			for (int i = 0; i < Width; ++i)
			{
				X += AddX;
				Error += Height;

				if (Error >= Width)
				{
					Y += AddY;
					Error -= Width;
				}

				SetPixel(MemDC, X, Y, RGB(0, 0, 0));
			}
		}		

		// ���̰� ���̺���, Ŭ ���
		else
		{		
			for (int i = 0; i < Height; ++i)
			{
				Y += AddY;
				Error += Width;

				if (Error >= Height)
				{
					X += AddX;
					Error -= Height;
				}

				SetPixel(MemDC, X, Y, RGB(0, 0, 0));
			}
		}
	}

	BitBlt(hdc, rt.left, rt.top, rt.right, rt.bottom, MemDC, rt.left, rt.top, SRCCOPY);

	DeleteObject(hMyBitmap);
	DeleteObject(hOldBitmap);
	DeleteDC(MemDC);
	ReleaseDC(hWnd, hdc);
}

// Jump_Point_Search_While()�� �Ϸ�� ��, ����/���� ����Ʈ, Ÿ�̸� �� �����ϱ�
void CFindSearch::LastClear()
{
	// 1. �� ������, Open����Ʈ�� Close����Ʈ �ʱ�ȭ
	//m_CloseList.Clear();
	//m_OpenList.Clear();

	// 2. FinNode�� �Ҹ� �ȵ����� �Ҹ��Ų��.
	delete[] FinNode;

	// Ÿ�̸ӵ� ������.
	KillTimer(hWnd, 1);

	ColorKey = 1;
}