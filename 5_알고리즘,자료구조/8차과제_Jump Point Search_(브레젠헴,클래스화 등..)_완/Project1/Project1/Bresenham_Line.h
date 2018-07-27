#ifndef __BRESENHAM_LINE_H__
#define __BRESENHAM_LINE_H__

class CBresenhamLine
{
private:
	int m_iStartX, m_iStartY, m_iFinX, m_iFinY;
	int m_iNowX, m_iNowY;

	int m_iError;

	int m_iAddX, m_iAddY;
	int m_iWidth, m_iHeight;	

public:
	// ������
	CBresenhamLine();

	// �ʱ�ȭ
	void Init(int StartX, int StartY, int FinX, int FinY);

	// �Ҹ���
	~CBresenhamLine();

	// ���� ��ġ ������
	bool GetNext(POINT* returnP);
	
};

#endif // !__BRESENHAM_LINE_H__
