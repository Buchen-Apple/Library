#ifndef __C_LINKED_LIST_H__
#define __C_LINKED_LIST_H__

struct Employee
{
	int m_iNumber;
	char m_cName[20];
};

class CLinkedList
{
	typedef Employee* LData;

	struct Node
	{
		LData m_data;
		Node* m_pNext;
	};

	Node* m_pTail;
	Node* m_pCur;
	Node* m_pBefore;

	int m_iNodeCount;

public:
	// �ʱ�ȭ
	void Init();

	// �Ӹ��� ����
	void Insert(LData Data);

	// ������ ����
	void Insert_Tail(LData Data);

	// ù ��� ��ȯ
	bool First(LData *pData);

	// ���� ��� ��ȯ
	bool Next(LData *pData);

	// ����
	LData Remove();

	// ��� �� ��ȯ
	int Size();
};

#endif // !__C_LINKED_LIST_H__
