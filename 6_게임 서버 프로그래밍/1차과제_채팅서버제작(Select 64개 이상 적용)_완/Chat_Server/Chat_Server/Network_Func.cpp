#include "stdafx.h"
#include "Network_Func.h"
#include "RingBuff\RingBuff.h"

#include <list>
#include <map>

using namespace std;

// ���� ����ü
struct stClinet
{
	SOCKET m_sock;								// �ش� Ŭ���̾�Ʈ�� ����
	CRingBuff m_RecvBuff;						// ���ú� ����
	CRingBuff m_SendBuff;						// ���� ����
	DWORD m_UserID;								// ���� ���� ID

	TCHAR m_NickName[dfNICK_MAX_LEN] = _T("0");	// ���� �г���
	DWORD m_JoinRoomNumber;						// ���� �������� �� �ѹ�. �������� ���� ������ 0
	TCHAR m_tIP[33];							// �ش� ������ IP
	WORD  m_wPort;								// �ش� ������ Port
};

// �� ����ü
struct stRoom
{
	DWORD m_RoomID;						// �� ���� ID
	TCHAR* m_RoomName;					// �� �̸�

	list<DWORD> m_JoinUserList;			// �ش� �뿡 �������� ������, ���� ���� ID
};

// Network_Process()�Լ�����, �̹��� select()�� �õ��� Client�� �����ϴ� ����ü
// ���ϰ� ���� ID�� �˰� �ִ�.
struct SOCKET_SAVE
{
	SOCKET* m_sock;
	DWORD  m_UserID;
};

// ���� ����
DWORD g_dwUserID = 0;
DWORD g_dwRoomID = 1;						// ��ѹ� 0�̸�, �뿡 �ȵ� �ִ� ������ �Ǵ��ϱ� ������ 1���� �����Ѵ�.
map <DWORD, stClinet*> map_ClientList;		// ������ ������ Ŭ���̾�Ʈ�� ���� ���(map).
map <DWORD, stRoom*> map_RoomList;			// ������ �����ϴ� �� ���� ���(map)

// ���ڷ� ���� UserID�� �������� Ŭ���̾�Ʈ ��Ͽ��� [������ ��󳽴�].(�˻�)
// ���� ��, �ش� ������ ���� ����ü�� �ּҸ� ����
// ���� �� nullptr ����
stClinet* ClientSearch(DWORD UserID)
{
	stClinet* NowUser;
	map <DWORD, stClinet*>::iterator iter;
	for (iter = map_ClientList.begin(); iter != map_ClientList.end(); ++iter)
	{
		if (iter->first == UserID)
		{
			NowUser = iter->second;
			return NowUser;
		}
	}

	return nullptr;
}

// ���ڷ� ���� RoomID�� �������� �� ��Ͽ��� [���� ��󳽴�].(�˻�)
// ���� ��, �ش� ���� ���� ����ü�� �ּҸ� ����
// ���� �� nullptr ����
stRoom* RoomSearch(DWORD RoomID)
{
	stRoom* NowRoom;
	map <DWORD, stRoom*>::iterator itor;
	for (itor = map_RoomList.begin(); itor != map_RoomList.end(); ++itor)
	{
		if (itor->first == RoomID)
		{
			NowRoom = itor->second;
			return NowRoom;
		}
	}

	return nullptr;
}

// ��Ʈ��ũ ���μ���
// ���⼭ false�� ��ȯ�Ǹ�, ���α׷��� ����ȴ�.
bool Network_Process(SOCKET* listen_sock)
{
	// Ŭ��� ��ſ� ����� ����
	static FD_SET rset;
	static FD_SET wset;

	SOCKADDR_IN clinetaddr;
	map <DWORD, stClinet*>::iterator itor;
	TIMEVAL tval;
	tval.tv_sec = 0;
	tval.tv_usec = 0;

	itor = map_ClientList.begin();

	DWORD dwFDCount;

	while (1)
	{
		// select �غ�	
		FD_ZERO(&rset);
		FD_ZERO(&wset);
		FD_SET(*listen_sock, &rset);
		dwFDCount = 0;

		SOCKET_SAVE NowSock[FD_SETSIZE-1];

		// ��� ����� ��ȸ�ϸ�, �ش� ������ �б� �°� ���� �¿� �ִ´�.
		// �ִٰ� 64���� �ǰų�, end�� �����ϸ� break
		while (itor != map_ClientList.end())
		{
			// �ش� Ŭ���̾�Ʈ���� ���� �����Ͱ� �ִ��� üũ�ϱ� ����, ��� Ŭ�� rset�� ���� ����
			FD_SET(itor->second->m_sock, &rset);
			NowSock[dwFDCount].m_sock = &itor->second->m_sock;
			NowSock[dwFDCount].m_UserID = itor->first;

			// ����, �ش� Ŭ���̾�Ʈ�� SendBuff�� ���� ������, wset���� ���� ����.
			if (itor->second->m_SendBuff.GetUseSize() != 0)
				FD_SET(itor->second->m_sock, &wset);				

			// 64�� �� á����, ���� �����ߴ��� üũ
			++itor;
			if (dwFDCount+2 == FD_SETSIZE || itor == map_ClientList.end())
				break;

			++dwFDCount;			
		}

		// Select()
		DWORD dCheck = select(0, &rset, &wset, 0, &tval);

		// Select()��� ó��
		if (dCheck == SOCKET_ERROR)
		{
			_tprintf(_T("select ����\n"));
			return false;
		}

		// select�� ���� 0���� ũ�ٸ� ���� �Ұ� �ִٴ� ���̴� ���� ����
		else if (dCheck > 0)
		{
			// ���� ���� ó��
			if (FD_ISSET(*listen_sock, &rset))
			{
				int addrlen = sizeof(clinetaddr);
				SOCKET client_sock = accept(*listen_sock, (SOCKADDR*)&clinetaddr, &addrlen);

				// ������ �߻��ϸ�, �׳� �� ������ ���� ��� ����
				if (client_sock == INVALID_SOCKET)
					_tprintf(_T("accept ����\n"));

				// ������ �߻����� �ʾҴٸ�, "�α��� ��û" ��Ŷ�� �� ��. �̿� ���� ó��
				else
					Accept(&client_sock, clinetaddr);	// Accept ó��.
			}

			// �������� �� ó��
			for (DWORD i = 0; i <= dwFDCount; ++i)
			{
				// �б� �� ó��
				if (FD_ISSET(*NowSock[i].m_sock, &rset))
				{
					// Recv() ó��
					// ����, RecvProc()�Լ��� false�� ���ϵȴٸ�, �ش� ���� ���� ����
					bool Check = RecvProc(NowSock[i].m_UserID);
					if (Check == false)
					{
						Disconnect(NowSock[i].m_UserID);
						continue;
					}
				}

				// ���� �� ó��
				if (FD_ISSET(*NowSock[i].m_sock, &wset))
				{
					// Send() ó��
					// ����, SendProc()�Լ��� false�� ���ϵȴٸ�, �ش� ���� ���� ����
					bool Check = SendProc(NowSock[i].m_UserID);
					if (Check == false)
						Disconnect(NowSock[i].m_UserID);
				}
			}
		}

		// ����, ��� Client�� ���� Selectó���� ��������, �̹� �Լ��� ���⼭ ����.
		if (itor == map_ClientList.end())
			break;

	}

	return true;
}

