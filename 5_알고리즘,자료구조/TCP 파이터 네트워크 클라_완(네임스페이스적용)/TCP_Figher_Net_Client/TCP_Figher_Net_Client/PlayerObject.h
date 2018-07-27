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
#define dfACTION_ATTACK_01_LEFT		20
#define dfACTION_ATTACK_02_LEFT		21
#define dfACTION_ATTACK_03_LEFT		22
#define dfACTION_ATTACK_01_RIGHT	23
#define dfACTION_ATTACK_02_RIGHT	24
#define dfACTION_ATTACK_03_RIGHT	25
#define dfACTION_IDLE		100

#define dfDIRECTION_LEFT	dfACTION_MOVE_LL
#define dfDIRECTION_RIGHT	dfACTION_MOVE_RR

//-----------------------------------------------------------------
// ȭ�� �̵� ����.
//-----------------------------------------------------------------
#define dfRANGE_MOVE_TOP	50 + 1
#define dfRANGE_MOVE_LEFT	10 + 2
#define dfRANGE_MOVE_RIGHT	630 - 3
#define dfRANGE_MOVE_BOTTOM	470 - 1


class PlayerObject :public BaseObject
{
private:
	BOOL m_bPlayerCharacter;	// �� ĳ�������� üũ. true�� �� ĳ���� / false�� �ٸ���� ĳ����
	BOOL m_bPacketProcCheck;	// �� ĳ���Ͱ� �ƴ� ���, ���� ���ο� ���� ��Ŷ�� �Դ��� üũ. �̰� TRUE�� ���� ������ ������ ó���Ѵ�.
	int m_iHP;					// �� HP
	DWORD m_dwActionCur;		// ���� �׼��� �������� ����.
	DWORD m_dwActionOld;		// ������ �ٷ� ���� �׼��� �������� ����. ���� �׼� / ���� �׼��� ���� �ٸ��ٸ� �޽����� ������.
	int m_iDirCur;				// ���� �ٶ󺸴� ���� (0 : �� / 4 : ��)
	int m_iDirOld;				// ������ �ٶ󺸴� ����		
	bool m_bAttackState;		// ���� ������ üũ�ϴ� ����. FLASE�� ���� �� �ƴ� / TRUE�� ���� ��
	bool m_bIdleState;			// ��� �ڼ����� üũ�ϴ� ����. FALSE�� ����� �ƴ� / TRUE�� �����.
	DWORD m_dwTargetID;			// ���� �������� ���� ����� ID
	DWORD m_dwNowAtkType;		// ���� �������� Ÿ��. 0�̸� ������ �ƴ�.

public:
	PlayerObject(int iObjectID, int iObjectType, int iCurX = 50, int iCurY = 300, int iDirCur = dfDIRECTION_RIGHT);

	virtual ~PlayerObject();
	virtual void Action();
	virtual void Draw(BYTE* bypDest, int iDestWidth, int iDestHeight, int iDestPitch);

	void NonMyAtackCheck(BOOL Check);
	void DirChange(int Dir);
	void MemberSetFunc(BOOL bPlayerChar, int iHP);
	void DamageFunc(int iHP, DWORD TargetID);
	void ActionProc();
	BOOL isPlayer();
	int GetHP();
	int GetCurDir();
	DWORD GetNowAtkType();
	void ChangeNowAtkType(DWORD dwAtktype);

};

#endif // !__PLAYER_OBJECT_H__
