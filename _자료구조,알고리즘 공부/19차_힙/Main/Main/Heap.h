#ifndef __HEAP_H__
#define __HEAP_H__

#define HEAP_LEN	100

template <typename T>
class Heap
{
	struct Node
	{
		T m_data;
		int m_iPr;	//�켱����
	};

	Node m_HeapArray[HEAP_LEN];
	int m_iSize;

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
	

public:
	// �ʱ�ȭ
	void Init()
	{  
		m_iSize = 0; 
	}

	// ����
	bool Insert(T data, int Pr);

	// ����
	bool Delete(T* pData);

	// ������ ����
	int Size()
	{  return m_iSize;  }
};


// ����
template <typename T>
bool Heap<T>::Insert(T data, int Pr)
{
	// ��á���� �� �̻� ������
	if (m_iSize == HEAP_LEN)
		return false;

	// �ӽ� ��ġ�� �־��
	int NowIndex = m_iSize + 1;

	Node Temp;
	Temp.m_data = data;
	Temp.m_iPr = Pr;	

	// ���� �ε����� 1�̾ƴϰ� (�� ��Ʈ�� �ƴϰ�)
	while (NowIndex != 1)
	{
		// �θ� �ε��� ����
		int ParnetIndex = GetParentIDX(NowIndex);

		// �θ��� �켱������ ������ ���ٸ� �� ��ü (�켱������ ���� �������� �켱������ ���ٰ� ����) 
		if (Pr < m_HeapArray[ParnetIndex].m_iPr)
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
	*pData = m_HeapArray[1].m_data;

	// ���� ������ ��ġ�� ���� �޾ƿ�
	Node Temp = m_HeapArray[m_iSize];

	// Temp�� ��Ʈ�� ���ٰ� ����. �ڽ����� Ÿ�� �������� �켱������ üũ�Ѵ�.
	int NowIndex = 1;

	// �ڽĳ�� ���翩�� üũ
	// ���� ���� �ڽ��� ������ �ڽ��� �ϳ��� ���°�. �����ڽ��� Index�� �� �۱⶧����.
	while (1)
	{
		int CheckIndex = 0;
		int LChildIndex = GetLeftChildIDX(NowIndex);

		// �ڽĳ�尡 0���� ���
		if (LChildIndex > m_iSize)
			break;		

		// �ڽĳ�尡 ���ʸ� ���� ���
		else if (LChildIndex == m_iSize)
		{
			CheckIndex = LChildIndex;
		}

		// �ڽĳ�尡 �� �� ���� ���
		else
		{
			int RChildIndex = GetRightChildIDX(NowIndex);

			// ������ �ڽ��� �켱������ ���� ���
			if (m_HeapArray[RChildIndex].m_iPr < m_HeapArray[LChildIndex].m_iPr)
			{
				CheckIndex = RChildIndex;
			}

			// ���� �ڽ��� �켱������ ���� ���
			else
				CheckIndex = LChildIndex;
		}	

		// ������ ����� �켱������ ���ٸ� break;
		if (Temp.m_iPr <= m_HeapArray[CheckIndex].m_iPr)
			break;

		// ã�ƿ� �ڽĳ���� �켱������ �� ���ٸ�, �� ��ü
		m_HeapArray[NowIndex] = m_HeapArray[CheckIndex];
		NowIndex = CheckIndex;
	}

	// ã�� ��ġ�� Temp�� �ִ´�.
	m_HeapArray[NowIndex] = Temp;
	m_iSize--;

	return true;
}


#endif // !__HEAP_H__
