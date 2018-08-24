#ifndef __PROTOCOL_BUFF_H__
#define __PROTOCOL_BUFF_H__

#include <Windows.h>
#include "ObjectPool\Object_Pool_LockFreeVersion.h"
#include "CrashDump\CrashDump.h"

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

	class CProtocolBuff
	{
	private:
		// ����ȭ ����
		char* m_pProtocolBuff;

		// ���� ������
		int m_Size;

		// Front
		int m_Front;

		// Rear
		int m_Rear;

		// ���۷��� ī��Ʈ. �Ҹ� üũ�ϴ� ī��Ʈ
		LONG m_RefCount;

		// CProtocolBuff�� �ٷ�� �޸�Ǯ (������)
		static CMemoryPoolTLS< CProtocolBuff>* m_MPool;

		// ���� ���� �� Crash �߻���ų ����.
		static CCrashDump* m_Dump;


	private:
		// �ʱ�ȭ
		void Init();

	public:
		// ������, �Ҹ���
		CProtocolBuff(int size);
		CProtocolBuff();
		~CProtocolBuff();

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

		// Rear�� ���ڷ� ���� ������ ������ �����Ű��
		int CompulsionMoveWritePos(int size);

		// Front �����̱�
		int MoveReadPos(int size);

		// Front�� ���ڷ� ���� ������ ������ �����Ű��
		int CompulsionMoveReadPos(int size);

		// ���� ������� �뷮 ���.
		int GetUseSize(void);

		// ���� ���ۿ� ���� �뷮 ���.
		int	GetFreeSize(void);

		// operator >>... ������ ����
		template<typename T>
		CProtocolBuff& operator >> (T& value)
		{
			GetData(reinterpret_cast<char*>(&value), sizeof(T));
			return *this;
		}

		// operator <<... ������ �ֱ�.
		template<typename T>
		CProtocolBuff& operator << (T value)
		{
			PutData(reinterpret_cast<char*>(&value), sizeof(T));
			return *this;
		}

	public:
		// ------
		// ���۷��� ī��Ʈ ���� �Լ�
		// ------

		// static �Լ� -----------
		// Alloc. ����� �� �ȿ��� new �� ���۷��� ī��Ʈ 1 ����
		static CProtocolBuff* Alloc();

		// Free. ����� �� �ȿ��� ���۷��� ī��Ʈ 1 ����.
		// ����, ���۷��� ī��Ʈ�� 0�̶�� ������. delete ��
		static void Free(CProtocolBuff* PBuff);

		// �Ϲ� �Լ� -----------
		// ���۷��� ī��Ʈ 1 Add�ϴ� �Լ�
		void Add();

		static CMemoryPoolTLS< CProtocolBuff>* Test();
	};


}

#endif // !__PROTOCOL_BUFF_H__