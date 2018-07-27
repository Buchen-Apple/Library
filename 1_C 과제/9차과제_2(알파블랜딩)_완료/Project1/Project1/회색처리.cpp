#include <stdio.h>
#include <stdlib.h>
#include <windows.h>

int main()
{
	BITMAPFILEHEADER FileHeader1;
	BITMAPINFOHEADER InfoHeader1;
	UINT* cFileBuffer_1;

	UINT Red1, Green1, Blue1, Alpha1;
	UINT RGBAverage;

	UINT iBreakCheck = 0;
	bool bBreakFlag = false;

	FILE* fp1;
	FILE* fp3;

	// ������ ��Ʈ�� ����
	fopen_s(&fp1, "aa.bmp", "rb");

	// ������ �������,������� �о��
	fread(&FileHeader1, sizeof(FileHeader1), 1, fp1);
	fread(&InfoHeader1, sizeof(InfoHeader1), 1, fp1);

	// ������ �̹��� ũ�⸸ŭ �����Ҵ�
	cFileBuffer_1 = (UINT*)malloc(InfoHeader1.biSizeImage);

	printf("1 : %d\n", InfoHeader1.biSizeImage);

	// ������ �̹��� ũ�⸸ŭ �������� �о�´�.
	fread(cFileBuffer_1, InfoHeader1.biSizeImage, 1, fp1);

	// 1,2�� ���� �о���� ��. ���� ��Ʈ�� �ݴ´�.
	fclose(fp1);

	// ���� �� ���� ��Ʈ�� ����
	fopen_s(&fp3, "Gray.bmp", "wb");

	// ���� ������ ������ [���� ���], [���� ���] ����
	fwrite(&FileHeader1, sizeof(FileHeader1), 1, fp3);
	fwrite(&InfoHeader1, sizeof(InfoHeader1), 1, fp3);

	// R, G, B, A�� �и��ؼ� ���� ��� �� ���ĺ����Ѵ�.
	while (1)
	{
		// iBreakCheck�� ���� ũ�⺸�� Ŀ���� ������ ������ ��.
		if (iBreakCheck > InfoHeader1.biSizeImage)
			bBreakFlag = true;

		// ������ R,G,B,A�� �и��Ѵ�.
		Red1 = *cFileBuffer_1 & 0x000000ff;
		Green1 = (*cFileBuffer_1 & 0x0000ff00) >> 8;
		Blue1 = (*cFileBuffer_1 & 0x00ff0000) >> 16;
		Alpha1 = (*cFileBuffer_1 & 0xff000000) >> 24;

		// RGB�� ����� ���� R,G,B ��� �� ���� ���´�. �̰� ȸ��ó���ϴ� ���.
		RGBAverage = (Red1 + Green1 + Blue1) / 3;

		// ������ RGBA �ڸ��� ��հ��� ���� ������.
		// Alpha�ڸ����� �׳� ������ Alpha�� ������.
		fwrite(&RGBAverage, 1, 1, fp3);
		fwrite(&RGBAverage, 1, 1, fp3);
		fwrite(&RGBAverage, 1, 1, fp3);
		fwrite(&Alpha1, 1, 1, fp3);

		// �̹��� ������ ����������, while�� ����
		if (bBreakFlag == true)
			break;

		// �� ���� ������ ���� 1�� ����.
		// �ٵ�, �������̱� ������, ����Ʈ�� ��(int)��ŭ ����. ��, 4����Ʈ ����.
		cFileBuffer_1++;

		// ���Ḧ üũ�� �������� 4 ����. ����� ����Ʈ ������ �����Ѵ�.
		iBreakCheck += 4;
	}

	// ���� ����� �������� ������ �ݴ´�.
	fclose(fp3);

	return 0;
}