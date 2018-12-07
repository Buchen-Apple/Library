// Main.cpp : 이 파일에는 'main' 함수가 포함됩니다. 거기서 프로그램 실행이 시작되고 종료됩니다.
//

#include "pch.h"
#include <iostream>
#include "NodeList2.h"
#include "Point.h"

using namespace std;

// 정렬 함수
bool CompareFunc(Point* D1, Point* D2)
{
	if (D1->m_iXpos < D2->m_iXpos)
		return true;

	else if (D1->m_iXpos == D2->m_iXpos)
	{
		if (D1->m_iYpos < D2->m_iYpos)
			return true;
	}

	return false;
}

int main()
{
    // 리스트 생성 및 초기화
	NodeList2 list;
	list.Init();

	// 데이터 정렬 함수 추가 (오름차순)
	list.SetSortRule(CompareFunc);

	// 데이터 삽입	
	Point* pData;	

	pData = new Point;
	pData->m_iXpos = 0;
	pData->m_iYpos = 0;
	list.Insert(pData);

	pData = new Point;
	pData->m_iXpos = 1;
	pData->m_iYpos = 1;
	list.Insert(pData);

	pData = new Point;
	pData->m_iXpos = 1;
	pData->m_iYpos = 0;
	list.Insert(pData);	

	// 전체 출력
	Point* pTemp;
	cout << "Total NodeCount : " << list.Size() << endl;
	if (list.LFirst(&pTemp))
	{
		cout << "[X:" << pTemp->m_iXpos << ", Y:" << pTemp->m_iYpos << "]" << endl;

		while (list.LNext(&pTemp))
		{
			cout << "[X:" << pTemp->m_iXpos << ", Y:" << pTemp->m_iYpos << "]" << endl;
		}
	}
	cout << endl;	

	return 0;
}