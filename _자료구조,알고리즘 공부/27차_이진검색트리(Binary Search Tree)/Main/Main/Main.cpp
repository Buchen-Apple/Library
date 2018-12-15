// Main.cpp : 이 파일에는 'main' 함수가 포함됩니다. 거기서 프로그램 실행이 시작되고 종료됩니다.
//

#include "pch.h"
#include <iostream>

using namespace std;

// 노드
struct Node
{
	int Data;
	Node* m_pParent;
	Node* m_pLeft;
	Node* m_pRight;
};

// 데이터 삽입
void BST_Insert(Node** pRoot, int Data);

// 데이터가 있는 노드 검색
Node* BST_Search(Node* pRoot, int TargetData);

// 데이터가 있는 노드 삭제
bool BST_Delete(Node* pRoot, int TargetData);

// 인자로 받은 pNode의 Successor 얻기. (나보다 큰 값들 중 가장 작은값)
Node* BST_SuccessorGet(Node* pNode);

// 인자로 받은 pNode의 Predecessor 얻기 (나보다 작은 값들 중 가장 큰값)
Node* BST_PredecessorGet(Node* pNode);

int main()
{
	// 루트 생성
	Node* Root = nullptr;

	// 값 Insert
	BST_Insert(&Root, 6);
	BST_Insert(&Root, 2);
	BST_Insert(&Root, 7);
	BST_Insert(&Root, 1);
	BST_Insert(&Root, 4);
	BST_Insert(&Root, 3);
	BST_Insert(&Root, 5);

	// 해당 값 삭제
	BST_Delete(Root, 2);

	return 0;
}

// 데이터 삽입
void BST_Insert(Node** pRoot, int Data)
{
	// 새로운 노드 생성
	Node* NewNode = new Node;
	NewNode->Data = Data;
	NewNode->m_pLeft = nullptr;
	NewNode->m_pRight = nullptr;

	// 새로운 노드가 들어갈 위치 찾기

	Node* y = nullptr;	// y는 x의 부모
	Node* x = *pRoot;	// x는 현재 위치

	// x가 nullptr이 되면 들어갈 위치를 찾은것
	while (x != nullptr)
	{
		// 삽입될 데이터가 현재 노드의 데이터보다 크거나 같다면
		if (Data >= x->Data)
		{
			y = x;
			x = x->m_pRight;
		}

		// 삽입될 데이터가 현재 노드의 데이터보다 작다면
		else if (Data < x->Data)
		{
			y = x;
			x = x->m_pLeft;
		}
	}

	// 들어갈 위치를 찾은 후 로직

	// 1) y가 nullptr인 경우 (즉, Empty Tree인 경우)
	if (y == nullptr)
	{
		NewNode->m_pParent = nullptr;

		// 새로운 노드가 루트가 됨.
		*pRoot = NewNode;
	}

	// 2) 들어갈 위치가 y의 왼쪽인 경우 (값으로 체크)
	else if (Data < y->Data)
	{
		NewNode->m_pParent = y;

		// y의 Left에 새로운 노드 연결
		y->m_pLeft = NewNode;		
	}

	// 3) 들어갈 위치가 y의 오른쪽인 경우
	else if (Data >= y->Data)
	{
		NewNode->m_pParent = y;

		// y의 Right에 새로운 노드 연결
		y->m_pRight = NewNode;
	}
}

// 데이터가 있는 노드 검색
Node* BST_Search(Node* pRoot, int TargetData)
{
	Node* RetNode = pRoot;

	// 타겟과 RetNode를 비교하며 노드 검색
	while (RetNode->Data != TargetData)
	{
		// TargetData가 현재 노드의 데이터보다 크거나 같다면
		if (RetNode->Data <= TargetData)
		{
			// RetNode를 오른쪽으로 이동해야 함.
			// 근데 오른쪽이 nullptr이면 끝에 도착한 것. 
			// 타겟이 없는것.
			if (RetNode->m_pRight == nullptr)
				return nullptr;

			// 있으면 다음으로 이동
			RetNode = RetNode->m_pRight;
		}

		// TargetData가 현재 노드의 데이터보다 작다면
		else
		{
			// RetNode를 왼쪽으로 이동해야 함.
			// 근데 왼쪽이 nullptr이면 끝에 도착한 것.
			// 타겟이 없음
			if (RetNode->m_pLeft == nullptr)
				return nullptr;

			// 있으면 다음으로 이동
			RetNode = RetNode->m_pLeft;
		}
	}

	// 여기까지오면 타겟 찾은것
	return RetNode;
}

