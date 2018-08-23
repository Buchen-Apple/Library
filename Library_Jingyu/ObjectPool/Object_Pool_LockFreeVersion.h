#ifndef __OBJECT_POOL_H__
#define __OBJECT_POOL_H__

#include <Windows.h>
#include <stdio.h>
#include <new.h>
#include <nmmintrin.h>


namespace Library_Jingyu
{

#define MEMORYPOOL_ENDCODE	890226

	template <typename DATA>
	class CMemoryPool
	{
	private:
		/* **************************************************************** */
		// �� �� �տ� ���� ��� ����ü.
		/* **************************************************************** */
		struct st_BLOCK_NODE
		{
			DATA		  stData;
			st_BLOCK_NODE *stpNextBlock;
			int			  stMyCode;
		};

		// -------------
		// Top���� ����� ����ü
		struct st_TOP
		{
			st_BLOCK_NODE* m_pTop;
			LONG64 m_l64Count = 0;
		};

	private:
		char* m_Memory;				// ���߿� �Ҹ��ڿ��� �� ���� free�ϱ� ���� ����
		int m_iBlockNum;			// �ִ� �� ����
		bool m_bPlacementNew;		// �÷��̽���Ʈ �� ����
		LONG m_iAllocCount;			// Ȯ���� �� ����. ���ο� ���� �Ҵ��� �� ���� 1�� ����. �ش� �޸�Ǯ�� �Ҵ��� �޸� �� ��
		LONG m_iUseCount;			// ������ ��� ���� �� ��. Alloc�� 1 ���� / free�� 1 ����
		alignas(16)	st_TOP m_stpTop;

	public:
		//////////////////////////////////////////////////////////////////////////
		// ������, �ı���.
		//
		// Parameters:	(int) �ִ� �� ����.
		//				(bool) ������ ȣ�� ����.
		// Return:
		//////////////////////////////////////////////////////////////////////////
		CMemoryPool(int iBlockNum, bool bPlacementNew = false);
		virtual	~CMemoryPool();


		//////////////////////////////////////////////////////////////////////////
		// �� �ϳ��� �Ҵ�޴´�.
		//
		// Parameters: ����.
		// Return: (DATA *) ����Ÿ �� ������.
		//////////////////////////////////////////////////////////////////////////
		DATA	*Alloc(void);

		//////////////////////////////////////////////////////////////////////////
		// ������̴� ���� �����Ѵ�.
		//
		// Parameters: (DATA *) �� ������.
		// Return: (BOOL) TRUE, FALSE.
		//////////////////////////////////////////////////////////////////////////
		bool	Free(DATA *pData);


		//////////////////////////////////////////////////////////////////////////
		// �Ҵ�� �޸�Ǯ�� �� ��. 
		//
		// Parameters: ����.
		// Return: (int) �޸� Ǯ ���� ��ü ����
		//////////////////////////////////////////////////////////////////////////
		int		GetAllocCount(void) { return m_iAllocCount; }

		//////////////////////////////////////////////////////////////////////////
		// ����ڰ� ������� �� ������ ��´�.
		//
		// Parameters: ����.
		// Return: (int) ������� �� ����.
		//////////////////////////////////////////////////////////////////////////
		int		GetUseCount(void) { return m_iUseCount; }

	};


