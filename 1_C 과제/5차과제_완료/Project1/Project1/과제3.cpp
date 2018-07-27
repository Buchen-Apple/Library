// 3. ���� ���̺�
// ù��° ��¥ ������ �̱� �κ��� ������ rand() �Լ��� ��ü�� �츮���� ������ �����.
// �츮�� ����� ������ ������ ���� �ȿ��� �ߺ����� ���� ������ �ʴ� ���� �Լ�.

#include "stdio.h"
#include "stdlib.h"
#include "time.h"
#include "string.h"
#include "conio.h"

#define ITEM_COUNT 7

struct st_ITEM
{
	char m_Name[30];
	int	 m_Rate;		// �������� ���� ����
};

st_ITEM g_Gatcha[] =
{
	{ "Į",						20 },
	{ "����",					20 },
	{ "�Ź�",					20 },
	{ "����",					20 },
	{ "�ʰ��·����",			 5 },
	{ "�ʰ��¹���",				 5 },
	{ "�ʰ��·�����Ź�",  1 }
};


int RandTable[1000];		// ���� ������ �ִ� 1000 ���� ����.

int TableRand(int Max)
{
	
	srand((unsigned int)time(NULL));

	int iCount = 0;
	static int iUseIndexCount = 0;

	int TempIndex1;
	int TempIndex2;
	int Temp;

	// ���� ���� �迭�� ��� ����ϸ�, iUseIndexCount�� ���� 0���� �����. �̴� �ٽ� �����ϱ� �����̴�.
	if (iUseIndexCount >= 1000)
		iUseIndexCount = 0;

	// iUseIndexCount�� 0�̸� ���� �ٽ� ������ �� �����Ѵ�.
	if(iUseIndexCount == 0)
	{	// 1000���� �迭�� 1~Max������ ���� ����.
		// ���� Max���� Ŀ���� �ٽ� 1���� �ִ´�.
		// ���� Max�� 170�̶��, 1000���� �迭���� 1...169,170,1,2,3...�� ���� �ݺ��ؼ� ����.
		for (int i = 0; i < 1000; ++i)
		{
			RandTable[i] = iCount + 1;
			if (iCount + 1 == Max)
				iCount = -1;

			iCount++;
		}

		// 100������ RandTable ���� ������ �����Ѵ�.
		for (int i = 0; i < 100; ++i)
		{
			TempIndex1 = rand() % 1000;
			TempIndex2 = rand() % 1000;

			Temp = RandTable[TempIndex1];
			RandTable[TempIndex1] = RandTable[TempIndex2];
			RandTable[TempIndex2] = Temp;
		}
	}

	return RandTable[iUseIndexCount++];
}


void Gatcha()
{
	srand((unsigned int)time(NULL));
	// 1. ��ü �����۵��� ���� �� ���� ����.
	int iTotal = 0;
	int iRandSave;
	int iPrev = 0;
	int iNext = 0;
	int i;
	int iCount = 1;

	for (i = 0; i < ITEM_COUNT; ++i)
	{
		iTotal += g_Gatcha[i].m_Rate;
	}

	while (1)
	{
		// 2. �� �������� �Լ��� ȣ���� ��, �� ���� �����Ѵ�.
		iRandSave = TableRand(iTotal);

		// 3. ��ü ������ ���̺��� ���鼭
		// ������ ���� Rand ���� �ش� ������ �������� ã�´�.
		for (i = 0; i < ITEM_COUNT; ++i)
		{
			iNext += g_Gatcha[i].m_Rate;

			if (iPrev < iRandSave && iRandSave <= iNext)
				break;

			iPrev += g_Gatcha[i].m_Rate;
		}

		printf("%d. %s (%d / %d)\n", iCount, g_Gatcha[i].m_Name, iTotal, g_Gatcha[i].m_Rate);
		iCount++;
		iNext = 0;
		iPrev = 0;

		_getch();
	}
}

int main()
{

	// ���� �Լ� ȣ��
	Gatcha();

	return 0;
}