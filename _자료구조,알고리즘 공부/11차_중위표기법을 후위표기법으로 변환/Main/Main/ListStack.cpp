#include "pch.h"
#include "ListStack.h"

// �ʱ�ȭ
void listStack::Init()
{
	m_pTop = nullptr;
	m_iSize = 0;
}

// ����
void listStack::Push(Data data)
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
bool listStack::Pop(Data *pData)
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
bool listStack::Peek(Data* pData)
{
	// ��尡 ������ return false
	if (m_pTop == nullptr)
		return false;

	// ������ ������ ����
	*pData = m_pTop->m_data;

	return true;
}

// ������� Ȯ��
bool listStack::IsEmpty()
{
	if (m_pTop == nullptr)
		return true;

	return false;
}

// ��� ��
int listStack::GetNodeSize()
{
	return m_iSize;
}