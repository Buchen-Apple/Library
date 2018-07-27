#include <iostream>
#include "Parser_Class.h"

using namespace std;
using namespace Library_Jingyu;

// ���� ����ü
class Monster
{
public:
	char m_iName[30];
	int m_iHP;
	int m_iMP;
	int m_iAtack;
	int m_iArmor;

public:
	// ���� ����ü ���� ���� �Լ�
	void ShowMonster() const
	{
		cout << "����!" << endl;
		cout << "�̸� : " << m_iName << endl;
		cout << "ü�� : " << m_iHP << endl;
		cout << "���� : " << m_iMP << endl;
		cout << "���ݷ� : " << m_iAtack << endl;
		cout << "���� : " << m_iArmor << endl;
	}
}; // ���� ����ü

// �÷��̾� ����ü
class Player
{
public:
	char m_iName[30];
	int m_iHP;
	int m_iMP;
	int m_iAtack;
	int m_iArmor;

public:
	// �÷��̾� ����ü ���� ���� �Լ�
	void ShowMonster() const
	{
		cout << "�÷��̾�!" << endl;
		cout << "�̸� : " << m_iName << endl;
		cout << "ü�� : " << m_iHP << endl;
		cout << "���� : " << m_iMP << endl;
		cout << "���ݷ� : " << m_iAtack << endl;
		cout << "���� : " << m_iArmor << endl;
	}
};

int main(void)
{
	Monster mon;
	Player ply;
	Parser parser;
	bool bParserAreaCheck = false;

	// ���� �ε�
	try
	{
		parser.LoadFile("Test.txt");
	}
	catch (int expn)
	{
		if (expn == 1)
		{
			cout << "���� ���� ����" << endl;
			exit(-1);
		}
		else if (expn == 2)
		{
			cout << "���� �о���� ����" << endl;
			exit(-1);
		}
	}	

	//----------------------
	//    ply�� �� ���� 
	//----------------------
	// ���� ����
	bParserAreaCheck = parser.AreaCheck("PLAYER");

	if (bParserAreaCheck)
	{
		// ���ϴ� �� ã��
		parser.GetValue_String("m_iName", ply.m_iName);
		parser.GetValue_Int("m_iHP", &ply.m_iHP);
		parser.GetValue_Int("m_iMP", &ply.m_iMP);
		parser.GetValue_Int("m_iAtack", &ply.m_iAtack);
		parser.GetValue_Int("m_iArmor", &ply.m_iArmor);
		// Ȯ���ϱ�
		ply.ShowMonster();
		cout << endl;
	}
	else
		cout << "�÷��̾� ã�� ������ �����ϴ�." << endl;

	//----------------------
	//    mon�� �� ���� 
	//----------------------
	
	// ���� ����
	bParserAreaCheck = parser.AreaCheck("MONSTER");

	if (bParserAreaCheck)
	{
		// ���ϴ� �� ã��
		parser.GetValue_String("m_iName", mon.m_iName);
		parser.GetValue_Int("m_iHP", &mon.m_iHP);
		parser.GetValue_Int("m_iMP", &mon.m_iMP);
		parser.GetValue_Int("m_iAtack", &mon.m_iAtack);
		parser.GetValue_Int("m_iArmor", &mon.m_iArmor);
		// Ȯ���ϱ�
		mon.ShowMonster();
	}
	else
		cout << "���� ã�� ������ �����ϴ�." << endl; 

	return 0;
}