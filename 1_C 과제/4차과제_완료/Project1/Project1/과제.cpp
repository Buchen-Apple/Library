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
	// ù �迭�� ���� 0���� ����, ù ���� ���簡 �����ϵ��� �Ѵ�.
	cKorBuffer[0] = 0;

	char cWard[WORD_LEN];
	int i = 0;
	int j;
	bool ibreakFlag = false;

	// ���� �ܾ��� ��������
	Dictionary Dic[WORD_COUNT] =
	{
	{"english","����"},
	{"korea","�ѱ�"},
	{"apple","���"},
	{"people","���"},
	{"love","���"},
	{"name","�̸�"},
	{"you","��"},
	{"stupid","�ٺ�����"},
	{"america","�̱�"},
	{"bag","����"},
	{"money","��"},
	{"paper","����"},
	};

	// 1. ���� ���ڿ� �Է� ��, cEngBuffer�� ����
	fputs("���� ���ڿ� �Է� : ", stdout);
	gets_s(cEngBuffer, BUFFER_LEN);

	// 2. ��� ���ڸ� �ҹ��ڷ� ��ȯ.	
	strlwr(cEngBuffer);

	// 3. cEngBuffer���� ���ڸ� ���� ������ �ɰ��� �ܾ����� ���ۿ� ����.
	while (1)
	{
		j = 0;
		// 4. �Է¹��� ���ڿ����� 1���� �ܾ� ����.
		while (1)
		{
			if (cEngBuffer[i] == ' ' || cEngBuffer[i] == 0)
				break;
			cWard[j] = cEngBuffer[i];
			i++;
			j++;
		}
		
		// 5. ������ ���ڿ��ٸ�, while���� ���������� �ϱ� ������ flag�� true�� ����.
		if (cEngBuffer[i] == 0)
			ibreakFlag = true;

		// 6. ������� �Դٴ� ���� �ܾ� 1���� ��� ã�Ҵٴ� ��. �ܾ� �������� �ι��ڸ� �־��ش�.
		cWard[j] = 0;


		// 7. ã�� �ܾ�� ������ �ܾ ���Ѵ�. ���ڿ��� ���ٸ� �ش� ������ �ѱ��� Kor���ۿ� ����.
		for (j = 0; j < WORD_COUNT; ++j)
		{
			if (0 == strcmp(Dic[j].Eng, cWard))
			{
				strcat_s(cKorBuffer, sizeof(cKorBuffer), Dic[j].Kor);
				cKorBuffer[strlen(cKorBuffer) + 1] = 0;
				cKorBuffer[strlen(cKorBuffer)] = ' ';
				
				break;
			}
		}
		// 8. ������ ���������� �����µ��� ��ġ�ϴ� �ܾ ������, ??�� �ִ´�.
		if (j == WORD_COUNT)
		{
			strcat_s(cKorBuffer, sizeof(cKorBuffer), "??");
			cKorBuffer[strlen(cKorBuffer) + 1] = 0;
			cKorBuffer[strlen(cKorBuffer)] = ' ';
		}

		// 9. ������ ���ڿ��ٸ� (5�� �������� üũ��)  while�� ����
		if (ibreakFlag == true)
			break;
		i++;
	}

	// 10. ġȯ�� ���ڿ� ǥ��
	printf("ġȯ �ѱ��� ǥ�� : %s\n", cKorBuffer);	

	return 0;
}