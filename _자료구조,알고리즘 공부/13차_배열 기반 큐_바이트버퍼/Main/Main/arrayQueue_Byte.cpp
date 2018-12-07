#include "pch.h"
#include "arrayQueue_Byte.h"
#include <Windows.h>

// �ʱ�ȭ
void arrayQueue_Byte::Init()
{
	m_iFront = 0;
	m_iRear = 0;
	m_iSize = 0;
}

// ��ť
bool arrayQueue_Byte::Enqueue(char* Data, int Size)
{
	// ť�� ������ ������� üũ
	if (GetFreeSize() < Size)
		return false;		

	// �� ���� ��ť ������ ������ �˾ƿ���.
	int EnqSize = GetNotBrokenSize_Enqueue();

	// ��� ����� �� ���� ��ť�� ������ ���
	if (EnqSize >= Size)
	{
		// mem���� �ѹ���
		memcpy_s(&m_cBuff[m_iRear], Size, Data, Size);

		// Rear �̵�
		m_iRear += Size;
	}

	// �� ���� ��ť �Ұ����� ���
	else
	{
		int MoveSize = Size;

		// mem ���� 2ȸ
		memcpy_s(&m_cBuff[m_iRear], EnqSize, Data, EnqSize);

		MoveSize = MoveSize - EnqSize;
		memcpy_s(&m_cBuff[0], MoveSize, &Data[EnqSize], MoveSize);

		// Rear �̵�
		m_iRear = MoveSize;
	}

	return true;
}

// ��ť
bool arrayQueue_Byte::Dequeue(char* Data, int Size)
{
	// ť�� ����ִ��� üũ
	if (m_iRear == m_iFront)
		return false;	

	// �� ���� ��ť ������ ������ �˾ƿ���.
	int DeqSize = GetNotBrokenSize_Dequeue();

	// ��� ����� �� ���� ��ť ������ ���
	if(DeqSize >= Size)
	{
		// mem���� �ѹ���
		memcpy_s(Data, Size, &m_cBuff[m_iFront], Size);

		// Front �̵�
		m_iFront += Size;
	}

	// �� ���� ��ť �Ұ����� ���
	else
	{
		int MoveSize = Size;

		// mem ���� 2ȸ
		memcpy_s(Data, DeqSize, &m_cBuff[m_iFront], DeqSize);

		MoveSize = MoveSize - DeqSize;
		memcpy_s(&Data[DeqSize], MoveSize, &m_cBuff[0], MoveSize);

		// Front �̵�
		m_iFront = MoveSize;
	}

	return true;
}

// ��
bool arrayQueue_Byte::Peek(char* Data, int Size)
{

}

// ��� ���� ������
int arrayQueue_Byte::GetUseSize()
{
	// rear�� �� �ռ��ִ� ���
	if (m_iFront < m_iRear)
		return m_iRear - m_iRear;

	// front�� �� �ռ��ִ� ���	
	else if (m_iFront > m_iRear)
		return m_iRear + (BUF_SIZE - m_iFront);

	// ���� ���ٸ� �� ���. ������� ����� ����
	else
		return 0;
}

// ��� ������ ������
int arrayQueue_Byte::GetFreeSize()
{
	// rear�� �� �ռ��ִ� ���
	if (m_iFront < m_iRear)
		return m_iFront + ((BUF_SIZE - 1) - m_iRear);

	// front�� �� �ռ��ִ� ���
	else if (m_iFront > m_iRear)
		return (m_iFront - 1) - m_iRear;

	// ���� ���� ��쿡��, �� ���.
	else
		return BUF_SIZE - 1;
}

// �� ���� ��ť ������ ����(������ �ʰ�)
int arrayQueue_Byte::GetNotBrokenSize_Enqueue()
{
	// Front�� 0�ϰ��
	if (m_iFront == 0)
		return (BUF_SIZE - 1) - m_iRear;

	// rear�� �� �ռ��ְų� ���� ���
	else if (m_iFront <= m_iRear)
		return BUF_SIZE - m_iRear;

	// front�� �� �ռ��ִ� ���	
	else
		return (m_iFront - 1) - m_iRear;
}

// �� ���� ��ť ������ ����(������ �ʰ�)
int arrayQueue_Byte::GetNotBrokenSize_Dequeue()
{
	// rear�� �� �ռ��ְų� ���� ���
	if (m_iFront <= m_iRear)
		return m_iRear - m_iFront;

	// front�� �� �ռ��ִ� ���	
	else
		return BUF_SIZE - m_iFront;
}