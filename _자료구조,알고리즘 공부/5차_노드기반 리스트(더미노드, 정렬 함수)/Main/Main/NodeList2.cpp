#include "pch.h"
#include "NodeList2.h"


// -----------------------------------------
// �⺻ ����
void  NodeList2::NormalInsert(LData data)
{
	// ��� ���� �� ����
	Node* NewNode = new Node;
	NewNode->m_Data = data;

	// �Ӹ��� ����
	NewNode->m_pNext = m_pHead->m_pNext;
	m_pHead->m_pNext = NewNode;

	m_iNodeCount++;
}

// ���� ����
void  NodeList2::SortInsert(LData data)
{
	// ��� ���� �� ����
	Node* NewNode = new Node;
	NewNode->m_Data = data;
	
	Node* pPred = m_pHead;

	// ������� ��� ��ȸ�ϸ� ��ġ ã�´�.
	// ���� �Լ��� �̿���, �̹� ��尡 ���� �� ��ġ�� ã�´�.
	// ���� �Լ����� 'true'�� ���ϵǸ� ���� �� ��ġ�̴�.
	while (pPred->m_pNext != nullptr && 
		CompareFunc(data, pPred->m_pNext->m_Data) != true)
	{
		// ��ã������ pPred�� ������ġ�� �̵���Ų��.
		pPred = pPred->m_pNext;
	}

	// ���ο� ���� pPred�� �����ʿ� ����ȴ�.
	NewNode->m_pNext = pPred->m_pNext;
	pPred->m_pNext = NewNode;

	m_iNodeCount++;
}
// -----------------------------------------


// �ʱ�ȭ
void NodeList2::Init()
{
	// ���� ��� �����α�
	m_pHead = new Node;
	m_pHead->m_pNext = nullptr;

	// ���� �ʱ�ȭ
	m_pCur = nullptr;
	m_pBefore = nullptr;
	CompareFunc = nullptr;
	m_iNodeCount = 0;	
}

// ����
void NodeList2::Insert(LData data)
{
	// ���� ������ ���ٸ� �⺻ ����.
	if (CompareFunc == nullptr)
		NormalInsert(data);

	// ���� ������ �ִٸ� ���� �������� ����
	else
		SortInsert(data);
}

// ù ��� ��ȯ
bool NodeList2::LFirst(LData* pData)
{
	// ��尡 ���ٸ� false ����
	if (m_pHead->m_pNext == nullptr)
		return false;

	// ��尡 ���� ���
	m_pBefore = m_pCur;
	m_pCur = m_pHead->m_pNext;
	*pData = m_pCur->m_Data;

	return true;
}

// ���� ��� ��ȯ
bool NodeList2::LNext(LData* pData)
{
	// �� ����� ���
	if (m_pCur->m_pNext == nullptr)
		return false;

	// �� ��尡 �ƴ� ���
	m_pBefore = m_pCur;
	m_pCur = m_pCur->m_pNext;
	*pData = m_pCur->m_Data;

	return true;
}

// ����
NodeList2::LData NodeList2::LRemove()
{
	// ������ ������ �޾Ƶα�
	LData ret = m_pCur->m_Data;

	// ������ ���, ����Ʈ���� �����ϱ�
	m_pBefore->m_pNext = m_pCur->m_pNext;

	// ��� ���� ��, m_pCur �̵�
	delete m_pCur;
	m_pCur = m_pBefore;

	m_iNodeCount--;

	return ret;
}

// ��� �� ��ȯ
int NodeList2::Size()
{
	return m_iNodeCount;
}

// ���� �Լ� �ޱ�
void NodeList2::SetSortRule(bool(*comp)(LData C1, LData C2))
{
	CompareFunc = comp;
}