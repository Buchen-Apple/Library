#pragma once
#ifndef __PLAYER_OBJECT_H__
#define __PLAYER_OBJECT_H__

#include "BaseObject.h"

#define PLAYER_XMOVE_PIXEL 3
#define PLAYER_YMOVE_PIXEL 2

#define dfACTION_MOVE_LL	0
#define dfACTION_MOVE_LU	1
#define dfACTION_MOVE_UU	2
#define dfACTION_MOVE_RU	3
#define dfACTION_MOVE_RR	4
#define dfACTION_MOVE_RD	5
#define dfACTION_MOVE_DD	6
#define dfACTION_MOVE_LD	7
#define dfACTION_ATTACK_01	10
#define dfACTION_ATTACK_02	11
#define dfACTION_ATTACK_03	12
#define dfACTION_IDLE		100

#define dfDIRECTION_LEFT	1
#define dfDIRECTION_RIGHT	2

class PlayerObject	:public BaseObject
{
private:
	BOOL m_bPlayerCharacter;	// �� ĳ�������� üũ. true�� �� ĳ���� / false�� �ٸ���� ĳ����
	int m_iHP;					// �� HP
	DWORD m_dwActionCur;		// ���� �׼��� �������� ����.
	DWORD m_dwActionOld;		// ������ �ٷ� ���� �׼��� �������� ����. ���� �׼� / ���� �׼��� ���� �ٸ��ٸ� �޽����� ������.
	int m_iDirCur;				// ���� �ٶ󺸴� ���� (1 : �� / 2 : ��)
	int m_iDirOld;				// ������ �ٶ󺸴� ����		

public:
	PlayerObject(int iObjectID, int iObjectType, int iCurX = 50, int iCurY = 300);

	virtual ~PlayerObject();
	virtual void Action();
	virtual void Draw(BYTE* bypDest, int iDestWidth, int iDestHeight, int iDestPitch);

	void MemberSetFunc(BOOL bPlayerChar, int iHP);
	void ActionProc();
	BOOL isPlayer();
	int GetHP();

};

#endif // !__PLAYER_OBJECT_H__
