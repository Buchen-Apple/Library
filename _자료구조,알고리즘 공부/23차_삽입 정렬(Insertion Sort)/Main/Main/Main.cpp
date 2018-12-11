// Main.cpp : 이 파일에는 'main' 함수가 포함됩니다. 거기서 프로그램 실행이 시작되고 종료됩니다.
//

#include "pch.h"
#include <iostream>

void Sort(int Arry[], int Size);

int main()
{
	int Array[10];	

	// 데이터 넣기
	Array[0] = 5;
	Array[1] = 3;
	Array[2] = 2;
	Array[3] = 4;
	Array[4] = 1;

	// 정렬하기
	Sort(Array, 5);

	return 0;
}


void Sort(int Arry[], int Size)
{	
	int NowIndex = 1;

	// NowIndex는, 위치를 찾을 인덱스.
	// 해당 인덱스가 들어갈 위치를 찾는다.
	// 끝에 도달할 때 까지
	while (NowIndex < Size)
	{
		int StartIndex = NowIndex-1;

		// NowIndex의 값
		int NowValue = Arry[NowIndex];

		while (StartIndex >= 0)
		{
			// StartIndex의 값이 NowIndex의 값 보다 작다면, break
			// 내가 들어갈 위치를 찾은것.
			if (Arry[StartIndex] < NowValue)
				break;

			// 그게 아니라면 위치만 1칸 밀기
			else
				Arry[StartIndex + 1] = Arry[StartIndex];

			StartIndex--;
		}

		// 찾은 위치에 넣기
		Arry[StartIndex + 1] = NowValue;

		NowIndex++;
	}
}
