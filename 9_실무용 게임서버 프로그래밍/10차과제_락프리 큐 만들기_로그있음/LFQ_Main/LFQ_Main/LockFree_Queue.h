#ifndef __LOCKFREE_QUEUE_H__
#define __LOCKFREE_QUEUE_H__

#include <Windows.h>
#include "ObjectPool\Object_Pool_LockFreeVersion.h"
#include "CrashDump\CrashDump.h"
#include <list>

using namespace std;

#define LOG_COUNT 160000
#define LOG_CLEAR_TIME	100


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

		// �޸� �α��
		struct st_Trace
		{
			// flag 
			// 1: ��ť���� ���� ���� �õ�, 2 : ����
			// 3: ��ť���� tail�̵� �õ�, 4 : ����
			// 5: ��ť���� tail �̵� �õ�, 6 : ����
			// 7: ��ť���� ��� �̵� �õ�, 8: ����
			// 9: ��ť���� tail�̵� �õ�2, 10 : ����2
			BOOL m_bFlag;

			ULONGLONG m_TempCount = 0;

			// ���, ������� �ּ�
			st_LFQ_NODE* m_head;
			st_LFQ_NODE* m_head_Next;
			st_LFQ_NODE* m_Localhead;

			// ����, �������� �ּ�
			st_LFQ_NODE* m_tail;
			st_LFQ_NODE* m_tail_Next;
			st_LFQ_NODE* m_Localtail;

			// �̹��� Tail�� ������ newNode
			st_LFQ_NODE* m_newNode;
		};

		// ������
		list<st_Trace> TraceArray[5];
		ULONGLONG TempCount = 0;
		ULONGLONG Time[5];

		// ������ ��� ��
		LONG m_NodeCount;

		// �޸�Ǯ
		CMemoryPool<st_LFQ_NODE>* m_MPool;

		// �������� �� ũ���� ���� �뵵
		CCrashDump* m_CDump;

		// ���� ��� ����Ű�� ������
		alignas(16) st_NODE_POINT m_stpHead;

		// ������ ��� ����Ű�� ������
		alignas(16) st_NODE_POINT m_stpTail;



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
		void Enqueue(T data, int ID);

		// Dequeue
		// out �Ű�����. ���� ä���ش�.
		//
		// return -1 : ť�� �Ҵ�� �����Ͱ� ����.
		// return 0 : �Ű������� ���������� ���� ä��.
		int Dequeue(T& OutData, int ID);

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
		m_stpHead.m_pPoint = m_MPool->Alloc();
		m_stpHead.m_pPoint->m_stpNextBlock = nullptr;

		m_stpTail.m_pPoint = m_stpHead.m_pPoint;

		// ������ !!
		for(int i=0; i<5; ++i)
			Time[i] = GetTickCount64();
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
	void CLF_Queue<T>::Enqueue(T data, int ID)
	{
		ULONGLONG TempTime = GetTickCount64();
		if (Time[ID] + LOG_CLEAR_TIME <= TempTime)
		{
			TraceArray[ID].clear();
			Time[ID] = TempTime;
		}

		st_LFQ_NODE* NewNode = m_MPool->Alloc();

		NewNode->m_Data = data;
		NewNode->m_stpNextBlock = nullptr;

		// ������ ���� (ī���� ���) ------------
		alignas(16) st_NODE_POINT LocalTail;

		while (true)
		{
			// tail ������
			LocalTail.m_pPoint = m_stpTail.m_pPoint;
			LocalTail.m_l64Count = m_stpTail.m_l64Count;

			// ���� m_stpTail�� LocalTail�� ���ٸ� ���� ����
			if (LocalTail.m_pPoint == m_stpTail.m_pPoint)
			{
				// Next�� Null�̸� ���� ����
				if (LocalTail.m_pPoint->m_stpNextBlock == nullptr)
				{
					// ���� �õ� �α�
					st_Trace Trace;
					Trace.m_bFlag = 1;

					Trace.m_head = m_stpHead.m_pPoint;
					Trace.m_tail = m_stpTail.m_pPoint;

					Trace.m_head_Next = Trace.m_head->m_stpNextBlock;
					Trace.m_tail_Next = Trace.m_tail->m_stpNextBlock;

					Trace.m_Localhead = nullptr;
					Trace.m_Localtail = LocalTail.m_pPoint;

					Trace.m_newNode = NewNode;

					Trace.m_TempCount = InterlockedIncrement(&TempCount);

					TraceArray[ID].push_back(Trace);

					if (InterlockedCompareExchange64((LONG64*)&m_stpTail.m_pPoint->m_stpNextBlock, (LONG64)NewNode, (LONG64)nullptr) == (LONG64)nullptr)
					{
						// ���� ���� �α�
						st_Trace Trace;
						Trace.m_bFlag = 2;

						Trace.m_head = m_stpHead.m_pPoint;
						Trace.m_tail = m_stpTail.m_pPoint;

						Trace.m_head_Next = Trace.m_head->m_stpNextBlock;
						Trace.m_tail_Next = Trace.m_tail->m_stpNextBlock;

						Trace.m_Localhead = nullptr;
						Trace.m_Localtail = LocalTail.m_pPoint;

						Trace.m_newNode = NewNode;

						Trace.m_TempCount = InterlockedIncrement(&TempCount);

						TraceArray[ID].push_back(Trace);



						// ���������� ���� Tail �̵��۾�
						// ��� �׳� �õ��Ѵ�. ���� �ٲ����� �����ϰ� ��.		

						// �̵� �õ� �α�
						Trace.m_bFlag = 3;

						Trace.m_head = m_stpHead.m_pPoint;
						Trace.m_tail = m_stpTail.m_pPoint;

						Trace.m_head_Next = Trace.m_head->m_stpNextBlock;
						Trace.m_tail_Next = Trace.m_tail->m_stpNextBlock;

						Trace.m_Localhead = nullptr;
						Trace.m_Localtail = LocalTail.m_pPoint;

						Trace.m_newNode = NewNode;

						Trace.m_TempCount = InterlockedIncrement(&TempCount);

						TraceArray[ID].push_back(Trace);

						if (InterlockedCompareExchange128((LONG64*)&m_stpTail, LocalTail.m_l64Count + 1, (LONG64)NewNode, (LONG64*)&LocalTail) == TRUE)
						{
							// �̵� ���� �α�
							st_Trace Trace;
							Trace.m_bFlag = 4;

							Trace.m_head = m_stpHead.m_pPoint;
							Trace.m_tail = m_stpTail.m_pPoint;

							Trace.m_head_Next = Trace.m_head->m_stpNextBlock;
							Trace.m_tail_Next = Trace.m_tail->m_stpNextBlock;

							Trace.m_Localhead = nullptr;
							Trace.m_Localtail = LocalTail.m_pPoint;

							Trace.m_newNode = NewNode;

							Trace.m_TempCount = InterlockedIncrement(&TempCount);

							TraceArray[ID].push_back(Trace);
						}

						break;
					}
				}

				else
				{
					// null�� �ƴ϶�� Tail�̵��۾� �õ�

					// �̵� �õ� �α�
					st_Trace Trace;
					Trace.m_bFlag = 9;

					Trace.m_head = m_stpHead.m_pPoint;
					Trace.m_tail = m_stpTail.m_pPoint;

					Trace.m_head_Next = Trace.m_head->m_stpNextBlock;
					Trace.m_tail_Next = Trace.m_tail->m_stpNextBlock;

					Trace.m_Localhead = nullptr;
					Trace.m_Localtail = LocalTail.m_pPoint;

					Trace.m_newNode = NewNode;

					Trace.m_TempCount = InterlockedIncrement(&TempCount);

					TraceArray[ID].push_back(Trace);

					if (InterlockedCompareExchange128((LONG64*)&m_stpTail, LocalTail.m_l64Count + 1, (LONG64)LocalTail.m_pPoint->m_stpNextBlock, (LONG64*)&LocalTail) == TRUE)
					{
						// �̵� ���� �α�
						st_Trace Trace;
						Trace.m_bFlag = 10;

						Trace.m_head = m_stpHead.m_pPoint;
						Trace.m_tail = m_stpTail.m_pPoint;

						Trace.m_head_Next = Trace.m_head->m_stpNextBlock;
						Trace.m_tail_Next = Trace.m_tail->m_stpNextBlock;

						Trace.m_Localhead = nullptr;
						Trace.m_Localtail = LocalTail.m_pPoint;

						Trace.m_newNode = NewNode;

						Trace.m_TempCount = InterlockedIncrement(&TempCount);

						TraceArray[ID].push_back(Trace);
					}
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
	int CLF_Queue<T>::Dequeue(T& OutData, int ID)
	{
		ULONGLONG TempTime = GetTickCount64();
		if (Time[ID] + LOG_CLEAR_TIME <= TempTime)
		{
			TraceArray[ID].clear();
			Time[ID] = TempTime;
		}

		// ť ������ ������ 1 ����
		// ��尡 ������ -1 ����
		if (InterlockedDecrement(&m_NodeCount) < 0)
			return -1;

		// ������ ���� --------------------------
		// ��ť��, �⺻������ ���� ��尡 ����Ű�� �ִ� �����, ���� ����� ���� �����Ѵ�.
		// ���� ������ ���̸� ����Ű�� ���̴�.
		alignas(16) st_NODE_POINT LocalHead, LocalTail;
		st_LFQ_NODE *pDeleteHead;

		while (true)
		{
			// head ������
			pDeleteHead = LocalHead.m_pPoint = m_stpHead.m_pPoint;
			LocalHead.m_l64Count = m_stpHead.m_l64Count;

			// tail ������
			LocalTail.m_pPoint = m_stpTail.m_pPoint;
			LocalTail.m_l64Count = m_stpTail.m_l64Count;

			// ���� m_stpHead�� LocalHead�� ���ٸ� ���� ����
			if (m_stpHead.m_pPoint == pDeleteHead)
			{
				// ����� ������ �����鼭
				if (pDeleteHead == LocalTail.m_pPoint)
				{
					// pLocalNext�� null���� üũ
					if (pDeleteHead->m_stpNextBlock == nullptr)
					{
						continue;
						//m_CDump->Crash();
						//return - 1;
					}

					else
					{
						// tail �̵� �õ�	

						// �̵� �õ� �α�
						st_Trace Trace;
						Trace.m_bFlag = 5;

						Trace.m_head = m_stpHead.m_pPoint;
						Trace.m_tail = m_stpTail.m_pPoint;

						Trace.m_head_Next = Trace.m_head->m_stpNextBlock;
						Trace.m_tail_Next = Trace.m_tail->m_stpNextBlock;

						Trace.m_Localhead = LocalHead.m_pPoint;
						Trace.m_Localtail = LocalTail.m_pPoint;

						Trace.m_newNode = nullptr;

						Trace.m_TempCount = InterlockedIncrement(&TempCount);

						TraceArray[ID].push_back(Trace);

						if (InterlockedCompareExchange128((LONG64*)&m_stpTail, LocalTail.m_l64Count + 1, (LONG64)LocalTail.m_pPoint->m_stpNextBlock, (LONG64*)&LocalTail) == TRUE)
						{
							// �̵� ���� �α�
							st_Trace Trace;
							Trace.m_bFlag = 6;

							Trace.m_head = m_stpHead.m_pPoint;
							Trace.m_tail = m_stpTail.m_pPoint;

							Trace.m_head_Next = Trace.m_head->m_stpNextBlock;
							Trace.m_tail_Next = Trace.m_tail->m_stpNextBlock;

							Trace.m_Localhead = LocalHead.m_pPoint;
							Trace.m_Localtail = LocalTail.m_pPoint;

							Trace.m_newNode = nullptr;

							Trace.m_TempCount = InterlockedIncrement(&TempCount);

							TraceArray[ID].push_back(Trace);
						}
					}
				}

				// ����� ������ �ٸ��� ��ť�۾� ����
				else
				{
					// ��� �̵� �õ�

					// �̵� �õ� �α�
					st_Trace Trace;
					Trace.m_bFlag = 7;

					Trace.m_head = m_stpHead.m_pPoint;
					Trace.m_tail = m_stpTail.m_pPoint;

					Trace.m_head_Next = Trace.m_head->m_stpNextBlock;
					Trace.m_tail_Next = Trace.m_tail->m_stpNextBlock;

					Trace.m_Localhead = LocalHead.m_pPoint;
					Trace.m_Localtail = LocalTail.m_pPoint;

					Trace.m_newNode = nullptr;

					Trace.m_TempCount = InterlockedIncrement(&TempCount);

					TraceArray[ID].push_back(Trace);

					if (InterlockedCompareExchange128((LONG64*)&m_stpHead, LocalHead.m_l64Count + 1, (LONG64)LocalHead.m_pPoint->m_stpNextBlock, (LONG64*)&LocalHead))
					{
						// �̵� ���� �α�
						st_Trace Trace;
						Trace.m_bFlag = 8;

						Trace.m_head = m_stpHead.m_pPoint;
						Trace.m_tail = m_stpTail.m_pPoint;

						Trace.m_head_Next = Trace.m_head->m_stpNextBlock;
						Trace.m_tail_Next = Trace.m_tail->m_stpNextBlock;

						Trace.m_Localhead = LocalHead.m_pPoint;
						Trace.m_Localtail = LocalTail.m_pPoint;

						Trace.m_newNode = nullptr;

						Trace.m_TempCount = InterlockedIncrement(&TempCount);

						TraceArray[ID].push_back(Trace);

						// out �Ű������� ������ ����
						OutData = pDeleteHead->m_stpNextBlock->m_Data;

						// �̵� ���� ����� �޸�Ǯ�� ��ȯ
						m_MPool->Free(pDeleteHead);
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
