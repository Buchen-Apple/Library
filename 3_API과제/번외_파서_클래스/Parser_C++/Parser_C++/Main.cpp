#include <iostream>
#include "Parser_Class.h"
#include <locale.h >

using namespace std;

// ���� ����ü
class Monster
{
public:
	TCHAR m_iName[30];
	int m_iHP;
	int m_iMP;
	int m_iAtack;
	int m_iArmor;

public:
	// ���� ����ü ���� ���� �Լ�
	void ShowMonster() const
	{
		_tprintf(_T("����\n"));
		_tprintf(_T("�̸� : %s \n"), m_iName);
		_tprintf(_T("ü�� : %d \n"), m_iHP);
		_tprintf(_T("���� : %d \n"), m_iMP);
		_tprintf(_T("���ݷ� : %d \n"), m_iAtack);
		_tprintf(_T("���� : %d \n\n"), m_iArmor);
	}
};

// �÷��̾� ����ü
class Player
{
public:
	TCHAR m_iName[30];
	int m_iHP;
	int m_iMP;
	int m_iAtack;
	int m_iArmor;

public:
	// �÷��̾� ����ü ���� ���� �Լ�
	void ShowMonster() const
	{	
		_tprintf(_T("�÷��̾�\n"));
		_tprintf(_T("�̸� : %s \n"),m_iName);
		_tprintf(_T("ü�� : %d \n"), m_iHP);
		_tprintf(_T("���� : %d \n"), m_iMP);
		_tprintf(_T("���ݷ� : %d \n"), m_iAtack);
		_tprintf(_T("���� : %d \n\n"), m_iArmor);		
	}
};

int main(void)
{
	_tsetlocale(LC_ALL, L"korean");

	Monster mon;
	Player ply;
	Parser parser;
	bool bParserAreaCheck = false;

	//----------------------
	//   ���� �ε�
	//----------------------
	try
	{
		parser.LoadFile(_T("Test_UNICODE.txt"));
	}
	catch (int expn)
	{
		if (expn == 1)
		{
			_tprintf(_T("���� ���� ����\n"));
			exit(-1);
		}
		else if (expn == 2)
		{
			_tprintf(_T("���� �о���� ����\n"));
			exit(-1);
		}
	}
	

	//----------------------
	//    ply�� �� ���� 
	//----------------------
	// ���� ����
	bParserAreaCheck = parser.AreaCheck(_T("PLAYER"));

	// ������ ���������� �����Ǿ����� �Ʒ� ����
	if (bParserAreaCheck)
	{
		// ���ϴ� �� ã��
		parser.GetValue_String(_T("m_iName"), ply.m_iName);
		parser.GetValue_Int(_T("m_iHP"), &ply.m_iHP);
		parser.GetValue_Int(_T("m_iMP"), &ply.m_iMP);
		parser.GetValue_Int(_T("m_iAtack"), &ply.m_iAtack);
		parser.GetValue_Int(_T("m_iArmor"), &ply.m_iArmor);
		// Ȯ���ϱ�
		ply.ShowMonster();
	}

	// ���� ���� ���� �� �Ʒ� ����
	else
		_tprintf(_T("�÷��̾� ���� ã�� ����\n"));


	////----------------------
	////    mon�� �� ���� 
	////----------------------
	// ���� ����
	bParserAreaCheck = parser.AreaCheck(_T("MONSTER"));

	// ������ ���������� �����Ǿ����� �Ʒ� ����
	if (bParserAreaCheck)
	{
		// ���ϴ� �� ã��
		parser.GetValue_String(_T("m_iName"), mon.m_iName);
		parser.GetValue_Int(_T("m_iHP"), &mon.m_iHP);
		parser.GetValue_Int(_T("m_iMP"), &mon.m_iMP);
		parser.GetValue_Int(_T("m_iAtack"), &mon.m_iAtack);
		parser.GetValue_Int(_T("m_iArmor"), &mon.m_iArmor);
		// Ȯ���ϱ�
		mon.ShowMonster();
	}

	// ���� ���� ���� �� �Ʒ� ����
	else
		_tprintf(_T("���� ���� ã�� ����\n"));


	return 0;
}