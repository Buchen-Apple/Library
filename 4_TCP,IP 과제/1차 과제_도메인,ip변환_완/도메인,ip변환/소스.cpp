#pragma comment(lib, "ws2_32")
#include <locale.h>
#include <stdio.h>
#include <tchar.h>
#include <WinSock2.h>
#include <Ws2tcpip.h>

using namespace std;

int main()
{
	setlocale(LC_ALL, "korean");
	TCHAR tDomainName[20];	
	TCHAR Buff[33];
	IN_ADDR Addr[10];
	IN6_ADDR Addr_6[10];
	WSADATA wsa;
	SOCKADDR_IN* SockAddr;
	SOCKADDR_IN6* SockAddr6;

	// ���� �ʱ�ȭ ����ó��
	if (WSAStartup(0x0202, &wsa) != 0)
	{
		int errorcode = WSAGetLastError();
		_tprintf(_T("WSAStartup ���� : %d\n"), errorcode);
		return 1;
	}

	while (1)
	{
		// ����� �ʱ�ȭ
		ADDRINFOW* pAddrInfo;	

		_tprintf(_T("������ �̸� : "));
		_tscanf_s(_T("%s"), tDomainName, (unsigned)_countof(tDomainName));

		// ������ ���� ���� ó��
		if (GetAddrInfo(tDomainName, _T("0"), NULL, &pAddrInfo) != 0)
		{
			int errorcode = WSAGetLastError();
			_tprintf(_T("GetAddrInfo ���� : %d\n"), errorcode);
			break;
		}

		else
		{
			int i = 0;

			// ���� �ݺ� �Է¹޴´�.
			while (1)
			{
				// IPv4 ������ ����
				if (pAddrInfo->ai_family == 2)
				{
					SockAddr = (SOCKADDR_IN*)pAddrInfo->ai_addr;
					Addr[i] = SockAddr->sin_addr;
				}

				// IPv6 ������ ����
				else
				{
					SockAddr6 = (SOCKADDR_IN6*)pAddrInfo->ai_addr;
					Addr_6[i] = SockAddr6->sin6_addr;
				}
						
				// ��Ʈ��ũ ���� -> ȣ��Ʈ ���ķ� ����
				ntohl(Addr[i].s_addr);

				//IPv4 ���� -> ��Ʈ������ ����
				if(pAddrInfo->ai_family == 2)
					InetNtop(AF_INET, &Addr[i], Buff, sizeof(Buff));

				
				//IPv6 ���� -> ��Ʈ������ ����
				else
					InetNtop(AF_INET6, &Addr_6[i], Buff, sizeof(Buff));

				// ���
				_tprintf(_T("IP : %s\n"), Buff);

				// ���� IP üũ.
				if (pAddrInfo->ai_next != NULL)
				{
					pAddrInfo = pAddrInfo->ai_next;
					i++;
				}
				else
					break;

			}
			_tprintf(_T("\n"));
		}

		FreeAddrInfo(pAddrInfo);
	}

	WSACleanup();
	return 0;
}