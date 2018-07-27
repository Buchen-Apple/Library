#pragma once
#ifndef __SPRITEDIB_H__
#define __SPRITEDIB_H__

#include <Windows.h>


class CSpritedib
{
public:
	enum e_SPRITE
	{
		eMAP = 0,

		ePLAYER_STAND_L01,
		ePLAYER_STAND_L02,
		ePLAYER_STAND_L03,
		ePLAYER_STAND_L04,
		ePLAYER_STAND_L05,

		ePLAYER_STAND_R01,
		ePLAYER_STAND_R02,
		ePLAYER_STAND_R03,
		ePLAYER_STAND_R04,
		ePLAYER_STAND_R05,

		ePLAYER_MOVE_L01,
		ePLAYER_MOVE_L02,
		ePLAYER_MOVE_L03,
		ePLAYER_MOVE_L04,
		ePLAYER_MOVE_L05,
		ePLAYER_MOVE_L06,
		ePLAYER_MOVE_L07,
		ePLAYER_MOVE_L08,
		ePLAYER_MOVE_L09,
		ePLAYER_MOVE_L10,
		ePLAYER_MOVE_L11,
		ePLAYER_MOVE_L12,

		ePLAYER_MOVE_R01,
		ePLAYER_MOVE_R02,
		ePLAYER_MOVE_R03,
		ePLAYER_MOVE_R04,
		ePLAYER_MOVE_R05,
		ePLAYER_MOVE_R06,
		ePLAYER_MOVE_R07,
		ePLAYER_MOVE_R08,
		ePLAYER_MOVE_R09,
		ePLAYER_MOVE_R10,
		ePLAYER_MOVE_R11,
		ePLAYER_MOVE_R12,

		ePLAYER_ATTACK1_L01,
		ePLAYER_ATTACK1_L02,
		ePLAYER_ATTACK1_L03,
		ePLAYER_ATTACK1_L04,

		ePLAYER_ATTACK1_R01,
		ePLAYER_ATTACK1_R02,
		ePLAYER_ATTACK1_R03,
		ePLAYER_ATTACK1_R04,

		ePLAYER_ATTACK2_L01,
		ePLAYER_ATTACK2_L02,
		ePLAYER_ATTACK2_L03,
		ePLAYER_ATTACK2_L04,

		ePLAYER_ATTACK2_R01,
		ePLAYER_ATTACK2_R02,
		ePLAYER_ATTACK2_R03,
		ePLAYER_ATTACK2_R04,

		ePLAYER_ATTACK3_L01,
		ePLAYER_ATTACK3_L02,
		ePLAYER_ATTACK3_L03,
		ePLAYER_ATTACK3_L04,
		ePLAYER_ATTACK3_L05,
		ePLAYER_ATTACK3_L06,

		ePLAYER_ATTACK3_R01,
		ePLAYER_ATTACK3_R02,
		ePLAYER_ATTACK3_R03,
		ePLAYER_ATTACK3_R04,
		ePLAYER_ATTACK3_R05,
		ePLAYER_ATTACK3_R06,

		eEFFECT_SPARK_01,
		eEFFECT_SPARK_02,
		eEFFECT_SPARK_03,
		eEFFECT_SPARK_04,

		eGUAGE_HP,
		eSHADOW,
		e_SPRITE_MAX,
	};

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


public:
	//*******************************************
	// ���� �ʹ� ���� �Լ�(����� ��� ��Ʈ���� �о�´�
	//*******************************************
	void GameInit();

	//*******************************************
	// �̱��� ���� �Լ�
	//*******************************************
	static CSpritedib* Getinstance(int iMaxSprite, DWORD dwColorKey, int iColorBit);

	//*******************************************
	// ��������Ʈ�� �� ���������� ó��
	//*******************************************
	void RedChange(int iSpriteIndex);

	// *******************************************
	// �ı���
	// *******************************************
	~CSpritedib();

	// *******************************************
	// BMP������ �о �ϳ��� ��������Ʈ�� ����
	// *******************************************
	BOOL LoadDibSprite(int iSpriteIndex, char* szFileName, int iCenterPointX, int iCenterPointY);

	// *******************************************
	// Ư�� �޸� ��ġ�� ��������Ʈ�� ����Ѵ� (Į��Ű, Ŭ���� ó��)
	// *******************************************
	BOOL DrawSprite(int iSpriteIndex, int iDrawX, int iDrawY, BYTE* bypDest, int iDestWidth,
		int iDestHeight, int iDestPitch, BOOL isPlayer, int iDrawLen = 100);

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

#pragma once
