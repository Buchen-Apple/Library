#pragma once
#ifndef __CLIST_TEMPLATE_H__
#define __CLIST_TEMPLATE_H__

using namespace std;

template <typename T>
class CList
{
public:
	struct Node
	{
		T _data;
		Node* _Prev;
		Node* _Next;
	};

	class iterator
	{
	private:
		Node * _node;

	public:
		// iterator: ������
		iterator(Node *node = nullptr)
		{
			//���ڷ� ���� Node �����͸� ����
			_node = node;
		}

		// iterator: ���� ������
		iterator(const iterator& ref)
		{
			_node = ref._node;
		}

		// iterator: ������� üũ
		bool Empty(iterator itor)
		{
			if (itor._node == NULL)
				return true;

			return false;
		}

		// iterator: == �� ������
		bool operator==(const iterator& ref)
		{
			if (_node == ref._node)
				return true;

			return false;
		}

		// iterator: != �� ������
		bool operator!=(const iterator& ref)
		{
			if (_node->_data != ref._node->_data)
				return true;

			return false;
		}

		// iterator: ���� ������
		iterator& operator = (const iterator& ref)
		{
			_node = ref._node;
			return *this;
		}

		//iterator: ���� ��带 ���� ���( >>> )�� �̵�. ���� ����
		iterator& operator ++(int)
		{
			_node = _node->_Next;
			return *this;
		}

		// iterator: ���� ��带 ���� ���( <<< )�� �̵�. ���� ����
		iterator& operator ++()
		{
			_node = _node->_Prev;
			return *this;
		}

		// iterator: ������ ����
		T& operator *()
		{
			//���� ����� �����͸� ����
			return _node->_data;
		}

		// iterator: ���� ��� ����. ������ ��� ����, �� �� ��尡 ���� �����.
		void NodeDelete()
		{
			_node->_Prev->_Next = _node->_Next;
			_node->_Next->_Prev = _node->_Prev;

		}

		// iterator: ���� ���ͷ������� ���� Next�� ��ġ�� ���� �ٲ۴�.
		void ListSwap(iterator Next)
		{
			T temp = _node->_data;
			_node->_data = Next._node->_data;
			Next._node->_data = temp;
		}


		friend iterator CList<T>::erase(iterator&) throw(int);
	};

private:
	int _size = 0;
	Node* _head;	// ���� ��带 ����Ų��.
	Node* _tail;	// ���� ��带 ����Ų��.

public:
	// CList: ������
	CList()
	{
		_head = new Node;
		_tail = new Node;

		_head->_Next = _tail;
		_head->_Prev = NULL;

		_tail->_Prev = _head;
		_tail->_Next = NULL;
	}

	// CList: ���� ������
	CList(const CList& ref)
	{
		_head = ref._head;
		_tail = ref._tail;
		_size = ref._size;
	}

	// CList: ���� ������
	CList& operator = (const CList& ref)
	{
		_head = ref._head;
		_tail = ref._tail;
		_size = ref._size;
		return *this;
	}

	// CList: ù��° ��带 ����Ű�� ���ͷ����� ����
	iterator begin() const
	{
		iterator itor;
		itor = _head->_Next;
		return itor;
	}

	// CList:  Tail ��带 ����Ű��(�����Ͱ� ���� ��¥ �� ���) ���ͷ����͸� ����
	// �Ǵ� ������ ������ �� �ִ� ���ͷ����͸� ����
	iterator end() const
	{
		iterator itor;
		itor = _tail;
		return itor;
	}

	// CList: ���ο� ��� �߰� (���)
	void push_front(T data)
	{
		Node* newNode = new Node;
		newNode->_data = data;

		if (_size == 0)
		{
			newNode->_Next = _tail;
			newNode->_Prev = _head;

			_head->_Next = newNode;
			_tail->_Prev = newNode;

			_size++;
			return;
		}

		_size++;
		newNode->_Next = _head->_Next;
		newNode->_Prev = _head;

		_head->_Next->_Prev = newNode;
		_head->_Next = newNode;
	}

	// CList: ���ο� ��� �߰� (����)
	void push_back(T data)
	{
		Node* newNode = new Node;
		newNode->_data = data;

		if (_size == 0)
		{
			newNode->_Next = _tail;
			newNode->_Prev = _head;

			_head->_Next = newNode;
			_tail->_Prev = newNode;

			_size++;
			return;
		}

		_size++;
		newNode->_Next = _tail;
		newNode->_Prev = _tail->_Prev;

		_tail->_Prev->_Next = newNode;
		_tail->_Prev = newNode;
	}

	// CList: ����Ʈ Ŭ����
	void clear()
	{
		_head->_Next = _tail;	// ��� ���̰� ������ ����Ŵ
		_tail->_Prev = _head;	// ���� ���̰� ��带 ����Ŵ. ��, ���� ���̳��� ����Ű�� �ؼ� �ʱ���·� ����.
		_size = 0;
	}

	// CList: ��� ���� ����
	int size() const { return _size; };

	// CList: ���� ��尡 ������� üũ
	bool is_empty(iterator itor)
	{
		bool bTemp = itor.Empty(itor);
		return bTemp;
	}

	// CList: ���ͷ������� �� ��带 ����.
	// �׸��� ���� ����� ���� ��带 ī��Ű�� ���ͷ����� ����
	iterator erase(iterator& itor)
	{
		if (itor == ++(CList::begin()) || itor == CList::end())
			throw 1;

		iterator deleteiotr = itor;
		itor++;
		deleteiotr.NodeDelete();
		_size--;

		delete deleteiotr._node;
		return itor;
	}

	// CList: �Ҹ���
	~CList()
	{}
};

#endif // !__CLIST_TEMPLATE_H__

