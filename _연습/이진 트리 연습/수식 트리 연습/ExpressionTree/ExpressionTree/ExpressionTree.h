#pragma once
#ifndef __EXPRESSION_TREE_H__
#define __EXPRESSION_TREE_H__

#include "BinaryTree.h"

// ���� Ʈ�� ����
BTreeNode* MakeExpTree(char exp[]);

// ���� Ʈ�� ���
int EvaluateExpTree(BTreeNode* bt);

// ���� ǥ��� ��� ���
void ShowPreFixTypeExp(BTreeNode* bt);

// ���� ǥ��� ��� ���
void ShowInFixTypeExp(BTreeNode* bt);

// ���� ǥ��� ��� ���
void ShowPostFixTypeExp(BTreeNode* bt);

#endif // !__EXPRESSION_TREE_H__
