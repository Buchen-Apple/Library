// Main.cpp : 이 파일에는 'main' 함수가 포함됩니다. 거기서 프로그램 실행이 시작되고 종료됩니다.
//

#include "pch.h"
#include <iostream>

using namespace std;

void QuickSort(int Array[], int Left, int Right);

int Partition(int Array[], int Left, int Right);

int PivotIndex(int Array[], int Left, int Rigt);

int main()
{
	int Array[] = { 1,3,2,6,4,7,5,8 };

	for (int i = 0; i < 8; ++i)
		cout << Array[i] << " ";
	cout << endl;

	QuickSort(Array, 0, 7);

	for (int i = 0; i < 8; ++i)
		cout << Array[i] << " ";
	cout << endl;

	return 0;
}

void QuickSort(int Array[], int Left, int Right)
{
	// Left가 Right와 같거나 넘어갔다면 더 이상 분할할 것이 없음
	if (Left >= Right)
		return;

	// 파티션 후 중간값 리턴
	int Pivot = Partition(Array, Left, Right);

	// 더 이상 분할 불가능할 때 까지 분할
	QuickSort(Array, Left, Pivot-1);
	QuickSort(Array, Pivot+1, Right);
}

int Partition(int Array[], int Left, int Right)
{
	// 1. 피벗 결정
	// 룰에 따라 피벗 가져옴
	int x = PivotIndex(Array, Left, Right);

	// 임시 피벗과 Right의 값을 Swap
	int Temp = Array[Right];
	Array[Right] = Array[x];
	Array[x] = Temp;

	// 결국 피벗은 Right
	x = Right;

	// 각종 변수 셋팅
	int i = Left - 1;
	int j = Left;

	// 2. 분할 및 정복
	while (j < Right)
	{
		// 피벗보다 j위치의 값이 더 작다면
		if (Array[j] < Array[x])
		{
			++i;
			int Temp = Array[i];
			Array[i] = Array[j];
			Array[j] = Temp;
		}

		j++;
	}

	// 마지막으로 i+1위치의 값과 pivot위치의 값을 Swap
	Temp = Array[i + 1];
	Array[i + 1] = Array[x];
	Array[x] = Temp;

	return i + 1;
}

int PivotIndex(int Array[], int Left, int Right)
{
	// 1. 버블정렬로, 중간 값이 있는 배열의 인덱스를 알아낸다.
	int IndexArray[3] = { Left, (Left + Right) / 2, Right };

	if (Array[IndexArray[0]] > Array[IndexArray[1]])
	{
		int Temp = IndexArray[0];
		IndexArray[0] = IndexArray[1];
		IndexArray[1] = Temp;
	}

	if (Array[IndexArray[1]] > Array[IndexArray[2]])
	{
		int Temp = IndexArray[1];
		IndexArray[1] = IndexArray[2];
		IndexArray[2] = Temp;
	}

	if (Array[IndexArray[0]] > Array[IndexArray[1]])
	{
		int Temp = IndexArray[0];
		IndexArray[0] = IndexArray[1];
		IndexArray[1] = Temp;
	}

	// 2. 인덱스를 리턴한다. 값 아님.
	return IndexArray[1];
}