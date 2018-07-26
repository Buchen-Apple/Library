/*
* ������Ʈ ���� : [2018�� 1�� 13��] ���� 0.7
*/

#include "BankingCommonDecl.h"
#include "Account.h"

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

// Account ���� ���� ������
Account& Account::operator=(const Account& ref)
{
	m_iID = ref.m_iID;
	m_iMoney = ref.m_iMoney;

	delete[] m_cName;
	m_cName = new char[strlen(ref.m_cName) + 1];
	strcpy_s(m_cName, strlen(ref.m_cName) + 1, ref.m_cName);

	return *this;
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