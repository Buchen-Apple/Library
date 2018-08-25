#include "pch.h"
#include "RingBuff\RingBuff.h"

namespace Library_Jingyu
{
#define _MyCountof(_array)		sizeof(_array) / (sizeof(_array[0]))

	// ������ ���� ���� ������ 1
	CRingBuff::CRingBuff(void)
	{
		Initial(BUF_SIZE);
	}

	// ������ ������ ������ 2
	CRingBuff::CRingBuff(int iBufferSize)
	{
		Initial(iBufferSize);
	}

	// ������ ������
	void CRingBuff::Resize(int size)
	{
		delete[] m_Buff;
		Initial(size);
	}

	// ���� �ʱ�ȭ (ť �غ�)
	void CRingBuff::Initial(int iBufferSize)
	{
		m_BuffSize = iBufferSize;
		m_Buff = new char[m_BuffSize];
		m_Front = 0;
		m_Rear = 0;

		// SRW Lock �ʱ�ȭ
		InitializeSRWLock(&sl);

		//InitializeCriticalSection(&cs);
	}

	// �Ҹ���
	CRingBuff::~CRingBuff()
	{
		//DeleteCriticalSection(&cs);
		delete[] m_Buff;
	}

	// ������ ������ ���.
	int	CRingBuff::GetBufferSize(void)
	{
		return m_BuffSize;
	}

	/////////////////////////////////////////////////////////////////////////
	// ���� ������� �뷮 ���.
	//
	// Parameters: ����.
	// Return: (int)������� �뷮.
	/////////////////////////////////////////////////////////////////////////
	int	CRingBuff::GetUseSize(void)
	{
		int TempRear = m_Rear;
		int TempFront = m_Front;

		// ����� ���� ���
		// rear�� �� ũ�ų� ���ٸ�
		if (TempRear >= TempFront)
			return TempRear - TempFront;

		// front�� �� ũ�ٸ�
		else
			return m_BuffSize - (TempFront - TempRear);
	}

	/////////////////////////////////////////////////////////////////////////
	// ���� ���ۿ� ���� �뷮 ���.
	//
	// Parameters: ����.
	// Return: (int)�����뷮.
	/////////////////////////////////////////////////////////////////////////
	int	CRingBuff::GetFreeSize(void)
	{
		int TempRear = m_Rear;
		int TempFront = m_Front;

		// ���� ���� ����ϱ�
		// rear�� �� ũ�ų� ���ٸ�
		if (TempRear >= TempFront)
			return (m_BuffSize - 1) - (TempRear - TempFront);

		// m_Front�� �� ũ�ٸ�
		else
			return (TempFront - 1) - TempRear;
	}

	/////////////////////////////////////////////////////////////////////////
	// ���� �����ͷ� �ܺο��� �ѹ濡 �а�, �� �� �ִ� ����.
	// (������ ���� ����)
	//
	// Parameters: ����.
	// Return: (int)��밡�� �뷮.
	////////////////////////////////////////////////////////////////////////
	// �� ���� ���� �� �ִ� ũ��
	int	CRingBuff::GetNotBrokenGetSize(void)
	{
		int TempRear = m_Rear;
		int TempFront = m_Front;

		// �� ���� ���� �� �ִ� ũ��
		// rear�� ũ�ų� ���ٸ�
		if (TempRear >= TempFront)
			return TempRear - TempFront;

		// front�� �� ũ�ٸ�
		else
			return m_BuffSize - (TempFront + 1);	
	}

	//�� ���� �� �� �ִ� ũ��
	int	CRingBuff::GetNotBrokenPutSize(void)
	{
		int TempRear = m_Rear;
		int TempFront = m_Front;

		//�� ���� �� �� �ִ� ũ��
		// rear�� �� ũ�ų� ���ٸ�
		if (TempRear >= TempFront)
			return m_BuffSize - (TempRear + 1);

		// front�� �� ũ�ٸ�
		else
			return (TempFront - 1) - TempRear;
	}

	// ť�� ���� �ε����� üũ�ϱ� ���� �Լ�
	int CRingBuff::NextIndex(int iIndex, int iSize)
	{
		if (iIndex + iSize >= m_BuffSize)
			return 0;

		else
			return iIndex += iSize;
	}

