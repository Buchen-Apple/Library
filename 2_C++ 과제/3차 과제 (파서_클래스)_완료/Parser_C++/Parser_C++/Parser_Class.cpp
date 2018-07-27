#include "Parser_Class.h"
#include <iostream>	

namespace Library_Jingyu{

	using namespace std;

	// ���� �ε�
	void Parser::LoadFile(const char* FileName)
	{
		FILE* fp;			// ���� ��Ʈ��
		size_t iFileCheck;	// ���� ���� ���� üũ, ������ ������ ����. �� 2���� �˵�
		char* cCheck;		// ���� �о�� �������� üũ
		char cTempBuffer[50];	// ���Ͽ��� �о�� ���ڿ� �ӽ� �����

		iFileCheck = fopen_s(&fp, FileName, "rt");
		if (iFileCheck != 0)
			throw 1;

		fseek(fp, 0, SEEK_END);
		iFileCheck = ftell(fp);
		fseek(fp, 0, SEEK_SET);

		m_cBuffer = new char[iFileCheck];
		memset(m_cBuffer, 0, 1);

		while (1)
		{
			cCheck = fgets(cTempBuffer, 50, fp);
			strcat_s(m_cBuffer, iFileCheck, cTempBuffer); // �о�� ���ڿ��� m_cBuffe�� ����
			if (cCheck == NULL)	//���Ͽ� ���� �����ϰų�, �о���⿡ ���������� while�� ����
				break;
		}
		if (feof(fp) == 0)	// while�� ���� ������ ���� �о���� ���п��ٸ�
			throw 2;

		fclose(fp);	// �� �� ���� ��Ʈ�� ����
	}

	bool Parser::AreaCheck(const char* AreaName)
	{
		int i = 0;
		char cTempWord[256];
		int iStartPos;
		int iEndPos;
		m_ilen = 0;
		bool bWhileBreakCheck = false;

		while (!bWhileBreakCheck)
		{
			cTempWord[0] = '\0';
			i = 0;
			// : ���ڸ� ã�´�.
			while (SkipNoneCommand(m_cBuffer[m_ilen]))
			{
				// len�� ���� cBuffer�� ������� Ŀ����,  �� �̻� : ���ڸ� ��ã�� ��.
				if (m_ilen > strlen(m_cBuffer))
					return false;

				m_ilen++;
			}

			// ã�� ���ڰ� : ��� �Ʒ� ���� ����
			if (m_cSkipWord == ':')
			{
				m_ilen++;
				// : �� ��������, �ϼ��Ǵ� ���ڸ� ã�´�. ��, ���� �̸��� ã�´�.
				while (1)
				{
					// len�� ���� cBuffer�� ������� Ŀ����, �迭�� ����Ŵ� false ����.
					if (m_ilen > strlen(m_cBuffer))
						return false;

					// ������� �޸�, ��ħǥ, ����ǥ, �����̽�, �齺���̽�, ��, �����ǵ�, ĳ���� ����
					// �� �� �ϳ��� �ش�Ǹ�, ���ڸ� �ϳ� �ϼ��� ��. ��, { �ٷ� ����(����) ��ġ�� ã�� ��.
					else if (m_cBuffer[m_ilen] == ',' || m_cBuffer[m_ilen] == '.' || m_cBuffer[m_ilen] == '"' || m_cBuffer[m_ilen] == 0x20 || m_cBuffer[m_ilen] == 0x08 || m_cBuffer[m_ilen] == 0x09 || m_cBuffer[m_ilen] == 0x0a || m_cBuffer[m_ilen] == 0x0d)
					{
						m_ilen++;
						// ã�� ���� �̸��� ���� ã�� �����̸��� �´ٸ�, 
						if (!strcmp(cTempWord, AreaName))
						{
							// { �� ��ġ�� ��´�.  ���� �̸� �ڿ� �����̽� ���� ���� �� �ֱ⶧���� �����ϰ� �ѹ� �� üũ���ִ� �����̴�.
							while (SkipNoneCommand(m_cBuffer[m_ilen]))
							{
								// len�� ���� cBuffer�� ������� Ŀ����, �ش� ���Ͽ��� {�� ���ٴ� ��
								if (m_ilen > strlen(m_cBuffer))
									return false;
								m_ilen++;

							}

							// ã�� ���ڰ� { ��� {�� ��ĭ ���������� �̵�. �׸��� while �� ���Ḧ true�� ����
							if (m_cSkipWord == '{')
							{
								bWhileBreakCheck = true;
								m_ilen++;
							}
						}

						break;
					}

					// ���� �ִ´�.
					// �׸��� ���� ���� �ֱ� ���� ���� �迭�� ��ġ�� 1ĭ �̵�
					cTempWord[i++] = m_cBuffer[m_ilen++];
				}
			}
			else
				m_ilen++;
		}

		// { �� ã������ ����, }�� ��ġ�� ��ƾ� �Ѵ�.
		// { �� ��ġ�� ��ŸƮ, ���� �����ǿ� �ִ´�.
		iStartPos = m_ilen;
		iEndPos = m_ilen;

		while (1)
		{
			// ������ ������ }�� ã�´�. }�� ���� �� ���� while���� �ݺ��Ѵ�.
			while (SkipNoneCommand(m_cBuffer[m_ilen]))
			{
				// len�� ���� cBuffer�� ������� Ŀ����, �о�� ������ ��� ��. false ����.
				// ��, ������ ������ �ôµ� }�� ���ٴ� ��.
				if (m_ilen > strlen(m_cBuffer))
					return false;

				m_ilen++;

			}

			// }�� ã�Ҵٸ�, ��, ������ �������� �����ߴٸ�
			if (m_cBuffer[m_ilen] == '}')
			{
				// ������ ��ġ�� �����Ѵ�.
				iEndPos = --m_ilen;

				// ã�� iStartPos�� iEndPos�� �����ؼ� m_cBuffer�� ���� m_cAreaBuffer�� �����Ѵ�.
				m_cAreaBuffer = new char[iEndPos - iStartPos];	// ����, m_cAreaBuffer�� �����Ҵ�.
				memcpy(m_cAreaBuffer, m_cBuffer + iStartPos, iEndPos);
				break;
			}
			m_ilen++;
		}
	}

