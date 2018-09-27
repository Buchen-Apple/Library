// Project1.cpp: 응용 프로그램의 진입점을 정의합니다.
//

#include "pch.h"
#include "Resource.h"
#include "MonitorClient.h"

#include <mmsystem.h>
#pragma comment(lib, "winmm.lib")

using namespace Library_Jingyu;

LONG g_lAllocNodeCount;


// 전역 변수
HINSTANCE g_hInst; // 현재 인스턴스입니다.
HWND g_hWnd;		// 윈도우 핸들


LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);


int APIENTRY _tWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	// TODO: 여기에 코드를 입력합니다.

	// timeBeginPeriod 적용
	timeBeginPeriod(1);

	// 윈도우 클래스 세팅
	WNDCLASSEXW wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_MONITORCLIENTMAIN));
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_MONITORCLIENTMAIN);
	wcex.lpszClassName = TEXT("MonitorView");
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	RegisterClassExW(&wcex);


	// 응용 프로그램 초기화를 수행합니다.

	g_hInst = hInstance; // 인스턴스 핸들을 전역 변수에 저장합니다.

	g_hWnd = CreateWindowW(TEXT("MonitorView"), TEXT("모니터링 뷰어"), WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,
		CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

	if (!g_hWnd)
		return FALSE;

	ShowWindow(g_hWnd, nCmdShow);
	UpdateWindow(g_hWnd);

	MSG msg;

	// 기본 메시지 루프입니다.
	while (GetMessage(&msg, nullptr, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	timeEndPeriod(1);

	return (int)msg.wParam;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	static RECT rt;
	static HBRUSH MyBrush, RedBrush;
	static bool bBackCheck = false;
	static bool bSoundCheck = false;
	static CMonitorClient cViewClient;

	switch (message)
	{
	case WM_CREATE:
		GetClientRect(hWnd, &rt);
		MyBrush = CreateSolidBrush(RGB(150, 150, 150));
		RedBrush = CreateSolidBrush(RGB(255, 0, 0));

		// 모니터링 클래스 셋팅
		cViewClient.SetMonitorClass(g_hInst, hWnd);

		// 모니터링 서버와 연결
		cViewClient.ClientStart();

		// 화면을 그릴 주기. 
		// 이 시간동안 안온 데이터는 무시한다.
		SetTimer(hWnd, 1, 1200, NULL);	

		break;
	case WM_COMMAND:
	{
		// 메뉴 선택을 구문 분석합니다.
		switch (LOWORD(wParam))
		{
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
	}
	break;

	case WM_SIZE:
		GetClientRect(hWnd, &rt);
		break;	

	case WM_TIMER:
	{
		// 모든 자식 윈도우에게 데이터 전달하는 타이머
		if (wParam == 1)
		{	
			cViewClient.Update();					
		}

		// 화면 빨갛게 만들기 종료 타이머
		else if (wParam == 2)
		{
			KillTimer(hWnd, 2);
			bBackCheck = false;
			InvalidateRect(hWnd, NULL, true);
		}

		// 알람 사운드를 끄는 타이머
		else if (wParam == 3)
		{
			bSoundCheck = false;
			KillTimer(hWnd, 3);
		}
	}

		break;

	case UM_PARENTBACKCHANGE:
		if (!bBackCheck)
		{
			// 자식으로부터 메시지 도착. 화면을 빨갛게 만들라는 것.
			SetTimer(hWnd, 2, 200, NULL);
			bBackCheck = true;

			if (!bSoundCheck)
			{
				PlaySound(TEXT("SystemDefault"), 0, SND_ALIAS | SND_ASYNC);	// 화면을 빨갛게 해야 할 경우, 에러 사운드를 출력한다.
				bSoundCheck = true;
				SetTimer(hWnd, 3, 700, NULL);	// 사운드가 씹히는 현상을 막기 위해, 타이머를 이용해 강제 700밀리세컨드 시간동안 출력.
			}

			InvalidateRect(hWnd, NULL, true);
		}
		break;

	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hWnd, &ps);

		// ---------------------
		// 빨간색으로 칠하기 체크
		// ---------------------

		// bBackCheck가 true면 빨간색으로 칠한다.
		if (bBackCheck)	
			FillRect(hdc, &rt, RedBrush);

		// bBackCheck가 false면 본래 색으로 칠한다.
		else 
			FillRect(hdc, &rt, MyBrush);

		EndPaint(hWnd, &ps);
	}
	break;

	case WM_DESTROY:
		PostQuitMessage(0);		
		KillTimer(hWnd, 1);
		DeleteObject(MyBrush);
		DeleteObject(RedBrush);
		break;

	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}