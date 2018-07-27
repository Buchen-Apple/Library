#include <stdio.h>
#include <stdlib.h>
#include <Windows.h>
#include <mmsystem.h>
#include <conio.h>

#define UNIT_COUNT 5

enum
{
	UNIT_CREATE = 49, EXIT
};

typedef struct _queue
{
	int front;
	int cur;
	int rear;
	ULONGLONG m_Tick[UNIT_COUNT];
}cQueue;

void QueueInit(cQueue* pq);
int NextPos(int pos);
bool Enqueue(cQueue* pq, ULONGLONG tick);
bool Dequeue(cQueue* pq);
bool Qpeek(cQueue* pq, ULONGLONG* tick);


int main()
{
	int Key;
	int icurShowTime;
	ULONGLONG tick;
	ULONGLONG curtick;
	bool bExitCheck = true;
	bool bCompleteCheck = false;
	bool bUnitMaxTextShow = false;
	
	// ť ���� �� �ʱ�ȭ
	cQueue queue;
	QueueInit(&queue);

	// Ÿ�̸� ���ͷ�Ʈ�� 1m/s�� ����
	timeBeginPeriod(1);

	while (1)
	{
		Key = 0;
		// Ű���� ����		
		if (_kbhit())
		{
			Key = _getch();
			tick = GetTickCount64();
		}

		// ����
		switch (Key)
		{
		case UNIT_CREATE:
			// ���� �������� ������ 0~3����� 1���� �߰�. 4����� �� �̻� ���� �Ұ���. 	
			if (!Enqueue(&queue, tick))
				bUnitMaxTextShow = true;	
			break;
		case EXIT:
			// ���α׷� ����
			bExitCheck = false;
			break;

		}
		// EXIT�� �������� ���α׷� ����
		if (bExitCheck == false)
			break;

		// ������ ����
		system("cls");

		puts("[ 1 : ���� ���� ��û / 2 : ���� ]\n");
		puts("-- CONTROL TOWER UNIT CREATE --");
		puts("#-------------------------------------------------------------------------");
		printf("# ");

		// ť�� ó������ ��ȸ�Ѵ�.
		while (Qpeek(&queue, &tick))
		{
			// ���� �ð��� �޾ƿ´�.
			
			curtick = GetTickCount64();

			// ���⼭�� 10��(10000�и�������)�� ����1���� �ϼ��ȴ�. ������ �Ʒ� ���� ����
			// �Ʒ� ������, ���� �ð��� ���� ���� ��û�� �ð�+10000�̶� ���� ������ ����� ���̴�.
			// 1�ʿ� 10%�� �����Ѵ�. 
			icurShowTime = (int)(100 - (((tick + 10000) - curtick) / 100));

			// ��� ���, 100%���� ū ���� ������ �׳� 100%�� �����.
			if (icurShowTime > 100)
				icurShowTime = 100;

			// 100%�� �ƴٸ�(���� ���� �Ϸ�)
			if (icurShowTime == 100)
			{
				// ���� �Ϸ�� ������ ����.
				Dequeue(&queue);
				// �Ʒ����� �Ϸ�Ǿ��ٴ� ���� �����ֱ� ���� �÷��׸� on ��Ų��.
				bCompleteCheck = true;
			}
			printf("���ֻ�����..[%02d%%] ", icurShowTime);

		}
		puts("\n#-------------------------------------------------------------------------");

		// ������ �� �ؽ�Ʈ ǥ��
		// ������ ��, ������ �Ϸ�� ������ ������ ���� ���� �Ϸ� �ؽ�Ʈ ������.
		if (bCompleteCheck == true)
		{
			puts("##���� ���� �Ϸ�!");
			bCompleteCheck = false;
		}

		// ���� ��, ���� ���â�� ����á���� ���â�� ����á�ٰ� �ؽ�Ʈ ������.
		if (bUnitMaxTextShow == true)
		{
			puts("##���� ���â�� ���� á���ϴ�!");
			bUnitMaxTextShow = false;
		}

		Sleep(600);
	}
	// Ÿ�̸� ���ͷ�Ʈ�� ����
	timeEndPeriod(1);	

	return 0;
}


void QueueInit(cQueue* pq)
{
	pq->front = 0;
	pq->rear = 0;
	pq->cur = pq->front;
}

int NextPos(int pos)
{
	if (pos == UNIT_COUNT - 1)
		return 0;
	else
		pos += 1;
}

bool Enqueue(cQueue* pq, ULONGLONG tick)
{
	// ť�� ���� �����ߴ��� üũ
	if (NextPos(pq->rear) == pq->front)
		return false;

	pq->rear = NextPos(pq->rear);
	pq->m_Tick[pq->rear] = tick;
	return true;
}

bool Dequeue(cQueue* pq)
{
	// ť�� �� �� �������� üũ
	if (pq->front == pq->rear)
		return false;
		
	pq->front = NextPos(pq->front);
	pq->cur = pq->front;
	pq->m_Tick[pq->front] = 0;	
	return true;
}

bool Qpeek(cQueue* pq, ULONGLONG* tick)
{
	// �� �� ��Ȳ���� üũ
	// ������ ��ȸ�ߴٸ�, �ٽ� cur�� front�� �̵�
	if (pq->cur == pq->rear)
	{
		pq->cur = pq->front;
		return false;
	}

	// �� �� ��Ȳ�� �ƴϸ�, ���� ��ġ�� ��ĭ �̵��� ��, ���� tick�� �ִ´�.
	pq->cur = NextPos(pq->cur);
	*tick = pq->m_Tick[pq->cur];
	
	return true;
}