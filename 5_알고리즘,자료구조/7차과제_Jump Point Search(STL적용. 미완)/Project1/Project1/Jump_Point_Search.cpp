#include "stdafx.h"
#include "Jump_Point_Search.h"
#include <math.h>

// ������
CFindSearch::CFindSearch(Map(*Copy)[MAP_WIDTH])
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
	StartNode->dir = NONE;

	// �������� G,H,F ����
	FinNode->m_fG = 0;
	FinNode->m_fH = 0;
	FinNode->m_fF = 0;
	FinNode->stpParent = nullptr;
	FinNode->dir = NONE;
}

// �ݺ��� ���鼭 ���� ����Ʈ ��ġ ����
void CFindSearch::Jump_Point_Search_While()
{
	// 1. ���� ��, �������� ���¸���Ʈ�� �ִ´�.
	//m_OpenList.Insert(StartNode);
	m_OpenList.push_back(StartNode);

	// 2. �ݺ� ����
	// ���� ����Ʈ�� �� ������ �ݺ�.
	//while (m_OpenList.GetCount() != 0)
	while (!m_OpenList.empty())
	{
		// 2-1. ���¸���Ʈ�� ������� �ʴٸ�, ���� ����Ʈ���� ��� �ϳ� ����.
		//stFindSearchNode* Node = m_OpenList.GetListNode();
		stFindSearchNode* Node = m_OpenList.front();
		m_OpenList.pop_front();

		// 2-2. ���� ��尡 ��������� break;
		if (Node->m_iX == FinNode->m_iX &&
			Node->m_iY == FinNode->m_iY)
		{
			// Node�� �������̴�, ������� �θ� Ÿ���鼭 ���� �ߴ´�.
			Show(Node);
			break;
		}

		// 2-3. ������ ��尡 �ƴϸ� ���� ��带 CloseList�� �ֱ�
		//m_CloseList.Insert(Node);
		m_CloseList.push_back(Node);

		if (stMapArrayCopy[Node->m_iY][Node->m_iX].m_eTileType == OPEN_LIST)
			stMapArrayCopy[Node->m_iY][Node->m_iX].m_eTileType = CLOSE_LIST;

		// 2-4. �׸��� ��� ���� �Լ� ȣ��
		// �� �ȿ��� ��Ȳ�� ���� Ÿ�� ����.
		CreateNodeCheck(Node);
	}

	// 3. �� ������, Open����Ʈ�� Close����Ʈ �ʱ�ȭ
	//m_CloseList.Clear();
	//m_OpenList.Clear();
	LISTNODE::iterator iter;
	size_t Idx;
	size_t IdxMax;

	iter = m_CloseList.begin();
	Idx = 0;
	IdxMax = m_CloseList.size();
	while (Idx < IdxMax)
	{
		delete (*iter);
		iter = m_CloseList.erase(iter);

		++Idx;
	}

	iter = m_OpenList.begin();
	Idx = 0;
	IdxMax = m_OpenList.size();
	while (Idx < IdxMax)
	{
		delete (*iter);
		iter = m_OpenList.erase(iter);

		++Idx;
	}


	// 4. FinNode�� �Ҹ� �ȵ����� �Ҹ��Ų��.
	delete[] FinNode;
}

