#ifndef __PROTOCOL_BUFF_H__
#define __PROTOCOL_BUFF_H__

/*
LanServer ���� ����ȭ ����.
NetServer �� ����ȭ ���۴� ���� ����
*/

#include <Windows.h>
#include "ObjectPool\Object_Pool_LockFreeVersion.h"
#include "CrashDump\CrashDump.h"
#include "CException.h"

#include <time.h>

namespace Library_Jingyu
{   	

	class CProtocolBuff_Lan
	{
		friend class CLanServer;	
		friend class CLanClient;

	private:
		// ----------- ������� ��ġ�� ���� �� 'ĳ�� ģȭ �ڵ�(Cache Friendly Code)' �ִ��� ���� ���
		// �� class���� �ٽ� �Լ��� PutData, GetData, MoveWritePos, MoveReadPos. 
		// �ش� �Լ��� �ڵ忡 ���缭 ������� ��ġ
		
		// ���� ������
		int m_Size;

		// Front
		int m_Front;

		// ���۷��� ī��Ʈ. �Ҹ� üũ�ϴ� ī��Ʈ
		LONG m_RefCount;

		// Rear
		int m_Rear;
	
		// ����ȭ ����
		char* m_pProtocolBuff;		

		// CProtocolBuff�� �ٷ�� �޸�Ǯ (������)
		static CMemoryPoolTLS< CProtocolBuff_Lan>* m_MPool;

		// ���� ���� �� Crash �߻���ų ����.
		static CCrashDump* m_Dump;

	private:
		// ����� ä��� �Լ�.
		void SetProtocolBuff_HeaderSet();

	public:
		// ������, �Ҹ���
		CProtocolBuff_Lan(int size);
		CProtocolBuff_Lan();
		~CProtocolBuff_Lan();

		// ���� ũ�� �缳�� (������ ������ ���� ���°� ����)
		int ReAlloc(int size);

		// ���� Ŭ����
		void Clear();

		// ������ �ֱ�
		int PutData(const char* pSrc, int size);

		// ������ ����
		int GetData(char* pSrc, int size);

		// ������ ������ ����.
		char* GetBufferPtr(void);

		// Rear �����̱�
		int MoveWritePos(int size);

		// Front �����̱�
		int MoveReadPos(int size);

		// ���� ������� �뷮 ���.
		int GetUseSize(void);

		// ���� ���ۿ� ���� �뷮 ���.
		int	GetFreeSize(void);

		// operator >>... ������ ����
		template<typename T>
		CProtocolBuff_Lan& operator >> (T& value)
		{
			GetData(reinterpret_cast<char*>(&value), sizeof(T));
			return *this;
		}

		// operator <<... ������ �ֱ�.
		template<typename T>
		CProtocolBuff_Lan& operator << (T value)
		{
			PutData(reinterpret_cast<char*>(&value), sizeof(T));
			return *this;
		}

	public:
		// ------
		// ���۷��� ī��Ʈ ���� �Լ�
		// ------

		// static �Լ� -----------
		// Alloc. ����� �� �ȿ��� Alloc ��, ���۷��� ī��Ʈ 1 ����
		static CProtocolBuff_Lan* Alloc();

		// Free. ����� �� �ȿ��� ���۷��� ī��Ʈ 1 ����.
		// ����, ���۷��� ī��Ʈ�� 0�̶�� ������. TLSPool�� Free ��
		static void Free(CProtocolBuff_Lan* PBuff);

		// ���� Alloc�� ��� ī��Ʈ ���
		static LONG GetNodeCount();

		// �Ϲ� �Լ� -----------
		// ���۷��� ī��Ʈ 1 Add�ϴ� �Լ�
		void Add();

		// ���۷��� ī��Ʈ�� ���ڷ� �޾Ƽ� Add�ϴ� �Լ�
		// ���� Add�Լ� �����ε�
		void Add(int Count);

		// TLS ����ȭ������ �� Alloc�� ûũ ��
		static LONG GetChunkCount()
		{
			return m_MPool->GetAllocChunkCount();
		}

		// TLS ����ȭ������ ûũ ��, �ۿ��� ������� ûũ ��
		static LONG GetOutChunkCount()
		{
			return m_MPool->GetOutChunkCount();
		}
	};
}

#endif // !__PROTOCOL_BUFF_H__