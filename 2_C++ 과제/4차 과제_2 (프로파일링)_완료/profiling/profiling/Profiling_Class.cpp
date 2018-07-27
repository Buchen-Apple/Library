#include "Profiling_Class.h"
#include <Windows.h>

namespace Library_Jingyu
{

	Profiling profile[PROFILING_SIZE];			// �������ϸ� ��ü
	LARGE_INTEGER Frequency;					// ���ļ� ���� ����
	LARGE_INTEGER count1;						// �ð� ���� ���� 1
	LARGE_INTEGER count2;						// �ð� ���� ���� 2

	int g_NowProfiling_Size = -1;					// �������ϸ� ��ü ���� üũ
#define MIN_SET 100000000

	Profiling::Profiling()
		:m_CallCount(0), m_StartTime(0), m_TotalTime(0), m_Average(0)
	{
		m_MaxTime[0] = 0;
		m_MaxTime[1] = 0;

		m_MInTime[0] = MIN_SET;
		m_MInTime[1] = MIN_SET;
	}

	// ���ļ� ���ϱ�	
	void FREQUENCY_SET()
	{
		// ���ļ� ���ϱ�	
		QueryPerformanceFrequency(&Frequency);
	}

	// �������ϸ� ����
	void BEGIN(const char* str)
	{
		QueryPerformanceCounter(&count1);
		int i = 0;
		bool bFlag = false;

		for (i = 0; i <= g_NowProfiling_Size; ++i)
		{
			if (strcmp(profile[i].m_Name, str) == 0)
			{
				bFlag = true;
				break;
			}
		}

		// ���� �Ҵ�� �������ϸ���ŭ �˻������� �̸��� ���ٸ�, �߰��� ������� �Ҵ�� ����� PROFILING_SIZE���� �۴ٸ�
		// ���ο� �������ϸ� ����
		if (bFlag == false && g_NowProfiling_Size < PROFILING_SIZE)
		{
			g_NowProfiling_Size++;
			strcpy_s(profile[g_NowProfiling_Size].m_Name, sizeof(profile[g_NowProfiling_Size].m_Name), str);	// �̸� ����
			profile[g_NowProfiling_Size].m_CallCount = 1;							// �ش� �Լ�(BEGIN ����, ���� BEGIN�� ȣ���� �Լ�)ȣ�� Ƚ�� ����
			profile[g_NowProfiling_Size].m_StartTime = (double)count1.QuadPart;		// ���� �ð� ����.

		}

		// �˻��� �̸��� �ִٸ�, �ش� �������ϸ��� ���� �ð� ����
		else if (bFlag == true)
		{
			profile[i].m_CallCount += 1;									// �ش� �Լ�(BEGIN ����, ���� BEGIN�� ȣ���� �Լ�)ȣ�� Ƚ�� ����
			profile[i].m_StartTime = (double)count1.QuadPart;				// ���� �ð� ����.
		}
	}

	// �������ϸ� ����
	void END(const char* str)
	{
		QueryPerformanceCounter(&count2);
		int i;
		bool bFlag = true;

		for (i = 0; strcmp(profile[i].m_Name, str) != 0; ++i)
		{
			if (i > g_NowProfiling_Size)
			{
				bFlag = false;
				break;
			}
		}

		// �˻��� �̸��� �ְ�, �� �������ϸ��� ���� �ð��� 0�� �ƴ϶��, ���� ����
		if (bFlag == true && profile[i].m_StartTime != 0)
		{
			bool bCallmulCheck = false;
			bool bTimeMulCheck = false;
			// BEGIN���� END������ �ð��� ����ũ�� ������ ������ ����.
			double dSecInterval = ((double)(count2.QuadPart - profile[i].m_StartTime) / Frequency.QuadPart) * 1000000;

			// �� �ð� ����
			profile[i].m_TotalTime += dSecInterval;

			// dSecInterval�� �ּҿ� �����ϴ��� �ִ뿡 �����ϴ��� ã�´�.
			// �������� �迭(�ּ�) üũ.
			// �������� �迭�� ���� ���. 
			if (profile[i].m_MInTime[0] > dSecInterval)
			{
				if (profile[i].m_MInTime[0] == MIN_SET)	// ����, ���� ȣ���̶��
				{
					profile[i].m_CallCount--;			// �� ���� 1 ����. ��ü�� �ƴ϶� �ƿ� ���� �ִ°��̱� ������ 1 �����ؾ� ��.	
					bCallmulCheck = true;				// �Ʒ� �ִ��� �������� �迭���� ���� dSecInterval���� �� �� �ֱ� ������ ���� ���ٴ� ���� üũ. �ߺ� ���� ����.
				}
				else		                            // ���� ȣ���� �ƴ϶��
				{
					profile[i].m_TotalTime -= dSecInterval;				// ���� ȣ���� �ƴϸ�, �� �ð����� ���� �ð�(dSecInterval)�� �ٽ� ����. ���� ȣ�� �ÿ��� �ð��� �E �ʿ䵵 ����. ���ʿ� �߰��� ���� ���� ������.
					profile[i].m_TotalTime += profile[i].m_MInTime[0];	// ������ [0] ��ġ(�������� �迭)�� �ð��� �� �ð��� ���Ѵ�. �̰� ���� �ּҷ� �� ���̱� ������.
					profile[i].m_MInTime[1] = profile[i].m_MInTime[0];	// ���� [0]�� �ð��� [1]�� �ű��.
					bTimeMulCheck = true;								// �Ʒ� �ִ��� �������� �迭���� ���� dSecInterval���� �� �� �ֱ� ������ �ð��� ���ٴ� ���� üũ. �ߺ� ���� ����
				}

				profile[i].m_MInTime[0] = dSecInterval;		// ���� ȣ�� ���ζ� �������, ���� �ð�(dSecInterval)�� [0]�� �ִ´�. �̰� �Ⱦ��� ���� ��.	

			}

			// �������� �ʴ� �迭�� ���� ���, �� �ð��� 1�� �迭�� �����Ѵ�.
			else if (profile[i].m_MInTime[1] >= dSecInterval)
				profile[i].m_MInTime[1] = dSecInterval;

			// ���� �ִ� üũ
			// �������� �迭�� ���� ���, �ð��� ���� ȣ��Ƚ�� ����. �׸��� ������ �ð��� 0�� �迭�� ����.
			if (profile[i].m_MaxTime[0] < dSecInterval)
			{
				if (profile[i].m_MaxTime[0] == 0)
				{
					if (bCallmulCheck == false)
					{
						profile[i].m_CallCount--;
					}
				}
				else
				{
					if (bTimeMulCheck == false)
						profile[i].m_TotalTime -= dSecInterval;
					profile[i].m_TotalTime += profile[i].m_MaxTime[0];
					profile[i].m_MaxTime[1] = profile[i].m_MaxTime[0];
				}

				profile[i].m_MaxTime[0] = dSecInterval;
			}

			// �������� �ʴ� �迭�� ���� ���, �� �ð��� 1�� �迭�� �����Ѵ�.
			else if (profile[i].m_MaxTime[1] <= dSecInterval)
				profile[i].m_MaxTime[1] = dSecInterval;

			profile[i].m_StartTime = 0;	// StartTime�� 0���� ����, �ش� BEGIN�� �����ٴ°� �˷���.
		}
		// �˻��� �̸��� ����.
		else if (bFlag == false)
		{
			printf("END(). %s �������� �ʴ� �Լ� �̸��Դϴ�\n", str);
			exit(1);
		}
		// �˻��� �̸��� �ִµ�, BEGIN���� �ƴ϶��
		else if (profile[i].m_StartTime == 0)
		{
			printf("END(). %s BEGIN������ ���� �Լ��Դϴ�.\n", str);
			exit(1);

		}

	}

