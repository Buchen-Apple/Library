#include "pch.h"
#include "ProtocolBuff_ObjectPool.h"

namespace Library_Jingyu
{


#define _MyCountof(_array)		sizeof(_array) / (sizeof(_array[0]))

	// ����ȭ ���� 1���� ũ��
#define BUFF_SIZE 512

	// LanServer Packet ��� ������.
#define dfNETWORK_PACKET_HEADER_SIZE 2

	// static �޸�Ǯ
	CMemoryPoolTLS<CProtocolBuff_Lan>* CProtocolBuff_Lan::m_MPool = new CMemoryPoolTLS<CProtocolBuff_Lan>(100, false);

	// ���� ���� �� Crash �߻���ų ����.
	CCrashDump* CProtocolBuff_Lan::m_Dump = CCrashDump::GetInstance();
	   	 
	// ������ ������ ������
	CProtocolBuff_Lan::CProtocolBuff_Lan(int size)
	{		
		m_Size = size;
		m_pProtocolBuff = new char[size];
		m_Front = 0;
		m_Rear = dfNETWORK_PACKET_HEADER_SIZE; // ó�� �տ� 2����Ʈ�� ����� �־�� �ϱ� ������ rear�� 2�� �����صд�.
		m_RefCount = 1;	// ���۷��� ī��Ʈ 1���� �ʱ�ȭ (�����Ǿ����� ī��Ʈ 1�� �Ǿ�� �Ѵ�.)
	}

	// ������ ���� ���� ������
	CProtocolBuff_Lan::CProtocolBuff_Lan()
	{		
		m_Size = BUFF_SIZE;
		m_pProtocolBuff = new char[BUFF_SIZE];
		m_Front = 0;
		m_Rear = dfNETWORK_PACKET_HEADER_SIZE; // ó�� �տ� 2����Ʈ�� ����� �־�� �ϱ� ������ rear�� 2�� �����صд�.
		m_RefCount = 1;	// ���۷��� ī��Ʈ 1���� �ʱ�ȭ (�����Ǿ����� ī��Ʈ 1�� �Ǿ�� �Ѵ�.)	
	}

	// �Ҹ���
	CProtocolBuff_Lan::~CProtocolBuff_Lan()
	{
		delete[] m_pProtocolBuff;
	}

	// ����� ä��� �Լ�. Lan���� ����
	void CProtocolBuff_Lan::SetProtocolBuff_HeaderSet()
	{
		// ����, ����� ������ ���̷ε� ������. ��, 8�� ����.
		WORD wHeader = GetUseSize() - dfNETWORK_PACKET_HEADER_SIZE;

		*(short *)m_pProtocolBuff = *(short *)&wHeader;	
	}


	// ���� ũ�� �缳��
	// ���������� ���� �Ⱦ�����.. ����Ϸ��� �� �� �� ������.
	int CProtocolBuff_Lan::ReAlloc(int size)
	{
		// ���� �����͸� temp�� ������Ų �� m_pProtocolBuff ����
		char* temp = new char[m_Size];
		memcpy(temp, &m_pProtocolBuff[m_Front], m_Size - m_Front);
		delete[] m_pProtocolBuff;

		// ���ο� ũ��� �޸� �Ҵ�
		m_pProtocolBuff = new char[size];

		// temp�� ������Ų ���� ����
		memcpy(m_pProtocolBuff, temp, m_Size - m_Front);

		// ���� ũ�� ����
		m_Size = size;

		// m_Rear�� m_Front�� ��ġ ����.
		// m_Rear�� 10�̾��� m_Front�� 3�̾�����, m_Rear�� 7�̵ǰ� m_Front�� 0�̵ȴ�. ��, ����� ����Ʈ�� ���̴� �����ϴ�.
		m_Rear -= m_Front;
		m_Front = 0;

		return size;
	}

	// ���� Ŭ����
	void CProtocolBuff_Lan::Clear()
	{
		m_Front = m_Rear = 0;
	}

