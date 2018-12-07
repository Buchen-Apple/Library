#ifndef __DOUBLE_LINKED_LIST_H__
#define __DOUBLE_LINKED_LIST_H__

class DLinkedList
{
	typedef int LData;

	struct Node
	{
		LData m_Data;
		Node* m_pNext;
		Node* m_pPrev;
	};

	Node* m_pHead;
	Node* m_pCur;
	int m_iNodeCount;

public:

	// �ʱ�ȭ
	void Init();

	// �Ӹ��� ����
	void Insert(LData data);

	// ù ��� ���
	bool First(LData* pData);

	// �� ���� ��� ���
	bool Next(LData* pData);

	// ����
	LData Remove();

	// ��� �� ��ȯ
	int Size();
};

#endif // !__DOUBLE_LINKED_LIST_H__
