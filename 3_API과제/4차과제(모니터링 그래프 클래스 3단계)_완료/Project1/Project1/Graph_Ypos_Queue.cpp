#include "stdafx.h"
#include "Graph_Ypos_Queue.h"

// �ʱ�ȭ
void Init(Queue* pq)
{
	pq->front = 0;
	pq->rear = 0;
}

// ���� ��ġ Ȯ��
int NextPos(int pos)
{
	if (pos == QUEUE_LEN - 1)
		return 0;
	else
		return pos + 1;
}

// ��ť
int Dequeue(Queue* pq)
{
	pq->front = NextPos(pq->front);
	return pq->queArr[pq->front];
}

// ��ť
void Enqueue(Queue* pq, int y)
{
	if (pq->front == NextPos(pq->rear))
	{
		Dequeue(pq);
	}
	pq->rear = NextPos(pq->rear);
	pq->queArr[pq->rear] = y;
}

// ù ť Peek
bool FirstPeek(Queue* pq, int* Data)
{
	if (pq->front == pq->rear)
		return false;

	pq->Peek = NextPos(pq->front);

	*Data = pq->queArr[pq->Peek];
	return true;

}

// ���� ť Peek
bool NextPeek(Queue* pq, int* Data)
{
	if (pq->Peek == pq->rear)
		return false;

	pq->Peek = NextPos(pq->Peek);

	*Data = pq->queArr[pq->Peek];
	return true;

}