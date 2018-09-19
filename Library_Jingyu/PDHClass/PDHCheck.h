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

	// ���� 1������ ����������, �����⸦ �ݺ��ؾ� �Ѵ�.
	class CPDH
	{
		// -----------------
		// ��� ����
		// -----------------

		// PDH�� ������ ����. ī��Ʈ�� ��Ī
		PDH_HQUERY	m_pdh_Query;		

		// ���� ������ ����� ����
		double			m_pdh_value;

	public:
		// ������
		CPDH();

		// ���� ������
		// ����� GetResult() �Լ��� ��� ����
		//
		// Parameter : ����(TCHARr*)
		// return : ����
		void Query(const TCHAR* tQuery);

		// ��� ���
		//
		// Parameter : ����
		// return : ��� ��(double)
		double GetResult();

	};
}

#endif // !__PDH_CHECK_H__
