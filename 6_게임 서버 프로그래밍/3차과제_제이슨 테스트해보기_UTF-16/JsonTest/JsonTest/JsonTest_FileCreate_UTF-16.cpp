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
bool FileCreate_UTF16(const TCHAR* FileName, const TCHAR* pJson, size_t StringSize);

int main()
{
	GenericStringBuffer<UTF16<>> StringJson;
	Writer< GenericStringBuffer<UTF16<>>, UTF16<>, UTF16<> > Writer(StringJson);

	stUser* UserSave[2];

	stUser* NewUser1 = new stUser;
	NewUser1->m_AccountID = 1;
	TCHAR Nick1[NICK_MAX_LEN] = L"������1";
	_tcscpy_s(NewUser1->m_NickName, _countof(Nick1), Nick1);

	stUser* NewUser2 = new stUser;
	NewUser2->m_AccountID = 2;
	TCHAR Nick2[NICK_MAX_LEN] = L"�����ԱԱԱ�2";
	_tcscpy_s(NewUser2->m_NickName, _countof(Nick2), Nick2);

	UserSave[0] = NewUser1;
	UserSave[1] = NewUser2;

	Writer.StartObject();
	Writer.String(L"Account");
	Writer.StartArray();
	for (int i = 0; i<2; ++i)
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

	// tpJson���� UTF-16�� ���·� ����ȴ�.
	const TCHAR* tpJson = StringJson.GetString();
	size_t Size = StringJson.GetSize();

	// ���� ���� �� ������ ���� �� ����
	if (FileCreate_UTF16(_T("json_test.txt"), tpJson, Size) == true)
		fputs("Json ���� ���� ����!\n", stdout);

	return 0;


	//StringBuffer StringJson;
	//Writer< StringBuffer, UTF16<> > Writer(StringJson);

	//stUser* UserSave[2];

	//stUser* NewUser1 = new stUser;
	//NewUser1->m_AccountID = 1;
	//TCHAR Nick1[NICK_MAX_LEN] = L"������1";
	//_tcscpy_s(NewUser1->m_NickName, _countof(Nick1), Nick1);

	//stUser* NewUser2 = new stUser;
	//NewUser2->m_AccountID = 1;
	//TCHAR Nick2[NICK_MAX_LEN] = L"�����ԱԱԱ�2";
	//_tcscpy_s(NewUser2->m_NickName, _countof(Nick2), Nick2);

	//UserSave[0] = NewUser1;
	//UserSave[1] = NewUser2;

	//Writer.StartObject();
	//Writer.String(L"Account");
	//Writer.StartArray();
	//for(int i=0; i<2; ++i)
	//{
	//	Writer.StartObject();
	//	Writer.String(L"AccountNo");
	//	Writer.Uint64(UserSave[i]->m_AccountID);
	//	Writer.String(L"NickName");
	//	Writer.String(UserSave[i]->m_NickName);
	//	Writer.EndObject();
	//}
	//Writer.EndArray();
	//Writer.EndObject();

	//// pJson���� UTF-8�� ���·� ����ȴ�.
	//const char* pJson = StringJson.GetString();
	//
	//// pJson�� UTF-8�� UTF-16���� ��ȯ.
	//StringStream source(pJson);
	//GenericStringBuffer<UTF16<>> target;

	//bool hasError = true;
	//while (source.Peek() != '\0')
	//{
	//	if (!Transcoder< UTF8<>, UTF16<> >::Transcode(source, target))
	//	{
	//		hasError = false;
	//		break;
	//	}
	//}
	//
	//if (!hasError)
	//{
	//	fputs("UTF-8�� UTF-16���� ��ȯ �� ���� �߻�\n", stdout);
	//	return 0;
	//}

	//const TCHAR* tpJson = target.GetString();

	//size_t Size = target.GetSize();
	//
	//// ���� ���� �� ������ ���� �� ����
	//if(FileCreate_UTF16(_T("json_test.txt"), tpJson, Size) == true)
	//	fputs("Json ���� ���� ����!\n", stdout);

	//return 0;
}

// ���� ���� �� ������ ���� �Լ�
bool FileCreate_UTF16(const TCHAR* FileName, const TCHAR* tpJson, size_t StringSize)
{
	FILE* fp;			// ���� ��Ʈ��
	size_t iFileCheck;	// ���� ���� ���� üũ, ������ ������ ����. �� 2���� �˵�
	
	// ���� ���� (UTF-16 ��Ʋ�����)
	iFileCheck = _tfopen_s(&fp, FileName, _T("wt, ccs=UTF-16LE"));
	if (iFileCheck != 0)
	{
		fputs("fopen �����߻�!\n", stdout);
		return false;
	}

	// ���Ͽ� ������ ����
	iFileCheck = fwrite(tpJson, 1, StringSize, fp);
	if (iFileCheck != StringSize)
	{
		fputs("fwrite �����߻�!\n", stdout);
		return false;
	}

	fclose(fp);
	return true;
}


