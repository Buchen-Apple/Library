// ProtocolBuff_SmartPointer.cpp: 콘솔 응용 프로그램의 진입점을 정의합니다.
//

#include "stdafx.h"
#include "stdlib.h"
#include <iostream>
#include "My_Smart_Ptr\My_Smart_Ptr.h"
#include "ProtocolBuff\ProtocolBuff.h"

using namespace Library_Jingyu;

class CTest
{
	char* Text;

public:
	CTest(const char* refText)
	{
		size_t len = (strlen(refText)) +1;
		Text = new char[len];

		strcpy_s(Text, len, refText);		
	}

	void Show()
	{
		printf("%s\n", Text);
	}


};





int _tmain()
{
	My_Smart_PTR<CTest> Test(new CTest("SongText01"));
	Test->Show();

	My_Smart_PTR<CTest> Test_Copy= Test;
	Test_Copy->Show();

	printf("%d\n", Test.GetRefCount());
	printf("%d\n", Test_Copy.GetRefCount());


	My_Smart_PTR<CTest> Test2(new CTest("SongText01"));
	printf("%d\n", Test2.GetRefCount());
	Test2->Show();







    return 0;
}

