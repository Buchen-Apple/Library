#include <stdio.h>
#include <stdlib.h>
#include <tchar.h>
#include <locale.h>
#include <windows.h>

#define STR_LEN 256
#define CMD_TOKEN_NUM 10

TCHAR ERROR_CMD[] = TEXT("'%s'��(��) ������ �� �ִ� ���α׷��� �ƴմϴ�. \n");

int CmdProcessing();
TCHAR* StrLower(TCHAR*);

int _tmain(int argc, TCHAR* argv[])
{
	// �ѱ� �Է��� ������ �ϱ� ����
	_tsetlocale(LC_ALL, TEXT("koeran"));

	DWORD isExit;
	while (1)
	{
		isExit = CmdProcessing();

		if (isExit)
		{
			_fputts(TEXT("��ɾ� ó���� �����մϴ�. \n"), stdout);
			break;
		}
	}
	return 0;
}

TCHAR cmdString[STR_LEN];
TCHAR cmdTokenList[CMD_TOKEN_NUM][STR_LEN];
TCHAR seps[] = TEXT(" ,\t\n");

int CmdProcessing()
{
	_fputts(TEXT("Best Command Prompt>> "), stdout);
	_getts_s(cmdString);
	TCHAR* token = _tcstok(cmdString, seps);
	int tokenNum = 0;
	while (token != NULL)
	{
		_tcscpy(cmdTokenList[tokenNum++], StrLower(token));
		token = _tcstok(NULL, seps);
	}

	if (!_tcscmp(cmdTokenList[0], TEXT("exit")))
		return TRUE;

	else if (!_tccmp(cmdTokenList[0], TEXT("�߰��Ǵ� ��ɾ�1")))
	{
	}

	else if (!_tccmp(cmdTokenList[0], TEXT("�߰��Ǵ� ��ɾ�2")))
	{
	}

	else 
	{
		_tprintf(ERROR_CMD, cmdTokenList[0]);
	}

	return 0;
}

TCHAR* StrLower(TCHAR *pStr)
{
	TCHAR *ret = pStr;

	while (*pStr)
	{
		if (_istupper(*pStr))
			*pStr = _totlower(*pStr);

		pStr++;
	}

	return ret;
}