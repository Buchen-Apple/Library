#pragma once
#ifndef __BASE_OBJECT_H__
#define __BASE_OBJECT_H__

class CBaseOBject
{
public:
	virtual void Action() = 0;																		// Ű�ٿ� üũ �� �̵�,����ó������ �ϴ� �Լ�
	virtual void Draw(int SpriteIndex, BYTE* bypDest, int iDestWidth, int iDestHeight, int iDestPitch) = 0;			// ������ ȭ�鿡 �׸��� �Լ�
};


#endif // !__BASE_OBJECT_H__

