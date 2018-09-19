#include "pch.h"
#include "PDHCheck.h"

#include <strsafe.h>


namespace Library_Jingyu
{
	// ������
	CPDH::CPDH()
	{
		m_iQueryIndex = 0;

		// ���� ����
		PdhOpenQuery(NULL, NULL, &m_pdh_Query);
	}

	// ���� �����ϱ�.
	// �ش� Ŭ������ ���ɸ���Ϳ� ���� ���� ����
	//
	// Parameter : ����� ����.
	// return : ��� ���� �� true, ���н� false
	bool CPDH::SetQuery(TCHAR* tQuery)
	{
		// ���� ����
		HRESULT ret =  StringCbCopy(m_tcQuery[m_iQueryIndex], 256, tQuery);

		// ���� �� false ����
		if (ret != S_OK)
		{
			printf("SetQuery(). PDH query Set Fail..\n");
			return false;
		}

		m_iQueryIndex++;

		return true;
	}

	// ��ϵ� ��� ������ ���� ����
	//
	void CPDH::UpdateInfo()
	{
		int i = 0;
		while (i< m_iQueryIndex)
		{
			// ---------- ī���� ���
			PdhAddCounter(m_pdh_Query, m_tcQuery[i], NULL, &m_pdh_Counter[i]);

			// ---------- ������ ����
			PdhCollectQueryData(m_pdh_Query);

			// ---------- ������ �̱�.
			PDH_STATUS Status;

			PDH_FMT_COUNTERVALUE CounterValue;

			Status = PdhGetFormattedCounterValue(m_pdh_Counter[i], PDH_FMT_DOUBLE, NULL, &CounterValue);
			if (Status == 0)
				m_pdh_value[i] = CounterValue.doubleValue;

			i++;
		}	
		
	}
}