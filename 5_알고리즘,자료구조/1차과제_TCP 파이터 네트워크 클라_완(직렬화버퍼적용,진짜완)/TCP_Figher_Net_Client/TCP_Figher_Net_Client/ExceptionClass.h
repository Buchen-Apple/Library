#ifndef __EXCEPTION_CLASS_H__
#define __EXCEPTION_CLASS_H__


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

#endif // !__EXCEPTION_CLASS_H__
