#include "SimpleHeap.h"

// �� �ʱ�ȭ
void HeapInit(Heap* ph, PriorityComp pc)
{
	ph->numOfData = 0;
	ph->comp = pc;
}

// �� ������� Ȯ��
int HIsEmpty(Heap* ph)
{
	if (ph->numOfData == 0)
		return TRUE;
	else
		return FALSE;
}


// �θ��� ID ����
int GetParentID(int idx)
{
	return idx / 2;
}

// ���� �ڽ��� ID ����
int GetLeftChildID(int idx)
{
	return idx * 2;
}

// ������ �ڽ��� ID ����
int GetRightChildID(int idx)
{
	return GetLeftChildID(idx) + 1;
}

// �ΰ��� �ڽ� �� ���� �켱������ �ڽ� ��� �ε����� ��ȯ
int GetHiPriChildID(Heap* ph, int idx)
{
	// ���� �ڽ���, ������ �����, ���Ұ͵� ���� ���ʳ�� �ٷ� ��ȯ
	if (GetLeftChildID(idx) == ph->numOfData)
		return GetLeftChildID(idx);

	// ���� �ڽ��� idx�� ������ ����� idx���� ũ�ٸ�, ���� ����̴�. 
	// ��, ���� idx�� ���� �ܸ�����̴�.
	else if (GetLeftChildID(idx) > ph->numOfData)
		return 0;

	// �ڽĳ�尡 �� �� �ִٸ� 
	else
	{
		// comp()�� ����� �Լ��� HData, �� ���� �Է¹޴´�. ���� �������� �켱���� ����.
		// 1�� ������ �켱������ ������ 0���� ū ��
		// 2�� ������ �켱������ ������ 0���� ���� ��
		// �� ������ �켱������ �����ϸ� 0 ��ȯ

		// ������ �ڽĳ���� ���� �켱������ ���ٸ� (�켱������ ���� ���� ������)
		if(ph->comp(ph->heapArry[GetLeftChildID(idx)], 
			ph->heapArry[GetRightChildID(idx)]) < 0)
			return GetRightChildID(idx);

		// ���� �ڽ��� �켱������ ������
		else
			return GetLeftChildID(idx);
	}
}

void HInsert(Heap* ph, HData data)
{
	// �� ����� �ε��� ����
	int idx = ph->numOfData + 1;

	// �� ��尡 ����� ��ġ�� ��Ʈ����� ��ġ�� �ƴ϶��, while�� �ݺ�
	while (idx != 1)
	{
		// �� ����� �θ� ����� �켱���� ��

		// ���� ��������, �� ����� �켱������ ���ٸ�
		if (ph->comp(data, ph->heapArry[GetParentID(idx)]) > 0)
		{
			// �θ� �� ����� ��ġ�� �̵�. ���� �ݿ�
			ph->heapArry[idx] = ph->heapArry[GetParentID(idx)];

			// �� ����� ��ġ�� �θ� ����� ��ġ�� �̵�. idx�� ����
			idx = GetParentID(idx);
		}

		// �θ� ����� �켱������ ���ٸ�, ��ġ�� ã�� ���̴� ����������.
		else
			break;
	}

	// ��ġ�� ã������ idx�� ���� �ִ´�.
	ph->heapArry[idx] = data;

	// �迭 �� 1 ����
	ph->numOfData += 1;
}

HData HDelete(Heap* ph)
{
	// ������ ������ ����. �� ���� ��ȯ�ȴ�.
	HData retData = ph->heapArry[1];

	// ���� ������ ������ ����
	HData lastData = ph->heapArry[ph->numOfData];

	// ������ ����� Index ����. �ϴ� 1���� ����
	int parentID = 1;

	// �ڽ��� Index�� ������ ����
	int childID;


	// ��Ʈ ����� �켱������ ���� �ڽ� ��带 �������� �ݺ��� ����
	while (childID = GetHiPriChildID(ph, parentID))
	{
		// GetHiPriChildID()�� ���� or ������ �ڽ��� Index�� ���� ����

		// ������ ���� �켱���� ��.
		// lastElem.pr(������ ����� �켱����)�� ph->heapArry[childID].pr(�ڽ��� �켱����)���� ���ٸ�(���� �۴ٸ�) �ڸ��� ã�����̴� break;
		if(ph->comp(lastData, ph->heapArry[childID]) > 0)
			break;

		// �ڽ��� �켱������ �� ���ٸ�, �ڽ��� data ���� �θ�� �̵���Ű��, ������ ����� ID�� �ڽ��� ID�� ����. 
		// �� �̰� ��������
		ph->heapArry[parentID] = ph->heapArry[childID];
		parentID = childID;

	}

	// �ڸ��� ã������ �ڸ��� �ִ´�.
	ph->heapArry[parentID] = lastData;

	// 1�� ���������� �� 1����
	ph->numOfData -= 1;

	// ������ �� ��ȯ.
	return retData;
}
