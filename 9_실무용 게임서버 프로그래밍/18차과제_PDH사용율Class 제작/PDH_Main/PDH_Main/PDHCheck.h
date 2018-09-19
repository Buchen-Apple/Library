#ifndef __PDH_CHECK_H__
#define __PDH_CHECK_H__

#include <Windows.h>

#include <Pdh.h>
#pragma comment(lib, "Pdh.lib")

// ---------------------
// ���� ����Ϳ��� ���� ������ Ŭ����
// PDH �̿�
// ---------------------
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

	class CPDH
	{
		// -----------------
		// ��� ����
		// -----------------

		// PDH�� ������ ����. ī��Ʈ�� ��Ī
		PDH_HQUERY	m_pdh_Query;

		// ���� ������ �� ī���� �ڵ� ����
		//PDH_HCOUNTER	m_pdh_Counter_NonPaged;
		PDH_HCOUNTER	m_pdh_Counter[30];

		// ���� ������ ����� ����
		//double		m_pdh_value_NonPaged;
		double			m_pdh_value[30];

		// ���� �����. �ִ� 30���� ���� ���� ����.
		// 1���� ������ �ִ� 256����
		TCHAR m_tcQuery[30][256];

		// ���� �����(�迭)�� �� ���� ������ ����ִ���.
		int m_iQueryIndex;

	public:
		// ������
		CPDH();

		// ���� �����ϱ�.
		// �ش� Ŭ������ ���ɸ���Ϳ� ���� ���� ����
		//
		// Parameter : ����� ����.
		// return : ��� ���� �� true, ���н� false
		bool SetQuery(TCHAR* tQuery);

		// ��ϵ� ��� ������ ���� ����
		//
		void UpdateInfo();


	};
}

#endif // !__PDH_CHECK_H__
