#ifndef __CPU_USAGE_H__
#define __CPU_USAGE_H__

#include <Windows.h>

// ---------------------
// '���μ���'�� ������� ���ϴ� Ŭ����
// ---------------------
namespace Library_Jingyu
{
	class CCpuUsage_Processor
	{
		// ---------------------
		// ��� ����		
		// ---------------------
		float m_fProcessorTotal;
		float m_fProcessorUser;
		float m_fProcessorKernel;		

		ULARGE_INTEGER m_ftProcessor_LastKernel;
		ULARGE_INTEGER m_ftProcessor_LastUser;
		ULARGE_INTEGER m_ftProcessor_LastIdle;

	public:
		// ---------------------
		// ������
		// ---------------------
		CCpuUsage_Processor();

		// ---------------------
		// ��� �Լ�
		// ---------------------

		// ���μ��� ����� ����
		//
		// �ش� �Լ��� ȣ��Ǹ�, �� PC ���μ����� Idle, User, Kernel ������ ���ŵȴ�.
		void UpdateCpuTime();


		// ---------------------
		// ���� �Լ�
		// ---------------------
		// ���μ��� total ���
		//
		// Parameter : ����
		// return : ���μ����� ��Ż�� (float)
		float  ProcessorTotal()
		{
			return m_fProcessorTotal;
		}

		// ���μ��� ��, User ��� ����� ���
		//
		// Parameter : ����
		// return : ���μ��� ��, User��� ����� (float)
		float  ProcessorUser()
		{
			return m_fProcessorUser;
		}

		// ���μ��� ��, Kernel ��� ����� ���
		//
		// Parameter : ����
		// return : ���μ��� ��, Kernel��� ����� (float)
		float  ProcessorKernel()
		{
			return m_fProcessorKernel;
		}
	};
}

// ---------------------
// '������ ���μ���'�� ������� ���ϴ� Ŭ����
// ---------------------
namespace Library_Jingyu
{
	class CCpuUsage_Process
	{
		// ---------------------
		// ��� ����		
		// ---------------------
		HANDLE m_hProcess;

		int m_iNumberOfProcessors;	// �ش� PC�� ������ ���� ��. (������ ������ ����)

		float m_fProcessTotal;
		float m_fProcessUser;
		float m_fProcessKernel;

		ULARGE_INTEGER m_ftProcess_LastKernel;
		ULARGE_INTEGER m_ftProcess_LastUser;
		ULARGE_INTEGER m_ftProcess_LastTime;

	public:
		// ������
		//
		// Parameter : ������� üũ�� ���μ��� �ڵ�. (����Ʈ : �ڱ��ڽ�)
		CCpuUsage_Process(HANDLE hProcess = INVALID_HANDLE_VALUE);



		// ---------------------
		// ��� �Լ�
		// ---------------------

		// ���μ��� ����� ����
		//
		// �ش� �Լ��� ȣ��Ǹ�, ������ ���μ����� Total, User, Kernel ������ ���ŵȴ�.
		void UpdateCpuTime();




		// ---------------------
		// ���� �Լ�
		// ---------------------

		// ������ ���μ����� total ���
		//
		// Parameter : ����
		// return : ������ ���μ����� ��Ż�� (float)
		float  ProcessTotal()
		{
			return m_fProcessTotal;
		}

		// ������ ���μ�����, User ��� ����� ���
		//
		// Parameter : ����
		// return : ������ ���μ�����, User��� ����� (float)
		float  ProcessUser()
		{
			return m_fProcessUser;
		}

		// ������ ���μ�����, Kernel ��� ����� ���
		//
		// Parameter : ����
		// return : ������ ���μ�����, Kernel��� ����� (float)
		float  ProcessKernel()
		{
			return m_fProcessKernel;
		}
	};
}

#endif // !__CPU_USAGE_H__
