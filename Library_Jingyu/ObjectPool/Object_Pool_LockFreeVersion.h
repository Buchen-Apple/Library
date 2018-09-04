#ifndef __OBJECT_POOL_H__
#define __OBJECT_POOL_H__

#include <Windows.h>
#include <stdio.h>
#include <new.h>
#include "CrashDump\CrashDump.h"


extern LONG g_lAllocNodeCount;
extern LONG g_lFreeNodeCount;

namespace Library_Jingyu
{

#define MEMORYPOOL_ENDCODE	890226

	/////////////////////////////////////////////////////
	// �޸� Ǯ(������Ʈ Ǯ)
	////////////////////////////////////////////////////
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

		/* **************************************************************** */
		// Top���� ����� ����ü
		/* **************************************************************** */
		struct st_TOP
		{
			st_BLOCK_NODE* m_pTop;
			LONG64 m_l64Count = 0;
		};

	private:
		// ----------- ������� ��ġ�� ���� ��, 'ĳ�� ģȭ �ڵ�(Cache Friendly Code)' �ִ��� ���� ���
		// �� class���� �ٽ� �Լ��� Alloc, Free. �ش� �Լ��� �ڵ忡 ���缭 ������� ��ġ

		char* m_Memory;					// ���߿� �Ҹ��ڿ��� �� ���� free�ϱ� ���� ����		
		alignas(16)	st_TOP m_stpTop;	// Top. �޸�Ǯ�� ���� �����̴�.
		int m_iBlockNum;				// �ִ� �� ����		
		bool m_bPlacementNew;			// �÷��̽���Ʈ �� ����
		LONG m_iAllocCount;				// Ȯ���� �� ����. ���ο� ���� �Ҵ��� �� ���� 1�� ����. �ش� �޸�Ǯ�� �Ҵ��� �޸� �� ��
		LONG m_iUseCount;				// ������ ��� ���� �� ��. Alloc�� 1 ���� / free�� 1 ����

	public:
		//////////////////////////////////////////////////////////////////////////
		// ������
		//
		// Parameters:	(int) �ִ� �� ����.
		//				(bool) ������ ȣ�� ����.
		//////////////////////////////////////////////////////////////////////////
		CMemoryPool(int iBlockNum, bool bPlacementNew = false);

