#include <iostream>
#include <cstring>
#include "Scene_Class.h"
#include "CList_Template.h"
#include "MoveQueue.h"

#define HEIGHT 30
#define WIDTH 81

#define UI_HEIGHT 27
#define UI_WIDTH 81

// �������� ����
HANDLE hConsole;
CList<CObject*> list;
CSceneHandle hSceneHandle;
char cBackBuffer[HEIGHT][WIDTH];

// �� enum
enum SceneType
{
	None = 0, Title = 1, Select, Ingame, Lose, Victory
};

// Title�� Select���� �������� ����ϴ� ���� ����. �̸�����
namespace TitleAndSelectText
{
	int m_iArrowShowPosy = 5;	// Ÿ��Ʋ ȭ��, ĳ���� ���ÿ��� ȭ��ǥ�� ǥ�õ� ��ġ. 5�� Ingame / 6�̸� Exit�� ǥ�õȴ�.
	bool m_bArrowShowFlag = true;	//Ÿ��Ʋ ȭ��, ĳ���� ���ÿ��� ȭ��ǥ �������� �����ϴ� ����. ture�� ������ / flase�� �Ⱥ�����. Ʈ���޽� �ݺ��ϸ鼭 ���������δ�.

	char m_cTip[23] = "�� ZŰ�� �����ּ��� ��";
	char m_cTitle[6] = "TITLE";
	char m_cStart[6] = "START";
	char m_cExit[5] = "EXIT";

	char g_cCharSeletTip[23] = "ĳ���͸� ������ �ּ���";
	char g_cCharSeletChaTip_1[35] = "(ü��: 1  / �߰��̻���: 2�ʿ� 2��)";
	char g_cCharSeletChaTip_2[35] = "(ü��: 2  / �߰��̻���: 2�ʿ� 1��)";
	char g_cCharSeletChaTip_3[35] = "(ü��: 5  / �߰��̻���:   ����   )";;
}

// �ܼ� ��� ���� �غ� �۾�
void cs_Initial(void)
{
	CONSOLE_CURSOR_INFO stConsoleCursor;
	// ȭ���� Ŀ�� �Ⱥ��̰� ����
	stConsoleCursor.bVisible = FALSE;
	stConsoleCursor.dwSize = 1; // Ŀ�� ũ��

								// �ܼ�ȭ�� �ڵ��� ���Ѵ�.
	hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleCursorInfo(hConsole, &stConsoleCursor);
}
// �ܼ� ȭ���� Ŀ���� x,y ��ǥ�� �̵�
void cs_MoveCursor(int iPosy, int iPosx)
{
	COORD stCoord;
	stCoord.X = iPosx;
	stCoord.Y = iPosy;
	// ���ϴ� ��ġ�� Ŀ�� �̵�
	SetConsoleCursorPosition(hConsole, stCoord);
}


