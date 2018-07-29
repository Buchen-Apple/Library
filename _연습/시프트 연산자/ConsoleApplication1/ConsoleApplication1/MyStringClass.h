#ifndef __MY_STRING_CLASS_H__
#define __MY_STRING_CLASS_H__


#include <iostream>

using namespace std;

class MyString
{
	char* m_String = nullptr;

	// �ι��� ���� ����
	size_t m_len;

public:

	// ---------------
	// ������ ��Ʈ
	// ---------------
	// ���̵� ������
	MyString()
		:m_len(0), m_String(nullptr)
	{}

	// �� �Է¹޴� ������
	MyString(const char* str);

	// ���� ������ (���� ����)
	MyString(const MyString& copy);

	// ---------------
	// ������ �����ε� ��Ʈ
	// ---------------
	// ���� ������ (���� ����)
	MyString& operator=(const MyString& copy);

	// + ������ �����ε�.
	// A+B ���ڿ��� ����� ��ü�� ��ȯ
	MyString operator+(const MyString& str);

	// += ������ �����ε�
	// A ���ڿ��� B�� ���δ�.
	MyString& operator+=(const MyString& ref);

	// == ������ �����ε�
	// A�� B ��
	// ������ true, �ٸ��� false ��ȯ
	bool operator==(const MyString& ref);

	// << ������ �����ε�
	// ȭ�� ��¿� 
	friend ostream& operator<<(ostream& os, const MyString& ref);

	// >> ������ �����ε�
	// �Է¹ޱ� �� 
	friend istream& operator<<(istream& os, const MyString& ref);


	~MyString();
};


// << ������ �����ε�
// ȭ�� ��¿� 
ostream& operator<<(ostream& os, const MyString& ref);

// >> ������ �����ε�
// �Է¹ޱ� �� 
istream& operator<<(istream& is, const MyString& ref);


#endif // !__MY_STRING_CLASS_H__
