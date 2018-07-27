#include "stdafx.h"
#include "ExceptionClass.h"

// ������
CException::CException(const wchar_t* str)
{
	_tcscpy_s(ExceptionText, _countof(ExceptionText), str);
}

// ���� �ؽ�Ʈ�� �ּ� ��ȯ
char* CException::GetExceptionText()
{
	return (char*)&ExceptionText;
}