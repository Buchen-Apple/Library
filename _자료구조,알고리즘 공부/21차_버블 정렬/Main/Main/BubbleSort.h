#ifndef __BUBBLE_SORT_H__
#define __BUBBLE_SORT_H__

template <class T>
class BubbleSort
{
	typedef void (*ActionFunc) (T Data);

	// ������ �迭
	T* m_DataArray;

	int m_iMaxSize;
	int m_iNowSize;

	// Action �Լ� ���ο��� ȣ��Ǵ� �Լ�.
	// �Լ� ������
	ActionFunc m_ActionFunc;

public:
	// ������
	BubbleSort(int ArraySize, ActionFunc fp)
	{
		m_iNowSize = 0;
		m_iMaxSize = ArraySize;
		m_DataArray = new T[ArraySize];
		m_ActionFunc = fp;
	}

	// �Ҹ���
	~BubbleSort()
	{
		delete m_DataArray;
	}


	// ����
	bool Insert(T Data)
	{
		if (m_iNowSize == m_iMaxSize)
			return false;

		m_DataArray[m_iNowSize] = Data;
		m_iNowSize++;

		return true;
	}

	// ����
	void Sort()
	{
		int LoopCount = m_iNowSize;		

		// n-1 ��ŭ ����.
		// ���� ������ �����ʹ� �� �ڸ��� �ڽ��� ���� �� ��ġ�̱� ������ n-1.
		while (LoopCount > 1)
		{
			// StartIndex���� LastIndex���� Loop
			int StartIndex = 0;
			int LastIndex = LoopCount - 1;

			while (StartIndex < LastIndex)
			{
				int NextIndex = StartIndex + 1;

				// StartIndex�� NextIndex�� �� ���� ũ�ٸ�, ���� ����.
				if (m_DataArray[StartIndex] > m_DataArray[NextIndex])
				{
					T Temp = m_DataArray[StartIndex];
					m_DataArray[StartIndex] = m_DataArray[NextIndex];
					m_DataArray[NextIndex] = Temp;
				}
				StartIndex++;
			}

			LoopCount--;
		}			
	}

	// �׼� �Լ�
	void Action()
	{
		// ��ȸ�ϸ� ���
		int TempSize = 0;
		while (TempSize < m_iNowSize)
		{
			// �ܺο��� ���� �Լ� ȣ��
			m_ActionFunc(m_DataArray[TempSize]);
			TempSize++;
		}		
	}
};

#endif // !__BUBBLE_SORT_H__
