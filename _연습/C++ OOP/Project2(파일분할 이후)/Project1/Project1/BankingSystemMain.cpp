/*
* ������Ʈ ���� : [2018�� 1�� 13��] ���� 0.7
*/

// 7. ���� ����

#include "BankingCommonDecl.h"  
/*
�Ʒ� ������ϵ��� AccountHandler.cpp�� ����Ǿ� �ִ�. �׸��� AccountHandler.cpp�� AccountHandler.h�� ��ũ��� �Ѵ�. 
������, �ش� BankingSystemMain.cpp �� ����� ���� �̹� �Ʒ� ������ϵ��� �� ��ũ��� �� �����̴�.
#include "Account.h"
#include "NormalAccount.h"
#include "HighCreditAccount.h"
*/
#include "AccountHandler.h"

int main()
{
	int iInput;
	AccountHandler AcHandler;

	while (1)
	{
		system("cls");

		cout << "----Menu----" << endl;
		cout << "1. ���� ����" << endl;
		cout << "2. �� ��" << endl;
		cout << "3. �� ��" << endl;
		cout << "4. �������� ��ü ���" << endl;
		cout << "5. ���α׷� ����" << endl;
		cin >> iInput;

		switch (iInput)
		{
		case CREATE:
			AcHandler.CreateAccount();
			_getch();

			break;
		case INSERT:
			AcHandler.InsertFunc();
			_getch();

			break;
		case OUT_MONEY:
			AcHandler.OutFunc();
			_getch();

			break;
		case ALL_SHOW:
			AcHandler.ShowFunc();
			_getch();

			break;
		case EXIT:
			return 0;

		default:
			cout << "���� ����Դϴ�." << endl;
			_getch();

			break;
		}

	}

	return 0;
}