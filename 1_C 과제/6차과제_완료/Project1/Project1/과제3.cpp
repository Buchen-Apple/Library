// ���� ��ŷ / ����ŷ
#pragma warning(disable:4996)

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct
{
	char m_name[30];
	int m_Size;
	int m_Offset;

}PackingFile;

typedef struct
{
	char* cContent;

}PackingContent;

int main()
{
	FILE* fp1;
	FILE* fp2;
	PackingFile* Filelist;
	PackingContent* FileContent;
	int iInput;
	int iFileCount;
	int iNowOffset;
	char c_MyFileCheck[11] = "SongJinGyu";
	char c_FackingFileName[20];
	char c_MyFileCheckSave[11];

	// 1. ��ŷ �ϱ�, Ǯ�� ����
	puts("1. ��ŷ�ϱ�");
	puts("2. ��ŷǮ��");
	scanf("%d", &iInput);

	switch (iInput)
	{
	case 1:
		// ��ŷ �ϱ� ����
		// 1. ��ŷ�� ���� ���� �Է�
		fputs("��ŷ�� ���� ������ �Է��ϼ��� : ", stdout);
		scanf("%d", &iFileCount);		

		// 2. �Է��� ���� ������ŭ Filelist�� �����Ҵ�.
		Filelist = (PackingFile*)malloc(sizeof(PackingFile) * iFileCount);
		memset(Filelist, 0, sizeof(PackingFile));

		FileContent = (PackingContent*)malloc(sizeof(PackingContent) * iFileCount);
		memset(FileContent, 0, sizeof(PackingContent));

		// 3. ��ŷ�� ���� �̸��� ��� �Է�.
		for (int i = 0; i < iFileCount; ++i)
		{
			printf("%d�� ���� �̸� �Է� : ", i);
			scanf("%s", Filelist[i].m_name);
		}

		// 4. ��ŷ �� �ϼ����� ���� �̸� �Է�.
		fputs("��ŷ �� ���� �̸� �Է� : ", stdout);
		scanf("%s", c_FackingFileName);
	

		// 5. ��ŷ�� ������ [������], [����]�� �����Ѵ�.
		for (int i = 0; i < iFileCount; ++i)
		{
			// ���� ��Ʈ�� ����
			fopen_s(&fp2, Filelist[i].m_name, "rb");

			// ������ ����� �����ϰ�.
			fseek(fp2, 0, SEEK_END);
			Filelist[i].m_Size = ftell(fp2);
			fseek(fp2, 0, SEEK_SET);

			// ���� ����
			FileContent[i].cContent = (char*)malloc(Filelist[i].m_Size);
			fread(FileContent[i].cContent, Filelist[i].m_Size, 1, fp2);

			// �� ������ ���� ��Ʈ�� ����.
			fclose(fp2);
		}	

		// 6. �о�� ���ϵ��� ������ �� ���� OffSet�� ���Ѵ�.
		// ������� �Ǹ�, iNowOffset�� ������ �� ���� ��ġ�� ���õȴ�.

		// [�� ���� ��ũ]�� �� ���� + [���� ����]�� �� ���� Ȯ��.
		iNowOffset = sizeof(c_MyFileCheck);
		iNowOffset += sizeof(iFileCount);

		//  �� ������ [�̸�], [������], [�����]�� �� ���� Ȯ��
		for (int i = 0; i < iFileCount; ++i)
		{
			iNowOffset += sizeof(Filelist[i].m_name);
			iNowOffset += sizeof(Filelist[i].m_Size);
			iNowOffset += sizeof(Filelist[i].m_Offset);
		}

		// 7. Offset�� ���� �����Ѵ�.
		for (int i = 0; i < iFileCount; ++i)
		{
			Filelist[i].m_Offset = iNowOffset;
			iNowOffset += Filelist[i].m_Size;
		}


		// 8. �ϼ����� ��� ��Ʈ�� ����
		fopen_s(&fp1, c_FackingFileName, "wb");

		// 9. �ϼ������ٰ� [�� ���� ��ũ], [���� ����]�� ������.
		fwrite(c_MyFileCheck, sizeof(c_MyFileCheck), 1, fp1);
		fwrite(&iFileCount, sizeof(iFileCount), 1, fp1);

		// 10. �о�� ������ [�̸�], [������], [�����]������ �ϼ����� ������.
		for (int i = 0; i < iFileCount; ++i)
		{
			// �̸��� ������.
			fwrite(Filelist[i].m_name, sizeof(Filelist[i].m_name), 1, fp1);

			// ����� ������
			fwrite(&Filelist[i].m_Size, sizeof(Filelist[i].m_Size), 1, fp1);
			
			// ����� ������.
			fwrite(&Filelist[i].m_Offset, sizeof(Filelist[i].m_Offset), 1, fp1);

		}
		// 11. ������ [����]�� �ϼ����� ������.

		for (int i = 0; i < iFileCount; ++i)
		{
			fwrite(FileContent[i].cContent, Filelist[i].m_Size, 1, fp1);
		}

		// 12. �ϼ��� ��Ʈ�� ����
		fclose(fp1);

		puts("���� ��ŷ �Ϸ�!");

		break;
	case 2:
		// ��ŷ Ǯ�� ����
		// 1. ��ŷ�� Ǯ �����̸� �Է�
		fputs("��ŷ�� Ǯ �����̸� �Է� : ", stdout);
		scanf("%s", c_FackingFileName);

		// 2. ������ �����ޱ� ���� ���̳ʸ� �б��� ����
		fopen_s(&fp1, c_FackingFileName, "rb");

		// 3. [�� ���� üũ]�� ���� �� ��ŭ�� �ӽ÷� �����޴´�.
		fread(c_MyFileCheckSave, sizeof(c_MyFileCheckSave), 1, fp1);
		if (0 != strcmp(c_MyFileCheckSave, c_MyFileCheck))
		{
			puts("�� ������ �ƴմϴ�!");
			return -1;
		}

		// 4. �� �����̶�°� �����Ǿ�����, �̾ ���� ������ �����޴´�.
		fread((char*)&iFileCount, sizeof(iFileCount), 1, fp1);
		
		// 5. ���� ������ŭ ���� ����Ʈ�� �����Ҵ� ��, [�̸�], [���� ũ��], [�����]�� �����޴´�.
		Filelist = (PackingFile*)malloc(sizeof(PackingFile) * iFileCount);
		memset(Filelist, 0, sizeof(PackingFile));

		for (int i = 0; i < iFileCount; ++i)
		{
			// �̸��� �����ް�
			fread(Filelist[i].m_name, sizeof(Filelist[i].m_name), 1, fp1);

			// ���� ũ�⸦ �����ް�
			fread(&Filelist[i].m_Size, sizeof(Filelist[i].m_Size), 1, fp1);

			// ������� �����޴´�.
			fread(&Filelist[i].m_Offset, sizeof(Filelist[i].m_Offset), 1, fp1);

		}

		// 6. ������ ���� ����ü�� ���� ������ŭ �����Ҵ� ��, ������� �����޴´�.
		FileContent = (PackingContent*)malloc(sizeof(PackingContent) * iFileCount);
		memset(FileContent, 0, sizeof(PackingContent));

		for (int i = 0; i < iFileCount; ++i)
		{
			FileContent[i].cContent = (char*)malloc(Filelist[i].m_Size);
			fread(FileContent[i].cContent, Filelist[i].m_Size, 1, fp1);
		}

		fclose(fp1);

		// 7. ���� ������ŭ ������ �����Ѵ�.
		for (int i = 0; i < iFileCount; ++i)
		{
			// ������ ������ ���� ��� ��Ʈ�� ����. ���°� �Բ� ���� ����.
			fopen_s(&fp2, Filelist[i].m_name, "wb");

			// ���� ������ ������.
			fwrite(FileContent[i].cContent, Filelist[i].m_Size, 1, fp2);

			// �������� ��Ʈ�� �ݴ´�.
			fclose(fp2);
		}

		puts("��ŷ Ǯ�� �Ϸ�");
		
		break;
	}

	return 0;
}