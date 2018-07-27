#include "stdafx.h"
#include "EffectObject.h"
#include "PlayerObject.h"

// ������
EffectObject::EffectObject(int iObjectID, int iObjectType, int iCurX, int iCurY, int AtkType)
	:BaseObject(iObjectID, iObjectType, iCurX, iCurY)
{
	m_bEffectStart = FALSE;
	m_dwAttackID = AtkType;

	// ���� Ÿ�Կ� ���� x,y ����
	if (m_dwAttackID == dfACTION_ATTACK_01_LEFT || m_dwAttackID == dfACTION_ATTACK_02_LEFT || m_dwAttackID == dfACTION_ATTACK_03_LEFT)
		MoveCurXY(iCurX - 60, iCurY - 60);

	else if (m_dwAttackID == dfACTION_ATTACK_01_RIGHT || m_dwAttackID == dfACTION_ATTACK_02_RIGHT || m_dwAttackID == dfACTION_ATTACK_03_RIGHT)
		MoveCurXY(iCurX + 60, iCurY - 60);

	// ���� Ÿ�Կ� ����, ����ϴ� ������ �� ����. �ش� ������ �� ��ŭ ����� �Ŀ� ����Ʈ ����� ���۵ȴ�.
	if (m_dwAttackID == dfACTION_ATTACK_03_LEFT || m_dwAttackID == dfACTION_ATTACK_03_RIGHT)
		m_StandByFrame = 10;

	else if (m_dwAttackID == dfACTION_ATTACK_01_LEFT || m_dwAttackID == dfACTION_ATTACK_01_RIGHT || m_dwAttackID == dfACTION_ATTACK_02_LEFT || m_dwAttackID == dfACTION_ATTACK_02_RIGHT)
		m_StandByFrame = 0;
}

// �Ҹ���
EffectObject::~EffectObject()
{
	// �Ұ� ����
}

// �׼�
void EffectObject::Action()
{	
	if (m_bEffectStart == TRUE)
	{
		// �ִϸ��̼� ���� ���������� �̵� ----------------
		NextFrame();
	}

	// m_StandByFrame�� 0�̰�(��� ������ ��), ����Ʈ ��� �÷��װ� FALSE��, ����Ʈ ��� ���� �÷��׸� TRUE�� �����.
	if (m_StandByFrame == 0 && m_bEffectStart == FALSE)
	{
		m_bEffectStart = TRUE;		
	}

	// m_StandByFrame�� 0�� �ƴϰ�, ����Ʈ ��� �÷��װ� FALSE��, ���� ����Ʈ ����� ���� �ȵȰ��̴� m_StandByFrame�� 1 ����
	else if(m_StandByFrame != 0 && m_bEffectStart == FALSE)
		m_StandByFrame--;
}

// Draw
void EffectObject::Draw(BYTE* bypDest, int iDestWidth, int iDestHeight, int iDestPitch)
{
	// ���� ����Ʈ�� �׸���.
	if (m_bEffectStart == TRUE)
	{		
		int iCheck = g_cSpriteDib->DrawSprite(m_iSpriteNow, m_iCurX, m_iCurY, bypDest, iDestWidth, iDestHeight, iDestPitch, 0);
		if (!iCheck)
			exit(-1);
	}
}