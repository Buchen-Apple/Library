#ifndef __MY_SMART_PTR_H__
#define __MY_SMART_PTR_H__

#include <Windows.h>
#include <map>


namespace Library_Jingyu
{

	// ------------------------
	// ���� ���� ����Ʈ������
	// ------------------------
	template <typename T>
	class  My_Smart_PTR
	{
	private:
		T * m_pPtr;		// ����Ű���ִ� ������
		long* m_lCount;	// ���� ����Ű�� �ִ� �������� ī����. �����Ҵ��ؼ� ����.

	public:
		// ------------- �����ڿ� �Ҹ��� ---------------
		// ������
		My_Smart_PTR(T* P);

		// ���� ������
		My_Smart_PTR(const My_Smart_PTR<T>& Copy);

		// �Ҹ���
		~My_Smart_PTR();

	public:
		// ------------- ���� ������ �����ε� ---------------
		// -> ������
		T* operator->() const;

		// * ������
		T& operator*() const;

		// ���� ������
		My_Smart_PTR<T>& operator=(const My_Smart_PTR<T>& ref);



	public:
		// ------------- ���� ---------------
		// g_My_Smart_PTR_Count �� ���
		long GetRefCount() const;


	};

	// �Ʒ��� Cpp �κ�
	//--------------------------------------------------------------------
	//--------------------------------------------------------------------
	// ******************************
	// �����ڿ� �Ҹ���
	// ******************************
	// ������
	template <typename T>
	My_Smart_PTR<T>::My_Smart_PTR(T* P)
	{
		m_pPtr = P;
		m_lCount = new long;
		*m_lCount = 1;
	}

	// ���� ������
	template <typename T>
	My_Smart_PTR<T>::My_Smart_PTR(const My_Smart_PTR<T>& Copy)
	{
		m_pPtr = Copy.m_pPtr;
		InterlockedIncrement(Copy.m_lCount);
		m_lCount = Copy.m_lCount;
		
	}

	// �Ҹ���
	template <typename T>
	My_Smart_PTR<T>::~My_Smart_PTR()
	{
		if (InterlockedDecrement(m_lCount) == 0)
		{
			delete m_pPtr;
			delete m_lCount;
		}		
	}


	// ******************************
	// ���� ������ �����ε�
	// ******************************
	// -> ������
	template <typename T>
	T* My_Smart_PTR<T>::operator->() const
	{
		return m_pPtr;
	}

	// * ������
	template <typename T>
	T& My_Smart_PTR<T>::operator*() const
	{
		return *m_pPtr;
	}

	// ���� ������
	template <typename T>
	My_Smart_PTR<T>& My_Smart_PTR<T>::operator=(const My_Smart_PTR<T>& ref)
	{
		// ������ ����Ű�� �������� ���۷��� ī��Ʈ�� 1 ���ҽ�Ű�� 0�̶�� ����
		if (InterlockedDecrement(m_lCount) == 0)
		{
			delete m_pPtr;
			delete m_lCount;
		}

		// ptr ����
		m_pPtr = ref.m_pPtr;

		// refcount ����
		InterlockedIncrement(Copy.m_lCount);
		m_lCount = Copy.m_lCount; 
		
	}



	// ******************************
	// ����
	// ******************************
	// g_My_Smart_PTR_Count �� ���
	template <typename T>
	long My_Smart_PTR<T>::GetRefCount() const
	{
		return *m_lCount;
	}


}



#endif // !__MY_SMART_PTR_H__
