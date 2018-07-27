#include "stdafx.h"
#include "ScreenDib.h"


//---------------------------
// ������ (private)
// --------------------------
CScreenDib::CScreenDib(int iWidth, int iHeight, int iColorBit)
{
	// ���� ������� �ʱ�ȭ
	m_iWidth = iWidth;
	m_iHeight = iHeight;
	m_iColorBit = iColorBit;	

	// �� ����� ���� �Լ� ȣ��
	CreateDibBuffer(m_iWidth, m_iHeight, m_iColorBit);
}

//---------------------------
// �ı��� (public)
// --------------------------
// ���� �ı���.
CScreenDib::~CScreenDib()
{
	ReleseDibBuffer();	// DibBuffer ����
}

//---------------------------
// �̱���. �����ڴ� private�̴�.
// --------------------------
CScreenDib *CScreenDib::Getinstance(int iWidth, int iHeight, int iColorBit)
{
	static CScreenDib cScreenDib(iWidth, iHeight, iColorBit);
	return &cScreenDib;
}


//---------------------------
// DibBuffer Create
// --------------------------
void CScreenDib::CreateDibBuffer(int iWidth, int iHeight, int iColorBit)
{
	/////////////////////////////////////////////////////
	// �Է¹��� ������ ������ �ɹ����� ���� ����
	////////////////////////////////////////////////////
	// pitch ���ϱ� (���� ����Ʈ���� ���� ����)
	m_iPitch = (iWidth * (iColorBit / 8)) + 3 & ~3;

	// ���� �̹��� ������ ���ϱ�
	m_iBufferSize = m_iPitch * iHeight;

	// BITMAPINFO (BITMAPINFOHEADER)�� �����Ѵ�.
	m_stDibInfo.bmiHeader.biSize = sizeof(m_stDibInfo.bmiHeader);	// ��Ʈ�� ���� ����� ������
	m_stDibInfo.bmiHeader.biWidth = iWidth;			// ���� (�ȼ�)
	m_stDibInfo.bmiHeader.biHeight = -iHeight;		// ���� (�ȼ�). BITMAPINFOHEADER ���̸� -�� ������ �����𸣰� �˾Ƽ� �������ش�. ��, ������ ��Ʈ���� ���������� ������ ���ش�.
	m_stDibInfo.bmiHeader.biPlanes = 1;				// ���� �𸣰����� ������ 1
	m_stDibInfo.bmiHeader.biBitCount = iColorBit;	// ��Ʈ ����
	m_stDibInfo.bmiHeader.biSizeImage = m_iBufferSize;	// ���� �̹��� ������. ���� ��Ʈ�� ����������� ���� ��Ʈ ���� ����� ���� �̹��� ����� �� ����.
	
	// �̹��� ����� ����Ͽ� ���ۿ� �̹��� �����Ҵ�
	m_bypBuffer = new BYTE[m_iBufferSize];
}

//---------------------------
// DibBuffer Relese
// --------------------------
void CScreenDib::ReleseDibBuffer()
{
	// �޸� ���� ~ 
	delete m_bypBuffer;
}

//---------------------------
// DC�� ���
// --------------------------
void CScreenDib::DrawBuffer(HWND hWnd, int iX, int iY)
{
	// �Է¹��� hWnd �ڵ��� DC�� �� DC�� X, Y ��ġ�� ��ũ�� ���� DIB�� ����Ѵ�.
	HDC hdc = GetDC(hWnd);
	SetStretchBltMode(hdc, COLORONCOLOR);			// �̹��� ��� �� ������� ����. COLORONCOLOR : ������ ���� ����. �÷� ��Ʈ�ʿ��� ���� ������ ����̶�� ��.
	StretchDIBits(hdc, iX, iY, m_iWidth, m_iHeight,	// ������(DC)�� x,y ��ǥ�� ��,����.
		0, 0, m_iWidth, m_iHeight,					// DIB�� x,y ��ǥ�� ��, ����. �ش� DIB�� ������ ������ ũ�⸸ŭ ���.
		m_bypBuffer, &m_stDibInfo, DIB_RGB_COLORS, SRCCOPY);	// ���� �̹����� ����Ǿ� �ִ� ������ ������, BITMAPINFO������ �Ѱܾ� ������, �츰 �ȷ�Ʈ �Ⱦ��� ����ȯ
																		// DIB_RGB_COLORS�� RGB�÷��� ������ �ȷ�Ʈ�� ������ ����, �������� ��¸��. �� �츮�� ����.

	ReleaseDC(hWnd,hdc);
}

//---------------------------
// �� ���� ���
// --------------------------
BYTE* CScreenDib::GetDibBuffer()
{
	return m_bypBuffer;
}

//---------------------------
// ���� ����
// --------------------------

int CScreenDib::GetWidth()
{
	return m_iWidth;
}
int CScreenDib::GetHeight()
{
	return m_iHeight;
}
int CScreenDib::GetPitch()
{
	return m_iPitch;
}