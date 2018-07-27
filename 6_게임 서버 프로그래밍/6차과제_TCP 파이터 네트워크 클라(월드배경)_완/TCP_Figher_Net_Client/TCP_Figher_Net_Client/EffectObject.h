#pragma once
#ifndef __EFFECT_OBJECT_H__
#define __EFFECT_OBJECT_H__

#include "BaseObject.h"

class CSpritedib;

class EffectObject :public BaseObject
{
private:
	bool m_bEffectStart;	// ���۵� ����Ʈ�� �´��� üũ
	DWORD m_dwAttackID;		// � ��������. (���� 1,2,3) ���ݿ� ���� � ��������Ʈ �����ӿ� ����Ʈ�� ��Ʈ���� �ϴ��� ã�´�.
	DWORD m_StandByFrame;	// ����Ʈ�� �����ǰ�, �� ������ �� ��ŭ ����� �Ŀ� ����Ʈ ��� ����
	bool m_EffectSKipCheck;	// Draw�� ��ŵ�Ǵ� ����Ʈ���� üũ.	

public:
	// ������
	EffectObject(int iObjectID, int iObjectType, int iCurX, int iCurY, int AtkType);

	// �Ҹ���
	~EffectObject();

	// �׼�
	virtual void Action();

	// Draw
	virtual void Draw(BYTE* bypDest, int iDestWidth, int iDestHeight, int iDestPitch);
};




#endif // !__EFFECT_OBJECT_H__

