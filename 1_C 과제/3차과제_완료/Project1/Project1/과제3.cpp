// ������
/*
1. �Ǻ��� L,R���
2. Left�� ->�� �̵�. Left > �Ǻ��̰ų� Left�ε��� == Right�ε��� �� �� ����
3. Right�� <-�� �̵�. Right < �Ǻ��̰ų�, Right�ε��� == Pivot�ε��� �� �� ����

4. 2, 3�� �Ϸ�� ��
-> Right�ε��� > Left�ε����� L,R�� �� ���� �� 2,3 �̾ ����
-> Right�ε��� <= Left�ε����� R,P ���� ��, while�� ����. ���� ��ͷ� ��,�� ����
*/

#include <stdio.h>
#include <windows.h>
#define QUICK_SORT_LEN 12

int Arr[QUICK_SORT_LEN] = { 5, 1, 8, 20, 2, 3, 6, 11, 4, 7, 12, 19};

void QuickSort(int, int);
void Print(int, int);

int main()
{
	// ���� ���� �� ���� ������.
	for (int i = 0; i < QUICK_SORT_LEN; ++i)
	{
		if (Arr[i] >= 10)
			printf(" %d ", Arr[i]);
		else
		{
			fputs(" 0", stdout);
			printf("%d ", Arr[i]);
		}			
	}
	fputs("\n", stdout);

	// ���� ���� ���� ���� ������
	fputs("\n", stdout);
	for (int i = 0; i < QUICK_SORT_LEN; ++i)
	{
		fputs("^^^^", stdout);
	}
	fputs("\n", stdout);
	
	// ������ ����
	QuickSort(1, QUICK_SORT_LEN-1);

	// ���� �Ϸ� �� �ٽ��ѹ� ���� ������
	for (int i = 0; i < QUICK_SORT_LEN; ++i)
	{
		if (Arr[i] >= 10)
			printf(" %d ", Arr[i]);
		else
		{
			fputs(" 0", stdout);
			printf("%d ", Arr[i]);
		}
	}
	fputs("\n", stdout);

	return 0;
}

// ��� �Լ�
void Print(int Left, int RIght)
{
	for (int i = 0; i < QUICK_SORT_LEN; ++i)
	{
		if (Arr[i] >= 10)
		{
			if (i == Left || i == RIght)
				printf("[%d]", Arr[i]);
			else
				printf(" %d ", Arr[i]);
		}
		else
		{
			if (i == Left || i == RIght)
			{
				fputs("[0", stdout);
				printf("%d]", Arr[i]);
			}				
			else
			{
				fputs(" 0", stdout);
				printf("%d ", Arr[i]);
			}
		}
		
	}
	fputs("\n", stdout);
}

void QuickSort(int Left, int Right)
{
	// 1. �Ǻ��� L,R ���
	int iPivot = Left-1;
	int iTemp;
	int iOriginalLeft = Left;
	int iOriginalRight = Right;

	// Ż������. <<���̳� >>���̸� Ż��
	if (Left > iOriginalRight || Right < iOriginalLeft)
		return;
			
	while (1)
	{		
		while (1)
		{
			// 2. Left��->�� �̵�.Left > �Ǻ��̰ų� Left�ε��� == Right�ε��� �� �� ����. �װ� �ƴ϶�� Left�ε��� ����.			
			if (Arr[iPivot] < Arr[Left] || Left == Right)
				break;
			Left++;
		}
		while (1)
		{
			// 3. Right�� <-�� �̵�.Right < �Ǻ��̰ų�, Right�ε��� == Pivot�ε��� �� �� ����. �װ� �ƴ϶�� Right�ε��� ����.
			if (Arr[iPivot] > Arr[Right] || iPivot == Right)
				break;
			Right--;;
		}
		
		/*
		4. 2, 3�� �Ϸ�� ��
			->Right�ε��� > Left�ε����� L, R�� �� ���� �� 2, 3 �̾ ����
			->Right�ε��� <= Left�ε����� R, P ���� ��, while�� ����.���� ��ͷ� ��, �� ����
		*/
		if (Right > Left)
		{ 
			iTemp = Arr[Right];
			Arr[Right] = Arr[Left];
			Arr[Left] = iTemp;	
		}
		
		else if (Right <= Left)
		{
			iTemp = Arr[Right];
			Arr[Right] = Arr[iPivot];
			Arr[iPivot] = iTemp;	
			Print(Left, Right);
			break;
		}
		Print(Left, Right);
	}	

	// ���� ���� ���� ������.
	fputs("\n", stdout);
	for (int i = 0; i < QUICK_SORT_LEN; ++i)
	{
		if (iPivot <= i && i <= Right)
			fputs("^^^^", stdout);
		else
			fputs("    ", stdout);
	}
	fputs("\n", stdout);
	
	// ���� �ǹ��� ����
	QuickSort(iOriginalLeft, Right-1);

	// ���� �ǹ��� ����
	QuickSort(Right+2, iOriginalRight);
}