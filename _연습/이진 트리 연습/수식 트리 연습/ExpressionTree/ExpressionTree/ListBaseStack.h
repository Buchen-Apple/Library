#pragma once
#ifndef __LIST_BASE_STACK_H__
#define __LIST_BASE_STACK_H__

#define TRUE 1
#define FALSE 0

#include "BinaryTree.h"

//typedef int Data;
typedef BTreeNode* Data;

typedef struct _node
{
	Data data;
	struct _node* next;
}Node;

typedef struct _liststack
{
	Node* head;

} ListStack;

typedef ListStack Stack;

// �����ʱ�ȭ
void StackInit(Stack* pstack);

// ���� ������� Ȯ��
int SIsEmpty(Stack* pstack);

// Push
void SPush(Stack* pstack, Data data);

//Pop
Data SPop(Stack* pstack);

//Peek
Data Peek(Stack* pstack);

#endif // !__LIST_BASE_STACK_H__

