#ifndef __NODE_LIST2_H__
#define __NODE_LIST2_H__

#include "Point.h"

class NodeList2
{
	typedef Point* LData;

	struct Node
	{
		LData m_Data;
		Node* m_pNext;
	};

	Node* m_pHead;
	Node* m_pCur;
	Node* m_pBefore;
	int m_iNodeCount;

	bool(*CompareFunc)(LData C1, LData C2);


private:
	// �⺻ ����
	void NormalInsert(LData data);

	// ���� ����
	void SortInsert(LData data);


public:
	// �ʱ�ȭ
	void Init();

	// ����
	void Insert(LData data);	

	// ù ��� ��ȯ
	bool LFirst(LData* pData);

	// ���� ��� ��ȯ
	bool LNext(LData* pData);

	// ����
	LData LRemove();

	// ��� �� ��ȯ
	int Size();

	// ���� �Լ� �ޱ�
	void SetSortRule(bool(*comp)(LData C1, LData C2));
};

#endif // !__NODE_LIST2_H__
