#include <stdio.h>


// 1. ���� ���� n-1���� A���� B�� �̵�
// 2. ū ���� 1���� A���� C�� �̵�
// 3. ���� ���� n-1���� B���� C�� �̵�

void hanoi(int num, char one, char two, char three)
{
	// �ϳ��� Ÿ�� Ż������ : �̵��� ������ 1����, ������ ����(���� ���� ����. 1 ����)�� one���� three�� �̵��ϴ� ���̴� Ż��
	if (num == 1)
	{
		printf("���� 1�� %c���� %c�� �̵�\n", one, three);
	}

	// �̵��� ������ 1�� �ƴ϶��, ������ ���� ���� �̵�
	else
	{
		// num-1�� ������ one���� three�� ���� two�� �̵���Ų��. 
		hanoi(num - 1, one, three, two);
		printf("���� %d��(��) %c���� %c�� �̵�\n", num, one, three);

		// num-1�� ������, two���� one�� ���� three�� �̵���Ų��.
		hanoi(num - 1, two, one, three);
	}

}

int main()
{
	hanoi(2, 'A', 'B', 'C');
	return 0;
}