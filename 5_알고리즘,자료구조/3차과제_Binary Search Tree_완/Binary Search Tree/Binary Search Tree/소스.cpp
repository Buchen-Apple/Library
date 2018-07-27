#include <stdio.h>
#include "Binary_Search_Tree.h"

int main()
{
	/*CBST cBinaryTree(10);
	cBinaryTree.Insert(9);
	cBinaryTree.Insert(11);

	cBinaryTree.Delete(10);

	printf("��Ʈ��� : %d\n", cBinaryTree.GetRootNodeData());
	cBinaryTree.Traversal();*/


	CBST cBinaryTree(10);
	printf("��Ʈ��� : %d\n", cBinaryTree.GetRootNodeData());

	cBinaryTree.Insert(14);
	cBinaryTree.Insert(12);
	cBinaryTree.Insert(11);
	cBinaryTree.Insert(13);
	
	cBinaryTree.Insert(8);
	cBinaryTree.Insert(9);
	cBinaryTree.Insert(5);
	cBinaryTree.Insert(6);
	cBinaryTree.Insert(7);

	cBinaryTree.Traversal();
	fputc('\n', stdout);

	// ��Ʈ���(10) ����
	printf("\n%d ����!\n", cBinaryTree.GetRootNodeData());
	cBinaryTree.Delete(cBinaryTree.GetRootNodeData());

	printf("��Ʈ��� : %d\n", cBinaryTree.GetRootNodeData());
	cBinaryTree.Traversal();
	fputc('\n', stdout);	

	// 7, 8 ����
	printf("\n%d,%d ����!\n", 7, 8);
	cBinaryTree.Delete(7);
	cBinaryTree.Delete(8);

	printf("��Ʈ��� : %d\n", cBinaryTree.GetRootNodeData());
	cBinaryTree.Traversal();
	fputc('\n', stdout);






	//// 12, 13 ����
	//printf("\n%d, %d ����!\n", 12, 13);
	//cBinaryTree.Delete(13);
	//cBinaryTree.Delete(12);

	//printf("��Ʈ��� : %d\n", cBinaryTree.GetRootNodeData());
	//cBinaryTree.Traversal();
	//fputc('\n', stdout);

	//// ��Ʈ���(9) ����
	//printf("\n%d ����!\n", cBinaryTree.GetRootNodeData());
	//cBinaryTree.Delete(cBinaryTree.GetRootNodeData());

	//printf("��Ʈ��� : %d\n", cBinaryTree.GetRootNodeData());
	//cBinaryTree.Traversal();
	//fputc('\n', stdout);

	//// 5 ����
	//printf("\n%d ����!\n", 5);
	//cBinaryTree.Delete(5);

	//printf("��Ʈ��� : %d\n", cBinaryTree.GetRootNodeData());
	//cBinaryTree.Traversal();
	//fputc('\n', stdout);

	//// ��Ʈ���(6) ����
	//printf("\n%d ����!\n", cBinaryTree.GetRootNodeData());
	//cBinaryTree.Delete(cBinaryTree.GetRootNodeData());

	//printf("��Ʈ��� : %d\n", cBinaryTree.GetRootNodeData());
	//cBinaryTree.Traversal();
	//fputc('\n', stdout);



	return 0;
}