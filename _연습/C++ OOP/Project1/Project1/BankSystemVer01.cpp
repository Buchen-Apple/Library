// 1�ܰ� : Ʋ ����

#include <iostream>
#include "conio.h"

using namespace std;

#define ACCOUNT_COUNT 50

enum 
{
	CREATE=1, INSERT, OUT, ALL_SHOW, EXIT
};

typedef struct
{
	char m_cName[30];
	int m_iID;
	int m_iMoney;

} Account;

// ���¸� �����ϴ� ����ü �迭
Account arrArc[ACCOUNT_COUNT];

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

	cout << "[���� ����]" << endl;
	cout << "���� ID : ";
	cin >> arrArc[iAccountCurCount].m_iID;

	cout << "�̸� : ";
	cin >> arrArc[iAccountCurCount].m_cName;

	cout << "�Ա� �� : ";
	cin >> iTempMoney;
	arrArc[iAccountCurCount].m_iMoney += iTempMoney;

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
		if (arrArc[i].m_iID == iTempID)
			break;
	}

	if (i >= iAccountCurCount)
	{
		cout << "���� �����Դϴ�." << endl;
		return;
	}

	cout << "�Ա� �� : ";
	cin >> iTempMoney;
	arrArc[i].m_iMoney += iTempMoney;

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
		if (arrArc[i].m_iID == iTempID)
			break;
	}

	if (i >= iAccountCurCount)
	{
		cout << "���� �����Դϴ�." << endl;
		return;
	}

	cout << "��� �� : ";
	cin >> iTempMoney;

	if (arrArc[i].m_iMoney < iTempMoney)
	{
		cout << "�ܾ��� �����մϴ�." << endl;
		return;
	}

	arrArc[i].m_iMoney -= iTempMoney;

	cout << "��� �Ϸ�!" << endl;

}

// ���� ���� ��ü ��� �Լ�
void ShowFunc()
{
	cout << "[���� ���� ��ü ���]" << endl;

	for (int i = 0; i < iAccountCurCount; ++i)
	{
		cout << "-----------------------" << endl;
		cout << "���� ID : " << arrArc[i].m_iID << endl;
		cout << "�̸� : " << arrArc[i].m_cName << endl;
		cout << "�ܾ� : " << arrArc[i].m_iMoney << endl << endl;
	}

	cout << "��� ��� �Ϸ�!" << endl;
}