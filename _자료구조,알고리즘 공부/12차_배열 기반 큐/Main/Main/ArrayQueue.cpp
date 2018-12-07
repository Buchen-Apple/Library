#include "pch.h"
#include "ArrayQueue.h"

// ���� ��ġ Ȯ��
int arrayQueue::NextPos(int Pos)
{
	// ť ��ġ�� ���̶�� 0���ε��� ��ȯ
	if (Pos == ARRAY_SIZE - 1)
		return 0;

	return Pos + 1;
}

// �ʱ�ȭ
void arrayQueue::Init()
{
	m_iFront = 0;
	m_iRear = 0;
	m_iSize = 0;
}

// ��ť
bool arrayQueue::Enqueue(Data data)
{
	// ť�� �� á�� Ȯ��
	int TempRear = NextPos(m_iRear);
	if (TempRear == m_iFront)
		return false;

	// ť�� ������ ���� ���, ���� rear �ڸ��� ���� �����δ�.
	m_Array[m_iRear] = data;
	m_iRear = TempRear;

	return true;
}

// ��ť
bool arrayQueue::Dequeue(Data *pData)
{
	// ť�� ����� Ȯ��.
	if (m_iRear == m_iFront)
		return false;

	// ť�� �����Ͱ� ���� ���, ���� Front�ڸ��� �ִ� �����͸� �а� �����δ�.
	*pData = m_Array[m_iFront];
	m_iFront = NextPos(m_iFront);
	
	return true;
}

// Peek
bool arrayQueue::Peek(Data *pData)
{
	// ť�� ����� Ȯ��.
	if (m_iRear == m_iFront)
		return false;

	// ť�� �����Ͱ� ���� ���, ���� Front�ڸ��� �ִ� �����͸� �д´�.
	*pData = m_Array[m_iFront];
	return true;
}

// ������
int arrayQueue::GetNodeSize()
{
	return m_iSize;
}