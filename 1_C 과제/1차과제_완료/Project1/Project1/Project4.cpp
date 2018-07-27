// Project4.cpp: �ܼ� ���� ���α׷��� �������� �����մϴ�.
//

#include "stdafx.h"

int main()
{
	unsigned int val = 0;
	unsigned int mask;
	unsigned int numInput;
	int input;
	int i;
	int j;
	int sum;
	int square;

	while (1)
	{
		fputs("��ġ <1~4> : ", stdout);
		scanf("%d", &input);

		fputs("�� [0~255] : ", stdout);
		scanf("%d", &numInput);
				
		//�ϴ�, �������� ����Ʈ�� ���� 0���� �ʱ�ȭ
		// 1. ���� ������ 1�� ��ġ�� �̵�
		// 2. ����
		// 3. and�������� �ش� ����Ʈ�� ���� ������� 0���� ����.
		for (i = 8; i > 0; --i)
		{			
			val = val & (~(1 << ((input * 8) - i)));
		}

		// val�� �ش� ����Ʈ���ٰ� �� �ֱ�.
		for (i = 8; i > 0; --i)
		{
			// numInput�� ���� ���� �� 1���� mask�� ����. (�Ʒ��� ����)
			// 0000 0000 0000 0000 0000 0000 1010 1101 numInput
			// 0000 0000 0000 0000 0000 0000 0000 0001 1
			// ------------------------------------------ and����
			// 0000 0000 0000 0000 0000 0000 0000 0001 mask
			mask = numInput & 1;

			// 1��Ʈ ��ġ�� ������ �� ����Ʈ�� ù ��ġ�� or����.
			val = val | (mask << ((input * 8) -i));

			// numInput�� �������� ��ĭ �о ���� ��Ʈ�� ������� ��. �׷��� ���� ��Ʈ(������ �ִ� ��Ʈ)�� ����� �� �����ϱ�.
			numInput = (numInput >> 1);
			
		}

		// 1����Ʈ �� ũ�� ǥ��
		// ���� ���� ����Ʈ(1�� ����Ʈ)���� ���������� ����.
		for (i = 1; i < 5; ++i)
		{
			sum = 0;
			square = 0;
			for (j = 8; j > 0; --j)
			{
				if (j == 8)
					square = 1;
				else
				{
					square *= 2;
				}

				if (val & (1 << ((i * 8) -j)))
					sum += square;
				
			}
			printf("%d ��° ����Ʈ �� : %d\n", i, sum);
		}
		fputs("\n", stdout);


		//��ü ����Ʈ ũ�� ǥ��
		//���� ����(4��) ����Ʈ���� ��Ʈ������ ������ 16������ 1�ڸ��� �� ǥ��
		fputs("��ü ����Ʈ ũ�� : 0x", stdout);
		for (i = 8; i > 0; --i)
		{
			sum = 0;
			square = 0;
			for (j = 4; j > 0; --j)
			{
				if (j == 4)
					square = 1;
				else
				{
					square *= 2;
				}

				if (val & (1 << ((i * 4) - j)))
					sum += square;
			}

			printf("%x", sum);
		}
		fputs("\n\n", stdout);
	}

	return 0;
}

