// ConsoleApplication1.cpp: 콘솔 응용 프로그램의 진입점을 정의합니다.
//

#include "stdafx.h"


void BubbleSort(int Array[], int Size, bool Rule = true);

int _tmain()
{
	int Array[] = { 0, 8, 7, 1, 2, 5, 4, 9 };

	int Size = sizeof(Array) / sizeof(int);

	for (int i = 0; i < Size; ++i)
		printf("%d ", Array[i]);

	fputs("\n", stdout);

	BubbleSort(Array, Size, true);

	for (int i = 0; i < Size; ++i)
		printf("%d ", Array[i]);

	fputs("\n", stdout);
	

    return 0;
}

// rule이 true면 오름차순,  false면 내림차순
void BubbleSort(int Array[], int Size, bool Rule)
{
	int Count = 1;


	int WhileCount = 0;
	while (WhileCount < Size -1)
	{
		for (int h = 0; h < Size - Count; ++h)
		{
			// -----------
			// 실제 버블정렬 파트
			// -----------
			// Rule 값이 true면 오름차순 로직. (작은수가 <<)
			if (Rule == true)
			{
				// 왼쪽의 값이 더 크다면, 오른쪽의 값을 왼쪽으로 옮긴다.
				if (Array[h] > Array[h + 1])
				{
					int Temp = Array[h];
					Array[h] = Array[h + 1];
					Array[h + 1] = Temp;
				}
			}

			// Rule 값이 false면 내림차순 로직(큰수가 <<)
			else
			{
				// 왼쪽의 값이 더 작다면, 오른쪽의 값을 왼쪽으로 옮긴다.
				if (Array[h] < Array[h + 1])
				{
					int Temp = Array[h];
					Array[h] = Array[h + 1];
					Array[h + 1] = Temp;
				}

			}
		}

		Count++;
		WhileCount++;
	}
}