	// GetNextWord���� �ܾ� ��ŵ ���θ� üũ�Ѵ�.
	// true�� ��ŵ / false�� ��ŵ�� �ƴϴ�.
	bool Parser::SkipNoneCommand(const char TempSkipWord)
	{
		m_cSkipWord = TempSkipWord;
		// ������� �޸�, ��ħǥ, ����ǥ, �����̽�, �齺���̽�, ��, �����ǵ�, ĳ���� ����, / ����(�ּ�üũ��)
		if (m_cSkipWord == ',' || m_cSkipWord == '.' || m_cSkipWord == '"' || m_cSkipWord == 0x20 || m_cSkipWord == 0x08 || m_cSkipWord == 0x09 || m_cSkipWord == 0x0a || m_cSkipWord == 0x0d || m_cSkipWord == '/')
			return true;

		return false;
	}

	// ���ۿ���, ���� �ܾ� ã��
	bool Parser::GetNextWord()
	{
		int i = 0;
		char cTempWord[256];

		// �ܾ� ���� ��ġ�� ã�´�. true�� ��� ��ŵ�ȴ�.
		while (SkipNoneCommand(m_cAreaBuffer[m_ilen]))
		{
			// len�� ���� cBuffer�� ������� Ŀ����, �迭�� ����Ŵ� false ����.
			if (m_ilen > strlen(m_cAreaBuffer))
				return false;

			m_ilen++;
		}

		// ������ġ�� ã�Ҵ�. ���� �� ��ġ�� ã�ƾ��Ѵ�.
		while (1)
		{
			// len�� ���� cBuffer�� ������� Ŀ����, �迭�� ����Ŵ� false ����.
			if (m_ilen > strlen(m_cBuffer))
				return false;

			// ������� �޸�, ��ħǥ, ����ǥ, �����̽�, �齺���̽�, ��, �����ǵ�, ĳ���� ����
			// �� �� �ϳ��� �ش�Ǹ�, �� ��ġ�� ã���Ŵ� while���� ������.
			else if (m_cAreaBuffer[m_ilen] == ',' || m_cAreaBuffer[m_ilen] == '.' || m_cAreaBuffer[m_ilen] == '"' || m_cAreaBuffer[m_ilen] == 0x20 || m_cAreaBuffer[m_ilen] == 0x08 || m_cAreaBuffer[m_ilen] == 0x09 || m_cAreaBuffer[m_ilen] == 0x0a || m_cAreaBuffer[m_ilen] == 0x0d)
				break;

			// ���� �ִ´�.
			// �׸��� ���� ���� �ֱ� ���� ���� �迭�� ��ġ�� 1ĭ �̵�
			cTempWord[i++] = m_cAreaBuffer[m_ilen++];

		}

		// �ܾ��� �������� �ι��ڸ� �־� ���ڿ��� �ϼ�.
		cTempWord[i] = '\0';
		m_ilen++;

		// ���ڿ� ����.
		memset(m_cWord, 0, 256);
		strcpy_s(m_cWord, sizeof(cTempWord), cTempWord);

		return true;
	}

