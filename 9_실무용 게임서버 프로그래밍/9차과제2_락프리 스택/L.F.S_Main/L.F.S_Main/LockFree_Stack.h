#ifndef __LOCKFREE_STACK_H__
#define __LOCKFREE_STACK_H__

#include <Windows.h>
#include "CrashDump\CrashDump.h"

namespace Library_Jingyu
{
	template <typename T>
	class CLF_Stack
	{
	private:
		struct st_LFS_NODE
		{
			T m_Data;
			st_LFS_NODE* m_stpNextBlock;
		};

		// ������ Last-In-First_Out
		st_LFS_NODE* m_stpTop = nullptr;

		// �������� �� ũ���� ���� �뵵
		CCrashDump* m_CDump = CCrashDump::GetInstance();

	public:
		// ������

		// �Ҹ���
		~CLF_Stack()
		{
			// null�� ���ö����� ���鼭 ����.
			while (1)
			{

			}


		}


		// �ε����� ���ÿ� �߰�
		//
		// return : ���� (void)
		void Push(T Data)
		{
			st_LFS_NODE* NewNode = new st_LFS_NODE;
			NewNode.m_Data = Data;
			NewNode.m_stpNextBlock = m_stpTop;

			m_stpTop = NewNode;
		}


		// �ε��� ���
		//
		// ���� ��, T ����
		// ���н� Crash �߻�
		T Pop()
		{
			// �� �̻� pop �Ұ� ������ Crash �߻�
			if (m_stpTop == nullptr)
				m_CDump->Crash();

			T retval = m_stpTop.m_Data;
			st_LFS_NODE* TempNode = m_stpTop;

			m_stpTop = m_stpTop.m_stpNextBlock;

			delete TempNode;

			return retval;
		}


		// �ε��� ���
		//
		// ��ȯ�� ULONGLONG ---> �� ��ȯ������ ����Ʈ ���� �� ���� �־, Ȥ�� �𸣴� unsigned�� �����Ѵ�.
		// 0�̻�(0����) : �ε��� ���� ��ȯ
		// 10000000(õ��) : �� �ε��� ����.
		ULONGLONG Pop()
		{
			// �ε��� ����� üũ ----------
			// ���� Max�� ������ ������ ��� ������ ���, �� �ε����� ����.
			if (m_iTop == 0)
				return 10000000;

			// �� �ε����� ������, m_iTop�� ���� �� �ε��� ����.
			m_iTop--;

			return m_iArray[m_iTop];
		}

		// �ε��� �ֱ�
		//
		// ��ȯ�� bool
		// true : �ε��� �������� ��
		// false : �ε��� ���� ���� (�̹� Max��ŭ �� ��)
		bool Push(ULONGLONG PushIndex)
		{
			// �ε����� ��á�� üũ ---------
			// �ε����� ��á���� false ����
			if (m_iTop == m_iMax)
				return false;

			// �ε����� ������ �ʾ�����, �߰�
			m_iArray[m_iTop] = PushIndex;
			m_iTop++;

			return true;
		}

		CLF_Stack(int Max)
		{
			m_iTop = Max;
			m_iMax = Max;

			m_iArray = new ULONGLONG[Max];

			// ���� ���� ��, ��� �� �ε���
			for (int i = 0; i < Max; ++i)
				m_iArray[i] = i;
		}

		~CLF_Stack()
		{
			delete[] m_iArray;
		}

	};
}


#endif // !__LOCKFREE_STACK_H__
