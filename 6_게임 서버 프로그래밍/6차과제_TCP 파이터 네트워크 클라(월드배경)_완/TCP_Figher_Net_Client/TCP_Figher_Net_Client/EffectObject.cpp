#include "stdafx.h"
#include "EffectObject.h"
#include "PlayerObject.h"

#include "List_Template.h"

extern CList<BaseObject*> list;					// Main���� �Ҵ��� ��ü ���� ����Ʈ	

#define PLAYER_X	71
#define PLAYER_Y	90

// ������
EffectObject::EffectObject(int iObjectID, int iObjectType, int iCurX, int iCurY, int AtkType)
	:BaseObject(iObjectID, iObjectType, iCurX, iCurY)
{
	m_EffectSKipCheck = true;
	m_bEffectStart = FALSE;
	m_dwAttackID = AtkType;

	// ���� Ÿ�Կ� ���� x,y ����
	if (m_dwAttackID == dfACTION_ATTACK_01_LEFT || m_dwAttackID == dfACTION_ATTACK_02_LEFT || m_dwAttackID == dfACTION_ATTACK_03_LEFT)
		MoveCurXY(iCurX - 80, iCurY - 60);

	else if (m_dwAttackID == dfACTION_ATTACK_01_RIGHT || m_dwAttackID == dfACTION_ATTACK_02_RIGHT || m_dwAttackID == dfACTION_ATTACK_03_RIGHT)
		MoveCurXY(iCurX + 80, iCurY - 60);

	// ���� Ÿ�Կ� ����, ����ϴ� ������ �� ����. �ش� ������ �� ��ŭ ����� �Ŀ� ����Ʈ ����� ���۵ȴ�.
	if (m_dwAttackID == dfACTION_ATTACK_03_LEFT || m_dwAttackID == dfACTION_ATTACK_03_RIGHT)
		m_StandByFrame = 10;

	else if (m_dwAttackID == dfACTION_ATTACK_01_LEFT || m_dwAttackID == dfACTION_ATTACK_01_RIGHT)
		m_StandByFrame = 0;

	else if (m_dwAttackID == dfACTION_ATTACK_02_LEFT || m_dwAttackID == dfACTION_ATTACK_02_RIGHT)
		m_StandByFrame = 4;
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

		// ����Ʈ ���� ������, ���� ǥ�õǾ�� �ϴ� ��ġ�� ���� �ִٸ�, Draw��´�. false�� ��ŵ�� ���Ѵٴ� ���̴�.
		CList<BaseObject*>::iterator itor;
		for (itor = list.begin(); itor != list.end(); itor++)
		{
			// �ش� ��ü�� �÷��̾���
			if ((*itor)->GetObjectType() == 1)
			{
				// ���⿡�� ��Ʈ�� �»�ܰ� �����Ǵ� ���� ��ǥ�� ����ȴ�.
				int WorldX = (*itor)->GetCurX() - PLAYER_X;
				int WorldY = (*itor)->GetCurY() - PLAYER_Y;

				//  �ش� �÷��̾��� ���� ��ǥ(�����̴�)�� ����Ʈ�� ǥ�õ� ��ġ�� ��ġ���� üũ
				//	��ġ�� �÷��̾ �߰ߵǸ�, ��ŵ ���θ� false�� �Ѵ�. 
				if (WorldX < m_iCurX && m_iCurX < WorldX + (PLAYER_X * 2) &&
					WorldY < m_iCurY && m_iCurY < WorldY + PLAYER_Y)
				{
					m_EffectSKipCheck = false;
					break;
				}					
			}
		}		
	}

	// m_StandByFrame�� 0�� �ƴϰ�, ����Ʈ ��� �÷��װ� FALSE��, ���� ����Ʈ ����� ���� �ȵȰ��̴� m_StandByFrame�� 1 ����
	else if (m_StandByFrame != 0 && m_bEffectStart == FALSE)
		m_StandByFrame--;
}

// Draw
void EffectObject::Draw(BYTE* bypDest, int iDestWidth, int iDestHeight, int iDestPitch)
{
	// ���� ����Ʈ�� �׸���.
	if (m_bEffectStart == TRUE && m_EffectSKipCheck == false)
	{
		// ī�޶� ��ǥ ����, ���� ��µ� ��ġ ���Ѵ�.
		CMap* SIngletonMap = CMap::Getinstance(0, 0, 0, 0, 0);
		int ShowX = SIngletonMap->GetShowPosX(m_iCurX);
		int ShowY = SIngletonMap->GetShowPosY(m_iCurY);

		/*
		if (ShowX < 0)
			ShowX = 0;

		if (ShowY < 0)
			ShowY = 0;
		*/

		int iCheck = g_cSpriteDib->DrawSprite(m_iSpriteNow, ShowX, ShowY, bypDest, iDestWidth, iDestHeight, iDestPitch, 0);
		if (!iCheck)
			exit(-1);
	}
}