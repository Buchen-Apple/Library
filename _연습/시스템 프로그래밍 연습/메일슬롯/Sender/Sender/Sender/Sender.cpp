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
	DWORD byteWrite;
	TCHAR str[50];

	SECURITY_ATTRIBUTES sa;
	sa.nLength = sizeof(sa);
	sa.lpSecurityDescriptor = NULL;
	sa.bInheritHandle = TRUE;	// �ڵ��� ��� ����(Y,N). TRUE�� Y

	// MailSlot ��Ʈ�� ���� //
	hMailSlot = CreateFile(
		SLOT_NAME,
		GENERIC_WRITE,
		FILE_SHARE_READ,
		&sa,	// ������ sa�� �־���. ���� �����Ǵ� ������ ��Ʈ�� ���ҽ��� �ڵ��� ��� Y���°� �Ǿ���.
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		0);

	if (hMailSlot == INVALID_HANDLE_VALUE)
	{
		_tcscpy_s(str, _countof(str), _T("���Ͻ��� ��Ʈ�� ���� ����!"));
		wprintf(_T("%s \n"), str);
		return 1;
	}

	_tprintf(_T("Inheritable Handle : %d \n"), hMailSlot);

	// �ڵ� ������ ������ �̿��� �ڽ� ���μ������� �����Ѵ�.
	FILE* fp;
	_tfopen_s(&fp, _T("InheritableHandle.txt"), _T("wt"));
	_ftprintf(fp, _T("%d"), hMailSlot);
	fclose(fp);

	STARTUPINFO si = { 0, };
	si.cb = sizeof(si);

	PROCESS_INFORMATION pi;

	TCHAR command[] = _T("Sender2.exe");

	CreateProcess(NULL, command,
		NULL, NULL,
		TRUE,
		CREATE_NEW_CONSOLE,
		NULL, NULL,
		&si, &pi);

	// Receiver�� ���Ͻ������� ������ ������
	while (1)
	{
		_tcscpy_s(str, _countof(str), _T("���� CMD>"));
		wprintf(_T("%s "), str);
		_fgetts(MsgBox, sizeof(MsgBox) / sizeof(TCHAR), stdin);

		if (!WriteFile(hMailSlot, MsgBox, _tcslen(MsgBox) * sizeof(TCHAR), &byteWrite, NULL))
		{
			_tcscpy_s(str, _countof(str), _T("������ ������ ����!"));
			wprintf(_T("%s \n"), str);
			CloseHandle(hMailSlot);
			return 1;
		}

		if (!_tcscmp(MsgBox, _T("exit")))
		{
			_tcscpy_s(str, _countof(str), _T("�߰�!"));
			wprintf(_T("%s \n"), str);
			break;
		}
	}

	CloseHandle(hMailSlot);
	return 0;
}