#include "stdafx.h"

#pragma comment(lib,"ws2_32")
#include <Ws2tcpip.h>

#include "HTTP_Exchange.h"


namespace Library_Jingyu
{
	// ������
	// ���� ���� ��, ���� ��Ʈ�� ���޹���.
	// [ȣ��Ʈ(IP�� ������ �� �� �ƹ��ų�), ���� ��Ʈ, Host�� ���������� ����] �� 3���� �Է¹���
	// true�� ������. false�� IP
	HTTP_Exchange::HTTP_Exchange(TCHAR* Host, USHORT Server_Port, bool DomainCheck)
	{
		// ---------------------------
		// ��Ʈ ����
		// ---------------------------
		m_usServer_Port = Server_Port;

		// ---------------------------
		// Host ����
		// ---------------------------
		_tcscpy_s(m_tHost, _Mycountof(m_tHost), Host);


		// ---------------------------
		// IP ����.
		// host�� �������̶��, �������� IP�� ����
		// ---------------------------

		if (DomainCheck == true)
			DomainToIP(m_tIP, Host);

		// �װ� �ƴ϶��, Host�� IP�ּҰ� �ԷµǾ� �ִ� ���̴�, IP�� Host�� �����Ѵ�.
		else
			_tcscpy_s(m_tIP, _Mycountof(m_tIP), Host);

	}

	// Request / Response���� ���ִ� �Լ�.
	// [Path, ������ Body, (out)UTF-16�� ���Ϲ��� TCHAR�� ����]
	// �� 3���� �Է¹���
	bool HTTP_Exchange::HTTP_ReqANDRes(TCHAR* Path, TCHAR* RquestBody, TCHAR* ReturnBuff)
	{
		// -----------------
		// HTTP ��� + body �����
		// -----------------
		/*
		POST http://127.0.0.1/auth_reg.php/ HTTP/1.1\r\n
		User-Agent: XXXXXXX\r\n
		Host: 127.0.0.1\r\n
		Connection: Close\r\n
		Content-Length: 84\r\n
		\r\n
		*/

		// Type. ����� ù ��. Path�� ������, Path�� ����� ���� ����.
		TCHAR Type[100];
		if (_tcslen(Path))
			swprintf_s(Type, _Mycountof(Type), L"POST http://%s/%s HTTP/1.1\r\n", m_tHost, Path);

		else
			swprintf_s(Type, _Mycountof(Type), L"POST http://%s/ HTTP/1.1\r\n", m_tHost);

		TCHAR User_Agent[50] = L"User-Agent: CPP!!\r\n";

		TCHAR HeaderHost[50];
		swprintf_s(HeaderHost, _Mycountof(HeaderHost), L"Host: %s\r\n", m_tHost);

		TCHAR Connection[50] = L"Connection: Close\r\n";

		TCHAR Content_Length[50];
		int Length = (int)_tcslen(RquestBody); // << ������ null ���� ���� ���ڿ� ���� ����.
		swprintf_s(Content_Length, _Mycountof(Content_Length), L"Content-Length: %d\r\n\r\n", Length);

		TCHAR HTTP_Data[1000];
		swprintf_s(HTTP_Data, _Mycountof(HTTP_Data), L"%s%s%s%s%s%s", Type, User_Agent, HeaderHost, Connection, Content_Length, RquestBody);
		




		// -----------------
		// ���� �����͸� UTF-8�� ��ȯ
		// -----------------
		char utf8_HTTP_Data[2000] = { 0, };

		// �Ʒ� �ڵ�� "_tcslen(HTTP_Data)"�� ���� ������ ������.
		// int len = WideCharToMultiByte(CP_UTF8, 0, HTTP_Data, (int)_tcslen(HTTP_Data), NULL, 0, NULL, NULL);
		int len = (int)_tcslen(HTTP_Data);

		WideCharToMultiByte(CP_UTF8, 0, HTTP_Data, (int)_tcslen(HTTP_Data), utf8_HTTP_Data, len, NULL, NULL);




		// -------------
		// TCP ���� ����
		// -------------
		WSADATA wsa;
		if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		{
			fputs("���� �ʱ�ȭ �� ����\n", stdout);
			return false;
		}

		SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);



		// -------------
		// Time_Wait�� ������ �ʱ� ���� Linger�ɼ� ����.
		// -------------
		LINGER optval;
		optval.l_onoff = 1;
		optval.l_linger = 0;
		setsockopt(sock, SOL_SOCKET, SO_LINGER, (char*)&optval, sizeof(optval));




		// -----------------
		// �� ������ connect (������)
		// -----------------
		// ������ �������� ����
		ULONG on = 1;
		DWORD check = ioctlsocket(sock, FIONBIO, &on);
		if (check == SOCKET_ERROR)
		{
			printf("NoneBlock Change Fail...\n");
			return 0;
		}

