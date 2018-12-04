#include "pch.h"
#include "ArrayList.h"

using namespace std;

// ����Ʈ �ʱ�ȭ.
void ArrayList::ListInit(int MaxCount)
{
	m_iMaxNodeCount = MaxCount;

	m_Array = new LData[MaxCount];

	m_iNodeCount = 0;
	m_iNowIndex = -1;
}

// ����Ʈ�� ������ �ֱ�
//
// Parameter : ���� ������.
// return : ����
void ArrayList::ListInsert(LData data)
{
	// Maxüũ
	if (m_iNodeCount == m_iMaxNodeCount)
	{
		cout << "count max" << endl;
		return;
	}

	// Max�� �ƴϸ� �ִ´�.	
	m_Array[m_iNodeCount] = data;
	m_iNodeCount++;
}

// ����Ʈ�� ù ������ ��������
//
// Parameter : �����͸� �޾ƿ� ������
// return :  �����Ͱ� ���� ��� false.
//			 �����Ͱ� ���� ��� true ����
bool ArrayList::ListFirst(LData* data)
{
	// ��尡 �ϳ��� ���� ���
	if (m_iNodeCount == 0)
		return false;

	// ��尡 ���� ���
	m_iNowIndex = 0;
	*data = m_Array[m_iNowIndex];

	return true;
}

// ����Ʈ�� �ι�° ���� ������ ��������
//
// Parameter : �����͸� �޾ƿ� ������
// return :  �����Ͱ� ���� ��� false.
//			 �����Ͱ� ���� ��� true ����
bool ArrayList::ListNext(LData* data)
{
	// ���� �������� ���
	if (m_iNowIndex == m_iNodeCount-1)
		return false;

	// ��尡 ���� ���
	m_iNowIndex++;
	*data = m_Array[m_iNowIndex];

	return true;
}

// ����Ʈ�� ���� ������ ����. (���� �ֱٿ� ���� ������)
//
// Parameter : ������ �����͸� �޾ƿ� ������
// return :  �����Ͱ� ���� ��� false.
//			 �����Ͱ� ���� ��� true ����
bool ArrayList::ListRemove(LData* data)
{
	// ��尡 �ϳ��� ���� ���
	if (m_iNodeCount == 0)
		return false;

	// ������ ������ �޾Ƶα�
	*data = m_Array[m_iNowIndex];

	// ��ĭ�� ��ܼ� ����� ä���
	int TempPos = m_iNowIndex;
	while (TempPos < m_iNodeCount)
	{
		m_Array[TempPos] = m_Array[TempPos + 1];
		TempPos++;
	}

	// ������ ����
	m_iNowIndex--;
	m_iNodeCount--;

	return true;
}

// ����Ʈ ������ ī��Ʈ ��
int ArrayList::ListCount()
{
	return m_iNodeCount;
}