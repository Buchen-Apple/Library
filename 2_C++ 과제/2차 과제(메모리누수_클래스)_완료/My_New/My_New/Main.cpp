#include <iostream>

#include "My_New.h"

using namespace std;

int main()
{
	// NOALLOC�� ���� ����
	int *test;
	char *test2;

	// �����Ҵ�	
	char *c1 = new char;
	short *s1 = new short[4];
	int *p1 = new int;
	long *l1 = new long;
	long long *dl1 = new long long[4];
	float *f1 = new float;
	double *d1 = new double[4];

	// ����Ʈ
	delete test;	// NOALLOC �߻�
	//delete c1;	// LEAK �߻� (LEAK�� ���� ��� �ּ�ó��)
	delete[] s1;
	delete[] p1;	// ARRAY �߻� (���� �Ϲ� delete�� �����ؾ� �ϳ�, ARRAY�� ���� ���� �迭 delete�� ����)
	delete test2;	// NOALLOC �߻�
	delete l1;
	delete dl1;		// ARRAY �߻� (���� �迭 delete�� �����ؾ� �ϳ�, ARRAY�� ���� ���� �Ϲ� delete�� ����)
	delete f1;
	delete[] d1;

	return 0;
}