// Accept ó��
void Accept(SOCKET* client_sock, SOCKADDR_IN clinetaddr)
{
	// ���ڷ� ���� clinet_sock�� �ش�Ǵ� ���� ����
	stClinet* NewUser = new stClinet;

	// ID, ���� ��ѹ�, sock ����
	NewUser->m_UserID = g_dwUserID;
	NewUser->m_JoinRoomNumber = 0;
	NewUser->m_sock = *client_sock;

	// �ش� ������ Ŭ���̾�Ʈ ������Ͽ� �߰�.
	typedef pair<DWORD, stClinet*> Itn_pair;
	map_ClientList.insert(Itn_pair(g_dwUserID, NewUser));

	// ���� ���� ID ����
	g_dwUserID++;

	// ������ ������ ip, port, ����ID ���
	TCHAR Buff[33];
	InetNtop(AF_INET, &clinetaddr.sin_addr, Buff, sizeof(Buff));
	WORD port = ntohs(clinetaddr.sin_port);

	_tcscpy_s(NewUser->m_tIP, _countof(Buff), Buff);
	NewUser->m_wPort = port;
	
	_tprintf(_T("Accept : [%s : %d / UserID : %d]\n"), Buff, port, NewUser->m_UserID);
}

// Disconnect ó��
void Disconnect(DWORD UserID)
{
	// ���ڷ� ���� UserID�� �������� Ŭ���̾�Ʈ ��Ͽ��� ���� �˾ƿ���.
	stClinet* NowUser = ClientSearch(UserID);

	// ���ܻ��� üũ
	// 1. �ش� ������ �α��� ���� �����ΰ�.
	// NowUser�� ���� nullptr�̸�, �ش� ������ ��ã�� ��. false�� �����Ѵ�.
	if (NowUser == nullptr)
	{
		_tprintf(_T("Disconnect(). �α��� ���� �ƴ� ������ ������� ���� �õ�.\n"));
		return;
	}		

	// 2. �ش� ������ �� �ȿ� �־��ٸ�, ���� �� ���� �������� ������ �޽����� �߼��ؾ� �Ѵ�.
	// �ش� ������ ������ ���� �� ������ SendBuff�� �����͸� �־�ΰ�, ���� Select�� �߼��Ѵ�.
	if (NowUser->m_JoinRoomNumber != 0)
	{
		char* packet = NULL;
		Network_Req_RoomLeave(UserID, packet);
	}

	// 3. �ش� ������ map ��Ͽ��� ����
	map_ClientList.erase(UserID);

	// 4. �ش� ������ ���� close
	closesocket(NowUser->m_sock);

	// 5. ���� ������ ������ ip, port, ����ID ���	
	_tprintf(_T("Disconnect : [%s : %d / UserID : %d]\n"), NowUser->m_tIP, NowUser->m_wPort, UserID);

	// 6. �ش� ���� ���� ����
	delete NowUser;

}

// ������ ������ ��� ������ SendBuff�� ��Ŷ �ֱ�
void BroadCast_All(CProtocolBuff* header, CProtocolBuff* Packet)
{
	map <DWORD, stClinet*>::iterator itor;
	for (itor = map_ClientList.begin(); itor != map_ClientList.end(); ++itor)
	{
		bool Check = SendPacket(itor->first, header, Packet);
		if (Check == false)
			Disconnect(itor->first);
	}	
}

// ���� �濡 �ִ� ��� ������, SendBuff�� ��Ŷ �ֱ�
// UserID�� -1�� ������ �� ���� ��� ���� ���.
// UserID�� �ٸ� ���� ������, �ش� ID�� ������ ����.
void BroadCast_Room(CProtocolBuff* header, CProtocolBuff* Packet, DWORD RoomID, int UserID)
{
	// �ش� ID�� ���� �˾ƿ´�.
	stRoom* NowRoom = RoomSearch(RoomID);

	// �ش� �� ���� ��� ������ SendBuff�� �޽��� �ֱ�
	// UserID�� 0���� �����̱� ������ -1�� ������, �׳� ���������̴�.
	list<DWORD>::iterator listitor;
	for (listitor = NowRoom->m_JoinUserList.begin(); listitor != NowRoom->m_JoinUserList.end(); ++listitor)
	{
		if (UserID == *listitor)
			continue;		

		bool Check = SendPacket(*listitor, header, Packet);
		if (Check == false)
			Disconnect(*listitor);
	}	
}



