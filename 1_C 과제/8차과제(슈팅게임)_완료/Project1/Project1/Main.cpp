// ���ð��� ����
#include <stdio.h>
#include <string.h>
#include <Windows.h>
#include <mmsystem.h>
#include "MoveQueue.h"
#include <conio.h>

HANDLE hConsole;

#define HEIGHT 30
#define WIDTH 81
#define UI_HEIGHT 27
#define UI_WIDTH 81
#define ENEMY_COUNT 50
#define ENEMY_MISSALE_COUNT 50
#define PLAYER_MISSALE_COUNT 50
#define ENEMY_AI_TIME 1400
#define PLAYER_AI_ATACK_TIME 2000

// �������� ����
int g_iState = 0;
char cBackBuffer[HEIGHT][WIDTH];
char g_cTitle[] = "TITLE";
char g_cStart[] = "START";
char g_cExit[] = "EXIT";
char g_cTip[] = "�� ZŰ�� �����ּ��� ��";
char g_cCharSeletTip[] = "ĳ���͸� ������ �ּ���";
char g_cCharSeletChaTip_1[] = "(ü��: 1  / �߰��̻���: 2�ʿ� 2��)";
char g_cCharSeletChaTip_2[] = "(ü��: 2  / �߰��̻���: 2�ʿ� 1��)";
char g_cCharSeletChaTip_3[] = "(ü��: 5  / �߰��̻���:   ����   )";
char g_Hp[] = "HP : ";
char g_Time[] = "Time : ";

// �÷��̾� ����ü
typedef struct
{
	int m_iPosX, m_iPosY;
	bool m_bAriveCheck;
	Queue queue;
	Queue Atkqueue;
	char m_cLook;
	char m_cMissaleLook;
	char m_cHP[2];
	int m_iCharSpeed;
	int m_iBonusMIssaleCount;
	ULONGLONG m_BonusMIssaleTImeSave;

}Player;

// ���� ����ü
typedef struct
{
	int m_iPosX, m_iPosY;
	bool m_bAriveCheck;
	int m_iAttackTick;
	ULONGLONG m_AttackTimeSave;

}Enemy;

// �̻��� ����ü
typedef struct
{
	int m_iPosX, m_iPosY;
	bool m_PlayerOrEnemy;
	bool m_bAriveCheck;
	bool m_BonusOrNot;
}Missale;


// �� �ѹ�
enum
{
	e_Title = 0, e_ChaSelect, e_Ingame, e_Exit
};

// �ܼ� ��� ���� �غ� �۾�
void cs_Initial(void);

// �ܼ� ȭ���� Ŀ���� x,y ��ǥ�� �̵�
void cs_MoveCursor(int iPosy, int iPosx);

// ���� �ø� �Լ�
void BufferFlip(int iPosy, int iPosx);

// Title �Լ�
void Title(int* iArrowShowPosy, bool* bArrowShowFlag);

// ĳ���� ���� �Լ�
void CharSeleet(int* iArrowShowPosy, bool* bArrowShowFlag);

// ���� �ʱ�ȭ �Լ�
void BufferClear();

