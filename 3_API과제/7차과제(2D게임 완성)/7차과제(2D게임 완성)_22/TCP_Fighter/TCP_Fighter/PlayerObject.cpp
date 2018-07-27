#include "stdafx.h"
#include "PlayerObject.h"

//---------------------------
// ������
// --------------------------
PlayerObject::PlayerObject(int iObjectID, int iObjectType, int iCurX, int iCurY)
	:BaseObject(iObjectID, iObjectType, iCurX, iCurY)
{	
	m_dwActionCur =  100;
	m_dwActionOld = 100;
	m_iDirCur = dfDIRECTION_RIGHT;
	m_iDirOld = 0;
}

//---------------------------
// �Ҹ���
// --------------------------
PlayerObject::~PlayerObject()
{
	// ���� �Ұ� ����..
}

//---------------------------
// ���� ��ǥ �̵�
// --------------------------
void PlayerObject::Action()
{
	m_dwActionCur = m_dwActionInput;

	// �ִϸ��̼� ���� ���������� �̵� ----------------
	NextFrame();

	// ��ǥ �̵�, �ִϸ��̼� ó�� ---------------------
	ActionProc();
}	

//---------------------------
// ��ǥ�̵��� �ִϸ��̼� ó��
// --------------------------
void PlayerObject::ActionProc()
{
	static bool bAttackState = FALSE;	// ���� ������ üũ�ϴ� ����. FLASE�� ���� �� �ƴ� / TRUE�� ���� ��
	static bool bIdleState = FALSE;		// ��� �ڼ����� üũ�ϴ� ����. FALSE�� ����� �ƴ� / TRUE�� �����.

	// �� ĳ���� �� ���� ���� ó��
	if (m_bPlayerCharacter == TRUE)
	{
		// ���� ���̰�, ������ �������� �ƴ϶�� �޽����� ������ ���ݸ޽����� ��ȯ. ������ ���� �� ���̴�.
		if (bAttackState == TRUE && Get_bGetEndFrame() == FALSE)
			m_dwActionCur = dfACTION_ATTACK_01;

		// �װ� �ƴ϶�� �޽��� ��ȯ ���ϰ� ���ݸ�� (bAttackState)�� FALSE�� �����.
		else
			bAttackState = FALSE;

		// ���� �޽��� ó�� ---------------------------------------
		switch (m_dwActionCur)
		{
		case dfACTION_ATTACK_01:
		case dfACTION_ATTACK_02:
		case dfACTION_ATTACK_03:
			if (bAttackState == FALSE)
			{
				bAttackState = TRUE;
				bIdleState = FALSE;
				if (m_dwActionCur == dfACTION_ATTACK_01)
				{
					if (m_iDirCur == dfDIRECTION_LEFT)
						SetSprite(CSpritedib::ePLAYER_ATTACK1_L01, 4, 3);
					else if (m_iDirCur == dfDIRECTION_RIGHT)
						SetSprite(CSpritedib::ePLAYER_ATTACK1_R01, 4, 3);
				}

				else if (m_dwActionCur == dfACTION_ATTACK_02)
				{
					if (m_iDirCur == dfDIRECTION_LEFT)
						SetSprite(CSpritedib::ePLAYER_ATTACK2_L01, 4, 4);
					else if (m_iDirCur == dfDIRECTION_RIGHT)
						SetSprite(CSpritedib::ePLAYER_ATTACK2_R01, 4, 4);
				}

				else if (m_dwActionCur == dfACTION_ATTACK_03)
				{
					if (m_iDirCur == dfDIRECTION_LEFT)
						SetSprite(CSpritedib::ePLAYER_ATTACK3_L01, 6, 4);
					else if (m_iDirCur == dfDIRECTION_RIGHT)
						SetSprite(CSpritedib::ePLAYER_ATTACK3_R01, 6, 4);
				}
			}

			break;
		}


		// IDLE �޽����� �̵� �޽��� ó�� ---------------------------------------
		switch (m_dwActionCur)
		{
		case dfACTION_IDLE:
			// ���� ���� �ƴϰ�, ��� �ڼ��� �ƴϸ� ������ ����.
			if (bAttackState == FALSE && bIdleState == FALSE)
			{
				if (m_iDirCur == dfDIRECTION_LEFT)
					SetSprite(CSpritedib::ePLAYER_STAND_L01, 5, 5);
				else if (m_iDirCur == dfDIRECTION_RIGHT)
					SetSprite(CSpritedib::ePLAYER_STAND_R01, 5, 5);
				bIdleState = TRUE;	// �����¸� TRUE�� �����, ���� ������ ����ڼ� �޽����� �� ���͵� �ִϸ��̼��� ó������ ��� ����.
			}
			break;

		default:
			bIdleState = FALSE;
			if (bAttackState == FALSE)
			{
				// Ű �Է¿� ���� ��ǥ �̵� --------------------
				if (m_dwActionCur == dfACTION_MOVE_LL)			// LL
				{
					m_iCurX -= PLAYER_XMOVE_PIXEL;
					m_iDirCur = dfDIRECTION_LEFT;
				}

				else if (m_dwActionCur == dfACTION_MOVE_LU)		// LU
				{
					m_iCurX -= PLAYER_XMOVE_PIXEL;
					m_iCurY -= PLAYER_YMOVE_PIXEL;
					m_iDirCur = dfDIRECTION_LEFT;
				}

				else if (m_dwActionCur == dfACTION_MOVE_UU)		// UU
				{
					m_iCurY -= PLAYER_YMOVE_PIXEL;
				}

				else if (m_dwActionCur == dfACTION_MOVE_RU)		// RU
				{
					m_iCurX += PLAYER_XMOVE_PIXEL;
					m_iCurY -= PLAYER_YMOVE_PIXEL;
					m_iDirCur = dfDIRECTION_RIGHT;
				}

				else if (m_dwActionCur == dfACTION_MOVE_RR)		// RR
				{
					m_iCurX += PLAYER_XMOVE_PIXEL;
					m_iDirCur = dfDIRECTION_RIGHT;
				}

				else if (m_dwActionCur == dfACTION_MOVE_RD)		// RD
				{
					m_iCurX += PLAYER_XMOVE_PIXEL;
					m_iCurY += PLAYER_YMOVE_PIXEL;
					m_iDirCur = dfDIRECTION_RIGHT;
				}

				else if (m_dwActionCur == dfACTION_MOVE_DD)		// DD
				{
					m_iCurY += PLAYER_YMOVE_PIXEL;
				}

				else if (m_dwActionCur == dfACTION_MOVE_LD)		// LD
				{
					m_iCurX -= PLAYER_XMOVE_PIXEL;
					m_iCurY += PLAYER_YMOVE_PIXEL;
					m_iDirCur = dfDIRECTION_LEFT;
				}

				// ���� ���¿� ���� ���°� �ٸ��ٸ�
				if (m_dwActionCur != m_dwActionOld)
				{
					// ���� �ִϸ��̼� ��ü
					if (m_iDirCur != m_iDirOld ||	// ���� ����� ���� ������ �ٸ��ų�
						m_dwActionOld == dfACTION_IDLE ||	// ���� ���°� ��⿴�ų�
						m_dwActionOld == dfACTION_ATTACK_01 || m_dwActionOld == dfACTION_ATTACK_02 || m_dwActionOld == dfACTION_ATTACK_03) // ���� ���°� ����1, ����2, ����3 �� �ϳ����ٸ�
					{
						if (m_iDirCur == dfDIRECTION_LEFT)
							SetSprite(CSpritedib::ePLAYER_MOVE_L01, 12, 4);

						else if (m_iDirCur == dfDIRECTION_RIGHT)
							SetSprite(CSpritedib::ePLAYER_MOVE_R01, 12, 4);
					}
				}
			}
			break;
		}

		m_iDirOld = m_iDirCur;			// ���� ��ü
		m_dwActionOld = m_dwActionCur;	// ���� ��ü
	}
}

