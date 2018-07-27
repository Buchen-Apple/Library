#ifndef __A_SEARCH_H__
#define __A_SEARCH_H__
//#include "DLinked_List.h"
#include "MapTile.h"

class CFindSearch
{
	enum Direction
	{
		NONE = 0, LL, LU, LD, RR, RU, RD, UU, DD
	};
public:
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

private:
	Map(*stMapArrayCopy)[MAP_WIDTH];			// �� Array �纻

	//
	typedef std::list<stFindSearchNode*> LISTNODE;
	LISTNODE m_OpenList;
	LISTNODE m_CloseList;
	//
	//DLinkedList<stFindSearchNode*> m_OpenList;
	//DLinkedList<stFindSearchNode*> m_CloseList;

	stFindSearchNode* StartNode;
	stFindSearchNode* FinNode;

	HWND hWnd;

public:
	// ������
	CFindSearch(Map(*Copy)[MAP_WIDTH]);

	// �Ҹ���
	~CFindSearch();

	// �ݺ��� ���鼭 Jump Point Search ����
	void Jump_Point_Search_While();

	// ���ڷ� ���� ��带 ��������, ��� �������� �̵��� ������ ����.
	void CheckCreateJump(int X, int Y, stFindSearchNode* parent, Direction dir);

	// ���ڷ� ���� ��带 ��������, ��� ���� ���� ���� üũ
	void CreateNodeCheck(stFindSearchNode* parent);

	// ������ �������� �����ϸ鼭 üũ
	bool Jump(int X, int Y, int* OutX, int* OutY, Direction dir);

	// ������, ������ �����ϱ�
	void Init(int StartX, int StartY, int FinX, int FinY, HWND hWnd);

	// ��� ���� �Լ�
	void CreateNode(int iX, int iY, stFindSearchNode* parent, Direction dir);

	// �׸���
	void Show(stFindSearchNode* Node = nullptr);

};

bool NodeCompare(CFindSearch::stFindSearchNode* _l, CFindSearch::stFindSearchNode* _r);

#endif // !__A_SEARCH_H__
