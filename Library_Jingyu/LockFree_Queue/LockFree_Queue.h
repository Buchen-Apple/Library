#ifndef __LOCKFREE_QUEUE_H__
#define __LOCKFREE_QUEUE_H__

#include <Windows.h>
#include "ObjectPool\Object_Pool_LockFreeVersion.h"
#include "CrashDump\CrashDump.h"

namespace Library_Jingyu
{
	template <typename T>
	class CLF_Queue
	{
		// ���
		struct st_LFQ_NODE
		{
			T m_Data;
			st_LFQ_NODE* m_stpNextBlock;
		};

		// tail�� ����ü
		struct st_NODE_POINT
		{
			st_LFQ_NODE* m_pPoint = nullptr;
			LONG64 m_l64Count = 0;
		};		

		// ������ ��� ��
		LONG m_NodeCount;

		// �޸�Ǯ
		CMemoryPool<st_LFQ_NODE>* m_MPool;		

		// ���� ��� ����Ű�� ������
		alignas(16) st_NODE_POINT m_stpHead;

		// ������ ��� ����Ű�� ������
		alignas(16) st_NODE_POINT m_stpTail;

		// �������� �� ũ���� ���� �뵵
		CCrashDump* m_CDump;



	public:
		// ������
		// ���� �޸�Ǯ�� �÷��̽���Ʈ �� ��뿩�� ���ڷ� ����. 
		// ����Ʈ�� false (�÷��̽���Ʈ �� ��� ����)
		CLF_Queue(int PoolCount, bool bPlacementNew = false);

		// �Ҹ���
		~CLF_Queue();

		// ���� ��� �� ���
		LONG GetInNode();

		// Enqueue
		void Enqueue(T data);

		// Dequeue
		// out �Ű�����. ���� ä���ش�.
		//
		// return -1 : ť�� �Ҵ�� �����Ͱ� ����.
		// return 0 : �Ű������� ���������� ���� ä��.
		int Dequeue(T& OutData);

	};

	// ������
	// 
	// [ť ���� �޸�Ǯ�� ��, �÷��̽���Ʈ�� ��뿩��] �� ���ڷ� �޴´�.
	// ť ���� �޸�Ǯ�� �� : 0 �Է� ��, ������
	// �÷��̽���Ʈ�� ��뿩�� : ����Ʈ�� false
	template <typename T>
	CLF_Queue<T>::CLF_Queue(int PoolCount, bool bPlacementNew)
	{
		// ���� ��� ����. ���ʴ� 0��
		m_NodeCount = 0;

		// �޸�Ǯ ������ ȣ��
		m_MPool = new CMemoryPool<st_LFQ_NODE>(PoolCount, bPlacementNew);

		// ũ���� ���� �뵵
		m_CDump = CCrashDump::GetInstance();

		// ���� ���� ��, ���̸� �ϳ� �����.
		// ���� ���̴� ����� ������ ����Ų��.
		// �� �� ������� ���̴�, �����Ͱ� ���� �����̴�.
		m_stpHead.m_pPoint = m_MPool->Alloc();
		m_stpHead.m_pPoint->m_stpNextBlock = nullptr;

		m_stpTail.m_pPoint = m_stpHead.m_pPoint;
	}

	// �Ҹ���
	template <typename T>
	CLF_Queue<T>::~CLF_Queue()
	{
		// null�� ���ö����� �޸� ��� �ݳ�. ����� �����̸鼭!
		while (m_stpHead.m_pPoint != nullptr)
		{
			st_LFQ_NODE* deleteNode = m_stpHead.m_pPoint;
			m_stpHead.m_pPoint = m_stpHead.m_pPoint->m_stpNextBlock;

			m_MPool->Free(deleteNode);
		}

		delete m_MPool;
	}

	// ���� ��� �� ���
	template <typename T>
	LONG CLF_Queue<T>::GetInNode()
	{
		return m_NodeCount;
	}


