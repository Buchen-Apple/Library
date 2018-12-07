// Main.cpp : 이 파일에는 'main' 함수가 포함됩니다. 거기서 프로그램 실행이 시작되고 종료됩니다.
//

#include "pch.h"
#include <iostream>
#include "Convertor_Cal.h"

using namespace std;

int main()
{
	char exp[] = "((1-2)+3)*(5-2)";
	Convertor_Cal Change;

	Change.ConvToRPNExp(exp);
	cout << exp << endl;

	cout << Change.EvalTPNExp(exp) << endl;

	return 0;
}
