#pragma once
#ifndef __SIMPLE_HEAP_H__
#define __SIMPLE_HEAP_H__

#define TRUE 1
#define FALSE 0

#define HEAP_LEN 100

typedef char* HData;
typedef int PriorityComp(HData d1, HData d2);


/////////////////////////////////////////////////////
// comp : �켱������ ���ϴ� �Լ��� �̸��� ������ �Լ�������
//
// *********************����**********************
// 1�� ������ �켱������ ������ 0���� ū ��
// 2�� ������ �켱������ ������ 0���� ���� ��
// �� ������ �켱������ �����ϸ� 0 ��ȯ
/////////////////////////////////////////////////////

// numOfData : heapArry�� ����� �����Ͱ� �ִ� ������ ��ġ
// heapArry : HeapElem�� �����ϴ� �迭
typedef struct _heap
{
	PriorityComp* comp;
	int numOfData;
	HData heapArry[HEAP_LEN];

}Heap;

void HeapInit(Heap* ph, PriorityComp pc);

int HIsEmpty(Heap* ph);

void HInsert(Heap* ph, HData data);

HData HDelete(Heap* ph);

#endif // !__SIMPLE_HEAP_H__
