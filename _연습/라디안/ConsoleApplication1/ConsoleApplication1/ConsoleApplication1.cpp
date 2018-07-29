// ConsoleApplication1.cpp: 콘솔 응용 프로그램의 진입점을 정의합니다.
//

#include "stdafx.h"
#include <Windows.h>

#define PI 3.14159265

// iDgree에 더하거나 뺼 값
#define VALUE 50

#define MAX	2000

int _tmain()
{
	// 0도부터 시작
	int iDgree = 0;

	// iDgree에 값을 더할지 증가할지 결정
	bool falsg = true;

	while (1)
	{
		// iDgree가, 내가 지정한 끝에 도착했으면 더 이상 증가하면 안된다. 빼기를 위해 flag를 false로 변경	
		if (iDgree >= MAX)
			falsg = false;

		else if(iDgree <= 0)
			falsg = true;

		// 한번 while문 돌 때 마다 VALUE도씩 증가 / 감소
		if(falsg)
			iDgree += VALUE;
		else
			iDgree -= VALUE;
		
		// 증가한 도에 대해 radian 구하기
		double iRadian = (iDgree * PI / 180) *2;

		// 라디안 큼 돌면서 * 찍기
		for (int i = 0; i<iRadian; ++i)
			printf("*");

		fputs("\n", stdout);

		Sleep(30);
	}
	

    return 0;
}

