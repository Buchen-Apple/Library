#include "stdafx.h"
#include "SpriteDIB.h"
#include <stdio.h>

// *******************************************
// ������, �ı��� (�̱����� ���� �����ڴ� Private)
// *******************************************
CSpritedib::CSpritedib(int iMaxSprite, DWORD dwColorKey, int iColorBit)
{
	// �ش� Ŭ������ �ִ� ��������Ʈ ���� ������ ����.	
	m_iMaxSprite = iMaxSprite;

	// �ִ� ��������Ʈ �� ��ŭ �̸� ������ �Ҵ�޴´�.
	m_stpSprite = new st_SPRITE[m_iMaxSprite];

	// �÷�Ű�� �����Ѵ�. �ش� Ŭ������ ����Ǵ� ��� ��������Ʈ�� ������ �÷�Ű�� ó���ȴ�.
	m_dwColorKey = dwColorKey;

	// �ش� ��������Ʈ�� �÷� ��Ʈ ����
	m_iColorByte = iColorBit / 8;
}

CSpritedib::~CSpritedib()
{
	// ��ü�� ���鼭 ��� ��������Ʈ�� �����.
	for (int i = 0; i < m_iMaxSprite; ++i)
		ReleseSprite(i);
}


// *******************************************
// �̱��� ���� �Լ�
// *******************************************
CSpritedib* CSpritedib::Getinstance(int iMaxSprite, DWORD dwColorKey, int iColorBit)
{
	static CSpritedib cSpritedib(iMaxSprite, dwColorKey, iColorBit);
	return &cSpritedib;
}

