#include "pch.h"
#include "PDHCheck.h"

#include <strsafe.h>


namespace Library_Jingyu
{
	// ���� ��Ʈ��
	/*

	L"\\Process(�̸�)\\Thread Count"		// ������
	L"\\Process(�̸�)\\Working Set"		// ��� �޸�

	L"\\Memory\\Available MBytes"		// ��밡��

	L"\\Memory\\Pool Nonpaged Bytes"	// ����������

	L"\\Network Interface(*)\\Bytes Received/sec"
	L"\\Network Interface(*)\\Bytes Sent/sec"

	*/


	// ������
	CPDH::CPDH()
	{
		// ���� ����
		PdhOpenQuery(NULL, NULL, &m_pdh_Query);

		// �ʱ� ����� 0���� �ʱ�ȭ
		m_pdh_value = 0;
	}


	// ���� ������
	// ����� GetResult() �Լ��� ��� ����
	//
	// Parameter : ����(TCHARr*)
	// return : ����
	void  CPDH::Query(const TCHAR* tQuery)
	{
		// ���� ī���� �ڵ� ����
		PDH_HCOUNTER	pdh_Counter;

		// ---------- ī���� ���
		PdhAddCounter(m_pdh_Query, tQuery, NULL, &pdh_Counter);

		// ---------- ������ ����
		PdhCollectQueryData(m_pdh_Query);

		// ---------- ������ �̱�.
		PDH_FMT_COUNTERVALUE CounterValue;

		if(PdhGetFormattedCounterValue(pdh_Counter, PDH_FMT_DOUBLE, NULL, &CounterValue) == ERROR_SUCCESS)
			m_pdh_value = CounterValue.doubleValue;		
		
	}

	// ��� ���
	//
	// Parameter : ����
	// return : ��� ��(double)
	double CPDH::GetResult()
	{
		return m_pdh_value;
	}
}