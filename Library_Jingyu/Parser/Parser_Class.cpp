#include "pch.h"
#include "Parser_Class.h"
#include <iostream>

using namespace std;

// ���� �ε�
void Parser::LoadFile(const TCHAR* FileName)
{
	FILE* fp;			// ���� ��Ʈ��
	size_t iFileCheck;	// ���� ���� ���� üũ, ������ ������ ����. �� 2���� �˵�

	iFileCheck = _tfopen_s(&fp, FileName, _T("rt, ccs=UTF-16LE"));
	if (iFileCheck != 0)
		throw 1;

	fseek(fp, 0, SEEK_END);
	iFileCheck = ftell(fp);
	fseek(fp, 0, SEEK_SET);

	m_cBuffer = new TCHAR[iFileCheck];

	iFileCheck = fread_s(m_cBuffer, iFileCheck, iFileCheck, 1, fp);
	if (iFileCheck != 0)
		throw 2;

	fclose(fp);	// �� �� ���� ��Ʈ�� ����
}

// ���� ����
//
// Parameter : ���� �̸�(TCHAR)
// return : ���� ���� ���� �� false	
bool Parser::AreaCheck(const TCHAR* AreaName)
{
	int i = 0;
	TCHAR cTempWord[256];
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
			if (m_ilen > _tcslen(m_cBuffer))
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
				if (m_ilen > _tcslen(m_cBuffer))
					return false;

				// ������� �޸�, ��ħǥ, ����ǥ, �����̽�, �齺���̽�, ��, �����ǵ�, ĳ���� ����
				// �� �� �ϳ��� �ش�Ǹ�, ���ڸ� �ϳ� �ϼ��� ��. ��, { �ٷ� ����(����) ��ġ�� ã�� ��.
				else if (m_cBuffer[m_ilen] == ',' || m_cBuffer[m_ilen] == '.' || m_cBuffer[m_ilen] == '"' || m_cBuffer[m_ilen] == 0x20 || m_cBuffer[m_ilen] == 0x08 || m_cBuffer[m_ilen] == 0x09 || m_cBuffer[m_ilen] == 0x0a || m_cBuffer[m_ilen] == 0x0d)
				{
					m_ilen++;
					// ã�� ���� �̸��� ���� ã�� �����̸��� �´ٸ�, 
					if (! _tcscmp(cTempWord, AreaName))
					{
						// { �� ��ġ�� ��´�.  ���� �̸� �ڿ� �����̽� ���� ���� �� �ֱ⶧���� �����ϰ� �ѹ� �� üũ���ִ� �����̴�.
						while (SkipNoneCommand(m_cBuffer[m_ilen]))
						{
							// len�� ���� cBuffer�� ������� Ŀ����, �ش� ���Ͽ��� {�� ���ٴ� ��
							if (m_ilen > _tcslen(m_cBuffer))
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
				cTempWord[i] = '\0';
			}
		}
		else
			m_ilen++;
	}	

	// { �� ã������ ����, }�� ��ġ�� ��ƾ� �Ѵ�.
	// { �� ��ġ�� ��ŸƮ, ���� �����ǿ� �ִ´�.
	iStartPos = m_ilen;
	iEndPos = iStartPos;

	while (1)
	{
		// ������ ������ }�� ã�´�. }�� ���� �� ���� while���� �ݺ��Ѵ�.
		while (SkipNoneCommand(m_cBuffer[m_ilen]))
		{
			// len�� ���� cBuffer�� ������� Ŀ����, �о�� ������ ��� ��. false ����.
			// ��, ������ ������ �ôµ� }�� ���ٴ� ��.
			if (m_ilen > _tcslen(m_cBuffer))
			//if (m_ilen > strlen(m_cBuffer))
				return false;

			m_ilen++;

		}

		// }�� ã�Ҵٸ�, ��, ������ �������� �����ߴٸ�
		if (m_cBuffer[m_ilen] == '}')
		{
			// ������ ��ġ�� �����Ѵ�.
			iEndPos = m_ilen;

			// ã�� iStartPos�� iEndPos�� �����ؼ� m_cBuffer�� ���� m_cAreaBuffer�� �����Ѵ�.
			m_cAreaBuffer = new TCHAR[iEndPos];

			memcpy(m_cAreaBuffer, m_cBuffer + iStartPos, (iEndPos - iStartPos) * sizeof(TCHAR));
			break;
		}
		m_ilen++;

	}

	return true;
}

// GetNextWord���� �ܾ� ��ŵ ���θ� üũ�Ѵ�.
// true�� ��ŵ / false�� ��ŵ�� �ƴϴ�.
bool Parser::SkipNoneCommand(TCHAR TempSkipWord)
{
	m_cSkipWord = TempSkipWord;

	// ������� �޸�, ����ǥ, �����̽�, �齺���̽�, ��, �����ǵ�, ĳ���� ����, / ����(�ּ�üũ��)
	if (m_cSkipWord == ',' || m_cSkipWord == '"' || m_cSkipWord == 0x20 || m_cSkipWord == 0x08 || m_cSkipWord == 0x09 || m_cSkipWord == 0x0a || m_cSkipWord == 0x0d || m_cSkipWord == '/')
		return true;

	return false;
}

