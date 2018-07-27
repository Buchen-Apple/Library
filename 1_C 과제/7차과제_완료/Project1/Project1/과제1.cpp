// �ؽ����̺�
#pragma warning(disable:4996)
#include <stdio.h>
#include <stdlib.h>
#include "LinkedLIst.h"

#define ListMax 10

enum 
{
	INSERT = 1, DELETE, ALL_SHOW, SEARCH 
};

int HashFunt(int);

int main()
{

	int iInput;
	int j;
	int iInputKey;
	int iIndex;
	char cInputData[100];
	bool NextCheck = true;
	LinkedList List[ListMax];
	Node* ShowNode = (Node*)malloc(sizeof(Node));

	// ����Ʈ �ʱ�ȭ
	for (int i = 0; i < ListMax; ++i)
	{
		Init(&List[i]);
	}

	while (1)
	{
		// 1. �޴� ����
		system("cls");
		puts("##MENU##");
		puts("1. ������ �߰�");
		puts("2. ������ ����");
		puts("3. ��ü����");
		puts("4. ã��");
		fputs(": ", stdout);
		scanf("%d", &iInput);

		// 2. �޴��� ���� ���� ����
		switch (iInput)
		{
		case INSERT:
			// ������ �߰� ���� ����
			// 1. Ű���� �����͸� �Է¹޴´�.
			fputs("KEY (����) : ", stdout);
			scanf("%d", &iInputKey);
			fputs("Data : ", stdout);
			scanf("%s", cInputData);

			// 2. Ű ���� �ؽ��Լ��� �̿��� �ε����� ġȯ��.
			iIndex = HashFunt(iInputKey);

			// 3. �ش� �ε�����, ��ũ�� ����Ʈ�� �� �߰�
			ListInsert_Head(&List[iIndex], iInputKey, cInputData);

			// 4. �߰� �Ϸ� ��Ʈ��
			puts("�μ�Ʈ �Ϸ�!");
			system("pause");
			break;

		case DELETE:
			// ������ �����ϱ� ����
			// 1. ������ �������� Key���� �Է¹޴´�.
			fputs("FIne Key(����) : ", stdout);
			scanf("%d", &iInputKey);

			// 2. �Է¹��� Ű�� �ؽ��Լ��� �̿��� �ε����� ġȯ
			iIndex = HashFunt(iInputKey);

			// 3. ġȯ�� �ε����� �ش� �ε����� ����Ǿ� �ִ� ��带 ��ȸ�ϸ� Key�� ���Ѵ�. �׸��� �� key�� ���� �ִ� ��带 ����Ʈ���� ���� free�Ѵ�.
			for (j = 0; j < ListMax; ++j)
			{				
				NextCheck = ListDelete(&List[j], iInputKey);
				// NextCheck�� true�� ���ϴ� ���� ã�Ƽ� ������ �Ϸ��ߴٴ� ��. �׷� �ݺ��� ����.
				if (NextCheck == true)
					break;
			}

			// 4. ������� ���� ��, NextCheck�� true��� ���ϴ°��� ã�Ƽ� ���°�. �׷��� ���� �Ϸ�! ��Ʈ���� ǥ���Ѵ�.
			if (NextCheck == true)
				puts("���� �Ϸ�!");
			// 5. ������� ���� ��, NextCheck�� false��� ���ϴ°��� ��ã���Ŵ�, ã�� ���� �����ϴ� ��Ʈ�� ǥ��.
			else
				puts("ã�� Ű�� �����ϴ�");

			system("pause");			
			break;

		case ALL_SHOW:
			// ������ �����ֱ� ����
			// 1. ����Ʈ ������� ���� ������.
			for (int i = 0; i < ListMax; ++i)
			{
				printf("[%02d] : ", i);
				// 2. �� ����Ʈ�� ���, ���� �տ� ���̰� �ֱ� ������ 1ĭ >>�������� ������ �� üũ�ؾ� ��.
				// �׷��� j�� �⺻ ���� 1�� �Ѵ�.
				j = 1;
				while (1)
				{
					// ListPeek�� �̿���, ShowNode���ٰ� ������ ����� ���� �����Ѵ�.
					// j�� �ε����̴�.
					NextCheck = ListShow(&List[i], ShowNode, j);
					// ���� NextCheck�� false�� ������ ������ ���̴� ���� ������ ����Ʈ�� �����ش�.
					if (NextCheck == false)
						break;

					printf(">> Key: %d, ", ShowNode->Key);
					printf("Data: %s ", ShowNode->Data);
					j++;
				}
				fputs("\n", stdout);
			}

			puts("��ü���� �Ϸ�!");
			system("pause");
			break;

		case SEARCH:
			// ������ ã�� ����
			// 1. key�� �Է¹޴´�.
			fputs("FIne Key(����) : ", stdout);
			scanf("%d", &iInputKey);

			// 2. �Է¹��� Ű�� �ؽ��Լ��� �̿��� �ε����� ġȯ
			iIndex = HashFunt(iInputKey);

			// 3. ġȯ�� �ε����� �ش� �ε����� ������ִ� ����Ʈ�� ������� �˻��ϸ�, key���� ���Ѵ�.
			NextCheck = ListPeek(&List[iIndex], cInputData, iInputKey);

			// 4. ���� ã������ ã�� ���� �����ش�.
			if (NextCheck == true)
			{
				printf("Fine Data : %s\n", cInputData);
				puts("ã�� ����!");
			}

			// 5. ��ã���� ��ã�Ҵٰ� �����ش�.
			else
			{
				puts("���� �������Դϴ�.");			
			}

			system("pause");
			break;

		}
	}	

	return 0;
}

int HashFunt(int iInputKey)
{
	iInputKey = ((iInputKey * 3 + 130) % ListMax);
	return iInputKey;
}