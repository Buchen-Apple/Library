#ifndef __D_LINKED_LIST_H__
#define __D_LINKED_LIST_H__

template <typename T>
class DLinkedList_Dummy
{
	struct Node
	{
		T m_Data;
		Node* m_pPrev;
		Node* m_pNext;
	};

	Node* m_pHead;
	Node* m_pTail;
	Node* m_pCur;
	int m_iNodeCount;

public:
	// �ʱ�ȭ
	void Init();

	// ������ ����
	void Insert_back(T data);

	// ù ��� ��ȯ
	bool First(T *pData);

	// ���� ��� ��ȯ
	bool Next(T *pData);

	// ����
	T Remove();

	// ��� ī��Ʈ ��ȯ
	int Size();
};


// �ʱ�ȭ
template <typename T>
void DLinkedList_Dummy<T>::Init()
{
	// ����� ���̿� ���� ���̳�� ����
	m_pHead = new Node;
	m_pTail = new Node;

	// ����� ������ ���� ���θ� ����Ų��.
	m_pHead->m_pNext = m_pTail;
	m_pHead->m_pPrev = nullptr;
	
	m_pTail->m_pNext = nullptr;
	m_pTail->m_pPrev = m_pHead;

	// �� �� ���� �ʱ�ȭ
	m_pCur = nullptr;
	m_iNodeCount = 0;
}

// ������ ����
template <typename T>
void DLinkedList_Dummy<T>::Insert_back(T data)
{
	// �� ��� ����
	Node* NewNode = new Node;
	NewNode->m_Data = data;

	// ���ο� ����� Next�� Tail�� ����Ų��.
	NewNode->m_pNext = m_pTail;

	// ���ο� ����� Prev�� Tail�� Prev�� ����Ų��.
	NewNode->m_pPrev = m_pTail->m_pPrev;

	// Tail�� Prev�� Next��, ���ο� ��带 ����Ų��.
	m_pTail->m_pPrev->m_pNext = NewNode;

	// Tail�� Prev��, ���ο� ��带 ����Ų��.
	m_pTail->m_pPrev = NewNode;

	m_iNodeCount++;
}

// ù ��� ��ȯ
template <typename T>
bool DLinkedList_Dummy<T>::First(T *pData)
{
	// ��尡 ���� ���
	if (m_iNodeCount == 0)
		return false;

	// ��尡 ���� ���
	m_pCur = m_pHead->m_pNext;
	*pData = m_pCur->m_Data;

	return true;
}

// ���� ��� ��ȯ
template <typename T>
bool DLinkedList_Dummy<T>::Next(T *pData)
{
	// ���� �������� ���
	if (m_pCur->m_pNext == m_pTail)
		return false;

	// ���� ���� �ƴ� ���
	m_pCur = m_pCur->m_pNext;
	*pData = m_pCur->m_Data;

	return true;
}

// ����
template <typename T>
T DLinkedList_Dummy<T>::Remove()
{
	// ������ ������ �޾Ƶα�
	T ret = m_pCur->m_Data;

	// ������ ��� �޾Ƶα�
	Node* DeleteNode = m_pCur;

	// ������ ��带, ����Ʈ���� ����
	m_pCur->m_pPrev->m_pNext = m_pCur->m_pNext;
	m_pCur->m_pNext->m_pPrev = m_pCur->m_pPrev;

	// pCur�� Prev�� �̵�
	m_pCur = m_pCur->m_pPrev;

	// �޸� ��ȯ
	delete DeleteNode;

	m_iNodeCount--;

	return ret;
}

// ��� ī��Ʈ ��ȯ
template <typename T>
int DLinkedList_Dummy<T>::Size()
{
	return m_iNodeCount;
}

#endif // !__D_LINKED_LIST_H__
