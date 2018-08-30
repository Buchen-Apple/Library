#include "pch.h"
#include "Profiling_Class.h"
#include "CrashDump\CrashDump.h"
#include <Windows.h>

namespace Library_Jingyu
{

#define PROFILING_SIZE		50			// ������ ���� �� ���� �±װ� ��������.
#define MIN_SET				100000000	// ���� ���� ��, Min�� ��. üũ�뵵.
#define MAX_PROFILE_COUNT	1000		// �ִ� �� ���� �����尡 �������ϸ� ��������

	// ������ ���� TLS�� ����Ǵ� ����ü.
	struct stThread_Profile
	{
		Profiling m_Profile[PROFILING_SIZE];
		int m_NowProfiling_Size = -1;
		LONG ThreadID;
	};

	// �� �����忡 �����Ҵ�Ǵ� stThread_Profile�� �ּ� �������. �� 1000���� ������ ����.
	stThread_Profile* g_pThread_Profile_Array[MAX_PROFILE_COUNT];
	LONG g_pThread_Profile_Array_Count = -1;
	
	CCrashDump* g_ProfileDump = CCrashDump::GetInstance();		// ���� ��ü
	LARGE_INTEGER g_Frequency;									// ���ļ� ���� ����
	DWORD g_TLSIndex;											// ��� �����尡 ����ϴ� TLS �ε���

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
		QueryPerformanceFrequency(&g_Frequency);