	// �������ϸ� ��ü ����
	void RESET()
	{
		for (int i = 0; i <= g_NowProfiling_Size; ++i)
		{
			profile[i].m_Name[0] = '\0';
			profile[i].m_TotalTime = 0;
			profile[i].m_MInTime[0] = 100000000;
			profile[i].m_MInTime[1] = 100000000;
			profile[i].m_MaxTime[0] = 0;
			profile[i].m_MaxTime[1] = 0;
			profile[i].m_StartTime = 0;
		}
		g_NowProfiling_Size = -1;
	}

	// �������ϸ� ���� ��ü ����
	void PROFILING_SHOW()
	{
		for (int i = 0; i <= g_NowProfiling_Size; ++i)
		{
			printf("\n�̸� : %s\n", profile[i].m_Name);
			printf("ȣ�� Ƚ�� :%d\n", profile[i].m_CallCount);
			printf("�� �ð� : %0.6lf\n", profile[i].m_TotalTime);
			printf("��� : %0.6lf\n", profile[i].m_TotalTime / profile[i].m_CallCount);
			printf("�ּ� �ð�1 / 2 : %0.6lf / %0.6lf\n", profile[i].m_MInTime[0], profile[i].m_MInTime[1]);
			printf("�ִ� �ð�1 / 2 : %0.6lf / %0.6lf\n", profile[i].m_MaxTime[0], profile[i].m_MaxTime[1]);
		}
	}

	// �������ϸ��� ���Ϸ� ���. Profiling�� friend
	void PROFILING_FILE_SAVE()
	{
		FILE* wStream;			// ���� ��Ʈ��
		size_t iFileCheck;	// ��Ʈ�� ���� ���� üũ
		iFileCheck = fopen_s(&wStream, "Profiling.txt", "wt");
		if (iFileCheck != 0)
			return;

		fprintf_s(wStream, "%15s  |%12s  |%11s   |%11s   |%11s |", "Name", "Average", "Min", "Max", "Call");
		fprintf_s(wStream, "\n---------------------------------------------------------------------------- ");

		for (int i = 0; i <= g_NowProfiling_Size; ++i)
		{
			// �ּ��� [1]�� �迭�� ���� �� �� ���ٸ�, [0]�� ���� [1]�� �ű��.
			// ���ÿ� TotalTime�� �ð��� �߰��ϰ� m_CallCount�� Ƚ���� �߰��Ѵ�.
			if (profile[i].m_MInTime[1] == 100000000)
			{
				profile[i].m_MInTime[1] = profile[i].m_MInTime[0];
				profile[i].m_TotalTime += profile[i].m_MInTime[1];
				profile[i].m_CallCount++;
			}

			if (profile[i].m_MaxTime[1] == 0)
			{
				profile[i].m_MaxTime[1] = profile[i].m_MaxTime[0];
				profile[i].m_TotalTime += profile[i].m_MaxTime[1];
				profile[i].m_CallCount++;
			}

			fprintf_s(wStream, "\n%16s |%11.3lf�� |%11.3lf�� |%11.3lf�� |%11d |", profile[i].m_Name, profile[i].m_TotalTime / profile[i].m_CallCount, profile[i].m_MInTime[1], profile[i].m_MaxTime[1], profile[i].m_CallCount);
		}

		fclose(wStream);

	}

}