#ifndef __POINT_H__
#define __POINT_H__

class Point
{
	int m_iXpos;
	int m_iYpos;

public:
	// X,Y �� ����
	//
	// Parameter : ������ x, y ��
	void SetPointPos(int x, int y);

	// X,Y �� ���
	void ShowPointPos();

	// �� Point���� ��
	// 
	// Parameter : ���� Point*
	// return 0 : ���� x,y�� ������ ����
	// return 1 : x�� ����
	// return 2 : y�� ����
	// return -1 : ��� �ٸ�
	int Comp(Point* Comp);
};

#endif // !__POINT_H__