// ���ڷ� ���� ��带 ��������, ���� ����
void CFindSearch::CreateNodeCheck(stFindSearchNode* Node)
{
	int Time = 500;

	// 1. ���ڷ� ���� ��尡 ������ �����, 8�������� Jump�ϸ鼭 ��� ���� ���� üũ
	if (Node->stpParent == nullptr)
	{
		// LL
		CheckCreateJump(Node->m_iX -1, Node->m_iY, Node, LL);
		Show();		
		Sleep(Time);		

		// RR
		CheckCreateJump(Node->m_iX + 1, Node->m_iY, Node, RR);
		Show();		
		Sleep(Time);

		// UU
		CheckCreateJump(Node->m_iX, Node->m_iY - 1, Node, UU);
		Show();	
		Sleep(Time);

		// DD
		CheckCreateJump(Node->m_iX, Node->m_iY + 1, Node, DD);
		Show();	
		Sleep(Time);


		// LU
		CheckCreateJump(Node->m_iX -1, Node->m_iY -1, Node, LU);
		Show();	
		Sleep(Time);

		// LD
		CheckCreateJump(Node->m_iX -1, Node->m_iY +1, Node, LD);	
		Show();	
		Sleep(Time);

		// RU
		CheckCreateJump(Node->m_iX +1, Node->m_iY -1, Node, RU);
		Show();	
		Sleep(Time);

		// RD
		CheckCreateJump(Node->m_iX +1, Node->m_iY +1, Node, RD);
		Show();
		Sleep(Time);
		
	}

	// �װ� �ƴ϶��, ���� / �밢���� ���� ��� ���� ���� üũ
	else
	{
		// ��
		if (Node->dir == UU)
		{
			// �� ����
			CheckCreateJump(Node->m_iX, Node->m_iY - 1, Node, Node->dir);

			// �»� üũ
			if (stMapArrayCopy[Node->m_iY][Node->m_iX - 1].m_eTileType == OBSTACLE && stMapArrayCopy[Node->m_iY - 1][Node->m_iX - 1].m_eTileType != OBSTACLE)
				CheckCreateJump(Node->m_iX - 1, Node->m_iY - 1, Node, LU);

			// ��� üũ
			if (stMapArrayCopy[Node->m_iY][Node->m_iX + 1].m_eTileType == OBSTACLE && stMapArrayCopy[Node->m_iY - 1][Node->m_iX + 1].m_eTileType != OBSTACLE)
				CheckCreateJump(Node->m_iX + 1, Node->m_iY - 1, Node, RU);

			Show();			
		}

		// ��
		else if (Node->dir == DD)
		{
			// �� ����
			CheckCreateJump(Node->m_iX, Node->m_iY + 1, Node, Node->dir);

			// ���� üũ
			if (stMapArrayCopy[Node->m_iY][Node->m_iX - 1].m_eTileType == OBSTACLE && stMapArrayCopy[Node->m_iY + 1][Node->m_iX - 1].m_eTileType != OBSTACLE)
				CheckCreateJump(Node->m_iX - 1, Node->m_iY + 1, Node, LD);

			// ���� üũ
			if (stMapArrayCopy[Node->m_iY][Node->m_iX + 1].m_eTileType == OBSTACLE && stMapArrayCopy[Node->m_iY + 1][Node->m_iX + 1].m_eTileType != OBSTACLE)
				CheckCreateJump(Node->m_iX + 1, Node->m_iY + 1, Node, RD);

			Show();	
		}

		// ��
		else if (Node->dir == LL)
		{
			// �� ����
			CheckCreateJump(Node->m_iX - 1, Node->m_iY, Node, Node->dir);

			// �»� üũ
			if (stMapArrayCopy[Node->m_iY -1][Node->m_iX].m_eTileType == OBSTACLE && stMapArrayCopy[Node->m_iY -1][Node->m_iX -1].m_eTileType != OBSTACLE)
				CheckCreateJump(Node->m_iX -1, Node->m_iY -1, Node, LU);

			// ���� üũ
			if (stMapArrayCopy[Node->m_iY +1][Node->m_iX].m_eTileType == OBSTACLE && stMapArrayCopy[Node->m_iY +1][Node->m_iX -1].m_eTileType != OBSTACLE)
				CheckCreateJump(Node->m_iX -1, Node->m_iY +1, Node, LD);

			Show();			
		}

		// ��
		else if (Node->dir == RR)
		{
			// �� ����
			CheckCreateJump(Node->m_iX + 1, Node->m_iY, Node, Node->dir);

			// ��� üũ
			if (stMapArrayCopy[Node->m_iY - 1][Node->m_iX].m_eTileType == OBSTACLE && stMapArrayCopy[Node->m_iY - 1][Node->m_iX + 1].m_eTileType != OBSTACLE)
				CheckCreateJump(Node->m_iX + 1, Node->m_iY - 1, Node, RU);

			// ���� üũ
			if (stMapArrayCopy[Node->m_iY + 1][Node->m_iX].m_eTileType == OBSTACLE && stMapArrayCopy[Node->m_iY + 1][Node->m_iX + 1].m_eTileType != OBSTACLE)
				CheckCreateJump(Node->m_iX + 1, Node->m_iY + 1, Node, RD);

			Show();			
		}

		// �»�
		else if (Node->dir == LU)
		{
			// ���� ����
			CheckCreateJump(Node->m_iX - 1, Node->m_iY, Node, LL);

			// ���� ����
			CheckCreateJump(Node->m_iX, Node->m_iY - 1, Node, UU);

			// �»� �밢��
			CheckCreateJump(Node->m_iX - 1, Node->m_iY - 1, Node, Node->dir);

			// ��� �밢��
			if (stMapArrayCopy[Node->m_iY][Node->m_iX + 1].m_eTileType == OBSTACLE && stMapArrayCopy[Node->m_iY - 1][Node->m_iX + 1].m_eTileType != OBSTACLE)
				CheckCreateJump(Node->m_iX + 1, Node->m_iY - 1, Node, RU);

			// ���� �밢�� 
			if (stMapArrayCopy[Node->m_iY + 1][Node->m_iX].m_eTileType == OBSTACLE && stMapArrayCopy[Node->m_iY + 1][Node->m_iX - 1].m_eTileType != OBSTACLE)
				CheckCreateJump(Node->m_iX - 1, Node->m_iY + 1, Node, LD);

			Show();		
		}

		// ����
		else if (Node->dir == LD)
		{
			// ���� ����
			CheckCreateJump(Node->m_iX - 1, Node->m_iY, Node, LL);

			// �Ʒ��� ����
			CheckCreateJump(Node->m_iX, Node->m_iY + 1, Node, DD);

			// ���� �밢��
			CheckCreateJump(Node->m_iX - 1, Node->m_iY + 1, Node, Node->dir);

			// �»� �밢��
			if (stMapArrayCopy[Node->m_iY - 1][Node->m_iX].m_eTileType == OBSTACLE && stMapArrayCopy[Node->m_iY - 1][Node->m_iX - 1].m_eTileType != OBSTACLE)
				CheckCreateJump(Node->m_iX - 1, Node->m_iY - 1, Node, LU);

			// ���� �밢�� 
			if (stMapArrayCopy[Node->m_iY][Node->m_iX + 1].m_eTileType == OBSTACLE && stMapArrayCopy[Node->m_iY + 1][Node->m_iX + 1].m_eTileType != OBSTACLE)
				CheckCreateJump(Node->m_iX + 1, Node->m_iY + 1, Node, RD);

			Show();	
		}

		// ���
		else if (Node->dir == RU)
		{
			// ������ ����
			CheckCreateJump(Node->m_iX + 1, Node->m_iY, Node, RR);

			// ���� ����
			CheckCreateJump(Node->m_iX, Node->m_iY - 1, Node, UU);

			// ��� �밢��
			CheckCreateJump(Node->m_iX + 1, Node->m_iY - 1, Node, Node->dir);

			// �»� �밢��
			if (stMapArrayCopy[Node->m_iY][Node->m_iX - 1].m_eTileType == OBSTACLE && stMapArrayCopy[Node->m_iY - 1][Node->m_iX - 1].m_eTileType != OBSTACLE)
				CheckCreateJump(Node->m_iX - 1, Node->m_iY - 1, Node, LU);

			// ���� �밢�� 
			if (stMapArrayCopy[Node->m_iY + 1][Node->m_iX].m_eTileType == OBSTACLE && stMapArrayCopy[Node->m_iY + 1][Node->m_iX + 1].m_eTileType != OBSTACLE)
				CheckCreateJump(Node->m_iX + 1, Node->m_iY + 1, Node, RD);

			Show();		
		}

		// ����
		else if (Node->dir == RD)
		{
			// ������ ����
			CheckCreateJump(Node->m_iX + 1, Node->m_iY, Node, RR);

			// �Ʒ��� ����
			CheckCreateJump(Node->m_iX, Node->m_iY + 1, Node, DD);

			// ���� �밢��
			CheckCreateJump(Node->m_iX + 1, Node->m_iY + 1, Node, Node->dir);

			// ��� �밢��
			if (stMapArrayCopy[Node->m_iY - 1][Node->m_iX].m_eTileType == OBSTACLE && stMapArrayCopy[Node->m_iY - 1][Node->m_iX + 1].m_eTileType != OBSTACLE)
				CheckCreateJump(Node->m_iX + 1, Node->m_iY - 1, Node, RU);

			// ���� �밢�� 
			if (stMapArrayCopy[Node->m_iY][Node->m_iX - 1].m_eTileType == OBSTACLE && stMapArrayCopy[Node->m_iY + 1][Node->m_iX - 1].m_eTileType != OBSTACLE)
				CheckCreateJump(Node->m_iX - 1, Node->m_iY + 1, Node, LD);

			Show();					
		}

		Sleep(Time);
	}		
}

