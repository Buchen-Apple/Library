#include "stdafx.h"
#include "ChildWindow.h"

//// �ڽ� ������ Ŭ���� CPP ////////////////////////////
CMonitorGraphUnit *CMonitorGraphUnit::pThis;

// ������
CMonitorGraphUnit::CMonitorGraphUnit(HINSTANCE hInstance, HWND hWndParent, COLORREF BackColor, TYPE enType, int iPosX, int iPosY, int iWidth, int iHeight, LPCTSTR str)
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

	// ���� enType(������ Ÿ��)�� [LINE_MULTI] ���, ȭ�� ���� ���� ����. �׸��� Client������ �ٽ� ����.
	if (enType == LINE_MULTI)
	{
		RightRt = rt;
		RightRt.top = TitleRt.bottom;
		RightRt.left = ClientRt.right - 100;

		ClientRt.right = RightRt.left;
	}

	// ���� enType(������ Ÿ��)��[BAR_COLUMN_VERT] ���, ȭ�� �ϴ� ���� ����. Client������ �ٽ� ����.
	else if (enType == BAR_COLUMN_VERT)
	{
		BottomRt = rt;
		BottomRt.top = ClientRt.bottom - 40;
		BottomRt.left = LeftRt.right;

		ClientRt.bottom = BottomRt.top;
	}

	/////////////////////////////////////////////////////////
	//  Ŭ���� ��������� �ʿ��� �� ����
	/////////////////////////////////////////////////////////
	_enGraphType = enType;
	this->BackColor = BackColor;
	this->hWndParent = hWndParent;
	this->hWnd = hWnd;
	this->hInstance = hInstance;
	wcscpy_s(tWinName, _countof(tWinName), str);
	
	SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)this);

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

	// ��. �� 5���� ���� �����д�. 5���� �����Ҵ��Ѵ�.
	r = GetRValue(BackColor);
	g = GetGValue(BackColor);
	b = GetBValue(BackColor);
	r *= 3;
	g *= 3;
	b *= 3;

	GraphPen = new HPEN[5];	
	GraphPen[0] = CreatePen(PS_SOLID, 1, RGB(r, g, b));			// ���� ���� �⺻ �� ��
	GraphPen[1] = CreatePen(PS_SOLID, 1, RGB(0, 84, 255));		// �Ķ� �迭
	GraphPen[2] = CreatePen(PS_SOLID, 1, RGB(209, 178, 255));	// ���� �迭
	GraphPen[3] = CreatePen(PS_SOLID, 1, RGB(255, 228, 0));		// ��� �迭
	GraphPen[4] = CreatePen(PS_SOLID, 1, RGB(255, 94, 0));		// ���� �迭

	SelectObject(MemDC, GraphPen[0]);	// ���� �⺻ 0������ ����Ѵ�.
	
	// ��Ʈ
	NormalFontColor = RGB(r, g, b);
	MyFont = CreateFont(17, 0, 0, 0, 0, 0, 0, 0,
		DEFAULT_CHARSET, 0, 0, 0,
		VARIABLE_PITCH | FF_ROMAN, L"Arial"); 

	SetBkMode(MemDC, TRANSPARENT);	
	NowFontColor = NormalFontColor;
	SetTextColor(MemDC, NowFontColor);

	ReleaseDC(hWnd, hdc);	

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
	for(int i=0; i<5; ++i)
		DeleteObject(GraphPen[i]);

	SelectObject(MemDC, GetStockObject(DEFAULT_GUI_FONT));
	DeleteObject(MyFont);

	DeleteDC(MemDC);
}

