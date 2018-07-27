#include "stdafx.h"
#include "PlayerObject.h"
#include "Network_Func.h"
#include "EffectObject.h"
#include "ProtocolBuff\ProtocolBuff.h"

extern CList<BaseObject*> list;					// Main���� �Ҵ��� ��ü ���� ����Ʈ	

using namespace Library_Jingyu;

//---------------------------
// ������
// --------------------------
PlayerObject::PlayerObject(int iObjectID, int iObjectType, int iCurX, int iCurY, int iDirCur)
	:BaseObject(iObjectID, iObjectType, iCurX, iCurY)
{
	m_dwActionCur = 100;
	m_dwActionOld = 100;
	m_iDirCur = iDirCur;
	m_iDirOld = 0;
	m_bAttackState = FALSE;
	m_bIdleState = FALSE;
	m_dwTargetID = NULL;
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
	// ���� ���� ���
	if (m_bAttackState == TRUE)
	{
		// ������ �������� �ƴ϶�� 
		if (Get_bGetEndFrame() == FALSE)
		{
			// �� ĳ������ ���, �޽����� ������ ���� �޽����� ��ȯ.��, �� ĳ���Ͱ� ���� �߿��� �ٸ� �׼� ��� ����
			if(m_bPlayerCharacter == TRUE)			
				m_dwActionCur = m_dwActionOld;
		}

		// ������ �������̶��
		else
		{		
			// ���� ���¸� IDLE�� ����
			m_dwActionCur = dfACTION_IDLE;			

			// �� ĳ���Ͷ�� 
			if (m_bPlayerCharacter == TRUE)
			{
				// ���� ��ü. �̰ɷ�, switch/case���� IDLE�� ���� �� ��Ŷ�� �Ⱥ����� �ȴ�.
				m_dwActionOld = m_dwActionCur;	
			}

			// ���� ��� ����
			m_bAttackState = FALSE;
			m_dwNowAtkType = 0;
						
		}			
	}

	// ���� �޽��� ó�� ---------------------------------------
	switch (m_dwActionCur)
	{
	case dfACTION_ATTACK_01_LEFT:
	case dfACTION_ATTACK_02_LEFT:
	case dfACTION_ATTACK_03_LEFT:

		// �� ĳ������ ���, ���� ���� �ƴҶ��� �Ʒ� ���� ó��
		// �� ĳ���Ͱ� �ƴ� ���, ���� ���� ��Ŷ�� �������� ������ ó��
		if ((m_bPlayerCharacter == TRUE && m_bAttackState == FALSE) || (m_bPlayerCharacter == FALSE && m_bPacketProcCheck == TRUE))		
		{
			m_bAttackState = TRUE;
			m_bIdleState = FALSE;
			m_bPacketProcCheck = FALSE;

			// ���ݸ�� ����
			if (m_dwActionCur == dfACTION_ATTACK_01_LEFT)
			{
				if (m_iDirCur == dfDIRECTION_LEFT)
				{
					SetSprite(CSpritedib::ePLAYER_ATTACK1_L01, 4, 3);
					m_dwNowAtkType = dfACTION_ATTACK_01_LEFT;					
				}
				else if (m_iDirCur == dfDIRECTION_RIGHT)
				{
					SetSprite(CSpritedib::ePLAYER_ATTACK1_R01, 4, 3);
					m_dwNowAtkType = dfACTION_ATTACK_01_RIGHT;
				}
			}

			else if (m_dwActionCur == dfACTION_ATTACK_02_LEFT)
			{
				if (m_iDirCur == dfDIRECTION_LEFT)
				{
					SetSprite(CSpritedib::ePLAYER_ATTACK2_L01, 4, 4);
					m_dwNowAtkType = dfACTION_ATTACK_02_LEFT;
				}
				else if (m_iDirCur == dfDIRECTION_RIGHT)
				{
					SetSprite(CSpritedib::ePLAYER_ATTACK2_R01, 4, 4);
					m_dwNowAtkType = dfACTION_ATTACK_02_RIGHT;
				}
			}

			else if (m_dwActionCur == dfACTION_ATTACK_03_LEFT)
			{
				if (m_iDirCur == dfDIRECTION_LEFT)
				{
					SetSprite(CSpritedib::ePLAYER_ATTACK3_L01, 6, 4);
					m_dwNowAtkType = dfACTION_ATTACK_03_LEFT;
				}
				else if (m_iDirCur == dfDIRECTION_RIGHT)
				{
					SetSprite(CSpritedib::ePLAYER_ATTACK3_R01, 6, 4);
					m_dwNowAtkType = dfACTION_ATTACK_03_RIGHT;
				}
			}	

			// ���� ����Ʈ ����
			// 1. �� ���� Ÿ�Կ� ���� ����Ʈ ����
			// �������� ���� Ÿ��(���� 1,2,3��)�� ���� ����Ʈ ����
			BaseObject* Effect = new EffectObject(NULL, 2, m_iCurX, m_iCurY, m_dwNowAtkType);

			// ������ ����Ʈ�� ���� ��������Ʈ ����
			Effect->SetSprite(CSpritedib::eEFFECT_SPARK_01, 4, 3);

			// ����Ʈ�� ���
			list.push_back(Effect);


			// ��Ʈ��ũ. 
			// �� ĳ������ ��� ��Ŷ ����
			if (m_bPlayerCharacter == TRUE)
			{
				CProtocolBuff headerBuff(dfNETWORK_PACKET_HEADER_SIZE);

				// ���� �ൿ�� ���, �����̾����� STOP��Ŷ ������ ����. �̹� �����ִ� �����̱� ������!
				if (m_dwActionOld != dfACTION_IDLE && m_dwActionOld != dfACTION_ATTACK_01_LEFT && m_dwActionOld != dfACTION_ATTACK_02_LEFT && m_dwActionOld != dfACTION_ATTACK_03_LEFT)
				{
					// ���� �ൿ�� �̵� ���̾�����, Stop ��Ŷ�� Send()�ؼ� �ϴ� ������·� �����.	
					try
					{
						CProtocolBuff packet(dfPACKET_CS_MOVE_STOP_SIZE);

						SendProc_MoveStop((char*)&headerBuff, (char*)&packet, m_iDirCur, m_iCurX, m_iCurY);
						SendPacket((char*)&headerBuff, (char*)&packet);

					}
					catch (CException exc)
					{
						TCHAR* text = (TCHAR*)exc.GetExceptionText();
						ErrorTextOut(text);
						exit(-1);
					}
					
				}

				// ���� ��Ŷ Send
				headerBuff.Clear();
				CProtocolBuff atkPacket(dfPACKET_CS_ATTACK1_SIZE);
				try
				{
					if (m_dwActionCur == dfACTION_ATTACK_01_LEFT)
						SendProc_Atk_01((char*)&headerBuff, (char*)&atkPacket, m_iDirCur, m_iCurX, m_iCurY);

					else if (m_dwActionCur == dfACTION_ATTACK_02_LEFT)
						SendProc_Atk_02((char*)&headerBuff, (char*)&atkPacket, m_iDirCur, m_iCurX, m_iCurY);

					else if (m_dwActionCur == dfACTION_ATTACK_03_LEFT)
						SendProc_Atk_03((char*)&headerBuff, (char*)&atkPacket, m_iDirCur, m_iCurX, m_iCurY);

					SendPacket((char*)&headerBuff, (char*)&atkPacket);

				}
				catch (CException exc)
				{
					TCHAR* text = (TCHAR*)exc.GetExceptionText();
					ErrorTextOut(text);
					exit(-1);
				}								
			}

		}

		break;

		// IDLE �޽��� ó�� ---------------------------------------
	case dfACTION_IDLE:
		// ���� ���� �ƴϰ�, ��� �ڼ��� �ƴϸ� ������ ����.
		if (m_bAttackState == FALSE && m_bIdleState == FALSE)
		{
			if (m_iDirCur == dfDIRECTION_LEFT)
				SetSprite(CSpritedib::ePLAYER_STAND_L01, 5, 5);
			else if (m_iDirCur == dfDIRECTION_RIGHT)
				SetSprite(CSpritedib::ePLAYER_STAND_R01, 5, 5);
			m_bIdleState = TRUE;	// �����¸� TRUE�� �����, ���� ������ ����ڼ� �޽����� �� ���͵� �ִϸ��̼��� ó������ ��� ����.		

			// �� ĳ�����̰�, �̵� �׼��� ���̵��� �ƴ϶�� ��� ���� ��Ŷ Send
			if (m_bPlayerCharacter == TRUE && m_dwActionCur != m_dwActionOld)
			{
				try
				{
					CProtocolBuff headerBuff(dfNETWORK_PACKET_HEADER_SIZE);
					CProtocolBuff packet(dfPACKET_CS_MOVE_STOP_SIZE);

					SendProc_MoveStop((char*)&headerBuff, (char*)&packet, m_iDirCur, m_iCurX, m_iCurY);
					SendPacket((char*)&headerBuff, (char*)&packet);

				}
				catch (CException exc)
				{
					TCHAR* text = (TCHAR*)exc.GetExceptionText();
					ErrorTextOut(text);
					exit(-1);
				}
				
			}
		}		
		break;

		// �̵� �޽��� ó�� ---------------------------------------
	default:
		m_bIdleState = FALSE;
		if (m_bAttackState == FALSE)
		{
			// Ű �Է¿� ���� ��ǥ �̵� --------------------
			if (m_dwActionCur == dfACTION_MOVE_LL)			// LL
			{
				m_iDirCur = dfDIRECTION_LEFT;

				// ���� �̻� ������ ���� ���ϵ��� ����
				m_iCurX -= PLAYER_XMOVE_PIXEL;				
				if (m_iCurX < dfRANGE_MOVE_LEFT)
					m_iCurX = dfRANGE_MOVE_LEFT;
				
			}

			else if (m_dwActionCur == dfACTION_MOVE_LU)		// LU
			{
				m_iDirCur = dfDIRECTION_LEFT;

				// ���� Y��ǥ�� dfRANGE_MOVE_TOP(���� �� ��)�̶��, x��ǥ �������� ����
				if (m_iCurY != dfRANGE_MOVE_TOP)
				{
					m_iCurX -= PLAYER_XMOVE_PIXEL;
					if (m_iCurX < dfRANGE_MOVE_LEFT)
						m_iCurX = dfRANGE_MOVE_LEFT;
				}

				// ����, X��ǥ�� dfRANGE_MOVE_LEFT(���� ���� ��)�̶��, Y��ǥ�� �������� ����. 
				if (m_iCurX != dfRANGE_MOVE_LEFT)
				{
					m_iCurY -= PLAYER_YMOVE_PIXEL;
					if (m_iCurY < dfRANGE_MOVE_TOP)
						m_iCurY = dfRANGE_MOVE_TOP;
				}
				
			}

			else if (m_dwActionCur == dfACTION_MOVE_UU)		// UU
			{
				m_iCurY -= PLAYER_YMOVE_PIXEL;
				if (m_iCurY < dfRANGE_MOVE_TOP)
					m_iCurY = dfRANGE_MOVE_TOP;
			}

			else if (m_dwActionCur == dfACTION_MOVE_RU)		// RU
			{
				m_iDirCur = dfDIRECTION_RIGHT;

				// ���� Y��ǥ�� dfRANGE_MOVE_TOP(���� �� ��)�̶��, x��ǥ �������� ����
				if (m_iCurY != dfRANGE_MOVE_TOP)
				{
					m_iCurX += PLAYER_XMOVE_PIXEL;
					if (m_iCurX > dfRANGE_MOVE_RIGHT)
						m_iCurX = dfRANGE_MOVE_RIGHT;
				}

				// ����, X��ǥ�� dfRANGE_MOVE_RIGHT(���� ������ ��)�̶��, Y��ǥ�� �������� ����. 
				if (m_iCurX != dfRANGE_MOVE_RIGHT)
				{
					m_iCurY -= PLAYER_YMOVE_PIXEL;
					if (m_iCurY < dfRANGE_MOVE_TOP)
						m_iCurY = dfRANGE_MOVE_TOP;
				}
				
			}

			else if (m_dwActionCur == dfACTION_MOVE_RR)		// RR
			{
				m_iDirCur = dfDIRECTION_RIGHT;

				m_iCurX += PLAYER_XMOVE_PIXEL;
				if (m_iCurX > dfRANGE_MOVE_RIGHT)
					m_iCurX = dfRANGE_MOVE_RIGHT;				
			}

			else if (m_dwActionCur == dfACTION_MOVE_RD)		// RD
			{
				m_iDirCur = dfDIRECTION_RIGHT;

				// ���� Y��ǥ�� dfRANGE_MOVE_BOTTOM(���� �ٴ� ��)�̶��, X��ǥ �������� ����
				if (m_iCurY != dfRANGE_MOVE_BOTTOM)
				{
					m_iCurX += PLAYER_XMOVE_PIXEL;
					if (m_iCurX > dfRANGE_MOVE_RIGHT)
						m_iCurX = dfRANGE_MOVE_RIGHT;
				}

				// ����, X��ǥ�� dfRANGE_MOVE_RIGHT(���� ������ ��)�̶��, Y��ǥ�� �������� ����. 
				if (m_iCurX != dfRANGE_MOVE_RIGHT)
				{
					m_iCurY += PLAYER_YMOVE_PIXEL;
					if (m_iCurY > dfRANGE_MOVE_BOTTOM)
						m_iCurY = dfRANGE_MOVE_BOTTOM;
				}
				
			}

			else if (m_dwActionCur == dfACTION_MOVE_DD)		// DD
			{
				m_iCurY += PLAYER_YMOVE_PIXEL;
				if (m_iCurY > dfRANGE_MOVE_BOTTOM)
					m_iCurY = dfRANGE_MOVE_BOTTOM;
			}

			else if (m_dwActionCur == dfACTION_MOVE_LD)		// LD
			{
				m_iDirCur = dfDIRECTION_LEFT;

				// ���� Y��ǥ�� dfRANGE_MOVE_BOTTOM(���� �ٴ� ��)�̶��, X��ǥ �������� ����
				if (m_iCurY != dfRANGE_MOVE_BOTTOM)
				{
					m_iCurX -= PLAYER_XMOVE_PIXEL;
					if (m_iCurX < dfRANGE_MOVE_LEFT)
						m_iCurX = dfRANGE_MOVE_LEFT;
				}

				// ����, X��ǥ�� dfRANGE_MOVE_LEFT(���� ���� ��)�̶��, Y��ǥ�� �������� ����.
				if (m_iCurX != dfRANGE_MOVE_LEFT)
				{
					m_iCurY += PLAYER_YMOVE_PIXEL;
					if (m_iCurY > dfRANGE_MOVE_BOTTOM)
						m_iCurY = dfRANGE_MOVE_BOTTOM;
				}
			}

			// ���� ���¿� ���� ���°� �ٸ��ٸ�
			if (m_dwActionCur != m_dwActionOld)
			{
				// ���� �ִϸ��̼� ��ü
				if (m_iDirCur != m_iDirOld ||	// ���� ����� ���� ������ �ٸ��ų�
					m_dwActionOld == dfACTION_IDLE ||	// ���� ���°� ��⿴�ų�
					m_dwActionOld == dfACTION_ATTACK_01_LEFT || m_dwActionOld == dfACTION_ATTACK_02_LEFT || m_dwActionOld == dfACTION_ATTACK_03_LEFT) // ���� ���°� ����1, ����2, ����3 �� �ϳ����ٸ�
				{
					if (m_iDirCur == dfDIRECTION_LEFT)
						SetSprite(CSpritedib::ePLAYER_MOVE_L01, 12, 4);

					else if (m_iDirCur == dfDIRECTION_RIGHT)
						SetSprite(CSpritedib::ePLAYER_MOVE_R01, 12, 4);
				}				
			}

			// �� ĳ�����̰�, ������ ���°� �ٸ��ٸ�, �̵� ���� ������ Send
			if (m_bPlayerCharacter == TRUE && m_dwActionOld != m_dwActionCur)
			{
				try
				{
					CProtocolBuff headerBuff(dfNETWORK_PACKET_HEADER_SIZE);
					CProtocolBuff packet(dfPACKET_CS_MOVE_START_SIZE);

					SendProc_MoveStart((char*)&headerBuff, (char*)&packet, m_dwActionCur, m_iCurX, m_iCurY);
					SendPacket((char*)&headerBuff, (char*)&packet);

				}
				catch (CException exc)
				{
					TCHAR* text = (TCHAR*)exc.GetExceptionText();
					ErrorTextOut(text);
					exit(-1);
				}				
			}
		}
		break;
	}

	m_iDirOld = m_iDirCur;			// ���� ��ü
	m_dwActionOld = m_dwActionCur;	// ���� ��ü
}