// ���ڷ� ���� �������� �����ϸ鼭, ������ ��尡 ������ �����Ѵ�.
void CFindSearch::CheckCreateJump(int X, int Y, stFindSearchNode* parent, Direction dir)
{
	int OutX = 0, OutY = 0;
	bool Check = Jump(X, Y, &OutX, &OutY, dir);
	
	if (Check == true)
		CreateNode(OutX, OutY, parent, dir);
}

// ����
// �ش� ���⿡ ������ ��尡 �ִ��� üũ(���)
bool CFindSearch::Jump(int X, int Y, int* OutX, int* OutY, Direction dir)
{
	// 1. �ش� ��ǥ�� ���� ��� ���� ���� ��ǥ���� üũ (�� ������, ���ع�����)
	if (stMapArrayCopy[Y][X].m_eTileType == OBSTACLE ||
		X < 0 || X >= MAP_WIDTH || Y < 0 || Y >= MAP_HEIGHT)
		return false;

	// 2. ���⿡ ���� ����
	// ��	
	if (dir == UU)
	{
		// ���� ��尡 ���������� üũ
		if (FinNode->m_iX == X && FinNode->m_iY == Y)
		{
			*OutX = X;
			*OutY = Y;
			return true;
		}

		// �������� �ƴ϶��, ���� ��尡 �ڳ����� üũ
		else if ((stMapArrayCopy[Y][X - 1].m_eTileType == OBSTACLE && stMapArrayCopy[Y - 1][X - 1].m_eTileType != OBSTACLE) ||
				(stMapArrayCopy[Y][X + 1].m_eTileType == OBSTACLE && stMapArrayCopy[Y - 1][X + 1].m_eTileType != OBSTACLE))
		{
			*OutX = X;
			*OutY = Y;
			return true;
		}

		// �ڳʰ� �ƴ϶��, �ش� ��ġ�� Ÿ���� JUMP�� �ٲ۴�. Ȯ���ߴܴ� �ǹ̷�! 
		// �ٵ� ������/������/���¸���Ʈ/��������Ʈ�� ������ �ȹٲ۴�. 
		// �׵��� �����ϰ� ���� �����Ǿ�� �Ѵ�.
		if (stMapArrayCopy[Y][X].m_eTileType != START && stMapArrayCopy[Y][X].m_eTileType != FIN &&
			stMapArrayCopy[Y][X].m_eTileType != OPEN_LIST && stMapArrayCopy[Y][X].m_eTileType != CLOSE_LIST)
		{
			stMapArrayCopy[Y][X].m_eTileType = JUMP;
		}	

		Jump(X, Y - 1, OutX, OutY, dir);
	}

	// ��	
	else if (dir == DD)
	{

		// ���� ��尡 ���������� üũ
		if (FinNode->m_iX == X && FinNode->m_iY == Y)
		{
			*OutX = X;
			*OutY = Y;
			return true;
		}

		// ���� ��尡 �ڳ����� üũ
		else if ((stMapArrayCopy[Y][X - 1].m_eTileType == OBSTACLE && stMapArrayCopy[Y + 1][X - 1].m_eTileType != OBSTACLE) ||
				(stMapArrayCopy[Y][X + 1].m_eTileType == OBSTACLE && stMapArrayCopy[Y + 1][X + 1].m_eTileType != OBSTACLE))
		{
			*OutX = X;
			*OutY = Y;
			return true;
		}

		if (stMapArrayCopy[Y][X].m_eTileType != START && stMapArrayCopy[Y][X].m_eTileType != FIN &&
			stMapArrayCopy[Y][X].m_eTileType != OPEN_LIST && stMapArrayCopy[Y][X].m_eTileType != CLOSE_LIST)
		{
			stMapArrayCopy[Y][X].m_eTileType = JUMP;
		}	

		Jump(X, Y + 1, OutX, OutY, dir);
	}


	//��	
	else if (dir == LL)
	{
		// ���� ��尡 ���������� üũ
		if (FinNode->m_iX == X && FinNode->m_iY == Y)
		{
			*OutX = X;
			*OutY = Y;
			return true;
		}

		// ���� ��尡 �ڳ����� üũ
		else if ((stMapArrayCopy[Y - 1][X].m_eTileType == OBSTACLE && stMapArrayCopy[Y - 1][X - 1].m_eTileType != OBSTACLE) ||
				(stMapArrayCopy[Y + 1][X].m_eTileType == OBSTACLE && stMapArrayCopy[Y + 1][X - 1].m_eTileType != OBSTACLE))
		{
			*OutX = X;
			*OutY = Y;
			return true;
		}

		if (stMapArrayCopy[Y][X].m_eTileType != START && stMapArrayCopy[Y][X].m_eTileType != FIN &&
			stMapArrayCopy[Y][X].m_eTileType != OPEN_LIST && stMapArrayCopy[Y][X].m_eTileType != CLOSE_LIST)
		{
			stMapArrayCopy[Y][X].m_eTileType = JUMP;
		}
		

		Jump(X - 1, Y, OutX, OutY, dir);
	}

	// ��	
	else if (dir == RR)
	{
		// ���� ��尡 ���������� üũ
		if (FinNode->m_iX == X && FinNode->m_iY == Y)
		{
			*OutX = X;
			*OutY = Y;
			return true;
		}

		// ���� ��尡 �ڳ����� üũ
		else if ((stMapArrayCopy[Y - 1][X].m_eTileType == OBSTACLE && stMapArrayCopy[Y - 1][X + 1].m_eTileType != OBSTACLE) ||
				(stMapArrayCopy[Y + 1][X].m_eTileType == OBSTACLE && stMapArrayCopy[Y + 1][X + 1].m_eTileType != OBSTACLE))
		{
			*OutX = X;
			*OutY = Y;
			return true;
		}

		if (stMapArrayCopy[Y][X].m_eTileType != START && stMapArrayCopy[Y][X].m_eTileType != FIN &&
			stMapArrayCopy[Y][X].m_eTileType != OPEN_LIST && stMapArrayCopy[Y][X].m_eTileType != CLOSE_LIST)
		{
			stMapArrayCopy[Y][X].m_eTileType = JUMP;
		}
		

		Jump(X + 1, Y, OutX, OutY, dir);
	}

	// �»�
	else if (dir == LU)
	{
		// ���� ��尡 ���������� üũ
		if (FinNode->m_iX == X && FinNode->m_iY == Y)
		{
			*OutX = X;
			*OutY = Y;
			return true;
		}

		// ���� ��ġ�� �ڳ����� üũ
		else if ((stMapArrayCopy[Y][X + 1].m_eTileType == OBSTACLE && stMapArrayCopy[Y - 1][X + 1].m_eTileType != OBSTACLE) ||
			(stMapArrayCopy[Y + 1][X].m_eTileType == OBSTACLE && stMapArrayCopy[Y + 1][X - 1].m_eTileType != OBSTACLE))
		{
			*OutX = X;
			*OutY = Y;
			return true;
		}

		// ���� ���� �Ÿ��� �ڳʰ� �ִ��� üũ
		else if (Jump(X - 1, Y, OutX, OutY, LL) == true)
		{
			*OutX = X;
			*OutY = Y;
			return true;
		}

		// �� �����Ÿ��� �ڳʰ� �ִ��� üũ
		else if (Jump(X, Y - 1, OutX, OutY, UU) == true)
		{
			*OutX = X;
			*OutY = Y;
			return true;
		}

		if (stMapArrayCopy[Y][X].m_eTileType != START && stMapArrayCopy[Y][X].m_eTileType != FIN &&
			stMapArrayCopy[Y][X].m_eTileType != OPEN_LIST && stMapArrayCopy[Y][X].m_eTileType != CLOSE_LIST)
		{
			stMapArrayCopy[Y][X].m_eTileType = JUMP;
		}
		

		Jump(X - 1, Y - 1, OutX, OutY, dir);
	}

	// ����
	else if (dir == LD)
	{
		// ���� ��尡 ���������� üũ
		if (FinNode->m_iX == X && FinNode->m_iY == Y)
		{
			*OutX = X;
			*OutY = Y;
			return true;
		}

		// ���� ��ġ�� �ڳ����� üũ
		else if ((stMapArrayCopy[Y - 1][X].m_eTileType == OBSTACLE && stMapArrayCopy[Y - 1][X - 1].m_eTileType != OBSTACLE) ||
			(stMapArrayCopy[Y][X + 1].m_eTileType == OBSTACLE && stMapArrayCopy[Y + 1][X + 1].m_eTileType != OBSTACLE))
		{
			*OutX = X;
			*OutY = Y;
			return true;
		}

		// ���� ���� �Ÿ��� �ڳʰ� �ִ��� üũ
		else if (Jump(X - 1, Y, OutX, OutY, LL) == true)
		{
			*OutX = X;
			*OutY = Y;
			return true;
		}

		// �Ʒ� �����Ÿ��� �ڳʰ� �ִ��� üũ
		else if (Jump(X, Y + 1, OutX, OutY, DD) == true)
		{
			*OutX = X;
			*OutY = Y;
			return true;
		}

		if (stMapArrayCopy[Y][X].m_eTileType != START && stMapArrayCopy[Y][X].m_eTileType != FIN &&
			stMapArrayCopy[Y][X].m_eTileType != OPEN_LIST && stMapArrayCopy[Y][X].m_eTileType != CLOSE_LIST)
		{
			stMapArrayCopy[Y][X].m_eTileType = JUMP;
		}
		

		Jump(X - 1, Y + 1, OutX, OutY, dir);
	}

	// ���
	else if (dir == RU)
	{
		// ���� ��尡 ���������� üũ
		if (FinNode->m_iX == X && FinNode->m_iY == Y)
		{
			*OutX = X;
			*OutY = Y;
			return true;
		}

		// ���� ��ġ�� �ڳ����� üũ
		else if ((stMapArrayCopy[Y][X - 1].m_eTileType == OBSTACLE && stMapArrayCopy[Y - 1][X - 1].m_eTileType != OBSTACLE) ||
			(stMapArrayCopy[Y + 1][X].m_eTileType == OBSTACLE && stMapArrayCopy[Y + 1][X + 1].m_eTileType != OBSTACLE))
		{
			*OutX = X;
			*OutY = Y;
			return true;
		}

		// ������ ���� �Ÿ��� �ڳʰ� �ִ��� üũ
		else if (Jump(X + 1, Y, OutX, OutY, RR) == true)
		{
			*OutX = X;
			*OutY = Y;
			return true;
		}

		// �� �����Ÿ��� �ڳʰ� �ִ��� üũ
		else if (Jump(X, Y - 1, OutX, OutY, UU) == true)
		{
			*OutX = X;
			*OutY = Y;
			return true;
		}

		if (stMapArrayCopy[Y][X].m_eTileType != START && stMapArrayCopy[Y][X].m_eTileType != FIN &&
			stMapArrayCopy[Y][X].m_eTileType != OPEN_LIST && stMapArrayCopy[Y][X].m_eTileType != CLOSE_LIST)
		{
			stMapArrayCopy[Y][X].m_eTileType = JUMP;
		}
		

		Jump(X + 1, Y - 1, OutX, OutY, dir);
	}

	// ����
	else if (dir == RD)
	{
		// ���� ��尡 ���������� üũ
		if (FinNode->m_iX == X && FinNode->m_iY == Y)
		{
			*OutX = X;
			*OutY = Y;
			return true;
		}

		// ���� ��ġ�� �ڳ����� üũ
		else if ((stMapArrayCopy[Y][X - 1].m_eTileType == OBSTACLE && stMapArrayCopy[Y + 1][X - 1].m_eTileType != OBSTACLE) ||
			(stMapArrayCopy[Y - 1][X].m_eTileType == OBSTACLE && stMapArrayCopy[Y - 1][X + 1].m_eTileType != OBSTACLE))
		{
			*OutX = X;
			*OutY = Y;
			return true;
		}

		// ������ ���� �Ÿ��� �ڳʰ� �ִ��� üũ
		else if (Jump(X + 1, Y, OutX, OutY, RR) == true)
		{
			*OutX = X;
			*OutY = Y;
			return true;
		}

		// �Ʒ� �����Ÿ��� �ڳʰ� �ִ��� üũ
		else if (Jump(X, Y + 1, OutX, OutY, DD) == true)
		{
			*OutX = X;
			*OutY = Y;
			return true;
		}

		if (stMapArrayCopy[Y][X].m_eTileType != START && stMapArrayCopy[Y][X].m_eTileType != FIN &&
			stMapArrayCopy[Y][X].m_eTileType != OPEN_LIST && stMapArrayCopy[Y][X].m_eTileType != CLOSE_LIST)
		{
			stMapArrayCopy[Y][X].m_eTileType = JUMP;
		}
		

		Jump(X + 1, Y + 1, OutX, OutY, dir);
	}
}