//*******************************************
// ���� �ʹ� ���� �Լ�(����� ��� ��Ʈ���� �о�´�
//*******************************************
void CSpritedib::GameInit()
{
	// �ʿ��� ��������Ʈ �ε��صα�.
	// �� �ε�
	BOOL iCheck = LoadDibSprite(eMAP, "..\\SpriteData\\_Map.bmp", 0, 0);
	if (!iCheck)
		exit(-1);

	// ���� Idle
	iCheck = LoadDibSprite(ePLAYER_STAND_L01, "..\\SpriteData\\Stand_L_01.bmp", 71, 90);
	if (!iCheck)
		exit(-1);

	iCheck = LoadDibSprite(ePLAYER_STAND_L02, "..\\SpriteData\\Stand_L_02.bmp", 71, 90);
	if (!iCheck)
		exit(-1);

	iCheck = LoadDibSprite(ePLAYER_STAND_L03, "..\\SpriteData\\Stand_L_03.bmp", 71, 90);
	if (!iCheck)
		exit(-1);

	iCheck = LoadDibSprite(ePLAYER_STAND_L04, "..\\SpriteData\\Stand_L_02.bmp", 71, 90);
	if (!iCheck)
		exit(-1);

	iCheck = LoadDibSprite(ePLAYER_STAND_L05, "..\\SpriteData\\Stand_L_01.bmp", 71, 90);
	if (!iCheck)
		exit(-1);


	// ���� Idle
	iCheck = LoadDibSprite(ePLAYER_STAND_R01, "..\\SpriteData\\Stand_R_01.bmp", 71, 90);
	if (!iCheck)
		exit(-1);

	iCheck = LoadDibSprite(ePLAYER_STAND_R02, "..\\SpriteData\\Stand_R_02.bmp", 71, 90);
	if (!iCheck)
		exit(-1);

	iCheck = LoadDibSprite(ePLAYER_STAND_R03, "..\\SpriteData\\Stand_R_03.bmp", 71, 90);
	if (!iCheck)
		exit(-1);

	iCheck = LoadDibSprite(ePLAYER_STAND_R04, "..\\SpriteData\\Stand_R_02.bmp", 71, 90);
	if (!iCheck)
		exit(-1);

	iCheck = LoadDibSprite(ePLAYER_STAND_R05, "..\\SpriteData\\Stand_R_01.bmp", 71, 90);
	if (!iCheck)
		exit(-1);


	// �·� �̵�
	iCheck = LoadDibSprite(ePLAYER_MOVE_L01, "..\\SpriteData\\Move_L_01.bmp", 71, 90);
	if (!iCheck)
		exit(-1);

	iCheck = LoadDibSprite(ePLAYER_MOVE_L02, "..\\SpriteData\\Move_L_02.bmp", 71, 90);
	if (!iCheck)
		exit(-1);

	iCheck = LoadDibSprite(ePLAYER_MOVE_L03, "..\\SpriteData\\Move_L_03.bmp", 71, 90);
	if (!iCheck)
		exit(-1);

	iCheck = LoadDibSprite(ePLAYER_MOVE_L04, "..\\SpriteData\\Move_L_04.bmp", 71, 90);
	if (!iCheck)
		exit(-1);

	iCheck = LoadDibSprite(ePLAYER_MOVE_L05, "..\\SpriteData\\Move_L_05.bmp", 71, 90);
	if (!iCheck)
		exit(-1);

	iCheck = LoadDibSprite(ePLAYER_MOVE_L06, "..\\SpriteData\\Move_L_06.bmp", 71, 90);
	if (!iCheck)
		exit(-1);

	iCheck = LoadDibSprite(ePLAYER_MOVE_L07, "..\\SpriteData\\Move_L_07.bmp", 71, 90);
	if (!iCheck)
		exit(-1);

	iCheck = LoadDibSprite(ePLAYER_MOVE_L08, "..\\SpriteData\\Move_L_08.bmp", 71, 90);
	if (!iCheck)
		exit(-1);

	iCheck = LoadDibSprite(ePLAYER_MOVE_L09, "..\\SpriteData\\Move_L_09.bmp", 71, 90);
	if (!iCheck)
		exit(-1);

	iCheck = LoadDibSprite(ePLAYER_MOVE_L10, "..\\SpriteData\\Move_L_10.bmp", 71, 90);
	if (!iCheck)
		exit(-1);

	iCheck = LoadDibSprite(ePLAYER_MOVE_L11, "..\\SpriteData\\Move_L_11.bmp", 71, 90);
	if (!iCheck)
		exit(-1);

	iCheck = LoadDibSprite(ePLAYER_MOVE_L12, "..\\SpriteData\\Move_L_12.bmp", 71, 90);
	if (!iCheck)
		exit(-1);


	// ��� �̵�
	iCheck = LoadDibSprite(ePLAYER_MOVE_R01, "..\\SpriteData\\Move_R_01.bmp", 71, 90);
	if (!iCheck)
		exit(-1);

	iCheck = LoadDibSprite(ePLAYER_MOVE_R02, "..\\SpriteData\\Move_R_02.bmp", 71, 90);
	if (!iCheck)
		exit(-1);

	iCheck = LoadDibSprite(ePLAYER_MOVE_R03, "..\\SpriteData\\Move_R_03.bmp", 71, 90);
	if (!iCheck)
		exit(-1);

	iCheck = LoadDibSprite(ePLAYER_MOVE_R04, "..\\SpriteData\\Move_R_04.bmp", 71, 90);
	if (!iCheck)
		exit(-1);

	iCheck = LoadDibSprite(ePLAYER_MOVE_R05, "..\\SpriteData\\Move_R_05.bmp", 71, 90);
	if (!iCheck)
		exit(-1);

	iCheck = LoadDibSprite(ePLAYER_MOVE_R06, "..\\SpriteData\\Move_R_06.bmp", 71, 90);
	if (!iCheck)
		exit(-1);

	iCheck = LoadDibSprite(ePLAYER_MOVE_R07, "..\\SpriteData\\Move_R_07.bmp", 71, 90);
	if (!iCheck)
		exit(-1);

	iCheck = LoadDibSprite(ePLAYER_MOVE_R08, "..\\SpriteData\\Move_R_08.bmp", 71, 90);
	if (!iCheck)
		exit(-1);

	iCheck = LoadDibSprite(ePLAYER_MOVE_R09, "..\\SpriteData\\Move_R_09.bmp", 71, 90);
	if (!iCheck)
		exit(-1);

	iCheck = LoadDibSprite(ePLAYER_MOVE_R10, "..\\SpriteData\\Move_R_10.bmp", 71, 90);
	if (!iCheck)
		exit(-1);

	iCheck = LoadDibSprite(ePLAYER_MOVE_R11, "..\\SpriteData\\Move_R_11.bmp", 71, 90);
	if (!iCheck)
		exit(-1);

	iCheck = LoadDibSprite(ePLAYER_MOVE_R12, "..\\SpriteData\\Move_R_12.bmp", 71, 90);
	if (!iCheck)
		exit(-1);


	// 1�� ����(����)
	iCheck = LoadDibSprite(ePLAYER_ATTACK1_L01, "..\\SpriteData\\Attack1_L_01.bmp", 71, 90);
	if (!iCheck)
		exit(-1);

	iCheck = LoadDibSprite(ePLAYER_ATTACK1_L02, "..\\SpriteData\\Attack1_L_02.bmp", 71, 90);
	if (!iCheck)
		exit(-1);

	iCheck = LoadDibSprite(ePLAYER_ATTACK1_L03, "..\\SpriteData\\Attack1_L_03.bmp", 71, 90);
	if (!iCheck)
		exit(-1);

	iCheck = LoadDibSprite(ePLAYER_ATTACK1_L04, "..\\SpriteData\\Attack1_L_04.bmp", 71, 90);
	if (!iCheck)
		exit(-1);


	// 1�� ����(����)
	iCheck = LoadDibSprite(ePLAYER_ATTACK1_R01, "..\\SpriteData\\Attack1_R_01.bmp", 71, 90);
	if (!iCheck)
		exit(-1);

	iCheck = LoadDibSprite(ePLAYER_ATTACK1_R02, "..\\SpriteData\\Attack1_R_02.bmp", 71, 90);
	if (!iCheck)
		exit(-1);

	iCheck = LoadDibSprite(ePLAYER_ATTACK1_R03, "..\\SpriteData\\Attack1_R_03.bmp", 71, 90);
	if (!iCheck)
		exit(-1);

	iCheck = LoadDibSprite(ePLAYER_ATTACK1_R04, "..\\SpriteData\\Attack1_R_04.bmp", 71, 90);
	if (!iCheck)
		exit(-1);


	// 2�� ����(����)
	iCheck = LoadDibSprite(ePLAYER_ATTACK2_L01, "..\\SpriteData\\Attack2_L_01.bmp", 71, 90);
	if (!iCheck)
		exit(-1);

	iCheck = LoadDibSprite(ePLAYER_ATTACK2_L02, "..\\SpriteData\\Attack2_L_02.bmp", 71, 90);
	if (!iCheck)
		exit(-1);

	iCheck = LoadDibSprite(ePLAYER_ATTACK2_L03, "..\\SpriteData\\Attack2_L_03.bmp", 71, 90);
	if (!iCheck)
		exit(-1);

	iCheck = LoadDibSprite(ePLAYER_ATTACK2_L04, "..\\SpriteData\\Attack2_L_04.bmp", 71, 90);
	if (!iCheck)
		exit(-1);


	// 2�� ����(����)
	iCheck = LoadDibSprite(ePLAYER_ATTACK2_R01, "..\\SpriteData\\Attack2_R_01.bmp", 71, 90);
	if (!iCheck)
		exit(-1);

	iCheck = LoadDibSprite(ePLAYER_ATTACK2_R02, "..\\SpriteData\\Attack2_R_02.bmp", 71, 90);
	if (!iCheck)
		exit(-1);

	iCheck = LoadDibSprite(ePLAYER_ATTACK2_R03, "..\\SpriteData\\Attack2_R_03.bmp", 71, 90);
	if (!iCheck)
		exit(-1);

	iCheck = LoadDibSprite(ePLAYER_ATTACK2_R04, "..\\SpriteData\\Attack2_R_04.bmp", 71, 90);
	if (!iCheck)
		exit(-1);


	// 3�� ����(����)
	iCheck = LoadDibSprite(ePLAYER_ATTACK3_L01, "..\\SpriteData\\Attack3_L_01.bmp", 71, 90);
	if (!iCheck)
		exit(-1);

	iCheck = LoadDibSprite(ePLAYER_ATTACK3_L02, "..\\SpriteData\\Attack3_L_02.bmp", 71, 90);
	if (!iCheck)
		exit(-1);

	iCheck = LoadDibSprite(ePLAYER_ATTACK3_L03, "..\\SpriteData\\Attack3_L_03.bmp", 71, 90);
	if (!iCheck)
		exit(-1);

	iCheck = LoadDibSprite(ePLAYER_ATTACK3_L04, "..\\SpriteData\\Attack3_L_04.bmp", 71, 90);
	if (!iCheck)
		exit(-1);

	iCheck = LoadDibSprite(ePLAYER_ATTACK3_L05, "..\\SpriteData\\Attack3_L_05.bmp", 71, 90);
	if (!iCheck)
		exit(-1);

	iCheck = LoadDibSprite(ePLAYER_ATTACK3_L06, "..\\SpriteData\\Attack3_L_06.bmp", 71, 90);
	if (!iCheck)
		exit(-1);


	// 3�� ����(����)
	iCheck = LoadDibSprite(ePLAYER_ATTACK3_R01, "..\\SpriteData\\Attack3_R_01.bmp", 71, 90);
	if (!iCheck)
		exit(-1);

	iCheck = LoadDibSprite(ePLAYER_ATTACK3_R02, "..\\SpriteData\\Attack3_R_02.bmp", 71, 90);
	if (!iCheck)
		exit(-1);

	iCheck = LoadDibSprite(ePLAYER_ATTACK3_R03, "..\\SpriteData\\Attack3_R_03.bmp", 71, 90);
	if (!iCheck)
		exit(-1);

	iCheck = LoadDibSprite(ePLAYER_ATTACK3_R04, "..\\SpriteData\\Attack3_R_04.bmp", 71, 90);
	if (!iCheck)
		exit(-1);

	iCheck = LoadDibSprite(ePLAYER_ATTACK3_R05, "..\\SpriteData\\Attack3_R_05.bmp", 71, 90);
	if (!iCheck)
		exit(-1);

	iCheck = LoadDibSprite(ePLAYER_ATTACK3_R06, "..\\SpriteData\\Attack3_R_06.bmp", 71, 90);
	if (!iCheck)
		exit(-1);

	// ����Ʈ 
	iCheck = LoadDibSprite(eEFFECT_SPARK_01, "..\\SpriteData\\xSpark_1.bmp", 70, 70);
	if (!iCheck)
		exit(-1);

	iCheck = LoadDibSprite(eEFFECT_SPARK_02, "..\\SpriteData\\xSpark_2.bmp", 70, 70);
	if (!iCheck)
		exit(-1);

	iCheck = LoadDibSprite(eEFFECT_SPARK_03, "..\\SpriteData\\xSpark_3.bmp", 70, 70);
	if (!iCheck)
		exit(-1);

	iCheck = LoadDibSprite(eEFFECT_SPARK_04, "..\\SpriteData\\xSpark_4.bmp", 70, 70);
	if (!iCheck)
		exit(-1);

	// �׸���, HP��
	iCheck = LoadDibSprite(eGUAGE_HP, "..\\SpriteData\\HPGuage.bmp", 0, 0);
	if (!iCheck)
		exit(-1);

	iCheck = LoadDibSprite(eSHADOW, "..\\SpriteData\\Shadow.bmp", 32, 4);
	if (!iCheck)
		exit(-1);



}

