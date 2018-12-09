#ifndef __LIST_ARRAY_H__
#define __LIST_ARRAY_H__

template <typename T>
class listStack
{
	struct Node
	{
		T m_data;
		Node* m_pNext;
	};

	Node* m_pTop;
	int m_iSize;

public:
	// �ʱ�ȭ
	void Init();

	// ����
	void Push(T data);

	// ����
	bool Pop(T *pData);

	// Peek
	bool Peek(T* pData);

	// ������� Ȯ��
	bool IsEmpty();

	// ��� ��
	int GetNodeSize();
};


// �ʱ�ȭ
template <typename T>
void listStack<T>::Init()
{
	m_pTop = nullptr;
	m_iSize = 0;
}

// ����
template <typename T>
void listStack<T>::Push(T data)
{
	// �� ��� ����
	Node* NewNode = new Node;
	NewNode->m_data = data;

	// Next ����
	if (m_pTop == nullptr)
		NewNode->m_pNext = nullptr;
	else
		NewNode->m_pNext = m_pTop;

	// Top ����
	m_pTop = NewNode;

	m_iSize++;
}

// ����
template <typename T>
bool listStack<T>::Pop(T *pData)
{
	// ��尡 ������ return false
	if (m_pTop == nullptr)
		return false;

	// ������ ��� �޾Ƶα�
	Node* DeleteNode = m_pTop;

	// Top�� Next�� �̵�
	m_pTop = m_pTop->m_pNext;

	// ������ ������ ����
	*pData = DeleteNode->m_data;

	// ��� �޸� ���� �� true ����
	delete DeleteNode;

	m_iSize--;

	return true;
}

// Peek
template <typename T>
bool listStack<T>::Peek(T* pData)
{
	// ��尡 ������ return false
	if (m_pTop == nullptr)
		return false;

	// ������ ������ ����
	*pData = m_pTop->m_data;

	return true;
}

// ������� Ȯ��
template <typename T>
bool listStack<T>::IsEmpty()
{
	if (m_pTop == nullptr)
		return true;

	return false;
}

// ��� ��
template <typename T>
int listStack<T>::GetNodeSize()
{
	return m_iSize;
}

#endif // !__LIST_ARRAY_H__
