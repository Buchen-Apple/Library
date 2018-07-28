#include "stdafx.h"
#include "RingBuff.h"
#include <string.h>

namespace Library_Jingyu
{
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
		// rear�� �� ũ�ٸ�
		if (TempRear > TempFront)
			return TempRear - TempFront;

		// ���� ���ٸ�
		else if (TempRear == TempFront)
			return 0;

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
		// rear�� �� ũ�ٸ�
		if (TempRear > TempFront)
			return (m_BuffSize - 1) - (TempRear - TempFront);

		// ���� ���ٸ�
		else if (TempRear == TempFront)
			return m_BuffSize - 1;

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
	int	CRingBuff::GetNotBrokenGetSize(void)
	{
		int TempRear = m_Rear;
		int TempFront = m_Front;

		// �� ���� ���� �� �ִ� ũ��
		// m_Rear�� m_Front�� ������, �� ������̴� ������ ����.
		if (TempRear == TempFront)
			return 0;

		// rear�� �� ũ�ٸ�.
		if (TempRear > TempFront)
			return TempRear - TempFront;

		// front�� �� ũ�ٸ�
		else
			return m_BuffSize - (TempFront + 1);	
	}

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

		// ���� ��ť ����� ������ ����
		int iRealEnqueueSize = 0;

		// ��ť �� ����� ������ ���� iRealCpySize.
		int iRealCpySize = 0;

		while (iSize > 0)
		{
			// Rear 1ĭ �̵�
			TempRear = NextIndex(TempRear, 1);		

			// ť ��á���� üũ
			if (TempRear == TempFront)
				return -1;

			// ���� ���� �� �ִ� ����� ã�´�.
			if (TempRear >= TempFront)
				iRealEnqueueSize = m_BuffSize - TempRear;
			else
				iRealEnqueueSize = TempFront - TempRear;

			if (iRealEnqueueSize > iSize)
				iRealEnqueueSize = iSize;

			// �޸� ����
			memcpy(&m_Buff[TempRear], chpData + iRealCpySize, iRealEnqueueSize);

			// rear�� ��ġ �̵�
			TempRear = NextIndex(TempRear, iRealEnqueueSize - 1);

			iRealCpySize += iRealEnqueueSize;
			iSize -= iRealEnqueueSize;			
		}

		m_Rear = TempRear;

		return iRealCpySize;
	}

	/////////////////////////////////////////////////////////////////////////
	// ReadPos ���� ����Ÿ ������. ReadPos �̵�.
	//
	// Parameters: (char *)����Ÿ ������. (int)ũ��.
	// Return: (int)������ ũ��. ť�� ������� -1
	/////////////////////////////////////////////////////////////////////////
	int	CRingBuff::Dequeue(char *chpDest, int iSize)
	{
		int TempFront = m_Front;
		int TempRear = m_Rear;

		// �Ű����� size�� 0�̸� ����
		if (iSize == 0)
			return 0;

		// ���� Dequeue �� ������
		int iRealDequeueSIze = 0;

		// Dequeue �� ����� ������ ���� iRealCpySize.
		int iRealCpySize = 0;

		while (iSize > 0)
		{
			// ť ����� üũ
			if (TempFront == TempRear)
				return -1;

			// �̹��� Dequeue�� �� �ִ� ����� ã�´�.
			// rear�� �� ũ�ٸ�.
			if (TempRear > TempFront)
				iRealDequeueSIze = TempRear - TempFront;

			// front�� �� ũ�ٸ�
			else
				iRealDequeueSIze = m_BuffSize - (TempFront + 1);

			if (iRealDequeueSIze > iSize)
				iRealDequeueSIze = iSize;

			// Front 1ĭ ������ �̵�
			TempFront = NextIndex(TempFront, 1);

			// �޸� ����
			if (iRealDequeueSIze == 0)
				iRealDequeueSIze = 1;

			memcpy_s(chpDest + iRealCpySize, GetFreeSize(), &m_Buff[TempFront], iRealDequeueSIze);
			//memcpy(chpDest + iRealCpySize, &m_Buff[TempFront], iRealDequeueSIze);

			// ��ť�� ��ŭ m_Front�̵�
			TempFront = NextIndex(TempFront, iRealDequeueSIze - 1);

			iRealCpySize += iRealDequeueSIze;
			iSize -= iRealDequeueSIze;
		}

		m_Front = TempFront;

		return iRealCpySize;
	}

	/////////////////////////////////////////////////////////////////////////
	// ReadPos ���� ����Ÿ �о��. ReadPos ����. (Front)
	//
	// Parameters: (char *)����Ÿ ������. (int)ũ��.
	// Return: (int)������ ũ��. ť�� ������� -1
	/////////////////////////////////////////////////////////////////////////
	int	CRingBuff::Peek(char *chpDest, int iSize)
	{
		// Peek �� ����� ������ ���� iRealCpySize.
		int TempFront = m_Front;
		int TempRear = m_Rear;
		int iRealCpySize = 0;

		// �Ű����� size�� 0�̸� ����
		if (iSize == 0)
			return 0;

		// ���� Peek �� ������
		int iRealPeekSIze = 0;		

		while (iSize > 0)
		{
			// ť ����� üũ
			if (TempFront == TempRear)
				return -1;

			// �̹��� Peek�� �� �ִ� ����� ã�´�.
			// rear�� �� ũ�ٸ�.
			if (TempRear > TempFront)
				iRealPeekSIze = TempRear - TempFront;

			// front�� �� ũ�ٸ�
			else
				iRealPeekSIze = m_BuffSize - (TempFront + 1);

			if (iRealPeekSIze > iSize)
				iRealPeekSIze = iSize;

			// �ӽ� Front 1ĭ ������ �̵�
			TempFront = NextIndex(TempFront, 1);
			
			// �޸� ����
			if (iRealPeekSIze == 0)
				iRealPeekSIze = 1;

			memcpy(chpDest + iRealCpySize, &m_Buff[TempFront], iRealPeekSIze);

			// ��ť�� ��ŭ �ӽ� m_Front�̵�
			TempFront = NextIndex(TempFront, iRealPeekSIze - 1);
			
			iRealCpySize += iRealPeekSIze;
			iSize -= iRealPeekSIze;
		}

		return iRealCpySize;
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

		if (iSize < 0)
			int abc = 10;

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