#pragma once
#ifndef __BASE_OBEJCT_H__
#define __BASE_OBEJCT_H__

#include <Windows.h>
#include "SpriteDib.h"


class BaseObject
{
private:
	bool m_bEndFrame;			// �ִϸ��̼� �÷��� ���� ����. true�� ����, false�� ���� ��
	int m_iDelayCount;			// 
	int m_iFrameDelay;			// �������� ������. �ִϸ��̼� ���� �����̰� ������ �ִ�.
	int m_iObjectID;			// ���� ���� ID
	int m_iObjectType;			// ������Ʈ�� Ÿ��. �÷��̾�/����Ʈ ���� (1 : �÷��̾� 2 : ����Ʈ)

protected:
	DWORD m_dwActionInput;		// ť ����̴�. Ű ������ �� Ű�� ���� ����ȴ�.
	CSpritedib* g_cSpriteDib;	// ��������Ʈ�� ������ ��ü.
	int m_iCurX;				// ���� X��ǥ ��ġ
	int m_iCurY;				// ���� Y��ǥ ��ġ
	int m_iSpriteStart;			// ����� ������ ��������Ʈ�� ��ȣ
	int m_iSpriteNow;			// ���� ���� ���� ��������Ʈ ��ȣ
	int m_iSpriteEnd;			// ��� �Ϸ�� ��������Ʈ ��ȣ

public:

	BaseObject(int iObjectID, int iObjectType, int iCurX, int iCurY);
	
	virtual void Action() = 0;
	virtual ~BaseObject();
	virtual void Draw(BYTE* bypDest, int iDestWidth, int iDestHeight, int iDestPitch) = 0;

	void ActionInput(int MoveXY);
	void NextFrame();
	void SetSprite(int iStartSprite, int iSpriteCount, int iFrameDelay);

	int GetCurX();
	int GetCurY();
	int GetObjectID();
	int GetSprite();
	bool Get_bGetEndFrame();	
	
};


#endif // !__BASE_OBEJCT_H__