//---------------------------
// ������ �� DC�� �����ϴ� �Լ�
//--------------------------
void PlayerObject::Draw(BYTE* bypDest, int iDestWidth, int iDestHeight, int iDestPitch)
{
	// ī�޶� ��ǥ ����, ���� ��µ� ��ġ ���Ѵ�.
	CMap* SIngletonMap = CMap::Getinstance(0, 0, 0, 0, 0);
	int ShowX = SIngletonMap->GetShowPosX(m_iCurX);
	int ShowY = SIngletonMap->GetShowPosY(m_iCurY);

	/*
	if (ShowX < 0)
		ShowX = 0;

	if(ShowY < 0)
		ShowY = 0;
	*/

	// �� �׸��� 
	BOOL iCheck = g_cSpriteDib->DrawSprite(CSpritedib::eSHADOW, ShowX, ShowY, bypDest, iDestWidth, iDestHeight, iDestPitch, isPlayer());
	if (!iCheck)
		exit(-1);

	// �� ĳ����
	// ĳ������ m_iCurX, m_iCurY�� ������ �������� ����� ĳ������ ��ġ�̴�. ��, ��Ʈ���� �»���� �ƴϴ�!	
	iCheck = g_cSpriteDib->DrawSprite(m_iSpriteNow, ShowX, ShowY, bypDest, iDestWidth, iDestHeight, iDestPitch, isPlayer());
	if (!iCheck)
		exit(-1);

	// �� HP ������
	iCheck = g_cSpriteDib->DrawSprite(CSpritedib::eGUAGE_HP, ShowX - 35, ShowY + 9, bypDest, iDestWidth, iDestHeight, iDestPitch, isPlayer(), GetHP());
	if (!iCheck)
		exit(-1);
}

//---------------------------
// ���� ��ȯ �Լ�
//--------------------------
void PlayerObject::DirChange(int Dir)
{
	m_iDirCur = Dir;
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
// ���ҵ� ��ŭ m_iHP ����
//--------------------------
void PlayerObject::DamageFunc(int iHP, DWORD TargetID)
{
	if (iHP != -1)
		m_iHP = iHP;

	m_dwTargetID = TargetID;
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

//---------------------------
// m_iDirCur ����
//--------------------------
int PlayerObject::GetCurDir()
{
	return m_iDirCur;
}

//---------------------------
// m_dwNowAtkType ����
//--------------------------
DWORD PlayerObject::GetNowAtkType()
{
	return m_dwNowAtkType;
}

void PlayerObject::ChangeNowAtkType(DWORD dwAtktype)
{
	m_dwNowAtkType = dwAtktype;
}

void PlayerObject::NonMyAtackCheck(BOOL Check)
{
	m_bPacketProcCheck = Check;
}