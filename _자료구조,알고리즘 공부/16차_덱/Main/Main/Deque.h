#ifndef __DEQUE_H__
#define __DEQUE_H__

class Deque
{
	typedef int Data;

	struct Node
	{
		Data m_Data;
		Node* m_pNext;
		Node* m_pPrev;
	};

	Node* m_pHead;
	Node* m_pTail;
	int m_iSize;

public:
	// �ʱ�ȭ
	void Init();

	// ������ ����
	void Enqueue_Head(Data data);

	// �տ� ������ ����
	bool Dequeue_Head(Data* pData);

	// �ڷ� ����
	void Enqueue_Tail(Data data);

	// ���� ������ ����
	bool Dequeue_Tail(Data* pData);

	// ���� ������ ����
	bool Peek_Head(Data* pData);

	// ���� ������ ����
	bool Peek_Tail(Data* pData);

	// ���� ������
	int Size();
};

#endif // !__DEQUE_H__
