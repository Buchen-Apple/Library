// Project3.cpp: �ܼ� ���� ���α׷��� �������� �����մϴ�.
//

#include "stdafx.h"

int main()
{
	unsigned short val = 0;

	int input;
	int OnOffinput;
	int i;


	while (1)
	{
		fputs("��Ʈ ��ġ : ",stdout);
		scanf("%d", &input);

		fputs("OFF/ON [0,1] : ", stdout);
		scanf("%d", &OnOffinput);

		// 1�� �Է��ߴٸ�, OR��Ʈ������ ���� �ϳ��� 1�� ������ ��� 1�� ��� �ٲ۴�.
		if (OnOffinput == 1)
			val = val | (OnOffinput << (input - 1));

		// 0�� �Է��ߴٸ�
		// 1. << �������� ��ġ�� �̵�
		// 2. ~ �������� ����
		// 3. AND�������� �� �� 1�ΰ͸� 1�� �ٲ�
		else
		{
			OnOffinput = 1;
			val = val & (~(OnOffinput << (input - 1)));
		}

		
		for (i = 15; i >= 0; --i)
		{
			printf("%d �� Bit : ", i + 1);
			if ((val >> i) & 1)
				fputs("ON", stdout);
			else
				fputs("OFF", stdout);
			printf("\n");
		}


		printf("\n");
	}

	return 0;
}

