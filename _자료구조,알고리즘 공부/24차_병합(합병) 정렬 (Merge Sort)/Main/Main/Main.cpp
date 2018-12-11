// Main.cpp : 이 파일에는 'main' 함수가 포함됩니다. 거기서 프로그램 실행이 시작되고 종료됩니다.
//

#include "pch.h"
#include <iostream>

void MergeSort(int Array[], int LeftIdx, int RightIdx);
void Merge(int Array[], int Left, int Mid, int Right);

int main()
{
	int Array[] = {8,2,3,7,1,5,4,6};

	MergeSort(Array, 0, 7);

	return 0;
}

void MergeSort(int Array[], int LeftIdx, int RightIdx)
{
	// 1. Left가 Right와 같거나, Left가 Right를 추월하면 return 
	if (LeftIdx >= RightIdx)
		return;

	// 2. Left가 Right보다 작다면, 아직 분할할 것이 있는것.
	// 중간 지점을 찾는다,
	int Mid = (LeftIdx + RightIdx) / 2;

	// 3. 둘로 나눠서 각각 정렬
	MergeSort(Array, LeftIdx, Mid);
	MergeSort(Array, Mid+1, RightIdx);

	// 4. 병합
	Merge(Array, LeftIdx, Mid, RightIdx);
}

void Merge(int Array[], int Left, int Mid, int Right)
{
	// 1. 정렬된 데이터를 임시로 보관할  배열 생성
	int *Temp = new int[Right + 1];

	// 2. 각종 변수들
	int i = Left;
	int j = Mid + 1;
	int k = Left;

	// 3. 실제 데이터 정렬 파트
	while (i <= Mid && j <= Right)
	{
		// 왼쪽 데이터가 더 작거나 같다면
		if (Array[i] <= Array[j])
		{
			Temp[k] = Array[i];
			k++;
			i++;
		}

		// 오른쪽 데이터가 더 작다면
		else
		{
			Temp[k] = Array[j];
			k++;
			j++;
		}
	}

	// 4. 한 쪽만 정렬되어도, 데이터 정렬은 완료된다.
	
	// 아직 왼쪽 데이터가 덜 정렬된 경우, 왼쪽 데이터를 그대로 뒤에 가져다 붙인다.
	while(i <= Mid)
	{
		Temp[k] = Array[i];
		k++;
		i++;
	}

	// 오른쪽 데이터가 덜 정렬된 경우
	while(j <= Right)
	{
		Temp[k] = Array[j];
		k++;
		j++;
	}

	// 5. 정렬된 데이터를 다시 Array로 복사
	i = Left;
	while (i <= Right)
	{
		Array[i] = Temp[i];
		i++;
	}

	delete Temp;
}