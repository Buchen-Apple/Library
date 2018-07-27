#pragma once
#ifndef __SPRITEDIB_H__
#define __SPRITEDIB_H__

#include <Windows.h>

class CSpritedib
{

private:
	// *******************************************
	// �̱����� ���� �����ڴ� Private�� ����
	// *******************************************
	CSpritedib(int iMaxSprite, DWORD dwColorKey, int iColorBit);

	// *******************************************
	// ���޵� ��������Ʈ�� �����Ҵ� ���� (�ı��ڿ��� ȣ��)
	// *******************************************
	void ReleseSprite(int iSpriteIndex);

	// *******************************************
	// DIB ��������Ʈ ����ü
	//
	// ��������Ʈ �̹����� ������ ������ ���´�.
	// *******************************************
	struct st_SPRITE
	{
		BYTE* bypImge;			// ��������Ʈ �̹���
		int iWidth;				// ����
		int iHeight;			// ����
		int iPitch;				// ��ġ(���� ����Ʈ ����)

		int iCenterPointX;		// ���� X
		int iCenterPointY;		// ���� Y
	};

	// *******************************************
	// �� ĳ���ʹ� ���������� ó��
	// *******************************************
	DWORD PlayerColorRed(BYTE* SorRGB);


public:
	//*******************************************
	// ���� �ʹ� ���� �Լ�(����� ��� ��Ʈ���� �о�´�
	//*******************************************
	void GameInit();
	
	//*******************************************
	// �̱��� ���� �Լ�
	//*******************************************
	static CSpritedib* Getinstance(int iMaxSprite, DWORD dwColorKey, int iColorBit);
		
	// *******************************************
	// �ı���
	// *******************************************
	virtual ~CSpritedib();

	// *******************************************
	// BMP������ �о �ϳ��� ��������Ʈ�� ����
	// *******************************************
	BOOL LoadDibSprite(int iSpriteIndex, char* szFileName, int iCenterPointX, int iCenterPointY);
	
	// *******************************************
	// Ư�� �޸� ��ġ�� ��������Ʈ�� ����Ѵ� (Į��Ű, Ŭ���� ó��)
	// *******************************************
	BOOL DrawSprite(int iSpriteIndex, int iDrawX, int iDrawY, BYTE* bypDest, int iDestWidth,
					int iDestHeight, int iDestPitch, int iDrawLen = 100);

	// *******************************************
	// Ư�� �޸� ��ġ�� ��������Ʈ�� ����Ѵ� (Ŭ���� ó����. ��� ������ ���)
	// *******************************************
	BOOL DrawImage(int iSpriteIndex, int iDrawX, int iDrawY, BYTE* bypDest, int iDestWidth,
					int iDestHeight, int iDestPitch, int iDrawLen = 100);


protected: 
	// ��� ������

	// *******************************************
	// �ִ� ��������Ʈ ��, Sprite �迭 ����, ��������Ʈ �÷�
	// *******************************************
	int			m_iMaxSprite;
	st_SPRITE	*m_stpSprite;
	int			m_iColorByte;

	// *******************************************
	// ���� �������� ����� �÷�
	// *******************************************
	DWORD m_dwColorKey;

};


#endif // !__SPRITEDIB_H__

