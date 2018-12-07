#include "pch.h"
#include "ArrayStack.h"

// �ʱ�ȭ
void arrayStack::Init()
{
	m_iTop = -1;	// ���� ��, Top �̵� �Ŀ� �����͸� �ֱ� ������ -1���� ����.
	m_iSize = 0;
}

// ����
bool arrayStack::Push(Data data)
{
	// �迭�� �� �̻� ������ ������ return false
	if (m_iTop == SIZE - 1)
		return false;

	// ������ ������ ������ �ֱ�
	m_iTop++;
	m_stackArr[m_iTop] = data;

	m_iSize++;

	return true;
}

// ����
bool arrayStack::Pop(Data *pData)
{
	// �迭�� �����Ͱ� �ϳ��� ������ return false
	if (m_iTop == -1)
		return false;

	// �����Ͱ� ������ ������ ����
	*pData = m_stackArr[m_iTop];
	m_iTop--;

	m_iSize--;

	return true;
}

// Peek
bool  arrayStack::Peek(Data *pData)
{
	// �迭�� �����Ͱ� �ϳ��� ������ return false
	if (m_iTop = -1)
		return false;

	// �����Ͱ� ������ ������ ����
	*pData = m_stackArr[m_iTop];

	return true;
}

// ������� üũ
bool arrayStack::IsEmpty()
{
	if (m_iTop == -1)
		return true;

	return false;
}

// ������ ������ ��
int arrayStack::GetNodeSize()
{
	return m_iSize;
}