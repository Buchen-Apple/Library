#ifndef __PDH_CHECK_H__
#define __PDH_CHECK_H__

#include <Windows.h>

#include <Pdh.h>
#include <pdhmsg.h>

#pragma comment(lib, "Pdh.lib")

// ---------------------
// ���� ����Ϳ��� ���� ������ Ŭ����
// PDH �̿�
// ---------------------
namespace Library_Jingyu
{

	// ���� 1������ ����������, �����⸦ �ݺ��ؾ� �Ѵ�.
	class CPDH
	{
		// -----------------
		// ī���Ϳ� ������
		// -----------------

		// ��밡�� �޸� ī��Ʈ
		PDH_HCOUNTER	m_pdh_AVA_MEM_Counter;		

		// �� �������� �޸� ��뷮 ī��Ʈ
		PDH_HCOUNTER	m_pdh_NonPagedPool_Counter;

		// ��Ʈ��ũ ���ú�� ī��Ʈ
		PDH_HCOUNTER	m_pdh_Recv_Counter;

		// ��Ʈ��ũ ����� ī��Ʈ
		PDH_HCOUNTER	m_pdh_Send_Counter;

		// ���� Ŀ�� �޸� ��뷮(Private Byte)
		PDH_HCOUNTER	m_pdh_UserCommit;
		


		// -----------------
		// ��� ����
		// -----------------	

		// ���� ���μ����� �̸�
		TCHAR m_ProcessName[50];

		// PDH�� ������ ����.
		PDH_HQUERY	m_pdh_Query;	

		// ��밡�� �޸� ����
		double m_pdh_AVA_Mem_Value;		

		// �� �������� �޸� ����
		double m_pdh_NonPagedPool_Value;

		// ��Ʈ��ũ ���ú�� ����
		double m_pdh_Recv_Value;

		// ��Ʈ��ũ ����� ����
		double m_pdh_Send_Value;

		// ���� Ŀ�� �޸� ��뷮(Private Byte)
		double m_pdh_UserCommit_Value;
		
		// ��Ʈ��ũ ����,���ú�� ����
		// �ش� ������ PdhGetFormattedCounterArray�� ���ؿ��� ������ �迭 �ʿ�.
		// ���̸� �̸� �����ڿ��� ���صд�.
		PDH_FMT_COUNTERVALUE_ITEM* m_pitems_Recv;
		DWORD m_dwBufferSize_Recv, m_dwItemCount_Recv;

		PDH_FMT_COUNTERVALUE_ITEM *m_pitems_Send;
		DWORD m_dwBufferSize_Send, m_dwItemCount_Send;


	private:
		// -----------------
		// ��� �Լ�
		// -----------------	

		// ���� ���μ��� �̸� ����
		// ��������� �����Ѵ�.
		//
		// Parameter : ����
		// return : ����
		void SetProcessName();

	public:
		// ������
		CPDH();

		// -----------------
		// ���� �Լ�
		// -----------------	

		// ���� �����ϱ�
		// ī������ ���� ��� ���� ��, �� ������ �־�д�.
		// 
		// Parameter : ����
		// return : ����
		void SetInfo();

		// ��밡�� �޸� ��� (MByte����)
		//
		// parameter : ����
		// return : �ϵ������ ��밡�� �޸�
		double Get_AVA_Mem();

		// �� �������� �޸� ��뷮 ��� (Byte����)
		//
		// parameter : ����
		// return : �ϵ������ �� �������� �޸� ��뷮
		double Get_NonPaged_Mem();

		// ��Ʈ��ũ �̴��� Recv ��� (Byte)
		//
		// parameter : ����
		// return : ��Ʈ��ũ �̴��� Recv
		double Get_Net_Recv();


		// ��Ʈ��ũ �̴��� Send ��� (Byte)
		//
		// parameter : ����
		// return : ��Ʈ��ũ �̴��� Send
		double Get_Net_Send();	

		// ���� Ŀ�� ũ�� (Byte)
		//
		// parameter : ����
		// return : ���� Ŀ�� ũ��
		double Get_UserCommit();

	};
}

#endif // !__PDH_CHECK_H__
