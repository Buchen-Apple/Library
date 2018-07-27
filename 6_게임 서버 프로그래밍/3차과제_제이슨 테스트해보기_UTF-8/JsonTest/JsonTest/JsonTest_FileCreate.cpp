// JsonTest.cpp: �ܼ� ���� ���α׷��� �������� �����մϴ�.
//

#include "stdafx.h"
#include "rapidjson\document.h"
#include "rapidjson\writer.h"
#include "rapidjson\stringbuffer.h"
#include <iostream>
#include "Header.h"

using namespace rapidjson;

// ���� ���� �� ������ ���� �Լ�
bool FileCreate_UTF8(const char* FileName, const char* pJson, size_t StringSize);

int main()
{
	StringBuffer StringJson;
	Writer< StringBuffer, UTF16<> > Writer(StringJson);

	stUser* UserSave[2];

	stUser* NewUser1 = new stUser;
	NewUser1->m_AccountID = 1;
	TCHAR Nick1[NICK_MAX_LEN] = L"������1";
	_tcscpy_s(NewUser1->m_NickName, _countof(Nick1), Nick1);

	stUser* NewUser2 = new stUser;
	NewUser2->m_AccountID = 1;
	TCHAR Nick2[NICK_MAX_LEN] = L"�����ԱԱԱ�2";
	_tcscpy_s(NewUser2->m_NickName, _countof(Nick2), Nick2);

	UserSave[0] = NewUser1;
	UserSave[1] = NewUser2;

	Writer.StartObject();
	Writer.String(L"Account");
	Writer.StartArray();
	for(int i=0; i<2; ++i)
	{
		Writer.StartObject();
		Writer.String(L"AccountNo");
		Writer.Uint64(UserSave[i]->m_AccountID);
		Writer.String(L"NickName");
		Writer.String(UserSave[i]->m_NickName);
		Writer.EndObject();
	}
	Writer.EndArray();
	Writer.EndObject();

	// pJson���� UTF-8�� ���·� ����ȴ�.
	const char* pJson = StringJson.GetString();
	size_t Size = StringJson.GetSize();	
	
	// ���� ���� �� ������ ���� �� ����
	if(FileCreate_UTF8("json_test.txt", pJson, Size) == true)
		fputs("Json ���� ���� ����!\n", stdout);

	return 0;
}

// ���� ���� �� ������ ���� �Լ�
bool FileCreate_UTF8(const char* FileName, const char* pJson, size_t StringSize)
{
	FILE* fp;			// ���� ��Ʈ��
	size_t iFileCheck;	// ���� ���� ���� üũ, ������ ������ ����. �� 2���� �˵�

	// ���� ����
	iFileCheck = _tfopen_s(&fp, _T("json_test.txt"), _T("wt"));
	//iFileCheck = fopen_s(&fp, FileName, "wb, ccs=UTF-8");
	if (iFileCheck != 0)
	{
		fputs("fopen �����߻�!\n", stdout);
		return false;
	}

	// ���Ͽ� ������ ����
	iFileCheck = fwrite(pJson, 1, StringSize, fp);
	if (iFileCheck != StringSize)
	{
		fputs("fwrite �����߻�!\n", stdout);
		return false;
	}

	fclose(fp);
	return true;
}


