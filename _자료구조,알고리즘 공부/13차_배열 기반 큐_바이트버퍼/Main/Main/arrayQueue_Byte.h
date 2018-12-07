#ifndef __ARRAY_QUEUE_BYTE_H__
#define __ARRAY_QUEUE_BYTE_H__

#define BUF_SIZE	5

class arrayQueue_Byte
{
	char m_cBuff[BUF_SIZE];
	int m_iFront;
	int m_iRear;
	int m_iSize;


public:
	// �ʱ�ȭ
	void Init();

	// ��ť
	bool Enqueue(char* Data, int Size);

	// ��ť
	bool Dequeue(char* Data, int Size);

	// ��
	bool Peek(char* Data, int Size);

	// ��� ���� ������
	int GetUseSize();

	// ��� ������ ������
	int GetFreeSize();

	// �� ���� ��ť ������ ����(������ �ʰ�)
	int GetNotBrokenSize_Enqueue();

	// �� ���� ��ť ������ ����(������ �ʰ�)
	int GetNotBrokenSize_Dequeue();
};

#endif // !__ARRAY_QUEUE_BYTE_H__
