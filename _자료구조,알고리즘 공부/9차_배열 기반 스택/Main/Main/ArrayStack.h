#ifndef __ARRAY_STACK_H__
#define __ARRAY_STACK_H__


#define SIZE 100

class arrayStack
{
	typedef int Data;

	Data m_stackArr[SIZE];
	int m_iTop;
	int m_iSize;


public:
	// �ʱ�ȭ
	void Init();

	// ����
	bool Push(Data data);

	// ����
	bool Pop(Data *pData);

	// Peek
	bool  Peek(Data *pData);
	
	// ������� üũ
	bool IsEmpty();

	// ������ ������ ��
	int GetNodeSize();
};

#endif // !__ARRAY_STACK_H__
