#ifndef __LOCKFREE_QUEUE_H__
#define __LOCKFREE_QUEUE_H__

#include <Windows.h>
#include "ObjectPool\Object_Pool_LockFreeVersion.h"
#include "CrashDump\CrashDump.h"
#include <list>

using namespace std;

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

			st_LFQ_NODE& operator=(const st_LFQ_NODE& TempNode)
			{
				m_Data = TempNode.m_Data;
				m_stpNextBlock = TempNode.m_stpNextBlock;
				return *this;
			}

			bool operator==(const st_LFQ_NODE& TempNode)
			{
				return(m_Data == TempNode.m_Data && m_stpNextBlock == TempNode.m_stpNextBlock);
			}
		};

		// ������ ��� ��
		LONG m_NodeCount;

		// �޸�Ǯ
		CMemoryPool<st_LFQ_NODE>* m_MPool;

		// �������� �� ũ���� ���� �뵵
		CCrashDump* m_CDump;

		// ���� ��� ����Ű�� ������
		st_LFQ_NODE* m_stpHead;

		// ������ ��� ����Ű�� ������
		st_LFQ_NODE* m_stpTail;



	public:
		// ������
		// ���� �޸�Ǯ�� �÷��̽���Ʈ �� ��뿩�� ���ڷ� ����. 
		// ����Ʈ�� false (�÷��̽���Ʈ �� ��� ����)
		CLF_Queue(bool bPlacementNew = false);

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
	// ���� �޸�Ǯ�� �÷��̽���Ʈ �� ��뿩�� ���ڷ� ����. 
	// ����Ʈ�� false (�÷��̽���Ʈ �� ��� ����)
	template <typename T>
	CLF_Queue<T>::CLF_Queue(bool bPlacementNew)
	{
		// ���� ��� ����. ���ʴ� 0��
		m_NodeCount = 0;

		// �޸�Ǯ ������ ȣ��
		m_MPool = new CMemoryPool<st_LFQ_NODE>(0, bPlacementNew);

		// ũ���� ���� �뵵
		m_CDump = CCrashDump::GetInstance();

		// ���� ���� ��, ���̸� �ϳ� �����.
		// ���� ���̴� ����� ������ ����Ų��.
		// �� �� ������� ���̴�, �����Ͱ� ���� �����̴�.
		m_stpHead = m_MPool->Alloc();
		m_stpTail = m_stpHead;

		// ������ Next�� null�̴�.			
		m_stpTail->m_stpNextBlock = nullptr;
	}

	// �Ҹ���
	template <typename T>
	CLF_Queue<T>::~CLF_Queue()
	{
		// null�� ���ö����� �޸� ��� �ݳ�. ����� �����̸鼭!
		while (m_stpHead != nullptr)
		{
			st_LFQ_NODE* deleteNode = m_stpHead;
			m_stpHead = m_stpHead->m_stpNextBlock;

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

		st_LFQ_NODE* pLocalTail;
		st_LFQ_NODE* pLocalNext;

		while (true)
		{
			// tail ������			
			pLocalTail = m_stpTail;

			// Next ������		
			pLocalNext = pLocalTail->m_stpNextBlock;

			// ������ ���� ----------------------------
			// tail�� localtail�� ������
			if (pLocalTail == m_stpTail && 
				pLocalTail->m_stpNextBlock == m_stpTail->m_stpNextBlock)
			{
				// �� �� Next�� Null�̶�� ���� ����
				if (pLocalNext == nullptr)
				{
					if (InterlockedCompareExchangePointer((PVOID*)&pLocalTail->m_stpNextBlock, NewNode, pLocalNext) == pLocalNext)
					{
						// tail �̵� �õ�
						InterlockedCompareExchangePointer((PVOID*)&m_stpTail, NewNode, pLocalTail);
						break;
					}
				}

				// Null�� �ƴ϶�� tail �̵��� �õ�
				else
				{
					InterlockedCompareExchangePointer((PVOID*)&m_stpTail, pLocalNext, pLocalTail);
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

		// ��尡 ������ -1 ����
		if (m_NodeCount == 0)
			return -1;

		// ������ ���� ----------------------------
		st_LFQ_NODE* pLocalHead;
		st_LFQ_NODE* pLocalHead_Next;
		st_LFQ_NODE* pLocalTail;
		while (true)
		{
			// ���, ���->Next, ���� ����
			pLocalHead = m_stpHead;
			pLocalHead_Next = pLocalHead->m_stpNextBlock;
			pLocalTail = m_stpTail;

			// ����, ����� ������ ��¥ ������
			if (pLocalHead == m_stpHead && 
				pLocalHead->m_stpNextBlock == m_stpHead->m_stpNextBlock)
			{
				// ����� Next�� ������ üũ�ؾ��Ѵ�.
				// �� ��, ���� null������ ����� ������ ����, next�� null�̸� �׶� ���� ���°�.
				if (pLocalHead == pLocalTail && 
					pLocalHead->m_stpNextBlock == pLocalTail->m_stpNextBlock && 
					pLocalHead_Next == nullptr)
				{
					m_CDump->Crash();
					//return - 1;
				}

				// ����� null�� �ƴ϶�� ���� ����
				else
				{
					// �Ű������� ���� ����.
					OutData = pLocalHead_Next->m_Data;

					// ��� �̵� �õ�
					if (InterlockedCompareExchangePointer((PVOID*)&m_stpHead, pLocalHead_Next, pLocalHead) == pLocalHead)
					{
						// ����� ���������� �̵��Ǿ��ٸ�, tail�̵� �õ�
						//InterlockedCompareExchangePointer((PVOID*)&m_stpTail, pLocalHead_Next, pLocalHead);

						// ���� ��� ��ȯ.
						m_MPool->Free(pLocalHead);
						break;
					}
				}

			}

		}

		// ť ������ ������ 1 ����
		InterlockedDecrement(&m_NodeCount);

		// ���������� 0 ����
		return 0;
	}

}



#endif // !__LOCKFREE_QUEUE_H__