int main()
{
	system("mode con cols=83 lines=33");

	bool bArrowShowFlag = true;	//Ÿ��Ʋ ȭ�鿡�� ȭ��ǥ �������� �����ϴ� ����. ture�� ������ / flase�� �Ⱥ�����. Ʈ���޽� �ݺ��ϸ鼭 ���������δ�.
	int iArrowShowPosy = 5;	// Ÿ��Ʋ ȭ�鿡�� ȭ��ǥ�� ǥ�õ� ��ġ. 5�� Ingame / 6�̸� Exit�� ǥ�õȴ�.
	int iTempPosx;	// �ӽ� x��ġ ����
	int iTempPosy;	// �ӽ� y��ġ ����
	int iEnemyAIState = 0; // ������ �̵� ����.
	ULONGLONG ulEnemyAiTimeSave = 0;	// Enemy�� ���� ���� �ð� üũ.
	ULONGLONG ulEnemyAiTimeCur = 0;	// Enemy�� ���� ���� �ð� üũ. ���� �ð� üũ
	bool bAiActiveCheck = false;	// Enemy�� AiȰ��ȭ üũ.
	int iPlayerMissaleCount = 0;	// ���� ����ִ�(ȭ�鿡 ��������) �÷��̾��� �̻��� ���� ī��Ʈ
	int iEnemyAriveCount = ENEMY_COUNT;	// ����ִ� Enemy �� üũ. ���Ͱ� ��� ������ ���� ����
	Queue MonsterAtkQueue;	// ������ ���� ����� �����ϴ� ť

	Init(&MonsterAtkQueue);

	// ---------------------------
	//	 �ð� UI�� ���� ���� ����
	// ---------------------------
	char Time[6];
	Time[5] = '\0';
	char a = '0';
	char b = '0';
	char c = '0';
	char d = '0';
	ULONGLONG ulUITimeSave = 0;	// UI�� ǥ�õǴ� �ð��� üũ�ϱ� ���� Ÿ�̸�.

	// -----------------------
	//	  �÷��̾� ���� ����
	// -----------------------
	// �÷��̾� ����ü ����
	Player player;	
	player.m_iPosX = 40;
	player.m_iPosY = 20;
	player.m_bAriveCheck = true;

	// �÷��̾� �ൿ ���� ť, ���� ���� ť �ʱ�ȭ
	Init(&player.queue);
	Init(&player.Atkqueue);

	// -----------------------
	//		���� ���� ����
	// -----------------------
	// ���� ����ü �迭 ����. ���� 50�����̴�! (ENEMY_COUNT�� 50)
	Enemy enemy[ENEMY_COUNT]; 
	int iEnemyPosx = 17;
	int iEnemyPosy = 5;
	iTempPosx = 1400;
	iTempPosy = 1000;
	iArrowShowPosy = 0;
	for (int i = 0; i < ENEMY_COUNT; ++i)
	{
		enemy[i].m_iPosX = iEnemyPosx;
		enemy[i].m_iPosY = iEnemyPosy;
		enemy[i].m_iAttackTick = iTempPosx;
		enemy[i].m_bAriveCheck = true;
		
		// �Ʒ� if���� ������ ���� AI ƽ ����
		if (iArrowShowPosy == 4)
		{
			iTempPosx = iTempPosy;
			iArrowShowPosy = 0;
			iTempPosy += 600;
		}
		else
		{
			iTempPosx += 1500;
			iArrowShowPosy++;
		}

		// �Ʒ� if���� ������ ���� ���� ��ġ ����
		if (iEnemyPosx > 60)
		{
			iEnemyPosx = 17;
			iEnemyPosy += 1;
		}
		else
			iEnemyPosx += 5;
	}

	iArrowShowPosy = 5;	//��� ���� ���� �ٽ� ����..

	// -----------------------
	//	  �̻��� ����ü ����
	// -----------------------
	//�迭 0 ~ 49�� ���� ��. �迭 50 ~ 99���� �÷��̾��� ��
	Missale missale[ENEMY_MISSALE_COUNT + PLAYER_MISSALE_COUNT];

	// �̻����� ������ �Ʊ�/���������� ����. Ʈ��� �Ʊ� / �޽��� ����
	for (int i = 0; i < ENEMY_MISSALE_COUNT + PLAYER_MISSALE_COUNT; ++i)	
	{
		if (i < ENEMY_MISSALE_COUNT)
			missale[i].m_PlayerOrEnemy = false;	// �޽��� ����
		else
			missale[i].m_PlayerOrEnemy = true;	// Ʈ��� �Ʊ�
	}

	// �̻����� ���� ���� ����. ó�� ���۶��� ��� ��Ȱ��ȭ����.
	for (int i = 0; i < ENEMY_MISSALE_COUNT + PLAYER_MISSALE_COUNT; ++i)
		missale[i].m_bAriveCheck = false;


	// ȭ���� Ŀ�� �Ⱥ��̰� ó�� �� �ڵ� ����
	cs_Initial();
		
	timeBeginPeriod(1);
	while (1)
	{
		switch (g_iState)
		{
		case e_Title:
			// ����� �ʱ�ȭ
			BufferClear();

			// Ÿ��Ʋ â ǥ�� �Լ� ȣ��
			Title(&iArrowShowPosy, &bArrowShowFlag);
			Sleep(200);

			// �������� �ϱ� ���� ����
			if (bArrowShowFlag == true)
				bArrowShowFlag = false;
			else
				bArrowShowFlag = true;
			

			// ������ ZŰ�� ������ ��, ���� ȭ��ǥ�� ��� ����Ű�� �ִ����� ���� ���� ���� �ΰ������� EXIT ���� ����
			if (GetAsyncKeyState(0x5A) & 0x8001)
				if (iArrowShowPosy == 5)
				{
					g_iState = e_ChaSelect;
					break;
				}
				else if(iArrowShowPosy == 6)
				{
					g_iState = e_Exit;
					break;
				}
			// ������ Ű���� ��/�Ʒ��� �����ϸ�, ������ ��ư�� ���� iArrowShowPosy�� ���� �ٲ���. ��, ȭ��ǥ�� �� �Ʒ��� ��� ����ų�� ����
			if (GetAsyncKeyState(VK_DOWN) & 0x8001)
				iArrowShowPosy = 6;
			if(GetAsyncKeyState(VK_UP) & 0x8001)
				iArrowShowPosy = 5;

			break;
		case e_ChaSelect:
			// ����� �ʱ�ȭ
			BufferClear();

			// ĳ���� ����â ǥ�� �Լ� ȣ��
			CharSeleet(&iArrowShowPosy, &bArrowShowFlag);
			Sleep(200);

			// �������� �ϱ� ���� ����
			if (bArrowShowFlag == true)
				bArrowShowFlag = false;
			else
				bArrowShowFlag = true;

			// ������ ZŰ�� ������ ��, ���� ȭ��ǥ�� ��� ����Ű�� �ִ����� ���� ĳ���� ���� ����
			if (GetAsyncKeyState(0x5A) & 0x8001)
			{
				if (iArrowShowPosy == 5)
				{
					player.m_cLook = 'A';
					player.m_cMissaleLook = '1';
					player.m_cHP[0] = '0';
					player.m_cHP[1] = '1';
					player.m_iCharSpeed = 1;
					player.m_iBonusMIssaleCount = 2;
				}
				else if (iArrowShowPosy == 7)
				{
					player.m_cLook = 'B';
					player.m_cMissaleLook = '2';
					player.m_cHP[0] = '0';
					player.m_cHP[1] = '2';
					player.m_iCharSpeed = 1;
					player.m_iBonusMIssaleCount = 1;
				}
				else if (iArrowShowPosy == 9)
				{
					player.m_cLook = 'V';
					player.m_cMissaleLook = '3';
					player.m_cHP[0] = '0';
					player.m_cHP[1] = '5';
					player.m_iCharSpeed = 1;
					player.m_iBonusMIssaleCount = 0;
				}

				g_iState = e_Ingame;
				break;
			}

			// ������ Ű���� ��/�Ʒ��� �����ϸ�, ������ ��ư�� ���� iArrowShowPosy�� ���� �ٲ���. ��, ȭ��ǥ�� ��, �߰�, �Ʒ� �� ��� ����ų�� ����
			if (GetAsyncKeyState(VK_DOWN) & 0x8001)
			{
				if(iArrowShowPosy != 9)
					iArrowShowPosy += 2;
			}
			if (GetAsyncKeyState(VK_UP) & 0x8001)
			{
				if (iArrowShowPosy != 5)
					iArrowShowPosy -= 2;
			}				

			break;
		case e_Ingame:	
			// ȭ�� ���� ��, Ȥ�� �𸣴�, player�� �������·� �����.
			player.m_bAriveCheck = true;

			// Ȥ�� �𸣴� ��� ���� �������·� �����.
			for (int i = 0; i < ENEMY_COUNT; ++i)
				enemy[i].m_bAriveCheck = true;

			// �� ���� ���� ���� �ð� üũ. ���� AI �۵��� ���� �ð� üũ
			ulEnemyAiTimeSave = GetTickCount64();

			// �� ������ �����ϸ�, ���� �ð� ����. ������ ���� AI �۵��� ���� �ð� üũ
			for (int i = 0; i < ENEMY_COUNT; ++i)
				enemy[i].m_AttackTimeSave = ulEnemyAiTimeSave;

			// �� ������ �����ϸ�, ���� �ð� ����. �Ʊ� ĳ������ �߰� �̻��� �߻縦 ����.
			player.m_BonusMIssaleTImeSave = ulEnemyAiTimeSave;

			// �� ������ �����ϸ�, ���� �ð� ����. UI�� �ð�ǥ�ø� ����
			ulUITimeSave = ulEnemyAiTimeSave;

			while (1)
			{
				// ����� �ʱ�ȭ
				BufferClear();

				// -----------------------
				//		Ű���� üũ
				// -----------------------
				//������ �������� ���� Ű���� üũ
				if (player.m_bAriveCheck)	
				{
					// ������ ���� Ű�� üũ��, �ൿ ������ ť�� ����. ������ �ൿ ������ '����'���� �۵�
					if (GetAsyncKeyState(VK_UP) & 0x8001)
					{
						if(player.m_iPosY -1 > (UI_HEIGHT /2))
							Enqueue(&player.queue, player.m_iPosX, player.m_iPosY - player.m_iCharSpeed);
					}
					if (GetAsyncKeyState(VK_DOWN) & 0x8001)
					{
						if(player.m_iPosY +1 < UI_HEIGHT -1)
							Enqueue(&player.queue, player.m_iPosX, player.m_iPosY + player.m_iCharSpeed);
					}
					if (GetAsyncKeyState(VK_RIGHT) & 0x8001)
					{
						if(player.m_iPosX +1 < 69)
							Enqueue(&player.queue, player.m_iPosX + player.m_iCharSpeed, player.m_iPosY);
					}
					if (GetAsyncKeyState(VK_LEFT) & 0x8001)
					{
						if(player.m_iPosX -1 > 11)
							Enqueue(&player.queue, player.m_iPosX - player.m_iCharSpeed, player.m_iPosY);
					}

					// ������ ���� ��ư�� ��������,���� ������ ť�� ����. ������ ���� ������ '����'���� �۵�
					if (GetAsyncKeyState(0x5A) & 0x8001)
					{
						if (iPlayerMissaleCount < PLAYER_MISSALE_COUNT)
						{
							iPlayerMissaleCount++;
							Enqueue(&player.Atkqueue, player.m_iPosX, player.m_iPosY - 1);							
						}
					}
				}

				// -----------------------
				//		    ����
				// -----------------------				

				////���� �̵� ���� (������ �������� ���� ���� ����) ////
				if (player.m_bAriveCheck)
				{
					// ���� ������ ��ġ�� ���ۿ� �����Ѵ�.
					cBackBuffer[player.m_iPosY][player.m_iPosX] = player.m_cLook;

					// ���� ������ x,y ��ġ�� �����Ѵ�.
					iTempPosx = player.m_iPosX;
					iTempPosy = player.m_iPosY;

					// �ൿ ������ �޾Ƽ�(ť), �̵��� ��ġ�� ������ �� ��ġ�� �̵���Ų��.
					while (Dequeue(&player.queue, &player.m_iPosX, &player.m_iPosY))
					{
						cBackBuffer[iTempPosy][iTempPosx] = ' ';	//��ť�� �����ϸ�, ������ ������ �ִ� ��ġ�� �����̽��� �����Ѵ�.
						cBackBuffer[player.m_iPosY][player.m_iPosX] = player.m_cLook;

						iTempPosx = player.m_iPosX;	// ���� while������ ��ť�� ������ �� ������, ��ť ���� ��ġ�� �����Ѵ�.
						iTempPosy = player.m_iPosY;
					}
				}

				//// Enemy �̵� AI ���� (���� ������ Enemy�� �������� ���� ����)	////
				// ���� ���� ��, ���� �ð��� �޾ƿ´�. 1.4��(ENEMY_AI_TIME)���� ����ִ� ���� AI ���Ͽ� ���� �����δ�.
				ulEnemyAiTimeCur = GetTickCount64();

				for (int i = 0; i < ENEMY_COUNT; ++i)
				{
					//  ����ִ� ������ ���� �Ʒ� ���� ����
					if (enemy[i].m_bAriveCheck == true)
					{
						// ���� AI�� �۵����� 1.4��(ENEMY_AI_TIME)�� �� ���
						if (ulEnemyAiTimeSave + ENEMY_AI_TIME <= ulEnemyAiTimeCur)
						{
							if (iEnemyAIState == 0)
								enemy[i].m_iPosX -= 2;
							else if (iEnemyAIState == 1)
								enemy[i].m_iPosY--;
							else if (iEnemyAIState == 2)
								enemy[i].m_iPosX += 2;
							else if (iEnemyAIState == 3)
								enemy[i].m_iPosY++;

							// AI�� �۵��ߴٴ� �÷��� on. ������ ���� or ù ���� �� �����ؼ� üũ�ϸ�, �ش� ���� �׾��� �� ������ �߻��ϴ� �׳� �߻��� �� ���� �÷��� ��
							bAiActiveCheck = true;
						}
						cBackBuffer[enemy[i].m_iPosY][enemy[i].m_iPosX] = 'M';
					}
				}

				// Enemy �̵� AI�� �۵��ߴٸ�
				if (bAiActiveCheck)
				{
					// �ð��� ���� �����Ѵ�. �׷��� ���� AI�� �۵���ų ������ �Ǳ� ������.
					ulEnemyAiTimeSave = GetTickCount64();

					// AI�� ������ ���ϱ��� ������ �ٽ� ó�� ������ �ؾ��ϴ� ó������ �����ش�.
					if (iEnemyAIState == 3)
						iEnemyAIState = 0;
					else
						iEnemyAIState++;

					// AI �۵� ���θ� ��������(False)�� ����
					bAiActiveCheck = false;
				}				

				//// ������ �̻��� �̵� �� �Ҹ� ���� (������ �������� ���� ���� ����)////		
				if (player.m_bAriveCheck)
				{
					// ����ִ� ��� �̻����� -y������ 1ĭ �̵� �� ���ۿ� ����.		
					for (int i = ENEMY_MISSALE_COUNT; i < ENEMY_MISSALE_COUNT + PLAYER_MISSALE_COUNT; ++i)
					{
						if (missale[i].m_bAriveCheck == true)
						{
							missale[i].m_iPosY--;

							// -y������ 1ĭ �̵��ߴµ�, �װ� ���̸� false�� ����.
							if (missale[i].m_iPosY == 3)
							{
								missale[i].m_bAriveCheck = false;
								iPlayerMissaleCount--;
							}

							// -y������ 1ĭ �̵��ߴµ�, �װ� ���͸� false�� ����� �ش� ��ġ�� ���͸� false�� ����(��� ����)
							// flase�� �� ���ʹ� �ڵ����� ���ۿ��� ���ܵȴ�.
							else if (cBackBuffer[missale[i].m_iPosY][missale[i].m_iPosX] == 'M')
							{
								int j = 0;
								for (; j < ENEMY_COUNT; ++j)
								{
									if (enemy[j].m_iPosY == missale[i].m_iPosY && enemy[j].m_iPosX == missale[i].m_iPosX)
									{
										// �ش� ���Ͱ� ����ִ� ���Ͷ��, (Ȥ�� �𸣴� ���⼭ �������� �ٽ��ѹ� üũ. �ڲ� �̻��Ѱ� 1~2���� ���Ҵµ� ������ �����..)
										if (enemy[j].m_bAriveCheck == true)
										{

											enemy[j].m_bAriveCheck = false;
											iEnemyAriveCount--;	// ���� ���� Enemy �� 1��ü ����
											if (iEnemyAriveCount == 0)
												cBackBuffer[enemy[j].m_iPosY][enemy[j].m_iPosX] = ' ';	// ���� 0���� �Ǹ�, ������ ���� �����̽��� �����. �׷��� ȭ�鿡�� ����� �� ó�� ����.

											missale[i].m_bAriveCheck = false;
											iPlayerMissaleCount--;	// ���� ���� �̻��� �� 1�� ����		
											break;
										}
									}
								}								

							}
							else
							{
								if(missale[i].m_BonusOrNot == false)
									cBackBuffer[missale[i].m_iPosY][missale[i].m_iPosX] = player.m_cMissaleLook;
								else
									cBackBuffer[missale[i].m_iPosY][missale[i].m_iPosX] = 'b';
							}
						}
					}
				}

				//// ���� ���� ���� ////
				// ���� ���� ��, �ð��� �޾ƿ´�.
				ulEnemyAiTimeCur = GetTickCount64();

				if (player.m_bAriveCheck)
				{
					int i = PLAYER_MISSALE_COUNT;
					// ���� ������ �޾Ƽ�(ť), �ش� ��ǥ�� �̻����� Ȱ��ȭ��Ų��.
					while (Dequeue(&player.Atkqueue, &iTempPosx, &iTempPosy))
					{						
						// ���� false�� �Ʊ� �̻����� ã�´�.
						while (missale[i].m_bAriveCheck)
						{
							// ����, i�� �̻����� ������ ������� break;
							if (i == ENEMY_MISSALE_COUNT + PLAYER_MISSALE_COUNT)
								break;

							// �װ� �ƴ϶�� ���� �迭 �˻�.
							i++;
						}

						// i�� ����, ENEMY_MISSALE_COUNT + PLAYER_MISSALE_COUNT���� ������ false�� �迭�� ã�Ҵٴ� ���̴� �Ʒ� ���� ����
						if (i < ENEMY_MISSALE_COUNT + PLAYER_MISSALE_COUNT)
						{
							// ã�� ��ġ�� �̻����� true�� �����, ã�� ��ġ�� x,y���� �����Ѵ�.
							missale[i].m_bAriveCheck = true;
							missale[i].m_BonusOrNot = false;	//���ʽ� ������ üũ�ϱ� ���Ѱ�. ���ʽ��� true / ���ʽ� �̻����� �ƴϸ� false
							missale[i].m_iPosX = iTempPosx;
							missale[i].m_iPosY = iTempPosy;

							// �׸���, ������ �̻����� ���ۿ� �����Ѵ�.
							cBackBuffer[missale[i].m_iPosY][missale[i].m_iPosX] = player.m_cMissaleLook;
						}
					}

					
					// ������ ���� �Է°��� ��� ����, ���� �ð����� ���ʽ� �̻����� �߻�ȴ�.
					// ����, ������ ���� �ƴٸ�,
					if (player.m_BonusMIssaleTImeSave + PLAYER_AI_ATACK_TIME <= ulEnemyAiTimeCur)
					{
						i = PLAYER_MISSALE_COUNT;
						
						// ���� �÷��̾ ������ �ִ� �߰� �̻��� �߻� ī��Ʈ��ŭ �ݺ��ϸ� �̻��� ����
						// �߰� �̻��� �߻� ī��Ʈ�� 0�̶��, �ݺ����� �ʴ´�.
						for (int h = 1; h <= player.m_iBonusMIssaleCount; ++h)
						{
							// ���� false�� �Ʊ� �̻����� ã�´�.
							while (missale[i].m_bAriveCheck)
							{
								// ����, i�� �̻����� ������ ������� break;
								if (i == ENEMY_MISSALE_COUNT + PLAYER_MISSALE_COUNT)
									break;

								// �װ� �ƴ϶�� ���� �迭 �˻�.
								i++;
							}
							// i�� ����, ENEMY_MISSALE_COUNT + PLAYER_MISSALE_COUNT���� ������ false�� �迭�� ã�Ҵٴ� ���̴� �Ʒ� ���� ����
							if (i < ENEMY_MISSALE_COUNT + PLAYER_MISSALE_COUNT)
							{
								// ã�� ��ġ�� �̻����� true�� �����, �̻����� �����Ѵ�.
								missale[i].m_bAriveCheck = true;
								missale[i].m_BonusOrNot = true;
								if (h == 1)
								{
									missale[i].m_iPosX = player.m_iPosX - 2;
									missale[i].m_iPosY = player.m_iPosY - 1;
									cBackBuffer[missale[i].m_iPosY][missale[i].m_iPosX] = 'b';
								}
								else if (h == 2)
								{
									missale[i].m_iPosX = player.m_iPosX + 2;
									missale[i].m_iPosY = player.m_iPosY - 1;
									cBackBuffer[missale[i].m_iPosY][missale[i].m_iPosX] = 'b';
								}								
							}
						}
						// �̻����� ��� ����������, ���� �ð��� �����Ѵ�. �׷��� ������ �� �߰� �̻����� �߻��ϴϱ�..
						player.m_BonusMIssaleTImeSave = ulEnemyAiTimeCur;
					}					
				}				

				//// Enemy ���� AI ���� (���� ������ Enemy�� �������� ���� ����). �� �̻����� �����Ѵ�.	////
				// ���� ���� ��, �ð��� �޾ƿ´�.
				ulEnemyAiTimeCur = GetTickCount64();
				
				// Enemy ���� ť �ֱ� ����
				for (int i = 0; i < ENEMY_COUNT; ++i)
				{
					// ����ִ� ������ ���� �Ʒ� ���� ����
					if (enemy[i].m_bAriveCheck == true)
					{
						// �ش� ������ ������ ���� ������, ���� ����� ť�� ����
						if (ulEnemyAiTimeCur >= enemy[i].m_AttackTimeSave + enemy[i].m_iAttackTick)
						{
							Enqueue(&MonsterAtkQueue, enemy[i].m_iPosX, enemy[i].m_iPosY + 1);
							enemy[i].m_AttackTimeSave = ulEnemyAiTimeCur;	// ������ ����ü ������ ���� �ð� ����.
						}
					}
				}


				//// Enemy �̻��� �̵� �� �Ҹ� ���� (���� ������ ��� ���� ���� ����)	////
				// ����ִ� ��� �̻����� -y������ 1ĭ �̵� �� ���ۿ� ����.		
				for (int i = 0; i < ENEMY_MISSALE_COUNT; ++i)
				{
					if (missale[i].m_bAriveCheck == true)
					{
						missale[i].m_iPosY++;

						// +y������ 1ĭ �̵��ߴµ�, �װ� ���̸� false�� ����.
						if (missale[i].m_iPosY == UI_HEIGHT - 1)
							missale[i].m_bAriveCheck = false;

						// +y������ 1ĭ �̵��ߴµ�, �װ� ������.  
						else if (cBackBuffer[missale[i].m_iPosY][missale[i].m_iPosX] == player.m_cLook)
						{
							// ������ hp�� 1 ��´�. 1���� ��, HP ��ġ�� �����ִ� ���ڿ��� üũ�� ü���� ���ҽ�Ų��.
							if (player.m_cHP[0] != '0')
							{
								player.m_cHP[0]--;
								if (player.m_cHP[0] == '0')
									player.m_cHP[1] = '9';
							}
							else if (player.m_cHP[1] != '0')
								player.m_cHP[1]--;
							
							// ������ hp�� 0�̶�� ������ �ش� ��ġ�� ������ false�� ����(��� ����)
							if (player.m_cHP[1] == '0')
								player.m_bAriveCheck = false;

							// ������� ��� ����, ������ ���ݹ����� ���� ��ġ�� �����̽��ٷ� �ٲ۴�. �̴�, ���� ���� �� �����̱� or ����ϸ� �Ⱥ��̰� �ϱ��� ��Ȱ�̴�.
							cBackBuffer[player.m_iPosY][player.m_iPosX] = ' ';
						}

						else
							cBackBuffer[missale[i].m_iPosY][missale[i].m_iPosX] = 'v';
					}
				}		


				//// Enemy ���� ť ������ ���� ////
				// ���� ������ �޾Ƽ�(ť), �ش� ��ǥ�� �̻����� Ȱ��ȭ��Ų��. 
				// ������ �������� ���� ���
				if (player.m_bAriveCheck)
				{
					while (Dequeue(&MonsterAtkQueue, &iTempPosx, &iTempPosy))
					{
						int i = 0;
						// ���� false�� ���� �̻����� ã�´�.
						while (missale[i].m_bAriveCheck)
						{
							// ����, i�� �̻����� ������ ������� break;
							if (i == ENEMY_MISSALE_COUNT)
								break;

							// �װ� �ƴ϶�� ���� �迭 �˻�.
							i++;
						}

						// i�� ����, ENEMY_MISSALE_COUNT���� ������ false�� �迭�� ã�Ҵٴ� ���̴� �Ʒ� ���� ����
						if (i < ENEMY_MISSALE_COUNT)
						{
							// ã�� ��ġ�� �̻����� true�� �����, ã�� ��ġ�� x,y���� �����Ѵ�.
							missale[i].m_bAriveCheck = true;
							missale[i].m_iPosX = iTempPosx;
							missale[i].m_iPosY = iTempPosy;

							// �׸���, ������ �̻����� ���ۿ� �����Ѵ�.
							cBackBuffer[missale[i].m_iPosY][missale[i].m_iPosX] = 'v';
						}
					}
				}

				// -----------------------
				//		   ������
				// -----------------------	
				//// �� �׵θ��� �����Ѵ�. ////
				for (int i = 3; i < UI_HEIGHT; ++i)
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

				//// UI ���� ////
				// HP UI ���� //
				memcpy(cBackBuffer[28] + 38, g_Hp, sizeof(g_Hp) - 1);
				memcpy(cBackBuffer[28] + 43, player.m_cHP, sizeof(player.m_cHP));

				// Time UI ���� //
				// Time : ��� ���ڿ� ����.
				memcpy(cBackBuffer[1] + 35, g_Time, sizeof(g_Time) - 1);

				// ���� �ð��� �޾ƿ´�. UITime ǥ�� üũ�� ����.
				ulEnemyAiTimeCur = GetTickCount64();
				
				// ������ �ð� üũ���� 1�ʰ� ������, ǥ�õǴ� �ð� ����.
				if (ulUITimeSave + 1000 <= ulEnemyAiTimeCur)
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
					ulUITimeSave = ulEnemyAiTimeCur;
				}

				Time[0] = a;
				Time[1] = b;
				Time[2] = ':';
				Time[3] = c;
				Time[4] = d;

				// Time���ڿ� ����. ��������� ����� ���� ������װ� ����Ȱ� ������ �׳� ���� ���� ����ɰ���. ���� ��� ����ִ� ���� �����������.
				memcpy(cBackBuffer[1] + 42, Time, sizeof(Time) - 1);

				//// ȭ�� �׸��� ////
				for (int i = 0; i< HEIGHT; ++i)
					BufferFlip(i, 0);

				Sleep(50);

				// -----------------------
				//	  ���� ���� üũ
				// -----------------------	
				// ���� 0���̰ų� �Ʊ��� ����ϸ� ���� ���� ���� ����
				if (iEnemyAriveCount == 0 || player.m_bAriveCheck == false)
				{
					fputs("\n���� ��!.", stdout);
					Sleep(1500);					

					g_iState = e_Exit;
					break;
				}
			}
			break;
		case e_Exit:
			fputs("\n", stdout);
			exit(1);
			break;
		}
	}
	timeEndPeriod(1);
	return 0;
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

