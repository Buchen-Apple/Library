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
	BOOL iCheck = LoadDibSprite(0, "..\\SpriteData\\_Map.bmp", 0, 0);
	if (!iCheck)
		exit(-1);

	iCheck = LoadDibSprite(1, "..\\SpriteData\\Attack1_L_01.bmp", 71, 90);
	if (!iCheck)
		exit(-1);

	iCheck = LoadDibSprite(2, "..\\SpriteData\\Attack1_L_02.bmp", 71, 90);
	if (!iCheck)
		exit(-1);

	iCheck = LoadDibSprite(3, "..\\SpriteData\\Attack1_L_03.bmp", 71, 90);
	if (!iCheck)
		exit(-1);

	iCheck = LoadDibSprite(4, "..\\SpriteData\\Attack1_L_04.bmp", 71, 90);
	if (!iCheck)
		exit(-1);

}

//*******************************************
// ���ڷ� ���� �÷��� ���� �迭�� ����.
// �÷��̾� ĳ������ �� ���� �� ��� (������ ����)
//*******************************************
DWORD CSpritedib::PlayerColorRed(BYTE* SorRGB)
{
	DWORD TempRGB;

	memcpy(&TempRGB, SorRGB, m_iColorByte);

	BYTE r = GetRValue(TempRGB);
	BYTE g = GetGValue(TempRGB);
	BYTE b = GetBValue(TempRGB);

	g = g / 2;
	b = b / 2;

	TempRGB = RGB(b, g, r);	// ���� 4����Ʈ�� ����ϱ� ������, ARGB ������� �Ǿ��ִ� �����Ͱ� �ʿ��ϴ�.

	return TempRGB;
}

// *******************************************
// BMP������ �о �ϳ��� ��������Ʈ�� ����.
// *******************************************
BOOL CSpritedib::LoadDibSprite(int iSpriteIndex, char* szFileName, int iCenterPointX, int iCenterPointY)
{
	// ����, ���޵� �ε����� �ִ� ��������Ʈ �������� ũ�ٸ� false ��ȯ.
	if (m_iMaxSprite <= iSpriteIndex)
		return FALSE;

	DWORD				dwRead;
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
	for (int i = 0; i < iSpriteHeight; ++i)
	{
		// �� �پ� �Ʒ��� �̵��ϸ鼭 �ȼ������� >> �̵��ϸ� �÷��� �Ͼ������ üũ�Ѵ�.
		for (int j = 0; j < iSpritePitch - (iAddX*m_iColorByte); j += m_iColorByte)
		{
			// �ش� �ȼ��� �÷��� �Ͼ���� �ƴ� ���� �� ����ۿ� ��´�. �Ͼ���̸� ���� �ʰ� ���� �ȼ��� �̵��Ѵ�.
			// memcmp�� ������ 0��ȯ. �ٸ��� 1 ��ȯ. ��, �Ͼ���� �ƴϸ� 1�� ��ȯ�Ǿ� ���� �Ǿ� ���� ����.
			if (memcmp(m_stpSprite[iSpriteIndex].bypImge + j, &m_dwColorKey, m_iColorByte))
			{		
				DWORD rgb = PlayerColorRed(m_stpSprite[iSpriteIndex].bypImge + j);	// ���� ��ġ�� �÷��� ���������� �����Ѵ� (�÷��̾� ����)			
				memcpy(bypDest + j, &rgb, m_iColorByte);	// 1�ȼ�(4����Ʈ)�� ī���Ѵ�.
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