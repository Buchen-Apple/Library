// Main.cpp : 이 파일에는 'main' 함수가 포함됩니다. 거기서 프로그램 실행이 시작되고 종료됩니다.
//

#include "pch.h"
#include <iostream>
#include "CLinkedList.h"

using namespace std;

bool WorkCheck(const char* Name, int Day, Employee** WorkPeople);

CLinkedList list;

int main()
{
	list.Init();

	// 직원 4명의 데이터 삽입
	Employee* NewEmployee;

	NewEmployee = new Employee;
	NewEmployee->m_iNumber = 1111;
	strcpy_s(NewEmployee->m_cName, 20, "Song");
	list.Insert_Tail(NewEmployee);

	NewEmployee = new Employee;
	NewEmployee->m_iNumber = 2222;
	strcpy_s(NewEmployee->m_cName, 20, "Jin");
	list.Insert_Tail(NewEmployee);

	NewEmployee = new Employee;
	NewEmployee->m_iNumber = 3333;
	strcpy_s(NewEmployee->m_cName, 20, "Gyu");
	list.Insert_Tail(NewEmployee);

	NewEmployee = new Employee;
	NewEmployee->m_iNumber = 4444;
	strcpy_s(NewEmployee->m_cName, 20, "zzzz");
	list.Insert_Tail(NewEmployee);

	// Song의 2일 후 당직자 체크
	if (WorkCheck("Song", 2, &NewEmployee) == false)
	{
		cout << "Not Found..." << endl;
	}
	else
	{
		cout << "[" << NewEmployee->m_cName << " : " << NewEmployee->m_iNumber << "]" << endl;
	}
	

	return 0;
}

bool WorkCheck(const char* Name, int Day, Employee** WorkPeople)
{
	// Name 으로 사람 검색
	int Size = list.Size();

	if (list.First(WorkPeople))
	{
		// 바로 첫 번째가 찾고자 하는 사람일 경우
		if (strcmp((*WorkPeople)->m_cName, Name) == 0)
		{
			// 여기부터 Day만큼 이동한다.
			while (Day > 0)
			{
				list.Next(WorkPeople);
				--Day;
			}

			return true;
		}

		--Size;

		// 첫 번째가 찾고자 하는 사람이 아니라면, Size만큼 돌면서 찾아본다.
		while (Size > 0)
		{
			list.Next(WorkPeople);

			// 원하는 사람을 찾았을 경우
			if (strcmp((*WorkPeople)->m_cName, Name) == 0)
			{
				// 여기부터 Day만큼 이동한다.
				while (Day > 0)
				{
					list.Next(WorkPeople);
					--Day;
				}

				return true;
			}

			--Size;
		}	
	}

	// 여기까지 오면 원하는 사람 못찾은 것. flase 리턴
	return false;	
}