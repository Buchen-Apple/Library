
#ifndef __PROFILING_CLASS_H__
#define __PROFILING_CLASS_H__

#include <Windows.h>

namespace Library_Jingyu
{
	class Profiling
	{
	private:
		char m_Name[30];
		int m_CallCount;
		double m_StartTime;

		double m_TotalTime;
		double m_Average;
		double m_MaxTime[2];
		double m_MInTime[2];

		LARGE_INTEGER m_StartCount;						// BEGIN���� ���. QueryPerformanceCounter�� ���۽ð��� ���� �� ���
		LARGE_INTEGER m_EndCount;						// END���� ���. QueryPerformanceCounter�� ���۽ð��� ���� �� ���

	public:
		Profiling();
		friend void BEGIN(const char*);
		friend void END(const char*);
		friend void RESET();
		friend void PROFILING_SHOW();
		friend void PROFILING_FILE_SAVE();	// ���ڷ�, ��������� ID�� ����� �迭�� Count�޾ƿ�.
	};

	// ���ļ� ���ϱ�	
	void FREQUENCY_SET();

	// �������ϸ� ����. Profiling�� friend
	void BEGIN(const char* str);

	// �������ϸ� ����. Profiling�� friend
	void END(const char* str);

	// �������ϸ� ��ü ����. Profiling�� friend
	void RESET();

	// �������ϸ� ���� ����. Profiling�� friend
	void PROFILING_SHOW();

	// �������ϸ��� ���Ϸ� ���. Profiling�� friend
	void PROFILING_FILE_SAVE();


#endif // !__PROFILING_CLASS_H__
}
