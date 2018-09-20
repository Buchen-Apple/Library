#include "pch.h"
#include "PDHCheck.h"
#include "CrashDump\CrashDump.h"

#include <strsafe.h>


namespace Library_Jingyu
{
	CCrashDump* gPdhDump = CCrashDump::GetInstance();

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

		// -------------- �ʿ��� ������ ��� ���

		// ��밡�� �޸� ���� ���
		if (PdhAddCounter(m_pdh_Query, L"\\Memory\\Available MBytes", NULL, &m_pdh_AVA_MEM_Counter) != ERROR_SUCCESS)
		{
			gPdhDump->Crash();
		}

		// �������� �޸� ��뷮 ���� ���
		if(PdhAddCounter(m_pdh_Query, L"\\Memory\\Pool Nonpaged Bytes", NULL, &m_pdh_NonPagedPool_Counter) != ERROR_SUCCESS)
		{
			gPdhDump->Crash();
		}

		// ��Ʈ��ũ, Recv�� ���� ���
		if (PdhAddCounter(m_pdh_Query, L"\\Network Interface(*)\\Bytes Received/sec", NULL, &m_pdh_Recv_Counter) != ERROR_SUCCESS)
		{
			gPdhDump->Crash();
		}

		// ��Ʈ��ũ, Send�� ���� ���
		if (PdhAddCounter(m_pdh_Query, L"\\Network Interface(*)\\Bytes Sent/sec", NULL, &m_pdh_Send_Counter) != ERROR_SUCCESS)
		{
			gPdhDump->Crash();
		}

		// ���� Ŀ�� ��뷮 ���� ���
		// ��������� ���μ��� �̸� ����
		SetProcessName();

		// ���� ���
		TCHAR tQuery[256] = { 0, };
		StringCbPrintf(tQuery, sizeof(TCHAR) * 256, L"\\Process(%s)\\Private Bytes", m_ProcessName);

		if(PdhAddCounter(m_pdh_Query, tQuery, NULL, &m_pdh_UserCommit) != ERROR_SUCCESS)
		{
			gPdhDump->Crash();
		}


		// -------------- ����,���ú� �迭 �̸� �����α�   	      

		// ������ ��������
		PdhCollectQueryData(m_pdh_Query);

		// 1. ���ú�
		// ������ PDH_MORE_DATA�� ���;� �Ѵ�. ���� ����� �����ϱ� ������.
		if (PdhGetFormattedCounterArray(m_pdh_Recv_Counter, PDH_FMT_DOUBLE, &m_dwBufferSize_Recv, &m_dwItemCount_Recv, m_pitems_Recv) == PDH_MORE_DATA)
		{
			// ���� �����Ҵ�
			m_pitems_Recv = (PDH_FMT_COUNTERVALUE_ITEM *)malloc(m_dwBufferSize_Recv);
		}
		else
			gPdhDump->Crash();