	/////////////////////////////////////////////////////////////////////////
	// WritePos �� ����Ÿ ����.
	//
	// Parameters: (char *)����Ÿ ������. (int)ũ��. 
	// Return: (int)���� ũ��. ť�� �� á���� -1
	/////////////////////////////////////////////////////////////////////////
	int	CRingBuff::Enqueue(char *chpData, int iSize)
	{
		int TempRear = m_Rear;
		int TempFront = m_Front;

		// �Ű����� size�� 0�̸� ����
		if (iSize == 0)
			return 0;

		// --------------------
		// 1. �ѹ��� ���� �� �ִ� ������, free ����� ��´�.
		// --------------------
		int EnqueueSize;
		int FreeSize;

		// Rear�� �� ũ�ų� ���ٸ�
		if (TempRear >= TempFront)
		{
			// �� ���� ���� �� �ִ� ������
			EnqueueSize = m_BuffSize - (TempRear + 1);

			// free ������ ���ϱ�
			FreeSize = (m_BuffSize - 1) - (TempRear - TempFront);
		}

		// front�� �� ũ�ٸ�
		else
		{
			// �� ���� ���� �� �ִ� ������
			EnqueueSize = (TempFront - 1) - TempRear;

			// free ������ ���ϱ�
			FreeSize = EnqueueSize;
		}

		// --------------------
		// 2. ���� ���������� 0�̶��, ���� ���� ��.
		// --------------------
		if (FreeSize == 0)
			return -1;

		// --------------------
		// 3. TempRear 1ĭ �̵�. ����� ī�� �� ������ �ϴ� �ൿ..
		// --------------------
		TempRear = NextIndex(TempRear, 1);


		// --------------------
		// 4. �� ���� ��ť�� �� �������
		// --------------------
		if (EnqueueSize >= iSize)
		{	
			// 1 ~ 8����Ʈ������ ĳ�����ؼ� ������ �ִ´�.
			if (iSize <= 8)
			{
				switch (iSize)
				{
				case 1:
					*(char *)&m_Buff[TempRear] = *(char *)chpData;
					break;
				case 2:
					*(short *)&m_Buff[TempRear] = *(short *)chpData;
					break;
				case 3:
					*(short *)&m_Buff[TempRear] = *(short *)chpData;
					*(char *)&m_Buff[TempRear + sizeof(short)] = *(char *)&chpData[sizeof(short)];
					break;
				case 4:
					*(int *)&m_Buff[TempRear] = *(int *)chpData;
					break;
				case 5:
					*(int *)&m_Buff[TempRear] = *(int *)chpData;
					*(char *)&m_Buff[TempRear + sizeof(int)] = *(char *)&chpData[sizeof(int)];
					break;
				case 6:
					*(int *)&m_Buff[TempRear] = *(int *)chpData;
					*(short *)&m_Buff[TempRear + sizeof(int)] = *(short *)&chpData[sizeof(int)];
					break;
				case 7:
					*(int *)&m_Buff[TempRear] = *(int *)chpData;
					*(short *)&m_Buff[TempRear + sizeof(int)] = *(short *)&chpData[sizeof(int)];
					*(char *)&m_Buff[TempRear + sizeof(int) + sizeof(short)] = *(char *)&chpData[sizeof(int) + sizeof(short)];
					break;
				case 8:
					*((__int64 *)&m_Buff[TempRear]) = *((__int64 *)chpData);
					break;
				}
			}

			// 8����Ʈ �̻��̸� memcpy�Ѵ�
			else
			{
				memcpy_s(&m_Buff[TempRear], iSize, chpData, iSize);
			}

			// ī�� �������� TempRear�����δ�.
			TempRear = NextIndex(TempRear, iSize - 1);
		}

		// --------------------
		// 4. �� ���� ��ť�� �� ���� ���
		// --------------------
		else
		{
			// memcpy_s 2ȸ
			// 1) ����, ���� ��ġ���� ���� ������ 1ȸ �ִ´�. 
			memcpy_s(&m_Buff[TempRear], EnqueueSize, chpData, EnqueueSize);

			// 2) TempRear �̵���Ų ��, �� ��ġ���� ������ �ִ´�.
			TempRear = NextIndex(TempRear, EnqueueSize);			
			memcpy_s(&m_Buff[TempRear], iSize - EnqueueSize, &chpData[EnqueueSize], iSize - EnqueueSize);

			// 3) TempRear ��ġ �ٽ� �̵�
			TempRear = NextIndex(TempRear, iSize - EnqueueSize - 1);

		}

		// --------------------
		// 5. m_Rear���� �� ����.
		// --------------------
		m_Rear = TempRear;

		return iSize;
	}