	//////////////////////////////////////////////////////////////////////////
	// ������, �ı���.
	//
	// Parameters:	(int) �ִ� �� ����.
	//				(bool) ������ ȣ�� ����.
	// Return:
	//////////////////////////////////////////////////////////////////////////
	template <typename DATA>
	CMemoryPool<DATA>::CMemoryPool(int iBlockNum, bool bPlacementNew)
	{

		/////////////////////////////////////
		// bPlacementNew
		// ���� �ű� ����(����) 1. �޸� ���� �Ҵ�(malloc, ������ ȣ�� ����). �� �� ����� �⺻ ���� ����(next, ���ڵ�)
		// ���� �ű� ���� 2. true ��, '��ü ������' ȣ������ ����.
		// ���� �ű� ���� 2. false ��, '��ü ������' ȣ��
		// 
		// ���� Alloc, Free
		// true 1. �������� ���� ��, '��ü ������' ȣ�� �� ����
		// true 2. �������� ���� ��, '��ü �Ҹ���' ȣ�� �� �޸� Ǯ�� �߰�
		// true 3. ���α׷� ���� ��, �޸�Ǯ �Ҹ��ڿ��� ��� ��带 ���� ����� �޸𸮸� 'free()' ��Ų��.
		//
		// false 1. �������� ���� ��, �״�� ����
		// false 2. �������� ���� ��, �״�� �޸�Ǯ�� �߰�
		// false 3. ���α׷� ���� ��, �޸�Ǯ �Ҹ��ڿ��� ��� ��带 ���� ��� DATA�� '��ü �Ҹ���' ȣ��. �׸��� ��带 'free()' ��Ų��.
		//
		// bPlacementNew�� TRUE : Alloc() ��, DATA�� Placement new�� ������ ȣ�� �� �ѱ�, Free() ��, DATA�� Placement new�� �Ҹ��� ȣ�� �� �� �޸� Ǯ�� �ִ´�. CMemoryPool�Ҹ��ڿ���, �ش� ��� free
		// bPlacementNew�� FALSE : Alloc() ��, DATA�� ������ ȣ�� ����. Free()�� �Ҹ��� ȣ�� ����. CMemoryPool�Ҹ��ڿ���, �ش� ��� free
		/////////////////////////////////////

		// ��� ���� ����	
		m_iBlockNum = iBlockNum;
		m_bPlacementNew = bPlacementNew;
		m_iAllocCount = m_iUseCount = 0;

		// iBlockNum > 0�̶��,
		if (iBlockNum > 0)
		{
			// ������Ʈ ���, �� ������ ����.
			int iMemSize = sizeof(st_BLOCK_NODE) * m_iBlockNum;

			// ������ ��ŭ �޸� �����Ҵ�.
			char* pMemory = (char*)malloc(iMemSize);

			// pMemory ����صд�. ���� �Ҹ��ڿ��� free�ϱ� ���ؼ�
			m_Memory = pMemory;

			// ù ��ġ�� Top���� ����Ų��. 			
			m_stpTop.m_pTop = (st_BLOCK_NODE*)pMemory;

			// ���� 1���� ��� ����
			st_BLOCK_NODE* pNode = (st_BLOCK_NODE*)pMemory;
			if (bPlacementNew == false)
				new (&pNode->stData) DATA();

			pNode->stpNextBlock = nullptr;
			pNode->stMyCode = MEMORYPOOL_ENDCODE;
			pMemory += sizeof(st_BLOCK_NODE);

			m_iAllocCount++;

			//  iBlockNum - 1 �� ��ŭ ��� �ʱ�ȭ
			for (int i = 1; i < iBlockNum; ++i)
			{
				st_BLOCK_NODE* pNode = (st_BLOCK_NODE*)pMemory;

				if (bPlacementNew == false)
					new (&pNode->stData) DATA();

				pNode->stpNextBlock = m_stpTop.m_pTop;
				m_stpTop.m_pTop = pNode;				

				pNode->stMyCode = MEMORYPOOL_ENDCODE;

				pMemory += sizeof(st_BLOCK_NODE);

				m_iAllocCount++;
			}
		}

		// iBlockNum�� 0���� �۰ų� ���ٸ�, ���� 1���� �����.
		else
		{
			st_BLOCK_NODE* pNode = (st_BLOCK_NODE*)malloc(sizeof(st_BLOCK_NODE));
			if (bPlacementNew == false)
				new (&pNode->stData) DATA();
			pNode->stpNextBlock = nullptr;
			pNode->stMyCode = MEMORYPOOL_ENDCODE;

			m_stpTop.m_pTop = pNode;

			m_iAllocCount++;
		}

	}

	template <typename DATA>
	CMemoryPool<DATA>::~CMemoryPool()
	{
		// �� Ǯ�� �ִ� ��� ��带 ��������
		// �÷��̽���Ʈ ���� ��� �ߵ� ���ߵ� delete�� �ȵ�. �Ҹ��ڰ� ������ ȣ��Ǵϱ�!! free�� �޸� �����ؾ���. 
		// �÷��̽���Ʈ ���� �׳� �ʱ�ȭ�� ��, ���� ������ �Ҵ��ϴ� ������ �ƴ�.
		//
		// �÷��̽���Ʈ ���� ����� ��� : �̹�, Free() �Լ����� �Ҹ��ڸ� ȣ�������� �Ҹ��� ȣ�� ����. 
		// �÷��̽���Ʈ ���� ������ ��� : �̹��� �Ҹ��ڸ� ȣ���Ѵ�. 
		//
		// m_iBlockNum�� 0�̶��, ������ malloc �� ���̴� ������ free �Ѵ�. (Alloc�� ���� ��������. Free()������ �÷��̽���Ʈ ���� ���� �Ҹ��ڸ� ȣ���߾���. �޸� ���� ������)
		// m_iBlockNum�� 0���� ũ�ٸ�, �������� �� ���� �޸� ��ü free �Ѵ�. 


		// �� �޸�Ǯ�� �ִ� ��带 ��� 'free()' �Ѵ�.
		while (1)
		{
			if (m_stpTop.m_pTop == nullptr)
				break;

			st_BLOCK_NODE* deleteNode = m_stpTop.m_pTop;
			m_stpTop.m_pTop = m_stpTop.m_pTop->stpNextBlock;

			// �÷��̽���Ʈ ���� false���, Free()�� '��ü �Ҹ���'�� ȣ�� ���� ���̴� ���⼭ ȣ�������� �Ѵ�.
			if (m_bPlacementNew == false)
				deleteNode->stData.~DATA();

			// m_iBlockNum�� 0�̶��, ������ Malloc������(Alloc�� ���� ��������. Free()������ �Ҹ��ڸ� ȣ���ϰ�, �޸� ������ ����.) free�Ѵ�.
			if (m_iBlockNum == 0)
				free(deleteNode);
		}

		//  m_iBlockNum�� 0���� ũ�ٸ�, �� ���� malloc �� ���̴� �ѹ��� free�Ѵ�.
		if (m_iBlockNum > 0)
			free(m_Memory);
	}

