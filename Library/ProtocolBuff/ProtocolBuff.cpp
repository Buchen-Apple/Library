#include "stdafx.h"
#include "ProtocolBuff.h"
#include <string.h>
#include <stdlib.h>
#include <tchar.h>

namespace Library_Jingyu
{

#define BUFF_SIZE 10000

	// ������ ������ ������
	CProtocolBuff::CProtocolBuff(int size)
	{
		Init(size);
	}

	// ������ ���� ���� ������
	CProtocolBuff::CProtocolBuff()
	{
		Init(BUFF_SIZE);
	}

	// �Ҹ���
	CProtocolBuff::~CProtocolBuff()
	{
		delete[] m_pProtocolBuff;
	}

	// �ʱ�ȭ
	void CProtocolBuff::Init(int size)
	{
		m_Size = size;
		m_pProtocolBuff = new char[size];
		m_Front = 0;
		m_Rear = 0;
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
	int CProtocolBuff::PutData(const char* pSrc, int size)	throw (CException)
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

	// ���� ������� �뷮 ���.
	int	CProtocolBuff::GetUseSize(void)
	{
		//return m_Size - GetFreeSize();

		return m_Rear;
	}

	// ���� ���ۿ� ���� �뷮 ���.
	int	CProtocolBuff::GetFreeSize(void)
	{
		return m_Size - m_Rear;
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