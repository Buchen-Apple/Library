#pragma once
#ifndef __PLAYER_H__
#define __PLAYER_H__

#include "BaseObject.h"
#include "ActionQueue.h"
#include "SpriteDIB.h"

class CPlayer	:public CBaseOBject
{
private:
	int m_PlayerPosX;
	int m_PlayerPosY;
	Queue m_PQueue;
	CSpritedib* g_cSpriteDib;

private:
	void KeyDownCheck();	// Ű���� �������� üũ
	void ActionLogic();			// �̵�, ���� ���� ó��

public:
	CPlayer();

	virtual void Action();			// Ű�ٿ� üũ �� �̵�, ���� ����ó������ �ϴ� �Լ�
	virtual void Draw(int SpriteIndex, BYTE* bypDest, int iDestWidth, int iDestHeight, int iDestPitch);			// ������ ȭ�鿡 �׸��� �Լ�
	

	~CPlayer();

};

#endif // !__PLAYER_H__
