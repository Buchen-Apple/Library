#include <iostream>
#include <cstring>
#include <stdlib.h>
#include "Memory_Leak_Linked_List.h"

using namespace std;

// MallocFunc�Լ��� ȣ���ϴ� ��ũ�� �Լ�
#define MallocMacro(type, size) MallocFunc<type>(size, __FILE__, __LINE__);

// �������� ����
int AllocSize = 0;
int AllocCount = 0;

// �޸� ���� ���� ����Ʈ ����
LinkedList list;

// MallocFunc �Լ� ���ø�
template <class T>
T* MallocFunc(int, const char*, int);

// FreeFunc �Լ� ���ø�
template <typename T>
void FreeFunc(T*);

// ���� �޸� ��Ȳ ȣ��
void ShowMomoryFunc();

int main()
{
	// ����Ʈ �ʱ�ȭ
	ListInit(&list);

	// �� �����Ҵ�
	char *c1 = MallocMacro(char, 40);
	short *s1 = MallocMacro(short, 4);
	int *p1 = MallocMacro(int, 4);
	long *l1 = MallocMacro(long, 4);
	long long *dl1 = MallocMacro(long long, 4);
	float *f1 = MallocMacro(float, 4);
	double *d1 = MallocMacro(double, 4);

	// ����Ʈ
	//FreeFunc(c1);
	FreeFunc(s1);
	//FreeFunc(p1);
	FreeFunc(l1);
	FreeFunc(dl1);
	FreeFunc(f1);
	//FreeFunc(d1);

	// �޸� ���� ��Ȳ Ȯ��
	ShowMomoryFunc();

	return 0;
}

// MallocFunc �Լ� ���ø�
template <class T>
T* MallocFunc(int size, const char* name, int line)
{
	Memory* mem = new Memory;

	T* temp = new T[size];

	mem->size = size * sizeof(T);
	strcpy_s(mem->_name, strlen(name) + 1, name);
	mem->_line = line;
	mem->_address = temp;

	ListInsert(&list, mem);

	AllocSize += mem->size;
	AllocCount++;

	return temp;
}

// FreeFunc �Լ� ���ø�
template <typename T>
void FreeFunc(T* ptr)
{
	bool bCheck = ListDelete(&list, ptr);

	if (!bCheck)
	{
		cout << "���� �Ҵ����� ���� �ּҴ� delete �� �� �����ϴ�." << endl;
		return;
	}

	delete ptr;
}

// ���� �޸� ��Ȳ ȣ��
void ShowMomoryFunc()
{
	cout << "-----------�޸� ���� ��Ȳ-----------" << endl;
	cout << "�� �����Ҵ� ������ : " << AllocSize << " Byte" << endl;
	cout << "�� �����Ҵ� Ƚ�� : " << AllocCount << endl << endl;;

	// �ݺ����� ���鼭, ��� ����Ʈ�� �˻�. ����Ʈ�� �����ִ� ������ �������� ���� �޸𸮵��̴�.
	Memory mem;

	if (ListSearchFirst(&list, &mem))	// ��带 ã���� ���, mem���ٰ� ã�� ����� ������ �ִ´�.
	{
		cout << "�������� ���� �޸� : [0x" << mem._address << "] / " << mem.size << " Byte" << endl;
		cout << "���� : " << mem._name << " / " << mem._line << "Line" << endl << endl;

		while (ListSearchSecond(&list, &mem))
		{
			cout << "�������� ���� �޸� : [0x" << mem._address << "] / " << mem.size << " Byte" << endl;
			cout << "���� : " << mem._name << " / " << mem._line << "Line" << endl << endl;
		}
	}
	else
	{
		cout << "�������� ���� �޸� ����" << endl;
	}
}