// --------------------------
// CPlayer Ŭ���� ����Լ�
// --------------------------
// ������
CPlayer::CPlayer()
{
	Type = 1;
	m_iPosX = 0;
	m_iPosY = 0;
	m_bAriveCheck = 0;
	m_MoveQueue = new Queue;
	m_AtkQueue = new Queue;
	m_cLook = 0;
	m_cMissaleLook = 0;
	m_cHP[0] = 0;
	m_cHP[1] = 0;
	m_iCharSpeed = 0;
	m_iBonusMIssaleCount = 0;
	m_BonusMIssaleTImeSave = 0;
	iPlayerMissaleCount = 0;
}
// �÷��̾� ���� ����. CSceneSelect���� �⺻ ���� ����
void CPlayer::InfoSetFunc_1(int m_iArrowShowPosy)
{
	if (m_iArrowShowPosy == 5)
	{
		m_cLook = 'A';
		m_cMissaleLook = '1';
		m_cHP[0] = '0';
		m_cHP[1] = '1';
		m_iCharSpeed = 1;
		m_iBonusMIssaleCount = 2;
	}
	else if (m_iArrowShowPosy == 7)
	{
		m_cLook = 'B';
		m_cMissaleLook = '2';
		m_cHP[0] = '0';
		m_cHP[1] = '2';
		m_iCharSpeed = 1;
		m_iBonusMIssaleCount = 1;
	}
	else if (m_iArrowShowPosy == 9)
	{
		m_cLook = 'V';
		m_cMissaleLook = '3';
		m_cHP[0] = '0';
		m_cHP[1] = '5';
		m_iCharSpeed = 1;
		m_iBonusMIssaleCount = 0;
	}
}
// �÷��̾� ���� ����. CSceneGame���� �� �� �ڼ��� ���� �߰�����
void CPlayer::InfoSetFunc_2()
{
	// �� ������ �����ϸ�, ���� �ð� ����. �Ʊ� ĳ������ �߰� �̻��� �߻縦 ����.
	m_BonusMIssaleTImeSave = GetTickCount64();

	m_iPosX = 40;	// �÷��̾� ���� ��ġ ����
	m_iPosY = 20;
	m_bAriveCheck = true;	// �÷��̾� ���� ���θ� true�� ����

	// �÷��̾� �ൿ ���� ť, ���� ���� ť �ʱ�ȭ
	Init(m_MoveQueue);
	Init(m_AtkQueue);
}
// �÷��̾� �̵�/ ���� ���� üũ
void CPlayer::KeyDownCheck()
{
	// ������ ���� Ű�� üũ��, �ൿ ������ ť�� ����. ������ �ൿ ������ '����'���� �۵�
	if (GetAsyncKeyState(VK_UP) & 0x8001)
	{
		if (m_iPosY - 1 > (UI_HEIGHT / 2))
			Enqueue(m_MoveQueue, m_iPosX, m_iPosY - m_iCharSpeed);
	}
	if (GetAsyncKeyState(VK_DOWN) & 0x8001)
	{
		if (m_iPosY + 1 < UI_HEIGHT - 1)
			Enqueue(m_MoveQueue, m_iPosX, m_iPosY + m_iCharSpeed);
	}
	if (GetAsyncKeyState(VK_RIGHT) & 0x8001)
	{
		if (m_iPosX + 1 < 69)
			Enqueue(m_MoveQueue, m_iPosX + m_iCharSpeed, m_iPosY);
	}
	if (GetAsyncKeyState(VK_LEFT) & 0x8001)
	{
		if (m_iPosX - 1 > 11)
			Enqueue(m_MoveQueue, m_iPosX - m_iCharSpeed, m_iPosY);
	}

	// ������ ���� ��ư�� ��������,���� ������ ť�� ����. ������ ���� ������ '����'���� �۵�
	if (GetAsyncKeyState(0x5A) & 0x8001)
	{
		if (iPlayerMissaleCount < PLAYER_MISSALE_COUNT)
		{
			iPlayerMissaleCount++;
			Enqueue(m_AtkQueue, m_iPosX, m_iPosY - 1);
		}
	}
}
// �÷��̾��� HP�� ��� �Լ�
char* CPlayer::GetHP()
{
	return m_cHP;
}
// �÷��̾��� �������θ� ��� �Լ�
bool CPlayer::GetArive()
{
	return m_bAriveCheck;
}
// �÷��̾��� �̵� ����ó�� �Լ�
void CPlayer::PlayerMoveFunc()
{
	////���� �̵� ���� (������ �������� ���� ���� ����) ////
	int iTempPosx;
	int iTempPosy;

	// ���� ������ ��ġ�� ���ۿ� �����Ѵ�.
	cBackBuffer[m_iPosY][m_iPosX] = m_cLook;

	// ���� ������ x,y ��ġ�� �����Ѵ�.
	iTempPosx = m_iPosX;
	iTempPosy = m_iPosY;

	// �ൿ ������ �޾Ƽ�(ť), �̵��� ��ġ�� ������ �� ��ġ�� �̵���Ų��.
	while (Dequeue(m_MoveQueue, &m_iPosX, &m_iPosY))
	{
		cBackBuffer[iTempPosy][iTempPosx] = ' ';	//��ť�� �����ϸ�, ������ ������ �ִ� ��ġ�� �����̽��� �����Ѵ�.
		cBackBuffer[m_iPosY][m_iPosX] = m_cLook;

		iTempPosx = m_iPosX;	// ���� while������ ��ť�� ������ �� ������, ��ť ���� ��ġ�� �����Ѵ�.
		iTempPosy = m_iPosY;
	}
	
}
// �̻��� �̵� �� �Ҹ� ó�� �Լ�
void CPlayer::MissaleMoveFunc(CMissale* missale, CEnemy* enemy, int* iEnemyAriveCount)
{	
	//// �����Ǿ� �ִ� ������ �̻��� �̵� �� �Ҹ� ���� (������ �������� ���� ���� ����)////		

	// �̻��� �̵� �� �Ҹ� ����// 
	// ����ִ� �̻��� ��, �Ʊ� �̻����� -y������ 1ĭ �̵��Ѵ�.
	for (int i = PLAYER_MISSALE_COUNT; i < ENEMY_MISSALE_COUNT + PLAYER_MISSALE_COUNT; i++)
	{
		if (missale[i].m_bAriveCheck == true && missale[i].m_PlayerOrEnemy == true)	//����ִ� �̻����̸� �Ʊ� �̻����� ���
		{
			missale[i].m_iPosY--;	// -y������ 1ĭ �̵���Ų��.
									
			if (missale[i].m_iPosY == 3)	// -y������ 1ĭ �̵��ߴµ�, �װ� ���̸� false�� ����.
			{
				missale[i].m_bAriveCheck = false;
				iPlayerMissaleCount--;
			}
			// -y������ 1ĭ �̵��ߴµ�, �װ� ���͸� false�� ����� �ش� ��ġ�� ���͸� false�� ����(��� ����)
			// flase�� �� ���ʹ� �ڵ����� ���ۿ��� ���ܵȴ�.
			else if (cBackBuffer[missale[i].m_iPosY][missale[i].m_iPosX] == 'M')
			{
				for (int j = 0; j < ENEMY_COUNT; j++)
				{
					if (enemy[j].m_iPosY == missale[i].m_iPosY && enemy[j].m_iPosX == missale[i].m_iPosX)
					{
						// �ش� ���Ͱ� ����ִ� ���Ͷ��, (Ȥ�� �𸣴� ���⼭ �������� �ٽ��ѹ� üũ. �ڲ� �̻��Ѱ� 1~2���� ���Ҵµ� ������ �����..)
						if (enemy[j].m_bAriveCheck == true)
						{
							enemy[j].m_bAriveCheck = false;
							(*iEnemyAriveCount)--;	// ���� ���� Enemy �� 1��ü ����
							if (*iEnemyAriveCount == 0)
								cBackBuffer[enemy[j].m_iPosY][enemy[j].m_iPosX] = ' ';	// ���� 0���� �Ǹ�, ������ ���� �����̽��� �����. �׷��� ȭ�鿡�� ����� �� ó�� ����.

							missale[i].m_bAriveCheck = false;
							iPlayerMissaleCount--;	// ���� ���� �̻��� �� 1�� ����		
							break;
						}
					}
				}

			}
			// �Ҹ���� �ʴ� �̻����̸� (���� �ƴϰ�, ���Ϳ� �浹������ ����), �̻����� �׸���.
			else
			{
				if (missale[i].m_BonusOrNot == false)
					cBackBuffer[missale[i].m_iPosY][missale[i].m_iPosX] = m_cMissaleLook;
				else
					cBackBuffer[missale[i].m_iPosY][missale[i].m_iPosX] = 'b';
			}
		}
	}

	
}
// �̻��� ���� ó�� �Լ�
void CPlayer::MissaleCreateFunc(CMissale* missale)
{
	//// �̻��� ���� ���� ////
	int iTempPosx;
	int iTempPosy;
	int i = 0;
	// ���� ������ �޾Ƽ�(ť), �ش� ��ǥ�� �̻����� Ȱ��ȭ��Ų��.
	while (Dequeue(m_AtkQueue, &iTempPosx, &iTempPosy))
	{
		// m_bAriveCheck ���� false��(���������� ����) �Ʊ� �̻����� ã�´�.
		for (i = PLAYER_MISSALE_COUNT; i < PLAYER_MISSALE_COUNT + ENEMY_MISSALE_COUNT; i++)
		{
			if (missale[i].m_bAriveCheck == false && missale[i].m_PlayerOrEnemy == true)
				break;
		}

		// i�� ����, PLAYER_MISSALE_COUNT + ENEMY_MISSALE_COUNT���� ������ ���� ������(false) �̻����� ã�Ҵٴ� ���̴� �Ʒ� ���� ����
		if (i < PLAYER_MISSALE_COUNT + ENEMY_MISSALE_COUNT)
		{
			// ã�� ��ġ�� �̻����� ����(true)���� �����, ã�� ��ġ�� x,y���� �����Ѵ�.
			missale[i].m_bAriveCheck = true;
			missale[i].m_BonusOrNot = false;	//���ʽ� ������ üũ�ϱ� ���Ѱ�. ���ʽ��� true / ���ʽ� �̻����� �ƴϸ� false
			missale[i].m_iPosX = iTempPosx;
			missale[i].m_iPosY = iTempPosy;

			// �׸���, ������ �̻����� ���ۿ� �����Ѵ�.
			cBackBuffer[missale[i].m_iPosY][missale[i].m_iPosX] = m_cMissaleLook;
		}
	}

}
// ���ʽ� �̻��� ���� ó��. �̵��� MissaleMoveFunc���� ���� ó��
void CPlayer::BonusMissaleFunc(CMissale* missale, ULONGLONG TimeCheck)
{
	// ������ ���� �Է°��� ��� ����, ���� �ð����� ���ʽ� �̻����� �߻�ȴ�.
	// ����, ������ ���� �ƴٸ�,
	if (m_BonusMIssaleTImeSave + PLAYER_AI_ATACK_TIME <= TimeCheck)
	{
		// ���� �÷��̾ ������ �ִ� �߰� �̻��� �߻� ī��Ʈ��ŭ �ݺ��ϸ� �̻��� ����
		// �߰� �̻��� �߻� ī��Ʈ�� 0�̶��, �ݺ����� �ʴ´�.
		for (int h = 1; h <= m_iBonusMIssaleCount; ++h)
		{
			int i = 0;

			// m_bAriveCheck ���� false��(���������� ����) �Ʊ� �̻����� ã�´�.
			for (i = PLAYER_MISSALE_COUNT; i < PLAYER_MISSALE_COUNT + ENEMY_MISSALE_COUNT; i++)
			{
				if (missale[i].m_bAriveCheck == false && missale[i].m_PlayerOrEnemy == true)
					break;
			}

			// i�� ����, ENEMY_MISSALE_COUNT + PLAYER_MISSALE_COUNT���� ������ false�� �迭�� ã�Ҵٴ� ���̴� �Ʒ� ���� ����
			if (i < ENEMY_MISSALE_COUNT + PLAYER_MISSALE_COUNT)
			{
				// ã�� ��ġ�� �̻����� ����(true)�� �����, ���ʽ� �̻���(true)�� �����ϰ� �̻����� �����Ѵ�.
				missale[i].m_bAriveCheck = true;
				missale[i].m_BonusOrNot = true;
				if (h == 1)
				{
					missale[i].m_iPosX = m_iPosX - 2;
					missale[i].m_iPosY = m_iPosY - 1;
					cBackBuffer[missale[i].m_iPosY][missale[i].m_iPosX] = 'b';
				}
				else if (h == 2)
				{
					missale[i].m_iPosX = m_iPosX + 2;
					missale[i].m_iPosY = m_iPosY - 1;
					cBackBuffer[missale[i].m_iPosY][missale[i].m_iPosX] = 'b';
				}
			}
		}
		// �̻����� ��� ����������, ���� �ð��� �����Ѵ�. �׷��� ������ �� �߰� �̻����� �߻��ϴϱ�..
		m_BonusMIssaleTImeSave = TimeCheck;
	}

}
// �Ҹ���
CPlayer::~CPlayer()
{
	delete m_MoveQueue;
	delete m_AtkQueue;
}


