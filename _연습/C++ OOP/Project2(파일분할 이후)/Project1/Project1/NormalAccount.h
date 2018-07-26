/*
* ������Ʈ ���� : [2018�� 1�� 13��] ���� 0.7
*/

// ����� ���ǰ� ���� �ִ�.
#pragma once
#ifndef __NORMAL_ACCOUNT_H__
#define _NORMAL_ACCOUNT_H__

#include "Account.h"

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


#endif // !__NORMAL_ACCOUNT_H__

