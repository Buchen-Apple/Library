#include "stdafx.h"
#include "Bresenham_Line.h"

// ������
CBresenhamLine::CBresenhamLine()
{	
	// �ϴ°� ����
}

// �ʱ�ȭ
void CBresenhamLine::Init(int StartX, int StartY, int FinX, int FinY)
{
	m_iStartX = StartX;
	m_iStartY = StartY;

	m_iFinX = FinX;
	m_iFinY = FinY;

	m_iNowX = StartX;
	m_iNowY = StartY;

	m_iError = 0;

	m_iWidth = FinX - StartX;
	m_iHeight = FinY - StartY;

	// AddX, AddY ����
	if (m_iWidth < 0)
	{
		m_iWidth = -m_iWidth;
		m_iAddX = -1;
	}
	else
		m_iAddX = 1;

	if (m_iHeight < 0)
	{
		m_iHeight = -m_iHeight;
		m_iAddY = -1;
	}
	else
		m_iAddY = 1;
}

// �Ҹ���
CBresenhamLine::~CBresenhamLine()
{}

// ���� ��ġ ������
bool CBresenhamLine::GetNext(POINT* returnP)
{
	// �������� �����ϸ� �� �̻� ���� �Ұ�.
	if (m_iNowX == m_iFinX && m_iNowY == m_iFinY)
		return false;

	// ���̰� ���̺���, ũ�ų� ���� ���
	if (m_iWidth >= m_iHeight)
	{
		m_iNowX += m_iAddX;
		m_iError += m_iHeight * 2;

		if (m_iError >= m_iWidth)
		{
			m_iNowY += m_iAddY;
			m_iError -= m_iWidth * 2;
		}
	}

	// ���̰� ���̺���, Ŭ ���
	else
	{
		m_iNowY += m_iAddY;
		m_iError += m_iWidth * 2;

		if (m_iError >= m_iHeight)
		{
			m_iNowX += m_iAddX;
			m_iError -= m_iHeight * 2;
		}		
	}

	returnP->x = m_iNowX;
	returnP->y = m_iNowY;

	return true;
}