// --------------------------
// CEnemy Ŭ���� ����Լ�
// --------------------------
// ������
CEnemy::CEnemy()
{
	Type = 2;
	EnemyAtkQueue = new Queue;
}
// ���� ������
CEnemy& CEnemy::operator=(const CEnemy* ref)
{
	m_iPosX = ref->m_iPosX;
	m_iPosY = ref->m_iPosY;
	m_bAriveCheck = ref->m_bAriveCheck;
	m_iAttackTick = ref->m_iAttackTick;
	m_MoveTimeSave = ref->m_MoveTimeSave;
	m_AttackTimeSave = ref->m_AttackTimeSave;
	EnemyAtkQueue = ref->EnemyAtkQueue;

	return *this;
}
// ���� ���� ����
void CEnemy::InfoSetFunc(int* iEnemyPosx, int* iEnemyPosy, int* iAttackTick1, int* iAttackTick2, int* iAtkPatten, ULONGLONG AttackTimeSave)
{
	// -----------------------
	//		���� ���� ����
	// -----------------------

	Init(EnemyAtkQueue);

	//  ���� �ð� ����. 
	m_AttackTimeSave = AttackTimeSave;	// ������ ���� AI �۵��� ���� �ð� üũ.�� �ð����� m_iAttackTick�ð� ���Ŀ� AI �۵�
	m_MoveTimeSave = AttackTimeSave;	// ������ �̵� AI �۵��� ���� �ð� üũ.�� �ð����� ENEMY_AI_TIME�ð� ���Ŀ� AI �۵�

	m_iPosX = *iEnemyPosx;
	m_iPosY = *iEnemyPosy;
	m_iAttackTick = *iAttackTick1;
	m_bAriveCheck = true;

	// �Ʒ� if���� ������ ���� AI ƽ ����
	if (*iAtkPatten == 4)
	{
		*iAttackTick1 = *iAttackTick2;
		*iAtkPatten = 0;
		*iAttackTick2 += 600;
	}
	else
	{
		*iAttackTick1 += 1500;
		(*iAtkPatten)++;
	}

	// �Ʒ� if���� ������ ���� ���� ��ġ ����
	if (*iEnemyPosx > 60)
	{
		*iEnemyPosx = 17;
		*iEnemyPosy += 1;
	}
	else
		*iEnemyPosx += 5;
}
// ���� AI �̵� ó��
void CEnemy::EnemyMoveFunc(int* iEnemyAIState, ULONGLONG ulEnemyAiTimeCur, bool* bAiActiveCheck)
{
	//// Enemy �̵� AI ���� (���� ������ Enemy�� �������� ���� ����)	////
	// ���� AI�� �۵����� 1.4��(ENEMY_AI_TIME)�� �� ���
	if (m_MoveTimeSave + ENEMY_AI_TIME <= ulEnemyAiTimeCur)
	{
		if (*iEnemyAIState == 0)
			m_iPosX -= 2;
		else if (*iEnemyAIState == 1)
			m_iPosY--;	
		else if (*iEnemyAIState == 2)
			m_iPosX += 2;
		else if (*iEnemyAIState == 3)
			m_iPosY++;

		*bAiActiveCheck = true;					// �� ���̶� �۵��ϸ� true�� ����
		m_MoveTimeSave = ulEnemyAiTimeCur;	// �ð��� ���� ����. �׷��� ���� AI�� �۵���ų ������ �Ǳ� ������.
	}	
	cBackBuffer[m_iPosY][m_iPosX] = 'M';
}
// ���� �������� ����
bool CEnemy::GetArive()
{
	return m_bAriveCheck;
}
// ���� ���� ��ť ó��
void CEnemy::EnemyEnqueueFunc(ULONGLONG ulEnemyAiTimeCur)
{
	//// Enemy ���� AI ���� (���� ������ Enemy�� �������� ���� ����). �� �̻����� �����Ѵ�.	////

	// Enemy ���� ť �ֱ� ����
	// �ش� ������ ������ ���� ������, ���� ����� ť�� ����
	if (ulEnemyAiTimeCur >= m_AttackTimeSave + m_iAttackTick)
	{
		Enqueue(EnemyAtkQueue, m_iPosX, m_iPosY + 1);
		m_AttackTimeSave = ulEnemyAiTimeCur;	// ������ ����ü ������ ���� �ð� ����.
	}
}
// ���� �̻��� �̵� ó��
void CEnemy::EnemyMissaleMoveFunc(CMissale* missale, CPlayer* player)
{
	//// Enemy �̻��� �̵� �� �Ҹ� ���� (���� ������ ��� ���� ���� ����)	////
	// ����ִ� ��� �̻����� -y������ 1ĭ �̵� �� ���ۿ� ����.		
	for (int i = 0; i < ENEMY_MISSALE_COUNT; i++)
	{
		if (missale[i].m_bAriveCheck == true && missale[i].m_PlayerOrEnemy == false)	// ����ְ�, ������ �̻����̸�. Y++�� 1ĭ �̵�
		{
			missale[i].m_iPosY++;

			// +y������ 1ĭ �̵��ߴµ�, �װ� ���̸� false�� ����.
			if (missale[i].m_iPosY == UI_HEIGHT - 1)
				missale[i].m_bAriveCheck = false;

			// +y������ 1ĭ �̵��ߴµ�, �װ� ������.  
			else if (cBackBuffer[missale[i].m_iPosY][missale[i].m_iPosX] == player->m_cLook)
			{
				// ������ hp�� 1 ��´�. 1���� ��, HP ��ġ�� �����ִ� ���ڿ��� üũ�� ü���� ���ҽ�Ų��.
				if (player->m_cHP[0] != '0')
				{
					player->m_cHP[0]--;
					if (player->m_cHP[0] == '0')
						player->m_cHP[1] = '9';
				}
				else if (player->m_cHP[1] != '0')
					player->m_cHP[1]--;

				// ������ hp�� 0�̶�� ������ �ش� ��ġ�� ������ false�� ����(��� ����)
				if (player->m_cHP[1] == '0')
					player->m_bAriveCheck = false;

				// ������� ��� ����, ������ ���ݹ����� ���� ��ġ�� �����̽��ٷ� �ٲ۴�. �̴�, ���� ���� �� �����̱� or ����ϸ� �Ⱥ��̰� �ϱ��� ��Ȱ�̴�.
				cBackBuffer[player->m_iPosY][player->m_iPosX] = ' ';
			}

			else
				cBackBuffer[missale[i].m_iPosY][missale[i].m_iPosX] = 'v';
		}
	}
}
// ���� ���� ��ť ó��
void CEnemy::EnemyDequeueFunc(CMissale* missale)
{
	//// Enemy ���� ť ������ ���� ////
	// ���� ������ �޾Ƽ�(ť), �ش� ��ǥ�� �̻����� Ȱ��ȭ��Ų��. 
	int iTempPosx;
	int iTempPosy;

	while (Dequeue(EnemyAtkQueue, &iTempPosx, &iTempPosy))
	{
		int i = 0;
		// ��� ������(false) ���� �̻����� ã�´�.
		for (; i < ENEMY_MISSALE_COUNT; i++)
		{
			if (missale[i].m_bAriveCheck == false && missale[i].m_PlayerOrEnemy == false)
				break;
		}

		// i�� ����, ENEMY_MISSALE_COUNT���� ������ false�� �迭�� ã�Ҵٴ� ���̴� �Ʒ� ���� ����
		if (i < ENEMY_MISSALE_COUNT)
		{
			// ã�� ��ġ�� �̻����� ����(true)���� �����, ã�� ��ġ�� x,y���� �����Ѵ�.
			missale[i].m_bAriveCheck = true;
			missale[i].m_iPosX = iTempPosx;
			missale[i].m_iPosY = iTempPosy;

			// �׸���, ������ �̻����� ���ۿ� �����Ѵ�.
			cBackBuffer[missale[i].m_iPosY][missale[i].m_iPosX] = 'v';
		}
	}
}
// �Ҹ���
CEnemy::~CEnemy()
{
	delete EnemyAtkQueue;
}