/////////////////////////
// Recv ó�� �Լ���
/////////////////////////
// Recv() ó��
bool RecvProc(DWORD UserID)
{
	// ���ڷ� ���� UserID�� �������� Ŭ���̾�Ʈ ��Ͽ��� ���� �˾ƿ���.
	stClinet* NowUser = ClientSearch(UserID);

	// ���ܻ��� üũ
	// 1. �ش� ������ �α��� ���� �����ΰ�.
	// NowUser�� ���� nullptr�̸�, �ش� ������ ��ã�� ��. false�� �����Ѵ�.
	if (NowUser == nullptr)
	{
		_tprintf(_T("RecvProc(). �α��� ���� �ƴ� ������ ������� RecvProc ȣ���. ���� ����\n"));
		return false;
	}

	////////////////////////////////////////////////////////////////////////
	// ���� ���ۿ� �ִ� ��� recv �����͸�, �ش� ������ recv�����۷� ���ú�.
	////////////////////////////////////////////////////////////////////////

	// 1. recv ������ ������ ������.
	char* recvbuff = NowUser->m_RecvBuff.GetBufferPtr();

	// 2. �� ���� �� �� �ִ� �������� ���� ���� ���
	int Size = NowUser->m_RecvBuff.GetNotBrokenPutSize();

	// 3. �� ���� �� �� �ִ� ������ 0�̶��
	if (Size == 0)
	{
		// rear 1ĭ �̵�
		NowUser->m_RecvBuff.MoveWritePos(1);

		// �׸��� �� ���� �� �� �ִ� ��ġ �ٽ� �˱�.
		Size = NowUser->m_RecvBuff.GetNotBrokenPutSize();
	}

	// 4. �װ� �ƴ϶��, 1ĭ �̵��� �ϱ�. 		
	else
	{
		// rear 1ĭ �̵�
		NowUser->m_RecvBuff.MoveWritePos(1);
	}

	// 5. 1ĭ �̵��� rear ������
	int* rear = (int*)NowUser->m_RecvBuff.GetWriteBufferPtr();

	// 6. recv()
	int retval = recv(NowUser->m_sock, &recvbuff[*rear], Size, 0);

	// 7. ���ú� ����üũ
	if (retval == SOCKET_ERROR)
	{
		// WSAEWOULDBLOCK������ �߻��ϸ�, ���� ���۰� ����ִٴ� ��. recv�Ұ� ���ٴ� ���̴� �Լ� ����.
		if (WSAGetLastError() == WSAEWOULDBLOCK)
			return true;
		
		// 10054 ������ ������ ������ ������ ��. 0�� ���ϵǴ°��� ��������.
		// �� ���� �׳� return false�� ���� ����� ������ ���´�.
		else if (WSAGetLastError() == 10054 || retval == 0)
			return false;

		// WSAEWOULDBLOCK������ �ƴϸ� ���� �̻��� ���̴� ���� ����
		else
		{
			_tprintf(_T("RecvProc(). retval �� ������ �ƴ� ������ ��(%d). ���� ����\n"), WSAGetLastError());
			return false;	// FALSE�� ���ϵǸ�, ������ �����.
		}
	}

	// 8. ������� ������ Rear�� �̵���Ų��.
	NowUser->m_RecvBuff.MoveWritePos(retval - 1);


	//////////////////////////////////////////////////
	// �Ϸ� ��Ŷ ó�� �κ�
	// RecvBuff�� ����ִ� ��� �ϼ� ��Ŷ�� ó���Ѵ�.
	//////////////////////////////////////////////////
	while (1)
	{
		// 1. RecvBuff�� �ּ����� ����� �ִ��� üũ. (���� = ��� ������� ���ų� �ʰ�. ��, �ϴ� �����ŭ�� ũ�Ⱑ �ִ��� üũ)		
		st_PACKET_HEADER HeaderBuff;
		int len = sizeof(HeaderBuff);

		// RecvBuff�� ��� ���� ���� ũ�Ⱑ ��� ũ�⺸�� �۴ٸ�, �Ϸ� ��Ŷ�� ���ٴ� ���̴� while�� ����.
		if (NowUser->m_RecvBuff.GetUseSize() < len)
			break;

		// 2. ����� Peek���� Ȯ���Ѵ�.		
		// Peek �ȿ�����, ��� �ؼ����� len��ŭ �д´�. ���۰� �� ���� ���� �̻�!
		int PeekSize = NowUser->m_RecvBuff.Peek((char*)&HeaderBuff, len);

		// Peek()�� ���۰� ������� -1�� ��ȯ�Ѵ�. ���۰� ������� �ϴ� ���´�.
		if (PeekSize == -1)
		{
			_tprintf(_T("RecvProc(). �Ϸ���Ŷ ��� ó�� �� RecvBuff�����. ���� ����\n"));
			return false;	// FALSE�� ���ϵǸ�, ������ �����.
		}

		// 3. ����� code Ȯ��(���������� �� �����Ͱ� �´°�)
		if (HeaderBuff.byCode != dfPACKET_CODE)
		{
			_tprintf(_T("RecvProc(). �Ϸ���Ŷ ��� ó�� �� PacketCodeƲ��. ���� ����\n"));
			return false;	// FALSE�� ���ϵǸ�, ������ �����.
		}

		// 4. ����� Len���� RecvBuff�� ������ ������ ��. (�ϼ� ��Ŷ ������ = ��� ������ + ���̷ε� Size )
		// ��� ���, �ϼ� ��Ŷ ����� �ȵǸ� while�� ����.
		if (NowUser->m_RecvBuff.GetUseSize() < (len + HeaderBuff.wPayloadSize))
			break;

		// 5. RecvBuff���� Peek�ߴ� ����� ����� (�̹� Peek������, �׳� Remove�Ѵ�)
		NowUser->m_RecvBuff.RemoveData(len);

		// 6. RecvBuff���� ���̷ε� Size ��ŭ ���̷ε� �ӽ� ���۷� �̴´�. (��ť�̴�. Peek �ƴ�)
		DWORD BuffArray = 0;
		CProtocolBuff PayloadBuff;
		DWORD PayloadSize = HeaderBuff.wPayloadSize;

		while (PayloadSize > 0)
		{
			int DequeueSize = NowUser->m_RecvBuff.Dequeue(PayloadBuff.GetBufferPtr() + BuffArray, PayloadSize);

			// Dequeue()�� ���۰� ������� -1�� ��ȯ. ���۰� ������� �ϴ� ���´�.
			if (DequeueSize == -1)
			{
				_tprintf(_T("RecvProc(). �Ϸ���Ŷ ���̷ε� ó�� �� RecvBuff�����. ���� ����\n"));
				return false;	// FALSE�� ���ϵǸ�, ������ �����.
			}

			PayloadSize -= DequeueSize;
			BuffArray += DequeueSize;
		}
		PayloadBuff.MoveWritePos(BuffArray);

		// 7. ����� üũ�� Ȯ�� (2�� ����)
		CProtocolBuff ChecksumBuff;
		memcpy(ChecksumBuff.GetBufferPtr(), PayloadBuff.GetBufferPtr(), PayloadBuff.GetUseSize());
		ChecksumBuff.MoveWritePos(PayloadBuff.GetUseSize());

		DWORD Checksum = 0;
		for (int i = 0; i < HeaderBuff.wPayloadSize; ++i)
		{
			BYTE ChecksumAdd;
			ChecksumBuff >> ChecksumAdd;
			Checksum += ChecksumAdd;
		}

		BYTE* MsgType = (BYTE*)&HeaderBuff.wMsgType;
		for (int i = 0; i < sizeof(HeaderBuff.wMsgType); ++i)
			Checksum += MsgType[i];

		Checksum = Checksum % 256;

		if (Checksum != HeaderBuff.byCheckSum)
		{
			_tprintf(_T("RecvProc(). �Ϸ���Ŷ üũ�� ����. ���� ����\n"));
			return false;	// FALSE�� ���ϵǸ�, ������ �����.
		}

		// 8. ����� ����ִ� Ÿ�Կ� ���� �б�ó��.
		bool check = PacketProc(HeaderBuff.wMsgType, UserID, (char*)&PayloadBuff);
		if (check == false)
			return false;
	}


	return true;
}

// ��Ŷ ó��
// PacketProc() �Լ����� false�� ���ϵǸ� �ش� ������ ������ �����.
bool PacketProc(WORD PacketType, DWORD UserID, char* Packet)
{
	_tprintf(_T("PacketRecv [UserID : %d / TypeID : %d]\n"), UserID, PacketType);

	bool check = true;

	try
	{
		switch (PacketType)
		{

		// ä�� ��û (Ŭ->��)
		case df_REQ_CHAT:
		{
			check = Network_Req_Chat(UserID, Packet);
			if (check == false)
				return false;	// �ش� ���� ���� ����
		}
		break;

		// ������ ��û (Ŭ->��)
		case df_REQ_ROOM_ENTER:
		{
			check = Network_Req_RoomJoin(UserID, Packet);
			if (check == false)
				return false;	// �ش� ���� ���� ����
		}
		break;


		// �� ���� ��û (Ŭ->��)
		case df_REQ_ROOM_LEAVE:
		{
			check = Network_Req_RoomLeave(UserID, Packet);
			if (check == false)
				return false;	// �ش� ���� ���� ����
		}
		break;

		// ����� ��û (Ŭ->��)
		case df_REQ_ROOM_CREATE:
		{
			check = Network_Req_RoomCreate(UserID, Packet);
			if (check == false)
				return false;	// �ش� ���� ���� ����
		}
		break;

		// �α��� ��û (Ŭ->��)
		case df_REQ_LOGIN:
		{
			check = Network_Req_Login(UserID, Packet);
			if (check == false)
				return false;	// �ش� ���� ���� ����
		}
		break;


		// ���� ��û (Ŭ->��) 
		case df_REQ_ROOM_LIST:
		{
			check = Network_Req_RoomList(UserID, Packet);
			if (check == false)
				return false;	// �ش� ���� ���� ����
		}
		break;			

		default:
			_tprintf(_T("�̻��� �޽����Դϴ�.\n"));
			return false;
		}


	}
	catch (CException exc)
	{
		TCHAR* text = (TCHAR*)exc.GetExceptionText();
		_tprintf(_T("%s.\n"), text);
		return false;

	}
	
	return true;
}