		//////////////////////////////////////////////////////////////////////////
		// �ı���
		//
		// ���ο� �ִ� ��� ��带 ��������
		//////////////////////////////////////////////////////////////////////////
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
	// ������
	//
	// Parameters:	(int) �ִ� �� ����.
	//				(bool) ������ ȣ�� ����.
	//////////////////////////////////////////////////////////////////////////
	template <typename DATA>
	CMemoryPool<DATA>::CMemoryPool(int iBlockNum, bool bPlacementNew)
	{		
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

	//////////////////////////////////////////////////////////////////////////
	// �ı���
	//
	// ���ο� �ִ� ��� ��带 ��������
	//////////////////////////////////////////////////////////////////////////
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
			// ���� ž�� nullptr�� �ɶ����� �ݺ�.
			if (m_stpTop.m_pTop == nullptr)
				break;

			// ������ ��带 �޾Ƶΰ�, Top�� ��ĭ �̵�.
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
				// ���������� ����ũ���� �����ϴ� ���� Count��.
				// ������, ���� ���� ���� ������ Count�� ���� �޾ƿ´�. 
				// top�� ���� �޾ƿ� ���, ���ؽ�Ʈ ����Ī���� ���� �ٸ� �����忡�� Count�� ���� ��, ������ ���� �޾ƿ� ���� �ִ�.
				localTop.m_l64Count = m_stpTop.m_l64Count;
				localTop.m_pTop = m_stpTop.m_pTop;	

				// nullüũ
				if (localTop.m_pTop == nullptr)
				{
					// null�̶��, ó������ �ٽ� ������ ������.
					// flag�� true�� �ٲٸ�, do while�� �ۿ��� üũ �� continue ����
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
	// Return: (bool) true, false.
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

		//�÷��̽���Ʈ ���� ����Ѵٸ� �޸� Ǯ�� �߰��ϱ� ���� '��ü �Ҹ���' ȣ��
		if (m_bPlacementNew == true)
			pData->~DATA();

		// ���� ����� ī��Ʈ ����
		// free(Push) �ÿ���, �ϴ� ī��Ʈ�� ���� ���ҽ�Ų��. �׷��� �ȴ�! ������ �������� 100%���� ����.
		InterlockedDecrement(&m_iUseCount);		
		
		// ---- ������ ���� ----
		st_BLOCK_NODE* localTop;

		do
		{
			// ���� Top ���� 
			// Push�� ���� ī��Ʈ�� �ʿ� ����.
			localTop = m_stpTop.m_pTop;

			// ���� ���� ����� Next�� Top���� �
			pNode->stpNextBlock = localTop;

			// Top�̵� �õ�			
		} while (InterlockedCompareExchange64((LONG64*)&m_stpTop.m_pTop, (LONG64)pNode, (LONG64)localTop) != (LONG64)localTop);

		return true;
	}



	// -------------------------------------------------------------------------------------------
	// -------------------------------------------------------------------------------------------
	// -------------------------------------------------------------------------------------------
	// -------------------------------------------------------------------------------------------
	// -------------------------------------------------------------------------------------------
	// -------------------------------------------------------------------------------------------
	// -------------------------------------------------------------------------------------------

	/////////////////////////////////////////////////////
	// �޸� Ǯ TLS ����
	//
	// ���ο��� ûũ�� �ٷ�
	////////////////////////////////////////////////////
	template <typename DATA>
	class CMemoryPoolTLS
	{	
		struct stChunk;

		// ���� ûũ ������ �����Ǵ� ���.
		struct Node
		{
			DATA  m_Data;
			stChunk* m_pMyChunk;
			int	stMyCode;
		};

	private:	
		// ----------------------------
		// ûũ ����ü
		// ----------------------------
		struct stChunk
		{	

#define NODE_COUNT 200	// 1���� ûũ�� �ٷ�� ����� ��			

			// ûũ �������

			// ----------- ������� ��ġ�� ���� ��, 'ĳ�� ģȭ �ڵ�(Cache Friendly Code)' �ִ��� ���� ���
			// �� struct������ ���� Ŭ������  CMemoryPoolTLS�� Alloc, Free�� �ٽ� �Լ�. 
			// �ش� �Լ��� �ڵ忡 ���缭 ������� ��ġ

			LONG m_iTop;					// top �� Allocī��Ʈ. 0���� ����
			LONG m_iFreeRef;				// Free ī��Ʈ. 0���� ����
			Node m_arrayNode[NODE_COUNT];	// ���� �ٷ�� ûũ ���� ���. ���� �迭			
			CCrashDump* m_ChunkDump;		// ���� �߻� �� ������ ���� ��������.

			// ûũ ������
			stChunk();

			// ûũ �Ҹ���
			~stChunk();			

			// Allocī��Ʈ ���
			int GetAllocCount()
			{	return m_iTop;	}

			// Free ī��Ʈ ���
			LONG GetFreeCount()
			{	return m_iFreeRef;	}
		};	


	private:
		// ------------------
		// ��� ����
		// ------------------
		// ----------- ������� ��ġ�� ���� ��, 'ĳ�� ģȭ �ڵ�(Cache Friendly Code)' �ִ��� ���� ���
		// �� class���� �ٽ� �Լ��� Alloc, Free. �ش� �Լ��� �ڵ忡 ���缭 ������� ��ġ

		DWORD m_dwIndex;					// �� �����尡 ����ϴ� TLS�� Index
		CMemoryPool<stChunk>* m_ChunkPool;	// TLSPool ���ο��� ûũ�� �ٷ�� �޸�Ǯ (������ ����)
		static bool m_bPlacementNew;		// ûũ ���ο��� �ٷ�� ��� ���� �������� �÷��̽���Ʈ �� ȣ�� ����(ûũ �ƴ�. ûũ ���ξ��� ����� �����Ϳ� ���� �÷��̽���Ʈ �� ȣ�� ����)		
		CCrashDump* m_TLSDump;				// ���� �߻��� ������ ���� ���� ����



	public:
		//////////////////////////////////////////////////////////////////////////
		// ������
		//
		// Parameters:	(int) �ִ� �� ����. (ûũ�� ��)
		//				(bool) ������ ȣ�� ����. (ûũ ���ο��� �����Ǵ� DATA���� ������ ȣ�� ����. ûũ ������ ȣ�⿩�� �ƴ�)
		//////////////////////////////////////////////////////////////////////////
		CMemoryPoolTLS(int iBlockNum, bool bPlacementNew);

		//////////////////////////////////////////////////////////////////////////
		// �ı���
		// 
		// ûũ �޸�Ǯ ����, TLSIndex ��ȯ.
		//////////////////////////////////////////////////////////////////////////
		virtual ~CMemoryPoolTLS();

		//////////////////////////////////////////////////////////////////////////
		// �� �ϳ��� �Ҵ�޴´�. (Pop)
		//
		// Parameters: ����.
		// Return: (DATA *) ����Ÿ �� ������.
		//////////////////////////////////////////////////////////////////////////
		DATA* Alloc();

		//////////////////////////////////////////////////////////////////////////
		// ������̴� ���� �����Ѵ�. (Push)
		//
		// Parameters: (DATA *) �� ������.
		// Return: ����
		//////////////////////////////////////////////////////////////////////////
		void Free(DATA* pData);

		// ���ο� �ִ� ûũ �� ���
		int GetAllocChunkCount()
		{	
			return m_ChunkPool->GetAllocCount(); 
		}

		// �ܺο��� ��� ���� ûũ �� ���
		int GetOutChunkCount()
		{	
			return m_ChunkPool->GetUseCount();	
		}



	};	

	template <typename DATA>
	bool CMemoryPoolTLS<DATA>::m_bPlacementNew;

	// ----------------------
	// 
	// CMemoryPoolTLS ������ �̳� ����ü, stChunk.
	// 
	// ----------------------

	// ûũ ������
	template <typename DATA>
	CMemoryPoolTLS<DATA>::stChunk::stChunk()
	{
		m_ChunkDump = CCrashDump::GetInstance();

		for (int i = 0; i < NODE_COUNT; ++i)
		{
			m_arrayNode[i].m_pMyChunk = this;
			m_arrayNode[i].stMyCode = MEMORYPOOL_ENDCODE;
		}

		m_iTop = 0;
		m_iFreeRef = 0;
	}

	// ûũ �Ҹ���
	template <typename DATA>
	CMemoryPoolTLS<DATA>::stChunk::~stChunk()
	{
		// �÷��̽���Ʈ �� ���ΰ� false���, ûũFree ��, �Ҹ��ڰ� ȣ�� �ȵȰ��̴� ���⼭ �Ҹ��� ȣ�������� ��
		if (m_bPlacementNew == false)
		{
			for (int i = 0; i < NODE_COUNT; ++i)
			{
				m_arrayNode[i].m_Data.~DATA();
			}
		}
	}

	// ----------------------
	// 
	// CMemoryPoolTLS �κ�
	// 
	// ----------------------

	//////////////////////////////////////////////////////////////////////////
	// ������
	//
	// Parameters:	(int) �ִ� �� ����. (ûũ�� ��)
	//				(bool) ������ ȣ�� ����. (ûũ ���ο��� �����Ǵ� DATA���� ������ ȣ�� ����. ûũ ������ ȣ�⿩�� �ƴ�)
	//////////////////////////////////////////////////////////////////////////
	template <typename DATA>
	CMemoryPoolTLS<DATA>::CMemoryPoolTLS(int iBlockNum, bool bPlacementNew)
	{
		// ûũ �޸�Ǯ ���� (ûũ�� ������ �÷��̽���Ʈ �� ��� ����)
		m_ChunkPool = new CMemoryPool<stChunk>(iBlockNum, false);

		// ���� ����
		m_TLSDump = CCrashDump::GetInstance();

		// �������� �÷��̽���Ʈ �� ���� ����(ûũ �ƴ�. ûũ ���ο��� �����Ǵ� �������� �÷��̽���Ʈ �� ȣ�� ����)
		m_bPlacementNew = bPlacementNew;

		// TLSIndex �˾ƿ���
		// ������ ��� ������� �� �ε����� ����� ûũ ����.
		// ���ϰ��� �ε��� �ƿ��̸� Crash
		m_dwIndex = TlsAlloc();
		if (m_dwIndex == TLS_OUT_OF_INDEXES)
			m_TLSDump->Crash();
	}
	
	//////////////////////////////////////////////////////////////////////////
	// �ı���
	// 
	// ûũ �޸�Ǯ ����, TLSIndex ��ȯ.
	//////////////////////////////////////////////////////////////////////////
	template <typename DATA>
	CMemoryPoolTLS<DATA>::~CMemoryPoolTLS()
	{
		// ûũ ���� �޸�Ǯ ����
		delete m_ChunkPool;

		// TLS �ε��� ��ȯ
		if (TlsFree(m_dwIndex) == FALSE)
		{
			DWORD Error = GetLastError();
			m_TLSDump->Crash();
		}
	}

	//////////////////////////////////////////////////////////////////////////
	// �� �ϳ��� �Ҵ�޴´�. (Pop)
	//
	// Parameters: ����.
	// Return: (DATA *) ����Ÿ �� ������.
	//////////////////////////////////////////////////////////////////////////
	template <typename DATA>
	DATA* CMemoryPoolTLS<DATA>::Alloc()
	{

		int abc = 0;
		int abcdef = 0;

		// ���� �� �Լ��� ȣ���� �������� TLS �ε����� ûũ�� �ִ��� Ȯ��
		stChunk* pChunk = (stChunk*)TlsGetValue(m_dwIndex);

		// TLS�� ûũ�� ������ ûũ ���� �Ҵ���.
		if (pChunk == nullptr)
		{
			pChunk = m_ChunkPool->Alloc();			
			if (TlsSetValue(m_dwIndex, pChunk) == FALSE)
			{
				DWORD Error = GetLastError();
				m_TLSDump->Crash();
			}
			abc = 10;

			if (pChunk->m_iTop != 0 || pChunk->m_iFreeRef != 0)
				abcdef = 10;
		}	

		InterlockedIncrement(&g_lAllocNodeCount);
			
		// ����, Top�� NODE_COUNT���� ũ�ų� ���ٸ� ���� �߸��Ȱ�.
		// �� ���� �̹� ĳġ�Ǿ���� ��
		if (pChunk->m_iTop >= NODE_COUNT)
			pChunk->m_ChunkDump->Crash();	

		// ���� Top�� �����͸� �����´�. �׸��� Top�� 1 ����
		DATA* retData = &pChunk->m_arrayNode[pChunk->m_iTop].m_Data;
		pChunk->m_iTop++;

		// ����, ûũ�� �����͸� ��� Alloc������, TLS ûũ�� NULL�� �����.
		if (pChunk->m_iTop == NODE_COUNT)
		{
			if (TlsSetValue(m_dwIndex, nullptr) == FALSE)
			{
				DWORD Error = GetLastError();
				pChunk->m_ChunkDump->Crash();
			}
		}

		// �÷��̽���Ʈ �� ���ο� ���� ������ ȣ��
		if (m_bPlacementNew == true)
			new (retData) DATA();

		return retData;		
	}

	//////////////////////////////////////////////////////////////////////////
	// ������̴� ���� �����Ѵ�. (Push)
	//
	// Parameters: (DATA *) �� ������.
	// Return: ����
	//////////////////////////////////////////////////////////////////////////
	template <typename DATA>
	void CMemoryPoolTLS<DATA>::Free(DATA* pData)
	{	
		// ûũ �˾ƿ�
		stChunk* pChunk = ((Node*)pData)->m_pMyChunk;

		// ������ �˻� -------
		// �̻��� �����Ͱ� ���� �׳� ����
		if (pData == NULL)
			m_TLSDump->Crash();

		// ���� �Ҵ��� ���� �´��� Ȯ��
		if (((Node*)pData)->stMyCode != MEMORYPOOL_ENDCODE)
			m_TLSDump->Crash();

		InterlockedDecrement(&g_lAllocNodeCount);
		//InterlockedIncrement(&g_lFreeNodeCount);

		// FreeRefCount 1 ����
		// ���� NODE_COUNT�� �Ǹ� ûũ ���� ���� �ʱ�ȭ �� ûũ ���� �޸�Ǯ�� Free
		if (InterlockedIncrement(&pChunk->m_iFreeRef) == NODE_COUNT)
		{
			// Free�ϱ�����, �÷��̽���Ʈ ���� ����Ѵٸ� ��� DATA�� �Ҹ��� ȣ��
			if (m_bPlacementNew == true)
			{
				for (int i = 0; i < NODE_COUNT; ++i)
				{
					pChunk->m_arrayNode[i].m_Data.~DATA();
				}
			}

			// Top�� RefCount �ʱ�ȭ
			pChunk->m_iTop = 0;
			pChunk->m_iFreeRef = 0;

			// ûũ ���� �޸�Ǯ�� ûũ Free
			if (m_ChunkPool->Free(pChunk) == false)
				m_TLSDump->Crash();
		}
	}




}

#endif // !__OBJECT_POOL_H__