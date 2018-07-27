#pragma once
#ifndef __SCENE_CLASS_H__
#define __SCENE_CLASS_H__

#include <Windows.h>
#define ENEMY_COUNT	50
#define ENEMY_MISSALE_COUNT 50
#define PLAYER_MISSALE_COUNT 50

// �ܼ� ��� ���� �غ� �۾�
void cs_Initial(void);
// �ܼ� ȭ���� Ŀ���� x,y ��ǥ�� �̵�
void cs_MoveCursor(int iPosy, int iPosx);

// ���漱��
enum SceneType;	// �� enum
class Queue;	// MoveQueue.h�� Queue Ŭ����
class CSceneHandle;	// CSceneHandle Ŭ����
class CPlayer;		// CPlayer ���� ����


// CObject Ŭ���� ����
class CObject
{	
public:
	int Type;	// 0 : ���� Ÿ�� �ο� �ȵ� 1 : �÷��̾� / 2 : ���� / 3 : �̻���
	CObject() : Type(0) {}	// ������
	
	virtual void InterfaceFunc() {};

};

class CMissale :public CObject
{
	int m_iPosX, m_iPosY;
	bool m_PlayerOrEnemy;
	bool m_bAriveCheck;
	bool m_BonusOrNot;
	friend class CPlayer;
	friend class CEnemy;
public:
	CMissale(); // ������
	CMissale& operator=(const CMissale*);	// ���� ������
	void InfoSetFunc(bool);	// �̻��� ���� ����
};


// CEnemy Ŭ���� ����
class CEnemy :public CObject
{
	friend class CPlayer;
	int m_iPosX, m_iPosY;
	bool m_bAriveCheck;
	int m_iAttackTick;
	ULONGLONG m_MoveTimeSave;
	ULONGLONG m_AttackTimeSave;
	Queue* EnemyAtkQueue;
	
	const int ENEMY_AI_TIME = 1400;	// ���� AI�۵� ƽ. 1400(1.4��)���� 1ȸ�� AI�۵�
public:
	CEnemy();	// ������
	CEnemy& operator=(const CEnemy*);								// ���� ������
	void InfoSetFunc(int*, int*, int*, int*, int*, ULONGLONG);		// ���� ���� ����
	void EnemyMoveFunc(int*, ULONGLONG, bool*);						// ���� AI �̵� ó��
	bool GetArive();												// �ش� ������ �������� ����
	void EnemyEnqueueFunc(ULONGLONG);								// �ش� ������ ���� ��ť
	void EnemyMissaleMoveFunc(CMissale*, CPlayer*);				// ���� �̻��� �̵� �� �Ҹ� ó�� �Լ�
	void EnemyDequeueFunc(CMissale*);										// �ش� ������ ���� ��ť
	~CEnemy();														// �Ҹ���
};

// CPlayer Ŭ���� ����
class CPlayer	:public CObject
{
	friend class CEnemy;
private:
	int m_iPosX, m_iPosY;
	bool m_bAriveCheck;
	Queue* m_MoveQueue;
	Queue* m_AtkQueue;
	char m_cLook;							// ĳ���� ���
	char m_cMissaleLook;					// �̻��� ���
	char m_cHP[2];
	int m_iCharSpeed;
	int m_iBonusMIssaleCount;				// �� ���� �߻�Ǵ� ���ʽ� �̻��� ����. ĳ���Ϳ� ���� �ٸ���.
	ULONGLONG m_BonusMIssaleTImeSave;		// �̻��� �߻� ���� �ð� ����. 
	int iPlayerMissaleCount;				// ���� ������ �̻��� ��

	const int UI_HEIGHT = 27;
	const int UI_WIDTH = 81;
	const int PLAYER_AI_ATACK_TIME = 2000;	// 2000(2��)���� 1ȸ�� ���ʽ� �̻��� �߻�. ���ʽ� �̻����� �ִ� ���!

public:	
	CPlayer();
	void InfoSetFunc_1(int);				// �÷��̾� ���� ����. CSceneSelect���� �⺻ ���� ����
	void InfoSetFunc_2();					// �÷��̾� ���� ����. CSceneGame���� �� �� �ڼ��� ���� �߰�����
	void KeyDownCheck();					// �÷��̾� �̵� / ���� ���� üũ
	char* GetHP();							// �÷��̾��� ���� Hp�� ���� �Լ�
	bool GetArive();						// �÷��̾��� �������θ� ���� �Լ�
	void PlayerMoveFunc();					// �÷��̾� �̵� ����ó�� �Լ�
	void MissaleMoveFunc(CMissale*, CEnemy*, int*);					// �̻��� �̵� �� �Ҹ� ó�� �Լ�
	void MissaleCreateFunc(CMissale*);							// �̻��� ���� ó�� �Լ�
	void BonusMissaleFunc(CMissale*, ULONGLONG);				// ���ʽ� �̻��� ���� ó��. �̵��� MissaleMoveFunc���� ���� ó��
	~CPlayer();													// �Ҹ���
};



