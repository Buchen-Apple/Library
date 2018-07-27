#include <stdio.h>
#include <stdlib.h>
#include <windows.h>

int main()
{
	BITMAPFILEHEADER FileHeader1;
	BITMAPFILEHEADER FileHeader2;
	UINT* cFileBuffer_1;

	BITMAPINFOHEADER InfoHeader1;	
	BITMAPINFOHEADER InfoHeader2;
	UINT* cFileBuffer_2;

	UINT Red1, Red2, Green1, Green2, Blue1, Blue2, Alpha1, Alpha2;
	UINT Red3, Green3, Blue3, Alpha3;

	UINT iBreakCheck = 0;
	bool bBreakFlag = false;

	FILE* fp1;
	FILE* fp2;
	FILE* fp3;

	// 1��° ����, 2��° ������ ��Ʈ�� ����
	fopen_s(&fp1, "aa.bmp", "rb");
	fopen_s(&fp2, "bb.bmp", "rb");

	// 1�� ������ �������,������� �о��
	fread(&FileHeader1, sizeof(FileHeader1), 1, fp1);
	fread(&InfoHeader1, sizeof(InfoHeader1), 1, fp1);

	// 2�� ������ �������,������� �о��
	fread(&FileHeader2, sizeof(FileHeader2), 1, fp2);	
	fread(&InfoHeader2, sizeof(InfoHeader2), 1, fp2);

	// �� ������ �̹��� ũ�⸸ŭ �����Ҵ�
	cFileBuffer_1 = (UINT*)malloc(InfoHeader1.biSizeImage);
	cFileBuffer_2 = (UINT*)malloc(InfoHeader2.biSizeImage);

	printf("1 : %d\n", InfoHeader1.biSizeImage);
	printf("2 : %d\n", InfoHeader2.biSizeImage);

	// �� ������ �̹��� ũ�⸸ŭ �������� �о�´�.
	fread(cFileBuffer_1, InfoHeader1.biSizeImage, 1, fp1);
	fread(cFileBuffer_2, InfoHeader2.biSizeImage, 1, fp2);

	// 1,2�� ���� �о���� ��. ���� ��Ʈ�� �ݴ´�.
	fclose(fp1);
	fclose(fp2);

	// ���� �� ���� ��Ʈ�� ����
	fopen_s(&fp3, "blend.bmp", "wb");

	// ���� ������ ������ [���� ���], [���� ���] ����
	fwrite(&FileHeader1, sizeof(FileHeader1), 1, fp3);
	fwrite(&InfoHeader1, sizeof(InfoHeader1), 1, fp3);

	// R, G, B, A�� �и��ؼ� ���� ��� �� ���ĺ����Ѵ�.
	while (1)
	{
		// iBreakCheck�� ���� ũ�⺸�� Ŀ���� ������ ������ ��.
		if (iBreakCheck > InfoHeader1.biSizeImage)
			bBreakFlag = true;
		
		// 1�� ������ R,G,B,A�� �и��Ѵ�.
		Red1 = *cFileBuffer_1 & 0x000000ff;
		Green1 = (*cFileBuffer_1 & 0x0000ff00) >> 8;
		Blue1 = (*cFileBuffer_1 & 0x00ff0000) >> 16;
		Alpha1 = (*cFileBuffer_1  & 0xff000000) >> 24;

		// 2�� ������ R,G,B,A�� �и��Ѵ�.
		Red2 = *cFileBuffer_2 & 0x000000ff;
		Green2 = (*cFileBuffer_2 & 0x0000ff00) >> 8;
		Blue2 = (*cFileBuffer_2 & 0x00ff0000) >> 16;
		Alpha2 = (*cFileBuffer_2 & 0xff000000) >> 24;
		
		// �� ���Ͽ� /2 �Ѱ��� ���� ���Ѵ�. �̰� ���ĺ����ϴ� ����̴� (�� ������ RGBA���� �������� ���� �� ���� ��ħ)
		Red3 = (Red1 / 2) + (Red2 / 2);
		Green3 = (Green1 / 2) + (Green2 / 2);
		Blue3 = (Blue1 / 2) + (Blue2 / 2);
		Alpha3 = (Alpha1 / 2) + (Alpha2 / 2);

		// R,G,B,A�� ���ο� ���Ϸ� ������.
		fwrite(&Red3, 1, 1, fp3);
		fwrite(&Green3, 1, 1, fp3);
		fwrite(&Blue3, 1, 1, fp3);
		fwrite(&Alpha3, 1, 1, fp3);

		// �̹��� ������ ����������, while�� ����
		if (bBreakFlag == true)
			break;

		// �� ���� ������ ���� 1�� ����.
		// �ٵ�, �������̱� ������, ����Ʈ�� ��(int)��ŭ ����. ��, 4����Ʈ ����.
		cFileBuffer_1++;
		cFileBuffer_2++;

		// ���Ḧ üũ�� �������� 4 ����. ����� ����Ʈ ������ �����Ѵ�.
		iBreakCheck += 4;
	}

	// ���� ����� �������� ������ �ݴ´�.
	fclose(fp3);
	
	return 0;
}