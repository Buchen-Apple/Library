// 1. ������ ����

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


void Gatcha()
{
	srand((unsigned int)time(NULL));
	// 1. ��ü �����۵��� ���� �� ���� ����.
	int iTotal = 0;
	int iRandSave;
	int iPrev = 0;
	int iNext;
	int i;
	int iCount = 1;

	for (i = 0; i < ITEM_COUNT; ++i)
	{
		iTotal += g_Gatcha[i].m_Rate;
	}		

	while (1)
	{
		// 2. rand() �Լ��� Ȯ���� ����
		// ���⼭ Ȯ���� 1/100 �� �ƴϸ�, 1/���պ��� ��.
		iRandSave = (rand() % iTotal) + 1;

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

	Gatcha();

	return 0;
}