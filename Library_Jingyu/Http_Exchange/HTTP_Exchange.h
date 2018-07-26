#ifndef __HTTP_EXCHANGE_H__
#define __HTTP_EXCHANGE_H__


#include <windows.h>

namespace Library_Jingyu
{
	// �����ڿ��� host / port�� �Է¹���.
	// ���ĺ��ʹ� Path�� �ٲ㰡�鼭 ȣ��.
	class HTTP_Exchange
	{
	private:

		USHORT m_usServer_Port;
		TCHAR m_tHost[100];
		TCHAR m_tIP[50];


	public:
		// ������
		// ���� ���� ��, ���� ��Ʈ�� ���޹���.
		// [ȣ��Ʈ(IP�� ������ �� �� �ƹ��ų�), ���� ��Ʈ, Host�� ���������� ����] �� 3���� �Է¹���
		// true�� ������. false�� IP]
		HTTP_Exchange(TCHAR* Host, USHORT Server_Port, bool DomainCheck = false);

		// Request / Response���� ���ִ� �Լ�.
		// [������ URL, ������ Body, (out)UTF-16�� ���Ϲ��� TCHAR�� ����, URL�� ���������� ����. true�� ������. false�� IP]
		// �� 4���� �Է¹���
		bool HTTP_ReqANDRes(TCHAR* Path, TCHAR* RquestBody, TCHAR* ReturnBuff);


	private:
		// HTTP_ReqANDRes() �Լ� �ȿ��� ȣ��
		// Host���� �������� �������.
		// Host�������� IP�� �����켭 "IP"�� ����.
		bool DomainToIP(TCHAR* IP, TCHAR* Host);

		// HTTP_ReqANDRes()�Լ� �ȿ��� ȣ��.
		// ����� �������� ������ �����͸� ��� ����
		// UTF-8�� ���·� Return Buff�� �־���
		// 
		// ���޹޴� ���ڰ�
		// [�������� ����� Socket, (out)UTF-8�� ���Ϲ��� char�� ����]
		// 
		// ���ϰ�
		// ture : ���������� ��� ���� / false : ���� ������ ���ܼ� �� ������.
		bool HTTP_Recv(SOCKET sock, char* ReturnBuff, int BuffSize);

	};
}


#endif // !__HTTP_EXCHANGE_H__
