#pragma once
#ifndef __CHILD_WINDOW_H__
#define __CHILD_WINDOW_H__

//// �ڽ� ������ Ŭ���� ��� ////////////////////////////
#include "Graph_Ypos_Queue.h"

#define dfMAXCHILD		100
#define UM_PARENTBACKCHANGE WM_USER+1

class CMonitorGraphUnit
{
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

	CMonitorGraphUnit(HINSTANCE hInstance, HWND hWndParent, COLORREF BackColor, TYPE enType, int iPosX, int iPosY, int iWidth, int iHeight, LPCTSTR str, int iMax, int AleCount);
	~CMonitorGraphUnit();

	/////////////////////////////////////////////////////////
	// ������ ���ν���
	/////////////////////////////////////////////////////////
	static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

	/////////////////////////////////////////////////////////
	// ������ �ֱ�.
	/////////////////////////////////////////////////////////
	void InsertData(int iData, double*);

private:
	//------------------------------------------------------
	// Paint�Լ�
	//------------------------------------------------------
	void Paint_LINE_SINGLE();

	//------------------------------------------------------
	// �θ� ��� ���������� ĥ�ϱ� üũ �Լ�
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
	RECT ClientRt;
	COLORREF BackColor;

	//------------------------------------------------------
	// ������
	//------------------------------------------------------	
	Queue queue;	// ť
	TCHAR tWinName[20];
	double iAddY;
	int Greedint[4];

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
	int iFontR, iFontG, iFontB;	// ��Ʈ�� �߰��� ����Ǳ� ������(����������) ���� ��Ʈ���� ������ ����.
	HPEN GraphPen;
	

	// static �ɹ� �Լ��� ���ν������� This �����͸� ã�� ����
	// HWND + Class Ptr �� ���̺�
	static CMonitorGraphUnit *pThis;

	// 1�� ���������� ������Ű�� ���� �ӽ� static
	double aaa;
};

#endif // !__CHILD_WINDOW_H__
