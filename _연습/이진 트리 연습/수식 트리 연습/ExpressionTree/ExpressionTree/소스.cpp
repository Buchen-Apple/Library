#include <stdio.h>
#include "ExpressionTree.h"

int main()
{
	char exp[] = "12+7*";
	BTreeNode* eTree = MakeExpTree(exp);

	printf("���� ǥ����� ���� : ");
	ShowPreFixTypeExp(eTree);
	printf("\n");


	printf("���� ǥ����� ���� : ");
	ShowInFixTypeExp(eTree);
	printf("\n");


	printf("���� ǥ����� ���� : ");
	ShowPostFixTypeExp(eTree);
	printf("\n");

	printf("���� ��� : %d\n", EvaluateExpTree(eTree));


	return 0;
}