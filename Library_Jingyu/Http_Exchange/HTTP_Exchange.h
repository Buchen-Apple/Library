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
		// 
		// Parameter : ȣ��Ʈ(IP or ������), HostPort, Host�� ���������� ���� (false�� IP. �⺻ false)	
		HTTP_Exchange(TCHAR* Host, USHORT Server_Port, bool DomainCheck = false);

		// �Ҹ���
		~HTTP_Exchange();

		// http�� Request, Response�ϴ� �Լ�
		//
		// Parameter : Path, ������ Body, (out)Response����� ���� TCHAR�� ���� (UTF-16)
		// return : ���� �� true, ���� �� false
		bool HTTP_ReqANDRes(TCHAR* Path, TCHAR* RquestBody, TCHAR* ReturnBuff);


	private:
		// �������� IP�� ����
		//
		// Parameter : (out)��ȯ�� Ip�� ������ ����, ������
		// return : ���� �� true, ���� �� false
		bool DomainToIP(TCHAR* IP, TCHAR* Host);

		// ����� �� �������� Recv �ϴ� �Լ�
		//
		// Parameter : �������� ����� Socket, (out)���Ϲ��� char�� ����(UTF-8), char�� ������ size
		// return : ���������� ��� ���� �� true
		bool HTTP_Recv(SOCKET sock, char* ReturnBuff, int BuffSize);

		// �� ������ Connect�ϴ� �Լ�
		//
		// Parameter : 
		// return : ���� �� true, ���� �� false
		bool HTTP_Connect(SOCKET sock);

	};
}


#endif // !__HTTP_EXCHANGE_H__
