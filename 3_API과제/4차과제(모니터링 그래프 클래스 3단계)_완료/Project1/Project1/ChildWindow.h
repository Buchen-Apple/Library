#pragma once
#ifndef __CHILD_WINDOW_H__
#define __CHILD_WINDOW_H__

//// �ڽ� ������ Ŭ���� ��� ////////////////////////////
#include "Graph_Ypos_Queue.h"

#define dfMAXCHILD		100
#define UM_PARENTBACKCHANGE WM_USER+1	// �ڽ��� �θ𿡰� ������ �޽���
#define UM_CHILDFONTCHANGE	WM_USER+2   //�θ� �ڽĿ��� ������ �޽���

// ���� ID
#define SERVER_1	1
#define SERVER_2	2
#define SERVER_3	3
#define SERVER_4	4

// ������ Ÿ��
#define DATA_TYPE_CPU_USE				1001	// cpu ��뷮
#define DATA_TYPE_MEMORY_USE			1002	// �޸� ��뷮
#define DATA_TYPE_CCU					1003	// ������ ��
#define DATA_TYPE_DB_BUFFER				1004	// DB ����

class CMonitorGraphUnit
{

private:
	struct ColumnInfo
	{
		int ServerID;
		int DataType;
		int iLastVal;			// ���� ť�� ���� ���� ������ ��
		TCHAR DataName[10];
		Queue queue;
	};

	ColumnInfo *CoInfo;

public:

	enum TYPE
	{
		BAR_SINGLE_VERT,
		BAR_SINGLE_HORZ,
		BAR_COLUMN_VERT,
		BAR_COLUMN_HORZ,
		LINE_SINGLE,
		LINE_MULTI,
		PIE
	};

public:

	CMonitorGraphUnit(HINSTANCE hInstance, HWND hWndParent, COLORREF BackColor, TYPE enType, int iPosX, int iPosY, int iWidth, int iHeight, LPCTSTR str);
	~CMonitorGraphUnit();

	/////////////////////////////////////////////////////////
	// ������ ���ν���
	/////////////////////////////////////////////////////////
	static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

	/////////////////////////////////////////////////////////
	// ������ �ֱ�, ó�� ����Ʈ �� �߰�����
	/////////////////////////////////////////////////////////
	void InsertData(int iData, int ServerID, int DataType);
	void AddData(int iMax, int AleCount, LPCTSTR Unit);
	
	/////////////////////////////////////////////////////////
	// �÷� ���� ���� ����
	/////////////////////////////////////////////////////////
	void SetColumnInfo(int iColumnCount, int ServerID[], int DataType[], TCHAR DataName[][10]);

private:
	//------------------------------------------------------
	// Paint�Լ�
	//------------------------------------------------------
	void Paint_LINE_SINGLE();
	void Paint_LINE_MULTI();
	void Paint_BAR_SINGLE_VERT();
	void Paint_BAR_COLUMN_VERT();

	//------------------------------------------------------
	// �˶� ��, �θ� ��� ���������� ĥ�ϱ� üũ �Լ�
	//------------------------------------------------------
	void ParentBackCheck(int Data);

	//------------------------------------------------------
	// �⺻ UI ����
	//------------------------------------------------------
	void UISet();

	//------------------------------------------------------
	// �θ� ������ �ڵ�, �� ������ �ڵ�, �ν��Ͻ� �ڵ�
	//------------------------------------------------------
	HWND hWndParent;
	HWND hWnd;
	HINSTANCE hInstance;

	//------------------------------------------------------
	// ������ ��ġ,ũ��,����, �׷��� Ÿ�� ��.. �ڷ�
	//------------------------------------------------------
	TYPE _enGraphType;
	RECT rt;
	RECT TitleRt;
	RECT LeftRt;
	RECT RightRt;	// LINE_MULTIŸ�� �׷��������� ���ȴ�. ȭ�� ������ �׷��� ���� ǥ���ϴ� ����.
	RECT BottomRt;	// BAR_COLUMN_VERTŸ�� �׷��������� ���ȴ�. ȭ�� �ϴܿ� �׷��� ���� ǥ���ϴ� ����.
	RECT ClientRt;
	COLORREF BackColor;	

	//------------------------------------------------------
	// ������
	//------------------------------------------------------	
	Queue queue;	// ť	
	double iAddY;
	int Greedint[4];
	bool AleOnOff;			//�ش� �����찡 �˶��� �︱�� ����. false�� �︮�� ����
	int iColumnCount;		// �ش� �������� �÷� �� ī��Ʈ

	//------------------------------------------------------
	// Ÿ��Ʋ�� ǥ�õ� �ؽ�Ʈ
	//------------------------------------------------------	
	TCHAR tWinName[20];		// ������ �̸�
	TCHAR tUnit[10];		// ǥ�� ����	

	//------------------------------------------------------
	// �׷��� Max�� , ��� �︮�� ��
	//------------------------------------------------------
	bool MaxVariable;
	int iMax;
	double AleCount;
	bool bObjectCheck;

	//------------------------------------------------------
	// MemDC�� ���ҽ�
	//------------------------------------------------------
	HDC MemDC;
	HBRUSH BackBrush, TitleBrush;
	HBITMAP MyBitmap, OldBitmap;
	HFONT MyFont;
	COLORREF NowFontColor, NormalFontColor;	// ����� ��Ʈ�� �÷� , �ش� �������� �⺻ ��Ʈ �÷�
											// ���� �� ������, �ڵ忡���� NowFontColor�� �����Ű�� ��Ȳ�� ���� NowFontColor��Ʈ�� NormalFontColor Ȥ�� ���� ��Ʈ�� �ȴ�.
	HPEN* GraphPen;
	

	//------------------------------------------------------
	// ���� this�� ������ ���� ������
	//------------------------------------------------------
	static CMonitorGraphUnit *pThis;
};

#endif // !__CHILD_WINDOW_H__
