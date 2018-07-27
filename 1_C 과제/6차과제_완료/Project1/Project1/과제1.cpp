// ���Ͽ� �ִ� ���ܾ �ѱ۷� �����ؼ� �����ֱ�.
#pragma warning(disable:4996)

#include <stdio.h>
#include <string.h>
#define BUFFER_LEN	250
#define WORD_LEN	100
#define WORD_COUNT	30

typedef struct
{
	char Eng[WORD_LEN];
	char Kor[WORD_LEN];

}Dictionary;

int main()
{
	char cEngBuffer[BUFFER_LEN];
	char cKorBuffer[BUFFER_LEN];
	// ù �迭 ����� ���� 0���� ����, ���簡 �����ϵ��� �Ѵ�.
	cKorBuffer[0] = 0;

	char cWard[WORD_LEN];
	char cInput[10];
	int iFilelen;
	int iWordCount = 0;
	int i = 0;
	int j;

	// ���� �ܾ��� ��������
	Dictionary Dic[WORD_COUNT] =
	{
		{ "english","����" },
		{ "korea","�ѱ�" },
		{ "apple","���" },
		{ "people","���" },
		{ "love","���" },
		{ "name","�̸�" },
		{ "you","��" },
		{ "stupid","�ٺ�����" },
		{ "america","�̱�" },
		{ "bag","����" },
		{ "money","��" },
		{ "paper","����" },
	};

	//FILE* fp = fopen("Test1.txt", "rt");
	
	fputs("������ ���� �̸��� �Է����ּ��� : ", stdout);
	scanf("%s", cInput);

	FILE* fp;
	fopen_s(&fp, cInput, "rt");
	

	if (fp == NULL)
	{
		puts("���� ���� ����");
		return -1;
	}

	
	while (1)
	{
		i = 0;

		// 1. ���Ͽ��� ���ڿ��� ���� ������ �о�´�. ������ ������ �о�´�.
		while ((cEngBuffer[i] = fgetc(fp)) != EOF)
			i++;
		
		// 3. ������ �� ���� ���� ����.
		iFilelen = i-1;

		i = 0;

		// 2. ��� ���� �ҹ��ڷ� ��ȯ
		strlwr(cEngBuffer);	

		while (1)
		{
			j = 0;
			// 3. cEngBuffer ���� 1���� �ܾ� ����.
			while (1)
			{
				if (cEngBuffer[i] == ' ' || cEngBuffer[i] == '\n')
					break;
				cWard[j] = cEngBuffer[i];
				i++;
				j++;
			}

			// 4. ������� �Դٴ� ���� �ܾ� 1���� ��� ã�Ҵٴ� ��. �������� ���� �־ ���ڿ��� �����.
			cWard[j] = 0;


			// 5. ã�� �ܾ�� ������ �ܾ ���Ѵ�. ���ڿ��� ���ٸ� �ش� ������ �ѱ��� Kor���ۿ� ����.
			for (j = 0; j < WORD_COUNT; ++j)
			{
				if (0 == strcmp(Dic[j].Eng, cWard))
				{
					// ���⿡ �°�, �ܾ� �ϳ��� ã�Ҵٴ� ���̴�, �ܾ� ī��Ʈ ����.
					iWordCount++;

					strcat_s(cKorBuffer, sizeof(cKorBuffer), Dic[j].Kor);
					cKorBuffer[strlen(cKorBuffer) +1] = 0;

					if (cEngBuffer[i] == ' ')
						cKorBuffer[strlen(cKorBuffer)] = ' ';

					else if (cEngBuffer[i] == '\n')
						cKorBuffer[strlen(cKorBuffer)] = '\n';						

					break;
				}
			}
			// 6. ������ ���������� �����µ��� ��ġ�ϴ� �ܾ ������, ??�� �ִ´�.
			if (j == WORD_COUNT)
			{
				strcat_s(cKorBuffer, sizeof(cKorBuffer), "??");
				cKorBuffer[strlen(cKorBuffer) + 1] = 0;

				if (cEngBuffer[i] == ' ')
					cKorBuffer[strlen(cKorBuffer)] = ' ';

				else if (cEngBuffer[i] == '\n')
					cKorBuffer[strlen(cKorBuffer)] = '\n';				
				
			}

			// 7. ����, �̹� ���ڿ��� �������̶�� while�� ����.
			if (i == iFilelen)
				break;

			++i;
		}

		// 8. ġȯ�� ���ڿ� ǥ��
		puts("\n--���� �Ϸ�!--");
		printf("������ �ܾ� �� : %d\n\n", iWordCount);
		printf("%s\n", cKorBuffer);		

		break;
		
	}	

	fclose(fp);
	return 0;
}