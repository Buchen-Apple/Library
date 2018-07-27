#include "stdafx.h"
#include "ProtocolBuff.h"
#include "ExceptionClass.h"
#include <string.h>

#define BUFF_SIZE 1024

// ������ ������ ������
CProtocolBuff::CProtocolBuff(int size)
{
	Init(size);
}

// ������ ���� ���� ������
CProtocolBuff::CProtocolBuff()
{
	Init(BUFF_SIZE);
}

// �Ҹ���
CProtocolBuff::~CProtocolBuff()
{
	delete m_pProtocolBuff;
}

// �ʱ�ȭ
void CProtocolBuff::Init(int size)
{
	m_Size = size;
	m_pProtocolBuff = new char[m_Size];
	m_Front = 0;
	m_Rear = 0;
}

// ���� ũ�� �缳��
int CProtocolBuff::ReAlloc(int size)
{
	// ���� �����͸� temp�� ������Ų �� m_pProtocolBuff ����
	char* temp = new char[m_Size];
	memcpy(temp, &m_pProtocolBuff[m_Front], m_Size - m_Front);
	delete m_pProtocolBuff;

	// ���ο� ũ��� �޸� �Ҵ�
	m_pProtocolBuff = new char[size];

	// temp�� ������Ų ���� ����
	memcpy(m_pProtocolBuff, temp, m_Size - m_Front);

	// ���� ũ�� ����
	m_Size = size;	

	// m_Rear�� m_Front�� ��ġ ����.
	// m_Rear�� 10�̾��� m_Front�� 3�̾�����, m_Rear�� 7�̵ǰ� m_Front�� 0�̵ȴ�. ��, ����� ����Ʈ�� ���̴� �����ϴ�.
	m_Rear -= m_Front;
	m_Front = 0;
}

// ���� Ŭ����
void CProtocolBuff::Clear()
{
	m_Front = m_Rear = 0;
}

// ť�� ���� �ε����� üũ�ϱ� ���� �Լ�
int CProtocolBuff::NextIndex(int iIndex, int size)
{
	if (iIndex + size > m_Size)
		return 0;

	else
		return iIndex += size;
}

// ������ �ֱ�
int CProtocolBuff::PutData(const char* pSrc, int size)	throw (CException)
{
	// ť ��á���� üũ
	if (NextIndex(m_Rear, 1) == m_Front)
		throw CException(_T("ProtocalBuff(). PutData�� ���۰� ����."));

	// �Ű����� size�� 0�̸� ����
	if (size == 0)
		return 0;

	// ���� ��ť ����� ������ ����
	int iRealCpySize;

	// rear�� ť�� ���� �����ϸ� �� �̻� ���� �Ұ���. �׳� �����Ų��.
	if (m_Rear >= m_Size)
		throw CException(_T("ProtocalBuff(). PutData�� Rear�� ������ ���� ����."));
	else
		iRealCpySize = size;

	// �޸� ����
	memcpy(&m_pProtocolBuff[m_Rear], pSrc, iRealCpySize);

	// rear�� ��ġ �̵�
	m_Rear = NextIndex(m_Rear, iRealCpySize);

	return iRealCpySize;

}

// ������ ����
int CProtocolBuff::GetData(char* pSrc, int size)
{
	// ť ����� üũ
	if (m_Front == m_Rear)
		throw CException(_T("ProtocalBuff(). GetData�� ť�� �������."));

	// �Ű����� size�� 0�̸� ����
	if (size == 0)
		return 0;

	// ���� ��ť ����� ������ ����
	int iRealCpySize;

	// front�� ť�� ���� �����ϸ� �� �̻� �б� �Ұ���. �׳� �����Ų��.
	if (m_Front >= m_Size)
		throw CException(_T("ProtocalBuff(). GetData�� front�� ������ ���� ����."));
	else
		iRealCpySize = size;

	if (iRealCpySize == 0)
		iRealCpySize = 1;

	// �޸� ����
	memcpy(pSrc, &m_pProtocolBuff[m_Front], iRealCpySize);

	// ��ť�� ��ŭ m_Front�̵�
	m_Front = NextIndex(m_Front, iRealCpySize);

	return iRealCpySize;
}