// "�α��� ��û" ��Ŷ ó��
bool Network_Req_Login(DWORD UserID, char* Packet)
{
	// 1. ����ȭ ���ۿ� ���̷ε� �ֱ�(�г���)
	CProtocolBuff* LoginPacket = (CProtocolBuff*)Packet;

	// 2. ����ȭ ���ۿ��� �г��� ��󳻱�.
	TCHAR NickName[dfNICK_MAX_LEN];
	*LoginPacket >> NickName;

	// 3. �ش� ID�� ���� �˾ƿ���.
	stClinet* NowUser = ClientSearch(UserID);

	// 4. ���� ó��
	// ���� ������ ���(���⼭ ���� ������, Accept�� ���� ����), ������ ���´�.
	if (NowUser == nullptr)
	{
		_tprintf(_T("Network_Req_Login(). Accept���� �ƴ� ������(UserID : %d). ���� ����\n"), UserID);
		return false;
	}

	// 4. �г��� �ߺ� üũ
	CProtocolBuff header(dfHEADER_SIZE);
	CProtocolBuff packet;

	map <DWORD, stClinet*>::iterator iter;
	for (iter = map_ClientList.begin(); iter != map_ClientList.end(); ++iter)
	{
		// �ߺ� �г����� �ִٸ�, "�ߺ� �г���" ��Ŷ�� �����.
		if (_tcscmp(iter->second->m_NickName, NickName) == 0)
		{
			// �ߺ� �г��� ��Ŷ �����
			CreatePacket_Res_Login((char*)&header, (char*)&packet, UserID, df_RESULT_LOGIN_DNICK);

			// "�α��� ��û ���"�� �ش� ������ SendBuff�� �ֱ�
			SendPacket(UserID, &header, &packet);

			// �׸��� ��� �������� Send()
			SendProc(UserID);

			// return fase�� ���� ������ �����Ų��.
			return false;
		}
	}

	// 5. �ߺ� �г����� ���ٸ�, [�г��� ����, ���� ��Ŷ ����] ����
	// �г��� ����
	_tcscpy_s(NowUser->m_NickName, dfNICK_MAX_LEN, NickName);

	// ���� ��Ŷ ����
	CreatePacket_Res_Login((char*)&header, (char*)&packet, UserID, df_RESULT_LOGIN_OK);

	// 6. "�α��� ��û ���"�� �ش� ������ SendBuff�� �ֱ�.
	bool Check = SendPacket(UserID, &header, &packet);
	if (Check == false)
		return false;

	return true;
}

// "���� ��û" ��Ŷ ó��
bool Network_Req_RoomList(DWORD UserID, char* Packet)
{
	// 1. �ش� ID�� ���� �˾ƿ���.
	stClinet* NowUser = ClientSearch(UserID);

	// 2. ���� ó��
	// ���� ������ ���(���⼭ ���� ������, Accept�� ���� ����), ������ ���´�.
	if (NowUser == nullptr)
	{
		_tprintf(_T("Network_Req_RoomList(). �α��� ���� ���� ������ ������ ��û��(UserID : %d). ���� ����\n"), UserID);
		return false;
	}

	// 3. "�� ��� ���" ��Ŷ ����
	CProtocolBuff header(dfHEADER_SIZE);
	CProtocolBuff packet;

	CreatePacket_Res_RoomList((char*)&header, (char*)&packet);

	// 4. ���� ��Ŷ�� �ش� ������ SendBuff�� �ֱ�.
	bool Check = SendPacket(UserID, &header, &packet);
	if (Check == false)
		return false;


	return true;
}

// "��ȭ�� ���� ��û" ��Ŷ ó��
bool Network_Req_RoomCreate(DWORD UserID, char* Packet)
{
	// 1. �ش� ID�� ���� �˾ƿ���.
	stClinet* NowUser = ClientSearch(UserID);

	// 2. ���� ó��
	// ���� ������ ���(���⼭ ���� ������, Accept�� ���� ����), ������ ���´�.
	if (NowUser == nullptr)
	{
		_tprintf(_T("Network_Req_RoomCreate(). �α��� ���� ���� ������ ����� ��û(UserID : %d). ���� ����\n"), UserID);
		return false;
	}

	CProtocolBuff* RoomCreatePacket = (CProtocolBuff*)Packet;

	// 3. 2����Ʈ ������. �� 2����Ʈ���� �� ������ Size�� ����ִ� (����Ʈ ����. �ι��� ����. ���ڴ� �����ڵ�)
	WORD wRoomNameSize = 0;
	*RoomCreatePacket >> wRoomNameSize;

	// 4. ���� �̾Ƴ���
	// �ι��� �ϼ��� ���� �����Ҵ� ��, +1(TCHAR�̱� ������ 2����Ʈ)�� �Ѵ�.
	// �̰� ���ؼ� �޸� ħ�� ��û ���Ծ���...��
	TCHAR* RoomName = new TCHAR[(wRoomNameSize/2)+1];
	int i;
	for (i = 0; i < wRoomNameSize / 2; ++i)
		*RoomCreatePacket >> RoomName[i];

	RoomName[i] = '\0';

	// 5. ���̸� �ߺ� üũ
	CProtocolBuff RCHeader(dfHEADER_SIZE);
	CProtocolBuff RCPacket;

	map <DWORD, stRoom*>::iterator iter;
	for (iter = map_RoomList.begin(); iter != map_RoomList.end(); ++iter)
	{
		// �� �̸��� �ߺ��̶��, �ߺ� ���̸� ��Ŷ�� �����.
		if (_tcscmp(iter->second->m_RoomName, RoomName) == 0)
		{
			// �ߺ� ���̸� ��Ŷ �����
			CreatePacket_Res_RoomCreate((char*)&RCHeader, (char*)&RCPacket, df_RESULT_ROOM_CREATE_DNICK, wRoomNameSize, (char*)RoomName);

			// "��ȭ�� ���� ��û ���"�� �ش� ������ SendBuff�� �ֱ�
			bool Check = SendPacket(UserID, &RCHeader, &RCPacket);
			if (Check == false)
				return false;

			return true;			
		}
	}


	// 6. �� �̸��� �ߺ����� �ʴ´ٸ�, [�� ����, ������Ŷ ����] ����
	// �� ����
	stRoom* NewRoom = new stRoom;
	NewRoom->m_RoomID = g_dwRoomID;
	NewRoom->m_RoomName = new TCHAR[wRoomNameSize];
	NewRoom->m_RoomName = RoomName;

	typedef pair<DWORD, stRoom*> Itn_pair;
	map_RoomList.insert(Itn_pair(g_dwRoomID, NewRoom));		

	// ���� ��Ŷ ����
	CreatePacket_Res_RoomCreate((char*)&RCHeader, (char*)&RCPacket, df_RESULT_ROOM_CREATE_OK, wRoomNameSize, (char*)RoomName);

	// �� ��ȣ ����
	g_dwRoomID++;

	// 7. "��ȭ�� ���� ��û ���"�� ��� ������ SendBuff�� �ֱ�.
	BroadCast_All(&RCHeader, &RCPacket);

	return true;
}

// "������ ��û" ��Ŷ ó��
bool Network_Req_RoomJoin(DWORD UserID, char* Packet)
{
	// 1. �ش� ID�� ���� ���� �˾ƿ���.
	stClinet* NowUser = ClientSearch(UserID);

	// 2. ���� ó��
	// ���� ������ ���(���⼭ ���� ������, Accept�� ���� ����), ������ ���´�.
	if (NowUser == nullptr)
	{
		_tprintf(_T("Network_Req_RoomJoin(). �α��� ���� ���� ������ ������ ��û(UserID : %d). ���� ����\n"), UserID);
		return false;
	}

	// 3. Packet���� �����ϰ��� �ϴ� ���� ID�� �̾ƿ´�.
	CProtocolBuff* ID = (CProtocolBuff*)Packet;
	DWORD dwJoinRoomID;

	*ID >> dwJoinRoomID;

	// 4. �ش� ID�� ���� �˾ƿ´�.
	stRoom* NowRoom = RoomSearch(dwJoinRoomID);
	
	CProtocolBuff header(dfHEADER_SIZE);
	CProtocolBuff packet;
	BYTE result;
	DWORD RoomID;

	// 5. ����, ���� ���ٸ�(�� No����) ����� "�����"�� �ִ´�.
	if (NowRoom == nullptr)
	{
		result = df_RESULT_ROOM_ENTER_NOT;
		RoomID = 0;
	}

	// ���� �ִٸ�, ����� "OK"�� �ִ´�
	else
	{
		result = df_RESULT_ROOM_ENTER_OK;

		// �׸��� ������, ������ ���ߴ� �濡 �߰��Ѵ�.
		NowUser->m_JoinRoomNumber = dwJoinRoomID;
		NowRoom->m_JoinUserList.push_back(UserID);
		RoomID = NowRoom->m_RoomID;
	}

	// 6. "������ ��û ���" ��Ŷ ����
	CreatePacket_Res_RoomJoin((char*)&header, (char*)&packet, result, RoomID);

	// 7. ���� ��Ŷ�� �ش� ������ SendBuff�� �ֱ�.
	bool Check = SendPacket(UserID, &header, &packet);
	if (Check == false)
		return false;

	// 9. �ش� ������ �濡 ���ٸ�,
	// �ش� ������ ���� �濡 �ִ� �ٸ� ��������, �ش� ������ �����ߴٰ� �˷�����Ѵ�.
	// "Ÿ ����� ����" ��Ŷ�� ����� ������.
	if (result == df_RESULT_ROOM_ENTER_OK)
	{
		Check = Network_Req_UserRoomJoin(UserID);
		if (Check == false)
			return false;
	}

	return true;
}