		// ������ �ε��� �ϳ� ���α�. ��� �������� TLS�� �� �ε��� ���.
		g_TLSIndex = TlsAlloc();
		if (g_TLSIndex == TLS_OUT_OF_INDEXES)
		{
			DWORD Error = GetLastError();
			g_ProfileDump->Crash();
		}
	}

	// �������ϸ� ����
	void BEGIN(const char* str)
	{
		// �������� TLS ��������
		stThread_Profile* MyProfile = (stThread_Profile*)TlsGetValue(g_TLSIndex);

		// ����, nullptr�̶�� ���� �Ҵ��Ѵ�.
		if (MyProfile == nullptr)
		{
			MyProfile = new stThread_Profile;

			// �Ҵ� ��, �� TLS�� �����Ѵ�.
			if (TlsSetValue(g_TLSIndex, MyProfile) == 0)
			{
				DWORD Error = GetLastError();
				g_ProfileDump->Crash();
			}

			LONG Temp = InterlockedIncrement(&g_pThread_Profile_Array_Count);
			MyProfile->ThreadID = GetThreadId(GetCurrentThread());
			g_pThread_Profile_Array[Temp] = MyProfile;
		}

		
		// �迭 ����, �ش� ��Ʈ���� �����ϴ��� üũ
		int i = 0;
		bool bFlag = false;

		for (i = 0; i <= MyProfile->m_NowProfiling_Size; ++i)
		{
			if (strcmp(MyProfile->m_Profile[i].m_Name, str) == 0)
			{
				bFlag = true;
				break;
			}
		}

		// ���� �Ҵ�� �������ϸ���ŭ �˻������� �̸��� ���ٸ�, �߰��� ������� �Ҵ�� ����� PROFILING_SIZE���� �۴ٸ�
		// ���ο� �������ϸ� ����
		if (bFlag == false && MyProfile->m_NowProfiling_Size < PROFILING_SIZE)
		{
			int NowSize = ++MyProfile->m_NowProfiling_Size;
			Profiling* safe_Profile = &MyProfile->m_Profile[NowSize];
			
			strcpy_s(safe_Profile->m_Name, sizeof(safe_Profile->m_Name), str);			// �̸� ����
			safe_Profile->m_CallCount = 1;												// BGEIN ȣ�� Ƚ�� ����
			QueryPerformanceCounter(&safe_Profile->m_StartCount);
			safe_Profile->m_StartTime = (double)safe_Profile->m_StartCount.QuadPart;	// ���� �ð� ����.		
		}

		// �˻��� �̸��� �ִٸ�, �ش� �������ϸ��� ���� �ð� ����
		else if (bFlag == true)
		{
			Profiling* safe_Profile = &MyProfile->m_Profile[i];

			safe_Profile->m_CallCount++;												// BEGIN ȣ�� Ƚ�� ����
			QueryPerformanceCounter(&safe_Profile->m_StartCount);
			safe_Profile->m_StartTime = (double)safe_Profile->m_StartCount.QuadPart;	// ���� �ð� ����		
		}
	}

	// �������ϸ� ����
	void END(const char* str)
	{
		// ī��Ʈ �� ���صα�.
		LARGE_INTEGER TempEndCount;
		QueryPerformanceCounter(&TempEndCount);

		// �������� TLS ��������
		stThread_Profile* MyProfile = (stThread_Profile*)TlsGetValue(g_TLSIndex);

		// ����, nullptr�̶�� ����.
		if (MyProfile == nullptr)
		{
			DWORD Error = GetLastError();
			g_ProfileDump->Crash();
		}		

		int i;
		bool bFlag = true;

		for (i = 0; strcmp(MyProfile->m_Profile[i].m_Name, str) != 0; ++i)
		{
			if (i > MyProfile->m_NowProfiling_Size)
			{
				bFlag = false;
				break;
			}
		}

		// �˻��� �̸��� �ְ�, �� �������ϸ��� ���� �ð��� 0�� �ƴ϶��, ���� ����
		if (bFlag == true && MyProfile->m_Profile[i].m_StartTime != 0)
		{
			Profiling* safe_Profile = &MyProfile->m_Profile[i];

			safe_Profile->m_EndCount = TempEndCount;

			bool bCallmulCheck = false;
			bool bTimeMulCheck = false;

			// BEGIN���� END������ �ð��� ����ũ�� ������ ������ ����.
			double dSecInterval = ((double)(safe_Profile->m_EndCount.QuadPart - safe_Profile->m_StartTime) / g_Frequency.QuadPart) * 1000000;

			// �� �ð� ����
			safe_Profile->m_TotalTime += dSecInterval;

			// dSecInterval�� �ּҿ� �����ϴ��� �ִ뿡 �����ϴ��� ã�´�.
			// �������� �迭(�ּ�) üũ.
			// �������� �迭�� ���� ���. 
			if (safe_Profile->m_MInTime[0] > dSecInterval)
			{
				if (safe_Profile->m_MInTime[0] == MIN_SET)	// ����, ���� ȣ���̶��
				{
					safe_Profile->m_CallCount--;			// �� ���� 1 ����. ��ü�� �ƴ϶� �ƿ� ���� �ִ°��̱� ������ 1 �����ؾ� ��.	
					bCallmulCheck = true;				// �Ʒ� �ִ��� �������� �迭���� ���� dSecInterval���� �� �� �ֱ� ������ ���� ���ٴ� ���� üũ. �ߺ� ���� ����.
				}
				else		                            // ���� ȣ���� �ƴ϶��
				{
					safe_Profile->m_TotalTime -= dSecInterval;				// ���� ȣ���� �ƴϸ�, �� �ð����� ���� �ð�(dSecInterval)�� �ٽ� ����. ���� ȣ�� �ÿ��� �ð��� �E �ʿ䵵 ����. ���ʿ� �߰��� ���� ���� ������.
					safe_Profile->m_TotalTime += safe_Profile->m_MInTime[0];	// ������ [0] ��ġ(�������� �迭)�� �ð��� �� �ð��� ���Ѵ�. �̰� ���� �ּҷ� �� ���̱� ������.
					safe_Profile->m_MInTime[1] = safe_Profile->m_MInTime[0];	// ���� [0]�� �ð��� [1]�� �ű��.
					bTimeMulCheck = true;								// �Ʒ� �ִ��� �������� �迭���� ���� dSecInterval���� �� �� �ֱ� ������ �ð��� ���ٴ� ���� üũ. �ߺ� ���� ����
				}

				safe_Profile->m_MInTime[0] = dSecInterval;		// ���� ȣ�� ���ζ� �������, ���� �ð�(dSecInterval)�� [0]�� �ִ´�. �̰� �Ⱦ��� ���� ��.	

			}

			// �������� �ʴ� �迭�� ���� ���, �� �ð��� 1�� �迭�� �����Ѵ�.
			else if (safe_Profile->m_MInTime[1] >= dSecInterval)
				safe_Profile->m_MInTime[1] = dSecInterval;

			// ���� �ִ� üũ
			// �������� �迭�� ���� ���, �ð��� ���� ȣ��Ƚ�� ����. �׸��� ������ �ð��� 0�� �迭�� ����.
			if (safe_Profile->m_MaxTime[0] < dSecInterval)
			{
				if (safe_Profile->m_MaxTime[0] == 0 && bCallmulCheck == false)
					safe_Profile->m_CallCount--;		

				else
				{
					if (bTimeMulCheck == false)
						safe_Profile->m_TotalTime -= dSecInterval;

					safe_Profile->m_TotalTime += safe_Profile->m_MaxTime[0];
					safe_Profile->m_MaxTime[1] = safe_Profile->m_MaxTime[0];
				}

				safe_Profile->m_MaxTime[0] = dSecInterval;
			}

			// �������� �ʴ� �迭�� ���� ���, �� �ð��� 1�� �迭�� �����Ѵ�.
			else if (safe_Profile->m_MaxTime[1] <= dSecInterval)
				safe_Profile->m_MaxTime[1] = dSecInterval;

			safe_Profile->m_StartTime = 0;	// StartTime�� 0���� ����, �ش� BEGIN�� �����ٴ°� �˷���.
		}
		// �˻��� �̸��� ����.
		else if (bFlag == false)
		{
			printf("END(). [%s] Not Find Name\n", str);
			g_ProfileDump->Crash();
		}
		// �˻��� �̸��� �ִµ�, BEGIN���� �ƴ϶��
		else if (MyProfile->m_Profile[i].m_StartTime == 0)
		{
			printf("END(). [%s] Not Begin Name\n", str);
			g_ProfileDump->Crash();

		}

	}

	// �������ϸ� ��ü ����
	void RESET()
	{
		// �������� TLS ��������
		stThread_Profile* MyProfile = (stThread_Profile*)TlsGetValue(g_TLSIndex);

		// ����, nullptr�̶�� ����.
		if (MyProfile == nullptr)
		{
			DWORD Error = GetLastError();
			g_ProfileDump->Crash();
		}

		for (int i = 0; i <= MyProfile->m_NowProfiling_Size; ++i)
		{
			MyProfile->m_Profile[i].m_Name[0] = '\0';
			MyProfile->m_Profile[i].m_TotalTime = 0;
			MyProfile->m_Profile[i].m_MInTime[0] = 100000000;
			MyProfile->m_Profile[i].m_MInTime[1] = 100000000;
			MyProfile->m_Profile[i].m_MaxTime[0] = 0;
			MyProfile->m_Profile[i].m_MaxTime[1] = 0;
			MyProfile->m_Profile[i].m_StartTime = 0;
		}
		MyProfile->m_NowProfiling_Size = -1;
	}

	// �������ϸ� ���� ��ü ����
	void PROFILING_SHOW()
	{
		// �������� TLS ��������
		stThread_Profile* MyProfile = (stThread_Profile*)TlsGetValue(g_TLSIndex);

		// ����, nullptr�̶�� ����.
		if (MyProfile == nullptr)
		{
			DWORD Error = GetLastError();
			g_ProfileDump->Crash();
		}

		for (int i = 0; i <= MyProfile->m_NowProfiling_Size; ++i)
		{
			printf("\n�̸� : %s\n", MyProfile->m_Profile[i].m_Name);
			printf("ȣ�� Ƚ�� :%d\n", MyProfile->m_Profile[i].m_CallCount);
			printf("�� �ð� : %0.6lf\n", MyProfile->m_Profile[i].m_TotalTime);
			printf("��� : %0.6lf\n", MyProfile->m_Profile[i].m_TotalTime / MyProfile->m_Profile[i].m_CallCount);
			printf("�ּ� �ð�1 / 2 : %0.6lf / %0.6lf\n", MyProfile->m_Profile[i].m_MInTime[0], MyProfile->m_Profile[i].m_MInTime[1]);
			printf("�ִ� �ð�1 / 2 : %0.6lf / %0.6lf\n", MyProfile->m_Profile[i].m_MaxTime[0], MyProfile->m_Profile[i].m_MaxTime[1]);
		}
	}

	// �������ϸ��� ���Ϸ� ���. Profiling�� friend
	void PROFILING_FILE_SAVE()
	{
		// ���� ��Ʈ��
		FILE* wStream;		

		// ��Ʈ�� ���� ���� üũ
		if (fopen_s(&wStream, "Profiling.txt", "wt") != 0)
			return;

		for (int i = 0; i <= g_pThread_Profile_Array_Count; ++i)
		{
			Profiling* NowSaveProfile = g_pThread_Profile_Array[i]->m_Profile;
			LONG TempSize = g_pThread_Profile_Array[i]->m_NowProfiling_Size;
			LONG ThreadID = g_pThread_Profile_Array[i]->ThreadID;

			// �� ���������� ���� ���� �����Ѵ�.
			fprintf_s(wStream, "\n============================================================================\n ");
			fprintf_s(wStream, "%13s	|%30s  |%12s  |%13s   |%13s   |%13s |", "ThreadID", "Name", "Average", "Min", "Max", "Call");
			fprintf_s(wStream, "\n------------------------------------------------------------------------------------------------------------------ ");

			for (int i = 0; i <= TempSize; ++i)
			{
				// �ּ��� [1]�� �迭�� ���� �� �� ���ٸ�, [0]�� ���� [1]�� �ű��.
				// ���ÿ� TotalTime�� �ð��� �߰��ϰ� m_CallCount�� Ƚ���� �߰��Ѵ�.
				if (NowSaveProfile[i].m_MInTime[1] == 100000000)
				{
					NowSaveProfile[i].m_MInTime[1] = NowSaveProfile[i].m_MInTime[0];
					NowSaveProfile[i].m_TotalTime += NowSaveProfile[i].m_MInTime[1];
					NowSaveProfile[i].m_CallCount++;
				}

				// �ִ뵵 ��������.
				if (NowSaveProfile[i].m_MaxTime[1] == 0)
				{
					NowSaveProfile[i].m_MaxTime[1] = NowSaveProfile[i].m_MaxTime[0];
					NowSaveProfile[i].m_TotalTime += NowSaveProfile[i].m_MaxTime[1];
					NowSaveProfile[i].m_CallCount++;
				}

				fprintf_s(wStream, "\n%13d	|%31s |%11.3lf�� |%11.3lf�� |%11.3lf�� |%11d |",
					ThreadID, NowSaveProfile[i].m_Name, NowSaveProfile[i].m_TotalTime / NowSaveProfile[i].m_CallCount,
					NowSaveProfile[i].m_MInTime[1], NowSaveProfile[i].m_MaxTime[1], NowSaveProfile[i].m_CallCount);
			}

			// 1�� ������ ���� �Ϸ�Ǿ����� �Ϸ�ǥ�� ����
			fprintf_s(wStream, "\n============================================================================\n\n ");

			// ������ ������ ��������
			delete g_pThread_Profile_Array[i];
		}

		// �ε��� �ݳ�
		if (TlsFree(g_TLSIndex) == 0)
		{
			DWORD Error = GetLastError();
			g_ProfileDump->Crash();
		}

		fclose(wStream);
	}

}