// ������ ������ ����.
char* CProtocolBuff::GetBufferPtr(void)
{
	return m_pProtocolBuff;
}

// Rear �����̱�
int CProtocolBuff::MoveWritePos(int size)
{
	// 0����� �̵��Ϸ��� �ϸ� ����
	if (size == 0)
		return 0;

	// ���� Rear�� ��ġ�� ������ ���̸� �� �̻� �̵� �Ұ�
	if (NextIndex(m_Rear, 1) == m_Front)
		return -1;

	// �Ű������� ���� iSize�� �� ���� �̵��� �� �ִ� ũ�⸦ ������� üũ
	int iRealMoveSize;

	if (m_Rear + size > m_Size)
		iRealMoveSize = m_Size - m_Rear;

	// ����� �ʴ´ٸ� �׳� �� �� ���
	else
		iRealMoveSize = size;

	if (iRealMoveSize == 0)
		iRealMoveSize = 1;

	// ���� �̵�
	int iReturnSize = 0;

	while (size > 0)
	{
		// Rear�� ��ġ�� ������ ���̿� ���������� �� �̻� �̵� �Ұ�
		if (NextIndex(m_Rear, 1) == m_Front)
			break;

		// ������ 1ĭ �̵�
		m_Rear = NextIndex(m_Rear, 1);

		// ������ ��ġ �̵� 
		m_Rear = NextIndex(m_Rear, iRealMoveSize - 1);

		// �̵��� ��ŭ iSize���� ����
		size -= iRealMoveSize;
		iReturnSize += iRealMoveSize;

		// ���� �̵� �� �ݿ�
		if (m_Rear + size >= m_Front - 1)
			iRealMoveSize = (m_Front - 1) - m_Rear;
		else
			iRealMoveSize = size;
	}

	return iReturnSize;
}

// Front �����̱�
int CProtocolBuff::MoveReadPos(int size)
{
	// 0����� �����Ϸ��� �ϸ� ����
	if (size == 0)
		return 0;

	// �����Ϸ��� ũ�Ⱑ, �����Ͱ� ����ִ� ũ�⺸�� ũ�ų� ���ٸ�, ���� Ŭ����.
	if (GetUseSize() <= size)
	{
		Clear();
		return -1;
	}

	// ���۰� �� ���¸� 0 ����
	if (m_Front == m_Rear)
		return 0;

	// �ѹ��� ������ �� �ִ� ũ�⸦ ������� üũ
	int iRealRemoveSize;

	if (m_Front + size > m_Size)
		iRealRemoveSize = m_Size - m_Front;

	// ����� �ʴ´ٸ� �׳� �� ũ�� ���
	else
		iRealRemoveSize = size;

	// ���� ����
	int iReturnSize = 0;

	while (size > 0)
	{
		// �����ϴٰ� ���۰� �� ��������� �� �̻� ���� �Ұ�
		if (m_Front == m_Rear)
			break;

		// ������ 1ĭ �̵�
		m_Front = NextIndex(m_Front, 1);

		// ������ ��ŭ ������ ��ġ �̵� 
		m_Front = NextIndex(m_Front, iRealRemoveSize - 1);

		// �̵��� ��ŭ iSize���� ����
		size -= iRealRemoveSize;
		iReturnSize += iRealRemoveSize;
		iRealRemoveSize = size;

	}

	return iReturnSize;
}

// ���� ������� �뷮 ���.
int	CProtocolBuff::GetUseSize(void)
{
	return m_Size - GetFreeSize();
}

// ���� ���ۿ� ���� �뷮 ���.
int	CProtocolBuff::GetFreeSize(void)
{
	int iSize;

	// ���� ���� ����ϱ�
	if (m_Rear >= m_Front)
		iSize = m_Size - (m_Rear - m_Front);
	else
		iSize = (m_Front - 1) - m_Rear;

	return iSize;
}