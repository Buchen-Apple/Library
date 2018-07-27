#pragma once
#ifndef __MY_NEW_H__
#define __MY_NEW_H__

// Ŭ���� �����
class MemoryCheck
{
private:
	size_t size;
	char _name[300];
	int _line;
	bool ArrayCheck;

public:
	void* _address;
	friend void* operator new (size_t size, const char* File, int Line); // operator new �����Լ�. MemoryCheck�� frined
	friend void* operator new[](size_t size, const char* File, int Line); // operator new[] �����Լ�. MemoryCheck�� frined
	friend void operator delete (void* ptr); // operator delete �����Լ�. MemoryCheck�� frined
	friend void operator delete[](void* ptr); // operator delete[] �����Լ�. MemoryCheck�� frined
	~MemoryCheck();
};

void* operator new (size_t size, const char* File, int Line);
void* operator new[](size_t size, const char* File, int Line);
void operator delete (void* ptr);
void operator delete[](void* ptr);
void operator delete (void* p, char* File, int Line) {}	// ����������
void operator delete[](void* p, char* File, int Line) {} // ����������

typedef MemoryCheck Data;

// ����Ʈ�� ��� Ŭ����
class Node	
{
public:
	Data* _data;
	Node* _prev;
	Node* _next;
};

// ����Ʈ Ŭ����
class LinkedList 
{
	Node * _cur;
	Node* _head;
	Node* _tail;

public:	
	bool SearchFunc(LinkedList* list, void* temp); // ���ϴ� �ּ�(temp)�� ����Ʈ�� ã�´�. ListDelete / ListSearch���� ���
	void ListInit(LinkedList* list); // ����Ʈ �ʱ�ȭ
	void ListInsert(LinkedList* list, Data* data); // ����Ʈ �߰� (������ �߰�)
	bool ListDelete(LinkedList* list, void* temp); // ����Ʈ ����. ���ϴ�(temp) �ּ��� ��带 ����Ʈ���� ����
	bool ListSearch(LinkedList* list, void* temp, Data* data); // ����Ʈ �˻�. ���ϴ�(temp) �ּҰ� ����Ʈ�� ������ �ش� ����Ʈ�� _data�� data�� ����. ���ϴ� �ּ��� ����Ʈ�� ������ false ��ȯ
	bool ListSearchFirst(LinkedList* list, Data* data); // ����Ʈ ��ȸ(First). ù ��尡 ������ true / ������ flase ��ȯ
	bool ListSearchSecond(LinkedList* list, Data* data); // ����Ʈ ��ȸ(Next). ���� ��尡 ������ true / ������ false ��ȯ
};

// operator new�Լ��� ȣ���ϴ� ��ũ��
#define new new(__FILE__,__LINE__)

#endif // !__MY_NEW_H__



