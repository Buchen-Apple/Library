// Project1.cpp: �ܼ� ���� ���α׷��� �������� �����մϴ�.
//

#include "stdafx.h"
#include <windows.h>
#include <math.h>

int main()
{
	double dRadian = 0;
	double Check = 1;
	int iAngle = 5;
	int iFlag = 1;

	while (1)
	{
		// ���� ����� �� ���� ���� ����
		dRadian = iAngle * 3.14 / 180;

		// sin���� ���� Check���� �ʱ�ȭ. Check�� �̿��� ������ �ٲ� �� ���� ���� üũ�Ѵ�.
		Check = 0;

		// Check�� ���� sine���� �۴ٸ�, Check���� 0.03�� ������Ű�鼭 ���� �׸���.
		// ��, Sine�� 0.015���� ���� 2��
		for (; Check < sin(dRadian); Check += 0.03)
		{
			printf("**");
			Sleep(1);
		}

		fputs("\n", stdout);

		// angle�� 90�� �Ǹ�, �÷��׸� off��Ų��. �÷��װ� off(0)�Ǹ� angle�� �����Ѵ�. �÷��װ� on(1)�� ���� ���� �����Ѵ�.
		// ����Ʈ on(1)�� �����Ѵ�.
		if (iAngle >= 90)
			iFlag = 0;
		else if (iAngle <= 0)
			iFlag = 1;

		// �÷��׸� üũ�� angle���� ��/�� ��Ų��.
		if (iFlag == 0)
			iAngle -= 5;
		else
			iAngle += 5;
	}

	return 0;
}