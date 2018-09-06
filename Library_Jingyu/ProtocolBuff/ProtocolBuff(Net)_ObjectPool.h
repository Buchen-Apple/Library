#ifndef __PROTOCOL_BUFF_NET_H__
#define __PROTOCOL_BUFF_NET_H__

/*
NetServer �� ����ȭ ����
*/

#include <Windows.h>
#include "ObjectPool\Object_Pool_LockFreeVersion.h"
#include "CrashDump\CrashDump.h"

#include <time.h>

namespace Library_Jingyu
{
	// Recv()��Ŷ ó�� ��, ���� �߻� �� ������ ����Ŭ�����̴�.
	class CException
	{
	private:
		wchar_t ExceptionText[100];

	public:
		// ������
		CException(const wchar_t* str);

		// ���� �ؽ�Ʈ ������ ��ȯ
		char* GetExceptionText();
	};

	//  NetServer�� ����ȭ ����
	class CProtocolBuff_Net
	{
		friend class CNetServer;

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

		// ����� ä�������� üũ�ϴ� Flag
		bool m_bHeadCheck;

		// CProtocolBuff�� �ٷ�� �޸�Ǯ (������)
		static CMemoryPoolTLS< CProtocolBuff_Net>* m_MPool;

		// ���� ���� �� Crash �߻���ų ����.
		static CCrashDump* m_Dump;

	public:
		// ���ڵ�
		// ������ ����, ����� �ִ´�. �� �� ��ȣȭ �� �ִ´�.
		//
		// Parameter : ��� �ڵ�, XORCode1, XORCode2
		// return : ����
		void Encode(BYTE bCode, BYTE bXORCode_1, BYTE bXORCode_2);

		// ���ڵ�
		// ��Ʈ��ũ�� ���� ��Ŷ ��, ����� �ؼ��Ѵ�.
		//
		// Parameter : ���̷ε� ����, ����xor�ڵ�, üũ��, XORCode1, XORCode2
		// return : CheckSum�� �ٸ� �� false
		bool Decode(WORD PayloadLen, BYTE RandXORCode, BYTE CheckSum, BYTE bXORCode_1, BYTE bXORCode_2);

	public:
		// ������ , �Ҹ���
		CProtocolBuff_Net(int size);
		CProtocolBuff_Net();
		~CProtocolBuff_Net();

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
		CProtocolBuff_Net& operator >> (T& value)
		{
			GetData(reinterpret_cast<char*>(&value), sizeof(T));
			return *this;
		}

		// operator <<... ������ �ֱ�.
		template<typename T>
		CProtocolBuff_Net& operator << (T value)
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
		static CProtocolBuff_Net* Alloc();

		// Free. ����� �� �ȿ��� ���۷��� ī��Ʈ 1 ����.
		// ����, ���۷��� ī��Ʈ�� 0�̶�� ������. TLSPool�� Free ��
		static void Free(CProtocolBuff_Net* PBuff);

		// �Ϲ� �Լ� -----------
		// ���۷��� ī��Ʈ 1 Add�ϴ� �Լ�
		void Add();

		static LONG GetChunkCount()
		{
			return m_MPool->GetAllocChunkCount();
		}

		static LONG GetOutChunkCount()
		{
			return m_MPool->GetOutChunkCount();
		}


	};

}

#endif // !__PROTOCOL_BUFF_NET_H__