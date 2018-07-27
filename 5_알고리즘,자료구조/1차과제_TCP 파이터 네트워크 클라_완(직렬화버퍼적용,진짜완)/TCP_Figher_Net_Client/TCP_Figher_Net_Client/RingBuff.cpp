#include "stdafx.h"
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
	// ����� ���� ���
	// rear�� �� ũ�ٸ�
	if (m_Rear > m_Front)
		return m_Rear - m_Front;

	// ���� ���ٸ�
	else if (m_Rear == m_Front)
		return 0;

	// front�� �� ũ�ٸ�
	else
		return m_BuffSize - (m_Front - m_Rear);

}

/////////////////////////////////////////////////////////////////////////
// ���� ���ۿ� ���� �뷮 ���.
//
// Parameters: ����.
// Return: (int)�����뷮.
/////////////////////////////////////////////////////////////////////////
int	CRingBuff::GetFreeSize(void)
{
	// ���� ���� ����ϱ�
	// rear�� �� ũ�ٸ�
	if (m_Rear > m_Front)
		return (m_BuffSize - 1) - (m_Rear - m_Front);

	// ���� ���ٸ�
	else if (m_Rear == m_Front)
		return 0;

	// m_Front�� �� ũ�ٸ�
	else
		return (m_Front - 1) - m_Rear;
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

	// rear�� �� ũ�ٸ�.
	if (m_Rear > m_Front)
		return m_Rear - m_Front;

	// front�� �� ũ�ٸ�
	else
		return m_BuffSize - (m_Front +1);
}

int	CRingBuff::GetNotBrokenPutSize(void)
{
	 //�� ���� �� �� �ִ� ũ��

	// rear�� �� ũ�ų� ���ٸ�
	if (m_Rear >= m_Front)
		return m_BuffSize - (m_Rear +1);

	// front�� �� ũ�ٸ�
	else
		return m_Front - m_Rear;
}

// ť�� ���� �ε����� üũ�ϱ� ���� �Լ�
int CRingBuff::NextIndex(int iIndex, int iSize)
{
	if (iIndex + iSize >= m_BuffSize)
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
	
	// ���� Peek �� ������
	int iRealPeekSIze = 0;

	// Peek �� ����� ������ ���� iRealCpySize.
	int TempFront = m_Front;
	int iRealCpySize = 0;

	while (iSize > 0)
	{
		// ������ ������ ������ ���̻� Peek �Ұ�
		if (TempFront == m_Rear)
			break;

		// �ӽ� Front 1ĭ ������ �̵�
		TempFront = NextIndex(TempFront, 1);

		// ť�� ���� üũ��, �̹��� Peek�� �� �ִ� ����� ã�´�.
		if (TempFront + iSize > m_BuffSize)
			iRealPeekSIze = m_BuffSize - TempFront;
		else
			iRealPeekSIze = iSize;

		// �޸� ����
		memcpy(&chpDest[iRealCpySize], &m_Buff[TempFront], iRealPeekSIze);

		// �ӽ� Front�� ��ġ �̵�
		TempFront = NextIndex(TempFront, iRealPeekSIze - 1);

		iRealCpySize += iRealPeekSIze;
		iSize -= iRealPeekSIze;
	}	

	return iRealCpySize;
}


/////////////////////////////////////////////////////////////////////////
// ���ϴ� ���̸�ŭ �б���ġ ���� ���� / ���� ��ġ �̵�
//
// Parameters: ����.
// Return: ���� ���� �� �̵��� ������.
/////////////////////////////////////////////////////////////////////////
int CRingBuff::RemoveData(int iSize)
{
	// 0����� �����Ϸ��� �ϸ� ����
	if (iSize == 0)
		return 0;

	// �����Ϸ��� ũ�Ⱑ, �����Ͱ� ����ִ� ũ�⺸�� ũ�ų� ���ٸ�, ���� Ŭ����.
	if (GetUseSize() <= iSize)
	{
		m_Front = m_Rear = 0;
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

		// ������ ��ŭ ������ ��ġ �̵� 
		m_Front = NextIndex(m_Front, iRealRemoveSize - 1);

		// �̵��� ��ŭ iSize���� ����
		iSize -= iRealRemoveSize;
		iReturnSize += iRealRemoveSize;
		iRealRemoveSize = iSize;

	}

	return iReturnSize;
}

int	CRingBuff::MoveWritePos(int iSize)
{
	// 0����� �̵��Ϸ��� �ϸ� ����
	if (iSize == 0)
		return 0;

	// ���� Rear�� ��ġ�� ������ ���̸� �� �̻� �̵� �Ұ�
	if (NextIndex(m_Rear, 1) == m_Front)
		return -1;

	// �Ű������� ���� iSize�� �� ���� �̵��� �� �ִ� ũ�⸦ ������� üũ
	int iRealMoveSize;

	if (m_Rear + iSize >= m_BuffSize)
		iRealMoveSize = m_BuffSize - m_Rear;

	// ����� �ʴ´ٸ� �׳� �� �� ���
	else
		iRealMoveSize = iSize;

	// ���� �̵�
	int iReturnSize = 0;

	while (iSize > 0)
	{
		// Rear�� ��ġ�� ������ ���̿� ���������� �� �̻� �̵� �Ұ�
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