// ���ۿ���, ���� �ܾ� ã��
bool Parser::GetNextWord()
{
	int i = 0;
	TCHAR cTempWord[256];

	// �ܾ� ���� ��ġ�� ã�´�. true�� ��� ��ŵ�ȴ�.
	while (SkipNoneCommand(m_cAreaBuffer[m_ilen]))
	{
		// len�� ���� cBuffer�� ������� Ŀ����, �迭�� ����Ŵ� false ����.
		if (m_ilen > _tcslen(m_cAreaBuffer))
			return false;

		m_ilen++;
	}

	// ������ġ�� ã�Ҵ�. ���� �� ��ġ�� ã�ƾ��Ѵ�.
	while (1)
	{
		// len�� ���� cBuffer�� ������� Ŀ����, �迭�� ����Ŵ� false ����.
		if (m_ilen > _tcslen(m_cBuffer))
			return false;

		// ������� �޸�, ��ħǥ, ����ǥ, �����̽�, �齺���̽�, ��, �����ǵ�, ĳ���� ����
		// �� �� �ϳ��� �ش�Ǹ�, �� ��ġ�� ã���Ŵ� while���� ������.
		else if (m_cAreaBuffer[m_ilen] == ',' || m_cAreaBuffer[m_ilen] == '"' || m_cAreaBuffer[m_ilen] == 0x20 || m_cAreaBuffer[m_ilen] == 0x08 || m_cAreaBuffer[m_ilen] == 0x09 || m_cAreaBuffer[m_ilen] == 0x0a || m_cAreaBuffer[m_ilen] == 0x0d)
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
	_tcscpy_s(m_cWord, sizeof(cTempWord), cTempWord);

	return true;
}

// �ļ�_���ϴ� �� ã�ƿ��� (int��)
bool Parser::GetValue_Int(const TCHAR* Name, int* value)
{
	m_ilen = 0;

	// cBuffer�� ���ڿ��� cWord�� �ű��. ����� �� ���� ���� ���ڿ��� �޾ƿ´�.
	// �޾ƿ��µ� �����ϸ� true, �޾ƿ��µ� �����ϸ� false�̴�. �޾ƿ��µ� �����ϴ� ����, ���������� ��� �˻縦 �� ���̴�.
	while (GetNextWord())
	{
		// ã�� ���ڿ��� ���� ���ϴ� ���ڿ����� Ȯ��. ���� ã�� ���ڿ��� �ƴ϶�� �ٽ� �� while������ ���� Ȯ��.
		if (_tcscmp(m_cWord, Name) == 0)
		{
			// ���� ã�� ���ڿ��̶��, ���� ���ڷ� = �� ã�´�.
			// if (GetNextWord(cWord, &len))�� true��� �ϴ� ���� ���ڿ��� ã�� ���̴�.
			if (GetNextWord())
			{
				// ���ڿ��� ã������ �� ���ڿ��� ��¥ = ���� Ȯ������.
				if (_tcscmp(m_cWord, _T("=")) == 0)
				{
					// = ���ڿ��̶��, ���� ���ڿ��� ã�ƿ´�. �̹��� ã�°��� ���� Value�̴�.
					if (GetNextWord())
					{
						// ������� ������, Value�� ã�� ���̴� ���� ���� ����.
						*value = _ttoi(m_cWord);
						//*value = atoi(m_cWord);
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
bool Parser::GetValue_String(const TCHAR* Name, TCHAR* value)
{
	m_ilen = 0;

	// cBuffer�� ���ڿ��� cWord�� �ű��. ����� �� ���� ���� ���ڿ��� �޾ƿ´�.
	// �޾ƿ��µ� �����ϸ� true, �޾ƿ��µ� �����ϸ� false�̴�. �޾ƿ��µ� �����ϴ� ����, ���������� ��� �˻縦 �� ���̴�.
	while (GetNextWord())
	{
		// ã�� ���ڿ��� ���� ���ϴ� ���ڿ����� Ȯ��. ���� ã�� ���ڿ��� �ƴ϶�� �ٽ� �� while������ ���� Ȯ��.
		if (_tcscmp(m_cWord, Name) == 0)
		{
			// ���� ã�� ���ڿ��̶��, ���� ���ڷ� = �� ã�´�.
			// if (GetNextWord(cWord, &len))�� true��� �ϴ� ���� ���ڿ��� ã�� ���̴�.
			if (GetNextWord())
			{
				// ���ڿ��� ã������ �� ���ڿ��� ��¥ = ���� Ȯ������.
				if (_tcscmp(m_cWord, _T("=")) == 0)
				{
					// = ���ڿ��̶��, ���� ���ڿ��� ã�ƿ´�. �̹��� ã�°��� ���� Value�̴�.
					if (GetNextWord())
					{
						// ������� ������, Value�� ã�� ���̴� ���� ���� ����.						
						_tcscpy_s(value, sizeof(m_cWord), m_cWord);
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
	delete m_cBuffer;
	delete m_cAreaBuffer;
}