// "ä�� ��û" ��Ŷ ó��
bool Network_Req_Chat(DWORD UserID, char* Packet)
{
	// 1. �ش� ID�� ���� ���� �˾ƿ���.
	stClinet* NowUser = ClientSearch(UserID);

	// 2. ���� ó��
	// ���� ������ ���(���⼭ ���� ������, Accept�� ���� ����), ������ ���´�.
	if (NowUser == nullptr)
	{
		_tprintf(_T("Network_Req_Chat(). �α��� ���� ���� ������ ä�� ��û�� ����(UserID : %d). ���� ����\n"), UserID);
		return false;
	}

	// �濡 ���� ������ ä�ÿ�û�� �����ٸ�, ������ ���´�.
	if (NowUser->m_JoinRoomNumber == 0)
	{
		_tprintf(_T("Network_Req_Chat(). �濡 ���� ���� ������ ä�ÿ�û�� ����.(UserID : %d). ���� ����\n"), UserID);
		return false;
	}

	// 3. �޽��� ���̸� �˾ƿ´�.
	CProtocolBuff* MessagePacket = (CProtocolBuff*)Packet;
	WORD MessageSize;
	*MessagePacket >> MessageSize;

	// 4. �޽����� �̾ƿ´�.
	TCHAR* Message = new TCHAR[MessageSize / 2];
	for (int i = 0; i < MessageSize / 2; ++i)
		*MessagePacket >> Message[i];

	// 5. "ä�� ��û ���" ��Ŷ ���� 
	CProtocolBuff header(dfHEADER_SIZE);
	CProtocolBuff packet;

	CreatePacket_Res_RoomJoin((char*)&header, (char*)&packet, MessageSize, (char*)Message, UserID);

	// 6. ���� ��Ŷ��, ���� �濡 �ִ� ��� ������ SendBuff�� �ִ´�. (�۽��� 1�� ����)
	BroadCast_Room(&header, &packet, NowUser->m_JoinRoomNumber, UserID);

	return true;
}

// "�� ���� ��û" ��Ŷ ó��
bool Network_Req_RoomLeave(DWORD UserID, char* Packet)
{
	// 1. �ش� ID�� ���� ���� �˾ƿ���.
	stClinet* NowUser = ClientSearch(UserID);

	// 2. ���� ó��
	// ���� ������ ���(���⼭ ���� ������, Accept�� ���� ����), ������ ���´�.
	if (NowUser == nullptr)
	{
		_tprintf(_T("Network_Req_RoomLeave(). �α��� ���� ���� ������ �� ���� ��û�� ����(UserID : %d). ���� ����\n"), UserID);
		return false;
	}

	// �濡 ���� ������ ä�ÿ�û�� �����ٸ�, ������ ���´�.
	if (NowUser->m_JoinRoomNumber == 0)
	{
		_tprintf(_T("Network_Req_RoomLeave(). �濡 ���� ���� ������ �� ���� ��û�� ����.(UserID : %d). ���� ����\n"), UserID);
		return false;
	}

	// 3. "�� ���� ��û ���" ��Ŷ ����.
	CProtocolBuff header(dfHEADER_SIZE);
	CProtocolBuff packet;

	CreatePacket_Res_RoomLeave((char*)&header, (char*)&packet, UserID, NowUser->m_JoinRoomNumber);

	// 4. ���� ��Ŷ�� ���� �� ���� ��� ������ SendBuff�� �ִ´�. (�� ����)
	BroadCast_Room(&header, &packet, NowUser->m_JoinRoomNumber, -1);

	// 5. �� ������ ��û�� ������, �濡�� ���ܽ�Ų��.
	stRoom* NowRoom = RoomSearch(NowUser->m_JoinRoomNumber);
	list<DWORD>::iterator listitor;
	for (listitor = NowRoom->m_JoinUserList.begin(); listitor != NowRoom->m_JoinUserList.end(); ++listitor)
	{
		if (*listitor == UserID)
		{
			NowRoom->m_JoinUserList.erase(listitor);
			break;
		}
	}
	NowUser->m_JoinRoomNumber = 0;

	// 6. ����, �ش� ���� ������ 0���� �Ǿ��ٸ�, �� ���� ��Ŷ�� ��� �������� ������.
	if (NowRoom->m_JoinUserList.size() == 0)
		Network_Req_RoomDelete(NowRoom->m_RoomID);

	return true;
}

// "�� ���� ���"�� ����� ������ 
// �濡 ����� �ִٰ�, 0���� �Ǵ� ���� �߻��Ѵ�.
bool Network_Req_RoomDelete(DWORD RoomID)
{
	// 1. "�� ���� ���" ��Ŷ �����
	CProtocolBuff header(dfHEADER_SIZE);
	CProtocolBuff packet;

	CreatePacket_Res_RoomDelete((char*)&header, (char*)&packet, RoomID);

	// 2. �α��� �� ��� ������ SendBuff�� "�� ���� ���" ��Ŷ �ֱ�
	BroadCast_All(&header, &packet);

	// �׸��� �� ���� �� ����Ʈ���� ���ܽ�Ų��.
	map_RoomList.erase(RoomID);
	return true;
}

// "Ÿ ����� ����" ��Ŷ�� ����� ������.
// � ������ �濡 ���� ��, �ش� �濡 �ִ� �ٸ� �������� �ٸ� ����ڰ� �����ߴٰ� �˷��ִ� ��Ŷ (��->Ŭ �� ����)
bool Network_Req_UserRoomJoin(DWORD JoinUserID)
{
	// 1. �ش� ������ ������ �˾ƿ´�.
	stClinet* NowUser = ClientSearch(JoinUserID);

	// 2. ����ó��
	// �α��� ���� ��������
	if (NowUser == nullptr)
	{
		_tprintf(_T("Network_Req_UserRoomJoin(). �α��� ���� ���� ������ �濡 �����ߴٰ� ��.(UserID : %d). ���� ����\n"), JoinUserID);
		return false;
	}

	// �濡 �ִ� ��������
	if (NowUser->m_JoinRoomNumber == 0)
	{
		_tprintf(_T("Network_Res_UserRoomJoin(). ���� ���� ������ �濡 �����ߴٰ� ó����. (UserID : %d). ���� ����\n"), JoinUserID);
		return false;
	}

	// 3. "Ÿ ����� ����" ��Ŷ �����
	CProtocolBuff header(dfHEADER_SIZE);
	CProtocolBuff packet;	

	CreatePacket_Res_UserRoomJoin((char*)&header, (char*)&packet, (char*)NowUser->m_NickName, JoinUserID);

	// 4. �ش� ������ �����濡 �ִ� ��� ����(�ش� ���� ����)�� SendBuff�� ��Ŷ�� �ִ´�.
	BroadCast_Room(&header, &packet, NowUser->m_JoinRoomNumber, JoinUserID);

	return true;
}



