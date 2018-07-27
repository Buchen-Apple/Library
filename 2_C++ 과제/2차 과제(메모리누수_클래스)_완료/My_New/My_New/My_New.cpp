
#include <cstring>
#include <stdio.h>
#include <time.h>
#include <windows.h>
#include <tchar.h>

#include "My_New.h"
#undef new // "My_New.h"���� ���� new ��ũ�� undef

// ���� �޽��� ���ڿ� �����
#define ARRAY_ERROR "ARRAY"
#define NOALLOC_ERROR "NOALLOC"
#define LEAK_ERROR "LEAK"

size_t AllocSize = 0;	// �� �˷� �뷮 ����
size_t AllocCount = 0;	// �� �˷� ī��Ʈ
time_t nowTime = time(NULL);	// ���α׷� ���� �� �ð� ����
tm Tdata;	// �ð��� ������ ����ü ����
TCHAR FileName[300];	// ���� �̸��� ������ ����
FILE* wStream;	// ���� ��Ʈ��
bool bCheck = false;	// ����Ʈ �������θ� üũ�ϴ� bool ����
MemoryCheck CTemp;	// ����Ʈ�� �Ѱܼ� ���� ���� �޾ƿ��� ��ü

// ����Ʈ ����
LinkedList list;

// ����Ʈ �ʱ�ȭ���� üũ
void NewListInitCheck()
{
	if (AllocCount == 0)
	{
		list.ListInit(&list);
		localtime_s(&Tdata, &nowTime);

		// ����, FileName�� TCHAR(wchat_s)�����̱� ������, �Ϲ� ���� �������� �Ϸ��� �����ڵ� ���ڸ� ��Ƽ����Ʈ�� �ٲ����. �װ� ��������, _tfopen_s�� ����Ѵ�.
		// _tfopen_s�� ���ڿ��� �����ڵ��� _wfopne�� ȣ�� / ���ڿ��� ��Ƽ����Ʈ��� fopen�� ȣ��
		swprintf_s(FileName, _countof(FileName), TEXT("Alloc_%d%02d%02d_%02d%02d%02d.txt"), Tdata.tm_year + 1900, Tdata.tm_mon + 1, Tdata.tm_mday, Tdata.tm_hour, Tdata.tm_min, Tdata.tm_sec);
		_tfopen_s(&wStream, FileName, _T("wt"));

		// �Ʒ� 2���� FileName�� char�� ������ ��� ��� ����
		//strftime(FileName, 300, "Alloc_%Y%m%d_%H%M%S.txt", &data);
		//fopen_s(&wStream, FileNameCopy, "wt");		
	}
}

// operator new �����Լ�. MemoryCheck�� frined
void* operator new (size_t size, const char* File, int Line)
{
	// ����Ʈ �ʱ�ȭ���� üũ �Լ� ȣ��. ���� �����Ҵ��̸� ����Ʈ �ʱ�ȭ
	NewListInitCheck();

	// ��ü �����Ҵ�.
	MemoryCheck* mem = new MemoryCheck;

	// �����Ҵ� ���� ��ü ����
	mem->size = size;
	strcpy_s(mem->_name, strlen(File) + 1, File);
	mem->_line = Line;
	mem->_address = malloc(size);
	mem->ArrayCheck = false;

	// �Ҵ���� �����Ҵ� ������ Ƚ�� ����
	AllocSize += mem->size;
	AllocCount++;

	// ����Ʈ�� �ִ´�.
	list.ListInsert(&list, mem);

	return mem->_address;
}

// operator new[] �����Լ�. MemoryCheck�� frined
void* operator new[](size_t size, const char* File, int Line)
{
	// ����Ʈ �ʱ�ȭ���� üũ �Լ� ȣ��. ���� �����Ҵ��̸� ����Ʈ �ʱ�ȭ
	NewListInitCheck();

	MemoryCheck* mem = new MemoryCheck;

	mem->size = size;
	strcpy_s(mem->_name, strlen(File) + 1, File);
	mem->_line = Line;
	mem->_address = malloc(size);
	mem->ArrayCheck = true;

	// �Ҵ���� �����Ҵ� ������ Ƚ�� ����
	AllocSize += mem->size;
	AllocCount++;

	// ����Ʈ�� �ִ´�.
	list.ListInsert(&list, mem);

	return mem->_address;
}

// operator delete �����Լ�. MemoryCheck�� frined
void operator delete (void* ptr)
{
	bCheck = list.ListSearch(&list, ptr, &CTemp);

	// ����Ʈ���� ���𰡸� �����Դٸ�.
	if (bCheck)
	{
		// �ش� ������ ����Ʈ���� ����
		bCheck = list.ListDelete(&list, ptr);

		// �����ƴ� ������ �Ϲ� new�� �Ҵ�� �����̶��
		if (CTemp.ArrayCheck == false)
		{
			puts("�Ϲ� ����Ʈ");
			return;
		}

		// �ش� ������ �迭 new�� �Ҵ�� ������ ��� (ARRAY������ ���Ͽ� ����)
		else
		{
			fprintf_s(wStream, "%s    [0x%X]  [    %zd]  %s : %d\n", ARRAY_ERROR, CTemp._address, CTemp.size, CTemp._name, CTemp._line);
			puts("�迭�� �Ϲ����� ���� �õ���.");
			return;
		}
	}

	// ����Ʈ�� ���ϴ� ������ ���ٸ� (NOALLOC������ ���Ͽ� ����)
	else
	{
		fprintf_s(wStream, "%s  [0x%X]\n", NOALLOC_ERROR, ptr);
	}

}

