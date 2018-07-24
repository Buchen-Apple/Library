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
		int PutData(const char* pSrc, int size) throw (CException);

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