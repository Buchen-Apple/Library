#include "pch.h"
#include "ProtocolBuff_ObjectPool.h"

namespace Library_Jingyu
{


#define _MyCountof(_array)		sizeof(_array) / (sizeof(_array[0]))

#define BUFF_SIZE 1024

	// static �޸�Ǯ
	CMemoryPool<CProtocolBuff>* CProtocolBuff::m_MPool = new CMemoryPool<CProtocolBuff>(0, true);

	// ���� ���� �� Crash �߻���ų ����.
	CCrashDump* CProtocolBuff::m_Dump = CCrashDump::GetInstance();

	// ������ ������ ������
	CProtocolBuff::CProtocolBuff(int size, bool bPlacementNew)
	{
		Init(size, bPlacementNew);
	}

	// ������ ���� ���� ������
	CProtocolBuff::CProtocolBuff(bool bPlacementNew)
	{
		Init(BUFF_SIZE, bPlacementNew);
	}

	// �Ҹ���
	CProtocolBuff::~CProtocolBuff()
	{
		delete[] m_pProtocolBuff;
	}

	// �ʱ�ȭ
	void CProtocolBuff::Init(int size, bool bPlacementNew)
	{
		m_Size = size;		
		m_pProtocolBuff = new char[size];
		m_Front = 0;
		m_Rear = 2; // ó�� �տ� 2����Ʈ�� ����� �־�� �ϱ� ������ rear�� 2�� �����صд�.
		m_RefCount = 0;	// ���۷��� ī��Ʈ 0���� �ʱ�ȭ
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
		// ť ��á���� üũ
		if (m_Rear == m_Size)
			throw CException(_T("ProtocalBuff(). PutData�� ���۰� ����."));

		// �Ű����� size�� 0�̸� ����
		if (size == 0)
			return 0;

		// �޸� ����
		memcpy(&m_pProtocolBuff[m_Rear], pSrc, size);

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

		// �Ű����� size�� 0�̸� ����
		if (size == 0)
			return 0;

		// front�� ť�� ���� �����ϸ� �� �̻� �б� �Ұ���. �׳� �����Ų��.
		if (m_Front >= m_Size)
			throw CException(_T("ProtocalBuff(). GetData�� front�� ������ ���� ����."));
		
		// �޸� ����
		memcpy(pSrc, &m_pProtocolBuff[m_Front], size);

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
		// 0����� �̵��Ϸ��� �ϸ� ����
		if (size == 0)
			return 0;

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
	
	// Rear�� ���ڷ� ���� ������ ������ �����Ű��
	int CProtocolBuff::CompulsionMoveWritePos(int size)
	{
		return m_Rear = size;
	}

	// Front �����̱�
	int CProtocolBuff::MoveReadPos(int size)
	{
		// 0����� �����Ϸ��� �ϸ� ����
		if (size == 0)
			return 0;

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
	
	// Front�� ���ڷ� ���� ������ ������ �����Ű��
	int CProtocolBuff::CompulsionMoveReadPos(int size)
	{
		m_Front = size;
	}

	// ���� ������� �뷮 ���.
	int	CProtocolBuff::GetUseSize(void)
	{
		//return m_Size - GetFreeSize();

		return m_Rear - m_Front;
	}

	// ���� ���ۿ� ���� �뷮 ���.
	int	CProtocolBuff::GetFreeSize(void)
	{
		return m_Size - m_Rear;
	}


	// Alloc. ����� �� �ȿ��� new �� ���۷��� ī��Ʈ 1 ����
	CProtocolBuff* CProtocolBuff::Alloc()
	{
		CProtocolBuff* NewAlloc = m_MPool->Alloc();

		//CProtocolBuff* NewAlloc = new CProtocolBuff;
		NewAlloc->m_RefCount++; // �̶� ���� �Ҵ��̱⶧���� ���Ͷ� �ʿ� ����.

		return NewAlloc;
	}

	// Free. ����� �� �ȿ��� ���۷��� ī��Ʈ 1 ����.
	// ����, ���۷��� ī��Ʈ�� 0�̶�� ������. delete ��
	void CProtocolBuff::Free(CProtocolBuff* pBuff)
	{
		// ���Ͷ����� �����ϰ� ����.
		// ���� ���� �� 0�̵ƴٸ� delete
		if (InterlockedDecrement(&pBuff->m_RefCount) == 0)
		{
			if (m_MPool->Free(pBuff) == false)
				m_Dump->Crash();

			//delete pBuff;
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
		_tcscpy_s(ExceptionText, _countof(ExceptionText), str);
	}

	// ���� �ؽ�Ʈ�� �ּ� ��ȯ
	char* CException::GetExceptionText()
	{
		return (char*)&ExceptionText;
	}
}