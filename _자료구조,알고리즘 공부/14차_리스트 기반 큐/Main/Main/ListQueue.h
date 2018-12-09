#ifndef __LIST_QUEUE_H__
#define __LIST_QUEUE_H__

class listQueue
{
	typedef int Data;

	struct Node
	{
		Data m_Data;
		Node* m_pNext;
	};

	Node* m_pFront;
	Node* m_pRear;
	int m_iNodeCount;

public:
	// �ʱ�ȭ
	void Init();

	// ����
	void Enqueue(Data data);

	// ����
	bool Dequeue(Data* pData);

	// ��
	bool Peek(Data* pData);

	// ��� �� Ȯ��
	int Size();
};


#endif // !__LIST_QUEUE_H__
