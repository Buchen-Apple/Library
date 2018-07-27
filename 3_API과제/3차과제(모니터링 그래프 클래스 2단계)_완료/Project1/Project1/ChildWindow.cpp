#include "stdafx.h"
#include "ChildWindow.h"

//// �ڽ� ������ Ŭ���� CPP ////////////////////////////
CMonitorGraphUnit *CMonitorGraphUnit::pThis;

// ������
CMonitorGraphUnit::CMonitorGraphUnit(HINSTANCE hInstance, HWND hWndParent, COLORREF BackColor, TYPE enType, int iPosX, int iPosY, int iWidth, int iHeight, LPCTSTR str, int iMax, int AleCount)
{

	/////////////////////////////////////////////////////////
	// ������ Ŭ���� ����
	/////////////////////////////////////////////////////////
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

	HWND hWnd = CreateWindowW(TEXT("�ڽ�������"), TEXT("�ڽ��������Ӵ�"),   WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS,
		iPosX, iPosY, iWidth, iHeight, hWndParent, nullptr, hInstance, nullptr);
	
	/////////////////////////////////////////////////////////
	//  ������ ���� ����
	/////////////////////////////////////////////////////////
	// �۾� ������ ���Ѵ�.
	GetClientRect(hWnd, &rt);

	// ȭ�� ��� Ÿ��Ʋ�� ȭ�� ���� ������ �����Ѵ�.
	TitleRt = rt;
	TitleRt.bottom = 30;

	LeftRt = rt;
	LeftRt.top = TitleRt.bottom;
	LeftRt.right = 40;

	// ������ ������ ���� �׷����� �׷��� ���� ����
	ClientRt = rt;
	ClientRt.left = LeftRt.right;
	ClientRt.top = TitleRt.bottom;

	/////////////////////////////////////////////////////////
	//  Ŭ���� ��������� �ʿ��� �� ����
	/////////////////////////////////////////////////////////
	_enGraphType = enType;
	this->BackColor = BackColor;
	this->hWndParent = hWndParent;
	this->hWnd = hWnd;
	this->hInstance = hInstance;
	wcscpy_s(tWinName, _countof(tWinName), str);
	this->iMax = iMax;
	if (iMax == 0)
		MaxVariable = true;
	else
		MaxVariable = false;
	
	SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)this);

	/////////////////////////////////////////////////////////
	//  �׷��� Max�� ����
	/////////////////////////////////////////////////////////
	if (!MaxVariable)
	{
		// �۾� ���� ����, y 1ĭ �̵� ��, ���� �̵��ؾ��ϴ� ��ġ ���
		iAddY = (double)(ClientRt.bottom - ClientRt.top) / iMax;

		Greedint[0] = iMax / 4;							// ù �׸��忡 ǥ�õ� ����
		Greedint[1] = Greedint[0] + Greedint[0];		// �� ��° �׸��忡 ǥ�õ� ����
		Greedint[2] = Greedint[1] + Greedint[0];		// �� ��° �׸��忡 ǥ�õ� ����
		Greedint[3] = iMax;								// �� ��° �׸��忡 ǥ�õ� ����
	}

	this->AleCount = ClientRt.bottom - (AleCount * iAddY);	// �˸��� �︱ �� ����

	/////////////////////////////////////////////////////////
	//  MemDC�� ���ҽ� ����
	/////////////////////////////////////////////////////////
	HDC hdc = GetDC(hWnd);
	MemDC = CreateCompatibleDC(hdc);

	// ��Ʈ��
	MyBitmap = CreateCompatibleBitmap(hdc, rt.right, rt.bottom);	
	OldBitmap = (HBITMAP)SelectObject(MemDC, MyBitmap);

	// �귯��
	BackBrush = CreateSolidBrush(BackColor);
	int r, g, b;
	r = GetRValue(BackColor);
	g = GetGValue(BackColor);
	b = GetBValue(BackColor);
	r /= 2;
	g /= 2;
	b /= 2;
	TitleBrush = CreateSolidBrush(RGB(r, g, b));

	// ��
	r = GetRValue(BackColor);
	g = GetGValue(BackColor);
	b = GetBValue(BackColor);
	r *= 3;
	g *= 3;
	b *= 3;
	GraphPen = CreatePen(PS_SOLID, 1, RGB(r, g, b));
	SelectObject(MemDC, GraphPen);
	
	// ��Ʈ
	iFontR = r, iFontG = g, iFontB = b;
	MyFont = CreateFont(17, 0, 0, 0, 0, 0, 0, 0,
		DEFAULT_CHARSET, 0, 0, 0,
		VARIABLE_PITCH | FF_ROMAN, L"Arial"); 

	SetBkMode(MemDC, TRANSPARENT);	

	ReleaseDC(hWnd, hdc);	

	/////////////////////////////////////////////////////////
	//  ť ����
	/////////////////////////////////////////////////////////
	Init(&queue);			
	Enqueue(&queue, 0);

	UpdateWindow(hWnd);
}

