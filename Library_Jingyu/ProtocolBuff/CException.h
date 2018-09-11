#ifndef __EXCEPTION_H__
#define __EXCEPTION_H__

#include <Windows.h>
#include <tchar.h>

namespace Library_Jingyu
{

#define _MyCountof(_array)		sizeof(_array) / (sizeof(_array[0]))

	// Recv()��Ŷ ó�� ��, ���� �߻� �� ������ ����Ŭ�����̴�.
	class CException
	{
	private:
		wchar_t ExceptionText[100];

	public:
		// ������
		CException(const wchar_t* str)
		{
			_tcscpy_s(ExceptionText, _MyCountof(ExceptionText), str);
		}

		// ���� �ؽ�Ʈ ������ ��ȯ
		char* GetExceptionText()
		{
			return (char*)&ExceptionText;
		}
	};	
}


#endif // !__EXCEPTION_H__