// ��� ���� �Լ�
void CFindSearch::CreateNode(int iX, int iY, stFindSearchNode* parent, Direction dir)
{	
	// 1. �ش� ��ǥ�� ��尡 Open ����Ʈ�� �ִ��� üũ
	//stFindSearchNode* SearchNode = m_OpenList.Search(iX, iY);
	LISTNODE::iterator iter;
	size_t Idx;
	size_t IdxMax;
	stFindSearchNode* SearchNode;

	SearchNode = NULL;
	iter = m_CloseList.begin();
	Idx = 0;
	IdxMax = m_CloseList.size();
	while (Idx < IdxMax)
	{
		if ((*iter)->m_iX == iX &&
			(*iter)->m_iY == iY)
		{
			SearchNode = (*iter);
			break;
		}

		++Idx;
		++iter;
	}

	if (SearchNode != nullptr)
	{
		// �ִٸ�, �ش� ����� G���� ���� ������ ����� G�� üũ.
		// �ش� ����� G���� �� ũ�ٸ�, 
		if (SearchNode->m_fG > (parent->m_fG + (fabs(parent->m_iX - iX) + fabs(parent->m_iY - iY))))
		{
			// parent�� �θ� ����
			SearchNode->stpParent = parent;

			// ���⵵ �缳��
			SearchNode->dir = dir;

			// G�� ����� (parent�� G�� + �θ���� �������� �Ÿ�)
			SearchNode->m_fG = parent->m_fG + (fabs(parent->m_iX - iX) + fabs(parent->m_iY - iY));

			// F�� �����.
			SearchNode->m_fF = (SearchNode->m_fG + SearchNode->m_fH);

			// ���ĵ� �ٽ��Ѵ�.
			//m_OpenList.Sort();
			m_OpenList.sort(NodeCompare);
		}

		return;
	}

	// 2. Close�� ����Ʈ�� �ִ����� üũ
	//SearchNode = m_CloseList.Search(iX, iY);
	SearchNode = NULL;
	iter = m_CloseList.begin();
	Idx = 0;
	IdxMax = m_CloseList.size();
	while (Idx < IdxMax)
	{
		if ((*iter)->m_iX == iX &&
			(*iter)->m_iY == iY)
		{
			SearchNode = (*iter);
			break;
		}

		++Idx;
		++iter;
	}

	if (SearchNode != nullptr)
	{
		// �ִٸ�, �ش� ����� G���� ���� ������ ����� G�� üũ.
		// �ش� ����� G���� �� ũ�ٸ�, 
		if (SearchNode->m_fG > (parent->m_fG + (fabs(parent->m_iX - iX) + fabs(parent->m_iY - iY))))
		{
			// parent�� �θ� ����
			SearchNode->stpParent = parent;

			// ���⵵ �缳��
			SearchNode->dir = dir;

			// G�� ����� (parent�� G�� + �θ���� �������� �Ÿ�)
			SearchNode->m_fG = parent->m_fG + (fabs(parent->m_iX - iX) + fabs(parent->m_iY - iY));

			// F�� �����.
			SearchNode->m_fF = (SearchNode->m_fG + SearchNode->m_fH);

			// ���ĵ� �ٽ��Ѵ�.
			//m_OpenList.Sort();
			m_OpenList.sort(NodeCompare);
		}

		return;
	}

	// 3. �� �ٿ� ������, ��� ���� ��, Open�� �ִ´�.
	stFindSearchNode* NewNode = new stFindSearchNode[sizeof(stFindSearchNode)];

	NewNode->stpParent = parent;
	NewNode->dir = dir;

	NewNode->m_iX = iX;
	NewNode->m_iY = iY;

	NewNode->m_fG = parent->m_fG + (fabs(parent->m_iX - iX) + fabs(parent->m_iY - iY));
	NewNode->m_fH = fabs(FinNode->m_iX - iX) + fabs(FinNode->m_iY - iY);
	NewNode->m_fF = NewNode->m_fG + NewNode->m_fH;

	// �ش� ��尡 ����,����, ���ع���尡 �ƴϸ�, OPEN_LIST�� ����
	if (stMapArrayCopy[iY][iX].m_eTileType != START && stMapArrayCopy[iY][iX].m_eTileType != FIN && stMapArrayCopy[iY][iX].m_eTileType != OBSTACLE)
		stMapArrayCopy[iY][iX].m_eTileType = OPEN_LIST;

	//m_OpenList.Insert(NewNode);
	m_OpenList.push_back(NewNode);

	// 4. ���ĵ� �ٽ��Ѵ�.
	//m_OpenList.Sort();
	m_OpenList.sort(NodeCompare);

	// 5. ����Ѵ�.
	Show();
}

