// 6. ���¸� ������� , �ſ���·� ����. �̿� ���� ��� �� �����Լ� ����

#include <iostream>
#include "conio.h"
#include <cstring>
#include <windows.h>

using namespace std;

#define ACCOUNT_COUNT 50

enum
{
	CREATE = 1, INSERT, OUT_MONEY, ALL_SHOW, EXIT
};

enum CREDIT
{
	A = 7, B = 4, C = 2
};

// ���� �⺻ ����
class Account
{
	char* m_cName;
	int m_iID;
	int m_iMoney;
public:
	// ������
	Account(char* name, int ID, int Money);

	// ���� ������
	Account(const Account& ref);

	// ID ��ȯ
	int GetID() const;

	// �Ա�
	virtual void Insert(int iTempMoney);

	// �ܾ� üũ �� ���
	bool Out(int iTempMoney);

	// ���� �����ֱ�
	virtual void Show() const;

	// �Ҹ���
	~Account();
};

// ���� ����
class NormalAccount :public Account
{
	int BonusPercent;

public:
	// ������
	NormalAccount(char* name, int ID, int Money, int Bonus);

	// ���� ���� �Ա�
	virtual void Insert(int iTempMoney);

	// ���� �����ֱ�
	virtual void Show() const;
};

// �ſ� ����
class HighCreditAccount :public NormalAccount
{
	int CreditRare;
	CREDIT CreditRareenum;

public:
	// ������
	HighCreditAccount(char* name, int ID, int Money, int Bonus, int Rare);

	// �ſ� ���� �Ա�
	virtual void Insert(int iTempMoney);

	// ���� �����ֱ�
	virtual void Show() const;
};

// �ڵ鷯
class AccountHandler
{
	Account* arrArc[ACCOUNT_COUNT]; // ���� ������ �����ϴ� ��ü ������ �迭
	int iAccountCurCount;	// ���� �� ���� ����

public:
	// ������
	AccountHandler()
		: iAccountCurCount(0) {}

protected:
	// ���� ���� ����
	void CreateFunc1();

	// �ſ� ���� ����
	void CreateFunc2();

public:
	// ���� ���� �Լ�
	void CreateAccount();

	// �Ա� �Լ�
	void InsertFunc();

	// ��� �Լ�
	void OutFunc();

	// ���� ���� ��ü ��� �Լ�
	void ShowFunc();

	// �Ҹ���
	~AccountHandler()
	{
		for (int i = 0; i < iAccountCurCount; ++i)
		{
			delete arrArc[i];
		}
	}
};

int main()
{
	int iInput;
	AccountHandler AcHandler;

	while (1)
	{
		system("cls");

		cout << "----Menu----" << endl;
		cout << "1. ���� ����" << endl;
		cout << "2. �� ��" << endl;
		cout << "3. �� ��" << endl;
		cout << "4. �������� ��ü ���" << endl;
		cout << "5. ���α׷� ����" << endl;
		cin >> iInput;

		switch (iInput)
		{
		case CREATE:
			AcHandler.CreateAccount();
			_getch();

			break;
		case INSERT:
			AcHandler.InsertFunc();
			_getch();

			break;
		case OUT_MONEY:
			AcHandler.OutFunc();
			_getch();

			break;
		case ALL_SHOW:
			AcHandler.ShowFunc();
			_getch();

			break;
		case EXIT:
			return 0;

		default:
			cout << "���� ����Դϴ�." << endl;
			_getch();

			break;
		}

	}

	return 0;
}

// ---------------------
// Account�� ����Լ�
// ---------------------

// Account ���� ������ (����Ƽ Ŭ����)
Account::Account(char* name, int ID, int Money)
	:m_iID(ID), m_iMoney(Money)
{
	size_t len = strlen(name) + 1;
	m_cName = new char[len];
	strcpy_s(m_cName, len, name);
}

// Account ���� ���� ������
Account::Account(const Account& ref)
	:m_iID(ref.m_iID), m_iMoney(ref.m_iMoney)
{
	size_t len = strlen(ref.m_cName) + 1;
	m_cName = new char[len];
	strcpy_s(m_cName, len, ref.m_cName);
}

// Account ���� ID ��ȯ
int Account::GetID() const
{
	return m_iID;
}

// Account ���� �Ա�
void Account::Insert(int iTempMoney)
{
	m_iMoney += iTempMoney;
}

// Account ���� �ܾ� üũ �� ���
bool Account::Out(int iTempMoney)
{
	// ���� ���� ���� iTempMoney���� ����. ��, ��� �Ұ�.
	if (m_iMoney < iTempMoney)
		return false;

	// �������� ����ϸ� ��� �Ϸ�.
	m_iMoney -= iTempMoney;
	return true;
}

// Account ���� ���� �����ֱ�
void Account::Show() const
{
	cout << "���� ID : " << m_iID << endl;
	cout << "�̸� : " << m_cName << endl;
	cout << "�ܾ� : " << m_iMoney << endl;
}

// Account ���� �Ҹ���
Account::~Account()
{
	delete[] m_cName;
}

// ---------------------
// NormalAccount�� ����Լ� (����Ƽ Ŭ����)  (Account ���)
// ---------------------

// NormalAccount���� ������
NormalAccount::NormalAccount(char* name, int ID, int Money, int Bonus)
	:Account(name, ID, Money), BonusPercent(Bonus)
{}

// NormalAccount���� ���� ���� �Ա� (�����Լ�)
void NormalAccount::Insert(int iTempMoney)
{
	Account::Insert(iTempMoney); // ���� �߰�
	Account::Insert(iTempMoney * (BonusPercent / 100.0));	// ���� �߰�
}

// NormalAccount���� ���� �����ֱ� (�����Լ�)
void NormalAccount::Show() const
{
	Account::Show();
	cout << "������ : " << BonusPercent << "%" << endl;
}


// ---------------------
// HighCreditAccount�� ����Լ� (����Ƽ Ŭ����) (NormalAccount ���)
// ---------------------

//HighCreditAccount ���� ������
HighCreditAccount::HighCreditAccount(char* name, int ID, int Money, int Bonus, int Rare)
	:NormalAccount(name, ID, Money, Bonus), CreditRareenum((CREDIT)Rare)
{
	if (Rare == 1)
		CreditRare = 7;
	else if (Rare == 2)
		CreditRare = 4;
	else if (Rare == 3)
		CreditRare = 2;
}

// HighCreditAccount ���� �ſ� ���� �Ա� (�����Լ�)
void HighCreditAccount::Insert(int iTempMoney)
{
	NormalAccount::Insert(iTempMoney);	// ���� �� ���� �߰�
	Account::Insert(iTempMoney * (CreditRare / 100.0));		// �ſ� ��޿� ���� �߰�
}

// HighCreditAccount ���� ���� �����ֱ� (�����Լ�)
void HighCreditAccount::Show() const
{
	NormalAccount::Show();
	cout << "�ſ��� : " << CreditRareenum << "���" <<  endl;
}


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
