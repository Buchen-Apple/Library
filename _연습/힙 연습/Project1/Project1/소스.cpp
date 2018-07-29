#include <stdio.h>
#include <string.h>
#include "SimpleHeap.h"

// �켱���� �� �Լ�
/////////////////////////////////////////////////////
// *********************����**********************
// 1�� ������ �켱������ ������ 0���� ū ��
// 2�� ������ �켱������ ������ 0���� ���� ��
// �� ������ �켱������ �����ϸ� 0 ��ȯ
/////////////////////////////////////////////////////
int DataPriorityComp(HData ch1, HData ch2);

int main()
{
	Heap heap;
	HeapInit(&heap, DataPriorityComp);

	// ���� ������ �߰�
	HInsert(&heap, "abc");
	HInsert(&heap, "abcde");
	HInsert(&heap, "abcdefg");
	printf("%s \n", HDelete(&heap));

	// ���� ������ �ٽ� �߰�
	HInsert(&heap, "abcdefgh");
	HInsert(&heap, "abcdefghij");
	HInsert(&heap, "abcdefghijklnm");
	printf("%s \n", HDelete(&heap));

	// ������ ��� ��ť�ϱ�
	while (!HIsEmpty(&heap))
		printf("%s \n", HDelete(&heap));

	return 0;
}

// �켱���� �� �Լ�
int DataPriorityComp(HData ch1, HData ch2)
{
	// 1�� ������ �켱������ ������ 0���� ū ��
	// 2�� ������ �켱������ ������ 0���� ���� ��
	// �� ������ �켱������ �����ϸ� 0 ��ȯ
	//return ch2 - ch1;

	return strlen(ch2) - strlen(ch1);

}