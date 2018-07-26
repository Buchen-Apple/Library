/*
* ������Ʈ ���� : [2018�� 1�� 13��] ���� 0.7
*/

#include "BankingCommonDecl.h"

#include "AccountHandler.h"
#include "Account.h"
#include "NormalAccount.h"
#include "HighCreditAccount.h"

// ---------------------
// AccountHandler�� ����Լ� (�ڵ鷯 Ŭ����)
// ---------------------

// AccountHandler ���� ���� ���� �Լ�
void AccountHandler::CreateAccount()
{
	int iInput2;
	cout << "[������������]" << endl;
	cout << "1. ���뿹�ݰ���  2. �ſ�ŷڰ���" << endl;
	cin >> iInput2;
	if (iInput2 == 1)
		CreateFunc1();
	else
		CreateFunc2();
}

// AccountHandler ���� ���� ���� ���� �Լ�
void AccountHandler::CreateFunc1()
{
	int iTempMoney;
	int iTempID;
	char cTempName[30];
	int Percent;

	cout << "[���� ����]" << endl;
	cout << "���� ID : ";
	cin >> iTempID;

	cout << "�̸� : ";
	cin >> cTempName;

	cout << "�Ա� �� : ";
	cin >> iTempMoney;

	cout << "������ : ";
	cin >> Percent;

	arrArc[iAccountCurCount] = new NormalAccount(cTempName, iTempID, iTempMoney, Percent);

	iAccountCurCount++;

	cout << "���� ���� �Ϸ�!" << endl;
}

// AccountHandler ���� �ſ� ���� ���� �Լ�
void AccountHandler::CreateFunc2()
{
	int iTempMoney;
	int iTempID;
	char cTempName[30];
	int Percent;
	int Rare;


	cout << "[���� ����]" << endl;
	cout << "���� ID : ";
	cin >> iTempID;

	cout << "�̸� : ";
	cin >> cTempName;

	cout << "�Ա� �� : ";
	cin >> iTempMoney;

	cout << "������ : ";
	cin >> Percent;

	cout << "�ſ� ���(1toA, 2toB, 3toC) : ";
	cin >> Rare;

	arrArc[iAccountCurCount] = new HighCreditAccount(cTempName, iTempID, iTempMoney, Percent, Rare);

	iAccountCurCount++;

	cout << "���� ���� �Ϸ�!" << endl;
}

// AccountHandler ���� �Ա� �Լ�
void AccountHandler::InsertFunc()
{
	int iTempID;
	int iTempMoney;
	int i;

	cout << "[�� ��]" << endl;
	cout << "���� ID : ";
	cin >> iTempID;

	for (i = 0; i < iAccountCurCount; ++i)
	{
		if (arrArc[i]->GetID() == iTempID)
			break;
	}

	if (i >= iAccountCurCount)
	{
		cout << "���� �����Դϴ�." << endl;
		return;
	}

	cout << "�Ա� �� : ";
	cin >> iTempMoney;
	arrArc[i]->Insert(iTempMoney);

	cout << "�Ա� �Ϸ�!" << endl;
}

// AccountHandler ���� ��� �Լ�
void AccountHandler::OutFunc()
{
	int iTempID;
	int iTempMoney;
	int i;

	cout << "[�� ��]" << endl;
	cout << "���� ID : ";
	cin >> iTempID;

	for (i = 0; i < iAccountCurCount; ++i)
	{
		if (arrArc[i]->GetID() == iTempID)
			break;
	}

	if (i >= iAccountCurCount)
	{
		cout << "���� �����Դϴ�." << endl;
		return;
	}

	cout << "��� �� : ";
	cin >> iTempMoney;

	if (arrArc[i]->Out(iTempMoney) == false)
	{
		cout << "�ܾ��� �����մϴ�." << endl;
		return;
	}
	else
	{
		cout << "��� �Ϸ�!" << endl;
	}

}

// AccountHandler ���� ���� ���� ��ü ��� �Լ�
void AccountHandler::ShowFunc()
{
	cout << "[���� ���� ��ü ���]" << endl;

	for (int i = 0; i < iAccountCurCount; ++i)
	{
		cout << "-----------------------" << endl;
		arrArc[i]->Show();
		cout << endl;
	}

	cout << "��� ��� �Ϸ�!" << endl;
}
