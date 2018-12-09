#ifndef __CONVERTOR_CAL_H__
#define __CONVERTOR_CAL_H__

#include "ListStack.h"

class Convertor_Cal
{

private:
	// �� �������� �켱���� ��
	int WhoPrecOp(char op1, char op2);

	// ���ڷ� ���� �������� �켱������ ���� ���·� ��ȯ
	int GetOpPrec(char op);

	// ���ڷ� ���� �����ڸ� �̿���, ���ڷ� ���� 2���� �����ڸ� �����Ѵ�.
	// ���� �� ����� �����Ѵ�. (int ������)
	char Calculation(char opE, char op1, char op2);


public:
	// ���� ǥ����� ����  ǥ������� ��ȯ�ϴ� �Լ�
	void ConvToRPNExp(char exp[]);

	// ���ڷ� ���� ����ǥ����� ����� ����� ��ȯ�ϴ� �Լ�
	int EvalTPNExp(char exp[]);

};

#endif // !__CONVERTOR_CAL_H__
