// 5. ��Ʈ�� Ŭ���� AccountHandler �߰�
// Entity Ŭ���� �߰�

#include <iostream>
#include "conio.h"
#include <cstring>

using namespace std;

#define ACCOUNT_COUNT 50

enum
{
	CREATE = 1, INSERT, OUT, ALL_SHOW, EXIT
};

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
	void Insert(int iTempMoney);

	// �ܾ� üũ �� ���
	bool Out(int iTempMoney);

	// ���� �����ֱ�
	void Show() const;

	// �Ҹ���
	~Account();
};

class AccountHandler
{	
	Account* arrArc[ACCOUNT_COUNT]; // ���� ������ �����ϴ� ��ü ������ �迭
	int iAccountCurCount;	// ���� �� ���� ����

public:
	// ������
	AccountHandler()
		: iAccountCurCount(0) {}

	//���� ���� �Լ�
	void CreateFunc();

	// �Ա� �Լ�
	void InsertFunc();

	// ��� �Լ�
	void OutFunc();

	// ���� ���� ��ü ��� �Լ�
	void ShowFunc();
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

		// ���α׷� ���� �÷���
		if (iInput == EXIT)
			break;

		switch (iInput)
		{
		case CREATE:
			AcHandler.CreateFunc();
			_getch();

			break;
		case INSERT:
			AcHandler.InsertFunc();
			_getch();

			break;
		case OUT:
			AcHandler.OutFunc();
			_getch();

			break;
		case ALL_SHOW:
			AcHandler.ShowFunc();
			_getch();

			break;
		}

	}

	return 0;
}

// ---------------------
// Account�� ����Լ�
// ---------------------

// Account ���� ������
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
	cout << "�ܾ� : " << m_iMoney << endl << endl;
}

// Account ���� �Ҹ���
Account::~Account()
{
	delete[] m_cName;
}


// ---------------------
// AccountHandler�� ����Լ�
// ---------------------

// AccountHandler ���� ���� ���� �Լ�
void AccountHandler::CreateFunc()
{
	int iTempMoney;
	int iTempID;
	char cTempName[30];

	cout << "[���� ����]" << endl;
	cout << "���� ID : ";
	cin >> iTempID;

	cout << "�̸� : ";
	cin >> cTempName;

	cout << "�Ա� �� : ";
	cin >> iTempMoney;

	arrArc[iAccountCurCount] = new Account(cTempName, iTempID, iTempMoney);

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
	}

	cout << "��� ��� �Ϸ�!" << endl;
}