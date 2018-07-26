#include <stdio.h>
#include <tchar.h>
#include <windows.h>
#include <locale.h>

#define SLOT_NAME	_T("\\\\.\\mailslot\\mailbox")

int _tmain(int argc, TCHAR* argv[])
{
	_tsetlocale(LC_ALL, _T("korean"));

	HANDLE hMailSlot;
	TCHAR MsgBox[50];
	DWORD byteRead;
	TCHAR str[50];

	// MailSlot ���� //
	hMailSlot = CreateMailslot(
		SLOT_NAME,
		0,
		MAILSLOT_WAIT_FOREVER,
		NULL);

	if (hMailSlot == INVALID_HANDLE_VALUE)
	{
		_tcscpy_s(str, _countof(str), _T("���Ͻ��� ���� ����!"));
		wprintf(_T("%s\n"), str);
		return 1;
	}

	// MailSlot ����//
	_fputts(_T("******* Message ******* \n"), stdout);
	while (1)
	{
		if (!ReadFile(hMailSlot, MsgBox, sizeof(TCHAR) * 50, &byteRead, NULL))
		{
			_tcscpy_s(str, _countof(str), _T("������ �о���� ����!"));
			wprintf(_T("%s\n"), str);
			CloseHandle(hMailSlot);
			return 1;
		}

		if (!_tcsncmp(MsgBox, _T("exit"), 4))
		{
			_tcscpy_s(str, _countof(str), _T("�߰�!"));
			wprintf(_T("%s\n"), str);
			break;
		}

		MsgBox[byteRead / sizeof(TCHAR)] = 0;
		_fputts(MsgBox, stdout);

	}

	CloseHandle(hMailSlot);	
	return 0;
}