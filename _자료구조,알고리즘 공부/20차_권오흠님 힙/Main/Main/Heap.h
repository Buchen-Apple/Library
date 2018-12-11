#ifndef __HEAP_H__
#define __HEAP_H__

#define LEN	100

template <typename T>
class Heap
{
	T m_DataArray[LEN];
	int m_iSize;

	// �ִ� �� ����. true�� �ִ� ��. false�� �ּ� ��
	bool m_MaxHeapFlag;

private:
	// Max_Heapify
	// Heapify�� ������ ��Ʈ�� �ε����� ���� �ִ� ����� ������
	void Max_Heapify(int Idx, int Size);

	// Min_Heapify
	// Heapify�� ������ ��Ʈ�� �ε����� ������.
	void Min_Heapify(int Idx, int Size);

	// �θ� �ε���
	int ParentIdx(int Idx)
	{
		return Idx / 2;
	}

	// �����ڽ� �ε���
	int LChildIdx(int Idx)
	{
		return Idx * 2;
	}

	// ������ �ڽ� �ε���
	int RChildIdx(int Idx)
	{
		return Idx * 2 + 1;
	}

	// Max_Heap���� ����
	void MaxHeap();

	// Min_Heap���� ����
	void MinHeap();

public:
	// �ʱ�ȭ
	// ����Ʈ : �ִ� ��
	void Init(bool MaxHeapFlag = true)
	{
		m_MaxHeapFlag = MaxHeapFlag;
		m_iSize = 0;
	}

	// ����
	bool Insert(T Data);	

	// ����
	bool Delete(T* pData);	

	// �������� ����
	void Sort_asce();

	// �������� ����
	void Sort_desc();
};


// ����
template <typename T>
bool Heap<T>::Insert(T Data)
{
	if (m_iSize == LEN - 1)
		return false;
	
	m_iSize++;
	m_DataArray[m_iSize] = Data;

	if (m_MaxHeapFlag == true)
		MaxHeap();
	else
		MinHeap();

	return true;
}

// ����
template <typename T>
bool Heap<T>::Delete(T* pData)
{
	if (m_iSize == 0)
		return false;

	*pData = m_DataArray[1];
	m_DataArray[1] = m_DataArray[m_iSize];
	m_iSize--;

	if (m_MaxHeapFlag == true)
		MaxHeap();
	else
		MinHeap();

	return true;
}

// Max_Heapify
template <typename T>
void Heap<T>::Max_Heapify(int Idx, int Size)
{
	int LChild = LChildIdx(Idx);
	int RChild = RChildIdx(Idx);

	// �ڽ��� ���� ���
	while (LChild <= Size)
	{
		// ���� �ڽĸ� �ִ� ���
		if (LChild == Size)
		{
			// ���� �����ڽĺ��� ũ�ų� ���ٸ� ���� ��
			if (m_DataArray[LChild] <= m_DataArray[Idx])
				break;

			// ���� �ڽİ� ���� ������ Swap
			T Temp = m_DataArray[Idx];
			m_DataArray[Idx] = m_DataArray[LChild];
			m_DataArray[LChild] = Temp;
			Idx = LChild;
		}

		// �� �� �ִ� ���
		else
		{
			// 2���� �ڽ� �� �� ū �ڽ��� �ε����� �˾ƿ´�.
			int BigChild;
			if (m_DataArray[LChild] > m_DataArray[RChild])
				BigChild = LChild;

			else
				BigChild = RChild;

			// �� �ڽĺ��� ���� ũ�ų� ���ٸ� ���� ��
			if (m_DataArray[BigChild] <= m_DataArray[Idx])
				break;		

			// �װ� �ƴ϶�� �� ū �ڽİ� ���� swap
			T Temp = m_DataArray[Idx];
			m_DataArray[Idx] = m_DataArray[BigChild];
			m_DataArray[BigChild] = Temp;
			Idx = BigChild;
		}

		LChild = LChildIdx(Idx);
		RChild = RChildIdx(Idx);
	}	
}