/////////////////////////
// ��Ŷ ���� �Լ�
/////////////////////////
// "�α��� ��û ���" ��Ŷ ����
void CreatePacket_Res_Login(char* header, char* Packet, int UserID, char result)
{
	// 1. "�α��� ��û ���" ��Ŷ ����
	CProtocolBuff* paylodBuff = (CProtocolBuff*)Packet;

	BYTE bResult = result;
	DWORD dwUserID = UserID;

	*paylodBuff << bResult;
	*paylodBuff << dwUserID;

	// 2. ��� ����
	CProtocolBuff* headerBuff = (CProtocolBuff*)header;

	BYTE byCode = dfPACKET_CODE;
	WORD wMsgType = df_RES_LOGIN;
	WORD wPayloadSize = df_RES_LOGIN_SIZE;

	// ����� üũ�� ����	
	CProtocolBuff ChecksumBuff;
	ChecksumBuff << bResult;
	ChecksumBuff << dwUserID;

	DWORD Checksum = 0;
	for (int i = 0; i < wPayloadSize; ++i)
	{
		BYTE bPayloadByte;
		ChecksumBuff >> bPayloadByte;
		Checksum += bPayloadByte;
	}

	BYTE* MsgType = (BYTE*)&wMsgType;
	for (int i = 0; i < sizeof(wMsgType); ++i)
		Checksum += MsgType[i];

	BYTE SaveChecksum = Checksum % 256;

	*headerBuff << byCode;
	*headerBuff << SaveChecksum;
	*headerBuff << wMsgType;
	*headerBuff << wPayloadSize;
}

// "�� ��� ��û ���" ��Ŷ ����
void CreatePacket_Res_RoomList(char* header, char* Packet)
{
	// 1. "�� ��� ��û ���" ��Ŷ ���� (���̷ε�)
	CProtocolBuff* paylodBuff = (CProtocolBuff*)Packet;
	CProtocolBuff ChecksumBuff;

	// ������ �����ϴ� �� ��
	WORD wCount = (WORD)map_RoomList.size();
	*paylodBuff << wCount;

	map <DWORD, stRoom*>::iterator itor;
	for (itor = map_RoomList.begin(); itor != map_RoomList.end(); ++itor)
	{
		// �� No
		DWORD dwRoomNumber = itor->second->m_RoomID;

		// �� �̸��� Size
		WORD wRoomNameSize = (WORD)(_tcslen(itor->second->m_RoomName) * 2);

		// �� �̸�(�����ڵ�)
		TCHAR* tRoomName = itor->second->m_RoomName;

		// ���� �ο�
		BYTE bJoinUser = (BYTE)itor->second->m_JoinUserList.size();

		// ������� ���̷ε� ���ۿ� ����.
		*paylodBuff << dwRoomNumber;
		*paylodBuff << wRoomNameSize;	

		for (int i = 0; i < wRoomNameSize/2; ++i)
			*paylodBuff << tRoomName[i];

		*paylodBuff << bJoinUser;

		// ���� �ο����� �г���(�����ڵ� 15���ھ� ���)
		list<DWORD>::iterator listitor;
		for (listitor = itor->second->m_JoinUserList.begin(); listitor != itor->second->m_JoinUserList.end(); ++listitor)
		{			
			stClinet* NowUser = ClientSearch(*listitor);

			for(int i=0; i<dfNICK_MAX_LEN; ++i)
				*paylodBuff << NowUser->m_NickName[i];
		}		
	}

	// 2. üũ���� ����ȭ���� ���� (���̷ε�)
	memcpy(ChecksumBuff.GetBufferPtr(), paylodBuff->GetBufferPtr(), paylodBuff->GetUseSize());
	ChecksumBuff.MoveWritePos(paylodBuff->GetUseSize());

	// 3. ��� ����
	CProtocolBuff* headerBuff = (CProtocolBuff*)header;

	BYTE byCode = dfPACKET_CODE;
	WORD wMsgType = df_RES_ROOM_LIST;
	WORD wPayloadSize = paylodBuff->GetUseSize();

	// ����� üũ�� ä���
	DWORD Checksum = 0;
	for (int i = 0; i < wPayloadSize; ++i)
	{
		BYTE bPayloadByte;
		ChecksumBuff >> bPayloadByte;
		Checksum += bPayloadByte;
	}

	BYTE* MsgType = (BYTE*)&wMsgType;
	for (int i = 0; i < sizeof(wMsgType); ++i)
		Checksum += MsgType[i];

	BYTE SaveChecksum = Checksum % 256;

	// 4. ������ ä���
	*headerBuff << byCode;
	*headerBuff << SaveChecksum;
	*headerBuff << wMsgType;
	*headerBuff << wPayloadSize;
}

// "��ȭ�� ���� ��û ���" ��Ŷ ����
void CreatePacket_Res_RoomCreate(char* header, char* Packet, char result, WORD RoomSize, char* cRoomName)
{
	// 1. "��ȭ�� ���� ��û ���" ��Ŷ ���� (���̷ε�)
	CProtocolBuff* paylodBuff = (CProtocolBuff*)Packet;

	BYTE bResult = result;
	DWORD dwRoomID = g_dwRoomID;
	WORD bRoomNameSize = RoomSize;
	TCHAR* RoomName = (TCHAR*)cRoomName;

	*paylodBuff << bResult;
	*paylodBuff << dwRoomID;
	*paylodBuff << bRoomNameSize;

	for(int i=0; i<bRoomNameSize/2; ++i)
		*paylodBuff << RoomName[i];

	// 2. �Ʒ����� ����� üũ�� ������ ����ϱ� ����, üũ�� ����ȭ���� ���� (���̷ε�)
	CProtocolBuff ChecksumBuff;
	memcpy(ChecksumBuff.GetBufferPtr(), paylodBuff->GetBufferPtr(), paylodBuff->GetUseSize());
	ChecksumBuff.MoveWritePos(paylodBuff->GetUseSize());

	// 3. ��� ����
	CProtocolBuff* headerBuff = (CProtocolBuff*)header;

	BYTE byCode = dfPACKET_CODE;
	WORD wMsgType = df_RES_ROOM_CREATE;
	WORD wPayloadSize = paylodBuff->GetUseSize();

	DWORD Checksum = 0;
	for (int i = 0; i < wPayloadSize; ++i)
	{
		BYTE bPayloadByte;
		ChecksumBuff >> bPayloadByte;
		Checksum += bPayloadByte;
	}

	BYTE* MsgType = (BYTE*)&wMsgType;
	for (int i = 0; i < sizeof(wMsgType); ++i)
		Checksum += MsgType[i];

	BYTE SaveChecksum = (BYTE)(Checksum % 256);

	// 4. ������ �ϼ�
	*headerBuff << byCode;
	*headerBuff << SaveChecksum;
	*headerBuff << wMsgType;
	*headerBuff << wPayloadSize;
}

