#pragma once
#ifndef __ACCOUNT_H__
#define __ACCOUNT_H__

/*
 * ������Ʈ ���� : [2018�� 1�� 13��] ���� 0.7
 */

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

	// ���� ������
	Account& operator=(const Account&);

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
#endif // !__ACCOUNT_H__

