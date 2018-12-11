#ifndef __HEAP_H__
#define __HEAP_H__

#define HEAP_LEN	100

template <typename T>
class Heap
{
	// ���Ŀ� �Լ� ������ Ÿ��
	typedef int PriorityComp(T d1, T d2);

	T m_HeapArray[HEAP_LEN];
	int m_iSize;

	// �켱���� üũ �Լ�. �Լ�������
	// ù ������ �켱������ ���� ��, 0���� ū ��
	// �� ��° ������ �켱������ ���� ��, 0���� ���� ��.
	// �켱������ ������ �� 0
	PriorityComp* Comp;

private:
	// �θ��� �ε���
	int GetParentIDX(int idx)
	{	return idx / 2;	}

	// ���� �ڽ��� �ε���
	int GetLeftChildIDX(int idx)
	{	return idx * 2; 	}

	// ������ �ڽ��� �ε���
	int GetRightChildIDX(int idx)
	{	return idx * 2 + 1; 	}

	// ���� 2���� �켱���� üũ
	int  GetHiPriChildIDX(int idx);
	

public:
	// �ʱ�ȭ
	void Init(PriorityComp TComp)
	{  
		Comp = TComp;
		m_iSize = 0; 
	}

	// ����
	bool Insert(T data);

	// ����
	bool Delete(T* pData);

	// ������ ����
	int Size()
	{  return m_iSize;  }
};

// ���� 2���� �켱���� üũ
template <typename T>
int  Heap<T>::GetHiPriChildIDX(int idx)
{
	int LeftIdx = GetLeftChildIDX(idx);
	int RightIdx = GetRightChildIDX(idx);

	// �ڽĳ�尡 0���� ���
	if (LeftIdx > m_iSize)
		return 0;

	// �ڽ� ��尡 ���� ���ۿ� ���� ���
	else if (LeftIdx == m_iSize)
		return LeftIdx;

	// �ڽĳ�尡 �� �� �ִ� ���
	else
	{
		// ������ �ڽ��� �켱������ ���� ���
		/*if (m_HeapArray[LeftIdx].m_iPr > m_HeapArray[RightIdx].m_iPr)
			return RightIdx;*/

		if (Comp(m_HeapArray[LeftIdx], m_HeapArray[RightIdx]) < 0)
			return RightIdx;

		// ���� �ڽ��� �켱������ ���� ���
		else
			return LeftIdx;
	}
}

// ����
template <typename T>
bool Heap<T>::Insert(T data)
{
	// ��á���� �� �̻� ������
	if (m_iSize == HEAP_LEN)
		return false;

	// �ӽ÷� ���� �������� ���ٰ� ����
	int NowIndex = m_iSize + 1;

	T Temp = data;

	// ���� �ε����� 1�̾ƴϰ� (�� ��Ʈ�� �ƴϰ�)
	while (NowIndex != 1)
	{
		// �θ� �ε��� ����
		int ParnetIndex = GetParentIDX(NowIndex);

		// �θ��� �켱������ ������ ���ٸ� �� ��ü (�켱������ ���� �������� �켱������ ���ٰ� ����) 
		//if (Pr < m_HeapArray[ParnetIndex].m_iPr)
		if (Comp(data, m_HeapArray[ParnetIndex]) > 0)
		{
			// �θ� �� ��ġ�� �̵�(���� �� ��ġ�� ����ֱ� ������ �ٷ� �̵����ѵ� ��)
			m_HeapArray[NowIndex] = m_HeapArray[ParnetIndex];

			// �θ� ���� ���
			NowIndex = ParnetIndex;

			// �θ� �ε��� �ٽ� ����
			ParnetIndex = GetParentIDX(NowIndex);
		}

		else
			break;
	}

	// ã�� ��ġ�� ���� ����
	m_HeapArray[NowIndex] = Temp;
	m_iSize++;

	return true;
}

// ����
template <typename T>
bool Heap<T>::Delete(T* pData)
{
	// �����Ͱ� ������ �� �̻� ����
	if (m_iSize == 0)
		return false;

	// ������ ������ �޾Ƶα�
	// ���� ������ ���� ���� ������ ����.
	*pData = m_HeapArray[1];

	// ���� ������ ��ġ�� ���� �޾ƿ�
	T Temp = m_HeapArray[m_iSize];

	// Temp�� ��Ʈ�� ���ٰ� ����. �ڽ����� Ÿ�� �������� �켱������ üũ�Ѵ�.
	int ParentIdx = 1;
	int ChildIdx;

	// �ڽĳ�� ���翩�� üũ
	while (ChildIdx = GetHiPriChildIDX(ParentIdx))
	{
		// ������ ����� �켱������ ���ų� ���ٸ� break;
		//if (Temp.m_iPr <= m_HeapArray[ChildIdx].m_iPr)
		if (Comp(Temp, m_HeapArray[ChildIdx]) >= 0)
			break;

		// ã�ƿ� �ڽĳ���� �켱������ �� ���ٸ�, �� ��ü
		m_HeapArray[ParentIdx] = m_HeapArray[ChildIdx];
		ParentIdx = ChildIdx;
	}

	// ã�� ��ġ�� Temp�� �ִ´�.
	m_HeapArray[ParentIdx] = Temp;
	m_iSize--;

	return true;
}


#endif // !__HEAP_H__