	// Enqueue
	template <typename T>
	void CLF_Queue<T>::Enqueue(T data)
	{
		st_LFQ_NODE* NewNode = m_MPool->Alloc();

		NewNode->m_Data = data;
		NewNode->m_stpNextBlock = nullptr;

		// ������ ���� (ī���� ���) ------------
		alignas(16) st_NODE_POINT LocalTail;

		while (true)
		{
			// tail ������
			LocalTail.m_l64Count = m_stpTail.m_l64Count;
			LocalTail.m_pPoint = m_stpTail.m_pPoint;		

			// LocalTail�� Next ������
			st_LFQ_NODE* LocalNext = LocalTail.m_pPoint->m_stpNextBlock;

			// ���� m_stpTail�� LocalTail�� ���ٸ� ���� ����
			if (LocalTail.m_l64Count == m_stpTail.m_l64Count)
			{
				// Next�� Null�̸� ���� ����
				if (LocalNext == nullptr)
				{
					// ���� ���� �õ�
					if (InterlockedCompareExchange64((LONG64*)&m_stpTail.m_pPoint->m_stpNextBlock, (LONG64)NewNode, (LONG64)nullptr) == (LONG64)nullptr)
					{
						// ������ ����Ǹ� Tail�̵� �õ�
						InterlockedCompareExchange128((LONG64*)&m_stpTail, LocalTail.m_l64Count + 1, (LONG64)NewNode, (LONG64*)&LocalTail);
						break;
					}
				}

				else
				{
					// null�� �ƴ϶�� Tail�̵��۾� �õ�				
					InterlockedCompareExchange128((LONG64*)&m_stpTail, LocalTail.m_l64Count + 1, (LONG64)LocalNext, (LONG64*)&LocalTail);
				}
			}

		}

		// ť ������ ������ 1 ����
		InterlockedIncrement(&m_NodeCount);
	}

	// Dequeue
	// out �Ű�����. ���� ä���ش�.
	//
	// return -1 : ť�� �Ҵ�� �����Ͱ� ����.
	// return 0 : �Ű������� ���������� ���� ä��.
	template <typename T>
	int CLF_Queue<T>::Dequeue(T& OutData)
	{
		// ť ������ ������ 1 ����
		// ��尡 ������ 1 �ø��� ����
		// !! ���ÿ� 2���� �����尡 ����� 1�� ���¿��� ��ť�� �õ��ϸ� ������ ���� !!
		// !! ���� ���� ����� �����尡 0���� �����, �ٸ� �����尡 -1�� ����� ���� !!
		// !! ��, �������� ��Ȳ�ε� ��� ī��Ʈ�� -1�� ��. !!
		// !! ��� ���� -1�̸�, �ٽ� 1 �÷��� 0���� ����� ������. !!
		if (InterlockedDecrement(&m_NodeCount) < 0)
		{
			InterlockedIncrement(&m_NodeCount);
			return 0;
		}	

		// ������ ���� --------------------------
		// ��ť��, �⺻������ ���� ��尡 ����Ű�� �ִ� �����, ���� ����� ���� �����Ѵ�.
		// ���� ������ ���̸� ����Ű�� ���̴�.
		alignas(16) st_NODE_POINT LocalHead, LocalTail;

		while (true)
		{
			// head ������
			LocalHead.m_l64Count = m_stpHead.m_l64Count;
			LocalHead.m_pPoint = m_stpHead.m_pPoint;			

			// tail ������
			LocalTail.m_l64Count = m_stpTail.m_l64Count;
			LocalTail.m_pPoint = m_stpTail.m_pPoint;	

			// LocalHead�� Next ������
			st_LFQ_NODE* LocalHead_Next = LocalHead.m_pPoint->m_stpNextBlock;	

			// ���� m_stpHead�� LocalHead�� ���ٸ� ���� ����
			if (m_stpHead.m_l64Count == LocalHead.m_l64Count)
			{
				// ����� ������ �����鼭
				if (LocalHead.m_pPoint == LocalTail.m_pPoint)
				{
					// LocalHead->Next�� null���� üũ
					if (LocalHead_Next == nullptr)
						continue;

					// �װ� �ƴϸ� tail �̵� �õ�	
					else
						InterlockedCompareExchange128((LONG64*)&m_stpTail, LocalTail.m_l64Count + 1, (LONG64)LocalTail.m_pPoint->m_stpNextBlock, (LONG64*)&LocalTail);
				}

				// ����� ������ �ٸ��� ��ť�۾� ����
				else
				{
					// LocalHead->Next�� null���� üũ
					if (LocalHead_Next == nullptr)
						continue;

					// ������ ������ �޾Ƶα� ------
					OutData = LocalHead_Next->m_Data;

					// ��� �̵� �õ�
					if (InterlockedCompareExchange128((LONG64*)&m_stpHead, LocalHead.m_l64Count + 1, (LONG64)LocalHead_Next, (LONG64*)&LocalHead))
					{
						// �̵� ���� ����� �޸�Ǯ�� ��ȯ
						m_MPool->Free(LocalHead.m_pPoint);
						break;
					}
				}
			}
		}

		// ���������� 0 ����
		return 0;
	}

}



#endif // !__LOCKFREE_QUEUE_H__
