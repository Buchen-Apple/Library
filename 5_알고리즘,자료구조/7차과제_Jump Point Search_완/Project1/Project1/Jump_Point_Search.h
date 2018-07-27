#ifndef __A_SEARCH_H__
#define __A_SEARCH_H__
#include "DLinked_List.h"
#include "MapTile.h"

class CFindSearch
{
	enum Direction
	{
		NONE = 0, LL, LU, LD, RR, RU, RD, UU, DD
	};

	struct stFindSearchNode
	{
		// ���⿡ ����Ǵ� x,y�� ��ϴ����� ����.
		int m_iX;
		int m_iY;

		// G : ��������� ���� �������� �Ÿ�.
		// H : ���� ������ ������������ �Ÿ�
		// F : G + H
		double m_fG;
		double m_fH;
		double m_fF;
		stFindSearchNode* stpParent;

		// ���� ����
		Direction dir;
	};

	COLORREF Color1;
	COLORREF Color2;
	COLORREF Color3;
	COLORREF Color4;
	COLORREF Color5;
	COLORREF Color6;
	COLORREF Color7;
	COLORREF Color8;

private:
	Map (*stMapArrayCopy)[MAP_WIDTH];			// �� Array �纻
	DLinkedList<stFindSearchNode*> m_OpenList;
	DLinkedList<stFindSearchNode*> m_CloseList;

	stFindSearchNode* StartNode;
	stFindSearchNode* FinNode;

	HWND hWnd;

	int ColorKey = 1;

private:
	// ���ڷ� ���� ��带 ��������, ��� �������� �̵��� ������ ����.
	//void CheckCreateJump(int X, int Y, stFindSearchNode* parent, COLORREF colorRGB, Direction dir)
	//{
	//	int OutX = 0, OutY = 0;
	//	double G = 0, OutG = parent->m_fG;
	//
	//	if (Jump(X, Y, &OutX, &OutY, G, &OutG, colorRGB, dir) == true)
	//		CreateNode(OutX, OutY, parent, OutG, dir);
	//}

	// ���ڷ� ���� ��带 ��������, ��� ���� ���� ���� üũ
	void CreateNodeCheck(stFindSearchNode* parent);

	// ������ �������� �����ϸ鼭 üũ
	bool Jump(int X, int Y, int* OutX, int* OutY, double G, double* OutG, COLORREF colorRGB, Direction dir);

	// ��� ���� �Լ�
	void CreateNode(int iX, int iY, stFindSearchNode* parent, double G, Direction dir);

	// Jump_Point_Search_While()�� �Ϸ�� ��, ����/���� ����Ʈ ���� �� �ϱ�
	void LastClear();

public:
	// ������
	CFindSearch(Map(*Copy)[MAP_WIDTH]);

	// �Ҹ���
	~CFindSearch();

	// ó�� ���� ��带 ���¸���Ʈ�� �ִ� �뵵.
	void StartNodeInsert();	

	// ������, ������ �����ϱ�
	void Init(int StartX, int StartY, int FinX, int FinY, HWND hWnd);	

	// �׸���
	void Show(stFindSearchNode* Node = nullptr, int StartX = -1, int StartY = -1, int FinX = -1, int FinY = -1);

	// ȣ��� �� ���� Jump Point Search ����
	void Jump_Point_Search_While();	
};


#endif // !__A_SEARCH_H__