// --------------------------
// CMissale Ŭ���� ����Լ�
// --------------------------
// ������
CMissale::CMissale()
{
	Type = 3;
}
// ���� ������
CMissale& CMissale::operator=(const CMissale* ref)
{
	m_iPosX = ref->m_iPosX;
	m_iPosY = ref->m_iPosY;
	m_PlayerOrEnemy = ref->m_PlayerOrEnemy;
	m_bAriveCheck = ref->m_bAriveCheck;
	m_BonusOrNot = ref->m_BonusOrNot;

	return *this;

}
// �̻��� ���� ����
void CMissale::InfoSetFunc(bool bIdentity)
{
	// -----------------------
	//	 �̻��� ����
	// -----------------------
	//�迭 0 ~ 49�� ���� ��. �迭 50 ~ 99���� �÷��̾��� ��
	m_PlayerOrEnemy = bIdentity;	// �̻����� ������ �Ʊ�/���������� ����. Ʈ��� �Ʊ� / �޽��� ����
	m_bAriveCheck = false;			// �̻����� ���� ���� ����. ó�� ���۶��� ��� ��Ȱ��ȭ����.
}


// --------------------------
// CSceneBase Ŭ���� ����Լ�
// --------------------------
// CSceneBase::���� �ʱ�ȭ �Լ�
void CSceneBase::BufferClear()
{
	memset(cBackBuffer, 0X20, HEIGHT*WIDTH);
	for (int i = 0; i < HEIGHT; ++i)
		cBackBuffer[i][WIDTH - 1] = '\0';
}
// CSceneBase::���� �ø� �Լ�
void CSceneBase::BufferFlip(int iPosy, int iPosx)
{
	cs_MoveCursor(iPosy, iPosx);
	printf("%s", cBackBuffer[iPosy]);
}


