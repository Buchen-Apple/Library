#pragma once
#ifndef __CHILD_WINDOW_H__
#define __CHILD_WINDOW_H__

//// �ڽ� ������ Ŭ���� ��� ////////////////////////////
#include "Graph_Ypos_Queue.h"

#define dfMAXCHILD		100

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

	typedef struct ST_HWNDtoTHIS
	{
		HWND			hWnd[dfMAXCHILD];
		CMonitorGraphUnit	*pThis[dfMAXCHILD];

	} stHWNDtoTHIS;

public:

	CMonitorGraphUnit(HINSTANCE hInstance, HWND hWndParent, COLORREF BackColor, TYPE enType, int iPosX, int iPosY, int iWidth, int iHeight);
	~CMonitorGraphUnit();

	/////////////////////////////////////////////////////////
	// ������ ���ν���
	/////////////////////////////////////////////////////////
	static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

	/////////////////////////////////////////////////////////
	// ������ �ֱ�.
	/////////////////////////////////////////////////////////
	void InsertData(int iData);

protected:

	//------------------------------------------------------
	// ������ �ڵ�, this ������ ��Ī ���̺� ����.
	//------------------------------------------------------
	BOOL				PutThis(void);
	static BOOL			RemoveThis(HWND hWnd);
	static CMonitorGraphUnit* GetThis(HWND hWnd);

private:

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
	COLORREF BackColor;

	//------------------------------------------------------
	// ������ ���ʻ��� üũ
	//------------------------------------------------------
	static bool FirstCheck;

	//------------------------------------------------------
	// ������
	//------------------------------------------------------	
	Queue queue;	// ť
	int y;			// �׷��� �׸� �� ����ϴ� x, y ����
	HBRUSH MyBrush;		// �� ����� ä�� �귯��.

	// static �ɹ� �Լ��� ���ν������� This �����͸� ã�� ����
	// HWND + Class Ptr �� ���̺�
	static CMonitorGraphUnit *pThis;
	static stHWNDtoTHIS MyThisStruct;
};
#endif // !__CHILD_WINDOW_H__