//*******************************************
// ��������Ʈ ���������� ���� �Լ� (���� �Ⱦ�����)
//*******************************************
void CSpritedib::RedChange(int iSpriteIndex)
{
	// ��������Ʈ�� ���������� �����ϱ�.

	// ���������� 1�پ� �Ʒ��� �̵�
	for (int i = 0; i < m_stpSprite[iSpriteIndex].iHeight; ++i)
	{
		// �� �پ� �Ʒ��� �̵��ϸ鼭 �ȼ������� >> �̵��ϸ� �÷��� �Ͼ������ üũ�Ѵ�.
		for (int j = 0; j < m_stpSprite[iSpriteIndex].iPitch; j += m_iColorByte)
		{
			// �ش� �ȼ��� �÷��� �Ͼ���� �ƴ� ���� �� ����ۿ� ��´�. �Ͼ���̸� ���� �ʰ� ���� �ȼ��� �̵��Ѵ�.
			// memcmp�� ������ 0��ȯ. �ٸ��� 1 ��ȯ. ��, �Ͼ���� �ƴϸ� 1�� ��ȯ�Ǿ� ���� �Ǿ� ���� ����.
			if (memcmp(m_stpSprite[iSpriteIndex].bypImge + j, &m_dwColorKey, m_iColorByte))
			{
				DWORD TempRGB;

				memcpy(&TempRGB, m_stpSprite[iSpriteIndex].bypImge + j, m_iColorByte);

				// GB�� /2�ؼ� �������� ����.
				BYTE r = GetRValue(TempRGB);
				BYTE g = GetGValue(TempRGB);
				BYTE b = GetBValue(TempRGB);

				g = g / 2;
				b = b / 2;

				// �ٽ� ��ģ��. ���� 4����Ʈ�� ����ϱ� ������, ARGB ������� �Ǿ��ִ� �����Ͱ� �ʿ��ϴ�.
				TempRGB = RGB(b, g, r);
				memcpy(m_stpSprite[iSpriteIndex].bypImge + j, &TempRGB, m_iColorByte);	// 1�ȼ�(4����Ʈ)�� ī���Ѵ�.
			}
		}
		m_stpSprite[iSpriteIndex].bypImge += m_stpSprite[iSpriteIndex].iPitch;
	}

}


