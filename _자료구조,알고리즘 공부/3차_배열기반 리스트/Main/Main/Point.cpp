#include "pch.h"
#include "Point.h"

using namespace std;

// X,Y �� ����
//
// Parameter : ������ x, y ��
void Point::SetPointPos(int x, int y)
{
	m_iXpos = x;
	m_iYpos = y;
}

// X,Y �� ���
void Point::ShowPointPos()
{
	cout << "[" << m_iXpos << "," << m_iYpos << "]" << endl;
}

// �� Point���� ��
// 
// return 0 : ���� x,y�� ������ ����
// return 1 : x�� ����
// return 2 : y�� ����
// return -1 : ��� �ٸ�
int Point::Comp(Point* Comp)
{
	if (m_iXpos == Comp->m_iXpos &&
		m_iYpos == Comp->m_iYpos)
	{
		return 0;
	}

	else if (m_iXpos == Comp->m_iXpos)
	{
		return 1;
	}

	else if (m_iYpos == Comp->m_iYpos)
	{
		return 1;
	}

	else
	{
		return -1;
	}
}