// ���� �ø� �Լ�
void BufferFlip(int iPosy, int iPosx)
{
	cs_MoveCursor(iPosy, iPosx);
	printf("%s", cBackBuffer[iPosy]);
}
// Title �Լ�
void Title(int* iArrowShowPosy, bool* bArrowShowFlag)
{

	memcpy(cBackBuffer[3] + 38, g_cTitle, sizeof(g_cTitle) - 1);
	memcpy(cBackBuffer[5] + 38, g_cStart, sizeof(g_cStart) - 1);
	memcpy(cBackBuffer[6] + 38, g_cExit, sizeof(g_cExit) - 1);

	memcpy(cBackBuffer[8] + 30, g_cTip, sizeof(g_cTip) - 1);

	// iArrowShowPosy�� ���� ���� ���� ȭ��ǥ�� ������
	if ((*iArrowShowPosy) == 5)
	{
		cBackBuffer[6][35] = ' ';
		cBackBuffer[6][36] = ' ';
	}
	else if ((*iArrowShowPosy) == 6)
	{
		cBackBuffer[5][35] = ' ';
		cBackBuffer[5][36] = ' ';
	}

	// ȭ��ǥ �������� ����
	if ((*bArrowShowFlag) == true)
	{
		cBackBuffer[(*iArrowShowPosy)][35] = '-';
		cBackBuffer[(*iArrowShowPosy)][36] = '>';
	}
	else
	{
		cBackBuffer[(*iArrowShowPosy)][35] = ' ';
		cBackBuffer[(*iArrowShowPosy)][36] = ' ';
	}


	for (int i = 0; i<HEIGHT; ++i)
		BufferFlip(i, 0);
}
// ĳ���� ���� �Լ�
void CharSeleet(int* iArrowShowPosy, bool* bArrowShowFlag)
{
	memcpy(cBackBuffer[3] + 30, g_cCharSeletTip, sizeof(g_cCharSeletTip) - 1);

	cBackBuffer[5][23] = 'A';
	memcpy(cBackBuffer[5] + 25, g_cCharSeletChaTip_1, sizeof(g_cCharSeletChaTip_1) - 1);

	cBackBuffer[7][23] = 'B';
	memcpy(cBackBuffer[7] + 25, g_cCharSeletChaTip_2, sizeof(g_cCharSeletChaTip_2) - 1);

	cBackBuffer[9][23] = 'V';
	memcpy(cBackBuffer[9] + 25, g_cCharSeletChaTip_3, sizeof(g_cCharSeletChaTip_3) - 1);

	memcpy(cBackBuffer[11] + 30, g_cTip, sizeof(g_cTip) - 1);

	// iArrowShowPosy�� ���� ���� ���� ȭ��ǥ�� ������
	if ((*iArrowShowPosy) == 5)
	{
		cBackBuffer[7][20] = ' ';
		cBackBuffer[7][21] = ' ';

		cBackBuffer[9][20] = ' ';
		cBackBuffer[9][21] = ' ';
	}
	else if ((*iArrowShowPosy) == 7)
	{
		cBackBuffer[5][20] = ' ';
		cBackBuffer[5][21] = ' ';

		cBackBuffer[9][20] = ' ';
		cBackBuffer[9][21] = ' ';
	}
	else if ((*iArrowShowPosy) == 9)
	{
		cBackBuffer[5][20] = ' ';
		cBackBuffer[5][21] = ' ';

		cBackBuffer[7][20] = ' ';
		cBackBuffer[7][21] = ' ';
	}

	// ȭ��ǥ �������� ����
	if ((*bArrowShowFlag) == true)
	{
		cBackBuffer[(*iArrowShowPosy)][20] = '-';
		cBackBuffer[(*iArrowShowPosy)][21] = '>';
	}
	else
	{
		cBackBuffer[(*iArrowShowPosy)][20] = ' ';
		cBackBuffer[(*iArrowShowPosy)][21] = ' ';
	}

	for (int i = 0; i<HEIGHT; ++i)
		BufferFlip(i, 0);
}
// ���� �ʱ�ȭ �Լ�
void BufferClear()
{
	memset(cBackBuffer, 0X20, HEIGHT*WIDTH);
	for (int i = 0; i < HEIGHT; ++i)
		cBackBuffer[i][WIDTH - 1] = '\0';
}