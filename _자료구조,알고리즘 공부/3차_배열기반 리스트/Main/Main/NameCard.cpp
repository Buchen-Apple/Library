#include "pch.h"
#include "NameCard.h"

using namespace std;

// �̸�, ����ȣ ���
void NameCard::ShowInfo()
{
	cout << "[" << m_cName << " : " << m_cPhone << "]" << endl;
	
}

// �̸� ��
// ������ 0, �ٸ��� 0�� �ƴ� �� ����
int NameCard::NameCompare(const char* Name)
{
	if (strcmp(m_cName, Name) == 0)
		return 0;

	return 1;
}

// ��ȭ��ȣ ����
void NameCard::ChangePhoneNum(const char* Phone)
{
	strcpy_s(m_cPhone, Phone);
}

// ������
NameCard::NameCard(const char* Name, const char* Phone)
{
	strcpy_s(m_cName, Name);
	strcpy_s(m_cPhone, Phone);
}



// NameCard �����Ҵ� �� �̸�,����ȣ ����
NameCard* CreateNameCard(const char* Name, const char* Phone)
{
	return new NameCard(Name, Phone);
}