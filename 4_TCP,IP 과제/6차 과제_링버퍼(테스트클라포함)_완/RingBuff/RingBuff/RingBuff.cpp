#include "RingBuff.h"
#include <string.h>

// ������ ���� ���� ������ 1
CRingBuff::CRingBuff(void)
{
	Initial(BUF_SIZE);
}

// ������ ������ ������ 2
CRingBuff::CRingBuff(int iBufferSize)
{
	Initial(iBufferSize);
}

// ������ ������
void CRingBuff::Resize(int size)
{
	delete m_Buff;
	Initial(size);
}

// ���� �ʱ�ȭ (ť �غ�)
void CRingBuff::Initial(int iBufferSize)
{
	m_BuffSize = iBufferSize;
	m_Buff = new char[m_BuffSize];
	m_Front = 0;
	m_Rear = 0;
}

// ������ ������ ���.
int	CRingBuff::GetBufferSize(void)
{
	return m_BuffSize;
}

/////////////////////////////////////////////////////////////////////////
// ���� ������� �뷮 ���.
//
// Parameters: ����.
// Return: (int)������� �뷮.
/////////////////////////////////////////////////////////////////////////
int	CRingBuff::GetUseSize(void)
{
	return m_BuffSize - GetFreeSize();
}

/////////////////////////////////////////////////////////////////////////
// ���� ���ۿ� ���� �뷮 ���.
//
// Parameters: ����.
// Return: (int)�����뷮.
/////////////////////////////////////////////////////////////////////////
int	CRingBuff::GetFreeSize(void)
{
	int iSize;

	// ���� ���� ����ϱ�
	if (m_Rear >= m_Front)
		iSize = m_BuffSize - (m_Rear - m_Front);
	else
		iSize = m_Front - m_Rear;

	return iSize;
}

/////////////////////////////////////////////////////////////////////////
// ���� �����ͷ� �ܺο��� �ѹ濡 �а�, �� �� �ִ� ����.
// (������ ���� ����)
//
// Parameters: ����.
// Return: (int)��밡�� �뷮.
////////////////////////////////////////////////////////////////////////
int	CRingBuff::GetNotBrokenGetSize(void)
{
	// �� ���� ���� �� �ִ� ũ��

	// m_Rear�� m_Front�� ������, �� ������̴� ������ ����.
	if (m_Rear == m_Front)
		return 0;

	// ����� ���� m_Rear�� ��ġ�� ����� ���Ѵ�.
	// m_Rear�� m_Front���� ũ�ٸ� m_Rear�� m_Front���� >> ���ʿ� �ִٴ� ��
	// �� ����, Front���� Rear���� �� ���� ���� �� �ִ�.
	else if (m_Rear > m_Front)
		return m_Rear - m_Front;

	// �ݴ�� m_Front�� m_Rear���� ũ�ٸ� m_Front�� m_Rear���� >> ���ʿ� �ִٴ� ��
	// �� ����, Front���� �迭�� �������� �� ���� ���� �� �ִ�.
	else
		return m_BuffSize - m_Front;
}

int	CRingBuff::GetNotBrokenPutSize(void)
{
	// �� ���� �� �� �ִ� ũ��

	// m_Rear + 1�� m_Front�� ���ٸ� �� �� �����̴� �� �̻� �� �� ����.
	if (NextIndex(m_Rear, 1) == m_Front)
		return 0;

	// ����� ���� m_Rear�� ��ġ�� ����� ���Ѵ�.
	// m_Front�� m_Rear���� ũ�ٸ� m_Front�� m_Rear���� >> ���ʿ� �ִٴ� ��
	// �� ����, m_Rear���� m_Front-1���� �� �� �ִ�. (m_Rear�� m_Front�� �������� �� �� ���°� �Ǵϱ�!)
	else if (m_Front > m_Rear)
		return (m_Front - 1) - m_Rear;

	// m_Rear�� m_Front���� ũ�ٸ� m_Rear�� m_Front���� >> ���ʿ� �ִٴ� ��
	// �� ����, �迭�� �� ������ �� �� �ִ�.
	else
		return m_BuffSize - m_Rear;
}