// operator delete[] �����Լ�. MemoryCheck�� frined
void operator delete[](void* ptr)
{
	bCheck = list.ListSearch(&list, ptr, &CTemp);

	// ����Ʈ���� ���𰡸� �����Դٸ�.
	if (bCheck)
	{
		// �ش� ������ ����Ʈ���� ����
		bCheck = list.ListDelete(&list, ptr);

		// �����ƴ� ������ �迭 new�� �Ҵ�� �����̶��
		if (CTemp.ArrayCheck == true)
		{
			puts("�迭 ����Ʈ");
			return;
		}

		//�ش� ������ �Ϲ� new�� ������ �����̶�� (ARRAY ������ ���Ϸ� ����)
		else
		{
			fprintf_s(wStream, "%s    [0x%X]  [    %zd]  %s : %d\n", ARRAY_ERROR, CTemp._address, CTemp.size, CTemp._name, CTemp._line);
			puts("�Ϲ��� �迭���� ���� �õ���.");
			return;
		}
	}

	// ����Ʈ�� ���ϴ� ������ ���ٸ� (NOALLOC������ ���Ͽ� ����)
	else
	{
		fprintf_s(wStream, "%s  [0x%X]\n", NOALLOC_ERROR, ptr);
	}

}

// �Ҹ���
MemoryCheck::~MemoryCheck()
{
	// ����Ʈ�� �����ִ� ������ �ϳ��� �˻�. 
	// ù ��� �˻�
	if (list.ListSearchFirst(&list, &CTemp))
	{
		// ��尡 �ִ� ���, �ش� ����� �޸� �� �޽����� ���Ͽ� ����
		fprintf_s(wStream, "%s     [0x%X]  [    %zd]  %s : %d\n", LEAK_ERROR, CTemp._address, CTemp.size, CTemp._name, CTemp._line);

		// ���� ��� �˻�. ����Ʈ�� ���� ������ �� ���� �˻�
		while (list.ListSearchSecond(&list, &CTemp))
			// ���� ��尡 ������, �ش� ����� �޸� �� �޽����� ���Ͽ� ����
			fprintf_s(wStream, "%s     [0x%X]  [    %zd]  %s : %d\n", LEAK_ERROR, CTemp._address, CTemp.size, CTemp._name, CTemp._line);
	}

	// �� �˷� ī��Ʈ / �뷮�� ���Ϸ� ����
	fprintf_s(wStream, "\n�� Alloc Ƚ�� : %d\n�� Alloc �뷮 : %d\n", AllocCount, AllocSize);

	// �Ұ� �� ������ ���� ��Ʈ�� ����
	fclose(wStream);
}

// ���ϴ� �ּ�(temp)�� ����Ʈ�� ã�´�. ListDelete / ListSearch���� ���
bool LinkedList::SearchFunc(LinkedList* list, void* temp)
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
		if(list->_cur->_data)
		if (list->_cur->_data->_address == temp)
			break;

		// �� ��Ȳ�� �� �� �ƴϸ� >>��ĭ �̵�
		list->_cur = list->_cur->_next;
	}

	return true;
}

// ����Ʈ �ʱ�ȭ
void LinkedList::ListInit(LinkedList* list)
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
void LinkedList::ListInsert(LinkedList* list, Data* data)
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

// ����Ʈ ����. ���ϴ�(temp) �ּ��� ��带 ����Ʈ���� ����
bool LinkedList::ListDelete(LinkedList* list, void* temp)
{
	// ���ϴ� �ּҸ� ã�´�. ���ϴ� �ּ�(temp)�� ������ false / ������ true ��ȯ
	bool bCheck = list->SearchFunc(list, temp);

	// �ּҸ� ã�Ҵٸ�
	if (bCheck == true)
	{
		// ���� ����� ������ ���� ����� ������ ���θ� �����Ѵ�.
		list->_cur->_prev->_next = list->_cur->_next;
		list->_cur->_next->_prev = list->_cur->_prev;

		return true;
	}

	return false;
}

// ����Ʈ �˻�. ���ϴ�(temp) �ּҰ� ����Ʈ�� ������ �ش� ����Ʈ�� _data�� data�� ����. ���ϴ� �ּ��� ����Ʈ�� ������ false ��ȯ
bool LinkedList::ListSearch(LinkedList* list, void* temp, Data* data)
{
	// ���ϴ� �ּҸ� ã�´�. ���ϴ� �ּ�(temp)�� ������ false / ������ true ��ȯ
	bool bCheck = list->SearchFunc(list, temp);

	// �ּҸ� ã�Ҵٸ�
	if (bCheck == true)
	{
		memcpy(data, list->_cur->_data, sizeof(Data));
		return true;
	}

	return false;
}

// ����Ʈ ��ȸ(First). ù ��尡 ������ true / ������ flase ��ȯ
bool LinkedList::ListSearchFirst(LinkedList* list, Data* data)
{
	list->_cur = list->_head->_next;

	if (list->_cur == list->_tail)
		return false;

	memcpy(data, list->_cur->_data, sizeof(Data));

	return true;
}

// ����Ʈ ��ȸ(Next). ���� ��尡 ������ true / ������ false ��ȯ
bool LinkedList::ListSearchSecond(LinkedList* list, Data* data)
{
	list->_cur = list->_cur->_next;

	if (list->_cur == list->_tail)
		return false;

	memcpy(data, list->_cur->_data, sizeof(Data));

	return true;
}