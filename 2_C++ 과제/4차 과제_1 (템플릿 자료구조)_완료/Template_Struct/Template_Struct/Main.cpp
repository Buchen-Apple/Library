#include <iostream>
#include "CList_Template.h"

// �׽�Ʈ Ŭ����
class Test
{
	int num;
	char str[10];

public:
	Test(int num, const char* str)
		:num(num)
	{
		strcpy_s(this->str, sizeof(str), str);
	}
	friend ostream& operator<< (ostream& os, const Test& ref);
};

ostream& operator<< (ostream& os, const Test& ref)
{
	os << ref.num << " " << ref.str;
	return os;
}

// ����Ʈ�� ������ �����ִ� �Լ�
template <class T>
void ShowList(CList<Test*>, CList<Test*>::iterator);

int main()
{

	CList<Test*> list;

	CList<Test*>::iterator itor;

	Test ts(1, "������");
	Test ts2(2, "�����");
	Test ts3(3, "������");

	list.push_front(&ts);
	list.push_back(&ts2);
	list.push_back(&ts3);

	ShowList<Test*>(list, itor);	// ����Ʈ ���
	itor = list.begin();

	if (list.is_empty(itor))
		cout << "����ִ�" << endl;
	else
		cout << "������� �ʴ�." << endl;

	
	// ��� ����.	
	try
	{
		itor = list.erase(itor);
	}
	catch (int expn)
	{
		if(expn == 1)
			cout << endl <<  "��� ���� or ���� ���̴� ���� �Ұ�" << endl;
	}
	
	// �ٽ� >> ��ġ�� �̵����� ������ �� ������ ���ܸ� ����
	itor++;	
	cout << endl;
	ShowList<Test*>(list, itor);	// ����Ʈ ���

	// ����Ʈ Ŭ����
	list.clear();
	cout << endl;
	ShowList<Test*>(list, itor);	// ����Ʈ ���


	return 0;
}

template <class T>
void ShowList(CList<Test*> list, CList<Test*>::iterator itor) 
{
	cout << "CList ������: " << list.size() << endl;

	for (itor = list.begin(); itor != list.end();)
	{
		cout << *(*itor) << endl;
		itor++;
	}
}

