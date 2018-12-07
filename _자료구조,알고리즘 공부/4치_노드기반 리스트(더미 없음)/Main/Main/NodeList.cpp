#include "pch.h"
#include "NodeList.h"
#include <iostream>

using namespace std;

// �ʱ�ȭ
void NodeList::ListInit()
{
	m_pCur = nullptr;
	m_pTail = nullptr;
	m_pHead = nullptr;
	m_pBefore = nullptr;

	m_iNodeCount = 0;
}

// �Ӹ��� ����
void NodeList::ListInsert(LData data)
{
	// ���ο� ��� ����
	Node* NewNode = new Node;
	NewNode->m_Data = data;	

	// ù ����� ���
	if (m_pHead == nullptr)
	{
		NewNode->m_pNext = nullptr;
		m_pHead = NewNode;
		m_pTail = NewNode;
	}

	// ù ��尡 �ƴ� ���
	else
	{
		// ���ο� ��尡 Next�� ����� ����Ų��.
		NewNode->m_pNext = m_pHead;

		// ���ο� ��尡 ����� ��
		m_pHead = NewNode;
	}
	m_iNodeCount++;
}

// ù ������ ��ȯ
bool NodeList::ListFirst(LData* pData)
{
	// ��尡 ���� ���
	if (m_pHead == nullptr)
		return false;

	// ��尡 ���� ���
	m_pBefore = m_pHead;
	m_pCur = m_pHead;
	*pData = m_pCur->m_Data;

	return true;
}

// �� ��° ���ĺ��� ������ ��ȯ
bool NodeList::ListNext(LData* pData)
{
	// ���� �������� ���
	if (m_pCur->m_pNext == nullptr)
		return false;

	// ���� �ƴ� ���	
	m_pBefore = m_pCur;
	m_pCur = m_pCur->m_pNext;
	*pData = m_pCur->m_Data;	

	return true;
}

// ���� Cur�� ����Ű�� ������ ����
NodeList::LData NodeList::ListRemove()
{
	// ������ ����� Next �޾Ƶα�
	Node* DeleteNext = m_pCur->m_pNext;

	// ������ ������ �޾Ƶα�
	LData ret = m_pCur->m_Data;

	// ������ ��尡 ù �����
	if (m_pCur == m_pHead)
	{
		// ����� DeleteNext�� ����Ų��.
		m_pHead = DeleteNext;	

		// ���� ��� ����
		delete m_pCur;
	}

	// ù ��尡 �ƴ϶��
	else
	{
		// Before�� Next�� DeleteNext�� ����Ų��
		m_pBefore->m_pNext = DeleteNext;

		// ���� ��� ����
		delete m_pCur;

		// m_pCur�� m_pBefore�� ����Ų��. (��ĭ �ڷ� �̵�)
		m_pCur = m_pBefore;
	}	

	// ��� �� ����
	m_iNodeCount--;

	// ����
	return ret;
}

// ���� ��� �� ��ȯ
int NodeList::ListCount()
{
	return m_iNodeCount;
}