	/////////////////////////////////////////////////////////////////////////
	// ReadPos ���� ����Ÿ ������. ReadPos �̵�.
	//
	// Parameters: (char *)����Ÿ ������. (int)ũ��.
	// Return: (int)������ ũ��. ť�� ������� -1
	/////////////////////////////////////////////////////////////////////////
	int	CRingBuff::Dequeue(char *chpDest, int iSize)
	{
		int TempRear = m_Rear;
		int TempFront = m_Front;

		// �Ű����� size�� 0�̸� ����
		if (iSize == 0)
			return 0;

		// --------------------
		// 1. �ѹ��� �� �� �ִ� ������, Use ����� ��´�.
		// --------------------
		int DequeueSize;
		int UseSize;


		// Rear�� �� ũ�ų� ���ٸ�
		if (TempRear >= TempFront)
		{
			// �� ���� �� �� �ִ� ������
			DequeueSize = TempRear - TempFront;

			// Use ������ ���ϱ�
			UseSize = TempRear - TempFront;
		}

		// front�� �� ũ�ٸ�
		else
		{
			// �� ���� �� �� �ִ� ������
			DequeueSize = m_BuffSize - (TempFront + 1);

			// free ������ ���ϱ�
			UseSize = m_BuffSize - (TempFront - TempRear);
		}

		// --------------------
		// 2. ���� ����� ������ 0�̶��, ��ť�� �����Ͱ� ���°�.
		// --------------------
		if (UseSize == 0)
			return -1;

		// --------------------
		// 3. TempFont 1ĭ �̵�. ����� ī�� �� ������ �ϴ� �ൿ..
		// --------------------
		TempFront = NextIndex(TempFront, 1);


		// --------------------
		// 4. �� ���� ��ť�� �� �������
		// --------------------
		if (DequeueSize >= iSize)
		{
			// 1 ~ 8����Ʈ������ ĳ�����ؼ� ������ �ִ´�.
			if (iSize <= 8)
			{
				switch (iSize)
				{
				case 1:
					*(char *)chpDest = *(char *)&m_Buff[TempFront];
					break;
				case 2:
					*(short *)chpDest = *(short *)&m_Buff[TempFront];
					break;
				case 3:
					*(short *)chpDest = *(short *)&m_Buff[TempFront];
					*(char *)&chpDest[sizeof(short)] = *(char *)&m_Buff[TempFront + sizeof(short)];
					break;
				case 4:
					*(int *)chpDest = *(int *)&m_Buff[TempFront];
					break;
				case 5:
					*(int *)chpDest = *(int *)&m_Buff[TempFront];
					*(char *)&chpDest[sizeof(int)] = *(char *)&m_Buff[TempFront + sizeof(int)];
					break;
				case 6:
					*(int *)chpDest = *(int *)&m_Buff[TempFront];
					*(short *)&chpDest[sizeof(int)] = *(short *)&m_Buff[TempFront + sizeof(int)];
					break;
				case 7:
					*(int *)chpDest = *(int *)&m_Buff[TempFront];
					*(short *)&chpDest[sizeof(int)] = *(short *)&m_Buff[TempFront + sizeof(int)];
					*(char *)&chpDest[sizeof(int) + sizeof(short)] = *(char *)&m_Buff[TempFront + sizeof(int) + sizeof(short)];
					break;
				case 8:
					*((__int64 *)chpDest) = *((__int64 *)&m_Buff[TempFront]);
					break;
				}
			}

			// 8����Ʈ �̻��̸� memcpy�Ѵ�
			else
			{
				memcpy_s(chpDest, iSize, &m_Buff[TempFront], iSize);
			}

			// ī�� �������� TempFront�����δ�.
			TempFront = NextIndex(TempFront, iSize - 1);
		}

		// --------------------
		// 4. �� ���� ��ť�� �� ���� ���
		// --------------------
		else
		{
			// memcpy_s 2ȸ
			// 1) ����, ���� ��ġ���� ���� ������ 1ȸ ��ť
			memcpy_s(chpDest, DequeueSize, &m_Buff[TempFront], DequeueSize);

			// 2) TempFront �̵���Ų ��, �� ��ġ���� ������ �ִ´�.
			TempFront = NextIndex(TempFront, DequeueSize);
			memcpy_s(&chpDest[DequeueSize], iSize - DequeueSize, &m_Buff[TempFront], iSize - DequeueSize);

			// 3) TempFront ��ġ �ٽ� �̵�
			TempFront = NextIndex(TempFront, iSize - DequeueSize - 1);

		}

		// --------------------
		// 5. m_Front���� �� ����.
		// --------------------
		m_Front = TempFront;

		return iSize;
	}

