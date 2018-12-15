// Main.cpp : 이 파일에는 'main' 함수가 포함됩니다. 거기서 프로그램 실행이 시작되고 종료됩니다.
//

#include "pch.h"
#include <iostream>

using namespace std;

// Countring Sort
void Counting_Sort(int Array[], int Arraylengh, int Temp[], int k, int Result[]);

int main()
{
	int k = 5;
	int aLengh = 8;

	int Array[] = { 2,5,3,0,2,3,0,3 };
	int* Temp = new int[k+1];
	int* Result = new int[aLengh];

	Counting_Sort(Array, aLengh, Temp, k, Result);

	int i = 0;
	while (i < aLengh)
	{
		cout << Result[i] << " ";
		i++;
	}
	cout << endl;

	delete[] Temp;
	delete[] Result;

	return 0;
}

void Counting_Sort(int Array[], int Arraylengh, int Temp[], int k, int Result[])
{
	memset(Temp, 0, sizeof(int) * (k + 1));	

	// Array에는 기존 데이터, Temp에는 정렬 시 사용되는 임시 데이터가 존재.
	
	// 1. 1차 카운트 배열 생성
	// Index : Value
	// Value : Value의 수
	int i = 0;	
	while (i < Arraylengh)
	{
		Temp[Array[i]]++;
		i++;
	}

	// 2. 2차 카운트 배열 생성
	// Index : Value
	// Value : Value의 누적(현재 Value보다 같거나 작은 Value의 수)
	i = 1;
	while (i <= k )
	{
		Temp[i] = Temp[i] + Temp[i-1];
		i++;
	}

	// 3. 정렬 결과 생성.
	// 동적할당 후 결과 배열에 정렬 결과 담기.
	// 완료된 배열의 주소 리턴
	i = 0;
	while (i < Arraylengh)
	{
		Result[Temp[Array[i]] - 1] = Array[i];

		Temp[Array[i]]--;
		i++;		
	}
}
