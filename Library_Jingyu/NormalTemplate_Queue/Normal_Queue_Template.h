#ifndef __NORMAL_QUEUE_TEMPLATE_H__
#define __NORMAL_QUEUE_TEMPLATE_H__

#include <windows.h>
#include "ObjectPool\Object_Pool_LockFreeVersion.h"
#include "CrashDump\CrashDump.h"

// --------------------------
// �Ϲ� ť (���ø�). ����Ʈ ����
// ������ �������� ����.
// --------------------------
namespace Library_Jingyu
{
	template <typename DATA>
	class CNormalQueue
	{
		// --------------------
		// �̳� Ŭ����
		// --------------------

		// ��� ����ü
		struct stNode
		{
			DATA m_data;
			stNode* m_Next;		// ����
		};


		// --------------------
		// ��� ���� 
		// --------------------

		// ��� �޸�Ǯ (������ ����)
		CMemoryPool<stNode>* m_NodePool;

		// ����� ����
		stNode* m_pHead;
		stNode* m_pTail;

		// ť ���� ��� ��. (���� ����)
		LONG m_iSize;		

		// ũ���ÿ�
		CCrashDump* m_dump;	
		

	public:
		// --------------------
		// ��� �Լ�
		// --------------------

		// ������
		CNormalQueue();

		// �Ҹ���
		~CNormalQueue();

		// Enqueue
		// 
		// Parameters: (DATA) ���� ������
		// Return: ����. ���ο��� ���� �� Crash
		void Enqueue(DATA Data);

		// Dequeue
		//
		// Parameters: (DATA&) �� �����͸� ���� ������
		// return -1 : ť�� �Ҵ�� �����Ͱ� ����.
		// return 0 : �Ű������� ���������� ���� ä��.
		int Dequeue(DATA& Data);

		// ť ������ ���(���� ��� ������ ������)
		//
		// Parameter : ����
		// return : (int)ť ������.
		int GetNodeSize()
		{	return m_iSize;	}

	};

	// ������
	template <typename DATA>
	CNormalQueue<DATA>::CNormalQueue()
	{
		// ��� �޸�Ǯ �����Ҵ�
		m_NodePool = new CMemoryPool<stNode>(0, false);

		// ������
		m_dump = CCrashDump::GetInstance();

		// ���, ���� ����. 
		// ���ʴ� ���̳��.
		m_pHead = m_NodePool->Alloc();
		m_pHead->m_Next = nullptr;

		m_pTail = m_pHead;

		// ť ������ 0���� ����
		m_iSize = 0;		
	}

	// �Ҹ���
	template <typename DATA>
	CNormalQueue<DATA>::~CNormalQueue()
	{
		// 1. ť�� ��� ���� �� ��� �޸�Ǯ�� delete
		stNode* NowNode = m_pHead;
		while (1)
		{
			// �ϴ� 1�� ��ȯ. �����ڿ��� 1�� �ޱ� ������, ��ȯ���� �ص� �ȴ�.
			m_NodePool->Free(NowNode);

			// ������ �����, break;
			if (NowNode->m_Next == nullptr)
				break;

			// ������ ��尡 �ƴ϶�� �������� �̵�
			NowNode = NowNode->m_Next;
		}

		// 2. ��� �޸�Ǯ delete
		delete m_NodePool;
	}

	// Enqueue
	// 
	// Parameters: (DATA) ���� ������
	// Return: ����. ���ο��� ���� �� Crash
	template <typename DATA>
	void CNormalQueue<DATA>::Enqueue(DATA Data)
	{
		// 1. ���� ������
		stNode* LocalTail = m_pTail;

		// 2. ���ο� ��� ����� ����
		stNode* NewNode = m_NodePool->Alloc();	

		NewNode->m_data = Data;	
		NewNode->m_Next = nullptr;		

		// 3. ������ Next�� nullptr�� �ƴϸ� Crash
		if (m_pTail->m_Next != nullptr)
			m_dump->Crash();	
		
		// 4. ������ -->�� 1ĭ �̵�		
		m_pTail = NewNode;

		// 5. ���������� �޾Ƶ� ������ �̿���, ������ Next ����
		LocalTail->m_Next = NewNode;	

		// 6. ť ������ 1 ����
		InterlockedIncrement(&m_iSize);
	}


	// Dequeue
	//
	// Parameters: (DATA&) �� �����͸� ���� ������
	// return -1 : ť�� �Ҵ�� �����Ͱ� ����.
	// return 0 : �Ű������� ���������� ���� ä��.
	template <typename DATA>
	int CNormalQueue<DATA>::Dequeue(DATA& Data)
	{
		// 1. ����� Next�� nullptr�̸� �����Ͱ� �ϳ��� ���°�.
		if (m_pHead->m_Next == nullptr)
			return -1;	

		// 2. ť ������ 1 ����
		InterlockedDecrement(&m_iSize);

		// 3. �װ� �ƴ϶�� �����Ͱ� �ִ°�.
		// ����� ���->Next�� �̵�
		stNode* DeqNode = m_pHead;
		m_pHead = m_pHead->m_Next;	

		// 4. ���ڿ� ����� Next �����͸� �ִ´�
		Data = m_pHead->m_data;		

		// 5. ��ȯ�� ��尡 ���� Tail�� ���ٸ� Crash.
		// ����, Enq���� �� ��Ȳ �߻� ���ϵ��� ó���߱� ������ ���� �ȵ�.
		if (DeqNode == m_pTail)
			m_dump->Crash();

		// 6. �޸�Ǯ�� ��ȯ
		m_NodePool->Free(DeqNode);			

		return 0;
	}
	   	 
}





#endif // !__NORMAL_QUEUE_TEMPLATE_H__
