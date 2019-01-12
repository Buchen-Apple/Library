// MPool_Test.cpp : 이 파일에는 'main' 함수가 포함됩니다. 거기서 프로그램 실행이 시작되고 종료됩니다.
//

#include "pch.h"
#include <iostream>
#include "MemoryPool.h"
#include "profiling/Profiling_Class.h"

using namespace Library_Jingyu;

#define SIZE 1000000

class Test
{
	int a;

public:
	Test()
	{
		a = 10;
	}

	void func()
	{
		printf("a : %d\n", a);
	}
};

Test* arr[SIZE];

int main()
{
	// 메모리풀
	CMemoryPoolTLS<Test>* mPool = new CMemoryPoolTLS<Test>(0, false);


	// 개인 제작 프로파일링 사용 
	Profiling profile;

	FREQUENCY_SET();

	char Name1[10];
	char Name2[10];
	char Name3[20];
	char Name4[20];
	for (int i = 0; i < 5; ++i)
	{
		sprintf_s(Name1, 10, "New_%d", i + 1);
		sprintf_s(Name2, 10, "delete_%d", i + 1);

		// New, Delete 테스트
		// New, delete 각각 (100만 * 100 = 1억회) 테스트
		printf("New/Delete Test Start : %d\n", i + 1);
		for (int i = 0; i < 100; ++i)
		{
			BEGIN(Name1);
			for (int i = 0; i < SIZE; ++i)
			{
				arr[i] = new Test;
			}
			END(Name1);

			BEGIN(Name2);
			for (int i = 0; i < SIZE; ++i)
			{
				delete arr[i];
			}
			END(Name2);

		}
		printf("New/Delete Test End : %d\n\n", i + 1);


		sprintf_s(Name3, 20, "TLS Alloc_%d", i + 1);
		sprintf_s(Name4, 20, "TLS Free_%d", i + 1);

		// 메모리풀의 Alloc, Free 테스트
		// Alloc, Free 각각 (100만 * 100 = 1억회) 테스트		
		printf("MPool Alloc/Free Test Start : %d\n", i + 1);
		for (int i = 0; i < 100; ++i)
		{
			BEGIN(Name3);
			for (int i = 0; i < SIZE; ++i)
			{
				arr[i] = mPool->Alloc();
			}
			END(Name3);

			BEGIN(Name4);
			for (int i = 0; i < SIZE; ++i)
			{
				mPool->Free(arr[i]);
			}
			END(Name4);

		}
		printf("MPool Alloc/Free Test End : %d\n\n", i + 1);
	}


	printf("File Save Start\n");
	PROFILING_FILE_SAVE();
	printf("File Save End\n");

	delete mPool;

	return 0;
}

