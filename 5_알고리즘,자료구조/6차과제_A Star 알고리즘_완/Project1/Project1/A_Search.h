#ifndef __A_SEARCH_H__
#define __A_SEARCH_H__
#include "DLinked_List.h"
#include "MapTile.h"

class CFindSearch
{
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
	};

private:
	Map (*stMapArrayCopy)[MAP_WIDTH];			// �� Array �纻
	DLinkedList<stFindSearchNode*> m_OpenList;
	DLinkedList<stFindSearchNode*> m_CloseList;

	stFindSearchNode* StartNode;
	stFindSearchNode* FinNode;

	HWND hWnd;

public:
	// ������
	CFindSearch(Map (*Copy)[MAP_WIDTH]);

	// �Ҹ���
	~CFindSearch();

	// �ݺ��� ���鼭 ���̽�Ÿ ����
	void A_Search_While();

	// ������, ������ �����ϱ�
	void Init(int StartX, int StartY, int FinX, int FinY, HWND hWnd);

	// ��� ���� �Լ�
	void CreateNode(int iX, int iY, stFindSearchNode* parent, double fWeight = 0);

	// �׸���
	void Show(stFindSearchNode* Node = nullptr);

};


#endif // !__A_SEARCH_H__