// ť�� ���� �ε����� üũ�ϱ� ���� �Լ�
int CRingBuff::NextIndex(int iIndex, int iSize)
{
	if (iIndex + iSize == m_BuffSize)
		return 0;

	else
		return iIndex += iSize;
}

/////////////////////////////////////////////////////////////////////////
// WritePos �� ����Ÿ ����.
//
// Parameters: (char *)����Ÿ ������. (int)ũ��. 
// Return: (int)���� ũ��.
/////////////////////////////////////////////////////////////////////////
int	CRingBuff::Enqueue(char *chpData, int iSize)
{
	// ť ��á���� üũ
	if (NextIndex(m_Rear, 1) == m_Front)
		return -1;

	// �Ű����� size�� 0�̸� ����
	if (iSize == 0)
		return 0;

	// ���� ��ť ����� ������ ����
	int iRealCpySize;

	// �ϴ� Rear 1ĭ �̵�
	m_Rear = NextIndex(m_Rear, 1);

	// ť�� �� �ε����� üũ��, ���� ť�� ���� �� �ִ� ����� ã�´�.
	if (m_Rear + iSize > m_BuffSize)
		iRealCpySize = m_BuffSize - m_Rear;
	else
		iRealCpySize = iSize;

	// �޸� ����
	memcpy(&m_Buff[m_Rear], chpData, iRealCpySize);

	// rear�� ��ġ �̵�
	m_Rear = NextIndex(m_Rear, iRealCpySize - 1);

	return iRealCpySize;
}

/////////////////////////////////////////////////////////////////////////
// ReadPos ���� ����Ÿ ������. ReadPos �̵�.
//
// Parameters: (char *)����Ÿ ������. (int)ũ��.
// Return: (int)������ ũ��.
/////////////////////////////////////////////////////////////////////////
int	CRingBuff::Dequeue(char *chpDest, int iSize)
{
	// ť ����� üũ
	if (m_Front == m_Rear)
		return -1;

	// �Ű����� size�� 0�̸� ����
	if (iSize == 0)
		return 0;

	// ���� ��ť ����� ������ ����
	int iRealCpySize;

	// �ϴ� Front 1ĭ �̵�
	m_Front = NextIndex(m_Front, 1);

	// ť�� ���� üũ��, ���� ��ť�� �� �ִ� ����� ã�´�.
	if (m_Front + iSize > m_BuffSize)
		iRealCpySize = m_BuffSize - m_Front;
	else
		iRealCpySize = iSize;

	// �޸� ����
	memcpy(chpDest, &m_Buff[m_Front], iRealCpySize);

	// ��ť�� ��ŭ m_Front�̵�
	m_Front = NextIndex(m_Front, iRealCpySize - 1);

	return iRealCpySize;
}

/////////////////////////////////////////////////////////////////////////
// ReadPos ���� ����Ÿ �о��. ReadPos ����. (Front)
//
// Parameters: (char *)����Ÿ ������. (int)ũ��.
// Return: (int)������ ũ��.
/////////////////////////////////////////////////////////////////////////
int	CRingBuff::Peek(char *chpDest, int iSize)
{
	// ť ����� üũ
	if (m_Front == m_Rear)
		return -1;

	// �Ű����� size�� 0�̸� ����
	if (iSize == 0)
		return 0;

	// ���� ��ť ����� ������ ����
	int iRealCpySize;

	// Peek�� ����� �ӽ� Front 1ĭ �̵�
	int TempFront = NextIndex(m_Front, 1);

	// ť�� ���� üũ��, ���� ��ť�� �� �ִ� ����� ã�´�.
	if (TempFront + iSize > m_BuffSize)
		iRealCpySize = m_BuffSize - TempFront;
	else
		iRealCpySize = iSize;

	// �޸� ����
	memcpy(chpDest, &m_Buff[TempFront], iRealCpySize);

	return iRealCpySize;
}