//---------------------------
// ������ �� DC�� �����ϴ� �Լ�
//--------------------------
void PlayerObject::Draw(BYTE* bypDest, int iDestWidth, int iDestHeight, int iDestPitch)
{
	// �� �׸��� 
	BOOL iCheck = g_cSpriteDib->DrawSprite(CSpritedib::eSHADOW, m_iCurX, m_iCurY, bypDest, iDestWidth, iDestHeight, iDestPitch, isPlayer());
	if (!iCheck)
		exit(-1);

	// �� ĳ����
	iCheck = g_cSpriteDib->DrawSprite(m_iSpriteNow, m_iCurX, m_iCurY, bypDest, iDestWidth, iDestHeight, iDestPitch, isPlayer());
	if (!iCheck)
		exit(-1);

	// �� HP ������
	iCheck = g_cSpriteDib->DrawSprite(CSpritedib::eGUAGE_HP, m_iCurX - 35, m_iCurY + 9, bypDest, iDestWidth, iDestHeight, iDestPitch, isPlayer());
	if (!iCheck)
		exit(-1);
}

//---------------------------
// ���� ������� ����
//--------------------------
void PlayerObject::MemberSetFunc(BOOL bPlayerChar, int iHP)
{
	m_bPlayerCharacter = bPlayerChar;
	m_iHP = iHP;
}

//---------------------------
// m_iHP ����
//--------------------------
int PlayerObject::GetHP()
{
	return m_iHP;
}

//---------------------------
// m_bPlayerCharacter ����
//--------------------------
BOOL PlayerObject::isPlayer()
{
	return m_bPlayerCharacter;
}