// �ڽĵ��� ������ ���ν���
LRESULT CALLBACK CMonitorGraphUnit::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{

	pThis = (CMonitorGraphUnit*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
	switch (message)
	{
	case WM_PAINT:
	{		
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hWnd, &ps);	

		// �׸��� �� �⺻ UI ����
		pThis->UISet();

		// ���� ���
		switch (pThis->_enGraphType)
		{
		case BAR_SINGLE_VERT:
			pThis->Paint_BAR_SINGLE_VERT();
			break;

		case BAR_SINGLE_HORZ:
			break;

		case BAR_COLUMN_VERT:
			pThis->Paint_BAR_COLUMN_VERT();
			break;

		case  BAR_COLUMN_HORZ:
			break;

		case LINE_SINGLE:
			pThis->Paint_LINE_SINGLE();
			break;

		case LINE_MULTI:
			pThis->Paint_LINE_MULTI();
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

// ������ �ֱ�.
void CMonitorGraphUnit::InsertData(int iData, int ServerID, int DataType)
{	
	for (int i = 0; i < iColumnCount; ++i)
	{
		// for���� ���鼭, ServerID,DataType �� �� ������ �÷��� �ִ� ��� �����͸� �ְ� ���� ����
		if (CoInfo[i].ServerID == ServerID && CoInfo[i].DataType == DataType)
		{
			// ���� �������� ���� ���� ������ �д�.
			// ĸ�ǹٿ� ǥ�õ� ��.
			CoInfo[i].iLastVal = iData;

			// iData�� ��ť�Ѵ�.
			Enqueue(&CoInfo[i].queue, iData);

			// true�� �θ��� ȭ�� ������ ĥ�ϱ⸦ üũ �Ѵ�. false�� ����.
			if (AleOnOff == true)
				ParentBackCheck(iData);
		}
	}

	// �ڽ�(�ڽ�)�� WM_PAINT ȣ��
	InvalidateRect(hWnd, NULL, false);
}

// �߰� ������ ����. ������� [Max�� , �˶� �︮�� ��, ǥ���� ����]
void CMonitorGraphUnit::AddData(int iMax, int AleCount, LPCTSTR Unit)
{
	this->iMax = iMax;

	// iMax�� 0�̸� iMax �� ������ �ƴ϶�� ��.
	if (iMax == 0)
		MaxVariable = true;
	else
		MaxVariable = false;

	/////////////////////////////////////////////////////////
	//  �׷��� Max�� ����
	/////////////////////////////////////////////////////////
	// Max�� 0�� �ƴ϶��, �� ����. 0�̶�� �����̶�� ���̱� ������ �Ź� üũ����� �Ѵ�.
	if (!MaxVariable)
	{
		// �۾� ���� ����, y 1ĭ �̵� ��, ���� �̵��ؾ��ϴ� ��ġ ���
		iAddY = (double)(ClientRt.bottom - ClientRt.top) / iMax;

		Greedint[0] = iMax / 4;							// ù �׸��忡 ǥ�õ� ����
		Greedint[1] = Greedint[0] + Greedint[0];		// �� ��° �׸��忡 ǥ�õ� ����
		Greedint[2] = Greedint[1] + Greedint[0];		// �� ��° �׸��忡 ǥ�õ� ����
		Greedint[3] = iMax;								// �� ��° �׸��忡 ǥ�õ� ����
	}

	/////////////////////////////////////////////////////////
	// �˸��� �︱ �� ����
	/////////////////////////////////////////////////////////
	// AleCount���� 0�̸� �˶� �︮�� ����.
	if (AleCount == 0)
		AleOnOff = false;
	else
	{
		AleOnOff = true;
		this->AleCount = AleCount;
	}

	/////////////////////////////////////////////////////////
	// ǥ���� ���� �� ����.
	/////////////////////////////////////////////////////////
	// Unit�� ���ڰ� NULL�̶��, �ش� ������� ������ ������� �ʴ´ٴ� �ǹ�. 
	// �׷��� Unit�� ���ڰ� NULL�� �ƴ� ���� ���� �����Ѵ�.
	// ���� �������� ������ ����Ʈ�� tUnit���� ���� ����ִ�.
	if (_tcscmp(Unit, L"NULL"))
		_tcscpy_s(tUnit, _countof(tUnit), Unit);
		
}

// �÷� ���� ���� ����
void CMonitorGraphUnit::SetColumnInfo(int iColumnCount, int ServerID[], int DataType[], TCHAR DataName[][10])
{	
	// �÷� ī��Ʈ ����	
	this->iColumnCount = iColumnCount;

	// �÷� �ϳ��� [���� ID, ������ Ÿ��, ������ �̸�, ť] �� 4�� ������ �Ҵ�ȴ�.
	CoInfo = new ColumnInfo[iColumnCount];

	// �÷� ī��Ʈ ��ŭ �ݺ��ϸ鼭 ������ ä���.
	for (int i = 0; i < iColumnCount; ++i)
	{
		CoInfo[i].ServerID = ServerID[i];	// ���� ID ä���
		CoInfo[i].DataType = DataType[i];	// ������ Ÿ�� ä���
		_tcscpy_s(CoInfo[i].DataName, _countof(CoInfo[i].DataName), DataName[i]);// ������ �̸� ä���
		Init(&CoInfo[i].queue);			// ť �ʱ�ȭ
		Enqueue(&CoInfo[i].queue, 0);
	}

}

// LINE_SINGLE ���
void CMonitorGraphUnit::Paint_LINE_SINGLE(void)
{
	double x, y;
	int data;	// ��ť ��, ������ y��.	
	TCHAR tTitleText[30];	// �ϼ��� Ÿ��Ʋ �̸�.

	// Ÿ��Ʋ �ؽ�Ʈ ���
	// ������ �̸� : ť���� ���� ū ��, ǥ�ô��� ���·� ���ڿ� �ϼ�
	swprintf_s(tTitleText, _countof(tTitleText), _T("%s : %d %s"), tWinName, CoInfo[0].iLastVal, tUnit);
	SelectObject(MemDC, MyFont);
	TextOut(MemDC, TitleRt.top + 5, TitleRt.left + 5, tTitleText, lstrlen(tTitleText));

	// �׷��� ���
	if (FirstPeek(&CoInfo[0].queue, &data))	// ť�� ���� ó���� �ִ� y���� ��ť�Ѵ�.
	{
		x = ClientRt.left;	// �Ź� �׸� �� ����, ���� ����<<���� �׷��� �ϱ� ������ 0���� �����.
		y = ClientRt.bottom - (iAddY * data);			// ��ť�� ����, ���� ������ �� y��ǥ 1�� ����� ��, ���� ��ġ���� ����. 
		MoveToEx(MemDC, x, y, NULL);			// ó�� ���� ��ġ ����.	
		while (NextPeek(&CoInfo[0].queue, &data))	// ���� y�� ��ť.
		{
			x += (double)(ClientRt.right - ClientRt.left) / 100;	// �۾� ������ �ʺ� 100������� ������, 1��� �������� ��´�.
			y = ClientRt.bottom - (iAddY * data);		// ��ť�� ���� ��������, ���� ������ �� ��ǥ��ŭ �ٽ� �̵�
			LineTo(MemDC, x, y);	// �׸��� �� ���� ���� ���κ��� �ߴ´�.
		}
	}
}

// Paint_LINE_MULTI ���
void CMonitorGraphUnit::Paint_LINE_MULTI()
{
	double x, y;
	int data;	// ��ť ��, ������ y��.	
	TCHAR tTitleText[30];	// �ϼ��� Ÿ��Ʋ �̸�.
	int TextLineY = 20;

	// ȭ�� ���� ������ ĥ�Ѵ�. ���⼭ �ϴ� ������, ȭ�� ���� ������ LINE_MULTI������ ���� ������.
	FillRect(MemDC, &RightRt, BackBrush);

	// Ÿ��Ʋ �ؽ�Ʈ ���
	// ������ �̸� :  ǥ�ô��� ���·� ���ڿ� �ϼ�
	swprintf_s(tTitleText, _countof(tTitleText), _T("%s (%s)"), tWinName, tUnit);
	SelectObject(MemDC, MyFont);
	TextOut(MemDC, TitleRt.top + 5, TitleRt.left + 5, tTitleText, lstrlen(tTitleText));

	// �׷��� ���
	for (int i = 0; i < iColumnCount; ++i)
	{
		if (FirstPeek(&CoInfo[i].queue, &data))	// ť�� ���� ó���� �ִ� y���� ��ť�Ѵ�.
		{
			x = ClientRt.left;	// �Ź� �׸� �� ����, ���� ����<<���� �׷��� �ϱ� ������ 0���� �����.
			y = ClientRt.bottom - (iAddY * data);			// ��ť�� ����, ���� ������ �� y��ǥ 1�� ����� ��, ���� ��ġ���� ����. 
			MoveToEx(MemDC, x, y, NULL);			// ó�� ���� ��ġ ����.	
			while (NextPeek(&CoInfo[i].queue, &data))	// ���� y�� ��ť.
			{
				x += (double)(ClientRt.right - ClientRt.left)/ 100;	// �۾� ������ �ʺ� 100������� ������, 1��� �������� ��´�.
				y = ClientRt.bottom - (iAddY * data);		// ��ť�� ���� ��������, ���� ������ �� ��ǥ��ŭ �ٽ� �̵�
				LineTo(MemDC, x, y);	// �׸��� �� ���� ���� ���κ��� �ߴ´�.
			}
		}

		MoveToEx(MemDC, RightRt.left + 10, RightRt.top + TextLineY, NULL);	// ȭ�� ������ �׷��� ������ �˷��ִ� ������ �ߴ´�.
		LineTo(MemDC, RightRt.left +25, RightRt.top + TextLineY);

		//SetTextAlign(MemDC, TA_CENTER);
		TextOut(MemDC, RightRt.left + 29, RightRt.top + TextLineY - 8, CoInfo[i].DataName, lstrlen(CoInfo[i].DataName));
		//SetTextAlign(MemDC, TA_TOP | TA_LEFT);

		TextLineY += 30;	// ���� �׷��� ���� ��ġ�� ���� ��ġ �̵�

		SelectObject(MemDC, GraphPen[i+1]);	// ����, ���� �׷����� ������ ����
	}

	SelectObject(MemDC, GraphPen[0]);

}

// BAR_SINGLE_VERT ���
void CMonitorGraphUnit::Paint_BAR_SINGLE_VERT()
{
	TCHAR tNowVal[20];
	TCHAR tTitleText[30];	// �ϼ��� Ÿ��Ʋ �̸�.

	// Ÿ��Ʋ �ؽ�Ʈ ��� (������ �̸� : ��, Ÿ��)
	swprintf_s(tTitleText, _countof(tTitleText), _T("%s : %d %s"), tWinName, CoInfo[0].iLastVal, tUnit);
	SelectObject(MemDC, MyFont);
	TextOut(MemDC, TitleRt.top + 5, TitleRt.left + 5, tTitleText, lstrlen(tTitleText));

	/////////////////////////////////////////////////////////
	//  ���� �� ���
	/////////////////////////////////////////////////////////
	// ���� �ٴ� ���� �ֱ��� ���� �������� �簢���� �׸��� �ȴ�.	

	HBRUSH BarBrush = CreateSolidBrush(RGB(255, 228, 0));		// Bar�� ĥ�� ����� �귯�� ����
	HBRUSH OldBrush = (HBRUSH)SelectObject(MemDC, BarBrush);	// ����� �귯�� ����. ���� �귯�ô� OldBrush�� ����ִ�.
	HPEN OldPen = (HPEN)SelectObject(MemDC, GetStockObject(NULL_PEN));	// ���� ������ ������ ����.

	// Ractangle �� �簢���� ���� , ������ ��ġ�� ���Ѵ�.
	int RectLx = ClientRt.left + 20;
	int RectRx = ClientRt.right - 20;

	Rectangle(MemDC, RectLx, ClientRt.bottom - (CoInfo[0].iLastVal *iAddY), RectRx, ClientRt.bottom);	// ���� �� �׸���

	SelectObject(MemDC, OldBrush);	// ����� �귯�� ���� ����.
	SelectObject(MemDC, OldPen);	// ���� �� ���� ����.

	DeleteObject(BarBrush);	//����� �귯�� ����. ���� ���� GetStockObject���� ������ ������ �������� ��.

	/////////////////////////////////////////////////////////
	//  ���� ���� ����� ���� �� ���.	
	/////////////////////////////////////////////////////////
	SetTextColor(MemDC, RGB(0, 0, 0));	// ��Ʈ ���� ���������� ���� 
	HFONT BarFont = CreateFont(30, 0, 0, 0, 0, 0, 0, 0,	// Bar�� ���簪�� ǥ���� Ŀ�ٶ� ��Ʈ ����.
		DEFAULT_CHARSET, 0, 0, 0,
		VARIABLE_PITCH | FF_ROMAN, L"Arial");	

	SelectObject(MemDC, BarFont);		// ���� Ŀ�ٶ� ��Ʈ ����

	_itot_s(CoInfo[0].iLastVal, tNowVal, _countof(tNowVal), 10);		// ���� ���� TCHAR������ ����	
	
	SetTextAlign(MemDC, TA_CENTER);	// �ؽ�Ʈ ��� ��带 TA_CENTER�� ����. TextOut���� �����ϴ� ��ǥ�� �߾� ��ġ�� �ȴ�.
	TextOut(MemDC, RectLx + ((RectRx - RectLx) / 2), ClientRt.bottom - (CoInfo[0].iLastVal *iAddY) / 2, tNowVal, lstrlen(tNowVal));	// �ؽ�Ʈ ���
	SetTextAlign(MemDC, TA_TOP | TA_LEFT);	// �ؽ�Ʈ ��� ��带 �ٽ� ����Ʈ�� ����. (����Ʈ�� TA_TOP | TA_LEFT �̴�)
		
	SetTextColor(MemDC, NowFontColor);	// ��Ʈ �� ���� ������ ����
	SelectObject(MemDC, MyFont);	// ���� ��Ʈ(MyFont) ����.
	DeleteObject(BarFont);			// �� �� Ŀ�ٶ� ��Ʈ ����
}

// BAR_COLUMN_VERT ���
void CMonitorGraphUnit::Paint_BAR_COLUMN_VERT()
{
	TCHAR tNowVal[20];
	TCHAR tTitleText[30];	// �ϼ��� Ÿ��Ʋ �̸�.

	// ȭ�� �ϴ� ������ ĥ�Ѵ�. ���⼭ �ϴ� ������, ȭ�� �ϴ� ������ BAR_COLUMN_VERT������ ���� ������.
	FillRect(MemDC, &BottomRt, BackBrush);

	// Ÿ��Ʋ �ؽ�Ʈ ��� (������ �̸��� ǥ��)
	swprintf_s(tTitleText, _countof(tTitleText), _T("%s"), tWinName);
	SelectObject(MemDC, MyFont);
	TextOut(MemDC, TitleRt.top + 5, TitleRt.left + 5, tTitleText, lstrlen(tTitleText));

	/////////////////////////////////////////////////////////
	//  ���� �� ���
	/////////////////////////////////////////////////////////
	// BAR�� ���� �ֱ��� ���� �������� �簢���� �׸��� �ȴ�.	

	HBRUSH BarBrush = CreateSolidBrush(RGB(255, 228, 0));		// Bar�� ĥ�� ����� �귯�� ����
	HBRUSH OldBrush = (HBRUSH)SelectObject(MemDC, BarBrush);	// ����� �귯�� ����. ���� �귯�ô� OldBrush�� ����ִ�.
	HPEN OldPen = (HPEN)SelectObject(MemDC, GetStockObject(NULL_PEN));	// ���� ������ ������ ����.
	HFONT BarFont = CreateFont(30, 0, 0, 0, 0, 0, 0, 0,	// Bar�� ���簪�� ǥ���� Ŀ�ٶ� ��Ʈ ����.
		DEFAULT_CHARSET, 0, 0, 0,
		VARIABLE_PITCH | FF_ROMAN, L"Arial");
	SelectObject(MemDC, BarFont);		// ���� Ŀ�ٶ� ��Ʈ ����
	
	// Ractangle �� ���� �簢���� ���� , ������ ��ġ�� ���Ѵ�.
	int RectLx = ClientRt.left + 20;
	int RectRx = ClientRt.left + 120;

	SetTextColor(MemDC, RGB(0, 0, 0));	// ��Ʈ ���� ���������� ���� 	
	SetTextAlign(MemDC, TA_CENTER);	// �ؽ�Ʈ ��� ��带 TA_CENTER�� ����. TextOut���� �����ϴ� ��ǥ�� �߾� ��ġ�� �ȴ�.

	// �÷� �� ��ŭ �ݺ��ϸ鼭 �簢���� �׸��� / �ٿ� �� �ؽ�Ʈ ���
	for (int i = 0; i < iColumnCount; ++i)
	{
		Rectangle(MemDC, RectLx, ClientRt.bottom - (CoInfo[i].iLastVal *iAddY), RectRx, ClientRt.bottom);	// ���� �� �׸���
		_itot_s(CoInfo[i].iLastVal, tNowVal, _countof(tNowVal), 10);		// ���� ť�� ������ ���� TCHAR������ ����
		TextOut(MemDC, RectLx + ((RectRx - RectLx) / 2), ClientRt.bottom - (CoInfo[i].iLastVal *iAddY) / 2, tNowVal, lstrlen(tNowVal));	// Ractangle�� �� ����� �ؽ�Ʈ ���.

		RectLx = RectRx + 20;
		RectRx = RectLx + 120;
	}

	// �ٽ� �÷� �� ��ŭ �ݺ��ϸ鼭 ȭ�� �ϴܿ� ������ Ÿ�� �̸� ����ϱ�. �簢�� �׸���� �����ϴ� ������ �ؽ�Ʈ ���� ��Ʈ�� �ٸ��� �����̴�.
	// ��� SelectObject�� SetTextColor�� �ݺ��ϴ� �� ���ٴ� ���� ���°� �� ���ڴٴ� �Ǵ��̴�.
	SetTextColor(MemDC, NowFontColor);	// ��Ʈ �� ���� ������ ����
	SelectObject(MemDC, MyFont);	// ���� ��Ʈ(MyFont) ����.
	RectLx = ClientRt.left + 20;
	RectRx = ClientRt.left + 120;
	for (int i = 0; i < iColumnCount; ++i)
	{
		TextOut(MemDC, RectLx + ((RectRx - RectLx) / 2), BottomRt.bottom - 20, CoInfo[i].DataName, _tcslen(CoInfo[i].DataName));	// ȭ�� �ϴܿ� ������ Ÿ�� �̸� ���
		RectLx = RectRx + 20;
		RectRx = RectLx + 120;
	}

	SetTextAlign(MemDC, TA_TOP | TA_LEFT);	// �ؽ�Ʈ ��� ��带 �ٽ� ����Ʈ�� ����. (����Ʈ�� TA_TOP | TA_LEFT �̴�)	
	DeleteObject(BarFont);			// �� �� Ŀ�ٶ� ��Ʈ ����

	SelectObject(MemDC, OldBrush);	// ����� �귯�� ���� ����.
	SelectObject(MemDC, OldPen);	// ���� �� ���� ����.
	DeleteObject(BarBrush);	//����� �귯�� ����. ���� ���� GetStockObject���� ������ ������ �������� ��.
}

// �θ��� ��׶��� ���������� ĥ�ϱ�
void CMonitorGraphUnit::ParentBackCheck(int data)
{
	// �������� ��ť�� Data�� �˶����� �۴ٸ�, �θ𿡰� ȭ�� ������ �϶�� ������ �޽����� ������. 
	// �̹� ���������� �ƴ����� �θ� üũ
	if (data > AleCount)
	{
		SendMessage(hWndParent, UM_PARENTBACKCHANGE, 0, 0);	// �θ𿡰� ȭ�� ������ �϶�� ����.

		// �θ𿡰� ������ �϶�� ������ ��, ��Ʈ�� �� ����. 
		// ���� �˶� ��ġ���� ���� ���� ���, ��Ʈ �� ���������� ����.
		// ��, ��� �����찡 ��Ʈ ���������� �� ���� �ִ�.
		// �̹� �����ߴٸ�(bObjectCheck�� true) if�� �۵� ����.
		if (!bObjectCheck)
		{
			bObjectCheck = true;
			NowFontColor = RGB(255, 0, 0);
			SetTextColor(MemDC, NowFontColor);
			// Timer�� �����Ѵ�. ������ ��Ʈ ǥ�� �ð��� ����.
		}
	}

	// Data�� �˶����� �۴ٸ�, �ٽ� �� ��Ʈ�� ������� ����. 
	else
	{
		bObjectCheck = false;
		NowFontColor = NormalFontColor;
		SetTextColor(MemDC, NowFontColor);
	}
}

// �⺻ UI ���� �Լ�
void CMonitorGraphUnit::UISet()
{
	double yTemp;
	double Temp;
	TCHAR GreedText[20];
	int data;

	// Ÿ��Ʋ, ȭ������, �۾����� �� ä���
	FillRect(MemDC, &TitleRt, TitleBrush);
	FillRect(MemDC, &LeftRt, BackBrush);
	FillRect(MemDC, &ClientRt, BackBrush);

	//  Max�� ������ ���, �׷��� Max���� �Ź� ����
	if (MaxVariable == true)
	{
		int iMax = 0;
		// �ϴ�, ��� ť�� Peek �ϸ鼭 ���� ū �� 1���� ã�Ƴ���.
		for (int i = 0; i < iColumnCount; ++i)
		{
			if (FirstPeek(&CoInfo[i].queue, &data))
			{
				if(iMax < data)
					iMax = data;
				while (NextPeek(&CoInfo[i].queue, &data))
				{
					if (data > iMax)
						iMax = data;
				}
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