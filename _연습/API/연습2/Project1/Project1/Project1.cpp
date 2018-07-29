#include "stdafx.h"
#include <locale.h>

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
HINSTANCE g_hInst;
HWND hWndMain;
LPCTSTR lpszClass = TEXT("Menu");

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdParam, int nCmdShow)
{
	HWND hWnd;
	MSG Message;
	WNDCLASS WndClass;
	g_hInst = hInstance;

	WndClass.cbClsExtra = 0;
	WndClass.cbWndExtra = 0;
	WndClass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);    //(HBRUSH)(COLOR_WINDOW+1);
	WndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	WndClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	WndClass.hInstance = hInstance;
	WndClass.lpfnWndProc = WndProc;
	WndClass.lpszClassName = lpszClass;
	WndClass.lpszMenuName = NULL;
	WndClass.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
	RegisterClass(&WndClass);

	hWnd = CreateWindow(lpszClass, lpszClass, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL, (HMENU)NULL, hInstance, NULL);
	_wsetlocale(LC_ALL, L"korean");
	ShowWindow(hWnd, nCmdShow);

	while (GetMessage(&Message, NULL, 0, 0))
	{
		TranslateMessage(&Message);
		DispatchMessage(&Message);
	}

	return (int)Message.wParam;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam)
{
	HDC hdc, MemDC;
	PAINTSTRUCT ps;
	HBITMAP MyBitmap, OldBitmap;
	RECT rt, Myrt;
	TCHAR str[20];
	static int x, y;

	switch (iMessage)
	{
	case WM_CREATE:
		hWndMain = hWnd;
		return 0;

	case WM_MOUSEMOVE:
		x = LOWORD(lParam);
		y = HIWORD(lParam);
		InvalidateRect(hWnd, NULL, false);
		return 0;

	case WM_PAINT:

		hdc = BeginPaint(hWnd, &ps);
		GetClientRect(hWnd, &rt);
		Myrt = rt;
		Myrt.right = 4000;
		Myrt.bottom = 4000;

		MemDC = CreateCompatibleDC(hdc);

		SetMapMode(MemDC, MM_ISOTROPIC);
		SetWindowExtEx(MemDC, Myrt.right, Myrt.bottom, NULL);
		SetViewportExtEx(MemDC, rt.right, rt.bottom, NULL);

		MyBitmap = CreateCompatibleBitmap(hdc, Myrt.right, Myrt.bottom);
		OldBitmap = (HBITMAP)SelectObject(MemDC, MyBitmap);

		FillRect(MemDC, &Myrt, (HBRUSH)GetStockObject(WHITE_BRUSH));
		wsprintf(str, L"x,y : %d, %d", x, y);
		TextOut(MemDC, 10, 10, str, lstrlen(str));		
		
		BitBlt(hdc, Myrt.left, Myrt.top, Myrt.right, Myrt.bottom, MemDC, Myrt.left, Myrt.top, SRCCOPY);

		EndPaint(hWnd, &ps);

		return 0;

	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}

	return(DefWindowProc(hWnd, iMessage, wParam, lParam));
}