#ifndef __SELECTION_SORT_H__
#define __SELECTION_SORT_H__

template <class T>
class SelectionSort
{
	T* m_DataArray;
	int m_iNowSize;
	int m_iMaxSIze;

public:
	// ������
	SelectionSort(int MaxSize)
	{
		m_iNowSize = 0;
		m_iMaxSIze = MaxSize;
		m_DataArray = new T[MaxSize];
	}

	// �Ҹ���
	~SelectionSort()
	{
		delete[] m_DataArray;
	}	

	// ����
	bool Insert(T Data)
	{
		if (m_iNowSize == m_iMaxSIze)
			return false;

		m_DataArray[m_iNowSize] = Data;
		m_iNowSize++;

		return true;
	}

	// ����
	void Sort()
	{
		int LoopCount = m_iNowSize-1;
		int EmptyIndex = 0;	// �̹��� �����͸� ���� ��ġ. 

		// LoopCount��ŭ ���鼭 ������ ����
		while (LoopCount > 0)
		{
			int SearchIndex = EmptyIndex;
			int TempIndex = EmptyIndex;

			// ���� ���� ���� �ִ� �ε����� ã�´�.
			while (SearchIndex < m_iNowSize)
			{				
				if (m_DataArray[SearchIndex] < m_DataArray[TempIndex])
				{
					TempIndex = SearchIndex;
				}

				SearchIndex++;
			}

			// ��ȯ�Ѵ�.
			T Temp = m_DataArray[EmptyIndex];
			m_DataArray[EmptyIndex] = m_DataArray[TempIndex];
			m_DataArray[TempIndex] = Temp;

			LoopCount--;
			EmptyIndex++;
		}
	}


};

#endif // !__SELECTION_SORT_H__