// "������ ��û ���" ��Ŷ ����
void CreatePacket_Res_RoomJoin(char* header, char* Packet, char result, DWORD RoomID)
{
	// ID�� �̿��� �ش� ���� �˾ƿ´�.
	stRoom* NowRoom = RoomSearch(RoomID);

	// 1. "������ ��û ���" ��Ŷ ���� (���̷ε�)
	CProtocolBuff* paylodBuff = (CProtocolBuff*)Packet;

	// ���� result�� df_RESULT_ROOM_ENTER_NOT(�����)���, result�� ���̷ε忡 �ִ´�. �̰ɷ� ���̷ε� �ϼ�
	if (result == df_RESULT_ROOM_ENTER_NOT)
	{
		BYTE bResult = result;
		*paylodBuff << bResult;
	}

	// ���� result�� df_RESULT_ROOM_ENTER_OK(����� �ƴ�)���, 
	else if(result == df_RESULT_ROOM_ENTER_OK)
	{
		// ���, ��ID, ���̸� ������, �� �̸�(�����ڵ�) ���̷ε忡 �ֱ�
		BYTE bResult = result;
		DWORD dwRoomID = RoomID;
		WORD bRoomNameSize = (WORD)(_tcslen(NowRoom->m_RoomName) * 2);
		TCHAR* RoomName = NowRoom->m_RoomName;

		*paylodBuff << bResult;
		*paylodBuff << dwRoomID;
		*paylodBuff << bRoomNameSize;

		for (int i = 0; i<bRoomNameSize / 2; ++i)
			*paylodBuff << RoomName[i];

		// �����ο� �ֱ�
		BYTE bJoinCount = (BYTE)NowRoom->m_JoinUserList.size();
		*paylodBuff << bJoinCount;

		// �����ο��� �г���(15����Ʈ ����)�� ����ID�� ���̷ε忡 �߰�
		list <DWORD>::iterator listitor;
		for (listitor = NowRoom->m_JoinUserList.begin(); listitor != NowRoom->m_JoinUserList.end(); ++listitor)
		{
			// �濡 �ִ� ���� ID
			DWORD dwUserID = *listitor;

			// �ش� ID�� ���� �г��� �˾Ƴ���
			TCHAR UserNicname[dfNICK_MAX_LEN];
			map <DWORD, stClinet*>::iterator clinetitor;
			for (clinetitor = map_ClientList.begin(); clinetitor != map_ClientList.end(); ++clinetitor)
			{
				if (dwUserID == clinetitor->first)
				{
					_tcscpy_s(UserNicname, dfNICK_MAX_LEN, clinetitor->second->m_NickName);
					break;
				}
			}

			// �г���, ID ������� ���̷ε忡 �߰�
			for (int i = 0; i < dfNICK_MAX_LEN; ++i)
				*paylodBuff << UserNicname[i];
			
			*paylodBuff << dwUserID;
		}
	}

	// 2. �Ʒ����� ����� üũ�� ������ ����ϱ� ����, üũ�� ����ȭ���� ���� (���̷ε�)
	CProtocolBuff ChecksumBuff;
	memcpy(ChecksumBuff.GetBufferPtr(), paylodBuff->GetBufferPtr(), paylodBuff->GetUseSize());
	ChecksumBuff.MoveWritePos(paylodBuff->GetUseSize());

	// 3. ��� ����
	CProtocolBuff* headerBuff = (CProtocolBuff*)header;

	BYTE byCode = dfPACKET_CODE;
	WORD wMsgType = df_RES_ROOM_ENTER;
	WORD wPayloadSize = paylodBuff->GetUseSize();

	DWORD Checksum = 0;
	for (int i = 0; i < wPayloadSize; ++i)
	{
		BYTE bPayloadByte;
		ChecksumBuff >> bPayloadByte;
		Checksum += bPayloadByte;
	}

	BYTE* MsgType = (BYTE*)&wMsgType;
	for (int i = 0; i < sizeof(wMsgType); ++i)
		Checksum += MsgType[i];

	BYTE SaveChecksum = Checksum % 256;

	// 4. ������ �ϼ�
	*headerBuff << byCode;
	*headerBuff << SaveChecksum;
	*headerBuff << wMsgType;
	*headerBuff << wPayloadSize;	
}

// "ä�� ��û ���" ��Ŷ ����
void CreatePacket_Res_RoomJoin(char* header, char* Packet, WORD MessageSize, char* cMessage, DWORD UserID)
{
	// 1. ���̷ε� �ϼ��ϱ�
	DWORD dwSenderID = UserID;
	WORD wMessageSize = MessageSize;
	TCHAR* tMessage = (TCHAR*)cMessage;

	CProtocolBuff* payloadBuff = (CProtocolBuff*)Packet;
	*payloadBuff << dwSenderID;
	*payloadBuff << wMessageSize;
	for (int i = 0; i < wMessageSize / 2; ++i)
		*payloadBuff << tMessage[i];

	// 2. �Ʒ����� ����� üũ�� ������ ����ϱ� ����, üũ�� ����ȭ���� ���� (���̷ε�)
	CProtocolBuff ChecksumBuff;
	memcpy(ChecksumBuff.GetBufferPtr(), payloadBuff->GetBufferPtr(), payloadBuff->GetUseSize());
	ChecksumBuff.MoveWritePos(payloadBuff->GetUseSize());

	// 3. ��� ����
	CProtocolBuff* headerBuff = (CProtocolBuff*)header;

	BYTE byCode = dfPACKET_CODE;
	WORD wMsgType = df_RES_CHAT;
	WORD wPayloadSize = payloadBuff->GetUseSize();

	DWORD Checksum = 0;
	for (int i = 0; i < wPayloadSize; ++i)
	{
		BYTE bPayloadByte;
		ChecksumBuff >> bPayloadByte;
		Checksum += bPayloadByte;
	}

	BYTE* MsgType = (BYTE*)&wMsgType;
	for (int i = 0; i < sizeof(wMsgType); ++i)
		Checksum += MsgType[i];

	BYTE SaveChecksum = Checksum % 256;

	// 4. ������ �ϼ�
	*headerBuff << byCode;
	*headerBuff << SaveChecksum;
	*headerBuff << wMsgType;
	*headerBuff << wPayloadSize;

}

// "�� ���� ��û ���" ��Ŷ ����
void CreatePacket_Res_RoomLeave(char* header, char* Packet, DWORD UserID, DWORD RoomID)
{
	// 1. ���̷ε� �����
	CProtocolBuff* payloadBuff = (CProtocolBuff*)Packet;
	DWORD dwUserID = UserID;

	*payloadBuff << dwUserID;

	// 2. �Ʒ����� ����� üũ�� ������ ����ϱ� ����, üũ�� ����ȭ���� ���� (���̷ε�)
	CProtocolBuff ChecksumBuff;
	ChecksumBuff << dwUserID;

	// 3. ��� ����
	CProtocolBuff* headerBuff = (CProtocolBuff*)header;

	BYTE byCode = dfPACKET_CODE;
	WORD wMsgType = df_RES_ROOM_LEAVE;
	WORD wPayloadSize = payloadBuff->GetUseSize();

	DWORD Checksum = 0;
	for (int i = 0; i < wPayloadSize; ++i)
	{
		BYTE bPayloadByte;
		ChecksumBuff >> bPayloadByte;
		Checksum += bPayloadByte;
	}

	BYTE* MsgType = (BYTE*)&wMsgType;
	for (int i = 0; i < sizeof(wMsgType); ++i)
		Checksum += MsgType[i];

	BYTE SaveChecksum = Checksum % 256;

	// 4. ������ �ϼ�
	*headerBuff << byCode;
	*headerBuff << SaveChecksum;
	*headerBuff << wMsgType;
	*headerBuff << wPayloadSize;

}

