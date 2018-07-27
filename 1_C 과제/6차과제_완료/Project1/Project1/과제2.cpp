// ���� ��ȣȭ/��ȣȭ

#include <stdio.h>
#include <stdlib.h>
#include <cstring>

int main()
{
	char cMyFileCheck[11] = "SongJinGyu";
	int iFileSize;
	int iFileReadCheck;
	char* cFileSave;
	char* cRefIleSave;
	unsigned int iMask = 32457;
	int j = 0;
	char cInput[30];

	// false�� ��ȣȭ , true�� ��ȣȭ
	bool bCryptography = false;

	FILE* fp;

	// 1. ��ȣȭ/��ȣȭ �� �����̸� �Է�
	fputs("��ȣȭ/��ȣȭ �� ���� �̸��� �Է����ּ��� : ", stdout);
	scanf("%s", cInput);


	// 2. �Է� ��Ʈ���� ���̳ʸ� ���� ����
	fopen_s(&fp, cInput, "rb");


	// 3. �о�� ������ ũ�⸦ ���Ѵ�.
	fseek(fp, 0, SEEK_END);
	iFileSize = ftell(fp);
	fseek(fp, 0, SEEK_SET);


	// 4. ���� ũ�� ��ŭ �����Ҵ�. 
	cFileSave = (char*)malloc(iFileSize);


	// 5. �����Ҵ� �� �޸𸮿� ������ �о�´�.
	iFileReadCheck = fread(cFileSave, iFileSize, 1, fp);

	// 6. �о���� �����ϸ� �����޽��� �߻�
	if (iFileReadCheck != 1)
	{
		puts("���� ������ �ֳ׿�!");
		return -1;
	}

	// 7. �Է� ��Ʈ�� ����
	fclose(fp);


	// 8. ��ȣȭ, ��ȣȭ üũ
	// �� ���ڿ��� strlen(cMyFileCheck) ���̸�ŭ ��.
	// ���� ���ٸ�, SongJinGyu ��� ���ڰ� �ִ°Ŵ�, ��ȣȭ �Ǿ� �ִ°����� �Ǵ�. ��, ��ȣȭ�� �ؾ� �Ѵ�.
	if (0 == strncmp(cFileSave, cMyFileCheck, strlen(cMyFileCheck)))
		bCryptography = true;

	// 9. ��� ��Ʈ�� ����	
	fopen_s(&fp, "test2.txt", "wb");

	// 8�� ��������, bCryptography�� false�� ��� ��ȣȭ ���� ����
	if (bCryptography == false)
	{
		// �ϴ� �� �����̶�°��� ǥ���ϴ� ���ڿ��� ���� ���Ͽ� �ִ´�.
		// sizeof(cMyFileCheck)�� -1�� �ϴ� ������, cMyFileCheck���� ���� ������ ���ڿ��� �� �ֱ� ������, ���� ���� �ִ°��̴�.
		fwrite(cMyFileCheck, sizeof(cMyFileCheck)-1, 1, fp);
		
		// ���Ͽ��� �о�� ���ڿ��� iMask�� xor�ؼ� ��ȣȭ �Ѵ�.
		// �ݺ����� ���� �ϳ��ϳ� ��ȣȭ.
		for (int i = 0; i < iFileSize; ++i)
		{
			cFileSave[i] ^= iMask;
			iMask = iMask >> 1;
			j++;
			if (j == 32)
			{
				iMask = 32457;
				j = 0;
			}
		}

		// ��ȣȭ �� ���ڿ��� ���̳ʸ� ���� ���Ϸ� ������.
		fwrite(cFileSave, iFileSize, 1, fp);
		
		puts("��ȣȭ �Ϸ�!");
	}

	// 8�� �������� bCryptography�� true�� ��� ��ȣȭ ���� ����
	else 	
	{
		// ��ȣȭ �� ������ ������ �޸� ���� �����Ҵ�.
		// �޸� ������ �о�� ������ ũ�� - �� ����ǥ�� ���ڿ��̴�.
		// �� ����ǥ�� ���ڿ����� -1�� �ϴ� ������ ���� ���������� �ζ����̴�.
		cRefIleSave = (char*)malloc(iFileSize - (sizeof(cMyFileCheck) -1));

		for (int i = sizeof(cMyFileCheck) -1; i < iFileSize; ++i)
		{
			cRefIleSave[i - (sizeof(cMyFileCheck) -1)] = (cFileSave[i] ^ iMask);
			
			iMask = iMask >> 1;
			j++;
			if (j == 32)
			{
				iMask = 32457;
				j = 0;
			}
		}

		// ��ȣȭ �� ���ڿ��� ���̳ʸ� ���� ���Ϸ� ������.
		fwrite(cRefIleSave, iFileSize - (sizeof(cMyFileCheck) -1), 1, fp);

		puts("��ȣȭ �Ϸ�!");
	}

	// 10. ��� ��Ʈ�� ����
	fclose(fp);

	return 0;
}