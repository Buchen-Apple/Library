#ifndef __ARRAY_QUEUE_H__
#define __ARRAY_QUEUE_H__

#define ARRAY_SIZE	100

class arrayQueue
{
	typedef int Data;

	Data m_Array[ARRAY_SIZE];
	
	int m_iFront;
	int m_iRear;
	int m_iSize;

private:
	// ���� ��ġ Ȯ��
	int NextPos(int Pos);

public:
	// �ʱ�ȭ
	void Init();

	// ��ť
	bool Enqueue(Data data);

	// ��ť
	bool Dequeue(Data *pData);

	// Peek
	bool Peek(Data *pData);

	// ������
	int GetNodeSize();
};

#endif // !__ARRAY_QUEUE_H__