// �Ҹ���
CMonitorGraphUnit::~CMonitorGraphUnit()
{
	SelectObject(MemDC, OldBitmap);
	DeleteObject(MyBitmap);

	DeleteObject(BackBrush);
	DeleteObject(TitleBrush);

	SelectObject(MemDC, GetStockObject(WHITE_PEN));
	DeleteObject(GraphPen);

	SelectObject(MemDC, GetStockObject(DEFAULT_GUI_FONT));
	DeleteObject(MyFont);

	DeleteDC(MemDC);
}

// �ڽĵ��� ������ ���ν���
LRESULT CALLBACK CMonitorGraphUnit::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	static double yTemp;
	static double Temp;
	static TCHAR GreedText[20];

	pThis = (CMonitorGraphUnit*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
	switch (message)
	{
	case WM_PAINT:
	{		
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hWnd, &ps);	

		// �׸���, ���ä��� �� �⺻ UI ����
		pThis->UISet();

		// ���� ���
		switch (pThis->_enGraphType)
		{
		case BAR_SINGLE_VERT:
			break;

		case BAR_SINGLE_HORZ:
			break;

		case BAR_COLUMN_VERT:
			break;

		case  BAR_COLUMN_HORZ:
			break;

		case LINE_SINGLE:
			pThis->Paint_LINE_SINGLE();
			break;

		case LINE_MULTI:
			break;

		case PIE:
			break;

		}		
		BitBlt(hdc, pThis->rt.left, pThis->rt.top, pThis->rt.right, pThis->rt.bottom, pThis->MemDC, pThis->rt.left, pThis->rt.top, SRCCOPY);
		
		EndPaint(hWnd, &ps);
	}
	break;
	
	case WM_DESTROY:	
		DeleteObject(pThis->BackBrush);
		PostQuitMessage(0);		
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

// ���� ������ �ֱ�

//void CMonitorGraphUnit::InsertData(int iData, double* a)
//{
//	int randCheck = rand() % 2;	// ���Ұ����� �E������ �� ���ϳ� ��������
//	double yTemp = y;
//
//	if (randCheck == 0)	// 0�̸� �A��.
//	{
//		yTemp -= (iAddY * iData);
//		if (ClientRt.top > yTemp || ClientRt.bottom < yTemp)	// �P�µ�, ȭ���� ����ٸ�, ���Ѵ�. ����� ���� �� ����.
//		{
//			yTemp = y;
//			yTemp += (iAddY * iData);
//		}
//	}
//	else  // 1�̸� ���Ѵ�.
//	{
//		yTemp += (iAddY * iData);
//		if (ClientRt.top > yTemp || ClientRt.bottom < yTemp)	//���ߴµ� ȭ���� ����ٸ�, �A��. ����� ���� �� ����
//		{
//			yTemp = y;
//			yTemp -= (iAddY * iData);
//		}			
//	}
//	y = yTemp;
//	*a =  (ClientRt.bottom - y) / iAddY;	// ClientRt.bottom - y�� ���� ������ �� 1ĭ�̴�.
//											// ���⿡ iAddY�� ������, (ClientRt.bottom - y) �ȿ� iAddY�� ��� �ִ��� �� �� �ִ�.
//											// ��, ���� ���� ī��Ʈ �� �� �ִ�.
//	Enqueue(&queue, (int)y);	// ���� y�� ��ť�Ѵ�.	
//	InvalidateRect(hWnd, NULL, false);	// WM_PAINT ȣ��	
//}

// ������ �ֱ�.
void CMonitorGraphUnit::InsertData(int iData, double* a)
{	
	// 1�� �����ϱ� �� ���, ����ϴ� ����.	
	//aaa += iData;	
	//if (aaa >= iMax)
		//aaa = 0;
	//Enqueue(&queue, aaa);	

	Enqueue(&queue, iData);	// iData�� ��ť�Ѵ�.	

	// ��ť �� �����͸� �������� �θ��� ��׶��� ���������� ĥ�ϱ� üũ
	//ParentBackCheck(ClientRt.bottom - (aaa * iAddY));
	ParentBackCheck(ClientRt.bottom - (iData * iAddY));

	InvalidateRect(hWnd, NULL, false);	// WM_PAINT ȣ��	
}

// LINE_SINGLE ���
void CMonitorGraphUnit::Paint_LINE_SINGLE(void)
{
	double x, y;
	int data;	// ��ť ��, ������ y��.
	if (FirstPeek(&queue, &data))	// ť�� ���� ó���� �ִ� y���� ��ť�Ѵ�.
	{
		x = ClientRt.left;	// �Ź� �׸� �� ����, ���� ����<<���� �׷��� �ϱ� ������ 0���� �����.
		y = ClientRt.bottom - (iAddY * data);			// ��ť�� ����, ���� ������ �� y��ǥ 1�� ����� ��, ���� ��ġ���� ����. 
		MoveToEx(MemDC, x, y, NULL);			// ó�� ���� ��ġ ����.	
		while (NextPeek(&queue, &data))	// ���� y�� ��ť.
		{
			x += (double)(ClientRt.right - ClientRt.left) / 100;	// �۾� ������ �ʺ� 100������� ������, 1��� �������� ��´�.
			y = ClientRt.bottom - (iAddY * data);		// ��ť�� ���� ��������, ���� ������ �� ��ǥ��ŭ �ٽ� �̵�
			LineTo(MemDC, x, y);	// �׸��� �� ���� ���� ���κ��� �ߴ´�.
		}
	}
}

// �θ��� ��׶��� ���������� ĥ�ϱ�
void CMonitorGraphUnit::ParentBackCheck(int data)
{
	// �������� ��ť�� Data�� �˶����� �۴ٸ�, �θ𿡰� ȭ�� ������ �϶�� ������ �޽����� ������. 
	// �̹� ���������� �ƴ����� �θ� üũ
	if (data < AleCount)
	{
		SendMessage(hWndParent, UM_PARENTBACKCHANGE, 0, 0);	// �θ𿡰� ȭ�� ������ �϶�� ����.

		// �θ𿡰� ������ �϶�� ������ ��, ��Ʈ�� �� ����. 
		// ���� �˶� ��ġ���� ���� ���� ���, ��Ʈ �� ���������� ����.
		// ��, ��� �����찡 ��Ʈ ���������� �� ���� �ִ�.
		// �̹� �����ߴٸ�(bObjectCheck�� true) if�� �۵� ����.
		if (!bObjectCheck)
		{
			bObjectCheck = true;
			SetTextColor(MemDC, RGB(255, 0, 0));
		}
	}

	// Data�� �˶����� ũ�ٸ�, �ٽ� �� ��Ʈ�� ������� ����. 
	else
	{
		bObjectCheck = false;
		SetTextColor(MemDC, RGB(iFontR, iFontG, iFontB));
	}

}

// �⺻ UI ���� �Լ�
void CMonitorGraphUnit::UISet()
{
	static double yTemp;
	static double Temp;
	static TCHAR GreedText[20];
	int data;

	// Ÿ��Ʋ, ȭ������, �۾����� ä���
	FillRect(MemDC, &TitleRt, TitleBrush);
	FillRect(MemDC, &LeftRt,BackBrush);
	FillRect(MemDC, &ClientRt, BackBrush);

	// Ÿ��Ʋ �ؽ�Ʈ ���
	SelectObject(MemDC, MyFont);
	TextOut(MemDC, TitleRt.top + 5, TitleRt.left + 5, tWinName, lstrlen(tWinName));

	//  Max�� ������ ���, �׷��� Max���� �Ź� ����
	if (MaxVariable == true)
	{
		// �ϴ�, ��� ť�� Peek �ϸ鼭 ���� ū �� 1���� ã�Ƴ���.
		if (FirstPeek(&queue, &data))
		{
			iMax = data;
			while (NextPeek(&queue, &data))
			{
				if(data > iMax)
					iMax = data;
			}
		}
		// ������� ���� iMax���� ���� ť�� ���� ū ���� ��� �ִ�.
		
		// ����, iMax�� 10���� �۴ٸ�, �׳� 10���� ����.. �׸��忡 ǥ���� ���ڰ� �ָ�����.
		if (iMax < 10)
			iMax = 10;

		// �׸��忡 ǥ���� ���� ����
		Greedint[0] = iMax / 4;							// ù �׸��忡 ǥ�õ� ����
		Greedint[1] = Greedint[0] + Greedint[0];		// �� ��° �׸��忡 ǥ�õ� ����
		Greedint[2] = Greedint[1] + Greedint[0];		// �� ��° �׸��忡 ǥ�õ� ����
		Greedint[3] = iMax;								// �� ��° �׸��忡 ǥ�õ� ����

		// �۾� ���� ����, y 1ĭ �̵� ��, ���� �̵��ؾ��ϴ� ��ġ ���
		iAddY = (double)(ClientRt.bottom - ClientRt.top) / iMax;

		// �˸��� �︱ �� �ٽ� ����. �׳� �۾� ������ �������� �ߴ�.
		AleCount = (ClientRt.bottom - ClientRt.top) / 2;	
	}

	// �׸��� �� �߱�, �ؽ�Ʈ ����ϱ�
	yTemp = ClientRt.bottom;
	Temp = (double)(ClientRt.bottom - ClientRt.top) / 4;
	SelectObject(MemDC, MyFont);
	for (int i = 0; i < 4; ++i)
	{
		yTemp -= Temp;
		MoveToEx(MemDC, ClientRt.left, (int)yTemp, NULL);
		LineTo(MemDC, ClientRt.right, (int)yTemp);

		// �׸��� �� ��ġ�� ����ϱ�
		_itow_s(Greedint[i], GreedText, _countof(GreedText), 10);
		TextOut(MemDC, LeftRt.left + 5, (int)yTemp - 6, GreedText, lstrlen(GreedText));
	}

}