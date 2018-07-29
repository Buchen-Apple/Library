#include "stdafx.h"
#include "MyStringClass.h"


// ---------------
// ������ ��Ʈ
// ---------------
// �� �Է¹޴� ������
MyString::MyString(const char* str)
{
	m_len = strlen(str);

	m_String = new char[m_len + 1];

	strcpy_s(m_String, m_len + 1, str);
}

// ���� ������ (���� ����)
MyString::MyString(const MyString& copy)
{
	// ���ο� ���ڿ��� �����Ҵ�.
	m_len = strlen(copy.m_String);

	m_String = new char[m_len + 1];

	strcpy_s(m_String, m_len + 1, copy.m_String);

}

// ---------------
// ������ �����ε� ��Ʈ
// ---------------
// ���� ������ (���� ����)
MyString& MyString::operator=(const MyString& copy)
{
	// ������ ���ڿ��� �ִ� ���, �Ҵ� ����
	if (m_String != nullptr)
		delete[] m_String;

	// ���ο� ���ڿ��� �����Ҵ�.
	m_len = copy.m_len;

	m_String = new char[m_len + 1];

	strcpy_s(m_String, m_len + 1, copy.m_String);

	return *this;
}

// + ������ �����ε�.
// A+B ���ڿ��� ����� ��ü�� ��ȯ
MyString MyString::operator+(const MyString& str)
{
	size_t Templen = m_len + strlen(str.m_String);

	char* Temp = new char[Templen + 1];

	strcpy_s(Temp, m_len + 1, m_String);
	strcat_s(Temp, Templen + 1, str.m_String);

	MyString retobj = Temp;

	delete[] Temp;

	return retobj;
}

// += ������ �����ε�
// A ���ڿ��� B�� ���δ�.
MyString& MyString::operator+=(const MyString& ref)
{
	size_t Templen = m_len + strlen(ref.m_String);

	char* Temp = new char[Templen + 1];

	strcpy_s(Temp, m_len + 1, m_String);
	strcat_s(Temp, Templen + 1, ref.m_String);

	delete[] m_String;


	m_len = Templen;
	m_String = new char[m_len + 1];
	strcpy_s(m_String, m_len + 1, Temp);

	delete[] Temp;

	return *this;
}

// == ������ �����ε�
// A�� B ��
// ������ true, �ٸ��� false ��ȯ
bool MyString::operator==(const MyString& ref)
{
	if (strcmp(m_String, ref.m_String) == 0)
		return true;

	else
		return false;
}

// �Ҹ���
MyString::~MyString()
{
	delete[] m_String;
}



// << ������ �����ε�
// ȭ�� ��¿� 
ostream& operator<<(ostream& os, const MyString& ref)
{
	os << ref.m_String;

	return os;
}

// >> ������ �����ε�
// �Է¹ޱ� �� 
istream& operator<<(istream& is, const MyString& ref)
{
	is >> ref.m_String;

	return is;
}