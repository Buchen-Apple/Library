// Main.cpp : 이 파일에는 'main' 함수가 포함됩니다. 거기서 프로그램 실행이 시작되고 종료됩니다.
//

#include "pch.h"
#include <iostream>
#include "Heap.h"
#include <algorithm>
#include <vector>

using namespace std;

int main()
{
	vector<int> aaa;
	aaa.push_back(1);
	aaa.push_back(2);
	aaa.push_back(3);
	aaa.push_back(4);
	aaa.push_back(5);
	aaa.push_back(6);

	make_heap(aaa.begin(), aaa.end(), less<int>());

	sort_heap(aaa.begin(), aaa.end(), less<int>());

	//Heap<int> heap;
	//heap.Init(false);

	//heap.Insert(4);
	//heap.Insert(1);
	//heap.Insert(3);
	//heap.Insert(2);
	//heap.Insert(16);
	//heap.Insert(9);
	//heap.Insert(10);
	//heap.Insert(14);
	//heap.Insert(8);
	//heap.Insert(7);

	//heap.Sort_desc();


	return 0;
}