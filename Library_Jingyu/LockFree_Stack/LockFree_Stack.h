#ifndef __LOCKFREE_STACK_H__
#define __LOCKFREE_STACK_H__

#include <Windows.h>
#include "ObjectPool\Object_Pool_LockFreeVersion.h"
#include "CrashDump\CrashDump.h"

namespace Library_Jingyu
{
	template <typename T>
	class CLF_Stack
	{
	private:

		LONG m_NodeCount;		// ����Ʈ ������ ��� ��

		struct st_LFS_NODE
		{
			T m_Data;
			st_LFS_NODE* m_stpNextBlock;
		};

		struct st_TOP
		{
			st_LFS_NODE* m_pTop = nullptr;
			LONG64 m_l64Count = 0;
		};

		alignas(16)	st_TOP m_stpTop;

		// �������� �� ũ���� ���� �뵵
		CCrashDump* m_CDump;

		// �޸�Ǯ
		CMemoryPool<st_LFS_NODE>* m_MPool;


	public:
		// ������
		// ���� �޸�Ǯ�� �÷��̽���Ʈ �� ��뿩�� ���ڷ� ����. 
		// ����Ʈ�� false (�÷��̽���Ʈ �� ��� ����)
		CLF_Stack(bool bPlacementNew = false);

		// �Ҹ���
		~CLF_Stack();

		// ���� ��� �� ���
		LONG GetInNode();

		// �ε����� ���ÿ� �߰�
		//
		// return : ���� (void)
		void Push(T Data);


		// �ε��� ���
		//
		// ���� ��, T ����
		// ���н� Crash �߻�
		T Pop();
	};


	// ������
	// ���� �޸�Ǯ�� �÷��̽���Ʈ �� ��뿩�� ���ڷ� ����. 
	// ����Ʈ�� false (�÷��̽���Ʈ �� ��� ����)
	template <typename T>
	CLF_Stack<T>::CLF_Stack(bool bPlacementNew)
	{
		m_NodeCount = 0;
		m_CDump = CCrashDump::GetInstance();
		m_MPool = new CMemoryPool<st_LFS_NODE>(0, bPlacementNew);
	}

	// �Ҹ���
	template <typename T>
	CLF_Stack<T>::~CLF_Stack()
	{
		// null�� ���ö����� �޸� ��� �ݳ�
		while (m_stpTop.m_pTop != nullptr)
		{
			st_LFS_NODE* deleteNode = m_stpTop.m_pTop;
			m_stpTop.m_pTop = m_stpTop.m_pTop->m_stpNextBlock;

			m_MPool->Free(deleteNode);
		}

		delete m_MPool;
	}

	// ���� ��� �� ���
	template <typename T>
	LONG CLF_Stack<T>::GetInNode()
	{
		return m_NodeCount;
	}


	// �ε����� ���ÿ� �߰�
	//
	// return : ���� (void)
	template <typename T>
	void CLF_Stack<T>::Push(T Data)
	{
		// ���ο� ��� �Ҵ���� ��, ������ ����
		st_LFS_NODE* NewNode = m_MPool->Alloc();
		NewNode->m_Data = Data;

		// ---- ������ ���� ----
		alignas(16)  st_TOP localTop;
		do
		{
			// ���� Top ����
			localTop.m_pTop = m_stpTop.m_pTop;
			localTop.m_l64Count = m_stpTop.m_l64Count;

			// �� ����� Next�� Top���� ����
			NewNode->m_stpNextBlock = localTop.m_pTop;

			// Top�̵� �õ�
		} while (!InterlockedCompareExchange128((LONG64*)&m_stpTop, localTop.m_l64Count + 1, (LONG64)NewNode, (LONG64*)&localTop));

		// ���� ��� �� ����.
		InterlockedIncrement(&m_NodeCount);
	}


	// �ε��� ���
	//
	// ���� ��, T ����
	// ���н� Crash �߻�
	template <typename T>
	T CLF_Stack<T>::Pop()
	{
		// �� �̻� pop �Ұ� ������ Crash �߻�
		if (m_stpTop.m_pTop == nullptr)
			m_CDump->Crash();

		// ���� ��� �� ����
		InterlockedDecrement(&m_NodeCount);

		// ---- ������ ���� ----
		alignas(16)  st_TOP localTop;
		do
		{
			localTop.m_pTop = m_stpTop.m_pTop;
			localTop.m_l64Count = m_stpTop.m_l64Count;

			// nullüũ
			if (localTop.m_pTop == nullptr)
			{
				m_CDump->Crash();
			}

		} while (!InterlockedCompareExchange128((LONG64*)&m_stpTop, localTop.m_l64Count + 1, (LONG64)localTop.m_pTop->m_stpNextBlock, (LONG64*)&localTop));	

		// ������ ������ �޾Ƶα�
		T retval = localTop.m_pTop->m_Data;

		// �Ⱦ��°� Free�ϱ�
		m_MPool->Free(localTop.m_pTop);

		// ����
		return retval;
	}



}


#endif // !__LOCKFREE_STACK_H__