// --------------------------
// CSceneTitle Ŭ���� ����Լ�
// --------------------------
// CSceneTitle:������
// CSceneTitle::Title �Լ�
void CSceneTitle::Title()
{
	memcpy(cBackBuffer[3] + 38, TitleAndSelectText::m_cTitle, sizeof(TitleAndSelectText::m_cTitle) - 1);
	memcpy(cBackBuffer[5] + 38, TitleAndSelectText::m_cStart, sizeof(TitleAndSelectText::m_cStart) - 1);
	memcpy(cBackBuffer[6] + 38, TitleAndSelectText::m_cExit, sizeof(TitleAndSelectText::m_cExit) - 1);

	memcpy(cBackBuffer[8] + 30, TitleAndSelectText::m_cTip, strlen(TitleAndSelectText::m_cTip) + 1 - 1);

	// iArrowShowPosy�� ���� ���� ���� ȭ��ǥ�� ������
	if (TitleAndSelectText::m_iArrowShowPosy == 5)
	{
		cBackBuffer[6][35] = ' ';
		cBackBuffer[6][36] = ' ';
	}
	else if (TitleAndSelectText::m_iArrowShowPosy == 6)
	{
		cBackBuffer[5][35] = ' ';
		cBackBuffer[5][36] = ' ';
	}

	// ȭ��ǥ �������� ����
	if (TitleAndSelectText::m_bArrowShowFlag == true)
	{
		cBackBuffer[TitleAndSelectText::m_iArrowShowPosy][35] = '-';
		cBackBuffer[TitleAndSelectText::m_iArrowShowPosy][36] = '>';
	}
	else
	{
		cBackBuffer[TitleAndSelectText::m_iArrowShowPosy][35] = ' ';
		cBackBuffer[TitleAndSelectText::m_iArrowShowPosy][36] = ' ';
	}


	for (int i = 0; i<HEIGHT; ++i)
		BufferFlip(i, 0);
}
// CSceneTitle::run �Լ�
void CSceneTitle::run()
{	
	// ����� �ʱ�ȭ
	BufferClear();
	// Ÿ��Ʋ â ǥ�� �Լ� ȣ��
	Title();
	Sleep(200);

	// �������� �ϱ� ���� ����
	if (TitleAndSelectText::m_bArrowShowFlag == true)
		TitleAndSelectText::m_bArrowShowFlag = false;
	else
		TitleAndSelectText::m_bArrowShowFlag = true;

	// ������ ZŰ�� ������ ��, ���� ȭ��ǥ�� ��� ����Ű�� �ִ����� ���� ���� ���� �ΰ������� EXIT ���� ����
	if (GetAsyncKeyState(0x5A) & 0x8001)
	{
		if (TitleAndSelectText::m_iArrowShowPosy == 5)
			hSceneHandle.LoadScene(Select);

		else if (TitleAndSelectText::m_iArrowShowPosy == 6)
			hSceneHandle.LoadScene(Lose);
	}

	// ������ Ű���� ��/�Ʒ��� �����ϸ�, ������ ��ư�� ���� iArrowShowPosy�� ���� �ٲ���. ��, ȭ��ǥ�� �� �Ʒ��� ��� ����ų�� ����
	if (GetAsyncKeyState(VK_DOWN) & 0x8001)
		TitleAndSelectText::m_iArrowShowPosy = 6;
	if (GetAsyncKeyState(VK_UP) & 0x8001)
		TitleAndSelectText::m_iArrowShowPosy = 5;
}



