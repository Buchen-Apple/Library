// Project2.cpp: 콘솔 응용 프로그램의 진입점을 정의합니다.
//

#include "stdafx.h"

int main()
{
	unsigned char val = 10;	//	0000 1010
	int i;

	fputs("10의 바이너리 : ",stdout);

	// val 변수의 가장 좌측값 부터 1과 체크한다.
	// 비트 단위로 체크해(and연산), 값이 1이라면 1, 0이라면 0을 출력한다.
	for (i = 7; i >= 0; --i)
	{		
		if ((1 << i) & val)
			printf("%d", 1);
		else
			printf("%d", 0);
	}

	printf("\n");
    return 0;
}

