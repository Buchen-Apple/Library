#ifndef __RING_BUFF_H__
#define __RING_BUFF_H__

#include <windows.h>

namespace Library_Jingyu
{
	class CRingBuff
	{ 
	#define BUF_SIZE 8192

	private:
		char* m_Buff;
		int m_Front;
		int m_Rear;
		int m_BuffSize;

		//CRITICAL_SECTION cs;
		SRWLOCK	sl;

	private:
		// �ʱ�ȭ
		void Initial(int iBufferSize);


	public:

		//������.
		CRingBuff(void);
		CRingBuff(int iBufferSize);

		// �Ҹ���
		~CRingBuff();

		// ť�� ������ üũ�ϱ� ���� �Լ�
		int NextIndex(int iIndex, int iSize);

		// ������ �ٽ� ���
		void Resize(int size);

		// ���� ������ ���
		int	GetBufferSize(void);

		/////////////////////////////////////////////////////////////////////////
		// ���� ������� �뷮 ���.
		//
		// Parameters: ����.
		// Return: (int)������� �뷮.
		/////////////////////////////////////////////////////////////////////////
		int	GetUseSize(void);

		/////////////////////////////////////////////////////////////////////////
		// ���� ���ۿ� ���� �뷮 ���.
		//
		// Parameters: ����.
		// Return: (int)�����뷮.
		/////////////////////////////////////////////////////////////////////////
		int	GetFreeSize(void);

		/////////////////////////////////////////////////////////////////////////
		// ���� �����ͷ� �ܺο��� �ѹ濡 �а�, �� �� �ִ� ����.
		// (������ ���� ����)
		//
		// Parameters: ����.
		// Return: (int)��밡�� �뷮.
		////////////////////////////////////////////////////////////////////////
		int	GetNotBrokenGetSize(void);
		int	GetNotBrokenPutSize(void);

		/////////////////////////////////////////////////////////////////////////
		// WritePos �� ����Ÿ ����.
		//
		// Parameters: (char *)����Ÿ ������. (int)ũ��. 
		// Return: (int)���� ũ��. ť�� �� á���� -1
		/////////////////////////////////////////////////////////////////////////
		int	Enqueue(char *chpData, int iSize);

		/////////////////////////////////////////////////////////////////////////
		// ReadPos ���� ����Ÿ ������. ReadPos �̵�.
		//
		// Parameters: (char *)����Ÿ ������. (int)ũ��.
		// Return: (int)������ ũ��. ť�� ������� -1
		/////////////////////////////////////////////////////////////////////////
		int	Dequeue(char *chpDest, int iSize);

		/////////////////////////////////////////////////////////////////////////
		// ReadPos ���� ����Ÿ �о��. ReadPos ����.
		//
		// Parameters: (char *)����Ÿ ������. (int)ũ��.
		// Return: (int)������ ũ��. ť�� ������� -1
		/////////////////////////////////////////////////////////////////////////
		int	Peek(char *chpDest, int iSize);


		/////////////////////////////////////////////////////////////////////////
		// ���ϴ� ���̸�ŭ �б���ġ ���� ���� / ���� ��ġ �̵�
		//
		// Parameters: ����.
		// Return: ����.
		/////////////////////////////////////////////////////////////////////////
		int RemoveData(int iSize);
		int	MoveWritePos(int iSize);

		/////////////////////////////////////////////////////////////////////////
		// ������ ��� ����Ÿ ����.
		//
		// Parameters: ����.
		// Return: ����.
		/////////////////////////////////////////////////////////////////////////
		void ClearBuffer(void);

		/////////////////////////////////////////////////////////////////////////
		// ������ �ڿ��� ��������, 0���ε������� front������ ����
		//
		// Parameters: ����.
		// Return: 0~front������ ������ ����
		/////////////////////////////////////////////////////////////////////////
		int GetFrontSize(void);


		/////////////////////////////////////////////////////////////////////////
		// ������ ������ ����.
		//
		// Parameters: ����.
		// Return: (char *) ���� ������.
		/////////////////////////////////////////////////////////////////////////
		char *GetBufferPtr(void);

		/////////////////////////////////////////////////////////////////////////
		// ������ ReadPos ������ ����.
		//
		// Parameters: ����.
		// Return: (char *) ���� ������.
		/////////////////////////////////////////////////////////////////////////
		char *GetReadBufferPtr(void);

		/////////////////////////////////////////////////////////////////////////
		// Fornt 1ĭ �� ��ġ�� Buff �ּ� ��ȯ
		//
		// Parameters: ����.
		// Return: (char*)&Buff[m_Front ��ĭ��]
		/////////////////////////////////////////////////////////////////////////
		char* GetFrontBufferPtr(void);

		/////////////////////////////////////////////////////////////////////////
		// ������ WritePos ������ ����.
		//
		// Parameters: ����.
		// Return: (char *) ���� ������.
		/////////////////////////////////////////////////////////////////////////
		char *GetWriteBufferPtr(void);

		/////////////////////////////////////////////////////////////////////////
		// Rear 1ĭ �� ��ġ�� Buff �ּ� ��ȯ
		//
		// Parameters: ����.
		// Return: (char*)&Buff[m_Rear ��ĭ��]
		/////////////////////////////////////////////////////////////////////////
		char* GetRearBufferPtr(void);

		/////////////////////////////////////////////////////////////////////////
		// �� ����
		//
		// Parameters: ����.
		// Return: ����
		/////////////////////////////////////////////////////////////////////////
		void EnterLOCK();

		/////////////////////////////////////////////////////////////////////////
		// �� ����
		//
		// Parameters: ����.
		// Return: ����
		/////////////////////////////////////////////////////////////////////////
		void LeaveLOCK();
	};
}

#endif // !__RING_BUFF_H__