// --------------------------
// CSceneSelect Ŭ���� ����Լ�
// --------------------------
// run �Լ�
void CSceneSelect::run()
{
	// ����� �ʱ�ȭ
	BufferClear();

	// ĳ���� ����â ǥ�� �Լ� ȣ��
	CharSeleet();
	Sleep(200);

	// �������� �ϱ� ���� ����
	if (TitleAndSelectText::m_bArrowShowFlag == true)
		TitleAndSelectText::m_bArrowShowFlag = false;
	else
		TitleAndSelectText::m_bArrowShowFlag = true;
	
	// Z��ư Ű �ٿ� üũ
	KeyDown();

	// ������ Ű���� ��/�Ʒ��� �����ϸ�, ������ ��ư�� ���� iArrowShowPosy�� ���� �ٲ���. ��, ȭ��ǥ�� ��, �߰�, �Ʒ� �� ��� ����ų�� ����
	if (GetAsyncKeyState(VK_DOWN) & 0x8001)
	{
		if (TitleAndSelectText::m_iArrowShowPosy != 9)
			TitleAndSelectText::m_iArrowShowPosy += 2;
	}
	if (GetAsyncKeyState(VK_UP) & 0x8001)
	{
		if (TitleAndSelectText::m_iArrowShowPosy != 5)
			TitleAndSelectText::m_iArrowShowPosy -= 2;
	}

}
// ĳ���� ���� �Լ�
void CSceneSelect::CharSeleet()
{
	memcpy(cBackBuffer[3] + 30, TitleAndSelectText::g_cCharSeletTip, sizeof(TitleAndSelectText::g_cCharSeletTip) - 1);

	cBackBuffer[5][23] = 'A';
	memcpy(cBackBuffer[5] + 25, TitleAndSelectText::g_cCharSeletChaTip_1, sizeof(TitleAndSelectText::g_cCharSeletChaTip_1) - 1);

	cBackBuffer[7][23] = 'B';
	memcpy(cBackBuffer[7] + 25, TitleAndSelectText::g_cCharSeletChaTip_2, sizeof(TitleAndSelectText::g_cCharSeletChaTip_2) - 1);

	cBackBuffer[9][23] = 'V';
	memcpy(cBackBuffer[9] + 25, TitleAndSelectText::g_cCharSeletChaTip_3, sizeof(TitleAndSelectText::g_cCharSeletChaTip_3) - 1);

	memcpy(cBackBuffer[11] + 30, TitleAndSelectText::m_cTip, sizeof(TitleAndSelectText::m_cTip) - 1);

	// iArrowShowPosy�� ���� ���� ���� ȭ��ǥ�� ������
	if ((TitleAndSelectText::m_iArrowShowPosy) == 5)
	{
		cBackBuffer[7][20] = ' ';
		cBackBuffer[7][21] = ' ';

		cBackBuffer[9][20] = ' ';
		cBackBuffer[9][21] = ' ';
	}
	else if (TitleAndSelectText::m_iArrowShowPosy == 7)
	{
		cBackBuffer[5][20] = ' ';
		cBackBuffer[5][21] = ' ';

		cBackBuffer[9][20] = ' ';
		cBackBuffer[9][21] = ' ';
	}
	else if (TitleAndSelectText::m_iArrowShowPosy == 9)
	{
		cBackBuffer[5][20] = ' ';
		cBackBuffer[5][21] = ' ';

		cBackBuffer[7][20] = ' ';
		cBackBuffer[7][21] = ' ';
	}

	// ȭ��ǥ �������� ����
	if (TitleAndSelectText::m_bArrowShowFlag == true)
	{
		cBackBuffer[TitleAndSelectText::m_iArrowShowPosy][20] = '-';
		cBackBuffer[TitleAndSelectText::m_iArrowShowPosy][21] = '>';
	}
	else
	{
		cBackBuffer[TitleAndSelectText::m_iArrowShowPosy][20] = ' ';
		cBackBuffer[TitleAndSelectText::m_iArrowShowPosy][21] = ' ';
	}

	for (int i = 0; i<HEIGHT; ++i)
		BufferFlip(i, 0);
}
// Ű���� üũ
void CSceneSelect::KeyDown()
{
	// ������ ZŰ�� ������ ��, ���� ȭ��ǥ�� ��� ����Ű�� �ִ����� ���� ĳ���� ���� ����
	if (GetAsyncKeyState(0x5A) & 0x8001)
	{
		/*CList<CObject*>::iterator itor;
		itor = list.begin();*/
		CPlayer* player = new CPlayer;

		player->InfoSetFunc_1(TitleAndSelectText::m_iArrowShowPosy);
		list.push_back(player);
		hSceneHandle.LoadScene(Ingame);
	}

}


