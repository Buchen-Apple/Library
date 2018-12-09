#include "pch.h"
#include "ListQueue.h"

// �ʱ�ȭ
void listQueue::Init()
{
	m_pFront = nullptr;
	m_pRear = nullptr;
	m_iNodeCount = 0;
}

// ����
void listQueue::Enqueue(Data data)
{
	Node* NewNode = new Node;
	NewNode->m_Data = data;
	NewNode->m_pNext = nullptr;

	// ���� ����� ���
	if (m_pFront == nullptr)
	{
		m_pFront = NewNode;
		m_pRear = NewNode;
	}

	// ���� ��尡 �ƴ� ���
	else
	{
		m_pRear->m_pNext = NewNode;
		m_pRear = NewNode;
	}

	m_iNodeCount++;
}

// ����
bool listQueue::Dequeue(Data* pData)
{
	// ��尡 �ϳ��� ���� ���
	if (m_pFront == nullptr)
		return false;

	// ��尡 ���� ���
	Node* DeleteNode = m_pFront;
	*pData = m_pFront->m_Data;

	m_pFront = m_pFront->m_pNext;
	delete DeleteNode;

	return true;
}

// ��
bool listQueue::Peek(Data* pData)
{
	// ��尡 �ϳ��� ���� ���
	if (m_pFront == nullptr)
		return false;

	// ��尡 ���� ���
	*pData = m_pFront->m_Data;
	return true;
}

// ��� �� Ȯ��
int listQueue::Size()
{
	return m_iNodeCount;
}