	// ������ �ֱ�
	int CProtocolBuff_Lan::PutData(const char* pSrc, int size)
	{
		// ť ��á�ų� rear�� Size�� ���������� üũ
		if (m_Rear >= m_Size)
			throw CException(_T("ProtocalBuff(). PutData�� ���۰� ����."));

		// �޸� ����
		// 1~8����Ʈ ������ memcpy���� ���Կ������� ó���Ѵ�.
		// Ŀ�θ�� ��ȯ�� �ٿ�, �ӵ� ���� ���.
		switch (size)
		{
		case 1:
			*(char *)&m_pProtocolBuff[m_Rear] = *(char *)pSrc;
			break;
		case 2:
			*(short *)&m_pProtocolBuff[m_Rear] = *(short *)pSrc;
			break;
		case 3:
			*(short *)&m_pProtocolBuff[m_Rear] = *(short *)pSrc;
			*(char *)&m_pProtocolBuff[m_Rear + 2] = *(char *)&pSrc[2];
			break;
		case 4:
			*(int *)&m_pProtocolBuff[m_Rear] = *(int *)pSrc;
			break;
		case 5:
			*(int *)&m_pProtocolBuff[m_Rear] = *(int *)pSrc;
			*(char *)&m_pProtocolBuff[m_Rear + 4] = *(char *)&pSrc[4];
			break;
		case 6:
			*(int *)&m_pProtocolBuff[m_Rear] = *(int *)pSrc;
			*(short *)&m_pProtocolBuff[m_Rear + 4] = *(short *)&pSrc[4];
			break;
		case 7:
			*(int *)&m_pProtocolBuff[m_Rear] = *(int *)pSrc;
			*(short *)&m_pProtocolBuff[m_Rear + 4] = *(short *)&pSrc[4];
			*(char *)&m_pProtocolBuff[m_Rear + 6] = *(char *)&pSrc[6];
			break;
		case 8:
			*((__int64 *)&m_pProtocolBuff[m_Rear]) = *((__int64 *)pSrc);
			break;

		default:
			memcpy_s(&m_pProtocolBuff[m_Rear], size, pSrc, size);
			break;
		}

		// rear�� ��ġ �̵�
		m_Rear += size;

		return size;
	}

	// ������ ����
	int CProtocolBuff_Lan::GetData(char* pSrc, int size)
	{
		// ť ����� üũ
		if (m_Front == m_Rear)
			throw CException(_T("ProtocalBuff(). GetData�� ť�� �������."));

		// front�� ť�� ���� �����ϸ� �� �̻� �б� �Ұ���. �׳� �����Ų��.
		if (m_Front >= m_Size)
			throw CException(_T("ProtocalBuff(). GetData�� front�� ������ ���� ����."));
		
		// �޸� ����
		// 1~8����Ʈ ������ memcpy���� ���Կ������� ó���Ѵ�.
		// Ŀ�θ�� ��ȯ�� �ٿ�, �ӵ� ���� ���.
		switch (size)
		{
		case 1:
			*(char *)pSrc = *(char *)&m_pProtocolBuff[m_Front];
			break;
		case 2:
			*(short *)pSrc = *(short *)&m_pProtocolBuff[m_Front];
			break;
		case 3:
			*(short *)pSrc = *(short *)&m_pProtocolBuff[m_Front];
			*(char *)&pSrc[2] = *(char *)&m_pProtocolBuff[m_Front + 2];
			break;
		case 4:
			*(int *)pSrc = *(int *)&m_pProtocolBuff[m_Front];
			break;
		case 5:
			*(int *)pSrc = *(int *)&m_pProtocolBuff[m_Front];
			*(char *)&pSrc[4] = *(char *)&m_pProtocolBuff[m_Front + 4];
			break;
		case 6:
			*(int *)pSrc = *(int *)&m_pProtocolBuff[m_Front];
			*(short *)&pSrc[4] = *(short *)&m_pProtocolBuff[m_Front + 4];
			break;
		case 7:
			*(int *)pSrc = *(int *)&m_pProtocolBuff[m_Front];
			*(short *)&pSrc[4] = *(short *)&m_pProtocolBuff[m_Front + 4];
			*(char *)&pSrc[6] = *(char *)&m_pProtocolBuff[m_Front + 6];
			break;
		case 8:
			*((__int64 *)pSrc) = *((__int64 *)&m_pProtocolBuff[m_Front]);
			break;

		default:
			memcpy_s(pSrc, size, &m_pProtocolBuff[m_Front], size);
			break;
		}

		// ��ť�� ��ŭ m_Front�̵�
		m_Front += size;

		return size;
	}