// Min_Heapify
// Heapify�� ������ ��Ʈ�� �ε����� ������.
template <typename T>
void Heap<T>::Min_Heapify(int Idx, int Size)
{
	int LChild = LChildIdx(Idx);
	int RChild = RChildIdx(Idx);

	// �ڽ��� ���� ���
	while (LChild <= Size)
	{
		// ���� �ڽĸ� �ִ� ���
		if (LChild == Size)
		{
			// ���� �����ڽĺ��� �۰ų� ���ٸ� ���� ��
			if (m_DataArray[LChild] >= m_DataArray[Idx])
				break;

			// ���� �ڽİ� ���� ������ Swap
			T Temp = m_DataArray[Idx];
			m_DataArray[Idx] = m_DataArray[LChild];
			m_DataArray[LChild] = Temp;
			Idx = LChild;
		}

		// �� �� �ִ� ���
		else
		{
			// 2���� �ڽ� �� �� ���� �ڽ��� �ε����� �˾ƿ´�.
			int SmalChild;
			if (m_DataArray[LChild] < m_DataArray[RChild])
				SmalChild = LChild;

			else
				SmalChild = RChild;

			// �ڽĺ��� ���� �۰ų� ���ٸ� ���� ��
			if (m_DataArray[SmalChild] >= m_DataArray[Idx])
				break;

			// �װ� �ƴ϶�� �� ���� �ڽİ� ���� swap
			T Temp = m_DataArray[Idx];
			m_DataArray[Idx] = m_DataArray[SmalChild];
			m_DataArray[SmalChild] = Temp;
			Idx = SmalChild;
		}

		LChild = LChildIdx(Idx);
		RChild = RChildIdx(Idx);
	}
}

// Max_Heap���� ����
template <typename T>
void Heap<T>::MaxHeap()
{
	// ���ʷ� �ڽ��� �ִ� ����, 2/n�̴�
	int StartIdx = m_iSize / 2;

	// �������� ���ư���, 1�̵ɶ� ���� ����.
	while (StartIdx >= 1)
	{
		Max_Heapify(StartIdx, m_iSize);
		--StartIdx;
	}
}

// Min_Heap���� ����
template <typename T>
void Heap<T>::MinHeap()
{
	// ���ʷ� �ڽ��� �ִ� ����, 2/n�̴�
	int StartIdx = m_iSize / 2;

	// �������� ���ư���, 1�̵ɶ� ���� ����.
	while (StartIdx >= 1)
	{
		Min_Heapify(StartIdx, m_iSize);
		--StartIdx;
	}
}

// �������� ����
template <typename T>
void Heap<T>::Sort_asce()
{
	// �ּ� ���� ���, �ִ� ������ ���� �� �������� ����
	if (m_MaxHeapFlag == false)
		MaxHeap();

	// �ִ� �� ���¿��� �������� ����
	int TempSize = m_iSize;
	while (TempSize > 1)
	{
		// 1. ��Ʈ��, ���� ������ �迭�� ���� Exchange
		T Temp = m_DataArray[1];
		m_DataArray[1] = m_DataArray[TempSize];
		m_DataArray[TempSize] = Temp;

		// 2. �� ����� 1 �پ������� ����
		TempSize--;

		// 3. ��Ʈ�� �������� Max_Heapify
		Max_Heapify(1, TempSize);
	}	
}

// �������� ����
template <typename T>
void Heap<T>::Sort_desc()
{
	// �ִ� ���� ���, �ּ� ������ ���� �� �������� ����
	if (m_MaxHeapFlag == false)
		MinHeap();

	// �ּ� �� ���¿��� �������� ����
	int TempSize = m_iSize;
	while (TempSize > 1)
	{
		// 1. ��Ʈ��, ���� ������ �迭�� ���� Exchange
		T Temp = m_DataArray[1];
		m_DataArray[1] = m_DataArray[TempSize];
		m_DataArray[TempSize] = Temp;

		// 2. �� ����� 1 �پ������� ����
		TempSize--;

		// 3. ��Ʈ�� �������� Min_Heapify
		Min_Heapify(1, TempSize);
	}

}

#endif // !__HEAP_H__
