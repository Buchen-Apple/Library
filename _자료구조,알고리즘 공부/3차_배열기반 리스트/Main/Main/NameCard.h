#ifndef __NAME_CARD_H__
#define __NAME_CARD_H__

#define NAME_LEN	30
#define PHONE_LEN	30

class NameCard
{
	char m_cName[NAME_LEN];		// �̸�
	char m_cPhone[PHONE_LEN];	// �̸��� �����Ǵ� �� ��ȣ

public:
	// �̸�, ����ȣ ���
	void ShowInfo();

	// �̸� ��
	// ������ 0, �ٸ��� 0�� �ƴ� �� ����
	int NameCompare(const char* Name);

	// ��ȭ��ȣ ����
	void ChangePhoneNum(const char* Phone);

	// ������
	NameCard(const char* Name, const char* Phone);
};


// NameCard �����Ҵ� �� �̸�,����ȣ ����
NameCard* CreateNameCard(const char* Name, const char* Phone);

#endif // !__NAME_CARD_H__
