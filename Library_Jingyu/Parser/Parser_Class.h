#pragma once
#ifndef __PARSER_CLASS_H__
#define __PARSER_CLASS_H__

#include <tchar.h>
#include <windows.h>

class Parser
{
private:
	TCHAR* m_cBuffer;		// ������ �о�� ����
	TCHAR* m_cAreaBuffer = nullptr;	// ���� üũ�� ������ ����. m_cBuffer ��, ���� �ʿ��� ������ ���� ����.
	TCHAR m_cWord[256];		// ��Ī�Ǵ� �ܾ ������ ����
	int m_ilen;				// GetValue �Լ��鿡�� ����ϴ� ���ڿ� ����.	
	TCHAR m_cSkipWord;		// SkipNoneCommand�Լ����� ���� ���ڰ� ��ŵ�ؾ��ϴ� �������� �����ϱ� ���� ����

public:
	// ���� �ε�. 
	// ���Ͽ��� ���� : throw 1 / ���� ������ �о������� : throw 2
	void LoadFile(const TCHAR* FileName);

	// ���� ����
	//
	// Parameter : ���� �̸�(TCHAR)
	// return : ���� ���� ���� �� false	
	bool AreaCheck(const TCHAR* AreaName);

	// GetNextWord���� �ܾ� ��ŵ ���θ� üũ�Ѵ�.
	// true�� ��ŵ / false�� ��ŵ�� �ƴϴ�.
	bool SkipNoneCommand(TCHAR TempSkipWord);

	// ���ۿ���, ���� �ܾ� ã��
	bool GetNextWord();

	// �ļ�_���ϴ� �� ã�ƿ��� (int��)
	bool GetValue_Int(const TCHAR* Name, int* value);

	// �ļ�_���ϴ� �� ã�ƿ��� (���ڿ�)
	bool GetValue_String(const TCHAR* Name, TCHAR* value);

	// �Ҹ���. m_cBuffer, m_cAreaBuffer����Ʈ
	~Parser();
};

#endif // !__PARSER_CLASS_H__

