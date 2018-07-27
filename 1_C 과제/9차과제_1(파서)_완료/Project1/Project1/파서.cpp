#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <string.h>

typedef struct
{
	char m_iName[30];
	int m_iHP;
	int m_iMP;	
	int m_iAtack;
	int m_iArmor;

}Monster;

// ������ �о�� ����
char* cBuffer;

// ���� ����ü ���� ���� �Լ�
void ShowMonster(Monster);

//--------------------------
// �ļ� ���� �Լ�
// -------------------------

// �ļ�_���� �ε�
bool Parser_LoadFile(const char* FileName);

// GetNextWord���� �ܾ� ��ŵ ���θ� üũ�Ѵ�.
// true�� ��ŵ / false�� ��ŵ�� �ƴϴ�.
bool SkipNoneCommand(char cTempWord);

// ���ۿ���, ���� �ܾ� ã��
bool GetNextWord(char cWord[], int* len);

// �ļ�_���ϴ� �� ã�ƿ��� (int)
bool Parser_GetValue_Int(const char* Name, int* value);

// �ļ�_���ϴ� �� ã�ƿ��� (���ڿ�)
bool Parser_GetValue_String(const char* Name, char* value);


int main(void)
{
	Monster mon;
	
	bool LoadFileCheck;

	//���� �ε�
	LoadFileCheck = Parser_LoadFile("Test.txt");
	if (LoadFileCheck == false)
	{
		printf("���� �ҷ����� ����\n");
		exit(-1);
	}

	// ���ϴ� �� ã��
	Parser_GetValue_String("m_iName", mon.m_iName);
	Parser_GetValue_Int("m_iHP", &mon.m_iHP);
	Parser_GetValue_Int("m_iMP", &mon.m_iMP);
	Parser_GetValue_Int("m_iAtack", &mon.m_iAtack);
	Parser_GetValue_Int("m_iArmor", &mon.m_iArmor);

	// Ȯ���ϱ�
	ShowMonster(mon);
	return 0;
}

// ���� ����ü ���� ���� �Լ�
void ShowMonster(Monster mon)
{
	printf("�̸� : %s\n", mon.m_iName);
	printf("ü�� : %d\n", mon.m_iHP);
	printf("���� : %d\n", mon.m_iMP);
	printf("���ݷ� : %d\n", mon.m_iAtack);
	printf("���� : %d\n", mon.m_iArmor);
}

//--------------------------
// �ļ� ���� �Լ�
// -------------------------

// �ļ�_���� �ε�
bool Parser_LoadFile(const char* FileName)
{
	FILE* fp;
	int iSize;
	size_t iFileCheck;

	iFileCheck = fopen_s(&fp, FileName, "rt");
	if (iFileCheck != 0)
		return false;

	fseek(fp, -7, SEEK_END);
	iSize = ftell(fp);
	fseek(fp, 0, SEEK_SET);

	cBuffer = (char*)malloc(iSize);

	iFileCheck = fread(cBuffer, iSize, 1, fp);
	if (iFileCheck < 1)
		return false;

	cBuffer[iSize] = 0;

	return true;
}

// GetNextWord���� �ܾ� ��ŵ ���θ� üũ�Ѵ�.
// true�� ��ŵ / false�� ��ŵ�� �ƴϴ�.
bool SkipNoneCommand(char cTempWord)
{
	// ������� �޸�, ��ħǥ, ����ǥ, �����̽�, �齺���̽�, ��, �����ǵ�, ĳ���� ����, / ����(�ּ�üũ��)
	if (cTempWord == ',' || cTempWord == '.' || cTempWord == '"' || cTempWord == 0x20 || cTempWord == 0x08 || cTempWord == 0x09 || cTempWord == 0x0a || cTempWord == 0x0d || cTempWord == '/')
		return true;

	return false;
}

