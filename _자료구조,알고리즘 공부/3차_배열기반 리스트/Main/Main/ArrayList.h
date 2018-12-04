#ifndef __ARRAY_LIST_H__
#define __ARRAY_LIST_H__

#include "NameCard.h"

// �迭 ��� ����Ʈ Ŭ����
// ������ å ���� (������ Ŭ�����θ� �ٲ�)
class ArrayList
{
	typedef NameCard* LData;

	LData* m_Array;;
	int m_iNowIndex;		// ���� ����Ű�� ���
	int m_iNodeCount;		// ������ ��� ��
	int m_iMaxNodeCount;	// �ִ� ���� ������ ���

public:
	// ����Ʈ �ʱ�ȭ.
	void ListInit(int MaxCount);

	// ����Ʈ�� ������ �ֱ�
	//
	// Parameter : ���� ������.
	// return : ����
	void ListInsert(LData data);

	// ����Ʈ�� ù ������ ��������
	//
	// Parameter : �����͸� �޾ƿ� ������
	// return :  �����Ͱ� ���� ��� false.
	//			 �����Ͱ� ���� ��� true ����
	bool ListFirst(LData* data);

	// ����Ʈ�� �ι�° ���� ������ ��������
	//
	// Parameter : �����͸� �޾ƿ� ������
	// return :  �����Ͱ� ���� ��� false.
	//			 �����Ͱ� ���� ��� true ����
	bool ListNext(LData* data);

	// ����Ʈ�� ���� ������ ����. (���� �ֱٿ� ���� ������)
	//
	// Parameter : ������ �����͸� �޾ƿ� ������
	// return :  �����Ͱ� ���� ��� false.
	//			 �����Ͱ� ���� ��� true ����
	bool ListRemove(LData* data);

	// ����Ʈ ������ ī��Ʈ ��
	int ListCount();
};




#endif // !__ARRAY_LIST_H__
