#include "stdafx.h"
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <stdio.h>
#include "CRingBuffer.h"
//
CRingBuffer::CRingBuffer(int _iSize)
	: m_iBufferSize(_iSize)
	, m_iFront(0)
	, m_iRear(0)
	, m_pBuffer(NULL)
{
	Resize(_iSize);
	InitializeCriticalSection(&m_Cri);
}
CRingBuffer::~CRingBuffer()
{
	if (m_pBuffer)
	{
		delete[] m_pBuffer;
	}
	DeleteCriticalSection(&m_Cri);
}
//
bool CRingBuffer::Resize(int _iSize)
{
	if (m_pBuffer)
	{
		delete[] m_pBuffer;
	}

	if (2 > _iSize)
	{
		//�ּ� ũ�� 2
		_iSize = 2;
		m_iBufferSize = _iSize;
	}

	m_pBuffer = new char[_iSize];

	//���� �� �ʱ�ȭ
	m_iFront = 0;
	m_iRear = 0;
	m_iBufferSize_Decrement = m_iBufferSize - 1;

	return true;
}
int CRingBuffer::GetBufferSize()
{
	return m_iBufferSize;
}
int CRingBuffer::GetUseSize()
{
	int iFront;
	int iRear;

	iFront = m_iFront;
	iRear = m_iRear;

	//�迭�� ������
	if (iRear >= iFront)
	{
		return iRear - iFront;
	}

	//������ ���κ��� �迭�� ȸ��
	return m_iBufferSize - (iFront - iRear);
}
int CRingBuffer::GetFreeSize()
{
	int iFront;
	int iRear;

	iFront = m_iFront;
	iRear = m_iRear;

	//�迭�� ������
	if (iRear >= iFront)
	{
		return m_iBufferSize_Decrement - (iRear - iFront);
	}

	//������ ���κ��� �迭�� ȸ��
	return iFront - iRear - 1;
}
int CRingBuffer::GetUsePieceSize()
{
	int iFront;
	int iRear;

	iFront = m_iFront;
	iRear = m_iRear;

	//�迭�� ������
	if (iRear >= iFront)
	{
		return iRear - iFront;
	}

	//������ ���κ��� �迭�� ȸ��
	return m_iBufferSize - iFront;
}
int CRingBuffer::GetFreePieceSize()
{
	int iFront;
	int iRear;

	iFront = m_iFront;
	iRear = m_iRear;

	//�迭�� ������
	if (iRear >= iFront)
	{
		return m_iBufferSize - iRear;
	}

	//������ ���κ��� �迭�� ȸ��
	return iFront - iRear - 1;
}
int CRingBuffer::Enqueue(char *pInBuff, int _iSize)
{
	if (!_iSize)
	{
		//������ 0����Ʈ
		return 0;
	}

	//////
	//������
	int iFront;
	int iRear;

	iFront = m_iFront;
	iRear = m_iRear;

	//////
	//��������, �������� Ȯ��
	int iFreeSize;
	int iPieceSize;
	if (iRear >= iFront)
	{
		iFreeSize = m_iBufferSize_Decrement - (iRear - iFront);
		iPieceSize = m_iBufferSize - iRear;
	}
	else
	{
		iFreeSize = iFront - iRear - 1;
		iPieceSize = iFreeSize;
	}

	if (0 == iFreeSize)
	{
		//��������
		return 0;
	}

	//////
	//ī�� ������ ũ�� Ȯ��
	int iCpySize;
	if (iFreeSize >= _iSize)
	{
		iCpySize = _iSize;
	}
	else
	{
		iCpySize = iFreeSize;
	}

	//������ ����
	if (iPieceSize >= iCpySize) //����� ����
	{
		if (8 >= iCpySize)
		{
			//8 ����Ʈ ���� �������� �ذ�
			switch (iCpySize)
			{
			case 1:
				*((char *)&m_pBuffer[iRear]) = *(char *)pInBuff;
				break;
			case 2:
				*((short *)&m_pBuffer[iRear]) = *(short *)pInBuff;
				break;
			case 3:
				*((short *)&m_pBuffer[iRear]) = *(short *)pInBuff;
				*((char *)&m_pBuffer[iRear + sizeof(short)]) = *(char *)&pInBuff[sizeof(short)];
				break;
			case 4:
				*((int *)&m_pBuffer[iRear]) = *(int *)pInBuff;
				break;
			case 5:
				*((int *)&m_pBuffer[iRear]) = *(int *)pInBuff;
				*((char *)&m_pBuffer[iRear + sizeof(int)]) = *(char *)&pInBuff[sizeof(int)];
				break;
			case 6:
				*((int *)&m_pBuffer[iRear]) = *(int *)pInBuff;
				*((short *)&m_pBuffer[iRear + sizeof(int)]) = *(short *)&pInBuff[sizeof(int)];
				break;
			case 7:
				*((int *)&m_pBuffer[iRear]) = *(int *)pInBuff;
				*((short *)&m_pBuffer[iRear + sizeof(int)]) = *(short *)&pInBuff[sizeof(int)];
				*((char *)&m_pBuffer[iRear + sizeof(int) + sizeof(short)]) = *(char *)&pInBuff[sizeof(int) + sizeof(short)];
				break;
			case 8:
				*((__int64 *)&m_pBuffer[iRear]) = *(__int64 *)pInBuff;
				break;
			}
		}
		else
		{
			memcpy_s(&m_pBuffer[iRear], iCpySize, pInBuff, iCpySize);
		}
	}
	else //������ ����
	{
		int iSubSize;

		iSubSize = iCpySize - iPieceSize;
		
		if (8 >= iPieceSize)
		{
			//8 ����Ʈ ���� �������� �ذ�
			switch (iPieceSize)
			{
			case 1:
				*((char *)&m_pBuffer[iRear]) = *(char *)pInBuff;
				break;
			case 2:
				*((short *)&m_pBuffer[iRear]) = *(short *)pInBuff;
				break;
			case 3:
				*((short *)&m_pBuffer[iRear]) = *(short *)pInBuff;
				*((char *)&m_pBuffer[iRear + sizeof(short)]) = *(char *)&pInBuff[sizeof(short)];
				break;
			case 4:
				*((int *)&m_pBuffer[iRear]) = *(int *)pInBuff;
				break;
			case 5:
				*((int *)&m_pBuffer[iRear]) = *(int *)pInBuff;
				*((char *)&m_pBuffer[iRear + sizeof(int)]) = *(char *)&pInBuff[sizeof(int)];
				break;
			case 6:
				*((int *)&m_pBuffer[iRear]) = *(int *)pInBuff;
				*((short *)&m_pBuffer[iRear + sizeof(int)]) = *(short *)&pInBuff[sizeof(int)];
				break;
			case 7:
				*((int *)&m_pBuffer[iRear]) = *(int *)pInBuff;
				*((short *)&m_pBuffer[iRear + sizeof(int)]) = *(short *)&pInBuff[sizeof(int)];
				*((char *)&m_pBuffer[iRear + sizeof(int) + sizeof(short)]) = *(char *)&pInBuff[sizeof(int) + sizeof(short)];
				break;
			case 8:
				*((__int64 *)&m_pBuffer[iRear]) = *(__int64 *)pInBuff;
				break;
			}
		}
		else
		{
			memcpy_s(&m_pBuffer[iRear], iPieceSize, pInBuff, iPieceSize);
		}
		
		if (7 >= iSubSize)
		{
			//8 ����Ʈ ���� �������� �ذ�
			switch (iSubSize)
			{
			case 1:
				*((char *)m_pBuffer) = *(char *)&pInBuff[iPieceSize];
				break;
			case 2:
				*((short *)m_pBuffer) = *(short *)&pInBuff[iPieceSize];
				break;
			case 3:
				*((short *)m_pBuffer) = *(short *)&pInBuff[iPieceSize];
				*((char *)&m_pBuffer[sizeof(short)]) = *(char *)&pInBuff[iPieceSize + sizeof(short)];
				break;
			case 4:
				*((int *)m_pBuffer) = *(int *)&pInBuff[iPieceSize];
				break;
			case 5:
				*((int *)m_pBuffer) = *(int *)&pInBuff[iPieceSize];
				*((char *)&m_pBuffer[sizeof(int)]) = *(char *)&pInBuff[iPieceSize + sizeof(int)];
				break;
			case 6:
				*((int *)m_pBuffer) = *(int *)&pInBuff[iPieceSize];
				*((short *)&m_pBuffer[sizeof(int)]) = *(short *)&pInBuff[iPieceSize + sizeof(int)];
				break;
			case 7:
				*((int *)m_pBuffer) = *(int *)&pInBuff[iPieceSize];
				*((short *)&m_pBuffer[sizeof(int)]) = *(short *)&pInBuff[iPieceSize + sizeof(int)];
				*((char *)&m_pBuffer[sizeof(int) + sizeof(short)]) = *(char *)&pInBuff[iPieceSize + sizeof(int) + sizeof(short)];
				break;
			case 8:
				*((__int64 *)m_pBuffer) = *(__int64 *)&pInBuff[iPieceSize];
				break;
			}
		}
		else
		{
			memcpy_s(m_pBuffer, iSubSize, &pInBuff[iPieceSize], iSubSize);
		}
	}

	iRear += iCpySize;
	if (m_iBufferSize <= iRear)
	{
		iRear -= m_iBufferSize;
	}
	m_iRear = iRear;

	return iCpySize;
}
int CRingBuffer::Dequeue(char *pOutBuff, int _iSize)
{
	if (!_iSize)
	{
		//������ 0����Ʈ
		return 0;
	}

	//////
	//������
	int iFront;
	int iRear;

	iFront = m_iFront;
	iRear = m_iRear;

	//////
	//������, �������� Ȯ��
	int iUseSize;
	int iPieceSize;
	if (iRear >= iFront)
	{
		iUseSize = iRear - iFront;
		iPieceSize = iUseSize;
	}
	else
	{
		iUseSize = m_iBufferSize - (iFront - iRear);
		iPieceSize = m_iBufferSize - iFront;
	}

	if (0 == iUseSize)
	{
		//������ ����
		return 0;
	}

	//////
	//ī�� ������ ũ�� Ȯ��
	int iCpySize;
	if (iUseSize >= _iSize)
	{
		iCpySize = _iSize;
	}
	else
	{
		iCpySize = iUseSize;
	}

	//������ ����
	if (iPieceSize >= iCpySize) //����� ����
	{
		if (8 >= iCpySize)
		{
			//8 ����Ʈ ���� �������� �ذ�
			switch (iCpySize)
			{
			case 1:
				*(char *)pOutBuff = *((char *)&m_pBuffer[iFront]);
				break;
			case 2:
				*(short *)pOutBuff = *((short *)&m_pBuffer[iFront]);
				break;
			case 3:
				*(short *)pOutBuff = *((short *)&m_pBuffer[iFront]);
				*(char *)&pOutBuff[sizeof(short)] = *((char *)&m_pBuffer[iFront + sizeof(short)]);
				break;
			case 4:
				*(int *)pOutBuff = *((int *)&m_pBuffer[iFront]);
				break;
			case 5:
				*(int *)pOutBuff = *((int *)&m_pBuffer[iFront]);
				*(char *)&pOutBuff[sizeof(int)] = *((char *)&m_pBuffer[iFront + sizeof(int)]);
				break;
			case 6:
				*(int *)pOutBuff = *((int *)&m_pBuffer[iFront]);
				*(short *)&pOutBuff[sizeof(int)] = *((short *)&m_pBuffer[iFront + sizeof(int)]);
				break;
			case 7:
				*(int *)pOutBuff = *((int *)&m_pBuffer[iFront]);
				*(short *)&pOutBuff[sizeof(int)] = *((short *)&m_pBuffer[iFront + sizeof(int)]);
				*(char *)&pOutBuff[sizeof(int) + sizeof(short)] = *((char *)&m_pBuffer[iFront + sizeof(int) + sizeof(short)]);
				break;
			case 8:
				*(__int64 *)pOutBuff = *((__int64 *)&m_pBuffer[iFront]);
				break;
			}
		}
		else
		{
			memcpy_s(pOutBuff, iCpySize, &m_pBuffer[iFront], iCpySize);
		}
	}
	else //������ ����
	{
		int iSubSize;

		iSubSize = iCpySize - iPieceSize;

		if (8 >= iPieceSize)
		{
			//8 ����Ʈ ���� �������� �ذ�
			switch (iPieceSize)
			{
			case 1:
				*(char *)pOutBuff = *((char *)&m_pBuffer[iFront]);
				break;
			case 2:
				*(short *)pOutBuff = *((short *)&m_pBuffer[iFront]);
				break;
			case 3:
				*(short *)pOutBuff = *((short *)&m_pBuffer[iFront]);
				*(char *)&pOutBuff[sizeof(short)] = *((char *)&m_pBuffer[iFront + sizeof(short)]);
				break;
			case 4:
				*(int *)pOutBuff = *((int *)&m_pBuffer[iFront]);
				break;
			case 5:
				*(int *)pOutBuff = *((int *)&m_pBuffer[iFront]);
				*(char *)&pOutBuff[sizeof(int)] = *((char *)&m_pBuffer[iFront + sizeof(int)]);
				break;
			case 6:
				*(int *)pOutBuff = *((int *)&m_pBuffer[iFront]);
				*(short *)&pOutBuff[sizeof(int)] = *((short *)&m_pBuffer[iFront + sizeof(int)]);
				break;
			case 7:
				*(int *)pOutBuff = *((int *)&m_pBuffer[iFront]);
				*(short *)&pOutBuff[sizeof(int)] = *((short *)&m_pBuffer[iFront + sizeof(int)]);
				*(char *)&pOutBuff[sizeof(int) + sizeof(short)] = *((char *)&m_pBuffer[iFront + sizeof(int) + sizeof(short)]);
				break;
			case 8:
				*(__int64 *)pOutBuff = *((__int64 *)&m_pBuffer[iFront]);
				break;
			}
		}
		else
		{
			memcpy_s(pOutBuff, iPieceSize, &m_pBuffer[iFront], iPieceSize);
		}

		if (8 >= iSubSize)
		{
			//8 ����Ʈ ���� �������� �ذ�
			switch (iSubSize)
			{
			case 1:
				*(char *)&pOutBuff[iPieceSize] = *((char *)m_pBuffer);
				break;
			case 2:
				*(short *)&pOutBuff[iPieceSize] = *((short *)m_pBuffer);
				break;
			case 3:
				*(short *)&pOutBuff[iPieceSize] = *((short *)m_pBuffer);
				*(char *)&pOutBuff[iPieceSize + sizeof(short)] = *((char *)&m_pBuffer[sizeof(short)]);
				break;
			case 4:
				*(int *)&pOutBuff[iPieceSize] = *((int *)m_pBuffer);
				break;
			case 5:
				*(int *)&pOutBuff[iPieceSize] = *((int *)m_pBuffer);
				*(char *)&pOutBuff[iPieceSize + sizeof(int)] = *((char *)&m_pBuffer[sizeof(int)]);
				break;
			case 6:
				*(int *)&pOutBuff[iPieceSize] = *((int *)m_pBuffer);
				*(short *)&pOutBuff[iPieceSize + sizeof(int)] = *((short *)&m_pBuffer[sizeof(int)]);
				break;
			case 7:
				*(int *)&pOutBuff[iPieceSize] = *((int *)m_pBuffer);
				*(short *)&pOutBuff[iPieceSize + sizeof(int)] = *((short *)&m_pBuffer[sizeof(int)]);
				*(char *)&pOutBuff[iPieceSize + sizeof(int) + sizeof(short)] = *((char *)&m_pBuffer[sizeof(int) + sizeof(short)]);
				break;
			case 8:
				*(__int64 *)&pOutBuff[iPieceSize] = *((__int64 *)m_pBuffer);
				break;
			}
		}
		else
		{
			memcpy_s(&pOutBuff[iPieceSize], iSubSize, m_pBuffer, iSubSize);
		}
	}

	iFront += iCpySize;
	if (m_iBufferSize <= iFront)
	{
		iFront -= m_iBufferSize;
	}
	m_iFront = iFront;

	return iCpySize;
}
int CRingBuffer::Peek(char *pOutBuff, int _iSize)
{
	if (!_iSize)
	{
		//������ 0����Ʈ
		return 0;
	}

	//////
	//������
	int iFront;
	int iRear;

	iFront = m_iFront;
	iRear = m_iRear;

	//////
	//������, �������� Ȯ��
	int iUseSize;
	int iPieceSize;
	if (iRear >= iFront)
	{
		iUseSize = iRear - iFront;
		iPieceSize = iUseSize;
	}
	else
	{
		iUseSize = m_iBufferSize - (iFront - iRear);
		iPieceSize = m_iBufferSize - iFront;
	}

	if (0 == iUseSize)
	{
		//������ ����
		return 0;
	}

	//////
	//ī�� ������ ũ�� Ȯ��
	int iCpySize;
	if (iUseSize >= _iSize)
	{
		iCpySize = _iSize;
	}
	else
	{
		iCpySize = iUseSize;
	}

	//������ ����
	if (iPieceSize >= iCpySize) //����� ����
	{
		if (8 >= iCpySize)
		{
			//8 ����Ʈ ���� �������� �ذ�
			switch (iCpySize)
			{
			case 1:
				*(char *)pOutBuff = *((char *)&m_pBuffer[iFront]);
				break;
			case 2:
				*(short *)pOutBuff = *((short *)&m_pBuffer[iFront]);
				break;
			case 3:
				*(short *)pOutBuff = *((short *)&m_pBuffer[iFront]);
				*(char *)&pOutBuff[sizeof(short)] = *((char *)&m_pBuffer[iFront + sizeof(short)]);
				break;
			case 4:
				*(int *)pOutBuff = *((int *)&m_pBuffer[iFront]);
				break;
			case 5:
				*(int *)pOutBuff = *((int *)&m_pBuffer[iFront]);
				*(char *)&pOutBuff[sizeof(int)] = *((char *)&m_pBuffer[iFront + sizeof(int)]);
				break;
			case 6:
				*(int *)pOutBuff = *((int *)&m_pBuffer[iFront]);
				*(short *)&pOutBuff[sizeof(int)] = *((short *)&m_pBuffer[iFront + sizeof(int)]);
				break;
			case 7:
				*(int *)pOutBuff = *((int *)&m_pBuffer[iFront]);
				*(short *)&pOutBuff[sizeof(int)] = *((short *)&m_pBuffer[iFront + sizeof(int)]);
				*(char *)&pOutBuff[sizeof(int) + sizeof(short)] = *((char *)&m_pBuffer[iFront + sizeof(int) + sizeof(short)]);
				break;
			case 8:
				*(__int64 *)pOutBuff = *((__int64 *)&m_pBuffer[iFront]);
				break;
			}
		}
		else
		{
			memcpy_s(pOutBuff, iCpySize, &m_pBuffer[iFront], iCpySize);
		}
	}
	else //������ ����
	{
		int iSubSize;

		iSubSize = iCpySize - iPieceSize;

		if (8 >= iPieceSize)
		{
			//8 ����Ʈ ���� �������� �ذ�
			switch (iPieceSize)
			{
			case 1:
				*(char *)pOutBuff = *((char *)&m_pBuffer[iFront]);
				break;
			case 2:
				*(short *)pOutBuff = *((short *)&m_pBuffer[iFront]);
				break;
			case 3:
				*(short *)pOutBuff = *((short *)&m_pBuffer[iFront]);
				*(char *)&pOutBuff[sizeof(short)] = *((char *)&m_pBuffer[iFront + sizeof(short)]);
				break;
			case 4:
				*(int *)pOutBuff = *((int *)&m_pBuffer[iFront]);
				break;
			case 5:
				*(int *)pOutBuff = *((int *)&m_pBuffer[iFront]);
				*(char *)&pOutBuff[sizeof(int)] = *((char *)&m_pBuffer[iFront + sizeof(int)]);
				break;
			case 6:
				*(int *)pOutBuff = *((int *)&m_pBuffer[iFront]);
				*(short *)&pOutBuff[sizeof(int)] = *((short *)&m_pBuffer[iFront + sizeof(int)]);
				break;
			case 7:
				*(int *)pOutBuff = *((int *)&m_pBuffer[iFront]);
				*(short *)&pOutBuff[sizeof(int)] = *((short *)&m_pBuffer[iFront + sizeof(int)]);
				*(char *)&pOutBuff[sizeof(int) + sizeof(short)] = *((char *)&m_pBuffer[iFront + sizeof(int) + sizeof(short)]);
				break;
			case 8:
				*(__int64 *)pOutBuff = *((__int64 *)&m_pBuffer[iFront]);
				break;
			}
		}
		else
		{
			memcpy_s(pOutBuff, iPieceSize, &m_pBuffer[iFront], iPieceSize);
		}

		if (8 >= iSubSize)
		{
			//8 ����Ʈ ���� �������� �ذ�
			switch (iSubSize)
			{
			case 1:
				*(char *)&pOutBuff[iPieceSize] = *((char *)m_pBuffer);
				break;
			case 2:
				*(short *)&pOutBuff[iPieceSize] = *((short *)m_pBuffer);
				break;
			case 3:
				*(short *)&pOutBuff[iPieceSize] = *((short *)m_pBuffer);
				*(char *)&pOutBuff[iPieceSize + sizeof(short)] = *((char *)&m_pBuffer[sizeof(short)]);
				break;
			case 4:
				*(int *)&pOutBuff[iPieceSize] = *((int *)m_pBuffer);
				break;
			case 5:
				*(int *)&pOutBuff[iPieceSize] = *((int *)m_pBuffer);
				*(char *)&pOutBuff[iPieceSize + sizeof(int)] = *((char *)&m_pBuffer[sizeof(int)]);
				break;
			case 6:
				*(int *)&pOutBuff[iPieceSize] = *((int *)m_pBuffer);
				*(short *)&pOutBuff[iPieceSize + sizeof(int)] = *((short *)&m_pBuffer[sizeof(int)]);
				break;
			case 7:
				*(int *)&pOutBuff[iPieceSize] = *((int *)m_pBuffer);
				*(short *)&pOutBuff[iPieceSize + sizeof(int)] = *((short *)&m_pBuffer[sizeof(int)]);
				*(char *)&pOutBuff[iPieceSize + sizeof(int) + sizeof(short)] = *((char *)&m_pBuffer[sizeof(int) + sizeof(short)]);
				break;
			case 8:
				*(__int64 *)&pOutBuff[iPieceSize] = *((__int64 *)m_pBuffer);
				break;
			}
		}
		else
		{
			memcpy_s(&pOutBuff[iPieceSize], iSubSize, m_pBuffer, iSubSize);
		}
	}

	return iCpySize;
}
void CRingBuffer::RemoveData(int _iSize)
{
	int iFront;

	iFront = m_iFront;

	iFront += _iSize;
	if (m_iBufferSize <= iFront)
	{
		iFront -= m_iBufferSize;
	}
	m_iFront = iFront;
}
void CRingBuffer::MoveWritePos(int _iSize)
{
	int iRear;

	iRear = m_iRear;

	iRear += _iSize;
	if (m_iBufferSize <= iRear)
	{
		iRear -= m_iBufferSize;
	}
	m_iRear = iRear;
}
void CRingBuffer::ClearBuffer()
{
	m_iFront = 0;
	m_iRear = 0;
}