		SOCKADDR_IN clientaddr;
		ZeroMemory(&clientaddr, sizeof(clientaddr));
		clientaddr.sin_family = AF_INET;
		clientaddr.sin_port = htons(m_usServer_Port);
		InetPton(AF_INET, m_tIP, &clientaddr.sin_addr.S_un.S_addr);

		// connect �õ�
		while (1)
		{
			connect(sock, (SOCKADDR*)&clientaddr, sizeof(clientaddr));

			DWORD Check = WSAGetLastError();

			// �������� �� ���
			if (Check == WSAEWOULDBLOCK)
			{
				// �����, ���ܼ�, ����
				FD_SET wset;
				FD_SET exset;
				wset.fd_count = 0;
				exset.fd_count = 0;

				wset.fd_array[wset.fd_count] = sock;
				wset.fd_count++;

				exset.fd_array[exset.fd_count] = sock;
				exset.fd_count++;

				// timeval ����. 500m/s  ���
				TIMEVAL tval;
				tval.tv_sec = 0;
				tval.tv_usec = 500000;

				// Select()
				DWORD retval = select(0, 0, &wset, &exset, &tval);

				

				// ���� �߻����� üũ
				if (retval == SOCKET_ERROR)
				{
					printf("Select error..(%d)\n", WSAGetLastError());

					LINGER optval;
					optval.l_onoff = 1;
					optval.l_linger = 0;
					setsockopt(sock, SOL_SOCKET, SO_LINGER, (char*)&optval, sizeof(optval));

					closesocket(sock);
					WSACleanup();
					return false;

				}

				// Ÿ�Ӿƿ� üũ
				else if (retval == 0)
				{
					printf("Select Timeout..\n");
					printf("%d\n", WSAGetLastError());

					LINGER optval;
					optval.l_onoff = 1;
					optval.l_linger = 0;
					setsockopt(sock, SOL_SOCKET, SO_LINGER, (char*)&optval, sizeof(optval));

					closesocket(sock);
					WSACleanup();
					return false;
					
				}

				// ������ �ִٸ�, ���ܼ¿� ������ �Դ��� üũ
				else if (exset.fd_count > 0)
				{
					//���ܼ� �����̸� ������ ��.
					printf("Select ---> exset problem..\n");					

					closesocket(sock);
					WSACleanup();
					return false;
				}

				// ����¿� ������ �Դ��� üũ
				// ������� ���� ��� �׳� ���������� �ѹ� �� üũ�غ�
				else if (wset.fd_count > 0)
				{
					// �ٽ� ������ �������� �ٲ� �� break;
					// ������ �������� ����
					ULONG on = 0;
					DWORD check = ioctlsocket(sock, FIONBIO, &on);
					if (check == SOCKET_ERROR)
					{
						printf("NoneBlock Change Fail...\n");
						return false;
					}

					break;

				}
				
			}			
		}




		// -----------------
		// �� ������ connect
		// -----------------
		//SOCKADDR_IN clientaddr;
		//ZeroMemory(&clientaddr, sizeof(clientaddr));
		//clientaddr.sin_family = AF_INET;
		//clientaddr.sin_port = htons(m_usServer_Port);
		//InetPton(AF_INET, m_tIP, &clientaddr.sin_addr.S_un.S_addr);

		//if (connect(sock, (SOCKADDR*)&clientaddr, sizeof(clientaddr)) == SOCKET_ERROR)
		//{
		//	_tprintf(L"Connect Fail...(%d)\n", WSAGetLastError());
		//	return false;
		//}
		





		// ---------------------------
		// Recv / Send �ð� ���� (���� �ɼ� ����)
		// ---------------------------
		int Optval = 3000;

