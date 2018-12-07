#ifndef __LIST_ARRAY_H__
#define __LIST_ARRAY_H__

class listStack
{
	typedef char Data;

	struct Node
	{
		Data m_data;
		Node* m_pNext;
	};

	Node* m_pTop;
	int m_iSize;

public:
	// �ʱ�ȭ
	void Init();

	// ����
	void Push(Data data);

	// ����
	bool Pop(Data *pData);

	// Peek
	bool Peek(Data* pData);

	// ������� Ȯ��
	bool IsEmpty();

	// ��� ��
	int GetNodeSize();
};

#endif // !__LIST_ARRAY_H__
