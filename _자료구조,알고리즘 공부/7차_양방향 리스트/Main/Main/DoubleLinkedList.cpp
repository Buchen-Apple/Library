#include "pch.h"
#include "DoubleLinkedList.h"

// �ʱ�ȭ
void DLinkedList::Init()
{
	m_pHead = nullptr;
	m_pCur = nullptr;
	m_iNodeCount = 0;
}

// �Ӹ��� ����
void DLinkedList::Insert(LData data)
{
	// ���ο� ��� ����
	Node* NewNode = new Node;
	NewNode->m_Data = data;
	NewNode->m_pPrev = nullptr;
	

	// ���� ����� ���
	if (m_pHead == nullptr)
	{
		// ���ο� ����� Next�� nullptr�� �ȴ�
		NewNode->m_pNext = nullptr;			
	}

	// ���� ��尡 �ƴ� ���
	else
	{
		// ���ο� ����� Next�� ����� ����Ų��.
		NewNode->m_pNext = m_pHead;

		// ����� Prev�� ���ο� ��带 ����Ų��.
		m_pHead->m_pPrev = NewNode;
	}

	// ���ο� ��尡 ����� �ȴ�.
	m_pHead = NewNode;

	// ��� ī��Ʈ ����
	m_iNodeCount++;
}

// ù ��� ���
bool DLinkedList::First(LData* pData)
{
	// ��尡 ���� ���
	if (m_pHead == nullptr)
		return false;

	// ��尡 ���� ���
	m_pCur = m_pHead;
	*pData = m_pCur->m_Data;

	return true;
}

// �� ���� ��� ���
bool DLinkedList::Next(LData* pData)
{
	// ���� ������ ���
	if (m_pCur->m_pNext == nullptr)
		return false;

	// ���� �ƴ� ���
	m_pCur = m_pCur->m_pNext;
	*pData = m_pCur->m_Data;

	return true;
}

// ����
DLinkedList::LData DLinkedList::Remove()
{
	// ������ ������ ���α�
	LData ret = m_pCur->m_Data;

	// ������ ��� ���α�
	Node* DeleteNode = m_pCur;

	// �����ϰ��� �ϴ� ��尡 Head�� ���
	if (DeleteNode == m_pHead)
	{
		// ��尡 1�����
		if (m_iNodeCount == 1)
		{
			// head�� null�̵ȴ�.
			m_pHead = nullptr;		
		}

		// 1���� �ƴ϶��
		else
		{
			// Head�� Head->Next�� �̵�
			m_pHead = m_pHead->m_pNext;

			// Head�� Prev�� nullptr��
			m_pHead->m_pPrev = nullptr;

			// m_pCur�� Next�� �̵�
			m_pCur = m_pCur->m_pNext;
		}		
	}

	// �����ϰ��� �ϴ� ��尡 Head�� �ƴ� ���
	else
	{
		// ������ ��带, ����Ʈ���� ����
		m_pCur->m_pPrev->m_pNext = m_pCur->m_pNext;

		// pCur�� Next�� null�� �ƴ� ��쿡�� �Ʒ� �۾� ����
		// ���� ������ ��� ���� ��, �Ʒ� �۾��� ���Ұ��̴�.
		if(m_pCur->m_pNext != nullptr)
			m_pCur->m_pNext->m_pPrev = m_pCur->m_pPrev;

		// m_pCur�� Prev�� �̵�
		m_pCur = m_pCur->m_pPrev;
	}

	// ��� ��ȯ
	delete DeleteNode;

	// ��� ī��Ʈ ����
	m_iNodeCount--;

	return ret;
}

// ��� �� ��ȯ
int DLinkedList::Size()
{
	return m_iNodeCount;
}