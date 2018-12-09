#include "pch.h"
#include "Deque.h"

// �ʱ�ȭ
void Deque::Init()
{
	m_pHead = nullptr;
	m_pTail = nullptr;
	m_iSize = 0;
}

// ������ ����
void Deque::Enqueue_Head(Data data)
{
	// ���ο� ��� ����
	Node* NewNode = new Node;
	NewNode->m_Data = data;
	NewNode->m_pPrev = nullptr;

	// ���� ����� ���
	if (m_pHead == nullptr)
	{		
		NewNode->m_pNext = nullptr;
		m_pHead = NewNode;
		m_pTail = NewNode;
	}

	// ���� ��尡 �ƴ� ���
	else
	{
		NewNode->m_pNext = m_pHead;
		m_pHead->m_pPrev = NewNode;
		m_pHead = NewNode;
	}

	m_iSize++;
}

// ���� ������ ����
bool Deque::Dequeue_Head(Data* pData)
{
	// �����Ͱ� ���� ���
	if (m_pHead == nullptr)
		return false;

	// �����Ͱ� ���� ���
	Node* DeleteNode = m_pHead;
	*pData = m_pHead->m_Data;

	// ������ �����
	if (m_pHead->m_pNext == nullptr)
	{
		m_pHead = nullptr;
		m_pTail = nullptr;
	}
	
	// ������ ��尡 �ƴ϶��
	else
	{
		m_pHead = m_pHead->m_pNext;
		m_pHead->m_pPrev = nullptr;
	}
	
	delete DeleteNode;

	m_iSize--;

	return true;
}

// �ڷ� ����
void Deque::Enqueue_Tail(Data data)
{
	// ���ο� ��� ����
	Node* NewNode = new Node;
	NewNode->m_Data = data;
	NewNode->m_pNext = nullptr;	

	// ���� ����� ���
	if (m_pTail == nullptr)
	{
		NewNode->m_pPrev = nullptr;
		m_pHead = NewNode;
		m_pTail = NewNode;
	}

	// ���� ��尡 �ƴ� ���
	else
	{
		NewNode->m_pPrev = m_pTail;
		m_pTail->m_pNext = NewNode;
		m_pTail = NewNode;
	}

	m_iSize++;

}

// ���� ������ ����
bool Deque::Dequeue_Tail(Data* pData)
{
	// �����Ͱ� ���� ���
	if (m_pTail == nullptr)
		return false;

	// �����Ͱ� ���� ���
	Node* DeleteNode = m_pTail;
	*pData = m_pTail->m_Data;

	// ������ ����� ���
	if (m_pTail->m_pPrev == nullptr)
	{
		m_pHead = nullptr;
		m_pTail = nullptr;
	}

	// ������ ��尡 �ƴ� ���
	else
	{
		m_pTail = m_pTail->m_pPrev;
		m_pTail->m_pNext = nullptr;
	}

	delete DeleteNode;

	m_iSize--;

	return true;
}

// ���� ������ ����
bool Deque::Peek_Head(Data* pData)
{
	// �����Ͱ� ���� ���
	if (m_pHead == nullptr)
		return false;

	// �����Ͱ� ���� ���
	*pData = m_pHead->m_Data;	

	return true;
}

// ���� ������ ����
bool Deque::Peek_Tail(Data* pData)
{
	// �����Ͱ� ���� ���
	if (m_pTail == nullptr)
		return false;

	// �����Ͱ� ���� ���
	*pData = m_pTail->m_Data;	

	return true;
}

// ���� ������
int Deque::Size()
{
	return m_iSize;
}