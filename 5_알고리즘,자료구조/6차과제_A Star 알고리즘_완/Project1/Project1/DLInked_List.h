#ifndef __D_LINKED_LIST_H__
#define __D_LINKED_LIST_H__

template <typename T>
class DLinkedList
{
	struct stListNode
	{
		T m_FindSearchNode;
		stListNode* m_pPrev;
		stListNode* m_pNext;
	};

	int m_iCount;
	stListNode* m_Head;
	stListNode* m_Tail;
	stListNode* m_pCur;
	stListNode* m_pReturnSaveNode;

public:
	// ������
	DLinkedList();

	// �˻� ��, �ش� ��� ��ȯ (���� �ƴ�)
	T Search(int x, int y);

	// ����
	void Insert(T InsertNode);

	// ���� ���� ��� 1�� ��ȯ�ϰ� ����Ʈ���� ���ܽ�Ű��.
	T GetListNode();

	// ����Ʈ ���� ��� �� ��ȯ
	int GetCount();

	// ����
	void Sort();

	// ����Ʈ�� ��� ��� ����
	void Clear();
};


// ������
template <typename T>
DLinkedList<T>::DLinkedList()
{
	// ���� ���ʳ�带 ������ ��, ���� ������ ��� ����
	m_pReturnSaveNode = new stListNode[sizeof(stListNode)];

	// ����, �� ���� 1���� ����
	m_Head = new stListNode[sizeof(stListNode)];
	m_Tail = new stListNode[sizeof(stListNode)];

	// ���� ����
	m_Head->m_pNext = m_Tail;
	m_Head->m_pPrev = nullptr;

	m_Tail->m_pNext = nullptr;
	m_Tail->m_pPrev = m_Head;

	m_iCount = 0;
}

// �˻� ��, �ش� ��� ��ȯ (���� �ƴ�)
template <typename T>
T DLinkedList<T>::Search(int x, int y)
{
	// ���ڷ� ���� x,y�� ���� ��尡 ����Ʈ ���� �ִ��� �˻�
	m_pCur = m_Head->m_pNext;

	if (m_pCur == m_Tail)
		return nullptr;

	for (int i = 0; i < m_iCount; ++i)
	{
		if (m_pCur->m_FindSearchNode->m_iX == x && m_pCur->m_FindSearchNode->m_iY == y)
			return m_pCur->m_FindSearchNode;

		m_pCur = m_pCur->m_pNext;
	}

	return nullptr;
}

// ����
template <typename T>
void DLinkedList<T>::Insert(T InsertNode)
{
	// 1. ���ڷ� ���� stFindSearchNode�� �����͸� ����Ű�� ��� ����
	stListNode* newNode =  new stListNode[sizeof(stListNode)];
	newNode->m_FindSearchNode = InsertNode;

	// 2. �Ӹ������� ����
	newNode->m_pPrev = m_Head;
	newNode->m_pNext = m_Head->m_pNext;

	m_Head->m_pNext->m_pPrev = newNode;
	m_Head->m_pNext = newNode;

	m_iCount++;

	// 2. Sort()
	Sort();
}

// ����
template <typename T>
void DLinkedList<T>::Sort()
{
	m_pCur = m_Head->m_pNext;
	stListNode* CurNext = m_pCur->m_pNext;

	// 1. �Ӹ����� ���������� ���鼭, ��������. ���� ���� �Ӹ������� ������.
	for (int i = 0; i < m_iCount; ++i)
	{
		for (int j = 0; j < m_iCount - i; ++j)
		{
			if (CurNext == m_Tail)
				break;

			// ���� ��尡, ���� ����� ���� ��庸�� ���� ũ�ٸ�, ��ġ ����
			if (m_pCur->m_FindSearchNode->m_fF > CurNext->m_FindSearchNode->m_fF)
			{
				stListNode* Temp = m_pCur;

				Temp->m_pPrev->m_pNext = Temp->m_pNext;
				Temp->m_pNext->m_pPrev = Temp->m_pPrev;

				Temp->m_pPrev = Temp->m_pNext;
				Temp->m_pNext = Temp->m_pNext->m_pNext;
				Temp->m_pNext->m_pPrev = Temp;
				Temp->m_pPrev->m_pNext = Temp;
			}

			m_pCur = CurNext;
			CurNext = m_pCur->m_pNext;
		}

		if (CurNext == m_Tail)
			break;
	}

}

// ���� ���� ��� 1�� ��ȯ�ϰ� ��带 ����Ʈ���� ���ܽ�Ű��.
template <typename T>
T DLinkedList<T>::GetListNode()
{
	stListNode* returnNode = m_Head->m_pNext;

	m_Head->m_pNext = returnNode->m_pNext;
	returnNode->m_pNext->m_pPrev = m_Head;	

	m_pReturnSaveNode->m_FindSearchNode = returnNode->m_FindSearchNode;

	delete[] returnNode;
	m_iCount--;

	return m_pReturnSaveNode->m_FindSearchNode;
}

// ����Ʈ ���� ��� �� ��ȯ
template <typename T>
int DLinkedList<T>::GetCount()
{
	return m_iCount;
}

// ����Ʈ�� ��� ��� ����
template <typename T>
void DLinkedList<T>::Clear()
{
	stListNode* Before = m_Head;
	m_pCur = Before->m_pNext;

	while (1)
	{
		if (m_pCur == m_Tail)
			break;

		// �翬��
		Before->m_pNext = m_pCur->m_pNext;
		m_pCur->m_pNext->m_pPrev = Before;

		m_iCount--;

		// ���� �Ҹ�
		delete[] m_pCur->m_FindSearchNode;
		delete m_pCur;
		
		m_pCur = Before->m_pNext;
	}
}


#endif // !__D_LINKED_LIST_H__