// "�� ���� ���" ��Ŷ ����
void CreatePacket_Res_RoomDelete(char* header, char* Packet, DWORD RoomID)
{
	// 1. ���̷ε� �����
	CProtocolBuff* payloadBuff = (CProtocolBuff*)Packet;
	DWORD dwRoomID = RoomID;

	*payloadBuff << RoomID;

	// 2. �Ʒ����� ����� üũ�� ������ ����ϱ� ����, üũ�� ����ȭ���� ���� (���̷ε�)
	CProtocolBuff ChecksumBuff;
	ChecksumBuff << RoomID;

	// 3. ��� ����
	CProtocolBuff* headerBuff = (CProtocolBuff*)header;

	BYTE byCode = dfPACKET_CODE;
	WORD wMsgType = df_RES_ROOM_DELETE;
	WORD wPayloadSize = payloadBuff->GetUseSize();

	DWORD Checksum = 0;
	for (int i = 0; i < wPayloadSize; ++i)
	{
		BYTE bPayloadByte;
		ChecksumBuff >> bPayloadByte;
		Checksum += bPayloadByte;
	}

	BYTE* MsgType = (BYTE*)&wMsgType;
	for (int i = 0; i < sizeof(wMsgType); ++i)
		Checksum += MsgType[i];

	BYTE SaveChecksum = Checksum % 256;

	// 4. ������ �ϼ�
	*headerBuff << byCode;
	*headerBuff << SaveChecksum;
	*headerBuff << wMsgType;
	*headerBuff << wPayloadSize;
}

// "Ÿ ����� ����" ��Ŷ ����
void CreatePacket_Res_UserRoomJoin(char* header, char* Packet, char* cNickName, DWORD UserID)
{
	// 1. ���̷ε� �����
	CProtocolBuff* payloadBuff = (CProtocolBuff*)Packet;

	TCHAR* UserNickName = (TCHAR*)cNickName;
	DWORD dwUserID = UserID;

	for (int i = 0; i < dfNICK_MAX_LEN; ++i)
		*payloadBuff << UserNickName[i];

	*payloadBuff << dwUserID;

	// 2. �Ʒ����� ����� üũ�� ������ ����ϱ� ����, üũ�� ����ȭ���� ���� (���̷ε�)
	CProtocolBuff ChecksumBuff;
	memcpy(ChecksumBuff.GetBufferPtr(), payloadBuff->GetBufferPtr(), payloadBuff->GetUseSize());
	ChecksumBuff.MoveWritePos(payloadBuff->GetUseSize());

	// 3. ��� ����
	CProtocolBuff* headerBuff = (CProtocolBuff*)header;

	BYTE byCode = dfPACKET_CODE;
	WORD wMsgType = df_RES_USER_ENTER;
	WORD wPayloadSize = payloadBuff->GetUseSize();

	DWORD Checksum = 0;
	for (int i = 0; i < wPayloadSize; ++i)
	{
		BYTE bPayloadByte;
		ChecksumBuff >> bPayloadByte;
		Checksum += bPayloadByte;
	}

	BYTE* MsgType = (BYTE*)&wMsgType;
	for (int i = 0; i < sizeof(wMsgType); ++i)
		Checksum += MsgType[i];

	BYTE SaveChecksum = Checksum % 256;

	// 4. ������ �ϼ�
	*headerBuff << byCode;
	*headerBuff << SaveChecksum;
	*headerBuff << wMsgType;
	*headerBuff << wPayloadSize;
}



/////////////////////////
// Send ó��
/////////////////////////
// Send���ۿ� ������ �ֱ�
bool SendPacket(DWORD UserID, CProtocolBuff* headerBuff, CProtocolBuff* payloadBuff)
{
	// ���ڷ� ���� UserID�� �������� Ŭ���̾�Ʈ ��Ͽ��� ���� �˾ƿ���.
	stClinet* NowUser = ClientSearch(UserID);

	// ���ܻ��� üũ
	// 1. �ش� ������ �α��� ���� �����ΰ�.
	// NowUser�� ���� nullptr�̸�, �ش� ������ ��ã�� ��. false�� �����Ѵ�.
	if (NowUser == nullptr)
	{
		_tprintf(_T("SendPacket(). �α��� ���� �ƴ� ������ ������� SendPacket�� ȣ���. ���� ����\n"));
		return false;
	}

	// 1. ���� q�� ��� �ֱ�
	int Size = headerBuff->GetUseSize();
	DWORD BuffArray = 0;
	int a = 0;
	while (Size > 0)
	{
		int EnqueueCheck = NowUser->m_SendBuff.Enqueue(headerBuff->GetBufferPtr() + BuffArray, Size);
		if (EnqueueCheck == -1)
		{
			_tprintf(_T("SendPacket(). ��� �ִ� �� ����ť ����. ���� ����\n"));
			return false;
		}

		Size -= EnqueueCheck;
		BuffArray += EnqueueCheck;
	}

	// 2. ���� q�� ���̷ε� �ֱ�
	WORD PayloadLen = payloadBuff->GetUseSize();
	BuffArray = 0;
	while (PayloadLen > 0)
	{
		int EnqueueCheck = NowUser->m_SendBuff.Enqueue(payloadBuff->GetBufferPtr() + BuffArray, PayloadLen);
		if (EnqueueCheck == -1)
		{
			_tprintf(_T("SendPacket(). ���̷ε� �ִ� �� ����ť ����. ���� ����\n"));
			return false;
		}

		PayloadLen -= EnqueueCheck;
		BuffArray += EnqueueCheck;
	}


	return true;
}

// Send������ �����͸� Send�ϱ�
bool SendProc(DWORD UserID)
{
	// ���ڷ� ���� UserID�� �������� Ŭ���̾�Ʈ ��Ͽ��� ���� �˾ƿ���.
	stClinet* NowUser = ClientSearch(UserID);

	// ���ܻ��� üũ
	// 1. �ش� ������ �α��� ���� �����ΰ�.
	// NowUser�� ���� nullptr�̸�, �ش� ������ ��ã�� ��. false�� �����Ѵ�.
	if (NowUser == nullptr)
	{
		_tprintf(_T("SendProc(). �α��� ���� �ƴ� ������ ������� SendProc�� ȣ���. ���� ����\n"));
		return false;
	}

	// 1. SendBuff�� ���� �����Ͱ� �ִ��� Ȯ��.
	if (NowUser->m_SendBuff.GetUseSize() == 0)
		return true;

	// 3. ���� �������� �������� ���� ������
	char* sendbuff = NowUser->m_SendBuff.GetBufferPtr();

	// 4. SendBuff�� �� �������� or Send����(����)���� ��� send
	while (1)
	{
		// 5. SendBuff�� �����Ͱ� �ִ��� Ȯ��(���� üũ��)
		if (NowUser->m_SendBuff.GetUseSize() == 0)
			break;

		// 6. �� ���� ���� �� �ִ� �������� ���� ���� ���
		int Size = NowUser->m_SendBuff.GetNotBrokenGetSize();

		// 7. ���� ������ 0�̶�� 1�� ����. send() �� 1����Ʈ�� �õ��ϵ��� �ϱ� ���ؼ�.
		// �׷��� send()�� ������ �ߴ��� Ȯ�� ����
		if (Size == 0)
			Size = 1;

		// 8. front ������ ����
		int *front = (int*)NowUser->m_SendBuff.GetReadBufferPtr();

		// 9. front�� 1ĭ �� index�� �˾ƿ���.
		int BuffArrayCheck = NowUser->m_SendBuff.NextIndex(*front, 1);

		// 10. Send()
		int SendSize = send(NowUser->m_sock, &sendbuff[BuffArrayCheck], Size, 0);

		// 11. ���� üũ. �����̸� �� �̻� �������� while�� ����. ���� Select�� �ٽ� ����
		if (SendSize == SOCKET_ERROR)
		{
			if (WSAGetLastError() == WSAEWOULDBLOCK)
				break;

			else
			{
				_tprintf(_T("SendProc(). Send�� ���� �߻�. ���� ����\n"));
				return false;
			}
		}

		// 12. ���� ����� ��������, �� ��ŭ remove
		NowUser->m_SendBuff.RemoveData(SendSize);
	}

	return true;
}