// CSceneBase Ŭ���� ����
class CSceneBase
{	
protected:
	// ���� �ʱ�ȭ �Լ�
	void BufferClear();
	// ���� �ø� �Լ�
	void BufferFlip(int iPosy, int iPosx);

public:
	virtual void run() = 0;
	virtual ~CSceneBase() {};
	
};

// CSceneTitle Ŭ���� ����
class CSceneTitle :public CSceneBase
{
public:	
	void Title();			// Title �Լ�	
	virtual void run();		// run �Լ�	

};

// CSceneSelect Ŭ���� ����
class CSceneSelect :public CSceneBase
{	
private:
	// Ű���� üũ �Լ�
	void KeyDown();
	// ĳ���� ���� �Լ�
	void CharSeleet();

public:	
	virtual void run();				// run �Լ�
	
};

// CSceneGame Ŭ���� ����
class CSceneGame :public CSceneBase
{
private:
	const char g_Hp[6] = "HP : ";
	const char g_Time[8] = "Time : ";

private:
	CPlayer* player;
	CEnemy* enemy;
	CMissale* missale;
	ULONGLONG m_ulStartTimeSave;									// ���� ���� ������ �ð� ����. �������� �� ����.
	ULONGLONG ulUITimeSave;											// UI�� ǥ�õǴ� �ð��� üũ�ϱ� ���� Ÿ�̸�.
	ULONGLONG ulUITimeNow;											// UI�� ǥ�õǴ� �ð��� üũ�ϱ� ���� Ÿ�̸�2. cur �ð� ����
	bool bAiActiveCheck;											// ������ AI�۵� ���� üũ.
	int iEnemyAIState;												// �̵� AI�� ���� ����. 1�϶��� �·� �̵� ���..
	int iEnemyAriveCount;											// ���� ���� ������ ��. �⺻�� 50(��� ����)

	char Time[6];		// UI �ð�ǥ�ø� ���� ����
	char a, b, c, d;	// UI �ð�ǥ�ø� ���� ������

public:
	CSceneGame();				// ������
	virtual void run();			// run �Լ�
	void Moverun();	// run �Լ� �ȿ��� ȣ��ȴ�. �̵� ������ ����ϴ� �Լ�
	void Atkrun();				// run �Լ� �ȿ��� ȣ��ȴ�. �̵� ������ ����ϴ� �Լ�
	void UIRendering();		// run �Լ� �ȿ��� ȣ��ȴ�. Ÿ�� ui�� �������ϴ� �Լ�
	void GetListData();			// run �Լ� �ȿ��� ȣ��ȴ�. ���� �����͸� ����Ʈ���� �������� ����
	void UploadListData();		// run �Լ� �ȿ��� ȣ��ȴ�. ���� �����͸� ����Ʈ�� �ִ� ����
	~CSceneGame();				// �Ҹ���
};

class CSceneLose :public CSceneBase
{
	const char LOSE_TEXT[13] = "Game Over...";
public:
	virtual void run();		//run �Լ�
	
};

class CSceneVictory :public CSceneBase
{
	const char VICTORY_TEXT[13] = "Victory!!...";
public:
	virtual void run();		//run �Լ�

};

// CSceneHandle Ŭ���� ����
class CSceneHandle
{
private:
	CSceneBase * m_NowScene;
	SceneType m_eNextScene;
	SceneType m_NowSceneText;

public:	
	CSceneHandle();	// ������	
	void run();		// run �Լ�						
	void LoadScene(SceneType temp); // CSceneHandle::�� ���� �Լ�
	~CSceneHandle();	// �Ҹ���
};


extern CSceneHandle hSceneHandle;

//#undef ENEMY_COUNT

#endif // !__SCENE_CLASS_H__