// --------------------------
// CSceneGame Ŭ���� ����Լ�
// --------------------------
// ������
CSceneGame::CSceneGame()
{
	bAiActiveCheck = false;
	iEnemyAIState = 0;	
	// ---------------------------
	//	���� ���� �����Ҵ�
	// ---------------------------
	player = new CPlayer;
	enemy = new CEnemy[ENEMY_COUNT];
	missale = new CMissale[ENEMY_MISSALE_COUNT + PLAYER_MISSALE_COUNT];

	// ---------------------------
	//	 �ð� UI�� ���� ���� ����
	// ---------------------------						
	Time[5] = '\0';
	a = '0';
	b = '0';
	c = '0';
	d = '0';

	timeBeginPeriod(1);
	m_ulStartTimeSave = GetTickCount64();
	ulUITimeSave = m_ulStartTimeSave;
	timeEndPeriod(1);

	// ---------------------------
	//	 �÷��̾� ���� �߰� ����
	// ---------------------------
	// Select���� ������ �÷��̾� ������ ������. 
	CList<CObject*>::iterator itor;
	for (itor = list.begin(); itor != list.end(); itor++)
	{
		player = (CPlayer*)*itor;
		if (player->Type == 1)
			break;
	}	

	// ������ �÷��̾� ���� ����
	player->InfoSetFunc_2();

	// ���� �÷��̾� ���� ���� ��, ���� ���õ� ������ List�� �ִ´�.
	list.erase(itor);
	list.push_back(player);

	// ---------------------------
	//	 ���� ���� ����
	// ---------------------------

	// ���� ���� ����
	// -> ���� ������, ��ü���� �ٸ� ������ ���� ������ ���⼭ ����
	int iEnemyPosx = 17;
	int iEnemyPosy = 5;
	int m_iAttackTick1 = 1400;
	int m_iAttackTick2 = 1000;
	int iAtkPatten = 0;

	// ������ ���� ������ ����
	iEnemyAriveCount = 0;;	// ���� ���� ������ ��. ���� ������ �� ���� �ϳ��� ����.
	for (int i = 0; i < ENEMY_COUNT; i++)
	{
		enemy[i].InfoSetFunc(&iEnemyPosx, &iEnemyPosy, &m_iAttackTick1, &m_iAttackTick2, &iAtkPatten, m_ulStartTimeSave);
		iEnemyAriveCount++;
		list.push_back(&enemy[i]);	// ���� ���� ���� �� List�� ����. �̰� 50�� �ݺ�.
	}

	// ---------------------------
	//	�̻��� ���� ����
	// ---------------------------
	for (int i = 0; i < ENEMY_MISSALE_COUNT + PLAYER_MISSALE_COUNT; i++)
	{
		if (i < ENEMY_MISSALE_COUNT)
			missale[i].InfoSetFunc(false);	// false�� ���� �̻���
		else
			missale[i].InfoSetFunc(true);	// true�� �÷��̾� �̻���

		list.push_back(&missale[i]);		// ������ �� ������ �ش� �̻����� ����Ʈ�� ����. �̰� 100�� �ݺ�.
	}

}
// run �Լ�
void CSceneGame::run()
{
	if (player->GetArive())	// ������ ����ִٸ�.
	{
		timeBeginPeriod(1);
		m_ulStartTimeSave = GetTickCount64();	// �������� ����ϱ� ����, ���� �ð� ����		
		GetListData();	// �÷��̾�, ����, �̻����� ����Ʈ���� ������		
		BufferClear();	// ����� �ʱ�ȭ

		// -----------------------
		//		Ű���� üũ
		// -----------------------
		player->KeyDownCheck();

		// -----------------------
		//		    ����
		// -----------------------	
		Moverun();					// ���� �� ���� �̵� ����
		Atkrun();					// ���� �� ���� ���� ����

		// -----------------------
		//		  ������
		// -----------------------	
		UIRendering();	// UI ����(�� �׵θ�, HP, �ð�)
		//// ȭ�� �׸���. �̶� UI�� ������ ��� ��ü(����� �÷��̾�� ����)�� �׷�����. ////
		for (int i = 0; i < HEIGHT; i++)
			BufferFlip(i, 0);

		UploadListData();		// �÷��̾�, ����, �̻����� ����Ʈ�� �ø�
		Sleep(50);
		timeEndPeriod(1);

		if (iEnemyAriveCount == 0)
			hSceneHandle.LoadScene(Victory);
	}
	else
		hSceneHandle.LoadScene(Lose);
}
// run �Լ� �ȿ��� ȣ��ȴ�. �̵� ������ ����ϴ� �Լ�
void CSceneGame::Moverun()
{
	//���� �̵� ���� (������ �������� ���� ���� ����)
	player->PlayerMoveFunc();

	// ���� �̵� ����
	for (int i = 0; i < ENEMY_COUNT; i++)
	{
		if (enemy[i].GetArive())	// ����ִ� ���ʹ�
			enemy[i].EnemyMoveFunc(&iEnemyAIState, m_ulStartTimeSave, &bAiActiveCheck);	// AI�� ���� �̵�
	}

	//// ���� �̵� AI�� �۵��ߴٸ�
	if (bAiActiveCheck)
	{
		// AI�� ������ ���ϱ��� ������ �ٽ� ó�� ������ �ؾ��ϴ� ó������ �����ش�.
		if (iEnemyAIState == 3)
			iEnemyAIState = 0;
		else
			iEnemyAIState++;	// �װ� �ƴ϶�� �׳� 1 ����.

								// AI �۵� ���θ� ��������(False)�� ����
		bAiActiveCheck = false;
	}

}
// run �Լ� �ȿ��� ȣ��ȴ�. ���� ������ ����ϴ� �Լ�
void CSceneGame::Atkrun()
{
	//���� ���� ���� //
	player->MissaleMoveFunc(missale, enemy, &iEnemyAriveCount);			// �̻��� �̵� �� �Ҹ�ó��.
	player->MissaleCreateFunc(missale);									// ���ο� �̻��� �����Ұ� ������ ���� ó��.
	player->BonusMissaleFunc(missale, m_ulStartTimeSave);				// ���ʽ� �̻��� ���� �� �̵�ó��

	// ���� ���� ����//
	// Enemy ���� ��ť ����
	for (int i = 0; i < ENEMY_COUNT; i++)
	{
		// ����ִ� ������ ���� �Ʒ� ���� ����
		if (enemy[i].GetArive())
		{
			enemy[i].EnemyEnqueueFunc(m_ulStartTimeSave);
		}
	}

	// ���� ��� �̻��� �̵� �� �Ҹ� ó��
	enemy[0].EnemyMissaleMoveFunc(missale, player);	// �ش� ������ CMissale�� �ִ°� �´°Ͱ�����... �̹� ���⿡ ������. �׳� 0�� �ϳ��� ȣ��

	// Enemy ���� ��ť ����
	for (int i = 0; i < ENEMY_COUNT; i++)
	{
		// ����ִ� ������ ���� �Ʒ� ���� ����
		if (enemy[i].GetArive())
		{
			enemy[i].EnemyDequeueFunc(missale);
		}
	}

}
// run �Լ� �ȿ��� ȣ��ȴ�. UI ����
void CSceneGame::UIRendering()
{
	//// �� �׵θ��� �����Ѵ�. ////
	for (int i = 3; i < UI_HEIGHT; i++)
	{
		if (i == 3 || i == UI_HEIGHT - 1)
			for (int j = 10; j < 71; ++j)
				cBackBuffer[i][j] = '=';
		else
		{
			cBackBuffer[i][10] = 'i';
			cBackBuffer[i][70] = 'i';
		}
	}

	//// HP UI ���� ////
	memcpy(cBackBuffer[28] + 38, g_Hp, sizeof(g_Hp) - 1);
	memcpy(cBackBuffer[28] + 43, player->GetHP(), 2);

	//// Time UI ���� ////
	// ���� �ð��� �޾ƿ´�. UITime ǥ�� üũ�� ����.
	ulUITimeNow = m_ulStartTimeSave;

	// Time : ��� ���ڿ� ����.
	memcpy(cBackBuffer[1] + 35, g_Time, sizeof(g_Time) - 1);

	// ������ �ð� üũ���� 1�ʰ� ������, ǥ�õǴ� �ð� ����.
	if (ulUITimeSave + 1000 <= ulUITimeNow)
	{
		// ���� 9�ʶ��(���� 10�ʰ� �� ���ʶ��)
		if (d == '9')
		{
			// 3��° �ڸ� üũ. 
			// 3���� �ڸ��� 5�� �ƴ϶�� ���� 59�ʰ� �ȵƴٴ� �Ŵϱ� 3��° �ڸ��� �ʸ� 1 ����. �׸��� 4��° �ڸ� �ʸ� 0���� ����.
			if (c != '5')
			{
				c++;
				d = '0';
			}
			// 3��° �ڸ��� 5��� 
			else
			{
				// 2��° �ڸ� üũ
				// 3��° �ڸ��� 5�ε�, 2��° �ڸ��� 9�� �ƴ϶��, 02:59(2�� 59��) �� �����̴�, 2��° �ڸ��� �ϳ� ������Ű�� �ʸ� ��� 0���� ����
				if (b != '9')
				{
					b++;
					c = '0';
					d = '0';
				}
				// 3��° �ڸ��� 5�ε�, 2��° �ڸ��� 9��� 09:59(9�� 59��) �̴� ù ��° �ڸ��� �ϳ� ������Ű�� ��θ� 0���� ����.
				else
				{
					a++;
					b = '0';
					c = '0';
					d = '0';
				}
			}
		}
		// �ڸ����� �ٲ� �ʿ䰡 ������ �׳� �ʸ� ����.
		else
			d++;

		// ���� �ð� üũŸ���� �˾ƾ��ϴ� ���� �ð� ����.
		ulUITimeSave = ulUITimeNow;
	}
	Time[0] = a;
	Time[1] = b;
	Time[2] = ':';
	Time[3] = c;
	Time[4] = d;

	// Time���ڿ� ����. ��������� ����� ���� ������װ� ����Ȱ� ������ �׳� ���� ���� ����ɰ���. ���� ��� ����ִ� ���� �����������.
	memcpy(cBackBuffer[1] + 42, Time, sizeof(Time) - 1);

}
// run �Լ� �ȿ��� ȣ��ȴ�. ���� �����͸� ����Ʈ���� �������� ����
void CSceneGame::GetListData()
{
	// �÷��̾�, ����, �̻����� ��� ����Ʈ���� ������
	// �÷��̾ ������. ������ ��, �ش� �÷��̾�� �����. �ֳ��ϸ�, ���� �� Run �Լ� �������� �ٽ� ���. 
	CList<CObject*>::iterator itor1;
	for (itor1 = list.begin(); itor1 != list.end(); itor1++)
	{
		player = (CPlayer*)*itor1;
		if (player->Type == 1)
			break;
	}
	list.erase(itor1);

	// ���͸� ������
	CList<CObject*>::iterator itor2;
	int i = 0;

	for (itor2 = list.begin(); itor2 != list.end();)
	{
		if (i == ENEMY_COUNT)
			break;
		enemy[i] = (CEnemy*)(*itor2);
		if (enemy[i].Type == 2)	// ���͸� ��󳽴�.
		{
			i++;					// ���Ͷ��, ���� �迭�� �غ��Ѵ�.
			list.erase(itor2);		// �׸���, ����Ʈ���� �����Ѵ�. �ֳ��ϸ�, ���� �� �ٽ� ����ϱ� ���ؼ�.
		}
		else                    // ���͸� ��ã�Ҵٸ�, itor�� ++�� �� ���� ����Ʈ�� ã�ƿ´�.
			itor2++;
	}

	// �̻����� ������
	// �Ʊ�, ���� ���� ���� ��� �̻����� �����´�.
	CList<CObject*>::iterator itor3;
	i = 0;

	for (itor3 = list.begin(); itor3 != list.end();)
	{
		if (i == PLAYER_MISSALE_COUNT + ENEMY_MISSALE_COUNT)
			break;
		missale[i] = (CMissale*)*itor3;
		if (missale[i].Type == 3)	// �̻����� ��󳽴�.
		{
			i++;					// �̻����̶�� ���� �迭�� �غ��Ѵ�.
			list.erase(itor3);		// �׸���, ����Ʈ���� �����Ѵ�. �ֳ��ϸ�, ���� �� �ٽ� ����ϱ� ���ؼ�.
		}
		else						// �̻����� ��ã�Ҵٸ�, itor�� ++�� �� ���� ����Ʈ�� ã�ƿ´�.
			itor3++;
	}

}
// run �Լ� �ȿ��� ȣ��ȴ�. ���� �����͸� ����Ʈ�� �ִ� ����
void CSceneGame::UploadListData()
{
	// -----------------------------------------
	//	�÷��̾�, ����, �̻����� ����Ʈ�� �߰�
	// -----------------------------------------
	list.push_front(player);	// �÷��̾�

	for (int i = 0; i < ENEMY_COUNT; i++)	// ����
	{
		list.push_back(&enemy[i]);
	}

	for (int i = 0; i < PLAYER_MISSALE_COUNT + ENEMY_MISSALE_COUNT; i++)	// �̻���
	{
		list.push_back(&missale[i]);
	}

}
// �Ҹ���
CSceneGame::~CSceneGame()
{
	delete player;
	delete[] enemy;
	delete[] missale;
}


