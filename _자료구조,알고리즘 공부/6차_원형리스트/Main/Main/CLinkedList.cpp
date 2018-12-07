#include "pch.h"
#include "CLinkedList.h"

// �ʱ�ȭ
void CLinkedList::Init()
{
	m_pTail = nullptr;
	m_pCur = nullptr;
	m_pBefore = nullptr;

	m_iNodeCount = 0;
}

// �Ӹ��� ����
void CLinkedList::Insert(LData Data)
{
	// ���ο� ��� ����
	Node* NewNode = new Node;
	NewNode->m_data = Data;

	// ù ����� ���
	if (m_pTail == nullptr)
	{
		// ������ ���ο� ��带 ����Ŵ
		m_pTail = NewNode;

		// ������ Next�� ���ο� ��带 ����Ŵ
		m_pTail->m_pNext = NewNode;
	}

	// ù ��尡 �ƴ� ���
	else
	{
		// ���ο� ����� Next�� ������ Next�� ����Ŵ
		NewNode->m_pNext = m_pTail->m_pNext;

		// ������ Next�� ���ο� ��带 ����Ŵ
		m_pTail->m_pNext = NewNode;
	}

	m_iNodeCount++;
}

// ������ ����
void CLinkedList::Insert_Tail(LData Data)
{
	// ���ο� ��� ����
	Node* NewNode = new Node;
	NewNode->m_data = Data;

	// ù ����� ���
	if (m_pTail == nullptr)
	{
		// ������ ���ο� ��带 ����Ŵ
		m_pTail = NewNode;

		// ������ Next�� ���ο� ��带 ����Ŵ
		m_pTail->m_pNext = NewNode;
	}

	// ù ��尡 �ƴ� ���
	else
	{
		// ���ο� ����� Next�� ������ Next�� ����Ŵ
		NewNode->m_pNext = m_pTail->m_pNext;

		// ������ Next�� ���ο� ��带 ����Ŵ
		m_pTail->m_pNext = NewNode;

		// ������ ���ο� ��带 ����Ŵ
		m_pTail = NewNode;
	}

	m_iNodeCount++;
}

// ù ��� ��ȯ
bool CLinkedList::First(LData *pData)
{
	// ��尡 ������ return false
	if (m_pTail == nullptr)
		return false;

	// ��尡 ���� ���, ������� ��ȸ�Ѵ� (����� Tail�� Next�̴�)
	m_pBefore = m_pTail;
	m_pCur = m_pTail->m_pNext;

	*pData = m_pCur->m_data;

	return true;
}

// ���� ��� ��ȯ
bool CLinkedList::Next(LData *pData)
{
	// ��尡 ������ return false
	if (m_pTail == nullptr)
		return false;

	// ��尡 ���� ��� ������ ��ȯ�ϰԵȴ�.
	m_pBefore = m_pCur;
	m_pCur = m_pCur->m_pNext;

	*pData = m_pCur->m_data;

	return true;	
}

// ����
CLinkedList::LData CLinkedList::Remove()
{
	// ������ ������ �޾Ƶα�
	LData ret = m_pCur->m_data;

	// ������ ��� �޾Ƶα�
	Node* DeleteNode = m_pCur;

	// ������ ��尡 tail�� ���
	if (DeleteNode == m_pTail)
	{
		// ����, �����ִ� ��尡 1�����, Tail�� nullptr�� ����Ų��.
		if (m_iNodeCount == 1)
			m_pTail = nullptr;

		// �����ִ� ��尡 1�� �̻��̶��, Tail�� Before�� �̵���Ų��.
		else
			m_pTail = m_pBefore;
	}

	// ������ ���, ����Ʈ���� ����
	m_pBefore->m_pNext = m_pCur->m_pNext;

	// m_pCur��ġ �̵� �� ��� �޸𸮹�ȯ.
	m_pCur = m_pBefore;
	delete DeleteNode;

	m_iNodeCount--;

	return ret;
}

// ��� �� ��ȯ
int CLinkedList::Size()
{
	return m_iNodeCount;
}