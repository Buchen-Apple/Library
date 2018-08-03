#ifndef __PROTOCOL_BUFF_H__
#define __PROTOCOL_BUFF_H__

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
		//!!
	public:
		enum eNewPos
		{
			eDel_SendCommit,
			eDel_Release,
			eDel_Release2,
			eDel_ETC1,
			eDel_ETC2,
			eCre_Join,
			eCre_Send,

			eLas_RecvPost,
			eLas_SendPost,
			eLas_SendPacket,
			eLas_SendPacket_Deque,
			eEnd
		};
		long m_lLastAccessPos = -1;
		long m_lDelPos = -1;
		long m_lCreatePos = -1;
		long m_lArrIdx = -1;
		long m_lCount = -1;
		//!!
	private:
		// ����ȭ ����
		char* m_pProtocolBuff;

		// ���� ������
		int m_Size;

		// Front
		int m_Front;

		// Rear
		int m_Rear;

	private:
		// �ʱ�ȭ
		void Init(int size);

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
	};


}

#endif // !__PROTOCOL_BUFF_H__