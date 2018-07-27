#include "stdafx.h"
#include "Player.h"

//---------------------------
// �����ڿ� �Ҹ���
//--------------------------
CPlayer::CPlayer()
{
	Init(&m_PQueue);
	g_cSpriteDib = CSpritedib::Getinstance(5, 0xffffffff, 32);	// �̱������� CSpritedib�� ��ü �ϳ� ���
	m_PlayerPosX = 300;
	m_PlayerPosY = 300;

}

CPlayer::~CPlayer()
{
	delete g_cSpriteDib;
}

//---------------------------
// Ű�ٿ� üũ �� �̵�,����ó������ �ϴ� �Լ�
//--------------------------
void CPlayer::Action()
{	
	KeyDownCheck();
	ActionLogic();
}

//---------------------------
// ������ �� DC�� �����ϴ� �Լ�
//--------------------------
void CPlayer::Draw(int SpriteIndex, BYTE* bypDest, int iDestWidth, int iDestHeight, int iDestPitch)
{
	BOOL iCheck = g_cSpriteDib->DrawSprite(SpriteIndex, m_PlayerPosX, m_PlayerPosY, bypDest, iDestWidth, iDestHeight, iDestPitch);
	if (!iCheck)
		exit(-1);
}						

//---------------------------
// ���� Ű üũ �� ť�� ���� �ִ´�.
//--------------------------
void CPlayer::KeyDownCheck()
{
	//---------------------------
	// Ű���� �Է� ��Ʈ. ť�� �ൿ�� �ִ´�.
	// --------------------------
	if (GetAsyncKeyState(VK_LEFT) & 0x8000 && GetAsyncKeyState(VK_UP) & 0x8000)		// �»�
		Enqueue(&m_PQueue, m_PlayerPosX -1, m_PlayerPosY -1);

	else if (GetAsyncKeyState(VK_RIGHT) & 0x8000 && GetAsyncKeyState(VK_UP) & 0x8000)	// ���
		Enqueue(&m_PQueue, m_PlayerPosX + 1, m_PlayerPosY - 1);

	else if (GetAsyncKeyState(VK_LEFT) & 0x8000 && GetAsyncKeyState(VK_DOWN) & 0x8000)	// ����
		Enqueue(&m_PQueue, m_PlayerPosX - 1, m_PlayerPosY + 1);

	else if (GetAsyncKeyState(VK_RIGHT) & 0x8000 && GetAsyncKeyState(VK_DOWN) & 0x8000)	// ����
		Enqueue(&m_PQueue, m_PlayerPosX + 1, m_PlayerPosY + 1);

	else if (GetAsyncKeyState(VK_UP) & 0x8000)						// ��
		Enqueue(&m_PQueue, m_PlayerPosX, m_PlayerPosY - 1);

	else if (GetAsyncKeyState(VK_DOWN) & 0x8000)					// ��	
		Enqueue(&m_PQueue, m_PlayerPosX, m_PlayerPosY + 1);

	else if (GetAsyncKeyState(VK_LEFT) & 0x8000)					// ��
		Enqueue(&m_PQueue, m_PlayerPosX - 1, m_PlayerPosY);

	else if (GetAsyncKeyState(VK_RIGHT) & 0x8000)					// ��
		Enqueue(&m_PQueue, m_PlayerPosX + 1, m_PlayerPosY);
}

void CPlayer::ActionLogic()
{
	// ť�� �ִ� ���� �����ͼ� �̵�ó��.
	Dequeue(&m_PQueue, &m_PlayerPosX, &m_PlayerPosY);

}