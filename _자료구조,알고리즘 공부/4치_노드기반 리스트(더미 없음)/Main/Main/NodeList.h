#ifndef __NODE_LIST_H__
#define __NODE_LIST_H__

// ��� ��� ����Ʈ
class NodeList
{
	typedef int LData;

	// ���
	struct Node
	{
		LData m_Data;
		Node* m_pNext;
	};

	Node* m_pHead;
	Node* m_pTail;
	Node* m_pCur;
	Node* m_pBefore;

	int m_iNodeCount;


public:
	// �ʱ�ȭ
	void ListInit();

	// �Ӹ��� ����
	void ListInsert(LData data);
	
	// ù ������ ��ȯ
	bool ListFirst(LData* pData);

	// �� ��° ���ĺ��� ������ ��ȯ
	bool ListNext(LData* pData);

	// ���� Cur�� ����Ű�� ������ ����
	LData ListRemove();

	// ���� ��� �� ��ȯ
	int ListCount();
};

#endif // !__NODE_LIST_H__
