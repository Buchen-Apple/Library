// 2. ����ü�� Ŭ������ ����

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
	Account(char* name, int ID, int Money)
		:m_iID(ID), m_iMoney(Money)
	{
		size_t len = strlen(name) + 1;
		m_cName = new char[len];
		strcpy_s(m_cName, len, name);
	}

	// ID ��ȯ
	int GetID()
	{
		return m_iID;
	}

	// �Ա�
	void Insert(int iTempMoney)
	{
		m_iMoney += iTempMoney;
	}

	// �ܾ� üũ �� ���
	bool Out(int iTempMoney)
	{
		// ���� ���� ���� iTempMoney���� ����. ��, ��� �Ұ�.
		if (m_iMoney < iTempMoney)
			return false;
		
		// �������� ����ϸ� ��� �Ϸ�.
		m_iMoney -= iTempMoney;
		return true;
	}

	// ���� �����ֱ�
	void Show()
	{		
		cout << "���� ID : " << m_iID << endl;
		cout << "�̸� : " << m_cName << endl;
		cout << "�ܾ� : " << m_iMoney << endl << endl;
	}

	~Account()
	{
		delete[] m_cName;
	}
};

// ���� ������ �����ϴ� ��ü ������ �迭
Account* arrArc[ACCOUNT_COUNT];

// ���� ���� �� ī��Ʈ
int iAccountCurCount = 0;

// ���� ���� �Լ�
void CreateFunc();

// �Ա� �Լ�
void InsertFunc();

// ��� �Լ�
void OutFunc();

// ���� ���� ��ü ��� �Լ�
void ShowFunc();

int main()
{
	int iInput;

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
			CreateFunc();
			_getch();

			break;
		case INSERT:
			InsertFunc();
			_getch();

			break;
		case OUT:
			OutFunc();
			_getch();

			break;
		case ALL_SHOW:
			ShowFunc();
			_getch();

			break;
		}

	}

	return 0;
}

//���� ���� �Լ�
void CreateFunc()
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

// �Ա� �Լ�
void InsertFunc()
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

// ��� �Լ�
void OutFunc()
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

// ���� ���� ��ü ��� �Լ�
void ShowFunc()
{
	cout << "[���� ���� ��ü ���]" << endl;

	for (int i = 0; i < iAccountCurCount; ++i)
	{
		cout << "-----------------------" << endl;
		arrArc[i]->Show();		
	}

	cout << "��� ��� �Ϸ�!" << endl;
}