	//////////////////////////////////////////////////////////////////////////
	// �� �ϳ��� �Ҵ�޴´�. (Pop)
	//
	// Parameters: ����.
	// Return: (DATA *) ����Ÿ �� ������.
	//////////////////////////////////////////////////////////////////////////
	template <typename DATA>
	DATA*	CMemoryPool<DATA>::Alloc(void)
	{
		bool bContinueFlag;		

		while (1)
		{
			bContinueFlag = false;

			//////////////////////////////////
			// m_pTop�� NULL�϶� ó��
			//////////////////////////////////
			if (m_stpTop.m_pTop == nullptr)
			{
				if (m_iBlockNum > 0)
					return nullptr;

				// m_iBlockNum <= 0 ���, ������ ���� ������ ���� ������. 
				// ���� �����Ѵ�.
				else
				{
					st_BLOCK_NODE* pNode = (st_BLOCK_NODE*)malloc(sizeof(st_BLOCK_NODE));
					pNode->stpNextBlock = NULL;
					pNode->stMyCode = MEMORYPOOL_ENDCODE;

					// �÷��̽���Ʈ �� ȣ��
					new (&pNode->stData) DATA();

					// allocī��Ʈ, ���� ����� ī��Ʈ ����
					InterlockedIncrement(&m_iAllocCount);
					InterlockedIncrement(&m_iUseCount);

					return &pNode->stData;
				}
			}

			//////////////////////////////////
			// m_pTop�� NULL�� �ƴ� �� ó��
			//////////////////////////////////
			alignas(16)  st_TOP localTop;
			// ---- ������ ���� ----
			do
			{
				localTop.m_l64Count = m_stpTop.m_l64Count;
				localTop.m_pTop = m_stpTop.m_pTop;				

				// nullüũ
				if (localTop.m_pTop == nullptr)
				{
					bContinueFlag = true;
					break;
				}						

			} while (!InterlockedCompareExchange128((LONG64*)&m_stpTop, localTop.m_l64Count + 1, (LONG64)localTop.m_pTop->stpNextBlock, (LONG64*)&localTop));


			if (bContinueFlag == true)
				continue;

			// �� �Ʒ������� ��� ���������� ���� localTop�� ���. �̰� �ϴ� �߿� m_pTop�� �ٲ� �� �ֱ� ������!
			// �÷��̽���Ʈ ���� ����Ѵٸ� ����ڿ��� �ֱ����� '��ü ������' ȣ��
			if (m_bPlacementNew == true)
				new (&localTop.m_pTop->stData) DATA();

			// ���� ����� ī��Ʈ ����. ���� �Ҵ��Ѱ� �ƴϱ� ������ Allocī��Ʈ�� ���� ����.
			InterlockedIncrement(&m_iUseCount);

			return &localTop.m_pTop->stData;

		}
		
	}

	//////////////////////////////////////////////////////////////////////////
	// ������̴� ���� �����Ѵ�. (Push)
	//
	// Parameters: (DATA *) �� ������.
	// Return: (BOOL) TRUE, FALSE.
	//////////////////////////////////////////////////////////////////////////
	template <typename DATA>
	bool	CMemoryPool<DATA>::Free(DATA *pData)
	{
		// �̻��� �����Ͱ����� �׳� ����
		if (pData == NULL)
			return false;

		// ���� �Ҵ��� ���� �´��� Ȯ��
		st_BLOCK_NODE* pNode = (st_BLOCK_NODE*)pData;

		if (pNode->stMyCode != MEMORYPOOL_ENDCODE)
			return false;	

		// ���� ����� ī��Ʈ ����
		// free(Push) �ÿ���, �ϴ� ī��Ʈ�� ���� ���ҽ�Ų��. �׷��� �ȴ�! ������ �������� 100%���� ����.
		InterlockedDecrement(&m_iUseCount);

		//�÷��̽���Ʈ ���� ����Ѵٸ� �޸� Ǯ�� �߰��ϱ� ���� '��ü �Ҹ���' ȣ��
		if (m_bPlacementNew == true)
			pData->~DATA();
		
		// ---- ������ ���� ----
		st_BLOCK_NODE* localTop;

		do
		{
			// ���� Top ���� 
			localTop = m_stpTop.m_pTop;

			// ���� ���� ����� Next�� Top���� �
			pNode->stpNextBlock = localTop;

			// Top�̵� �õ�			
		} while (InterlockedCompareExchange64((LONG64*)&m_stpTop.m_pTop, (LONG64)pNode, (LONG64)localTop) != (LONG64)localTop);

		return true;
	}
}

#endif // !__OBJECT_POOL_H__