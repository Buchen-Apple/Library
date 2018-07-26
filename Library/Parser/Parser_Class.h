#ifndef __PARSER_CLASS_H__
#define __PARSER_CLASS_H__

namespace Library_Jingyu
{
	class Parser
	{
	private:
		char* m_cBuffer;		// ������ �о�� ����
		char* m_cAreaBuffer;	// m_cBuffer ��, ���� �ʿ��� ������ ���� ����. {�� }�� ��������.
		char m_cWord[256];	// ��Ī�Ǵ� �ܾ ������ ����
		int m_ilen;			// GetValue �Լ��鿡�� ����ϴ� ���ڿ� ����.	
		char m_cSkipWord;		// SkipNoneCommand�Լ����� ���� ���ڰ� ��ŵ�ؾ��ϴ� �������� �����ϱ� ���� ����

	public:
		// ���� �ε�. 
		// ���Ͽ��� ���� : throw 1 / ���� ������ �о������� : throw 2
		void LoadFile(const char* FileName) throw (int);

		// ���� üũ
		bool AreaCheck(const char*);

		// GetNextWord���� �ܾ� ��ŵ ���θ� üũ�Ѵ�.
		// true�� ��ŵ / false�� ��ŵ�� �ƴϴ�.
		bool SkipNoneCommand(const char);

		// ���ۿ���, ���� �ܾ� ã��
		bool GetNextWord();

		// �ļ�_���ϴ� �� ã�ƿ��� (int��)
		bool GetValue_Int(const char* Name, int* value);

		// �ļ�_���ϴ� �� ã�ƿ��� (���ڿ�)
		bool GetValue_String(const char* Name, char* value);

		// �Ҹ���. m_cBuffer����Ʈ
		~Parser();
	};
}

#endif // !__PARSER_CLASS_H__

