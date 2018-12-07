#include "pch.h"
#include "Convertor_Cal.h"
#include "windows.h"

// ���� ǥ����� ����  ǥ������� ��ȯ�ϴ� �Լ�
void Convertor_Cal::ConvToRPNExp(char exp[])
{
	// ��ȭ�� ���������ڸ� ������ �迭
	int expLen = (int)strlen(exp);
	char* pResult = new char[expLen + 1];
	ZeroMemory(pResult, expLen+1);

	// �߰��� ��� ��Ȱ ����
	listStack stack;
	stack.Init();

	// exp�ȿ� �ִ� ���ڸ� �ϳ��ϳ� ���鼭 �۾� ����
	int TempIndex = 0;
	char cTok;
	int i = 0;

	while (i < expLen)
	{
		// ���� �ϳ� ��󳻱�.
		cTok = exp[i];

		// �̹� ���ڰ� �������� Ȯ��
		// ���ڶ�� true ����
		if (isdigit(cTok))
		{
			// ���ڶ�� �ٷ� �迭�� �ְ� ��
			pResult[TempIndex] = cTok;
			TempIndex++;
		}
		
		// ���ڰ� �ƴϸ� ������.
		// �������� ��� ����
		else
		{
			switch (cTok)
			{
				// �����ڰ� ( ��� �Ұ�ȣ ����.
			case '(':
			{
				// ���ÿ� �Ұ�ȣ �����ڸ� �־�д�.
				stack.Push(cTok);
			}
			break;

			// �����ڰ� )��� �Ұ�ȣ ��.
			case ')':
			{
				char PopData;

				// ���ÿ� �ִ� ��� �����ڸ� ������ ��� �迭�� �ִ´�.
				while (1)
				{
					stack.Pop(&PopData);

					// �ٽ� ���� �����ڸ� ���� �� ����
					if (PopData == '(')
						break;

					pResult[TempIndex] = PopData;
					TempIndex++;
				}
			}
			break;

			// �����ڰ� +, -, *, /���
			case '+':
			case '-':
			case '*':
			case '/':
			{
				// ���ÿ� �����ڰ� ���� ���, ���� ���� �����ڿ� �̹� �����ڸ� ��.		
				char op1;

				if (stack.Peek(&op1) == true)
				{
					// ���� ���� �������� �켱������ �� ���ų� ���� ���, ���ÿ��� ���� �����ڸ� �� ��, ��� �迭�� �ִ´�.
					while (stack.IsEmpty() == false && WhoPrecOp(op1, cTok) >= 0)
					{
						stack.Pop(&op1);
						pResult[TempIndex] = op1;
						TempIndex++;

						stack.Peek(&op1);
					}
				}

				// �� �Ϸ� ��, ���� �����ڸ� ���ÿ� �ִ´�.
				stack.Push(cTok);
				break;
			}

			}			
		}

		i++;
	}

	// ���ÿ� �����ִ� ������ ��θ� ����迭�� �̵�
	while (stack.Pop(&cTok))
	{
		pResult[TempIndex] = cTok;
		TempIndex++;
	}

	// ��ȯ�� ���������� ������, �Ű������� �ִ´�.
	strcpy_s(exp, expLen+1, pResult);

	// ���� ����迭�� �޸� ����
	delete[] pResult;
}

// �� �������� �켱���� ��
int Convertor_Cal::WhoPrecOp(char op1, char op2)
{
	// op1�� op2�� �켱���� �˾ƿ���
	int op1Prec = GetOpPrec(op1);
	int op2Prec = GetOpPrec(op2);

	// op1�� �� �켱������ ���ٸ� 1
	if (op1Prec > op2Prec)
		return 1;

	// op2�� �� �켱������ ���ٸ� -1
	else if (op1Prec < op2Prec)
		return -1;

	// �켱������ ���ٸ� 0
	else
		return 0;
}

// ���ڷ� ���� �������� �켱������ ���� ���·� ��ȯ
int Convertor_Cal::GetOpPrec(char op)
{	
	switch (op)
	{
		// *�� /�� �켱���� 5
	case '*':
	case '/':
		return 5;

		// +�� -�� �켱���� 3
	case '+':
	case '-':
		return 3;

		// (�� 1. ���� ����
	case '(':
		return 1;
	}

	// �� �ܿ� ���� �����ڴ� -1
	return -1;
}

// ���ڷ� ���� ����ǥ����� ����� ����� ��ȯ�ϴ� �Լ�
int Convertor_Cal::EvalTPNExp(char exp[])
{
	listStack stack;
	stack.Init();

	// exp���� �ϳ��� ���� ����Ѵ�.
	int expLen = (int)strlen(exp);
	char Tok;
	
	int i = 0;
	while (i < expLen)
	{
		// �ϳ� ������
		Tok = exp[i];

		// �� �� ���ڰ� �������� üũ
		if (isdigit(Tok))
		{
			// ���ڶ�� ���ÿ� �ְ� ��.
			// '-'�� �����ν�, �ƽ�Ű���ڰ� �ƴ϶� ���� ������ ���� �����Ѵ�.
			stack.Push(Tok - '0');
		}

		// ���ڰ� �ƴ϶� �����ڶ��
		else
		{
			// ���ÿ��� �ǿ����� 2�� ���´�.
			// ���� ������ �ǿ����ڰ� op2�� �ȴ�.
			// op1�̶� op2�� ������ ������ ��ġ�ȴ�. (ex. op1 / op2)
			// �� ������, ������, ���� ���� ������ �߿��ϱ� �����̴�.
			char op1, op2;

			stack.Pop(&op2);
			stack.Pop(&op1);

			// ��� �� ������� ���ÿ� Push.
			// �ٽ� �ǿ����ڰ� �ȴ�
			stack.Push(Calculation(Tok, op1, op2));
		}

		i++;
	}

	// ���ÿ��� ����� �����ְ� �ȴ�.
	char Result;
	stack.Pop(&Result);

	return Result;
}

// ���ڷ� ���� �����ڸ� �̿���, ���ڷ� ���� 2���� �����ڸ� �����Ѵ�.
// ���� �� ����� �����Ѵ�. (char ������)
char Convertor_Cal::Calculation(char opE, char op1, char op2)
{
	switch (opE)
	{
	case '+':
		return op1 + op2;

	case '-':
		return op1 - op2;

	case '*':
		return op1 * op2;

	case '/':
		return op1 / op2;
	}	
}