// ���ۿ���, ���� �ܾ� ã��
bool GetNextWord(char cWord[], int* len)
{
	int i = 0;
	char cTempWord[256];

	// �ܾ� ���� ��ġ�� ã�´�. true�� ��� ��ŵ�ȴ�.
	while (SkipNoneCommand(cBuffer[*len]))
	{
		// len�� ���� cBuffer�� ������� Ŀ����, �迭�� ����Ŵ� false ����.
		if (*len > strlen(cBuffer))
			return false;

		(*len)++;
	}

	// ������ġ�� ã�Ҵ�. ���� �� ��ġ�� ã�ƾ��Ѵ�.
	while (1)
	{
		// len�� ���� cBuffer�� ������� Ŀ����, �迭�� ����Ŵ� false ����.
		if (*len > strlen(cBuffer))
			return false;

		// ������� �޸�, ��ħǥ, ����ǥ, �����̽�, �齺���̽�, ��, �����ǵ�, ĳ���� ����
		// �� �� �ϳ��� �ش�Ǹ�, �� ��ġ�� ã���Ŵ� while���� ������.
		else if (cBuffer[*len] == ',' || cBuffer[*len] == '.' || cBuffer[*len] == '"' || cBuffer[*len] == 0x20 || cBuffer[*len] == 0x08 || cBuffer[*len] == 0x09 || cBuffer[*len] == 0x0a || cBuffer[*len] == 0x0d)
			break;

		// ���� �ִ´�.
		// �׸��� ���� ���� �ֱ� ���� ���� �迭�� ��ġ�� 1ĭ �̵�
		cTempWord[i++] = cBuffer[(*len)++];

	}

	// �ܾ��� �������� �ι��ڸ� �־� ���ڿ��� �ϼ�.
	cTempWord[i] = '\0';
	(*len)++;

	// ���ڿ� ����.
	memset(cWord, 0, 256);
	strcpy_s(cWord, sizeof(cTempWord), cTempWord);

	return true;
}

// �ļ�_���ϴ� �� ã�ƿ��� (int��)
bool Parser_GetValue_Int(const char* Name, int* value)
{
	int len = 0;
	char cWord[256];

	// cBuffer�� ���ڿ��� cWord�� �ű��. ����� �� ���� ���� ���ڿ��� �޾ƿ´�.
	// �޾ƿ��µ� �����ϸ� true, �޾ƿ��µ� �����ϸ� false�̴�. �޾ƿ��µ� �����ϴ� ����, ���������� ��� �˻縦 �� ���̴�.
	while (GetNextWord(cWord, &len))
	{
		// ã�� ���ڿ��� ���� ���ϴ� ���ڿ����� Ȯ��. ���� ã�� ���ڿ��� �ƴ϶�� �ٽ� �� while������ ���� Ȯ��.
		if (strcmp(cWord, Name) == 0)
		{
			// ���� ã�� ���ڿ��̶��, ���� ���ڷ� = �� ã�´�.
			// if (GetNextWord(cWord, &len))�� true��� �ϴ� ���� ���ڿ��� ã�� ���̴�.
			if (GetNextWord(cWord, &len))
			{
				// ���ڿ��� ã������ �� ���ڿ��� ��¥ = ���� Ȯ������.
				if (strcmp(cWord, "=") == 0)
				{
					// = ���ڿ��̶��, ���� ���ڿ��� ã�ƿ´�. �̹��� ã�°��� ���� Value�̴�.
					if (GetNextWord(cWord, &len))
					{
						// ������� ������, Value�� ã�� ���̴� ���� ���� ����.
						*value = atoi(cWord);
						return true;
					}

				}

			}

		}

	}

	// ������� �°�, cBuffer�� ��� �����͸� ã�Ҵµ��� ���� ���ϴ� �����Ͱ� ���� ���̴�, false ��ȯ.
	return false;
}

// �ļ�_���ϴ� �� ã�ƿ��� (���ڿ�)
bool Parser_GetValue_String(const char* Name, char* value)
{
	int len = 0;
	char cWord[256];

	// cBuffer�� ���ڿ��� cWord�� �ű��. ����� �� ���� ���� ���ڿ��� �޾ƿ´�.
	// �޾ƿ��µ� �����ϸ� true, �޾ƿ��µ� �����ϸ� false�̴�. �޾ƿ��µ� �����ϴ� ����, ���������� ��� �˻縦 �� ���̴�.
	while (GetNextWord(cWord, &len))
	{
		// ã�� ���ڿ��� ���� ���ϴ� ���ڿ����� Ȯ��. ���� ã�� ���ڿ��� �ƴ϶�� �ٽ� �� while������ ���� Ȯ��.
		if (strcmp(cWord, Name) == 0)
		{
			// ���� ã�� ���ڿ��̶��, ���� ���ڷ� = �� ã�´�.
			// if (GetNextWord(cWord, &len))�� true��� �ϴ� ���� ���ڿ��� ã�� ���̴�.
			if (GetNextWord(cWord, &len))
			{
				// ���ڿ��� ã������ �� ���ڿ��� ��¥ = ���� Ȯ������.
				if (strcmp(cWord, "=") == 0)
				{
					// = ���ڿ��̶��, ���� ���ڿ��� ã�ƿ´�. �̹��� ã�°��� ���� Value�̴�.
					if (GetNextWord(cWord, &len))
					{
						// ������� ������, Value�� ã�� ���̴� ���� ���� ����.
						strcpy_s(value, sizeof(cWord), cWord);
						return true;
					}

				}

			}

		}

	}

	// ������� �°�, cBuffer�� ��� �����͸� ã�Ҵµ��� ���� ���ϴ� �����Ͱ� ���� ���̴�, false ��ȯ.
	return false;
}