// �׸���
void CFindSearch::Show(stFindSearchNode* Node)
{
	static stFindSearchNode* SaveNode = nullptr;

	if(Node!= nullptr)
		SaveNode = Node;

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

			// CLOSE_LIST�� ��ο� �Ķ���
			else if (stMapArrayCopy[i][j].m_eTileType == CLOSE_LIST)
			{
				hMyBrush = CreateSolidBrush(RGB(0, 0, 127));
				hOldBrush = (HBRUSH)SelectObject(MemDC, hMyBrush);
				bBrushCheck = true;
			}

			// JUMP�� ���� �����
			else if (stMapArrayCopy[i][j].m_eTileType == JUMP)
			{
				hMyBrush = CreateSolidBrush(RGB(255, 255, 0));
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
	if (SaveNode != nullptr)
	{
		MoveToEx(MemDC, stMapArrayCopy[SaveNode->m_iY][SaveNode->m_iX].m_iMapX,
			stMapArrayCopy[SaveNode->m_iY][SaveNode->m_iX].m_iMapY, NULL);

		HPEN hMyPen = CreatePen(PS_SOLID, 3, RGB(255, 0, 0));
		HPEN OldPen = (HPEN)SelectObject(MemDC, hMyPen);

		stFindSearchNode* ParentNode = SaveNode->stpParent;
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
	ReleaseDC(hWnd, hdc);
}
//
bool NodeCompare(CFindSearch::stFindSearchNode* _l, CFindSearch::stFindSearchNode* _r)
{
	return _l->m_fF < _r->m_fF;
}