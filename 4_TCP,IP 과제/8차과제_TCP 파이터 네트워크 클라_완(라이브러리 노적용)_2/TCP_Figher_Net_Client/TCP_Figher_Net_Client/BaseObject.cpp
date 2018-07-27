#include "stdafx.h"
#include "BaseObject.h"

//---------------------------
// ������
// --------------------------
BaseObject::BaseObject(int iObjectID, int iObjectType, int iCurX, int iCurY)
{
	m_bEndFrame = false;
	m_dwActionInput = 0;
	m_iCurX = iCurX;
	m_iCurY = iCurY;
	m_iDelayCount = 0;
	m_iFrameDelay = 0;
	m_iObjectID = iObjectID;
	m_iObjectType = iObjectType;
	m_iSpriteStart = 0;
	m_iSpriteNow = 0;
	m_iSpriteEnd = 0;
	g_cSpriteDib = CSpritedib::Getinstance(31, 0xffffffff, 32);	// �̱������� CSpritedib�� ��ü �ϳ� ���
}

//---------------------------
// �Ҹ���
// --------------------------
BaseObject::~BaseObject()
{
	// ���� �Ұ� ����
}

//---------------------------
// ��������Ʈ �ִϸ��̼� ����
// --------------------------
void BaseObject::SetSprite(int iStartSprite, int iSpriteCount, int iFrameDelay)
{
	m_iSpriteStart = iStartSprite;
	m_iSpriteEnd = iStartSprite + (iSpriteCount - 1);
	m_iFrameDelay = iFrameDelay;

	m_iSpriteNow = iStartSprite;
	m_iDelayCount = 0;
	m_bEndFrame = FALSE;
}


//---------------------------
// �ܺηκ����� Ű�Է� üũ
// --------------------------
void BaseObject::ActionInput(int MoveXY)
{
	m_dwActionInput = MoveXY;
}

//---------------------------
// ������ �������� �ѱ�� �Լ�
// --------------------------
void BaseObject::NextFrame()
{
	if (0 > m_iSpriteStart)		// 0���� ���� ���� ���۰��̸� ����.
		return;

	// ������ ������ ���� �Ѿ�� ���� ���������� �Ѿ��.
	m_iDelayCount++;

	if (m_iDelayCount >= m_iFrameDelay)
	{
		m_iDelayCount = 0;
		m_iSpriteNow++;

		if (m_iSpriteNow > m_iSpriteEnd)
		{
			m_iSpriteNow = m_iSpriteStart;
			m_bEndFrame = TRUE;
		}
	}
}

//---------------------------
// X,Y ���� �Լ� (��Ʈ��ũ��)
// --------------------------
void BaseObject::MoveCurXY(int X, int Y)
{
	m_iCurX = X;
	m_iCurY = Y;
}

//---------------------------
// ���� ���� �Լ�
// --------------------------
int BaseObject::GetCurX()
{
	return m_iCurX;
}

int BaseObject::GetCurY()
{
	return m_iCurY;
}

int BaseObject::GetObjectID()
{
	return m_iObjectID;
}

int BaseObject::GetSprite()
{
	return m_iSpriteNow;
}

int BaseObject::GetObjectType()
{
	return m_iObjectType;
}

bool BaseObject::Get_bGetEndFrame()
{
	return m_bEndFrame;
}

