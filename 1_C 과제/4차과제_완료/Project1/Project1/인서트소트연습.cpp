/*
�μ�Ʈ ��Ʈ
1. 1�� �ε���(0���� ��ĭ ������)���� ����
2. ��ĭ�� ���������� �̵��ϸ鼭 ���ʿ� ���� �� ��Ұ� ������ ã�´�
3. ã�´ٸ� �� ��ġ�� ����, �� ��ġ ���� �ִ� �����ʹ� ��� ��ĭ�� �ڷ� �и���.
4. ��, ���� �������� ã�Ƽ� �װ��� �μ�Ʈ�Ѵٴ� ����
*/

#include <stdio.h>
#define ARR_LEN 10

void print(int);
void printLine(int);

// 1. �迭 �� ����
int Arr[ARR_LEN] = { 8, 6, 13, 50, 1, 3, 7, 9, 20, 15 };

int main()
{
	// 2. Check���� 1�� �ε����� ����Ű���� �Ѵ�.
	int iCheck = 1;
	int iTemp;

	puts("���� ��");
	for (int i = 0; i < ARR_LEN; ++i)
	{
		printf(" %02d ", Arr[i]);
	}
	fputs("\n\n\n", stdout);

	while (iCheck < ARR_LEN)
	{
		
		// 3. Arr[Check]�� Arr[Check-1]���� ������ üũ. ���� �� �Ʒ� ���� ����. ũ�ٸ�, ���� ���ϰ� Check ++;
		if (Arr[iCheck] < Arr[iCheck - 1])
		{
			print(iCheck);
			printLine(iCheck);
			for (int i = 0; i < iCheck; ++i)
			{
				// 4.��ȸ ��, Arr[iCheck]���� ū ���� ã���� ����.
				if (Arr[iCheck] < Arr[i])
				{
					// 5. ���� ��, Arr[iCheck]�� ���� ������ �� �� ���� ���������� ��ĭ�� ����. iCheck -1�� iCheck�� �� �� ����.
					iTemp = Arr[iCheck];
					for (int j = iCheck; j > i; --j)
					{
						Arr[j] = Arr[j - 1];

					}
					// 6. ��� ������� �Ʊ� ã�� ū ���� 1ĭ �տ�(������) ���� �ִ´�.
					Arr[i] = iTemp;
				}
				
			}
			
		}		
		iCheck++;
	}	
	// 3 ~ 6�� �ݺ��ϴٰ� ������ ���ڱ��� ��� üũ�ϸ� break.


	// 7. ���� �Ϸ�� �迭 ������
	puts("\n\n���� �Ϸ�");
	for (int i = 0; i < ARR_LEN; ++i)
	{
		printf(" %02d ", Arr[i]);
	}
	fputs("\n", stdout);
	
	return 0;
}

void print(int iCheck)
{
	for (int i = 0; i < ARR_LEN; ++i)
	{
		if(i == iCheck)
			printf("[%02d]", Arr[i]);
		else
			printf(" %02d ", Arr[i]);
	}
	fputs("\n", stdout);
}

void printLine(int iCheck)
{
	for (int i = 0; i < iCheck; ++i)
	{
		fputs("----", stdout);
	}
	fputs("\n", stdout);
}