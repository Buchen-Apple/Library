/*
* ������Ʈ ���� : [2018�� 1�� 13��] ���� 0.7
*/

#pragma once
#ifndef __ACCOUNT__HANDLER_H__
#define __ACCOUNT__HANDLER_H__

#include "Account.h"
#include "BankingCommonDecl.h"
#include "AccountArray.h"


// �ڵ鷯
class AccountHandler
{
	ArrayBoundCheck* arrArc[ACCOUNT_COUNT]; // ���� ������ �����ϴ� ��ü ������ �迭
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

#endif // !__ACCOUNT__HANDLER_H__

