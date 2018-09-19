#include "pch.h"
#include "CPUUsage.h"

// ---------------------
// '���μ���'�� ������� ���ϴ� Ŭ����
// ---------------------
namespace Library_Jingyu
{
	// ������
	CCpuUsage_Processor::CCpuUsage_Processor()
	{
		// ������� �ʱ�ȭ
		m_fProcessorTotal = 0;
		m_fProcessorUser = 0;
		m_fProcessorKernel = 0;

		m_ftProcessor_LastKernel.QuadPart = 0;
		m_ftProcessor_LastUser.QuadPart = 0;
		m_ftProcessor_LastIdle.QuadPart = 0;

		// ����, CPU ����� ����
		UpdateCpuTime();
	}

	// ���μ��� ����� ����
	//
	// �ش� �Լ��� ȣ��Ǹ�, �� PC ���μ����� Idle, User, Kernel ������ ���ŵȴ�.
	void CCpuUsage_Processor::UpdateCpuTime()
	{
		// �ʿ��� ����ü�� FILETIME ������, ULARGE_INTEGER�� ������ �����Ƿ� �̸� ���.
		// FILETIME�� �ʿ��� ������ 100 ���뼼���� ������ �ð��� �ʿ��ϱ� ������.
		// ������ ���е��� ��������.
		ULARGE_INTEGER Idle;
		ULARGE_INTEGER Kernel;
		ULARGE_INTEGER User;

		// 1. �ý��� ��� �ð��� ���Ѵ�.
		// ���̵� / Ŀ�� / ���� Ÿ��
		if (GetSystemTimes((PFILETIME)&Idle, (PFILETIME)&Kernel, (PFILETIME)&User) == false)
		{
			return;
		}

		// 2. Ŀ��, ����, ���̵� �ð� ���Ѵ�.
		// Ŀ�� Ÿ�ӿ��� ���̵� Ÿ���� ���ԵǾ� �ִ�.
		ULONGLONG KernelDiff = Kernel.QuadPart - m_ftProcessor_LastKernel.QuadPart;
		ULONGLONG UserDiff = User.QuadPart - m_ftProcessor_LastUser.QuadPart;
		ULONGLONG IdleDiff = Idle.QuadPart - m_ftProcessor_LastIdle.QuadPart;

		// 3. ��Ż ���Ѵ�.
		ULONGLONG Total = KernelDiff + UserDiff;

		// ����, Total�� 0�̶��, ���μ����� ���� ������ ���� ���̴� ��� �� 0���� ����
		if (Total == 0)
		{
			m_fProcessorTotal = 0;
			m_fProcessorUser = 0;
			m_fProcessorKernel = 0;
		}

		// Total�� 0��, �ƴ϶�� �� ���
		else
		{
			// Ŀ�� Ÿ�ӿ� ���̵� Ÿ���� �����Ƿ�, ����.
			m_fProcessorTotal = (float)((double)(Total - IdleDiff) / Total * 100.0f);
			m_fProcessorUser = (float)((double)UserDiff / Total * 100.0f);
			m_fProcessorKernel = (float)((double)(KernelDiff - IdleDiff) / Total * 100.0f);

		}

		// 4. ��� �ð�, Ŀ�� �ð�, ���� �ð� ����
		m_ftProcessor_LastKernel = Kernel;
		m_ftProcessor_LastUser = User;
		m_ftProcessor_LastIdle = Idle;

	}
}


// ---------------------
// '������ ���μ���'�� ������� ���ϴ� Ŭ����
// ---------------------
namespace Library_Jingyu
{
	// ������
	//
	// Parameter : ������� üũ�� ���μ��� �ڵ�. (����Ʈ : �ڱ��ڽ�)
	CCpuUsage_Process::CCpuUsage_Process(HANDLE hProcess)
	{
		// �Է¹��� ���μ��� �ڵ��� ���ٸ�, �ڱ� �ڽ��� ������� üũ.
		if (hProcess == INVALID_HANDLE_VALUE)
			m_hProcess = hProcess;

		// ���μ��� �� Ȯ��(�ھ� �� Ȯ��)
		// ���μ��� (exe) ������ ��� ��, CPU ���� ���� ���� ������� ���Ѵ�.
		SYSTEM_INFO SystemInfo;

		GetSystemInfo(&SystemInfo);
		m_iNumberOfProcessors = SystemInfo.dwNumberOfProcessors;

		// ������� �ʱ�ȭ
		m_fProcessTotal = 0;
		m_fProcessUser = 0;
		m_fProcessKernel = 0;

		m_ftProcess_LastKernel.QuadPart = 0;
		m_ftProcess_LastUser.QuadPart = 0;
		m_ftProcess_LastTime.QuadPart = 0;

		// ����, CPU ����� ����
		UpdateCpuTime();
			   
	}

	// ���μ��� ����� ����
	//
	// �ش� �Լ��� ȣ��Ǹ�, ������ ���μ����� Total, User, Kernel ������ ���ŵȴ�.
	void CCpuUsage_Process::UpdateCpuTime()
	{
		// 1. ����� �ð��� ���Ѵ� (100 ���뼼���� ����)
		// ���� �ð��� �޾Ƶ� ����		
		ULARGE_INTEGER NowTime;

		// ����ð� ���Ѵ�.
		GetSystemTimeAsFileTime((LPFILETIME)&NowTime);


		// 2. �ش� ���μ����� ����� �ð� ����		
		// �� ��°, �� ��° ���ڴ� ����, ���� �ð�. �ʿ������ ���ĸ� ���߰� �Ⱦ���.
		// �� �� ���ڷ� ���� ����
		ULARGE_INTEGER None;

		// Ŀ�� �ð�, ���� �ð��� �޾Ƶ� ����
		ULARGE_INTEGER Kernel;
		ULARGE_INTEGER User;

		// �ð� ���ϱ�
		GetProcessTimes(m_hProcess, (LPFILETIME)&None, (LPFILETIME)&None, (LPFILETIME)&Kernel, (LPFILETIME)&User);


		// 3. ������ ����� ���μ��� �ð����� ���� ���ؼ� ������ ���� �ð��� �������� ���
		// �� ����� ���� ������ �ð����� ������ ������� ���´�.
		ULONGLONG TimeDiff = NowTime.QuadPart - m_ftProcess_LastTime.QuadPart;
		ULONGLONG UserDiff = User.QuadPart - m_ftProcess_LastUser.QuadPart;
		ULONGLONG KernelDiff = Kernel.QuadPart - m_ftProcess_LastKernel.QuadPart;

		ULONGLONG Total = KernelDiff + UserDiff;

		m_fProcessTotal = (float)(Total / (double)m_iNumberOfProcessors / (double)TimeDiff * 100.0f);
		m_fProcessKernel = (float)(KernelDiff / (double)m_iNumberOfProcessors / (double)TimeDiff * 100.0f);
		m_fProcessUser = (float)(UserDiff / (double)m_iNumberOfProcessors / (double)TimeDiff * 100.0f);


		// 4. ��� �ð�, Ŀ�νð�, �����ð� ����
		m_ftProcess_LastTime = NowTime;
		m_ftProcess_LastKernel = Kernel;
		m_ftProcess_LastUser = User;
	}
}