// *******************************************
// BMP������ �о �ϳ��� ��������Ʈ�� ����.
// *******************************************
BOOL CSpritedib::LoadDibSprite(int iSpriteIndex, const char* szFileName, int iCenterPointX, int iCenterPointY)
{
	// ����, ���޵� �ε����� �ִ� ��������Ʈ �������� ũ�ٸ� false ��ȯ.
	if (m_iMaxSprite <= iSpriteIndex)
		return FALSE;

	size_t				dwRead;
	int					iPitch;
	int					iImageSize;
	BITMAPFILEHEADER	stFileHeader;
	BITMAPINFOHEADER	stInfoHeader;

	// ��Ʈ�� ����� ���� BMP ���� Ȯ��-------------
	FILE* fp;
	int iCheck = fopen_s(&fp, szFileName, "rb");
	if (iCheck != NULL)
		return FALSE;

	// �������, ������� �о����.----------------
	dwRead = fread(&stFileHeader, sizeof(BITMAPFILEHEADER), 1, fp);
	if (dwRead != 1)
		return FALSE;

	fread(&stInfoHeader, sizeof(BITMAPINFOHEADER), 1, fp);
	if (dwRead != 1)
		return FALSE;

	// ��ġ�� ���ϱ�--------------
	iPitch = (stInfoHeader.biWidth * (stInfoHeader.biBitCount / 8)) + 3 & ~3;	// ���� ���� �޸� ���� (pitch�� ����Ʈ ���� ���� ���� ����)	

																				// ��������Ʈ ����ü�� �ʿ��� �� ����.--------------
	m_stpSprite[iSpriteIndex].iCenterPointX = iCenterPointX;	// ���� x ����
	m_stpSprite[iSpriteIndex].iCenterPointY = iCenterPointY;	// ���� y����
	m_stpSprite[iSpriteIndex].iHeight = stInfoHeader.biHeight;	// ���� ����
	m_stpSprite[iSpriteIndex].iPitch = iPitch;					// ���� pitch ����
	m_stpSprite[iSpriteIndex].iWidth = stInfoHeader.biWidth;	// ���� ����.

																// �̹��� ��ü ũ�� ���� �� ��������Ʈ ���ۿ� �޸� �����Ҵ�.
	iImageSize = iPitch * stInfoHeader.biHeight;
	m_stpSprite[iSpriteIndex].bypImge = new BYTE[iImageSize];

	// ---------------------------------------
	// �̹����� ��������Ʈ ���۷� �о�´�.
	// DIB�� �������� ������ �ӽ� ���ۿ� ���� ��, �������鼭 �� ��������Ʈ ���ۿ� �����Ѵ�.
	// ---------------------------------------
	BYTE* bypTempBuffer = new BYTE[iImageSize];
	fread(bypTempBuffer, iImageSize, 1, fp);
	if (dwRead != 1)
		exit(-1);

	// ��Ʈ�� �̹����� ���̸�ŭ �ݺ��ϸ� ��Ʈ�� �ϴܺ��� ���� (�������� ����)
	bypTempBuffer += (iImageSize - iPitch);
	for (int i = 0; i < stInfoHeader.biHeight; ++i)
	{
		memcpy(m_stpSprite[iSpriteIndex].bypImge, bypTempBuffer, iPitch);
		m_stpSprite[iSpriteIndex].bypImge += iPitch;
		bypTempBuffer -= iPitch;
	}

	// �� ��������Ʈ ������ �ּҸ� ������Ű�鼭 ��������� ���� ��ġ�� �̵���Ų��.
	m_stpSprite[iSpriteIndex].bypImge -= (iPitch * stInfoHeader.biHeight);

	// ���� ��Ʈ�� �ݰ� �� �� �ڵ� Ŭ����-------------
	fclose(fp);
	return TRUE;
}

