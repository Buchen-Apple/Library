#include "pch.h"
#include "ProtocolBuff_ObjectPool.h"

namespace Library_Jingyu
{


#define _MyCountof(_array)		sizeof(_array) / (sizeof(_array[0]))

#define BUFF_SIZE 256

	// static �޸�Ǯ
	CMemoryPoolTLS<CProtocolBuff>* CProtocolBuff::m_MPool = new CMemoryPoolTLS<CProtocolBuff>(100, false);

	// ���� ���� �� Crash �߻���ų ����.
	CCrashDump* CProtocolBuff::m_Dump = CCrashDump::GetInstance();

	// ������ ������ ������
	CProtocolBuff::CProtocolBuff(int size)
	{		
		m_Size = size;
		m_pProtocolBuff = new char[size];
		m_Front = 0;
		m_Rear = 2; // ó�� �տ� 2����Ʈ�� ����� �־�� �ϱ� ������ rear�� 2�� �����صд�.
		m_RefCount = 1;	// ���۷��� ī��Ʈ 1���� �ʱ�ȭ (�����Ǿ����� ī��Ʈ 1�� �Ǿ�� �Ѵ�.)
	}

	// ������ ���� ���� ������
	CProtocolBuff::CProtocolBuff()
	{		
		m_Size = BUFF_SIZE;
		m_pProtocolBuff = new char[BUFF_SIZE];
		m_Front = 0;
		m_Rear = 2; // ó�� �տ� 2����Ʈ�� ����� �־�� �ϱ� ������ rear�� 2�� �����صд�.
		m_RefCount = 1;	// ���۷��� ī��Ʈ 1���� �ʱ�ȭ (�����Ǿ����� ī��Ʈ 1�� �Ǿ�� �Ѵ�.)	
	}

	// �Ҹ���
	CProtocolBuff::~CProtocolBuff()
	{
		delete[] m_pProtocolBuff;
	}

	// ���� ũ�� �缳��
	int CProtocolBuff::ReAlloc(int size)
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
	void CProtocolBuff::Clear()
	{
		m_Front = m_Rear = 0;
	}

	// ������ �ֱ�
	int CProtocolBuff::PutData(const char* pSrc, int size)
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
			*(char *)&m_pProtocolBuff[m_Rear + sizeof(short)] = *(char *)&pSrc[sizeof(short)];
			break;
		case 4:
			*(int *)&m_pProtocolBuff[m_Rear] = *(int *)pSrc;
			break;
		case 5:
			*(int *)&m_pProtocolBuff[m_Rear] = *(int *)pSrc;
			*(char *)&m_pProtocolBuff[m_Rear + sizeof(int)] = *(char *)&pSrc[sizeof(int)];
			break;
		case 6:
			*(int *)&m_pProtocolBuff[m_Rear] = *(int *)pSrc;
			*(short *)&m_pProtocolBuff[m_Rear + sizeof(int)] = *(short *)&pSrc[sizeof(int)];
			break;
		case 7:
			*(int *)&m_pProtocolBuff[m_Rear] = *(int *)pSrc;
			*(short *)&m_pProtocolBuff[m_Rear + sizeof(int)] = *(short *)&pSrc[sizeof(int)];
			*(char *)&m_pProtocolBuff[m_Rear + sizeof(int) + sizeof(short)] = *(char *)&pSrc[sizeof(int) + sizeof(short)];
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
	int CProtocolBuff::GetData(char* pSrc, int size)
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
			*(char *)&pSrc[sizeof(short)] = *(char *)&m_pProtocolBuff[m_Front + sizeof(short)];
			break;
		case 4:
			*(int *)pSrc = *(int *)&m_pProtocolBuff[m_Front];
			break;
		case 5:
			*(int *)pSrc = *(int *)&m_pProtocolBuff[m_Front];
			*(char *)&pSrc[sizeof(int)] = *(char *)&m_pProtocolBuff[m_Front + sizeof(int)];
			break;
		case 6:
			*(int *)pSrc = *(int *)&m_pProtocolBuff[m_Front];
			*(short *)&pSrc[sizeof(int)] = *(short *)&m_pProtocolBuff[m_Front + sizeof(int)];
			break;
		case 7:
			*(int *)pSrc = *(int *)&m_pProtocolBuff[m_Front];
			*(short *)&pSrc[sizeof(int)] = *(short *)&m_pProtocolBuff[m_Front + sizeof(int)];
			*(char *)&pSrc[sizeof(int) + sizeof(short)] = *(char *)&m_pProtocolBuff[m_Front + sizeof(int) + sizeof(short)];
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
	char* CProtocolBuff::GetBufferPtr(void)
	{
		return m_pProtocolBuff;
	}

	// Rear �����̱�
	int CProtocolBuff::MoveWritePos(int size)
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
	int CProtocolBuff::MoveReadPos(int size)
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
	int	CProtocolBuff::GetUseSize(void)
	{
		return m_Rear - m_Front;
	}

	// ���� ���ۿ� ���� �뷮 ���.
	int	CProtocolBuff::GetFreeSize(void)
	{
		return m_Size - m_Rear;
	}


	// �޸�Ǯ���� ����ȭ���� 1�� Alloc
	CProtocolBuff* CProtocolBuff::Alloc()
	{
		return  m_MPool->Alloc();;
	}

	// Free. ���۷��� ī��Ʈ 1 ����.
	// ����, ���۷��� ī��Ʈ�� 0�̶�� �޸�Ǯ�� Free��
	void CProtocolBuff::Free(CProtocolBuff* pBuff)
	{
		// ���Ͷ����� �����ϰ� ����.
		// ���� ���� �� 0�̵ƴٸ� delete
		if (InterlockedDecrement(&pBuff->m_RefCount) == 0)
		{
			pBuff->m_Rear = 2;		// rear �� 2�� �ʱ�ȭ
			pBuff->m_RefCount = 1;	// ref�� 1�� �ʱ�ȭ
			m_MPool->Free(pBuff);
		}

	}

	// ���۷��� ī��Ʈ 1 Add�ϴ� �Լ�
	void CProtocolBuff::Add()
	{
		// ���Ͷ����� �����ϰ� ����
		InterlockedIncrement(&m_RefCount);
	}





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