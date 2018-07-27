#pragma once
#ifndef __MEMORY_LEAK_LINKED_LIST_H__
#define __MEMORY_LEAK_LINKED_LIST_H__

#include <cstring>
#define MEM_ARRAY_MAX 300

// �Ҵ� ��, �޸� ������ üũ�ϱ� ���� ����ü
struct Memory
{
	int size;
	char _name[300];
	int _line;
	void* _address;
};

typedef Memory Data;

struct Node
{
	Data* _data;
	Node* _prev;
	Node* _next;
};

struct LinkedList
{
	Node* _cur;
	Node* _head;
	Node* _tail;
};

// ����Ʈ �ʱ�ȭ
void ListInit(LinkedList* list)
{
	// ���, ���� ���� ����
	Node* headDumy = new Node;
	Node* tailDumy = new Node;

	// ���, ���� ���̰� ���θ� ����Ų��.
	headDumy->_prev = 0;
	headDumy->_next = tailDumy;

	tailDumy->_prev = headDumy;
	tailDumy->_next = 0;

	// ����Ʈ�� ���� ������ ���� ���̸� ����Ŵ
	list->_head = headDumy;
	list->_tail = tailDumy;
	list->_cur = list->_head;
}

// ����Ʈ �߰� (������ �߰�)
void ListInsert(LinkedList* list, Data* data)
{
	// ���ο� ��� ����
	Node* newNode = new Node;

	// ���ο� ���, _data�� data�� ���� �ִ´�.
	newNode->_data = data;

	// ���ο� ���, _data�� data�� ���� �ִ´�.

	// �ش� ��带 �ڿ������� �߰��Ѵ�. ��, �������� �߰�. 
	// ù ��� �߰��϶��� �� ���Ķ��� ������ �ٸ���.
	if (list->_tail->_prev == list->_head)	// ù �����
	{
		newNode->_prev = list->_head;
		newNode->_next = list->_tail;

		list->_head->_next = newNode;
		list->_tail->_prev = newNode;
	}

	else  // ù ��尡 �ƴ϶�� 
	{
		list->_tail->_prev->_next = newNode;
		newNode->_prev = list->_tail->_prev;

		newNode->_next = list->_tail;
		list->_tail->_prev = newNode;
	}

}

// ����Ʈ ����. ���ϴ� �ּ�(temp)�� ������ false / ������ true ��ȯ
bool ListDelete(LinkedList* list, void* temp)
{
	// temp(�ּ�)�� üũ�� ��带 ��� ��ġ�� �̵�
	list->_cur = list->_head->_next;

	// ��� ��ġ���� ��ĭ�� >>������ �̵��ϸ鼭 data._address �˻�.
	while (1)
	{
		// ���� ��尡 �������̶��, ���� ���� �����ߴµ��� ���ϴ� �ּҸ� ��ã���Ŵ� false ��ȯ;
		if (list->_cur == list->_tail)
			return false;

		// ���� ����� �ּҰ� temp�� ������ ���ϴ� �ּҸ� ã�����̴� break;
		if (list->_cur->_data->_address == temp)
			break;

		// �� ��Ȳ�� �� �� �ƴϸ� >>��ĭ �̵�
		list->_cur = list->_cur->_next;
	}

	// ���� ����� ������ ���� ����� ������ ���θ� �����Ѵ�.
	list->_cur->_prev->_next = list->_cur->_next;
	list->_cur->_next->_prev = list->_cur->_prev;

	// ���� ����� _data�� �Ҵ� �����Ѵ�
	delete list->_cur->_data;

	// ���� ��带 �Ҵ� �����Ѵ�.
	delete list->_cur;

	return true;
}

// ����Ʈ �˻�(First). ù ��尡 ������ true / ������ flase ��ȯ
bool ListSearchFirst(LinkedList* list, Data* data)
{
	list->_cur = list->_head->_next;

	if (list->_cur == list->_tail)
		return false;

	memcpy(data, list->_cur->_data, sizeof(Data));

	return true;
}

// ����Ʈ �˻�(Next). ���� ��尡 ������ true / ������ false ��ȯ
bool ListSearchSecond(LinkedList* list, Data* data)
{
	list->_cur = list->_cur->_next;

	if (list->_cur == list->_tail)
		return false;

	memcpy(data, list->_cur->_data, sizeof(Data));

	return true;
}

#endif // !__MEMORY_LEAK_LINKED_LIST_H__

