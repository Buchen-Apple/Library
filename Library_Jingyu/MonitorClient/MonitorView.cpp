#include "pch.h"
#include "MonitorView.h"
#include <math.h>

// --------------- 
// ť ����
// --------------- 
namespace Library_Jingyu
{
	// �ʱ�ȭ
	void Init(Queue* pq)
	{
		pq->front = 0;
		pq->rear = 0;
	}

	// ���� ��ġ Ȯ��
	int NextPos(int pos)
	{
		if (pos == QUEUE_LEN - 1)
			return 0;
		else
			return pos + 1;
	}

	// ��ť
	int Dequeue(Queue* pq)
	{
		pq->front = NextPos(pq->front);
		return pq->queArr[pq->front];
	}

	// ��ť
	void Enqueue(Queue* pq, int y)
	{
		if (pq->front == NextPos(pq->rear))
		{
			Dequeue(pq);
		}
		pq->rear = NextPos(pq->rear);
		pq->queArr[pq->rear] = y;
	}

	// ù ť Peek
	bool FirstPeek(Queue* pq, int* Data)
	{
		if (pq->front == pq->rear)
			return false;

		pq->Peek = NextPos(pq->front);

		*Data = pq->queArr[pq->Peek];
		return true;

	}

	// ���� ť Peek
	bool NextPeek(Queue* pq, int* Data)
	{
		if (pq->Peek == pq->rear)
			return false;

		pq->Peek = NextPos(pq->Peek);

		*Data = pq->queArr[pq->Peek];
		return true;

	}
}

// --------------- 
// ����͸� ���
// --------------- 
namespace Library_Jingyu
{
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