		if (setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, (char*)&Optval, sizeof(Optval)) == SOCKET_ERROR)
		{
			fputs("SO_SNDTIMEO ���� �� ����\n", stdout);
			return false;
		}

		if (setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (char*)&Optval, sizeof(Optval)) == SOCKET_ERROR)
		{
			fputs("SO_RCVTIMEO ���� �� ����\n", stdout);
			return false;
		}

		

		





		// -----------------
		// �� ������ ������ Send
		// -----------------
		if (send(sock, utf8_HTTP_Data, (int)strlen(utf8_HTTP_Data), 0) == SOCKET_ERROR)
		{
			_tprintf(L"send Fail...(%d)\n", WSAGetLastError());
			return false;
		}



		// -----------------
		// send ���������� �ٷ� recv ȣ��.
		// UTF-8�� ���·� Json_Buff�� �־���
		// -----------------
		char Json_Buff[2000];
		if (HTTP_Recv(sock, Json_Buff, 2000) == false)
			return false;





		// -----------------
		// HTTP_Recv()���� ���� UTF-8�� UTF-16���� ��ȯ ��, ReturnBuff�� ����.
		// -----------------
		// �Ʒ� �ڵ�� "strlen(Json_Buff)"�� ���� ������.
		//len = MultiByteToWideChar(CP_UTF8, 0, Json_Buff, (int)strlen(Json_Buff), NULL, NULL);
		len = (int)strlen(Json_Buff);

		MultiByteToWideChar(CP_UTF8, 0, Json_Buff, (int)strlen(Json_Buff), ReturnBuff, len);





		// -----------------
		// ���� ����
		// -----------------
		
		closesocket(sock);
		WSACleanup();

		return true;


	}


	// HTTP_ReqANDRes() �Լ� �ȿ��� ȣ��
	// Host���� �������� �������.
	// Host�������� IP�� �����켭 "IP"�� ����.
	bool HTTP_Exchange::DomainToIP(TCHAR* IP, TCHAR* Host)
	{
		SOCKADDR_IN* SockAddr;

		ADDRINFOW* pAddrInfo;
		if (GetAddrInfo(Host, _T("0"), NULL, &pAddrInfo) != 0)
		{
			int errorcode = WSAGetLastError();
			_tprintf(_T("GetAddrInfo Fail.. : %d\n"), errorcode);
			return false;
		}

		SockAddr = (SOCKADDR_IN*)pAddrInfo->ai_addr;
		IN_ADDR Add = SockAddr->sin_addr;

		// ��Ʈ��ũ ���� -> ȣ��Ʈ ���ķ� ����
		ntohl(Add.s_addr);

		//IPv4 ���� -> ��Ʈ������ ����
		InetNtop(AF_INET, &Add, IP, sizeof(IP));

		// ���ҽ� ����
		FreeAddrInfo(pAddrInfo);

		return true;

	}

	// HTTP_ReqANDRes()�Լ� �ȿ��� ȣ��.
	// ����� �������� ������ �����͸� ��� ����
	// UTF-8�� ���·� Return Buff�� �־���
	// 
	// ���޹޴� ���ڰ�
	// [�������� ����� Socket, (out)UTF-8�� ���Ϲ��� char�� ����]
	// 
	// ���ϰ�
	// ture : ���������� ��� ���� / false : ���� ������ ���ܼ� �� ������.
	bool HTTP_Exchange::HTTP_Recv(SOCKET sock, char* ReturnBuff, int BuffSize)
	{
		while (1)
		{
			char recvBuff[1024];
			// --------------
			// recv()
			// --------------
			DWORD Check = recv(sock, recvBuff, 1024, 0);
			if (Check == SOCKET_ERROR)
			{
				_tprintf(L"recv Fail...(%d)\n", WSAGetLastError());
				return false;
			}

			// �������� �� break;
			if (Check == 0)
				break;


			// --------------
			// ����� HTTP code ��󳻱�
			// --------------
			char* startPtr = strstr(recvBuff, " ");
			++startPtr;
			char* endPtr = strstr(startPtr, " ");

			__int64 Size = endPtr - startPtr;

			char Code[10];
			strncpy_s(Code, 10, startPtr, Size);




			// --------------
			// HTTP code �˻��ϱ�. 200�ƴϸ� �� ����
			// --------------		
			if (strcmp(Code, "200") != 0)
			{
				printf("Recv Header Code Error...(%s)\n", Code);
				return false;
			}




			// --------------
			// ���� ����� �� �ȹ޾�����, �ٽ� recv�Ϸ� ����.
			// --------------	
			char* HeaderEndPtr = strstr(recvBuff, "\r\n\r\n");
			if (HeaderEndPtr == NULL)
				continue;




			// --------------
			// ����� �� �޾�����, Content-Length�� �˾ƿͼ� �׸�ŭ recv�ߴ��� Ȯ��
			// "Content-Length: " �� ������� ������ �� 16����. �� ��Ʈ�� ������ ���̰� ��Ʈ������ ����ִ�.
			// --------------	
			startPtr = strstr(recvBuff, "Content-Length: ");
			startPtr += 16;
			endPtr = strstr(startPtr, "\r\n");

			Size = endPtr - startPtr;

			char LengthString[10];
			strncpy_s(LengthString, 10, startPtr, Size);

			int Content_Length = atoi(LengthString);


			// HeaderEndPtr���� ����� ��(\r\n\r\n)���� ����ִ�.
			// ��, Content_Length + 4 ��ŭ length�� �־�� �Ѵ�.
			int BodyLength = (int)strlen(HeaderEndPtr);

			// ���� ���޾����� �ٽ� ó������ ���ư��� Recv�Ѵ�.
			if (BodyLength < Content_Length + 4)
				continue;



			// -------------
			// �� �޾�����, HeaderEndPtr�� 4ĭ ������(\r\n\r\n�̵�)�̵���Ų ��, 
			// ���Ϲ��ۿ� copy�Ѵ�.
			// -------------
			HeaderEndPtr += 4;
			strncpy_s(ReturnBuff, BuffSize, HeaderEndPtr, Content_Length);
			break;
		}


		return true;

	}
}