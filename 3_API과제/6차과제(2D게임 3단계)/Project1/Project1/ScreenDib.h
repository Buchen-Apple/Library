#pragma once
#ifndef __SCREENDIB_H__
#define __SCREENDIB_H__

#include <Windows.h>

class CScreenDib
{
private:
	//---------------------------
	// �̱����� ���� �����ڸ� private�� ����.
	// --------------------------	
	CScreenDib(int iWidth, int iHeight, int iColorBit);

public:
	//---------------------------
	// �ı���
	// --------------------------	
	virtual ~CScreenDib();

	//---------------------------
	// �̱��� ���� �Լ�
	// --------------------------
	static CScreenDib* Getinstance(int iWidth, int iHeight, int iColorBit);


protected:
	//---------------------------
	// �� ����� �����Ҵ� / �����Ҵ� ���� �Լ�
	// �����ڿ� �Ҹ��ڿ��� ȣ���Ѵ�.
	// --------------------------
	void CreateDibBuffer(int iWidth, int iHeight, int iColorBit);
	void ReleseDibBuffer();

public:
	//---------------------------
	// Flip �Լ�
	// --------------------------
	void DrawBuffer(HWND hWnd, int iX = 0, int iY = 0);

	//---------------------------
	// ���� �����Լ�
	// --------------------------
	BYTE *GetDibBuffer();	// m_bypBuffer�� ��ȯ�ϴ� �Լ�
	int GetWidth();			// m_iWidth�� ��ȯ�ϴ� �Լ�
	int GetHeight();		// m_iHeight�� ��ȯ�ϴ� �Լ�
	int GetPitch();			// m_iPitch�� ��ȯ�ϴ� �Լ�

protected:
	//---------------------------
	// ��� ����
	// --------------------------
	BITMAPINFO	m_stDibInfo;	// ���� ���� BITMAPINFO
	BYTE		*m_bypBuffer;	// �� �����.
	
	int m_iWidth;				// ������� ����. �ȼ� ����.
	int m_iHeight;				// ������� ����. �ȼ�����
	int m_iPitch;				// ������� ��ġ. (����Ʈ ���� ���� ũ��)
	int m_iColorBit;			// �÷� ����. 8bit / 16bit ��..
	int m_iBufferSize;			// ���� �̹��� ������. ����Ʈ ����.
	
};

#endif // !__SCREENDIB_H__
