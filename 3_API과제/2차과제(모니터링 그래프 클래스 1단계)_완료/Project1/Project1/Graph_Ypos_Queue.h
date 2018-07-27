#pragma once
#ifndef __GRAPH_YPOS_QUEUE_H__
#define __GRAPH_YPOS_QUEUE_H__

#define QUEUE_LEN 100
// ť ����
typedef struct
{
	int front;
	int rear;
	int Peek;
	int queArr[QUEUE_LEN];
}Queue;

// �ʱ�ȭ
void Init(Queue* pq);
// ���� ��ġ Ȯ��
int NextPos(int pos);
// ��ť
int Dequeue(Queue* pq);
// ��ť
void Enqueue(Queue* pq, int y);
// ù ť Peek
bool FirstPeek(Queue* pq, int* Data);
// ���� ť Peek
bool NextPeek(Queue* pq, int* Data);

#endif // !__GRAPH_YPOS_QUEUE_H__