// 데이터가 있는 노드 삭제
bool BST_Delete(Node* pRoot, int TargetData)
{
	// 삭제하고자 하는 노드 검색
	Node* DeleteNode = BST_Search(pRoot, TargetData);
	if (DeleteNode == nullptr)
		return false;

	while (1)
	{
		// Case 1. 삭제 노드의 자식이 없는 경우
		if (DeleteNode->m_pLeft == nullptr && DeleteNode->m_pRight == nullptr)
		{
			Node* pParent = DeleteNode->m_pParent;

			// 바로 트리에서 제외
			// 근데 만약, 삭제하고자 하는 노드가 루트라면 부모가 nullptr임.
			// 이 때는 pRoot를 nullptr로 만들고 나간다.
			if (DeleteNode->m_pParent == nullptr)
				pRoot = nullptr;

			// 내가 부모의 왼쪽 자식일 경우 부모의 왼쪽 포인터를 nullptr로.
			else if (pParent->m_pLeft == DeleteNode)
				pParent->m_pLeft = nullptr;

			// 내가 부모의 오른쪽 자식일 경우 부모의 오른쪽 포인터를 nullptr로.
			else if (pParent->m_pRight == DeleteNode)
				pParent->m_pRight = nullptr;

			// 반복문 탈출	
			break;
		}

		// Case 3. 삭제 노드의 자식이 2명인 경우
		// 핵심 : Case 1 or Case 2로 만든다.
		if (DeleteNode->m_pLeft != nullptr && DeleteNode->m_pRight != nullptr)
		{
			// 삭제 노드의 Successor 혹은 Predecessor를 구한다.
			// 여기서 Successor나 Predecessor가 없을리는 없다. 왜냐하면 이미 if문에서 왼쪽과 오른쪽이 둘 다 있다고 확인했기 때문에.
			// 여기서는 Successor를 구한다.		
			Node* Suc = BST_SuccessorGet(DeleteNode);

			// Successor의 값을 DeleteNode에 복사
			DeleteNode->Data = Suc->Data;

			// Successor가 DeleteNode가 된다.
			// 이제 상황은, Case 1 or Case 2가 된다.
			DeleteNode = Suc;

			continue;
		}

		// Case 2. 삭제 노드의 자식이 1명인 경우 (왼쪽 혹은 오른쪽 자식이 존재할 경우. 자식 1명)
		else if (DeleteNode->m_pLeft != nullptr || DeleteNode->m_pRight != nullptr)
		{
			Node* pParent = DeleteNode->m_pParent;

			// 내가 부모의 왼쪽 자식일 경우
			if (pParent->m_pLeft == DeleteNode)
			{
				// 왼쪽 자식이 존재한다면, 부모의 왼쪽이 나의 왼쪽 자식을 가리킴
				if (DeleteNode->m_pLeft != nullptr)
					pParent->m_pLeft = DeleteNode->m_pLeft;

				// 오른쪽 자식이 존재한다면, 부모의 왼쪽이 나의 오른쪽 자식을 가리킴
				else
					pParent->m_pLeft = DeleteNode->m_pRight;				
			}

			// 내가 부모의 오른쪽 자식일 경우
			else
			{
				// 왼쪽 자식이 존재한다면, 부모의 오른쪽이 나의 왼쪽 자식을 가리킴
				if (DeleteNode->m_pLeft != nullptr)
					pParent->m_pRight = DeleteNode->m_pLeft;

				// 오른쪽 자식이 존재한다면, 부모의 오른쪽이 나의 오른쪽 자식을 가리킴
				else
					pParent->m_pRight = DeleteNode->m_pRight;
			}

			// 반복문 탈출	
			break;
		}
	}

	// 데이터 동적해제 및 리턴 true
	delete DeleteNode;
	return true;
}

// 인자로 받은 pNode의 Successor 얻기. (나보다 큰 값들 중 가장 작은값)
Node* BST_SuccessorGet(Node* pNode)
{
	// 1. pNode의 오른쪽이 존재하는지 체크
	if (pNode->m_pRight != nullptr)
	{
		Node* pRight = pNode->m_pRight;

		// 오른쪽 서브트리의, 최소값을 찾아간다
		while (pRight->m_pLeft != nullptr)
			pRight = pRight->m_pLeft;

		// 찾은 최소값을 리턴
		return pRight;
	}

	// 2. 오른쪽이 없다면, 부모를 타고 올라가다가, 내가 부모의 왼쪽자식이 되는 순간을 찾는다.
	// 그 순간의 부모가 나의 Successor가 된다.
	else if (pNode->m_pParent != nullptr)
	{
		Node* pParent = pNode->m_pParent;
		Node* pTempNode = pNode;

		// 부모가 nullptr이 아니라면 로직 진행.
		// 여기서 리턴 안하고 while문을 빠져나간다면 루트까지 갔다는 것
		while (pParent != nullptr)
		{
			// 내가 부모의 왼쪽 자식이 됐다면 리턴한다.
			if (pParent->m_pLeft == pTempNode)
				return pParent;

			pTempNode = pParent;
			pParent = pParent->m_pParent;
		}
	}

	// 3. 여기까지오면 Successor가 없는것.
	return nullptr;
}

// 인자로 받은 pNode의 Predecessor 얻기 (나보다 작은 값들 중 가장 큰값)
Node* BST_PredecessorGet(Node* pNode)
{
	// 1. pNode의 왼쪽이 존재하는지 체크
	if (pNode->m_pLeft != nullptr)
	{
		Node* pLeft = pNode->m_pLeft;

		// 왼쪽 서브트리의, 최대값을 찾아간다
		while (pLeft->m_pRight != nullptr)
			pLeft = pLeft->m_pRight;

		// 찾은 최대값을 리턴
		return pLeft;
	}

	// 2. 왼쪽이 없다면, 부모를 타고 올라가다가, 내가 부모의 오른쪽자식이 되는 순간을 찾는다.
	// 그 순간의 부모가 나의 Predecessor가 된다.
	else if (pNode->m_pParent != nullptr)
	{
		Node* pParent = pNode->m_pParent;
		Node* pTempNode = pNode;

		// 부모가 nullptr이 아니라면 로직 진행.
		// 여기서 리턴 안하고 while문을 빠져나간다면 루트까지 갔다는 것
		while (pParent != nullptr)
		{
			// 내가 부모의 오른쪽 자식이 됐다면 리턴한다.
			if (pParent->m_pRight == pTempNode)
				return pParent;

			pTempNode = pParent;
			pParent = pParent->m_pParent;
		}
	}

	// 3. 여기까지오면 Predecessor가 없는것.
	return nullptr;
}