	// �ļ�_���ϴ� �� ã�ƿ��� (int��)
	bool Parser::GetValue_Int(const char* Name, int* value)
	{
		m_ilen = 0;

		// cBuffer�� ���ڿ��� cWord�� �ű��. ����� �� ���� ���� ���ڿ��� �޾ƿ´�.
		// �޾ƿ��µ� �����ϸ� true, �޾ƿ��µ� �����ϸ� false�̴�. �޾ƿ��µ� �����ϴ� ����, ���������� ��� �˻縦 �� ���̴�.
		while (GetNextWord())
		{
			// ã�� ���ڿ��� ���� ���ϴ� ���ڿ����� Ȯ��. ���� ã�� ���ڿ��� �ƴ϶�� �ٽ� �� while������ ���� Ȯ��.
			if (strcmp(m_cWord, Name) == 0)
			{
				// ���� ã�� ���ڿ��̶��, ���� ���ڷ� = �� ã�´�.
				// if (GetNextWord(cWord, &len))�� true��� �ϴ� ���� ���ڿ��� ã�� ���̴�.
				if (GetNextWord())
				{
					// ���ڿ��� ã������ �� ���ڿ��� ��¥ = ���� Ȯ������.
					if (strcmp(m_cWord, "=") == 0)
					{
						// = ���ڿ��̶��, ���� ���ڿ��� ã�ƿ´�. �̹��� ã�°��� ���� Value�̴�.
						if (GetNextWord())
						{
							// ������� ������, Value�� ã�� ���̴� ���� ���� ����.
							*value = atoi(m_cWord);
							return true;
						}

					}

				}

			}

		}

		// ������� �°�, cBuffer�� ��� �����͸� ã�Ҵµ��� ���� ���ϴ� �����Ͱ� ���� ���̴�, false ��ȯ.
		return false;
	}

	// �ļ�_���ϴ� �� ã�ƿ��� (���ڿ�)
	bool Parser::GetValue_String(const char* Name, char* value)
	{
		m_ilen = 0;

		// cBuffer�� ���ڿ��� cWord�� �ű��. ����� �� ���� ���� ���ڿ��� �޾ƿ´�.
		// �޾ƿ��µ� �����ϸ� true, �޾ƿ��µ� �����ϸ� false�̴�. �޾ƿ��µ� �����ϴ� ����, ���������� ��� �˻縦 �� ���̴�.
		while (GetNextWord())
		{
			// ã�� ���ڿ��� ���� ���ϴ� ���ڿ����� Ȯ��. ���� ã�� ���ڿ��� �ƴ϶�� �ٽ� �� while������ ���� Ȯ��.
			if (strcmp(m_cWord, Name) == 0)
			{
				// ���� ã�� ���ڿ��̶��, ���� ���ڷ� = �� ã�´�.
				// if (GetNextWord(cWord, &len))�� true��� �ϴ� ���� ���ڿ��� ã�� ���̴�.
				if (GetNextWord())
				{
					// ���ڿ��� ã������ �� ���ڿ��� ��¥ = ���� Ȯ������.
					if (strcmp(m_cWord, "=") == 0)
					{
						// = ���ڿ��̶��, ���� ���ڿ��� ã�ƿ´�. �̹��� ã�°��� ���� Value�̴�.
						if (GetNextWord())
						{
							// ������� ������, Value�� ã�� ���̴� ���� ���� ����.
							strcpy_s(value, sizeof(m_cWord), m_cWord);
							return true;
						}

					}

				}

			}

		}

		// ������� �°�, cBuffer�� ��� �����͸� ã�Ҵµ��� ���� ���ϴ� �����Ͱ� ���� ���̴�, false ��ȯ.
		return false;
	}

	Parser::~Parser()
	{
		delete[] m_cBuffer;
	}

}