// --------------------------
// CSceneLose Ŭ���� ����Լ�
// --------------------------
void CSceneLose::run()
{
	BufferClear();	// ����� �ʱ�ȭ
	memcpy(cBackBuffer[10] + 38, LOSE_TEXT, sizeof(LOSE_TEXT) - 1);
	for (int i = 0; i < HEIGHT; i++)
		BufferFlip(i, 0);

	Sleep(1500);
	fputs("\n", stdout);
	exit(-1);	
}

// --------------------------
// CSceneVictory Ŭ���� ����Լ�
// --------------------------

void CSceneVictory::run()
{
	BufferClear();	// ����� �ʱ�ȭ
	memcpy(cBackBuffer[10] + 38, VICTORY_TEXT, sizeof(VICTORY_TEXT) - 1);
	for (int i = 0; i < HEIGHT; i++)
		BufferFlip(i, 0);

	Sleep(1500);
	fputs("\n", stdout);
	exit(-1);

}


// --------------------------
// CSceneHandle Ŭ���� ����Լ�
// --------------------------
// ������
CSceneHandle::CSceneHandle()
{
	m_NowScene = new CSceneTitle;
	m_NowSceneText = Title;
}
// CSceneHandle::run �Լ�
void CSceneHandle::run()
{
	if (m_eNextScene != None)
	{
		delete m_NowScene;
		if (m_NowSceneText != Select)
		{
			CList<CObject*>::iterator itor;
			for (itor = list.begin(); itor != list.end();)
			{
				itor = list.erase(itor);
			}
		}
		
		switch (m_eNextScene)
		{
		case Title:
			m_NowScene = new CSceneTitle;
			m_NowSceneText = m_eNextScene;
			break;
		case Select:
			m_NowScene = new CSceneSelect;
			m_NowSceneText = m_eNextScene;
			break;
		case Ingame:
			m_NowScene = new CSceneGame;
			m_NowSceneText = m_eNextScene;
			break;
		case Lose:
			m_NowScene = new CSceneLose;
			m_NowSceneText = m_eNextScene;
			break;
		case Victory:
			m_NowScene = new CSceneVictory;
			m_NowSceneText = m_eNextScene;
			break;
		}
	}
	m_eNextScene = None;	
	m_NowScene->run();
}
// CSceneHandle::�� ���� �Լ�
void CSceneHandle::LoadScene(SceneType temp)
{
	m_eNextScene = temp;
}
// CSceneHandle::�Ҹ���
CSceneHandle::~CSceneHandle()
{
	delete m_NowScene;
}