	// ������ ������ ����.
	char* CProtocolBuff_Lan::GetBufferPtr(void)
	{
		return m_pProtocolBuff;
	}

	// Rear �����̱�
	int CProtocolBuff_Lan::MoveWritePos(int size)
	{
		// ���� Rear�� ��ġ�� ������ ���̸� �� �̻� �̵� �Ұ�
		if (m_Rear == m_Size)
			return -1;		
		
		// �Ű������� ���� iSize�� �� ���� �̵��� �� �ִ� ũ�⸦ ������� üũ
		int iRealMoveSize;

		if (m_Rear + size > m_Size)
			iRealMoveSize = m_Size - m_Rear;

		// ����� �ʴ´ٸ� �׳� �� �� ���
		else
			iRealMoveSize = size;

		m_Rear += iRealMoveSize;

		return iRealMoveSize;
		
		
	}

	// Front �����̱�
	int CProtocolBuff_Lan::MoveReadPos(int size)
	{
		// ���۰� �� ���¸� 0 ����
		if (m_Front == m_Rear)
			return 0;

		// �ѹ��� ������ �� �ִ� ũ�⸦ ������� üũ
		int iRealRemoveSize;

		if (m_Front + size > m_Size)
			iRealRemoveSize = m_Size - m_Front;

		// ����� �ʴ´ٸ� �׳� �� ũ�� ���
		else
			iRealRemoveSize = size;

		m_Front += iRealRemoveSize;

		return iRealRemoveSize;		
	}

	// ���� ������� �뷮 ���.
	int	CProtocolBuff_Lan::GetUseSize(void)
	{
		return m_Rear - m_Front;
	}

	// ���� ���ۿ� ���� �뷮 ���.
	int	CProtocolBuff_Lan::GetFreeSize(void)
	{
		return m_Size - m_Rear;
	}


	// �޸�Ǯ���� ����ȭ���� 1�� Alloc
	CProtocolBuff_Lan* CProtocolBuff_Lan::Alloc()
	{
		return  m_MPool->Alloc();
	}

	// Free. ���۷��� ī��Ʈ 1 ����.
	// ����, ���۷��� ī��Ʈ�� 0�̶�� �޸�Ǯ�� Free��
	void CProtocolBuff_Lan::Free(CProtocolBuff_Lan* pBuff)
	{
		// ���Ͷ����� �����ϰ� ����.
		// ���� ���� �� 0�̵ƴٸ� delete
		if (InterlockedDecrement(&pBuff->m_RefCount) == 0)
		{
			pBuff->m_Rear = dfNETWORK_PACKET_HEADER_SIZE;		// rear �� 2�� �ʱ�ȭ. ��� ���� Ȯ��
			pBuff->m_RefCount = 1;	// ref�� 1�� �ʱ�ȭ
			m_MPool->Free(pBuff);
		}

	}

	// ���۷��� ī��Ʈ 1 Add�ϴ� �Լ�
	void CProtocolBuff_Lan::Add()
	{
		// ���Ͷ����� �����ϰ� ����
		InterlockedIncrement(&m_RefCount);
	}

	// ------------------------------------
	// ------------------------------------
	// ------------------------------------
	// ------------------------------------
	// ���ܿ�

	// ������
	CException::CException(const wchar_t* str)
	{
		_tcscpy_s(ExceptionText, _MyCountof(ExceptionText), str);
	}

	// ���� �ؽ�Ʈ�� �ּ� ��ȯ
	char* CException::GetExceptionText()
	{
		return (char*)&ExceptionText;
	}
}