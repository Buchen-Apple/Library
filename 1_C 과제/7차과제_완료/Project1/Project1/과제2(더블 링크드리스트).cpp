#include <stdio.h>
#include <stdlib.h>
#include <Windows.h>
#include <mmsystem.h>
#include <conio.h>

#define UNIT_COUNT 4

int g_iGlobalUnitCount = 0;

enum 
{
	UNIT_CREATE = 49, EXIT
};

typedef struct _node
{
	_node* pPrev;
	_node* pNext;
	ULONGLONG m_Tick;

}unit;

typedef struct
{
	_node* pHead;
	_node* pTail;
	_node* pCur;

}LinkedList;

void ListInit(LinkedList* pList);
bool ListCreate(LinkedList* pList, ULONGLONG Tick);
bool ListFirstPeek(LinkedList* pList, ULONGLONG* Tick);
bool ListNextPeek(LinkedList* pList, ULONGLONG* Tick);
bool ListDelete(LinkedList* pList);


int main()
{
	int Key;
	int icurShowTime;
	ULONGLONG tick;
	ULONGLONG curtick;
	bool bExitCheck = true;
	bool bCompleteCheck = false;
	bool bUnitMaxTextShow = false;

	LinkedList list;
	ListInit(&list);

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
			if (!ListCreate(&list, tick))
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
		// ù ��° ����Ʈ ������ �����´�. ù ���� ����Ʈ�� �ִ� ��쿡�� �����´�. ��, ���� ��û�� ������ 0���� �̰� ���� ����.
		if (ListFirstPeek(&list, &tick))
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
				ListDelete(&list);
				// �Ʒ����� �Ϸ�Ǿ��ٴ� ���� �����ֱ� ���� �÷��׸� on ��Ų��.
				bCompleteCheck = true;
			}
			printf("���ֻ�����..[%02d%%] ", icurShowTime);

			// ù ��° ���� ����Ʈ ������ ���� ��쿡�� �����´�.
			while (ListNextPeek(&list,&tick))
			{
				curtick = GetTickCount64();
				icurShowTime = (int)(100 - (((tick + 10000) - curtick) / 100));
				if (icurShowTime > 100)
					icurShowTime = 100;
				if (icurShowTime == 100)
				{
					ListDelete(&list);
					bCompleteCheck = true;
				}
				printf("���ֻ�����..[%02d%%] ", icurShowTime);		
			}
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
	timeEndPeriod(1);

	return 0;
}


void ListInit(LinkedList* pList)
{
	//��� ���� ����
	unit* HeadNode = (unit*)malloc(sizeof(unit));
	HeadNode->m_Tick = NULL;
	HeadNode->pPrev = NULL;

	//���� ���� ����
	unit* TailNode = (unit*)malloc(sizeof(unit));
	TailNode->m_Tick = NULL;
	TailNode->pNext = NULL;

	// ��� ���̿� ���� ���̸� ���� ����
	HeadNode->pNext = TailNode;
	TailNode->pPrev = HeadNode;

	// ��尡 �����̸�, ������ �������̸� ����Ų��. 
	pList->pHead = HeadNode;
	pList->pTail = TailNode;
	pList->pCur = NULL;
}

bool ListCreate(LinkedList* pList, ULONGLONG Tick)
{
	if (g_iGlobalUnitCount == UNIT_COUNT)
		return false;

	// ���ο� ���� �������� �߰��ȴ�.
	// ���ο� ��� ����
	unit* newNode = (unit*)malloc(sizeof(unit));
	newNode->m_Tick = Tick;

	// ���ο� ����� ������ ������, ���ο� ����� ������ ������ ������ ����Ų��.
	newNode->pNext = pList->pTail;
	newNode->pPrev = pList->pTail->pPrev;
	pList->pCur = NULL;

	// ������ ������ ������ ���ο� ��带, ������ ������ ���ο� ��带 ����Ų��.
	pList->pTail->pPrev->pNext = newNode;
	pList->pTail->pPrev = newNode;	

	g_iGlobalUnitCount++;

	return true;
}

bool ListFirstPeek(LinkedList* pList, ULONGLONG* Tick)
{
	// ù ��带 ��ȯ�Ѵ� (��� ���� �������� üũ)
	// 1. �ϴ�, pCur�����Ͱ� ���� ���� ��带 ����Ų��.
	pList->pCur = pList->pHead->pNext;

	// 2. ����, ���� ����Ű�°� ���� ������ ��, �ϳ��� �߰��Ȱ� ���ٴ� ���̴� false�� ��ȯ�Ѵ�.
	if (pList->pCur->pNext == NULL)
		return false;

	// 3. ������� ������ ���̰� �ƴ϶�� ���̴�, Tick���ٰ� ���� �ְ� true�� ��ȯ�Ѵ�.
	(*Tick) = pList->pCur->m_Tick;

	return true;
}

bool ListNextPeek(LinkedList* pList, ULONGLONG* Tick)
{
	// ���� ��� ��ȯ
	// 1. �ϴ� pCur�����Ͱ� pCur->pNext�� ����Ų��.
	pList->pCur = pList->pCur->pNext;

	// 2. ���� ���� ����Ű�°� �������̶��, �������� �����ߴٴ� ���̴� false�� ��ȯ
	if (pList->pCur->pNext == NULL)
		return false;

	// 3. ������� ������ ���̰� �ƴ϶�� ���̴�, Tick���ٰ� ���� �ְ� true�� ��ȯ�Ѵ�.
	(*Tick) = pList->pCur->m_Tick;
	return true;
}

bool ListDelete(LinkedList* pList)
{
	// ���� ����Ű�� �ִ� ����Ʈ�� �����Ѵ�.
	// 1. �ӽ� �����Ͱ� ���縦 ����Ų��.
	unit* pTempCur = pList->pCur;

	if (pTempCur->pPrev == NULL || pTempCur->pNext == NULL)
		return false;

	// 2. ������ ������ �������� ������ ������ ����Ų��.
	pList->pCur->pPrev->pNext = pList->pCur->pNext;

	// 3. ������ ������ �������� ������ ������ ����Ų��.
	pList->pCur->pNext->pPrev = pList->pCur->pPrev;

	// 4. ���簡 ������ ������ ����Ų��.
	pList->pCur = pList->pCur->pPrev;

	// 5. ���� �Ҹ�
	free(pTempCur);

	g_iGlobalUnitCount--;
	return true;
}