#include "stdafx.h"
#include "Profiling.h"

Profiling profile[PROFILING_SIZE];			// �������ϸ� ��ü
LARGE_INTEGER Frequency;					// ���ļ� ���� ����
LARGE_INTEGER count1;						// �ð� ���� ���� 1
LARGE_INTEGER count2;						// �ð� ���� ���� 2

int g_NowProfiling_Size = -1;					// �������ϸ� ��ü ���� üũ
#define MIN_SET 100000000

Profiling::Profiling()
	:m_CallCount(0), m_StartTime(0), m_Average(0)
{
	m_TotalTime.QuadPart = 0;
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
		profile[g_NowProfiling_Size].m_CallCount = 1;														// �ش� �Լ�(BEGIN ����, ���� BEGIN�� ȣ���� �Լ�)ȣ�� Ƚ�� ����
		profile[g_NowProfiling_Size].m_StartTime = count1.QuadPart;											// ���� �ð� ����.

	}

	// �˻��� �̸��� �ִٸ�, �ش� �������ϸ��� ���� �ð� ����
	else if (bFlag == true)
	{
		profile[i].m_CallCount += 1;									// �ش� �Լ�(BEGIN ����, ���� BEGIN�� ȣ���� �Լ�)ȣ�� Ƚ�� ����
		profile[i].m_StartTime = count1.QuadPart;						// ���� �ð� ����.
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
		bool bMinCheck = false;

		// BEGIN���� END������ �ð��� QueryPerformanceCounter�� �״�� ����. ����ũ�� ������� �ٲٴ°�, ����� �� �Ѵ�.
		profile[i].m_TotalTime.QuadPart += count2.QuadPart - profile[i].m_StartTime;
		long long loSecInterval = count2.QuadPart - profile[i].m_StartTime;

		// �ּҿ� �����ϴ��� �ִ뿡 �����ϴ��� ã�´�.
		// �ּҿ� ���� ���
		if (profile[i].m_MInTime[0] > loSecInterval || profile[i].m_MInTime[1] > loSecInterval)
		{
			// �ּ� �߿����� 0���� ���� �ϴ� ���
			if (profile[i].m_MInTime[0] > loSecInterval)
			{
				profile[i].m_TotalTime.QuadPart -= loSecInterval;			// ���� �ð��� ����

				if (profile[i].m_MInTime[0] == MIN_SET)						// [0]�� ���ʷ� �ִ°Ŷ��
					profile[i].m_CallCount--;								// �ݼ��� ���ҽ�Ų��. �߰��Ѱ� �ϳ��� �����ϱ�!

				else															// ���ʷ� [0]�� ���°� �ƴ϶��			
					profile[i].m_TotalTime.QuadPart += profile[i].m_MInTime[0];	// [0]�� �ð��� �ִ´�.

				profile[i].m_MInTime[1] = profile[i].m_MInTime[0];				// ���� [0]�� [1]�� ����
				profile[i].m_MInTime[0] = loSecInterval;						// ���� �ð�(loSecInterval)�� [0]���ٰ� �ִ´�.
			}

			// �ּ� �߿����� 1���� ���� �ϴ� ���
			else if (profile[i].m_MInTime[1] > loSecInterval)
				profile[i].m_MInTime[1] = loSecInterval;						// ���� �ð�(loSecInterval)�� [1]���ٰ� �ִ´�
		}

		// �ִ뿡 ���� ���
		else if (profile[i].m_MaxTime[0] < loSecInterval || profile[i].m_MaxTime[1] < loSecInterval)
		{
			// �ִ� �߿����� 0���� ���� �ϴ� ���
			if (profile[i].m_MaxTime[0] < loSecInterval)
			{
				profile[i].m_TotalTime.QuadPart -= loSecInterval;			// ���� �ð��� ����

				if (profile[i].m_MaxTime[0] == 0)							// [0]�� ���ʷ� �ִ°Ŷ��
					profile[i].m_CallCount--;								// �ݼ��� ���ҽ�Ų��. �߰��Ѱ� �ϳ��� �����ϱ�!

				else                                                            // ���ʷ� [0]�� ���°� �ƴ϶��	
					profile[i].m_TotalTime.QuadPart += profile[i].m_MaxTime[0];	// [0] �ð��� �ִ´�.

				profile[i].m_MaxTime[1] = profile[i].m_MaxTime[0];				// ���� [0]�� [1]�� ����
				profile[i].m_MaxTime[0] = loSecInterval;						// ���� �ð�(loSecInterval)�� [0]���ٰ� �ִ´�.
			}
			// �ִ� �߿����� 1���� ���� �ϴ� ���
			else if (profile[i].m_MaxTime[1] < loSecInterval)
				profile[i].m_MaxTime[1] = loSecInterval;						// ���� �ð�(loSecInterval)�� [1]���ٰ� �ִ´�.
		}

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
		profile[i].m_TotalTime.QuadPart = 0;
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
		printf("�� �ð� : %0.6lf\n", (double)profile[i].m_TotalTime.QuadPart / (double)Frequency.QuadPart * 1000000);
		printf("��� : %0.6lf\n", (double)profile[i].m_TotalTime.QuadPart / (double)Frequency.QuadPart * 1000000 / profile[i].m_CallCount);
		printf("�ּ� �ð�1 / 2 : %0.6lf / %0.6lf\n", ((double)profile[i].m_MInTime[0] / (double)Frequency.QuadPart) * 1000000, ((double)profile[i].m_MInTime[1] / (double)Frequency.QuadPart) * 1000000);
		printf("�ִ� �ð�1 / 2 : %0.6lf / %0.6lf\n", ((double)profile[i].m_MaxTime[0] / (double)Frequency.QuadPart) * 1000000, (double)(profile[i].m_MaxTime[1] / (double)Frequency.QuadPart) * 1000000);
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
		if (profile[i].m_MInTime[1] == MIN_SET)
		{
			profile[i].m_MInTime[1] = profile[i].m_MInTime[0];
			profile[i].m_TotalTime.QuadPart += profile[i].m_MInTime[1];
			profile[i].m_CallCount++;
		}

		// ���������� �ִ��� [1]�� �迭�� ���� �� �� ���ٸ�, [0]�� ���� [1]�� �ű��.
		if (profile[i].m_MaxTime[1] == 0)
		{
			profile[i].m_MaxTime[1] = profile[i].m_MaxTime[0];
			profile[i].m_TotalTime.QuadPart += profile[i].m_MaxTime[1];
			profile[i].m_CallCount++;
		}
		// ����ũ�� ������ ������ ���� �� ���
		fprintf_s(wStream, "\n%16s |%11.3lf�� |%11.3lf�� |%11.3lf�� |%11d |", profile[i].m_Name, ((double)profile[i].m_TotalTime.QuadPart / (double)Frequency.QuadPart) * 1000000 / profile[i].m_CallCount, ((double)profile[i].m_MInTime[1] / (double)Frequency.QuadPart) * 1000000, ((double)profile[i].m_MaxTime[1] / (double)Frequency.QuadPart) * 1000000, profile[i].m_CallCount);
	}

	fclose(wStream);

}

