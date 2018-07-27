#pragma once
#ifndef __LINKED_LIST_H__
#include <stdlib.h>
#include <string.h>

#define __LINKED_LIST_H__

typedef struct node
{
	int Key;
	char Data[100];
	node* pPrev;
	node* pNext;

}Node;

typedef struct linkedlist
{
	Node* pHead;
	Node* pTail;


}LinkedList;

// ����Ʈ �ʱ�ȭ
void Init(LinkedList* pList)
{
	// ���� 2�� ����
	Node* headDumy = (Node*)malloc(sizeof(Node));
	headDumy->Key = -1;
	strcpy_s(headDumy->Data, 5, "Dumy");

	Node* tailDumy = (Node*)malloc(sizeof(Node));
	tailDumy->Key = -1;
	strcpy_s(tailDumy->Data, 5, "Dumy");

	// ���� 2���� ���� ����
	headDumy->pPrev = NULL;
	headDumy->pNext = tailDumy;

	tailDumy->pPrev = headDumy;
	tailDumy->pNext = NULL;	

	// ���õ� ���̸� ����Ʈ�� ����Ų��.
	pList->pHead = headDumy;
	pList->pTail = tailDumy;
}

// �Ӹ��� ����Ʈ �߰�
void ListInsert_Head(LinkedList* pList, int Key, char* Data)
{
	// ���ο� ����� �� ����
	Node* newNode = (Node*)malloc(sizeof(Node));
	newNode->Key = Key;
	strcpy_s(newNode->Data, strlen(Data)+1, Data);

	// ���ο� ����� ����, ���� ����
	newNode->pPrev = pList->pHead;
	newNode->pNext = pList->pHead->pNext;

	// ����� ������ ���ο� ��尡 �ǰ�, ����� ������ ������ ���ο� ��尡 �ȴ�.
	pList->pHead->pNext->pPrev = newNode;
	pList->pHead->pNext = newNode;	
}

bool ListShow(LinkedList* pList, Node* returnNode, int pos)
{
	// ��带 �������� pos��ŭ >>�� ������ ��带 ��ȯ�Ѵ�.
	// ����, ��ȯ�� ��带 ����ų ������ ����
	LinkedList returnList;

	// returnList�� pList�� ���(����)�� ����Ų��.
	returnList.pHead = pList->pHead;	

	// pList�� pos��ŭ �����̰� ��, ���� returnList�� ����Ų��.
	for (int i = 0; i < pos; ++i)
		returnList.pHead = returnList.pHead->pNext;

	// ����, pos��ŭ ������ ��, ���� ��尡 ���̶�� false ��ȯ
	if (returnList.pHead->pNext == NULL)
		return false;

	// pos��ŭ ������ ����� ���� returnNode�� �ִ´�.
	returnNode->Key = returnList.pHead->Key;
	strcpy_s(returnNode->Data, strlen(returnList.pHead->Data) + 1, returnList.pHead->Data);

	return true;
}

bool ListPeek(LinkedList* pList, char Data[100], int SearchKey)
{
	// ��带 �������� >>�� �����̸鼭, key���� ������ ��带 ã�´�.
	// ����, ��ȯ�� ��带 ����ų �����͸� �����Ѵ�.
	LinkedList SearchList;

	// SearchList�� pList�� ����� ������ ����Ų��.
	SearchList.pHead = pList->pHead->pNext;

	// ��ĭ�� ������ �����̸鼭 key�� ���Ѵ�.
	while (1)
	{
		// ����, ���� ����(����)��� ������ Ű�� ���°Ŵ� false ��ȯ
		if (SearchList.pHead->pNext == NULL)
			return false;

		// ���� ���ϴ� Ű�� ã�Ҿ �ݺ��� ����
		if (SearchList.pHead->Key == SearchKey)
			break;

		SearchList.pHead = SearchList.pHead->pNext;

	}	

	// ������� ������ ���ϴ� Ű�� ã���Ŵ� �ش� ����� Data�� �����Ѵ�.
	strcpy_s(Data, sizeof(SearchList.pHead->Data), SearchList.pHead->Data);
	return true;
}

bool ListDelete(LinkedList* pList, int Key)
{
	// ������ >>�� �����̸鼭 Key�� ã�´�. Key�� ã���� ������ true ��ȯ / ��ã������ false��ȯ
	// ����, ��ȸ�� �����͸� �����Ѵ�.
	LinkedList DeleteList;

	// DeleteList�� pList�� ��� ������ ����Ų��.
	DeleteList.pHead = pList->pHead->pNext;

	while (1)
	{
		// ����, ���� ���� ����(����)���, ���������� ��ȸ�� ���̴� false ��ȯ
		if (DeleteList.pHead->pNext == NULL)
			return false;

		// ���� ���ϴ� �� Key�� ã������ �ݺ��� ����
		if (DeleteList.pHead->Key == Key)
			break;

		DeleteList.pHead = DeleteList.pHead->pNext;
	}

	// ������� ������ ���ϴ� Ű�� ã���Ŵ�, ���� ������ �����Ѵ�.
	DeleteList.pHead->pPrev->pNext = DeleteList.pHead->pNext;
	DeleteList.pHead->pNext->pPrev = DeleteList.pHead->pPrev;
	free(DeleteList.pHead);

	return true;
}


#endif // !__LINKED_LIST_H__
