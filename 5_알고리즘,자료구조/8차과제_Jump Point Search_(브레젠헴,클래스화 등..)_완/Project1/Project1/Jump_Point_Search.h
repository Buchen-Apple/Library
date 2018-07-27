#ifndef __A_SEARCH_H__
#define __A_SEARCH_H__
#include "DLinked_List.h"

struct stFindSearchNode;

// �� Ÿ���� Ÿ�� enum
enum Tile_Type
{
	NONE = 0, START, FIN, OBSTACLE, OPEN_LIST, CLOSE_LIST, JUMP, TEST
};

// �� ������ ����ü.
struct Map
{
	// ���⿡ ����Ǵ� X,Y�� ���� �ȼ� ��ǥ
	int m_iMapX;
	int m_iMapY;
	Tile_Type m_eTileType;
	COLORREF m_coColorRGB;
};




class CFindSearch
{
public:
	// ������
	CFindSearch(Map* Copy, int Width, int Height, int Radius, int MaxSearchCount);

	// �Ҹ���
	~CFindSearch();

	// ����� ���� enum
	enum Direction
	{
		NONE = 0, LL, LU, LD, RR, RU, RD, UU, DD
	};

	// ��� ������ ����ü
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

//////////////////////////
// Ŭ���� ����
/////////////////////////
private:
	// Jump�� üũ�� Ÿ���� ĥ�� ��. 8���� �����ΰ� ������� ��������.
	COLORREF Color1;
	COLORREF Color2;
	COLORREF Color3;
	COLORREF Color4;
	COLORREF Color5;
	COLORREF Color6;
	COLORREF Color7;
	COLORREF Color8;
	
	// �� Array ������
	Map* stMapArrayCopy;		

	// ���� ����Ʈ, ���� ����Ʈ
	DLinkedList<stFindSearchNode*> m_OpenList;
	DLinkedList<stFindSearchNode*> m_CloseList;

	// ������, ������
	stFindSearchNode* StartNode;
	stFindSearchNode* FinNode;

	// �׸���� ������ (DC��)
	HWND m_hWnd;
	HDC m_hMemDC;
	HBITMAP m_hMyBitmap, m_hOldBitmap;
	RECT rt;

	// ���� ������� �÷��� �� ������ ����. Color1 ~ Color8
	int m_iColorKey = 1;

	// ���� ����, ����, Ÿ��1���� ������, ��� Ÿ�ϱ��� Jump�� ������ �����ϴ� ����.
	int m_iWidth, m_iHeight, m_iRadius, m_iMaxSearchCount;

	// G�� ��� �� ���� ����ġ, �밢�� ����ġ
	double m_dStraight_Line;
	double m_dDiagonal_Line;

//////////////////////////
// JPS ����ó���� �Լ���
/////////////////////////
private:	

	// ���ڷ� ���� ��� üũ. ����� �Ӽ��� ���� CheckCreateJump() �Լ��� ���� ����.
	void CreateNodeCheck(stFindSearchNode* parent, bool ShowCheck = true);

	// ���ڷ� ���� ��� ����, ��� �������� �̵��� ������ ����. �� �� ������ Jump??() �Լ� ȣ��
	void CheckCreateJump(int X, int Y, stFindSearchNode* parent, COLORREF colorRGB, Direction dir);	

	// CheckCreateJump()�Լ����� ȣ��ȴ�. ������ �������� �����ϸ鼭 ��� ���� ��ġ üũ
	bool JumpUU(int X, int Y, int* OutX, int* OutY, double G, double* OutG, COLORREF colorRGB, Direction dir);
	bool JumpDD(int X, int Y, int* OutX, int* OutY, double G, double* OutG, COLORREF colorRGB, Direction dir);
	bool JumpRR(int X, int Y, int* OutX, int* OutY, double G, double* OutG, COLORREF colorRGB, Direction dir);
	bool JumpLL(int X, int Y, int* OutX, int* OutY, double G, double* OutG, COLORREF colorRGB, Direction dir);
	bool JumpLU(int X, int Y, int* OutX, int* OutY, double G, double* OutG, COLORREF colorRGB, Direction dir);
	bool JumpLD(int X, int Y, int* OutX, int* OutY, double G, double* OutG, COLORREF colorRGB, Direction dir);
	bool JumpRU(int X, int Y, int* OutX, int* OutY, double G, double* OutG, COLORREF colorRGB, Direction dir);
	bool JumpRD(int X, int Y, int* OutX, int* OutY, double G, double* OutG, COLORREF colorRGB, Direction dir);

	// Jump??() �Լ����� true�� ȣ��Ǹ� CheckCreateJump()�Լ��� �ش� �Լ� ȣ��. 
	// ��� ���� �Լ�
	void CreateNode(int iX, int iY, stFindSearchNode* parent, double G, Direction dir);

	// Jump_Point_Search_While()�� �Ϸ�� ��, ����/���� ����Ʈ ���� �� �ϱ�
	void LastClear();

public:
	// ó�� ���� ��带 ���¸���Ʈ�� �ִ� �뵵.
	void StartNodeInsert();

	// ������, ������ �����ϱ�
	void Init(int StartX, int StartY, int FinX, int FinY);

	// ȣ��� �� ���� Jump Point Search ����
	void Jump_Point_Search_While();


//////////////////////////
// �׸���� �Լ�
/////////////////////////
private:	
	// ���ο��� ���߱��
	void Line(stFindSearchNode* Node, COLORREF rgb = -1);

public:
	// ������ �ڵ��� ���޹ް� MemDC���� �����ϴ� �Լ�
	void DCSet(HWND hWnd);

	// �׸��� �׸���
	void GridShow();

	// �ܺο��� POINT���ڸ� �޾Ƽ� �� �߱�
	void Out_Line(POINT* p, int Count, COLORREF rgb = -1);


//////////////////////////
// �극���� �˰��� ���� �Լ�
/////////////////////////
private:
	// �극���� ���� üũ
	void Bresenham_Line(stFindSearchNode* FinNode);	

public:
	// �ۿ��� ������, ������� Map ����ü�� ������, ��θ� POINT����ü �ȿ� �־��ִ� �Լ�. 
	// POINT����ü �迭 0������ ä���ش�.
	// Use_Bresenham������ true�� ������ �극���� �˰������� ��ã�´�.
	// �迭�� ���� �����Ѵ�.
	int PathGet(int StartX, int StartY, int FinX, int FinY, POINT* Outmap, bool Use_Bresenham);

	// �극���� �׽�Ʈ �Լ�(Ÿ�� ��ĥ�ϱ�. ���� Main�Լ����� ȣ����)
	void Bresenham_Line_Test(int StartX, int StartY, int FinX, int FinY);
};


#endif // !__A_SEARCH_H__