	/////////////////////////////////////////////////////////////////////////
	// ReadPos ���� ����Ÿ �о��. ReadPos ����. (Front)
	//
	// Parameters: (char *)����Ÿ ������. (int)ũ��.
	// Return: (int)������ ũ��. ť�� ������� -1
	/////////////////////////////////////////////////////////////////////////
	int	CRingBuff::Peek(char *chpDest, int iSize)
	{
		int TempRear = m_Rear;
		int TempFront = m_Front;

		// �Ű����� size�� 0�̸� ����
		if (iSize == 0)
			return 0;

		// --------------------
		// 1. �ѹ��� �� �� �ִ� ������, Use ����� ��´�.
		// --------------------
		int DequeueSize;
		int UseSize;


		// Rear�� �� ũ�ų� ���ٸ�
		if (TempRear >= TempFront)
		{
			// �� ���� �� �� �ִ� ������
			DequeueSize = TempRear - TempFront;

			// Use ������ ���ϱ�
			UseSize = TempRear - TempFront;
		}

		// front�� �� ũ�ٸ�
		else
		{
			// �� ���� �� �� �ִ� ������
			DequeueSize = m_BuffSize - (TempFront + 1);

			// free ������ ���ϱ�
			UseSize = m_BuffSize - (TempFront - TempRear);
		}

		// --------------------
		// 2. ���� ����� ������ 0�̶��, ��ť�� �����Ͱ� ���°�.
		// --------------------
		if (UseSize == 0)
			return -1;

		// --------------------
		// 3. TempFont 1ĭ �̵�. ����� ī�� �� ������ �ϴ� �ൿ..
		// --------------------
		TempFront = NextIndex(TempFront, 1);


		// --------------------
		// 4. �� ���� ���� �� �������
		// --------------------
		if (DequeueSize >= iSize)
		{
			// 1 ~ 8����Ʈ������ ĳ�����ؼ� ������ �ִ´�.
			if (iSize <= 8)
			{
				switch (iSize)
				{
				case 1:
					*(char *)chpDest = *(char *)&m_Buff[TempFront];
					break;
				case 2:
					*(short *)chpDest = *(short *)&m_Buff[TempFront];
					break;
				case 3:
					*(short *)chpDest = *(short *)&m_Buff[TempFront];
					*(char *)&chpDest[sizeof(short)] = *(char *)&m_Buff[TempFront + sizeof(short)];
					break;
				case 4:
					*(int *)chpDest = *(int *)&m_Buff[TempFront];
					break;
				case 5:
					*(int *)chpDest = *(int *)&m_Buff[TempFront];
					*(char *)&chpDest[sizeof(int)] = *(char *)&m_Buff[TempFront + sizeof(int)];
					break;
				case 6:
					*(int *)chpDest = *(int *)&m_Buff[TempFront];
					*(short *)&chpDest[sizeof(int)] = *(short *)&m_Buff[TempFront + sizeof(int)];
					break;
				case 7:
					*(int *)chpDest = *(int *)&m_Buff[TempFront];
					*(short *)&chpDest[sizeof(int)] = *(short *)&m_Buff[TempFront + sizeof(int)];
					*(char *)&chpDest[sizeof(int) + sizeof(short)] = *(char *)&m_Buff[TempFront + sizeof(int) + sizeof(short)];
					break;
				case 8:
					*((__int64 *)chpDest) = *((__int64 *)&m_Buff[TempFront]);
					break;
				}
			}

			// 8����Ʈ �̻��̸� memcpy�Ѵ�
			else
			{
				memcpy_s(chpDest, iSize, &m_Buff[TempFront], iSize);
			}

			// ī�� �������� TempFront�����δ�.
			TempFront = NextIndex(TempFront, iSize - 1);
		}

		// --------------------
		// 4. �� ���� ���� �� ���� ���
		// --------------------
		else
		{
			// memcpy_s 2ȸ
			// 1) ����, ���� ��ġ���� ���� ������ 1ȸ ��ť
			memcpy_s(chpDest, DequeueSize, &m_Buff[TempFront], DequeueSize);

			// 2) TempFront �̵���Ų ��, �� ��ġ���� ������ �ִ´�.
			TempFront = NextIndex(TempFront, DequeueSize);
			memcpy_s(&chpDest[DequeueSize], iSize - DequeueSize, &m_Buff[TempFront], iSize - DequeueSize);

			// 3) TempFront ��ġ �ٽ� �̵�
			TempFront = NextIndex(TempFront, iSize - DequeueSize - 1);

		}

		// --------------------
		// 5. m_Front���� '���ϰ�!!' �׳� ����
		// --------------------

		return iSize;
	}


