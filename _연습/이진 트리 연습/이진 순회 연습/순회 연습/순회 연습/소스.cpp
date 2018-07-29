#include <stdio.h>
#include "BinaryTree.h"

// ��ȸ �� �ϴ� �ൿ �Լ�
void ShowIntData(int data);

int main()
{
	// �׽�Ʈ ��� 4�� ����
	BTreeNode* Tree1 = MakeBTreeNode();
	BTreeNode* Tree2 = MakeBTreeNode();
	BTreeNode* Tree3 = MakeBTreeNode();
	BTreeNode* Tree4 = MakeBTreeNode();

	// �׽�Ʈ ��忡 ������ ����
	SetData(Tree1, 1);
	SetData(Tree2, 2);
	SetData(Tree3, 3);
	SetData(Tree4, 4);

	// �����ϱ�
	MakeLeftSubTree(Tree1, Tree2);
	MakeRightSubTree(Tree1, Tree3);
	MakeLeftSubTree(Tree2, Tree4);	

	// ���� ��ȸ �ϱ�
	SecondTraverse(Tree1, ShowIntData);
	fputc('\n', stdout);

	// ���ο� ��� ���� Tree2�� ���ʿ����̱�
	BTreeNode* Tree5 = MakeBTreeNode();
	SetData(Tree5, 10);

	// ������ Tree4(���� 4 �������)�� �� �����Ǵ��� Ȯ��.
	MakeLeftSubTree(GetLeftSubTree(Tree1), Tree5);
	SecondTraverse(Tree1, ShowIntData);
	fputc('\n', stdout);

	return 0;
}

// ��ȸ �� �ϴ� �ൿ �Լ�
void ShowIntData(int data)
{
	printf("%d ", data);
}