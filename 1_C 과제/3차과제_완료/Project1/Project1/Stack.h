#pragma once
#ifndef __STACK_H__
#define __STACK_H__

typedef int Data;

#define STACK_LEN 330

typedef struct
{
	Data m_xpos[STACK_LEN];
	Data m_ypos[STACK_LEN];
	int topIndex;

}Stack;

void StackInit(Stack* pstack)
{
	pstack->topIndex =  - 1;
}

bool Push(Stack* pstack, Data xpos, Data ypos)
{
	// ����, ���� ���� +1������ max��� false ��ȯ.
	if (pstack->topIndex + 1 >= STACK_LEN)
		return false;

	// ���� 1 ����
	pstack->topIndex += 1;

	// x,y�� ����.
	pstack->m_xpos[pstack->topIndex] = xpos;
	pstack->m_ypos[pstack->topIndex] = ypos;

	return true;
}

bool Pop(Stack* pstack, Data* xpos, Data* ypos)
{
	// ����, ������ 0���� ���� ���¶��(��, -1�̶��. -1�� �ʱ����) �� �̻� �E �����Ͱ� ���� ������ false��ȯ
	if (pstack->topIndex < 0)
		return false;

	// x,y��ǥ�� ���� �Է�
	*xpos = pstack->m_xpos[pstack->topIndex];
	*ypos = pstack->m_ypos[pstack->topIndex];

	// ����1 ����
	pstack->topIndex -= 1;

	return true;
}


#endif // !__STACK_H__