// *******************************************
// ���޵� Index�� ��������Ʈ�� ����.
// *******************************************
void CSpritedib::ReleseSprite(int iSpriteIndex)
{
	// ���޵� ��������Ʈ Index�� �ִ� ��������Ʈ �̻��̶�� �߸��� �����̴� �׳� ����.
	if (m_iMaxSprite <= iSpriteIndex)
		return;

	// ���޵� ��������Ʈ Index�� ��� ���� ��������Ʈ���, �׶� ����.
	else if (NULL != m_stpSprite[iSpriteIndex].bypImge)
	{
		delete[] m_stpSprite[iSpriteIndex].bypImge;		// ����, m_stpSprite�� �Ҵ�� Img�����Ҵ� ���� ����.
		memset(&m_stpSprite[iSpriteIndex], 0, sizeof(st_SPRITE));	// �׸���, �ش� Index�� �޸𸮸� 0���� ���½�Ų��. ������� �ϸ� ������ ��.
	}
}

// *******************************************
// Ư�� �޸� ��ġ�� ��������Ʈ�� ����Ѵ� (Į��Ű/Ŭ���� ó��, ĳ���� ������ ���)
// *******************************************
BOOL CSpritedib::DrawSprite(int iSpriteIndex, int iDrawX, int iDrawY, BYTE* bypDest, int iDestWidth,
	int iDestHeight, int iDestPitch, BOOL isPlayer, int iDrawLen)
{
	// ����, ���޵� �ε����� �ִ� ��������Ʈ �������� ũ�ٸ� ���� ��������Ʈ�� ����϶�� �� ���̴�, false ��ȯ.
	if (m_iMaxSprite <= iSpriteIndex)
		return FALSE;

	BYTE* save = m_stpSprite[iSpriteIndex].bypImge;	// �پ� �� ������ ��������Ʈ �̹����� ���� �ּ�(0,0)

													// �ش� �Լ������� �����, �������� �����Ѵ�. Ŭ������ �ϱ� ������ ���� ������ �ʿ��ϴ�.
													//int iSpriteWidth = m_stpSprite[iSpriteIndex].iWidth;		<<�̰� �����Դµ� �ȽἭ �׳� �ּ�ó����. ���߿� �ʿ��ϸ� �ּ� ��������.
	int iSpriteHeight = m_stpSprite[iSpriteIndex].iHeight;
	int iSpritePitch = m_stpSprite[iSpriteIndex].iPitch;

	// iDrawX, iDrawY�� ��������Ʈ�� ������ ���εǴ� ��ǥ�̴�. ���� ��Ʈ���� ���� �ϴ� �� ������� x,y ��ġ�� ���Ѵ�. (iDestStartX, iDestStartY)
	// ���� ������ �ȼ� ������ ����̴�.
	int iDestStartX = iDrawX - m_stpSprite[iSpriteIndex].iCenterPointX;
	int iDestStartY = iDrawY - m_stpSprite[iSpriteIndex].iCenterPointY;

	// ��������Ʈ �̹�����, ���� ��ġ�� ���Ѵ�. Ŭ���� ó���� �Ϸ���, �̹��� ��� ���� ��ġ�� �̵����Ѿ� �Ѵ�.
	int iSpriteStartX = 0;
	int iSpriteStartY = 0;

	// Ŭ���� ó�� -------------
	// ��
	if (iDestStartY < 0)
	{
		int iAddY = iDestStartY * -1;	// ���� �󸶳� �ö󰬴��� ����.
		iDestStartY = 0;				// �ϴ�, Dest(�� �����)�� y��ġ�� 0�̴�.
		iSpriteStartY += iAddY;			// ��������Ʈ�� ������ y ��ġ�� �Ʒ��� iAddY��ŭ ������.
		iSpriteHeight -= iAddY;			// ��������Ʈ�� ���̸� iAddY��ŭ ���δ�. 
	}

	// ��
	if (iDestStartY + m_stpSprite[iSpriteIndex].iHeight > iDestHeight)
	{
		int iAddY = iDestStartY + m_stpSprite[iSpriteIndex].iHeight - iDestHeight;		// �Ʒ��� �󸶳� ���������� ����	
		iSpriteHeight -= iAddY;															// ��������Ʈ�� ���̸� iAddY��ŭ ���δ�. 		
	}

	// ��
	int iAddX = 0;
	if (iDestStartX < 0)
	{
		iAddX = iDestStartX * -1;	// �·� �󸶳� ������ ����.
		iDestStartX = 0;				// �ϴ�, Dest(�� �����)�� x��ġ�� 0�̴�.
		iSpriteStartX += iAddX;			// ��������Ʈ�� ������ x ��ġ�� �������� iAddX��ŭ �����δ�.
	}

	// ��
	if (iDestStartX + m_stpSprite[iSpriteIndex].iWidth > iDestWidth)
		iAddX = iDestStartX + m_stpSprite[iSpriteIndex].iWidth - iDestWidth;	// ��� �󸶳� ������ ����. ��� �̷��� ��!

																				// ���� ������ �̵� -------------
																				// Dest�� �̹����� ���� ������ �̵�, ��������Ʈ�� ���� ������ �̵�
																				// ���⼭ ���� ����Ʈ ������ ����Ѵ�.
	bypDest += (iDestStartX * m_iColorByte) + (iDestStartY * iDestPitch);
	m_stpSprite[iSpriteIndex].bypImge += (iSpriteStartX * m_iColorByte) + (iSpriteStartY * iSpritePitch);

	// ��������Ʈ ���� -------------
	// ���������� 1�پ� �Ʒ��� �̵�
	DWORD dwMasking;;
	DWORD Temp;

	for (int i = 0; i < iSpriteHeight; ++i)
	{
		// �� �پ� �Ʒ��� �̵��ϸ鼭 �ȼ������� >> �̵��ϸ� �÷��� �Ͼ������ üũ�Ѵ�.
		for (int j = 0; j < ((iSpritePitch - (iAddX*m_iColorByte)) / 100.0f) * iDrawLen; j += m_iColorByte)
		{
			// dwMasking��, �̹��� ���� 1�ȼ��� �� ����.
			memcpy(&dwMasking, m_stpSprite[iSpriteIndex].bypImge + j, m_iColorByte);

			// �ȼ��� �� �� ���ĸ� ��󳽴� (����ŷ)
			dwMasking = dwMasking & 0x00ffffff;

			// �ش� �ȼ��� �÷��� �Ͼ���� �ƴ� ���� �� ����ۿ� ��´�. �Ͼ���̸� ���� �ʰ� ���� �ȼ��� �̵��Ѵ�.
			// memcmp�� ������ 0��ȯ. �ٸ��� 1 ��ȯ. ��, �Ͼ���� �ƴϸ� 1�� ��ȯ�Ǿ� ���� �Ǿ� ���� ����.
			if (dwMasking != m_dwColorKey)
			{
				// ����, ���� ��°� �׸��ڶ�� ���ĺ��� ���δ�.
				if (iSpriteIndex == eSHADOW)
				{
					memcpy(&Temp, bypDest + j, m_iColorByte);

					BYTE r = ((dwMasking & 0xff0000) >> 16) + (((Temp & 0xff0000) >> 16) / 2);
					BYTE g = ((dwMasking & 0x00ff00) >> 8) + (((Temp & 0x00ff00) >> 8) / 2);
					BYTE b = (dwMasking & 0x0000ff) + ((Temp & 0x0000ff) / 2);

					dwMasking = (r << 16) | (g << 8) | (b << 0);		// ���� �ȼ� ������ ��� ������,  ARGB������� �����ؾ� �Ѵ�.
				}

				// ���� ��°� HP�� �ƴ϶�� �Ʒ� ����
				else if (iSpriteIndex != eGUAGE_HP)
				{
					// �� ĳ������ ���, ĳ���� ���������� ó��. �� ĳ���� �ƴϸ� �׳� ��´�.
					if (isPlayer)
					{
						BYTE r = ((dwMasking & 0xff0000) >> 16);
						BYTE g = ((dwMasking & 0x00ff00) >> 8) / 2;
						BYTE b = (dwMasking & 0x0000ff) / 2;

						dwMasking = (r << 16) | (g << 8) | (b << 0);		// ���� �ȼ� ������ ��� ������,  ARGB������� �����ؾ� �Ѵ�.
					}
				}

				// ���������� ��´�.
				memcpy(bypDest + j, &dwMasking, m_iColorByte);	// 1�ȼ�(4����Ʈ)�� ī���Ѵ�.

			}
		}
		bypDest += iDestPitch;
		m_stpSprite[iSpriteIndex].bypImge += iSpritePitch;
	}

	// �� ��������Ʈ ������ �ּҸ� ������Ű�鼭 ��������� ���� ��ġ�� �̵���Ų��.
	m_stpSprite[iSpriteIndex].bypImge = save;

	return TRUE;
}