/////////////////////////////////////////////////////////////////////////
// ���ϴ� ���̸�ŭ �б���ġ ���� ���� / ���� ��ġ �̵�
//
// Parameters: ����.
// Return: ����.
/////////////////////////////////////////////////////////////////////////
int CRingBuff::RemoveData(int iSize)
{
	// 0����� �����Ϸ��� �ϸ� ����
	if (iSize == 0)
		return 0;

	// �����Ϸ��� ũ�Ⱑ, �����Ͱ� ����ִ� ũ�⺸�� ũ�ų� ���ٸ�, ���� Ŭ����.
	if (GetUseSize() <= iSize)
	{
		ClearBuffer();
		return -1;
	}

	// ���۰� �� ���¸� 0 ����
	if (m_Front == m_Rear)
		return 0;

	// �ѹ��� ������ �� �ִ� ũ�⸦ ������� üũ
	int iRealRemoveSize;

	if (m_Front + iSize > m_BuffSize)
		iRealRemoveSize = m_BuffSize - m_Front;

	// ����� �ʴ´ٸ� �׳� �� ũ�� ���
	else
		iRealRemoveSize = iSize;

	// ���� ����
	int iReturnSize = 0;

	while (iSize > 0)
	{
		// �����ϴٰ� ���۰� �� ��������� �� �̻� ���� �Ұ�
		if (m_Front == m_Rear)
			break;

		// ������ 1ĭ �̵�
		m_Front = NextIndex(m_Front, 1);

		// ������ ��ġ �̵� 
		m_Front = NextIndex(m_Front, iRealRemoveSize - 1);

		// �̵��� ��ŭ iSize���� ����
		iSize -= iRealRemoveSize;
		iReturnSize += iRealRemoveSize;

		// ���� �̵� �� �ݿ�
		if (m_Front + iSize >= m_Rear)
			iRealRemoveSize = m_Rear - m_Front;
		else
			iRealRemoveSize = iSize;

	}

	return iReturnSize;
}

int	CRingBuff::MoveWritePos(int iSize)
{
	// 0����� �̵��Ϸ��� �ϸ� ����
	if (iSize == 0)
		return 0;

	// ������ ���� ���������� �� �̻� �̵� �Ұ�
	if (NextIndex(m_Rear, 1) == m_Front)
		return -1;

	// �� ���� �̵��� �� �ִ� ũ�⸦ ������� üũ
	int iRealMoveSize;

	if (m_Rear + iSize > m_BuffSize)
		iRealMoveSize = m_BuffSize - m_Rear;

	// ����� �ʴ´ٸ� �׳� �� �� ���
	else
		iRealMoveSize = iSize;

	// ���� �̵�
	int iReturnSize = 0;

	while (iSize > 0)
	{
		// ������ ���� ���������� �� �̻� �̵� �Ұ�
		if (NextIndex(m_Rear, 1) == m_Front)
			break;

		// ������ 1ĭ �̵�
		m_Rear = NextIndex(m_Rear, 1);

		// ������ ��ġ �̵� 
		m_Rear = NextIndex(m_Rear, iRealMoveSize - 1);

		// �̵��� ��ŭ iSize���� ����
		iSize -= iRealMoveSize;
		iReturnSize += iRealMoveSize;

		// ���� �̵� �� �ݿ�
		if (m_Rear + iSize >= m_Front - 1)
			iRealMoveSize = (m_Front - 1) - m_Rear;
		else
			iRealMoveSize = iSize;
	}

	return iReturnSize;

}

/////////////////////////////////////////////////////////////////////////
// ������ ��� ����Ÿ ����.
//
// Parameters: ����.
// Return: ����.
/////////////////////////////////////////////////////////////////////////
void CRingBuff::ClearBuffer(void)
{
	m_Front = m_Rear = 0;
}

/////////////////////////////////////////////////////////////////////////
// ������ ������ ����.
//
// Parameters: ����.
// Return: (char *) ���� ������.
/////////////////////////////////////////////////////////////////////////
char* CRingBuff::GetBufferPtr(void)
{
	return m_Buff;
}

/////////////////////////////////////////////////////////////////////////
// ������ ReadPos ������ ����.
//
// Parameters: ����.
// Return: (char *) ���� ������.
/////////////////////////////////////////////////////////////////////////
char* CRingBuff::GetReadBufferPtr(void)
{
	return (char*)&m_Front;
}

/////////////////////////////////////////////////////////////////////////
// ������ WritePos ������ ����.
//
// Parameters: ����.
// Return: (char *) ���� ������.
/////////////////////////////////////////////////////////////////////////
char* CRingBuff::GetWriteBufferPtr(void)
{
	return (char*)&m_Rear;
}