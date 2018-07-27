#include "stdafx.h"
#include "ChildWindow.h"

//// �ڽ� ������ Ŭ���� CPP ////////////////////////////

// CMonitorGraphUnit Ŭ������ static��.
CMonitorGraphUnit::stHWNDtoTHIS CMonitorGraphUnit::MyThisStruct;
bool CMonitorGraphUnit::FirstCheck = true;
CMonitorGraphUnit *CMonitorGraphUnit::pThis;

// ������
CMonitorGraphUnit::CMonitorGraphUnit(HINSTANCE hInstance, HWND hWndParent, COLORREF BackColor, TYPE enType, int iPosX, int iPosY, int iWidth, int iHeight)
{
	if (FirstCheck)	// ������ �̸� �� �Ȱ�����..(lpszClassName�� ����) ���� 1ȸ�� ����.
	{
		FirstCheck = false;
		for (int i = 0; i < dfMAXCHILD; ++i)	// ���� 1ȸ���� ��� MyThisStruct�� NULL�� �ʱ�ȭ. NULL�� ��� �� �������� �Ǵ�.
		{
			MyThisStruct.hWnd[i] = NULL;
		}

		// ������ Ŭ���� ����
		WNDCLASSEXW wcex;

		wcex.cbSize = sizeof(WNDCLASSEX);

		wcex.style = CS_HREDRAW | CS_VREDRAW;
		wcex.lpfnWndProc = CMonitorGraphUnit::WndProc;
		wcex.cbClsExtra = 0;
		wcex.cbWndExtra = 0;
		wcex.hInstance = hInstance;
		wcex.hIcon = NULL;
		wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
		wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
		wcex.lpszMenuName = NULL;
		wcex.lpszClassName = TEXT("�ڽ�������");
		wcex.hIconSm = NULL;

		RegisterClassExW(&wcex);		
	}

	// ������ ����
	HWND hWnd = CreateWindowW(TEXT("�ڽ�������"), TEXT("�ڽ��������Ӵ�"),  WS_CAPTION | WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS,
		iPosX, iPosY, iWidth, iHeight, hWndParent, nullptr, hInstance, nullptr);

	// �۾� ������ ���´�.
	GetClientRect(hWnd, &rt);

	// ���� �ʿ��� ���� Ŭ���� ��������� �ִ´�.
	_enGraphType = enType;
	this->BackColor = BackColor;
	this->hWndParent = hWndParent;
	this->hWnd = hWnd;
	this->hInstance = hInstance;
	MyBrush = CreateSolidBrush(BackColor);
	
	Init(&queue);								// ť �ʱ�ȭ
	y = rt.bottom - (rt.bottom / 2);			// �۾� ���� ����, �߰� ������ ���� y�� �ִ´�. �̴�, ó�� ���� ��ġ�� �����Ѵ�.
	Enqueue(&queue, y);						    // ������ y(ó�� ���� ��ġ)�� ť�� �ִ´�.

	// ���� this�� ���� ����
	bool Check = PutThis();

	if (!Check)	// ����, �� �̻� �����츦 ������ �� ���ٸ� �׳� ����.
		exit(0);

	UpdateWindow(hWnd);
}

// �Ҹ���
CMonitorGraphUnit::~CMonitorGraphUnit()
{
	// ����� �Ұ� ����...�ϴ� ��������
}

// �ڽĵ��� ������ ���ν���
LRESULT CALLBACK CMonitorGraphUnit::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	pThis = GetThis(hWnd);	// ���� hWnd�� this ������.

	switch (message)
	{
	case WM_PAINT:
	{		
		int x;
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hWnd, &ps);	
		HDC MemDC = CreateCompatibleDC(hdc);
		HBITMAP MyBitmap = CreateCompatibleBitmap(hdc, pThis->rt.right, pThis->rt.bottom);
		HBITMAP OldBitmap = (HBITMAP)SelectObject(MemDC, MyBitmap);

		FillRect(MemDC, &pThis->rt, pThis->MyBrush);
		int data;	// ��ť ��, ������ y��.
		if (FirstPeek(&pThis->queue, &data))	// ť�� ���� ó���� �ִ� y���� ��ť�Ѵ�.
		{
			x = 0;	// �Ź� �׸� �� ����, ���� ����<<���� �׷��� �ϱ� ������ 0���� �����.
			MoveToEx(MemDC, x, data, NULL);			// ó�� ���� ��ġ ����.	
			while (NextPeek(&pThis->queue, &data))	// ���� y�� ��ť.
			{
				x += pThis->rt.right / 100;;	// �������� �ʺ� 100������� ������, 1��� �������� ��´�.
				LineTo(MemDC, x, data);	// ��ť�� y���� ����� ���� ��ġ���� �ߴ´�.
			}
		}
		BitBlt(hdc, pThis->rt.left, pThis->rt.top, pThis->rt.right, pThis->rt.bottom, MemDC, pThis->rt.left, pThis->rt.top, SRCCOPY);
		
		SelectObject(MemDC, OldBitmap);
		DeleteObject(MyBitmap);
		DeleteDC(MemDC);

		EndPaint(hWnd, &ps);
	}
	break;

	case WM_DESTROY:	
		DeleteObject(pThis->MyBrush);
		PostQuitMessage(0);		
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

// ������ �ֱ�.
void CMonitorGraphUnit::InsertData(int iData)
{
	int randCheck = rand() % 2;	// ���Ұ����� �E������ �� ���ϳ� ��������

	if (randCheck == 0)	// 0�̸� �A��.
	{
		y -= iData;
		while (rt.top > y || rt.bottom < y)	// �P�µ�, ȭ���� ����ٸ�, ���Ѵ�. ����� ���� �� ����.
			y += iData;
	}
	else  // 1�̸� ���Ѵ�.
	{
		y += iData;
		while (rt.top > y || rt.bottom < y)	//���ߴµ� ȭ���� ����ٸ�, �A��. ����� ���� �� ����
			y -= iData;
	}
	Enqueue(&queue, y);	// ���� y�� ��ť�Ѵ�.
	InvalidateRect(hWnd, NULL, false);	// WM_PAINT ȣ��
}

//------------------------------------------------------
// ������ �ڵ�, this ������ ��Ī ���̺� ����.
//------------------------------------------------------
BOOL CMonitorGraphUnit::PutThis(void)
{
	for (int i = 0; i < dfMAXCHILD; ++i)
	{
		// hWnd�� NULL�� ������ ã����, �ش� ������ �ڵ��̶� this ����
		if (MyThisStruct.hWnd[i] == NULL)
		{
			MyThisStruct.hWnd[i] = hWnd;
			MyThisStruct.pThis[i] = this;
			return true;
		}
	}
	
	return false;
}
BOOL CMonitorGraphUnit::RemoveThis(HWND hWnd)
{
	for (int i = 0; i < dfMAXCHILD; ++i)
	{
		// ���ڷ� ���� hWnd�� ���� ������ ã����, �ش� ������ NULL�� ���� �� ���� ���.
		if (MyThisStruct.hWnd[i] == hWnd)
		{
			MyThisStruct.hWnd[i] = NULL;
			return true;
		}
	}

	return false;
}

CMonitorGraphUnit *CMonitorGraphUnit::GetThis(HWND hWnd)
{
	// �ʿ��� This ������.
	for (int i = 0; i < dfMAXCHILD; ++i)
		if (MyThisStruct.hWnd[i] == hWnd)
			return MyThisStruct.pThis[i];
}