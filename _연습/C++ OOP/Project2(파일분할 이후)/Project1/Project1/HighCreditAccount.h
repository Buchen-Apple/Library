/*
* ������Ʈ ���� : [2018�� 1�� 13��] ���� 0.7
*/

#pragma once
#ifndef __HIGH_CREDIT_ACCOUNT_H__
#define __HIGH_CREDIT_ACCOUNT_H__

#include "NormalAccount.h"
#include "BankingCommonDecl.h"

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
	cout << "�ſ��� : " << CreditRareenum << "���" << endl;
}

#endif // !__HIGH_CREDIT_ACCOUNT_H__