// *******************************************
// Ư�� �޸� ��ġ�� ��������Ʈ�� ����Ѵ� (Ŭ���� ó����. ��� ������ ���)
// *******************************************
BOOL CSpritedib::DrawImage(int iSpriteIndex, int iDrawX, int iDrawY, BYTE* bypDest, int iDestWidth,
	int iDestHeight, int iDestPitch, int iDrawLen)
{
	// ����, ���޵� �ε����� �ִ� ��������Ʈ �������� ũ�ٸ� ���� ��������Ʈ�� ����϶�� �� ���̴�, false ��ȯ.
	if (m_iMaxSprite <= iSpriteIndex)
		return FALSE;

	BYTE* save = m_stpSprite[iSpriteIndex].bypImge;	// �پ� �� ������ ��������Ʈ �̹����� ���� �ּ�(0,0)

													// �ش� �Լ������� �����, �������� �����Ѵ�. Ŭ������ �ϱ� ������ ���� ������ �ʿ��ϴ�.
													//int iSpriteWidth = m_stpSprite[iSpriteIndex].iWidth;		<<�̰� �����Դµ� �ȽἭ �׳� �ּ�ó����. ���߿� �ʿ��ϸ� �ּ� ��������.
	int iSpriteHeight = m_stpSprite[iSpriteIndex].iHeight;
	int iSpritePitch = m_stpSprite[iSpriteIndex].iPitch;

	// ��������Ʈ �̹�����, ���� ��ġ�� ���Ѵ�. Ŭ���� ó���� �Ϸ���, �̹��� ��� ���� ��ġ�� �̵����Ѿ� �Ѵ�.
	int iSpriteStartX = 0;
	int iSpriteStartY = 0;

	// Ŭ���� ó�� -------------
	// ��
	if (iDrawY < 0)
	{
		int iAddY = iDrawY * -1;	// ���� �󸶳� �ö󰬴��� ����.
		iDrawY = 0;				// �ϴ�, Dest(�� �����)�� y��ġ�� 0�̴�.
		iSpriteStartY += iAddY;			// ��������Ʈ�� ������ y ��ġ�� �Ʒ��� iAddY��ŭ ������.
		iSpriteHeight -= iAddY;			// ��������Ʈ�� ���̸� iAddY��ŭ ���δ�. 
	}

	// ��
	if (iDrawY + m_stpSprite[iSpriteIndex].iHeight > iDestHeight)
	{
		int iAddY = iDrawY + m_stpSprite[iSpriteIndex].iHeight - iDestHeight;		// �Ʒ��� �󸶳� ���������� ����	
		iSpriteHeight -= iAddY;															// ��������Ʈ�� ���̸� iAddY��ŭ ���δ�. 		
	}

	// ��
	int iAddX = 0;
	if (iDrawX < 0)
	{
		iAddX = iDrawX * -1;	// �·� �󸶳� ������ ����.
		iDrawX = 0;				// �ϴ�, Dest(�� �����)�� x��ġ�� 0�̴�.
		iDrawX += iAddX;			// ��������Ʈ�� ������ x ��ġ�� �������� iAddX��ŭ �����δ�.
	}

	// ��
	if (iDrawX + m_stpSprite[iSpriteIndex].iWidth > iDestWidth)
		iAddX = iDrawX + m_stpSprite[iSpriteIndex].iWidth - iDestWidth;	// ��� �󸶳� ������ ����. ��� �̷��� ��!

																		// ���� ������ �̵� -------------
																		// ��������Ʈ�� ���� ������ �̵�. Dest�� �̹����� ���� �����ʹ� �̵���Ű�� �ʴ´�. ������ ȭ�� �»�ܺ��� ���;� �ϱ� ������!
																		// ���⼭ ���� ����Ʈ ������ ����Ѵ�.
	m_stpSprite[iSpriteIndex].bypImge += (iSpriteStartX * m_iColorByte) + (iSpriteStartY * iSpritePitch);


	// ��������Ʈ ���� -------------
	// ���������� 1�پ� �Ʒ��� �̵�
	//BYTE white = 0x00ff00;
	//for (int i = 0; i < iSpriteHeight; ++i)
	//{
	//	for (int j = 0; j < iSpritePitch - (iAddX*m_iColorByte); j += m_iColorByte)
	//	{
	//		// ��´�.
	//		memcpy(bypDest + j, &white, m_iColorByte);	// 1�ȼ�(4����Ʈ)�� ī���Ѵ�.
	//	}
	//	bypDest += iDestPitch;
	//}

	for (int i = 0; i < iSpriteHeight; ++i)
	{
		memcpy(bypDest, m_stpSprite[iSpriteIndex].bypImge, iSpritePitch - (iAddX*m_iColorByte));	// �÷�Ű ó���� ���ϴ�, 1�پ� �����Ѵ�.
		bypDest += iDestPitch;
		m_stpSprite[iSpriteIndex].bypImge += iSpritePitch;
	}


	// �� ��������Ʈ ������ �ּҸ� ������Ű�鼭 ��������� ���� ��ġ�� �̵���Ų��.
	m_stpSprite[iSpriteIndex].bypImge = save;

	return TRUE;

}