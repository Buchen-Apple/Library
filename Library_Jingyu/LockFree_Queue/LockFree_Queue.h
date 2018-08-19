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
		// 노드
		struct st_LFQ_NODE
		{
			T m_Data;
			st_LFQ_NODE* m_stpNextBlock;
		};

		// tail용 구조체
		struct st_NODE_POINT
		{
			st_LFQ_NODE* m_pPoint = nullptr;
			LONG64 m_l64Count = 0;
		};

		// 내부의 노드 수
		LONG m_NodeCount;

		// 메모리풀
		CMemoryPool<st_LFQ_NODE>* m_MPool;

		// 에러났을 때 크래시 내는 용도
		CCrashDump* m_CDump;

		// 시작 노드 가리키는 포인터
		alignas(16) st_NODE_POINT m_stpHead;

		// 마지막 노드 가리키는 포인터
		alignas(16) st_NODE_POINT m_stpTail;



	public:
		// 생성자
		// 내부 메모리풀의 플레이스먼트 뉴 사용여부 인자로 받음. 
		// 디폴트는 false (플레이스먼트 뉴 사용 안함)
		CLF_Queue(int PoolCount, bool bPlacementNew = false);

		// 소멸자
		~CLF_Queue();

		// 내부 노드 수 얻기
		LONG GetInNode();

		// Enqueue
		void Enqueue(T data);

		// Dequeue
		// out 매개변수. 값을 채워준다.
		//
		// return -1 : 큐에 할당된 데이터가 없음.
		// return 0 : 매개변수에 성공적으로 값을 채움.
		int Dequeue(T& OutData);

	};

	// 생성자
	// 
	// [큐 내부 메모리풀의 수, 플레이스먼트뉴 사용여부] 를 인자로 받는다.
	// 큐 내부 메모리풀의 수 : 0 입력 시, 무제한
	// 플레이스먼트뉴 사용여부 : 디폴트로 false
	template <typename T>
	CLF_Queue<T>::CLF_Queue(int PoolCount, bool bPlacementNew)
	{
		// 내부 노드 셋팅. 최초는 0개
		m_NodeCount = 0;

		// 메모리풀 생성자 호출
		m_MPool = new CMemoryPool<st_LFQ_NODE>(PoolCount, bPlacementNew);

		// 크래시 내는 용도
		m_CDump = CCrashDump::GetInstance();

		// 최초 생성 시, 더미를 하나 만든다.
		// 만든 더미는 헤더와 테일이 가리킨다.
		// 이 때 만들어진 더미는, 데이터가 없는 더미이다.
		m_stpHead.m_pPoint = m_MPool->Alloc();
		m_stpHead.m_pPoint->m_stpNextBlock = nullptr;

		m_stpTail.m_pPoint = m_stpHead.m_pPoint;
	}

	// 소멸자
	template <typename T>
	CLF_Queue<T>::~CLF_Queue()
	{
		// null이 나올때까지 메모리 모두 반납. 헤더를 움직이면서!
		while (m_stpHead.m_pPoint != nullptr)
		{
			st_LFQ_NODE* deleteNode = m_stpHead.m_pPoint;
			m_stpHead.m_pPoint = m_stpHead.m_pPoint->m_stpNextBlock;

			m_MPool->Free(deleteNode);
		}

		delete m_MPool;
	}

	// 내부 노드 수 얻기
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

		// 락프리 구조 (카운터 사용) ------------
		alignas(16) st_NODE_POINT LocalTail;

		while (true)
		{
			// tail 스냅샷
			LocalTail.m_pPoint = m_stpTail.m_pPoint;
			LocalTail.m_l64Count = m_stpTail.m_l64Count;

			// 정말 m_stpTail이 LocalTail와 같다면 로직 진행
			if (LocalTail.m_pPoint == m_stpTail.m_pPoint)
			{
				// Next가 Null이면 로직 진행
				if (LocalTail.m_pPoint->m_stpNextBlock == nullptr)
				{
					// 라인 연결 시도
					if (InterlockedCompareExchange64((LONG64*)&m_stpTail.m_pPoint->m_stpNextBlock, (LONG64)NewNode, (LONG64)nullptr) == (LONG64)nullptr)
					{
						// 라인이 연결되면 Tail이동 시도
						InterlockedCompareExchange128((LONG64*)&m_stpTail, LocalTail.m_l64Count + 1, (LONG64)NewNode, (LONG64*)&LocalTail);
						break;
					}
				}

				else
				{
					// null이 아니라면 Tail이동작업 시도				
					InterlockedCompareExchange128((LONG64*)&m_stpTail, LocalTail.m_l64Count + 1, (LONG64)LocalTail.m_pPoint->m_stpNextBlock, (LONG64*)&LocalTail);
				}
			}

		}

		// 큐 내부의 사이즈 1 증가
		InterlockedIncrement(&m_NodeCount);
	}

	// Dequeue
	// out 매개변수. 값을 채워준다.
	//
	// return -1 : 큐에 할당된 데이터가 없음.
	// return 0 : 매개변수에 성공적으로 값을 채움.
	template <typename T>
	int CLF_Queue<T>::Dequeue(T& OutData)
	{
		// 큐 내부의 사이즈 1 감소
		// 노드가 없으면 -1 리턴
		if (InterlockedDecrement(&m_NodeCount) < 0)
			return -1;

		// 락프리 구조 --------------------------
		// 디큐는, 기본적으로 현재 헤드가 가리키고 있는 노드의, 다음 노드의 값을 리턴한다.
		// 헤드는 무조건 더미를 가리키는 것이다.
		alignas(16) st_NODE_POINT LocalHead, LocalTail;
		st_LFQ_NODE *pDeleteHead;

		while (true)
		{
			// head 스냅샷
			pDeleteHead = LocalHead.m_pPoint = m_stpHead.m_pPoint;
			LocalHead.m_l64Count = m_stpHead.m_l64Count;

			// tail 스냅샷
			LocalTail.m_pPoint = m_stpTail.m_pPoint;
			LocalTail.m_l64Count = m_stpTail.m_l64Count;

			// 정말 m_stpHead이 LocalHead와 같다면 로직 진행
			if (m_stpHead.m_pPoint == pDeleteHead)
			{
				// 헤더와 테일이 같으면서
				if (pDeleteHead == LocalTail.m_pPoint)
				{
					// pLocalNext가 null인지 체크
					if (pDeleteHead->m_stpNextBlock == nullptr)
					{
						continue;
						//m_CDump->Crash();
						//return - 1;
					}

					else
					{
						// tail 이동 시도	
						InterlockedCompareExchange128((LONG64*)&m_stpTail, LocalTail.m_l64Count + 1, (LONG64)LocalTail.m_pPoint->m_stpNextBlock, (LONG64*)&LocalTail);
					}
				}

				// 헤더와 테일이 다르면 디큐작업 진행
				else
				{

					// 헤더 이동 시도
					if (InterlockedCompareExchange128((LONG64*)&m_stpHead, LocalHead.m_l64Count + 1, (LONG64)LocalHead.m_pPoint->m_stpNextBlock, (LONG64*)&LocalHead))
					{
						// 성공시 ------
						OutData = pDeleteHead->m_stpNextBlock->m_Data;

						// 이동 전의 헤더를 메모리풀에 반환
						m_MPool->Free(pDeleteHead);
						break;
					}
				}
			}
		}

		// 성공했으니 0 리턴
		return 0;
	}

}



#endif // !__LOCKFREE_QUEUE_H__