	/////////////////////////////////////////////////////////////////////////
	// ���ϴ� ���̸�ŭ �б���ġ ���� ���� / ���� ��ġ �̵�
	//
	// Parameters: ����.
	// Return: ���� ���� �� �̵��� ������.
	/////////////////////////////////////////////////////////////////////////
	int CRingBuff::RemoveData(int iSize)
	{
		int tempFront = m_Front;

		// 0����� �̵��Ϸ��� �ϸ� ����
		if (iSize == 0)
			return 0;

		// �Ű������� ���� iSize�� �� ���� �̵��� �� �ִ� ũ�⸦ ������� üũ
		int iRealMoveSize;

		if (tempFront + iSize >= m_BuffSize)
			iRealMoveSize = (tempFront + iSize) - m_BuffSize;

		else
			iRealMoveSize = tempFront + iSize;

		m_Front = iRealMoveSize;

		return iRealMoveSize;
	}

	// rear �̵�
	int	CRingBuff::MoveWritePos(int iSize)
	{
		int tempRear = m_Rear;

		// 0����� �̵��Ϸ��� �ϸ� ����
		if (iSize == 0)
			return 0;

		// �Ű������� ���� iSize�� �� ���� �̵��� �� �ִ� ũ�⸦ ������� üũ
		int iRealMoveSize;

		if (tempRear + iSize >= m_BuffSize)
			iRealMoveSize = (tempRear + iSize) - m_BuffSize;

		else
			iRealMoveSize = tempRear + iSize;

		m_Rear = iRealMoveSize;

		return iRealMoveSize;
	}

	/////////////////////////////////////////////////////////////////////////
	// ������ ��� ����Ÿ ����.
	//
	// Parameters: ����.
	// Return: ����.
	/////////////////////////////////////////////////////////////////////////
	void CRingBuff::ClearBuffer(void)
	{
		m_Front = m_Rear = 0;
	}

	/////////////////////////////////////////////////////////////////////////
	// ������ �ڿ��� ��������, 0���ε������� front������ ����
	//
	// Parameters: ����.
	// Return: 0~front������ ������ ����
	/////////////////////////////////////////////////////////////////////////
	int CRingBuff::GetFrontSize(void)
	{
		return m_Front;
	}


	/////////////////////////////////////////////////////////////////////////
	// ������ ������ ����.
	//
	// Parameters: ����.
	// Return: (char *) ���� ������.
	/////////////////////////////////////////////////////////////////////////
	char* CRingBuff::GetBufferPtr(void)
	{
		return m_Buff;
	}

	/////////////////////////////////////////////////////////////////////////
	// ������ ReadPos ������ ����.
	//
	// Parameters: ����.
	// Return: (char *) ���� ������.
	/////////////////////////////////////////////////////////////////////////
	char* CRingBuff::GetReadBufferPtr(void)
	{
		return (char*)&m_Front;
	}

	/////////////////////////////////////////////////////////////////////////
	// Fornt 1ĭ �� ��ġ�� Buff �ּ� ��ȯ
	//
	// Parameters: ����.
	// Return: (char*)&Buff[m_Front ��ĭ��]
	/////////////////////////////////////////////////////////////////////////
	char* CRingBuff::GetFrontBufferPtr(void)
	{
		return m_Buff + NextIndex(m_Front, 1);	
	}

	/////////////////////////////////////////////////////////////////////////
	// ������ WritePos ������ ����.
	//
	// Parameters: ����.
	// Return: (char *) ���� ������.
	/////////////////////////////////////////////////////////////////////////
	char* CRingBuff::GetWriteBufferPtr(void)
	{
		return (char*)&m_Rear;
	}

	/////////////////////////////////////////////////////////////////////////
	// Rear 1ĭ �� ��ġ�� Buff �ּ� ��ȯ
	//
	// Parameters: ����.
	// Return: (char*)&Buff[m_Rear ��ĭ��]
	/////////////////////////////////////////////////////////////////////////
	char* CRingBuff::GetRearBufferPtr(void)
	{
		return m_Buff + NextIndex(m_Rear, 1);
	}	

	/////////////////////////////////////////////////////////////////////////
	// ũ��Ƽ�� ���� ����
	//
	// Parameters: ����.
	// Return: ����
	/////////////////////////////////////////////////////////////////////////
	void CRingBuff::EnterLOCK()
	{
		//EnterCriticalSection(&cs);
		AcquireSRWLockExclusive(&sl);
	}


	/////////////////////////////////////////////////////////////////////////
	// ũ��Ƽ�� ���� ����
	//
	// Parameters: ����.
	// Return: ����
	/////////////////////////////////////////////////////////////////////////
	void CRingBuff::LeaveLOCK()
	{
		//LeaveCriticalSection(&cs);
		ReleaseSRWLockExclusive(&sl);
	}

}