		// 2. ����
		// ������ PDH_MORE_DATA�� ���;� �Ѵ�. ���� ����� �����ϱ� ������.
		if (PdhGetFormattedCounterArray(m_pdh_Send_Counter, PDH_FMT_DOUBLE, &m_dwBufferSize_Send, &m_dwItemCount_Send, m_pitems_Send) == PDH_MORE_DATA)
		{
			// ���� �����Ҵ�
			m_pitems_Send = (PDH_FMT_COUNTERVALUE_ITEM *)malloc(m_dwBufferSize_Send);
		}
		else
			gPdhDump->Crash();
		

	}



	// -----------------
	// ��� �Լ�
	// -----------------	

	// ���� ���μ��� �̸� ����
	// ��������� �����Ѵ�.
	//
	// Parameter : ����
	// return : ����
	void CPDH::SetProcessName()
	{
		TCHAR Name[1000];

		// ������ ��ΰ� ���´�.
		GetModuleFileName(NULL, Name, 1000);

		// Path�� ���̸� �˾ƿ´�.
		int Size = (int)_tcslen(Name);
		--Size;

		// ���� ���� ����Ų��. ������ <<�� ��ĭ�� ���鼭 \�� ã�´�.
		TCHAR* Start = &Name[Size];
		while (1)
		{
			// \�� ã������, >>�� ��ĭ ������, ���� �̸��� ó���� ����Ű�� �Ѵ�.
			if (*Start == L'\\')
			{
				Size++;
				Start = &Name[Size];
				break;
			}

			--Size;
			Start = &Name[Size];
		}

		// Start�� ���� �̸��� ����Ű����. ������, exe���� �پ��ֱ� ������ 3�� ����.
		// ��Ȯ���� ".exe"�� ������ ������, StringCbCopy�� Null���� �����, �ϳ� �� �� �����ϱ� ������
		// �����ص� �ȴ�.
		//
		// ex) PDH_Main.exe�� ���̸� ���ϸ� 12�� ���´�. 
		// ex) 3������ 9�� ���´�.
		// ex) �ι��� ����ϸ� �� 8���� ���ڰ� ����ȴ� (PDH_Main ������ �����)
		StringCbCopy(m_ProcessName, sizeof(TCHAR) * (_tcslen(Start) - 3), Start);
	}





	// ���� �����ϱ�
	// ī������ ���� ��� ���� ��, �� ������ �־�д�.
	// 
	// Parameter : ����
	// return : ����
	void CPDH::SetInfo()
	{
		// ����, ���ú� �� 0���� ����.
		m_pdh_Recv_Value = m_pdh_Send_Value = 0;


		// ---------- ������ ��������
		PdhCollectQueryData(m_pdh_Query);

		// ---------- ������ �̱��� �� Value ����
		PDH_FMT_COUNTERVALUE CounterValue;

		// 1. ��밡�� �޸�
		if (PdhGetFormattedCounterValue(m_pdh_AVA_MEM_Counter, PDH_FMT_DOUBLE, NULL, &CounterValue) == ERROR_SUCCESS)
			m_pdh_AVA_Mem_Value = CounterValue.doubleValue;

		// 2. ��Ʈ��ũ ���ú�
		if (PdhGetFormattedCounterArray(m_pdh_Recv_Counter, PDH_FMT_DOUBLE, &m_dwBufferSize_Recv, &m_dwItemCount_Recv, m_pitems_Recv) == ERROR_SUCCESS)
		{
			// Loop through the array and print the instance name and counter value.
			for (DWORD i = 0; i < m_dwItemCount_Recv; i++)
			{
				m_pdh_Recv_Value += m_pitems_Recv[i].FmtValue.doubleValue;
			}
		}		

		// 3. ��Ʈ��ũ ����
		if (PdhGetFormattedCounterArray(m_pdh_Send_Counter, PDH_FMT_DOUBLE, &m_dwBufferSize_Send, &m_dwItemCount_Send, m_pitems_Send) == ERROR_SUCCESS)
		{
			// Loop through the array and print the instance name and counter value.
			for (DWORD i = 0; i < m_dwItemCount_Send; i++)
			{
				m_pdh_Send_Value += m_pitems_Send[i].FmtValue.doubleValue;
			}
		}

		// 4. �������� �޸�
		if (PdhGetFormattedCounterValue(m_pdh_NonPagedPool_Counter, PDH_FMT_DOUBLE, NULL, &CounterValue) == ERROR_SUCCESS)
			m_pdh_NonPagedPool_Value = CounterValue.doubleValue;

		// 5. �޸� ���� Ŀ�� ��뷮
		if (PdhGetFormattedCounterValue(m_pdh_UserCommit, PDH_FMT_DOUBLE, NULL, &CounterValue) == ERROR_SUCCESS)
			m_pdh_UserCommit_Value = CounterValue.doubleValue;
	}

	// ��밡�� �޸� ��� (MByte����)
	//
	// parameter : ����
	// return : �ϵ������ ��밡�� �޸�
	double CPDH::Get_AVA_Mem()
	{
		return m_pdh_AVA_Mem_Value;
	}

	// �� �������� �޸� ��뷮 ��� (Byte����)
	//
	// parameter : ����
	// return : �ϵ������ �� �������� �޸� ��뷮
	double CPDH::Get_NonPaged_Mem()
	{
		return m_pdh_NonPagedPool_Value;
	}

	// ��Ʈ��ũ �̴��� Recv ��� (Byte)
	//
	// parameter : ����
	// return : ��Ʈ��ũ �̴��� Recv
	double CPDH::Get_Net_Recv()
	{
		return m_pdh_Recv_Value;
	}


	// ��Ʈ��ũ �̴��� Send ��� (Byte)
	//
	// parameter : ����
	// return : ��Ʈ��ũ �̴��� Send
	double CPDH::Get_Net_Send()
	{
		return m_pdh_Send_Value;
	}

	// ���� Ŀ�� ũ�� (Byte)
	//
	// parameter : ����
	// return : ���� Ŀ�� ũ��
	double  CPDH::Get_UserCommit()
	{
		return m_pdh_UserCommit_Value;
	}

}