		HWND hWnd = CreateWindowW(TEXT("�ڽ�������"), TEXT("�ڽ��������Ӵ�"), WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS,
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
		if (enType == LINE_MULTI || enType == PIE)
		{
			RightRt = rt;
			RightRt.top = TitleRt.bottom;
			RightRt.left = ClientRt.right - 100;

			ClientRt.right = RightRt.left;
		}

		// ���� enType(������ Ÿ��)��[BAR_COLUMN_VERT] ���, ȭ�� �ϴ� ���� ����. Client������ �ٽ� ����.
		if (enType == BAR_COLUMN_VERT)
		{
			BottomRt = rt;
			BottomRt.top = ClientRt.bottom - 40;
			BottomRt.left = LeftRt.right;

			ClientRt.bottom = BottomRt.top;
		}

		// ���� enType(������ Ÿ��)��[PIE] ���, ȭ�� ���� ���� �ʿ����. Client���� �ٽ� ����.
		if (enType == PIE)
			ClientRt.left = rt.left;


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

		// ���̿� �귯�� ����. PIE�� ������, Paint�� �� ���� ���� ����°� ���񰰾Ƽ� �׳� �� ����� �д�.
		PieBrush[0] = CreateSolidBrush(RGB(r, g, b));
		PieBrush[1] = CreateSolidBrush(RGB(0, 84, 255));
		PieBrush[2] = CreateSolidBrush(RGB(209, 178, 255));
		PieBrush[3] = CreateSolidBrush(RGB(255, 228, 0));
		PieBrush[4] = CreateSolidBrush(RGB(255, 94, 0));

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
		for (int i = 0; i < 5; ++i)
			DeleteObject(PieBrush[i]);

		SelectObject(MemDC, GetStockObject(WHITE_PEN));
		for (int i = 0; i < 5; ++i)
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
				pThis->Paint_PIE();
				break;

			case ONOFF:
				pThis->paint_ONOFF();
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

				// ���⿡�� �ּ� üũ
				if(MinAleOnOff == true)
					ParentBackCheck_Min(iData);
			}
		}

		// �ڽ�(�ڽ�)�� WM_PAINT ȣ��
		InvalidateRect(hWnd, NULL, false);
	}

	// �߰� ������ ����. ������� [Max�� , �˶� �︮�� �ִ밪, �˶� �︮�� �ּҰ�, ǥ���� ����]
	void CMonitorGraphUnit::AddData(int iMax, int MaxAleCount, int MinAleCount, LPCTSTR Unit)
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
		// ���ڷ� ���� MaxAleCount ����
		// ���� 0�̸� �˶� �︮�� ����.
		if (MaxAleCount == 0)
			AleOnOff = false;
		else
		{
			AleOnOff = true;
			AleCount = MaxAleCount;
		}

		// ���ڷ� ���� MinAleCount ����
		// ���� 0�̸� �˶� �︮�� ����.
		if (MinAleCount == 0)
			MinAleOnOff = false;
		else
		{
			MinAleOnOff = true;
			this->MinAleCount = MinAleCount;
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
	void CMonitorGraphUnit::SetColumnInfo(int iColumnCount, int ServerID[], int DataType[], TCHAR DataName[][20])
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
			MoveToEx(MemDC, (int)x, (int)y, NULL);			// ó�� ���� ��ġ ����.	
			while (NextPeek(&CoInfo[0].queue, &data))	// ���� y�� ��ť.
			{
				x += (double)(ClientRt.right - ClientRt.left) / 100;	// �۾� ������ �ʺ� 100������� ������, 1��� �������� ��´�.
				y = ClientRt.bottom - (iAddY * data);		// ��ť�� ���� ��������, ���� ������ �� ��ǥ��ŭ �ٽ� �̵�
				LineTo(MemDC, (int)x, (int)y);	// �׸��� �� ���� ���� ���κ��� �ߴ´�.
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
				MoveToEx(MemDC, (int)x, (int)y, NULL);			// ó�� ���� ��ġ ����.	
				while (NextPeek(&CoInfo[i].queue, &data))	// ���� y�� ��ť.
				{
					x += (double)(ClientRt.right - ClientRt.left) / 100;	// �۾� ������ �ʺ� 100������� ������, 1��� �������� ��´�.
					y = ClientRt.bottom - (iAddY * data);		// ��ť�� ���� ��������, ���� ������ �� ��ǥ��ŭ �ٽ� �̵�
					LineTo(MemDC, (int)x, (int)y);	// �׸��� �� ���� ���� ���κ��� �ߴ´�.
				}
			}

			MoveToEx(MemDC, RightRt.left + 10, RightRt.top + TextLineY, NULL);	// ȭ�� ������ �׷��� ������ �˷��ִ� ������ �ߴ´�.
			LineTo(MemDC, RightRt.left + 25, RightRt.top + TextLineY);

			TextOut(MemDC, RightRt.left + 29, RightRt.top + TextLineY - 8, CoInfo[i].DataName, lstrlen(CoInfo[i].DataName));

			TextLineY += 30;	// ���� �׷��� ���� ��ġ�� ���� ��ġ �̵�

			SelectObject(MemDC, GraphPen[i + 1]);	// ����, ���� �׷����� ������ ����
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

		Rectangle(MemDC, RectLx, (int)(ClientRt.bottom - (CoInfo[0].iLastVal *iAddY)), RectRx, ClientRt.bottom);	// ���� �� �׸���

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
		TextOut(MemDC, (int)(RectLx + ((RectRx - RectLx) / 2)), (int)(ClientRt.bottom - (CoInfo[0].iLastVal *iAddY) / 2), tNowVal, lstrlen(tNowVal));	// �ؽ�Ʈ ���
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
			Rectangle(MemDC, RectLx, (int)(ClientRt.bottom - (CoInfo[i].iLastVal *iAddY)), RectRx, ClientRt.bottom);	// ���� �� �׸���
			_itot_s(CoInfo[i].iLastVal, tNowVal, _countof(tNowVal), 10);		// ���� ť�� ������ ���� TCHAR������ ����
			TextOut(MemDC, (int)(RectLx + ((RectRx - RectLx) / 2)), (int)(ClientRt.bottom - (CoInfo[i].iLastVal *iAddY) / 2), tNowVal, (int)lstrlen(tNowVal));	// Ractangle�� �� ����� �ؽ�Ʈ ���.

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
			TextOut(MemDC, RectLx + ((RectRx - RectLx) / 2), BottomRt.bottom - 20, CoInfo[i].DataName, (int)_tcslen(CoInfo[i].DataName));	// ȭ�� �ϴܿ� ������ Ÿ�� �̸� ���
			RectLx = RectRx + 20;
			RectRx = RectLx + 120;
		}

		SetTextAlign(MemDC, TA_TOP | TA_LEFT);	// �ؽ�Ʈ ��� ��带 �ٽ� ����Ʈ�� ����. (����Ʈ�� TA_TOP | TA_LEFT �̴�)	
		DeleteObject(BarFont);			// �� �� Ŀ�ٶ� ��Ʈ ����

		SelectObject(MemDC, OldBrush);	// ����� �귯�� ���� ����.
		SelectObject(MemDC, OldPen);	// ���� �� ���� ����.
		DeleteObject(BarBrush);	//����� �귯�� ����. ���� ���� GetStockObject���� ������ ������ �������� ��.
	}

	// PIE ���
	void CMonitorGraphUnit::Paint_PIE()
	{
		TCHAR tTitleText[30];					// �ϼ��� Ÿ��Ʋ �̸�.
		int TextLineY = 20;						// ȭ�� ������ �ؽ�Ʈ ǥ���� Y��ġ

		int iData;
		int iFontData;
		int iTotalData = 0;						// �� ������ ��
		int iDiameter = 300;					// pie �׷��� ����

		int iGraphDrawX = ClientRt.left + 10;	// pie �׷��� ���� ��ġ. �»��
		int iGraphDrawY = ClientRt.top + 10;	// pie �׷��� ���� ��ġ. �»��

		int iX = iDiameter;						// ȣ 1���� ���� X��ǥ
		int iY = iDiameter / 2;					// ȣ 1���� ���� Y��ǥ

		int iEndX = iX;						// ȣ 1���� ���� X��ǥ
		int iEndY = iY;						// ȣ 1���� ���� Y��ǥ. iX,IY ���� iEndX, iEndY���� ȣ�� �����.

		double dfAccuRadian = 0;				// ���� ���� ��ġ ���� ����.
		double dfFontFadian = 0;				// ���� ���� ��ġ ���� ����. (��Ʈ ǥ�ø� ���Ѱ�)

		// ȭ�� ���� ������ ĥ�Ѵ�. ���⼭ �ϴ� ������, ȭ�� ���� ������ LINE_MULTI������ ���� ������.
		FillRect(MemDC, &RightRt, BackBrush);

		// Ÿ��Ʋ �ؽ�Ʈ ��� (������ �̸��� ǥ��)
		swprintf_s(tTitleText, _countof(tTitleText), _T("%s (%s)"), tWinName, tUnit);
		SelectObject(MemDC, MyFont);
		TextOut(MemDC, TitleRt.top + 5, TitleRt.left + 5, tTitleText, lstrlen(tTitleText));


		/////////////////////////////////////////////////////////
		//  �� �տ� ���� �� �������� ������ ������ �˾Ƴ���.
		//  360�� -> 6.2831 ���� (2����)
		//  ��ü : 6.2831 / �� ������ : N Radian
		/////////////////////////////////////////////////////////
		// �� ������ �� ���ϱ�.
		for (int i = 0; i < iColumnCount; ++i)
			iTotalData += CoInfo[i].iLastVal;

		HBRUSH OldBrush = (HBRUSH)SelectObject(MemDC, PieBrush[0]);

		for (int i = 0; i < iColumnCount; ++i)
		{
			iData = CoInfo[i].iLastVal;
			iFontData = iData / 2;

			// ���� ���� ���ϱ� (����ġ)
			dfAccuRadian += iData * 6.2831 / iTotalData;
			dfFontFadian += iFontData * 6.2831 / iTotalData;

			// ���ȿ� ���� X�� ��, �׸��� 100�� Ȯ��.
			iX = iDiameter / 2 + (int)(cos(dfAccuRadian) * 100);

			// ���ȿ� ���� Y�� ��, �׸��� 100�� Ȯ��.
			iY = iDiameter / 2 + (int)(sin(dfAccuRadian) * 100);

			// ���� ���.
			HPEN OldPen = (HPEN)SelectObject(MemDC, GetStockObject(NULL_PEN));	// ��� �� ��� �������� ����
			Pie(MemDC, iGraphDrawX, iGraphDrawY, iGraphDrawX + iDiameter, iGraphDrawY + iDiameter,
				iGraphDrawX + iX,
				iGraphDrawY + iY,
				iGraphDrawX + iEndX,
				iGraphDrawY + iEndY);
			SelectObject(MemDC, OldPen);	//�ٽ� ���������� ����				

			// ��Ʈ ǥ�ø� ���� X, Y�� ��, �׸��� 100�� Ȯ��
			int iFontX = iDiameter / 2 + (int)(cos(dfFontFadian) * 80);
			int iFontY = iDiameter / 2 + (int)(sin(dfFontFadian) * 80);

			// ��Ʈ ��ġ�� ��������, ��Ʈ ��ġ ���ȵ� ������ �̵���Ų��.
			dfFontFadian += iFontData * 6.2831 / iTotalData;

			// ��ü ������ ����, iData�� ������ ���� ��, ��Ʈ������ �����Ѵ�.
			int iPercent = 100 * iData / iTotalData;
			TCHAR tPercentText[5];
			swprintf_s(tPercentText, _countof(tPercentText), _T("%d%s"), iPercent, tUnit);

			// n%�� ����Ѵ�.
			SetTextColor(MemDC, RGB(0, 0, 0));	// ��Ʈ ���� ���������� ���� 	
			SetTextAlign(MemDC, TA_CENTER);
			TextOut(MemDC, iGraphDrawX + iFontX, iGraphDrawY + iFontY, tPercentText, lstrlen(tPercentText));
			SetTextColor(MemDC, NowFontColor);	// ��Ʈ �� ���� ������ ����
			SetTextAlign(MemDC, TA_TOP | TA_LEFT);
			iEndX = iX;
			iEndY = iY;

			// ȭ�� ������ �׷��� ������ �˷��ִ� ������ �ߴ´�.
			MoveToEx(MemDC, RightRt.left + 10, RightRt.top + TextLineY, NULL);
			LineTo(MemDC, RightRt.left + 25, RightRt.top + TextLineY);

			// ȭ�� ������ �ؽ�Ʈ ���
			TextOut(MemDC, RightRt.left + 29, RightRt.top + TextLineY - 8, CoInfo[i].DataName, lstrlen(CoInfo[i].DataName));

			// ���� �׷��� ���� ��ġ�� ���� ��ġ �̵�
			TextLineY += 30;

			// ����, ���� �׷����� ������ ����
			SelectObject(MemDC, GraphPen[i + 1]);

			// �귯�õ� ���� ������ ����
			SelectObject(MemDC, PieBrush[i + 1]);

		}
		SelectObject(MemDC, OldBrush);
		SelectObject(MemDC, GraphPen[0]);
	}

	// ���� On/Off ��¿�
	void CMonitorGraphUnit::paint_ONOFF()
	{
		int data;	// ��ť ��, ������ y��.	
		TCHAR tTitleText[30];	// �ϼ��� Ÿ��Ʋ �̸�.

		// Ÿ��Ʋ �ؽ�Ʈ ���
		// ������ �̸� : �ܺο��� ������ WinName
		swprintf_s(tTitleText, _countof(tTitleText), _T("%s"), tWinName);
		SelectObject(MemDC, MyFont);
		TextOut(MemDC, TitleRt.top + 5, TitleRt.left + 5, tTitleText, lstrlen(tTitleText));

		SetTextAlign(MemDC, TA_CENTER);	// �ؽ�Ʈ ����, �߾����ķ� ����

		HFONT BigFont = CreateFont(70, 0, 0, 0, 0, 0, 0, 0,	// ���� ���¸� ǥ���� Ŀ�ٶ� ��Ʈ ����.
			DEFAULT_CHARSET, 0, 0, 0,
			VARIABLE_PITCH | FF_ROMAN, L"Arial");
		SelectObject(MemDC, BigFont);		// ���� Ŀ�ٶ� ��Ʈ ����

		// ť���� ������ 1�� ������ �����ϱ�.
		if (FirstPeek(&CoInfo[0].queue, &data))	// OnOff�� ������ �÷� 1����� ����
		{
			CoInfo[0].iLastVal = data;

			// Peek�ߴ� �� �ϳ� ��ť�Ѵ�.
			Dequeue(&CoInfo[0].queue);
		}

		// �׷��� ���
		if (CoInfo[0].iLastVal == 1)
		{
			TCHAR tShowText[] = _T("ON");
			TextOut(MemDC, ClientRt.top + 40, ClientRt.left, tShowText, lstrlen(tShowText));
		}

		else
		{
			TCHAR tShowText[] = _T("OFF");
			TextOut(MemDC, ClientRt.top + 40, ClientRt.left, tShowText, lstrlen(tShowText));
		}

		SelectObject(MemDC, MyFont);	// ���� ��Ʈ(MyFont) ����.
		SetTextAlign(MemDC, TA_LEFT);	// ���� �ؽ�Ʈ ���� ����

		DeleteObject(BigFont);			// �� �� Ŀ�ٶ� ��Ʈ ����
	}

	// �θ��� ��׶��� ���������� ĥ�ϱ� (�ִ� üũ��)
	void CMonitorGraphUnit::ParentBackCheck(int data)
	{
		// �������� ��ť�� Data�� �˶����� ũ�ٸ�, �θ𿡰� ȭ�� ������ �϶�� ������ �޽����� ������. 
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

	// �θ��� ��׶��� ���������� ĥ�ϱ� (�ּ� üũ��)
	void CMonitorGraphUnit::ParentBackCheck_Min(int data)
	{
		// �������� ��ť�� Data�� �˶����� �۴ٸ�, �θ𿡰� ȭ�� ������ �϶�� ������ �޽����� ������. 
		// �̹� ���������� �ƴ����� �θ� üũ
		if (data < MinAleCount)
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

		// Data�� �˶����� ũ�ٸ�, �ٽ� �� ��Ʈ�� ������� ����. 
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
					if (iMax < data)
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

			// iMax�� 10���� ũ��, 99���� �۴ٸ�, Max +5�� ��¥ Max�� ��´�.
			else if (iMax < 99)
				iMax = iMax + 5;

			//iMax�� 99���� ũ��, 999���� �۴ٸ�, Max +20�� ��¥ Max�� ��´�.
			else if (iMax < 999)
				iMax = iMax + 20;

			//iMax�� 999���� ũ��, 9999���� �۴ٸ�, Max +500�� ��¥ Max�� ��´�.
			else if (iMax < 9999)
				iMax = iMax + 500;

			// �׸��忡 ǥ���� ���� ����
			Greedint[0] = iMax / 4;							// ù �׸��忡 ǥ�õ� ����
			Greedint[1] = Greedint[0] + Greedint[0];		// �� ��° �׸��忡 ǥ�õ� ����
			Greedint[2] = Greedint[1] + Greedint[0];		// �� ��° �׸��忡 ǥ�õ� ����
			Greedint[3] = iMax;								// �� ��° �׸��忡 ǥ�õ� ����

			// �۾� ���� ����, y 1ĭ �̵� ��, ���� �̵��ؾ��ϴ� ��ġ ���
			iAddY = (double)(ClientRt.bottom - ClientRt.top) / iMax;
		}

		// �׷��� Ÿ���� PIE�� ON/OFF�� �ƴ� ���� �׸��带 �ߴ´�. 
		// �����϶��� ���� ���;� �ϱ� ������ �׸��� �ʿ� ����.
		// ON/OFF �϶��� �׳� �ؽ�Ʈ�� ����ϱ⶧���� �ʿ� ����.
		if (_enGraphType != PIE && _enGraphType != ONOFF)
		{
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

	}

}