//2. ���̺� ���� ���
#include "stdio.h"
#include "stdlib.h"
#include "time.h"
#include "string.h"
#include "conio.h"

#define ITEM_COUNT 9

struct st_ITEM
{
	char	Name[30];
	int	Rate;		// �Ϲ� ���� �̱�� ���� �뵵
	int	WinTime;	// �� �������� ���� �̱� ȸ��.
					// 0 �̸� �Ϲ� ������
					// 0 �� �ƴϸ� �� ȸ������ ����
};

st_ITEM g_Gatcha[] = {
	{ "Į",						 20, 0 },
	{ "����",					 20, 0 },
	{ "�Ź�",					 20, 0 },
	{ "����",					 20, 0 },
	{ "�ʰ��·����",			  5, 0 },
	{ "�ʰ��¹���",				  5, 0 },
	{ "�ʰ��·�����Ź�1", 1, 50 },
	{ "�ʰ��·�����Ź�2", 1, 51 },
	{ "�ʰ��·�����Ź�3", 1, 10 }

// ������ 3���� �������� �Ϲ� Ȯ���δ� ������ ������
// �ڿ� �Էµ� WinTime ȸ������ 100% �� ����.
};


void Gatcha()
{
	srand((unsigned int)time(NULL));

	int i;
	int iTotal = 0;
	int iRandSave;
	int iCount = 0;
	int iPrev;
	int iNext;
	bool bSeedCheck = false;

	// 1. ��ü �����۵��� ���� �� ���� ����.
	// ��, WinTime �� ������ �������� Ȯ���� ��� ��ü�� �ƴϱ� ������ ����.
	for (i = 0; i < ITEM_COUNT; ++i)
	{
		if (g_Gatcha[i].WinTime == 0)
			iTotal += g_Gatcha[i].Rate;
	}

	while (1)
	{
		_getch();
		iNext = 0;
		iPrev = 0;
		bSeedCheck = false;

		while (1)
		{
			// �̱� ȸ�� ����. (�̴� ���������� ��� �Ǿ�� ��)
			iCount++;

			// 2. �� �̱� ȸ���� ���� ���� �������� �ִ��� Ȯ��
			// WinTime �� iCount �� ���� �������� ã�´�.
			// �ִٸ�.. �� �������� �̰� �ߴ�.
			for (i = 0; i < ITEM_COUNT; ++i)
			{
				if (g_Gatcha[i].WinTime == iCount)
				{
					bSeedCheck = true;
					break;
				}
			}

			if (bSeedCheck == true)
				break;

			// 3. rand() �Լ��� Ȯ���� ����
			// ���⼭ Ȯ���� 1/100 �� �ƴϸ�, 1/���պ��� ��.
			iRandSave = (rand() % iTotal) + 1;

			// 4. ��ü ������ ���̺��� ���鼭
			// ������ ���� Rand ���� �ش� ������ �������� ã�´�.
			for (i = 0; i < ITEM_COUNT; ++i)
			{
				iNext += g_Gatcha[i].Rate;

				if (iPrev < iRandSave && iRandSave <= iNext)
					break;

				iPrev += g_Gatcha[i].Rate;
			}


			// 5. �̱� ȸ���� �ʱ�ȭ �ؾ����� �Ǵ��Ͽ� �ʱ�ȭ.
			if (iCount == 100)
				iCount = 0;

			break;
		}
		printf("%02d. %s (%d / %d)\n", iCount, g_Gatcha[i].Name, iTotal, g_Gatcha[i].Rate);
	}
}

int main(){

	Gatcha();

	return 0;
}

