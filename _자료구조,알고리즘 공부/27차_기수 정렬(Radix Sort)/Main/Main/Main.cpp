// Main.cpp : 이 파일에는 'main' 함수가 포함됩니다. 거기서 프로그램 실행이 시작되고 종료됩니다.
//

#include "pch.h"
#include <iostream>

using namespace std;

void Radix_Sort(int Array[], int ArrayLengh);

int main()
{
	int ArrayLen = 7;

	int Array[] = { 329, 457, 657, 839, 436, 720, 355 };	

	for (int i = 0; i < ArrayLen; ++i)
		cout << Array[i] << " ";

	cout << endl;

	Radix_Sort(Array, ArrayLen);

	for (int i = 0; i < ArrayLen; ++i)
		cout << Array[i] << " ";

	cout << endl;

	return 0;
}


void Radix_Sort(int Array[], int ArrayLengh)
{
	// 1. 최대 자리수 구하기
	int Ciphers = 0;	

	int i = 1;
	while (1)
	{
		if ((Array[0] % i) == Array[0])
		{
			if (Ciphers == 0)
				Ciphers++;

			break;
		}

		i = i *10;
		Ciphers++;
	}

	// 2. 구한 자리수 사용, 카운팅 정렬을 이용해 기수정렬 진행
	int k = 10;
	int* Temp = new int[k];
	int* Result = new int[ArrayLengh];
	int z = 1;

	while (Ciphers > 0)
	{
		memset(Temp, 0, sizeof(int) * k);

		// Array에는 기존 데이터, Temp에는 정렬 시 사용되는 임시 데이터가 존재.

		// 1. 1차 카운트 배열 생성
		// Index : Value
		// Value : Value의 수
		int i = 0;
		while (i < ArrayLengh)
		{
			// 이번 자리수를 기준으로 인덱스 구하기
			int Idx = (Array[i] % (z * 10)) / z;

			Temp[Idx]++;
			i++;
		}

		// 2. 2차 카운트 배열 생성
		// Index : Value
		// Value : Value의 누적(현재 Value보다 같거나 작은 Value의 수)
		i = 1;
		while (i < k)
		{
			Temp[i] = Temp[i] + Temp[i - 1];
			i++;
		}

		// 3. 정렬 결과 생성.
		// 동적할당 후 결과 배열에 정렬 결과 담기.
		// 완료된 배열의 주소 리턴
		i = ArrayLengh - 1;
		while (i >= 0)
		{
			// 이번 자리수를 기준으로 인덱스 구하기
			int Idx = (Array[i] % (z * 10)) / z;

			Result[Temp[Idx] - 1] = Array[i];

			Temp[Idx]--;
			i--;
		}

		// 4. Array에 복사
		i = 0;
		while (i < ArrayLengh)
		{
			Array[i] = Result[i];
			i++;
		}

		z = z * 10;
		Ciphers--;
	}
}
