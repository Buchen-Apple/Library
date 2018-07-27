#include "stdafx.h"
#include "Network_Func.h"
#include "RingBuff\RingBuff.h"

#include "rapidjson\document.h"
#include "rapidjson\writer.h"
#include "rapidjson\stringbuffer.h"

#include <list>
#include <map>

using namespace std;

// Accept ���� ����ü
struct stAcceptUser
{
	SOCKET m_sock;								// �ش� Accept ������ ����
	CRingBuff m_RecvBuff;						// ���ú� ����
	CRingBuff m_SendBuff;						// ���� ����

	UINT64  m_AccountNo;						// Accept ������ �����(�α��� ��) ���� ȸ�� �ѹ�. 0�� ���, �α��� �ȵ� Accept ����

	TCHAR m_tIP[33];							// �ش� ������ IP
	WORD  m_wPort;								// �ش� ������ Port
};

// ȸ�� ���� ����ü
struct stAccount
{
	UINT64  m_AccountNo;						// ���� ȸ�� �ѹ�
	TCHAR	m_NickName[dfNICK_MAX_LEN];			// �г���

	UINT	m_FriendCount = 0;					// �� ģ�� �� ī��Ʈ
	UINT	m_RequestCount = 0;					// ���� ���� ģ����û �� ī��Ʈ
	UINT	m_ReplyCount = 0;					// ���� ���� ģ����û �� ī��Ʈ
};

// ģ������, ���� ��û, ���� ��û ���� �����ϴ� ����ü
struct stFromTo
{
	UINT64 m_FromAccount;
	UINT64 m_ToAccount;
	UINT64 m_Time;
};

// ���� ����
UINT64 g_AcceptCount = 0;							// Accept �� ���� ��
UINT64 g_uiAccountNo = 1;							// ���� AccountNo�� üũ�ϴ� ����. 1���� ����.
UINT64 g_uiFriendIndex = 1;							// map_FriendList���� ���� Index (ģ�� ��� Index)
UINT64 g_uiRequestIndex = 1;						// map_FriendRequestList���� ���� Index (���� ģ����û ��� Index)
UINT64 g_uiReplyIndex = 1;							// map_FriendReplyList���� ���� Index (���� ģ����û ��� Index)

// [Accept ���� ���]
// Key : ���� ��
map <SOCKET, stAcceptUser*> map_AcceptList;

// [ȸ�� ���]
// Key : ȸ�� �ѹ�.
map <UINT64, stAccount*> map_AccountList;

// [ģ�� ���]
// Key : map_FriendList ���� ����Index(g_uiFriendIndex) / Value : stFromTo ����ü. 
// Value�� fromģ��, toģ��, �ð� ��� ����ִ�.
map <UINT64, stFromTo*> map_FriendList;

// [���� ģ����û ���] 
// Key : map_FriendRequestList(g_uiRequestIndex) ���� ���� Index / Value : stFromTo ����ü. 
// Value�� from�� ���� ����, to�� ���� �����̴�.
map <UINT64, stFromTo*> map_FriendRequestList;

// [���� ģ����û ���] 
// Key : map_FriendReplyList ���� ���� Index(g_uiReplyIndex) / Value : stFromTo ����ü. 
// Value�� from�� ��û�� ���� ����, to�� ���� �����̴�.
map <UINT64, stFromTo*> map_FriendReplyList;

// ���ڷ� ���� ȸ��No ���� �������� [ȸ�� ���]���� [������ ��󳽴�].(�˻�)
// ���� ��, �ش� ������ ȸ�� ���� ����ü�� �ּҸ� ����
// ���� �� nullptr ����
stAccount* ClientSearch_AccountList(UINT64 AccountNo)
{
	stAccount* NowAccount;
	map <UINT64, stAccount*>::iterator iter;
	for (iter = map_AccountList.begin(); iter != map_AccountList.end(); ++iter)
	{
		if (iter->first == AccountNo)
		{
			NowAccount = iter->second;
			return NowAccount;
		}
	}

	return nullptr;
}

// ���ڷ� ���� Socket ���� �������� [Accept ���]���� [������ ��󳽴�].(�˻�)
// ���� ��, �ش� ������ ���� ����ü�� �ּҸ� ����
// ���� �� nullptr ����
stAcceptUser* ClientSearch_AcceptList(SOCKET sock)
{
	stAcceptUser* NowAccount;
	map <SOCKET, stAcceptUser*>::iterator iter;
	for (iter = map_AcceptList.begin(); iter != map_AcceptList.end(); ++iter)
	{
		if (iter->first == sock)
		{
			NowAccount = iter->second;
			return NowAccount;
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
	map <SOCKET, stAcceptUser*>::iterator itor;
	TIMEVAL tval;
	tval.tv_sec = 0;
	tval.tv_usec = 0;

	itor = map_AcceptList.begin();

	DWORD dwFDCount;

	while (1)
	{
		// select �غ�	
		FD_ZERO(&rset);
		FD_ZERO(&wset);
		SOCKET* NowSock[FD_SETSIZE];
		dwFDCount = 0;

		// �������� ����		
		FD_SET(*listen_sock, &rset);
		NowSock[dwFDCount] = listen_sock;

		// ��� ����� ��ȸ�ϸ�, �ش� ������ �б� �°� ���� �¿� �ִ´�.
		// �ִٰ� 64���� �ǰų�, end�� �����ϸ� break
		while (itor != map_AcceptList.end())
		{
			++dwFDCount;

			// �ش� Ŭ���̾�Ʈ���� ���� �����Ͱ� �ִ��� üũ�ϱ� ����, ��� Ŭ�� rset�� ���� ����
			FD_SET(itor->second->m_sock, &rset);
			NowSock[dwFDCount] = &itor->second->m_sock;

			// ����, �ش� Ŭ���̾�Ʈ�� SendBuff�� ���� ������, wset���� ���� ����.
			if (itor->second->m_SendBuff.GetUseSize() != 0)
				FD_SET(itor->second->m_sock, &wset);

			// 64�� �� á����, ���� �����ߴ��� üũ		
			++itor;
			if (dwFDCount + 1 == FD_SETSIZE || itor == map_AcceptList.end())
				break;			
		}

		// Select()
		DWORD dCheck = select(0, &rset, &wset, 0, &tval);

		// Select()��� ó��
		if (dCheck == SOCKET_ERROR)
		{
			_tprintf(_T("select ����(%d)\n"), WSAGetLastError());
			return false;
		}

		// select�� ���� 0���� ũ�ٸ� ���� �Ұ� �ִٴ� ���̴� ���� ����
		else if (dCheck > 0)
		{
			for (DWORD i = 0; i <= dwFDCount; ++i)
			{
				// ���� ���� ó��
				if (FD_ISSET(*NowSock[i], &rset) && (*NowSock[i] == *listen_sock))
				{
					int addrlen = sizeof(clinetaddr);
					SOCKET client_sock = accept(*listen_sock, (SOCKADDR*)&clinetaddr, &addrlen);

					// ������ �߻��ϸ�, �׳� �� ������ ���� ��� ����
					if (client_sock == INVALID_SOCKET)
						_tprintf(_T("accept ����\n"));

					// ������ �߻����� �ʾҴٸ�, "�α��� ��û" ��Ŷ�� �� ��. �̿� ���� ó��
					else
						Accept(&client_sock, clinetaddr);	// Accept ó��.

					// ���� �����̸� �б�/���� ������ ���� �ȵǴϱ� conitnue�Ѵ�.
					continue;
				}

				// �б� �� ó��
				if (FD_ISSET(*NowSock[i], &rset))
				{
					// Recv() ó��
					// ����, RecvProc()�Լ��� false�� ���ϵȴٸ�, �ش� ���� ���� ����
					bool Check = RecvProc(*NowSock[i]);
					if (Check == false)
					{
						Disconnect(*NowSock[i]);
						continue;
					}
				}

				// ���� �� ó��
				if (FD_ISSET(*NowSock[i], &wset))
				{
					// Send() ó��
					// ����, SendProc()�Լ��� false�� ���ϵȴٸ�, �ش� ���� ���� ����
					bool Check = SendProc(*NowSock[i]);
					if (Check == false)
						Disconnect(*NowSock[i]);
				}
			}
		}

		// ����, ��� Client�� ���� Selectó���� ��������, �̹� �Լ��� ���⼭ ����.
		if (itor == map_AcceptList.end())
			break;

	}

	return true;
}

// Accept ó��
void Accept(SOCKET* client_sock, SOCKADDR_IN clinetaddr)
{
	// 1. ���ڷ� ���� clinet_sock�� �ش�Ǵ� ���� ����
	stAcceptUser* NewAccount = new stAcceptUser;

	// 2. sock ����. (m_AccountNo�� 0�� �ִ´�. 0�̸�, �α��� ���� ���� �����̴�)
	NewAccount->m_sock = *client_sock;
	NewAccount->m_AccountNo = 0;	

	// 3. ������ ������ ip, port, ���� ���
	TCHAR Buff[33];
	InetNtop(AF_INET, &clinetaddr.sin_addr, Buff, sizeof(Buff));
	WORD port = ntohs(clinetaddr.sin_port);

	_tprintf(_T("Accept : [%s : %d / Socket : %lld]\n"), Buff, port, *client_sock);
		
	// 4. IP�� Port ����
	_tcscpy_s(NewAccount->m_tIP, _countof(Buff), Buff);
	NewAccount->m_wPort = port;
	
	// 5. Accept�� �������� �����ϴ� map�� �߰�	
	typedef pair<SOCKET, stAcceptUser*> Itn_pair;
	map_AcceptList.insert(Itn_pair(*client_sock, NewAccount));

	g_AcceptCount++;
}

// Disconnect ó��
void Disconnect(SOCKET sock)
{
	// ���ڷ� ���� sock�� �������� Accept ��Ͽ��� ���� �˾ƿ���.
	stAcceptUser* NowAccount = ClientSearch_AcceptList(sock);

	// ���ܻ��� üũ
	// 1. �ش� ������ �α��� ���� �����ΰ�.
	// NowUser�� ���� nullptr�̸�, �ش� ������ ��ã�� ��. false�� �����Ѵ�.
	if (NowAccount == nullptr)
	{
		_tprintf(_T("Disconnect(). Accept ���� �ƴ� ������ ������� ���� �õ�.\n"));
		return;
	}	

	// 2. �ش� Accept ������ AcceptList���� ����
	map_AcceptList.erase(sock);

	// 3. ���� ������ ������ ip, port, AccountNo, socket ���	
	_tprintf(_T("Disconnect : [%s : %d / AccountNo : %lld / Socket : %lld]\n"), NowAccount->m_tIP, NowAccount->m_wPort, NowAccount->m_AccountNo, NowAccount->m_sock);

	// 4. �ش� ������ ���� close
	closesocket(NowAccount->m_sock);

	// 5. �ش� Accept���� ���� ����
	delete NowAccount;

	g_AcceptCount--;
}

// ��Ŷ ��� ����
void CreateHeader(CProtocolBuff* headerBuff, WORD MsgType, WORD PayloadSize)
{
	BYTE byCode = dfPACKET_CODE;
	*headerBuff << byCode;
	*headerBuff << MsgType;
	*headerBuff << PayloadSize;
}

// ���� Ŀ��Ʈ�� ���� �� �˾ƿ���
UINT64 AcceptCount()
{
	return g_AcceptCount;
}




/////////////////////////////
// Json �� ���� ����� �Լ�
////////////////////////////

// ���̽��� ������ �����ϴ� �Լ�(UTF-16)
bool Json_Create()
{
	using namespace rapidjson;

	GenericStringBuffer<UTF16<>> StringJson;
	Writer< GenericStringBuffer<UTF16<>>, UTF16<>, UTF16<> > Writer(StringJson);

	Writer.StartObject();	

	// ȸ�� ��� 
	map<UINT64, stAccount*>::iterator Accountitor;
	Writer.String(L"Account");
	Writer.StartArray();
	for (Accountitor = map_AccountList.begin(); Accountitor != map_AccountList.end(); ++Accountitor)
	{
		Writer.StartObject();

		// AccountNo		
		Writer.String(L"AccountNo");
		Writer.Uint64(Accountitor->first);

		// NickName
		Writer.String(L"NickName");
		Writer.String(Accountitor->second->m_NickName);
		
		// ȸ���� ģ�� ��
		Writer.String(L"FriendCount");
		Writer.Uint64(Accountitor->second->m_FriendCount);

		// ȸ���� ���� ��û ��
		Writer.String(L"RequestCount");
		Writer.Uint64(Accountitor->second->m_RequestCount);

		// ȸ���� ���� ��û ��
		Writer.String(L"ReplyCount");
		Writer.Uint64(Accountitor->second->m_ReplyCount);

		Writer.EndObject();
	}
	Writer.EndArray();
	
	// ģ�� ��� 
	map<UINT64, stFromTo*>::iterator Frienditor;
	Writer.String(L"Friend");
	Writer.StartArray();
	for (Frienditor = map_FriendList.begin(); Frienditor != map_FriendList.end(); ++Frienditor)
	{
		Writer.StartObject();

		// FromAccountNo
		Writer.String(L"FromAccountNo");
		Writer.Uint64(Frienditor->second->m_FromAccount);

		// ToAccountNo
		Writer.String(L"ToAccountNo");
		Writer.Uint64(Frienditor->second->m_ToAccount);

		// Time
		Writer.String(L"Time");
		Writer.Uint64(Frienditor->second->m_Time);

		Writer.EndObject();

	}
	Writer.EndArray();

	// ������û ���
	map<UINT64, stFromTo*>::iterator Requestitor;
	Writer.String(L"FriendRequest");
	Writer.StartArray();
	for (Requestitor = map_FriendRequestList.begin(); Requestitor != map_FriendRequestList.end(); ++Requestitor)
	{
		Writer.StartObject();

		// FromAccountNo
		Writer.String(L"FromAccountNo");
		Writer.Uint64(Requestitor->second->m_FromAccount);

		// ToAccountNo
		Writer.String(L"ToAccountNo");
		Writer.Uint64(Requestitor->second->m_ToAccount);

		// Time
		Writer.String(L"Time");
		Writer.Uint64(Requestitor->second->m_Time);

		Writer.EndObject();

	}
	Writer.EndArray();


	// ������û ���
	map<UINT64, stFromTo*>::iterator Replyitor;
	Writer.String(L"FriendReply");
	Writer.StartArray();
	for (Replyitor = map_FriendReplyList.begin(); Replyitor != map_FriendReplyList.end(); ++Replyitor)
	{
		Writer.StartObject();

		// FromAccountNo
		Writer.String(L"FromAccountNo");
		Writer.Uint64(Replyitor->second->m_FromAccount);

		// ToAccountNo
		Writer.String(L"ToAccountNo");
		Writer.Uint64(Replyitor->second->m_ToAccount);

		// Time
		Writer.String(L"Time");
		Writer.Uint64(Replyitor->second->m_Time);

		Writer.EndObject();

	}
	Writer.EndArray();
	Writer.EndObject();

	// pJson���� UTF-16�� ���·� ����ȴ�.
	const TCHAR* tpJson = StringJson.GetString();
	size_t Size = StringJson.GetSize();

	// ���� ���� �� ������ ���� �� ����
	if (FileCreate_UTF16(_T("FriendDB_Json.txt"), tpJson, Size) == false)
	{
		fputs("Json ���� ���� ����...\n", stdout);
		return false;
	}

	fputs("Json ���� ���� ����!\n", stdout);
	return true;
}

// ���� ���� �� ������ ���� �Լ�
bool FileCreate_UTF16(const TCHAR* FileName, const TCHAR* tpJson, size_t StringSize)
{
	FILE* fp;			// ���� ��Ʈ��
	size_t iFileCheck;	// ���� ���� ���� üũ, ������ ������ ����. �� 2���� �˵�

	// ���� ���� (UTF-16 ��Ʋ�����)
	iFileCheck = _tfopen_s(&fp, FileName, _T("wt, ccs=UTF-16LE"));
	if (iFileCheck != 0)
	{
		fputs("fopen �����߻�!\n", stdout);
		return false;
	}

	// ���Ͽ� ������ ����
	iFileCheck = fwrite(tpJson, 1, StringSize, fp);
	if (iFileCheck != StringSize)
	{
		fputs("fwrite �����߻�!\n", stdout);
		return false;
	}

	fclose(fp);
	return true;

}

// ���̽����� ������ �о�� �����ϴ� �Լ�(UTF-16)
bool Json_Get()
{
	using namespace rapidjson;

	TCHAR* tpJson = nullptr;

	// ���Ͽ��� ������ �о����
	if (LoadFile_UTF16(_T("FriendDB_Json.txt"), &tpJson) == false)
	{
		fputs("Json ���� �о���� ����...\n", stdout);
		return false;
	}

	else if (tpJson == nullptr)
	{
		fputs("LoadFile() �Լ����� �о�� ���̽� ������ nullptr.\n", stdout);
		fputs("Json ���� �о���� ����...\n", stdout);
		return false;
	}

	// Json������ �Ľ��ϱ� (�̹� UTF-16�� �ִ���)
	GenericDocument<UTF16<>> Doc;
	Doc.Parse(tpJson);

	/////////////////////////////////
	// ȸ�� ���� ����
	////////////////////////////////
	// AccountArray�� Account���� ������ �ֱ�
	GenericValue<UTF16<>> &AccountArray = Doc[_T("Account")];
	SizeType Size = AccountArray.Size();

	// �ʿ��� ���� ����
	UINT64  AccountNo;			// ���� ȸ�� �ѹ�
	const TCHAR	*NickName;		// �г���

	UINT	FriendCount;		// �� ģ�� �� ī��Ʈ
	UINT	RequestCount;		// ���� ���� ģ����û �� ī��Ʈ
	UINT	ReplyCount;			// ���� ���� ģ����û �� ī��Ʈ
	
	for (SizeType i = 0; i < Size; ++i)
	{		
		GenericValue<UTF16<>> &AccountObject = AccountArray[i];

		// AccountNo ������
		AccountNo = AccountObject[_T("AccountNo")].GetInt64();

		// �г��� ������
		NickName = AccountObject[_T("NickName")].GetString();

		// FriendCount������
		FriendCount = AccountObject[_T("FriendCount")].GetInt64();

		// RequestCount������
		RequestCount = AccountObject[_T("RequestCount")].GetInt64();

		// ReplyCount ������
		ReplyCount = AccountObject[_T("ReplyCount")].GetInt64();
		
		// ���� �����͸� ȸ�� ����ü�� ����
		stAccount* NewAccount = new stAccount;

		NewAccount->m_AccountNo = AccountNo;
		NewAccount->m_FriendCount = FriendCount;
		NewAccount->m_RequestCount = RequestCount;
		NewAccount->m_ReplyCount = ReplyCount;
		_tcscpy_s(NewAccount->m_NickName, dfNICK_MAX_LEN, NickName);

		// ������ ȸ���� [ȸ�� ���]�� �߰�
		typedef pair<UINT64, stAccount*> Itn_pair;
		map_AccountList.insert(Itn_pair(AccountNo, NewAccount));
	}

	// ȸ�� ���� ID �����(���� �������� �� �̾���ϴϱ�!)
	g_uiAccountNo = Size+1;


	/////////////////////////////////
	// ģ�� ��� ����
	////////////////////////////////
	// FriendArray�� Friend���� ������ �ֱ�
	GenericValue<UTF16<>> &FriendArray = Doc[_T("Friend")];
	Size = FriendArray.Size();

	// �ʿ��� ���� ����
	UINT64 FromAccountNo;
	UINT64 ToAccountNo;
	UINT64 Time;

	for (SizeType i = 0; i < Size; ++i)
	{
		GenericValue<UTF16<>> &FriendObject = FriendArray[i];

		// FromAccountNo������
		FromAccountNo = FriendObject[_T("FromAccountNo")].GetInt64();

		// ToAccountNo������
		ToAccountNo = FriendObject[_T("ToAccountNo")].GetInt64();

		// Time������
		Time = FriendObject[_T("Time")].GetInt64();
		
		// ���� �����͸� stFromTo ����ü�� ����
		stFromTo* NewFriendList = new stFromTo;

		NewFriendList->m_FromAccount = FromAccountNo;
		NewFriendList->m_ToAccount = ToAccountNo;
		NewFriendList->m_Time = Time;

		// ������ �����͸� [ģ�� ���]�� �߰�
		// ������ ȸ���� [ȸ�� ���]�� �߰�
		typedef pair<UINT64, stFromTo*> Itn_pair;
		map_FriendList.insert(Itn_pair(g_uiFriendIndex, NewFriendList));
		g_uiFriendIndex++;
	}



	/////////////////////////////////
	// ������û ��� ����
	////////////////////////////////
	// RequestArray�� Friend���� ������ �ֱ�
	GenericValue<UTF16<>> &RequestArray = Doc[_T("FriendRequest")];
	Size = RequestArray.Size();

	for (SizeType i = 0; i < Size; ++i)
	{
		GenericValue<UTF16<>> &RequestObject = RequestArray[i];

		// FromAccountNo������
		FromAccountNo = RequestObject[_T("FromAccountNo")].GetInt64();

		// ToAccountNo������
		ToAccountNo = RequestObject[_T("ToAccountNo")].GetInt64();

		// Time������
		Time = RequestObject[_T("Time")].GetInt64();

		// ���� �����͸� stFromTo ����ü�� ����
		stFromTo* NewRequestList = new stFromTo;

		NewRequestList->m_FromAccount = FromAccountNo;
		NewRequestList->m_ToAccount = ToAccountNo;
		NewRequestList->m_Time = Time;

		// ������ ȸ���� [ȸ�� ���]�� �߰�
		typedef pair<UINT64, stFromTo*> Itn_pair;
		map_FriendRequestList.insert(Itn_pair(g_uiRequestIndex, NewRequestList));
		g_uiRequestIndex++;
	}



	/////////////////////////////////
	// ������û ��� ����
	////////////////////////////////
	// ReplyArray�� Friend���� ������ �ֱ�
	GenericValue<UTF16<>> &ReplyArray = Doc[_T("FriendReply")];
	Size = ReplyArray.Size();

	for (SizeType i = 0; i < Size; ++i)
	{
		GenericValue<UTF16<>> &ReplyObject = ReplyArray[i];

		// FromAccountNo������
		FromAccountNo = ReplyObject[_T("FromAccountNo")].GetInt64();

		// ToAccountNo������
		ToAccountNo = ReplyObject[_T("ToAccountNo")].GetInt64();

		// Time������
		Time = ReplyObject[_T("Time")].GetInt64();

		// ���� �����͸� stFromTo ����ü�� ����
		stFromTo* NewReplyList = new stFromTo;

		NewReplyList->m_FromAccount = FromAccountNo;
		NewReplyList->m_ToAccount = ToAccountNo;
		NewReplyList->m_Time = Time;

		// ������ �����͸� [ģ�� ���]�� �߰�
		// ������ ȸ���� [ȸ�� ���]�� �߰�
		typedef pair<UINT64, stFromTo*> Itn_pair;
		map_FriendReplyList.insert(Itn_pair(g_uiReplyIndex, NewReplyList));
		g_uiReplyIndex++;
	}

	fputs("���̽� ���� ���� ����!\n", stdout);

	return true;
}

// ���Ͽ��� ������ �о���� �Լ�
// pJson�� �о�� �����Ͱ� ����ȴ�.
bool LoadFile_UTF16(const TCHAR* FileName, TCHAR** pJson)
{
	FILE* fp;			// ���� ��Ʈ��
	size_t iFileCheck;	// ���� ���� ���� üũ, ������ ������ ����. �� 2���� �˵�

	// ���� ����
	iFileCheck = _tfopen_s(&fp, FileName, _T("rt, ccs=UTF-16LE"));
	if (iFileCheck != 0)
	{
		// ������ ���� ���, ENOENT(2) ������ �߱⵵ ��
		if (iFileCheck == ENOENT)
			fputs("���̽� �о�� ������ ����!\n", stdout);

		// ������ �ִµ��� ������ �ߴ°Ÿ� �׳� ������.
		else
			fputs("fopen �����߻�!\n", stdout);

		return false;
	}

	// ���� �о���� ���� ���� ũ�⸸ŭ �����Ҵ�
	fseek(fp, 0, SEEK_END);
	iFileCheck = ftell(fp);
	fseek(fp, 0, SEEK_SET);

	// BOM�ڵ� ��������. (2����Ʈ)
	// UTF-16LE�� BOM�� HxD�� ������ FF FE�̴�.
	// �ٵ�, �о���� ��Ʋ��������� ����Ǳ� ������, BOMCheck���� FE FF�� ����ִ�.
	TCHAR BOMCheck;
	if (fread_s(&BOMCheck, sizeof(TCHAR), 2, 1, fp) != 1)
	{
		fputs("fread_s BOM�ڵ� �о���� ��������!\n", stdout);
		return false;
	}

	// BOM�ڵ� �������� �ʴ´ٸ�, ���������ڸ� �ٽ� ó������ ������, ó������ �ٽ� �о�;��Ѵ�.
	if (BOMCheck != 0xfeff)
		fseek(fp, 0, SEEK_SET);

	// BOM�ڵ尡 �����Ѵٸ�, �̹� �׸�ŭ �̵��� ���̴�, BOM��ŭ(2����Ʈ)�� ������ ���� iFileCheck ������
	else
		iFileCheck -= 2;

	*pJson = new TCHAR[iFileCheck];

	// ���Ͽ��� ������ �о����.
	size_t iSize = fread_s(*pJson, iFileCheck, 1, iFileCheck, fp);
	if (iSize != iFileCheck)
	{
		fputs("fread_s �����߻�!\n", stdout);
		return false;
	}

	fclose(fp);

	return true;
}




/////////////////////////
// Recv ó�� �Լ���
/////////////////////////
bool RecvProc(SOCKET sock)
{
	// ���ڷ� ���� sock�� �������� Accept ��Ͽ��� ���� �˾ƿ���.
	stAcceptUser* NowAccount = ClientSearch_AcceptList(sock);

	// ���ܻ��� üũ
	// 1. �ش� ������ �α��� ���� �����ΰ�.
	// NowUser�� ���� nullptr�̸�, �ش� ������ ��ã�� ��. false�� �����Ѵ�.
	if (NowAccount == nullptr)
	{
		_tprintf(_T("RecvProc(). �α��� ���� �ƴ� ������ ������� RecvProc ȣ���. ���� ����\n"));
		return false;
	}

	////////////////////////////////////////////////////////////////////////
	// ���� ���ۿ� �ִ� ��� recv �����͸�, �ش� ������ recv�����۷� ���ú�.
	////////////////////////////////////////////////////////////////////////

	// 1. recv ������ ������ ������.
	char* recvbuff = NowAccount->m_RecvBuff.GetBufferPtr();

	// 2. �� ���� �� �� �ִ� �������� ���� ���� ���
	int Size = NowAccount->m_RecvBuff.GetNotBrokenPutSize();

	// 3. �� ���� �� �� �ִ� ������ 0�̶��
	if (Size == 0)
	{
		// rear 1ĭ �̵�
		NowAccount->m_RecvBuff.MoveWritePos(1);

		// �׸��� �� ���� �� �� �ִ� ��ġ �ٽ� �˱�.
		Size = NowAccount->m_RecvBuff.GetNotBrokenPutSize();
	}

	// 4. �װ� �ƴ϶��, 1ĭ �̵��� �ϱ�. 		
	else
	{
		// rear 1ĭ �̵�
		NowAccount->m_RecvBuff.MoveWritePos(1);
	}

	// 5. 1ĭ �̵��� rear ������
	int* rear = (int*)NowAccount->m_RecvBuff.GetWriteBufferPtr();

	// 6. recv()
	int retval = recv(NowAccount->m_sock, &recvbuff[*rear], Size, 0);

	// 7. ���ú� ����üũ
	if (retval == SOCKET_ERROR)
	{
		// WSAEWOULDBLOCK������ �߻��ϸ�, ���� ���۰� ����ִٴ� ��. recv�Ұ� ���ٴ� ���̴� �Լ� ����.
		if (WSAGetLastError() == WSAEWOULDBLOCK)
			return true;

		// 10054 ������ ������ ������ ������ ��.
		// �� ���� �׳� return false�� ���� ����� ������ ���´�.
		else if (WSAGetLastError() == 10054)
			return false;

		// WSAEWOULDBLOCK������ �ƴϸ� ���� �̻��� ���̴� ���� ����
		else
		{
			_tprintf(_T("RecvProc(). retval �� ������ �ƴ� ������ ��(%d). ���� ����\n"), WSAGetLastError());
			return false;	// FALSE�� ���ϵǸ�, ������ �����.
		}
	}

	// 0�� ���ϵǴ� ���� ��������.
	else if(retval == 0)
		return false;

	// 8. ������� ������ Rear�� �̵���Ų��.
	NowAccount->m_RecvBuff.MoveWritePos(retval - 1);


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
		if (NowAccount->m_RecvBuff.GetUseSize() < len)
			break;

		// 2. ����� Peek���� Ȯ���Ѵ�.		
		// Peek �ȿ�����, ��� �ؼ����� len��ŭ �д´�. ���۰� �� ���� ���� �̻�!
		int PeekSize = NowAccount->m_RecvBuff.Peek((char*)&HeaderBuff, len);

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
		if (NowAccount->m_RecvBuff.GetUseSize() < (len + HeaderBuff.wPayloadSize))
			break;

		// 5. RecvBuff���� Peek�ߴ� ����� ����� (�̹� Peek������, �׳� Remove�Ѵ�)
		NowAccount->m_RecvBuff.RemoveData(len);

		// 6. RecvBuff���� ���̷ε� Size ��ŭ ���̷ε� �ӽ� ���۷� �̴´�. (��ť�̴�. Peek �ƴ�)
		DWORD BuffArray = 0;
		CProtocolBuff PayloadBuff;
		DWORD PayloadSize = HeaderBuff.wPayloadSize;

		while (PayloadSize > 0)
		{
			int DequeueSize = NowAccount->m_RecvBuff.Dequeue(PayloadBuff.GetBufferPtr() + BuffArray, PayloadSize);

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

		// 7. ����� ����ִ� Ÿ�Կ� ���� �б�ó��.
		bool check = PacketProc(HeaderBuff.wMsgType, sock, (char*)&PayloadBuff);
		if (check == false)
			return false;
	}


	return true;
}

// ��Ŷ ó��
// PacketProc() �Լ����� false�� ���ϵǸ� �ش� ������ ������ �����.
bool PacketProc(WORD PacketType, SOCKET sock, char* Packet)
{
	_tprintf(_T("PacketRecv [Socket : %lld / TypeID : %d]\n"), sock, PacketType);

	bool check = true;

	try
	{
		switch (PacketType)
		{

		// ȸ�� ���� ��û
		case df_REQ_ACCOUNT_ADD:
		{
			check = Network_Req_AccountAdd(sock, Packet);
			if (check == false)
				return false;	// �ش� ���� ���� ����
		}
		break;

		// ȸ�� �α��� ��û
		case df_REQ_LOGIN:
		{
			check = Network_Req_Login(sock, Packet);
			if (check == false)
				return false;	// �ش� ���� ���� ����
		}
		break;


		// ȸ�� ��� ��û
		case df_REQ_ACCOUNT_LIST:
		{
			check = Network_Req_AccountList(sock, Packet);
			if (check == false)
				return false;	// �ش� ���� ���� ����
		}
		break;

		// �� ģ�� ��� ��û
		case df_REQ_FRIEND_LIST:
		{
			check = Network_Req_FriendList(sock, Packet);
			if (check == false)
				return false;	// �ش� ���� ���� ����
		}
		break;

		// ���� ģ����û ��� ��û
		case df_REQ_FRIEND_REQUEST_LIST:
		{
			check = Network_Req_RequestList(sock, Packet);
			if (check == false)
				return false;	// �ش� ���� ���� ����
		}
		break;


		// ���� ģ����û ��� ��û
		case df_REQ_FRIEND_REPLY_LIST:
		{
			check = Network_Req_ReplytList(sock, Packet);
			if (check == false)
				return false;	// �ش� ���� ���� ����
		}
		break;

		// ģ�� ����
		case df_REQ_FRIEND_REMOVE:
		{
			check = Network_Req_FriendRemove(sock, Packet);
			if (check == false)
				return false;	// �ش� ���� ���� ����
		}
		break;

		// ģ�� ��û
		case df_REQ_FRIEND_REQUEST:
		{
			check = Network_Req_FriendRequest(sock, Packet);
			if (check == false)
				return false;	// �ش� ���� ���� ����
		}
		break;

		// ���� ģ����û ���
		case df_REQ_FRIEND_CANCEL:
		{
			check = Network_Req_RequestCancel(sock, Packet);
			if (check == false)
				return false;	// �ش� ���� ���� ����
		}
		break;

		// ���� ģ����û ����
		case df_REQ_FRIEND_DENY:
		{
			check = Network_Req_RequestDeny(sock, Packet);
			if (check == false)
				return false;	// �ش� ���� ���� ����
		}
		break;

		// ģ����û ����
		case df_REQ_FRIEND_AGREE:
		{
			check = Network_Req_FriendAgree(sock, Packet);
			if (check == false)
				return false;	// �ش� ���� ���� ����
		}
		break;

		// ��Ʈ���� �׽�Ʈ�� ����
		case df_REQ_STRESS_ECHO:
		{
			check = Network_Req_StressTest(sock, Packet);
			if (check == false)
				return false;	// �ش� ���� ���� ����
		}
		break;

		default:
			_tprintf(_T("�̻��� ��Ŷ�Դϴ�.\n"));
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

// "ȸ������ ��û(ȸ����� �߰�)" ��Ŷ ó��
bool Network_Req_AccountAdd(SOCKET sock, char* Packet)
{
	// 1. ���ڷ� ���� sock�� �̿��� Accept�� ȸ�� �˾ƿ���
	stAcceptUser* NowAccount = ClientSearch_AcceptList(sock);

	// 2. ���� ó��
	// ���� ������ ���(���⼭ ���� ������, Accept�� ���� ����), ������ ���´�.
	// �̷���Ȳ�� �߻������� �𸣰�����..Ȥ�ø𸣴� ó����
	if (NowAccount == nullptr)
	{
		_tprintf(_T("Network_Req_AccountAdd(). Accept���� ���� ������ ȸ������(ȸ������߰�)��û. (SOCKET : %lld). ���� ����\n"), sock);
		return false;
	}	

	// 3. �г��� ��󳻱�
	CProtocolBuff* AccountAddPacket = (CProtocolBuff*)Packet;
	TCHAR NickName[dfNICK_MAX_LEN];
	*AccountAddPacket >> NickName;	

	// 4. ������ ȸ�� ���� ���� (�г��� AccountNo)
	stAccount* NewUser = new stAccount;
	_tcscpy_s(NewUser->m_NickName, _countof(NickName), NickName);
	NewUser->m_AccountNo = g_uiAccountNo;
	g_uiAccountNo++;

	// 5. "ȸ������ ��û(ȸ����� �߰�) ���" ��Ŷ ����
	CProtocolBuff header(dfHEADER_SIZE);
	CProtocolBuff Payload;

	Network_Res_AccountAdd((char*)&header, (char*)&Payload, NewUser->m_AccountNo);

	// 6. �ش� ������ ���� �� [ȸ�� ���] map�� �߰�
	typedef pair<UINT64, stAccount*> Itn_Pair;
	map_AccountList.insert(Itn_Pair(NewUser->m_AccountNo, NewUser));
	
	// 7. ���� ��Ŷ�� �ش� ������, SendBuff�� �ֱ�
	bool check = SendPacket(sock, &header, &Payload);
	if (check == false)
		return false;

	return true;
}

// "�α��� ��û" ��Ŷ ó��
bool Network_Req_Login(SOCKET sock, char* Packet)
{
	// 1. ���ڷ� ���� sock�� �̿��� Accept�� ȸ�� �˾ƿ���
	stAcceptUser* NowAccount = ClientSearch_AcceptList(sock);

	// 2. ���� ó��
	// ���� ������ ���(���⼭ ���� ������, Accept ���� ����), ������ ���´�.
	// �̷���Ȳ�� �߻������� �𸣰�����..Ȥ�ø𸣴� ó����
	if (NowAccount == nullptr)
	{
		_tprintf(_T("Network_Req_Login(). Accept���� ���� ������ �α��� ��û��. (SOCKET : %lld). ���� ����\n"), sock);
		return false;
	}

	// 3. �α��� ��ǥ AccountNo�� �˾ƿ´�.
	CProtocolBuff* LoginPacket = (CProtocolBuff*)Packet;
	UINT64 AcoNo;

	*LoginPacket >> AcoNo;

	// 4. �ش� AccountNo�� [ȸ�� ���]�� �ִ��� üũ
	stAccount* NowUser = nullptr;
	map<UINT64, stAccount*>::iterator itor;
	for (itor = map_AccountList.begin(); itor != map_AccountList.end(); ++itor)
	{
		if (itor->first == AcoNo)
		{
			NowUser = itor->second;
			break;
		}
	}

	// 5-1 ����, �ش� AccountNo�� ���� ���� ���ٸ�, ���� �޽����� ������ ���� AcoNo�� 0�� �ִ´�.
	TCHAR NickName[dfNICK_MAX_LEN];

	if (NowUser == nullptr)
		AcoNo = 0;

	// 5-2. ����, �ش� AccountNo�� ���� ���� �ִٸ� �ش� AccountNo ȸ���� �г��� ����
	// �׸���, �ش� Accept������ �α��� �� ���� ����
	else
	{
		_tcscpy_s(NickName, _countof(NickName), NowUser->m_NickName);
		NowAccount->m_AccountNo = NowUser->m_AccountNo;
	}

	// 6. "�α��� ��û ���" ��Ŷ ����
	CProtocolBuff header(dfHEADER_SIZE);
	CProtocolBuff Payload;

	Network_Res_Login((char*)&header, (char*)&Payload, AcoNo, NickName);

	// 7. ���� ��Ŷ�� �ش� ������, SendBuff�� �ֱ�
	bool check = SendPacket(sock, &header, &Payload);
	if (check == false)
		return false;

	return true;
}

// "ȸ�� ��� ��û" ��Ŷ ó��
bool Network_Req_AccountList(SOCKET sock, char* Packet)
{
	// 1. ���ڷ� ���� sock�� �̿��� Accept�� ȸ������ �˾ƿ���
	stAcceptUser* NowAccount = ClientSearch_AcceptList(sock);

	// 2. ���� ó��
	// ���� ������ ���(���⼭ ���� ������, Accept�� ���� ����), ������ ���´�.
	// �̷���Ȳ�� �߻������� �𸣰�����..Ȥ�ø𸣴� ó����
	if (NowAccount == nullptr)
	{
		_tprintf(_T("Network_Req_AccountList(). Accept���� ���� ������ ȸ�� ��Ͽ�û. (SOCKET : %lld). ���� ����\n"), sock);
		return false;
	}

	// 3. "ȸ�� ��� ��û ���" ��Ŷ ����
	CProtocolBuff header(dfHEADER_SIZE);
	CProtocolBuff Payload;

	Network_Res_AccountList((char*)&header, (char*)&Payload);
	
	// 4. ���� ��Ŷ�� �ش� ������, SendBuff�� �ֱ�
	bool check = SendPacket(sock, &header, &Payload);
	if (check == false)
		return false;

	return true;
}

// "ģ�� ��� ��û" ��Ŷ ó��
bool Network_Req_FriendList(SOCKET sock, char* Packet)
{
	// 1. ���ڷ� ���� sock�� �̿��� Accept�� ȸ������ �˾ƿ���
	stAcceptUser* NowAccount = ClientSearch_AcceptList(sock);

	// 2. ���� ó��
	// ���� ������ ���(���⼭ ���� ������, Accept�� ���� ����), ������ ���´�.
	// �̷���Ȳ�� �߻������� �𸣰�����..Ȥ�ø𸣴� ó����
	if (NowAccount == nullptr)
	{
		_tprintf(_T("Network_Req_AccountList(). Accept���� ���� ������ ȸ�� ��Ͽ�û. (SOCKET : %lld). ���� ����\n"), sock);
		return false;
	}

	// 3. "ģ�� ��� ��û ���" ��Ŷ ����
	CProtocolBuff header(dfHEADER_SIZE);
	CProtocolBuff Payload;

	stAccount* NowUser = ClientSearch_AccountList(NowAccount->m_AccountNo);

	// ���� �α����� ������ �ƴ϶��, ģ�� ī��Ʈ�� 0 ����
	UINT FriendCount;
	if (NowUser == nullptr)
		FriendCount = 0;
	else
		FriendCount = NowUser->m_FriendCount;

	Network_Res_FriendList((char*)&header, (char*)&Payload, NowAccount->m_AccountNo, FriendCount);

	// 4. ���� ��Ŷ�� �ش� ������, SendBuff�� �ֱ�
	bool check = SendPacket(sock, &header, &Payload);
	if (check == false)
		return false;

	return true;

}

// "���� ģ����û ���" ��Ŷ ó��
bool Network_Req_RequestList(SOCKET sock, char* Packet)
{
	// 1. ���ڷ� ���� sock�� �̿��� Accept�� ȸ������ �˾ƿ���
	stAcceptUser* NowAccount = ClientSearch_AcceptList(sock);

	// 2. ���� ó��
	// ���� ������ ���(���⼭ ���� ������, Accept�� ���� ����), ������ ���´�.
	// �̷���Ȳ�� �߻������� �𸣰�����..Ȥ�ø𸣴� ó����
	if (NowAccount == nullptr)
	{
		_tprintf(_T("Network_Req_RequestList(). Accept���� ���� ������ [���� ģ����û ���] ��û. (SOCKET : %lld). ���� ����\n"), sock);
		return false;
	}

	// 3. "���� ģ����û ��� ���" ��Ŷ ����
	CProtocolBuff header(dfHEADER_SIZE);
	CProtocolBuff Payload;

	stAccount* NowUser = ClientSearch_AccountList(NowAccount->m_AccountNo);

	// ���� �α����� ������ �ƴ϶��, ���� ��û ���� 0 ����
	UINT RequestCount;
	if (NowUser == nullptr)
		RequestCount = 0;
	else
		RequestCount = NowUser->m_RequestCount;

	Network_Res_RequestList((char*)&header, (char*)&Payload, NowAccount->m_AccountNo, RequestCount);

	// 4. ���� ��Ŷ�� �ش� ������, SendBuff�� �ֱ�
	bool check = SendPacket(sock, &header, &Payload);
	if (check == false)
		return false;

	return true;

}

// "���� ģ����û ���" ��Ŷ ó��
bool Network_Req_ReplytList(SOCKET sock, char* Packet)
{
	// 1. ���ڷ� ���� sock�� �̿��� Accept�� ȸ������ �˾ƿ���
	stAcceptUser* NowAccount = ClientSearch_AcceptList(sock);

	// 2. ���� ó��
	// ���� ������ ���(���⼭ ���� ������, Accept�� ���� ����), ������ ���´�.
	// �̷���Ȳ�� �߻������� �𸣰�����..Ȥ�ø𸣴� ó����
	if (NowAccount == nullptr)
	{
		_tprintf(_T("Network_Req_ReplytList(). Accept���� ���� ������ [���� ģ����û ���] ��û. (SOCKET : %lld). ���� ����\n"), sock);
		return false;
	}

	// 3. "���� ģ����û ��� ���" ��Ŷ ����
	CProtocolBuff header(dfHEADER_SIZE);
	CProtocolBuff Payload;

	stAccount* NowUser = ClientSearch_AccountList(NowAccount->m_AccountNo);

	// ���� �α����� ������ �ƴ϶��, ���� ��û �� 0 ����
	UINT ReplyCount;
	if (NowUser == nullptr)
		ReplyCount = 0;
	else
		ReplyCount = NowUser->m_ReplyCount;

	Network_Res_ReplytList((char*)&header, (char*)&Payload, NowAccount->m_AccountNo, ReplyCount);

	// 4. ���� ��Ŷ�� �ش� ������, SendBuff�� �ֱ�
	bool check = SendPacket(sock, &header, &Payload);
	if (check == false)
		return false;

	return true;
}

// "ģ������ ��û" ��Ŷ ó��
bool Network_Req_FriendRemove(SOCKET sock, char* Packet)
{
	// 1. ���ڷ� ���� sock�� �̿��� Accept�� ȸ������ �˾ƿ���
	stAcceptUser* AcceptUser = ClientSearch_AcceptList(sock);

	// 2. ���� ó��
	// ���� ������ ���(���⼭ ���� ������, Accept�� ���� ����), ������ ���´�.
	// �̷���Ȳ�� �߻������� �𸣰�����..Ȥ�ø𸣴� ó����
	if (AcceptUser == nullptr)
	{
		_tprintf(_T("Network_Req_FriendRemove(). Accept���� ���� ������ [ģ�� ����] ��û. (SOCKET : %lld). ���� ����\n"), sock);
		return false;
	}

	// 3. ģ���� ���� ȸ���� AccountNo
	CProtocolBuff* RemovePacekt = (CProtocolBuff*)Packet;

	UINT64 RemoveAccountNo;
	*RemovePacekt >> RemoveAccountNo;

	// 4. ģ���� ���� ȸ���� �����ϴ��� üũ
	stAccount* NowAccount = ClientSearch_AccountList(RemoveAccountNo);

	// 5. ģ���� ���� ȸ���� �� ģ������ üũ
	// ���� ȸ���� ���� ģ���� ���, Frienditor�� end()�� �ƴ�
	map<UINT64, stFromTo*>::iterator Frienditor;
	for (Frienditor = map_FriendList.begin(); Frienditor != map_FriendList.end(); ++Frienditor)
		if (Frienditor->second->m_FromAccount == AcceptUser->m_AccountNo)	// ���� ģ���� �������� ã�Ұ�			
			if (Frienditor->second->m_ToAccount == RemoveAccountNo)			// �� �������� �̹��� ģ���� ���� ȸ���̶��, ���� ģ���� ������ ��. for�� ����
				break;
	

	// 6. ��� ����
	BYTE result;

	// 6-1. Accept ������ �α��� ���� �ƴ϶��, �α��� �� �ƴ϶�� ��� ����
	if (AcceptUser->m_AccountNo == 0)
		result = df_RESULT_FRIEND_REMOVE_FAIL;

	// 6-2. ģ�� ��û�� ���� ȸ���� ���ų�, �� �ڽ��� ������ ���� ģ����û(?)�� �����ϴ� ���, ȸ�� ���ٰ� ��� ����
	// �� �ڽ��� ������ ģ����û�� ���� ���� ���ʿ� ģ����û�Ҷ� ������, ������ �߻����� �𸣴� ���⼭�� ����ó��.
	else if (NowAccount == nullptr || RemoveAccountNo == AcceptUser->m_AccountNo)
		result = df_RESULT_FRIEND_REMOVE_NOTFRIEND;

	// 6-3. ģ���� �ƴ� ������ ģ�����⸦ �Ϸ��� �ϴ� ���, ȸ�� ���ٰ� ����
	else if (Frienditor == map_FriendList.end())
		result = df_RESULT_FRIEND_REMOVE_NOTFRIEND;

	// 6-4. ��� �ش���� �ʴ´ٸ�, ���� ��� ����
	// �׸���, [ģ�� ���]���� ���� ���踦 �����Ѵ�.
	else
	{
		result = df_RESULT_FRIEND_REMOVE_OK;
		stAccount* MyAccount = ClientSearch_AccountList(AcceptUser->m_AccountNo);

		// [ģ�� ���]���� ���� ���� ����
		map <UINT64, stFromTo*>::iterator itor;
		itor = map_FriendList.begin();

		while (itor != map_FriendList.end())
		{
			// �� ģ���� ã�Ҵµ�
			if (itor->second->m_FromAccount == AcceptUser->m_AccountNo)
			{				
				if (itor->second->m_ToAccount == RemoveAccountNo)			// �� ģ���� �̹��� ������ ģ�����
				{
					itor = map_FriendList.erase(itor);						// �ش� ���� ����
					MyAccount->m_FriendCount--;								// �׸��� ���� ģ�� �� 1 ����
				}
											
				else														 // ��������, ���� �̹��� ������ ģ���� �ƴ϶��
					++itor;													// ���ͷ����� �������� �̵�
			}
 
			// ����� ģ���� ã�Ҵµ�
			else if (itor->second->m_FromAccount == RemoveAccountNo)
			{				
				if (itor->second->m_ToAccount == AcceptUser->m_AccountNo)		// �װ� �����		
				{
					itor = map_FriendList.erase(itor);							// �ش� ���� ����
					NowAccount->m_FriendCount--;								// �׸��� ����� ģ�� �� 1 ����
				}

				else															// �������� ���� �ƴ϶��
					++itor;														// ���ͷ����� �������� �̵�
			}

			// itor->first�� �����ƴϰ� ��뵵 �ƴ϶�� �ٷ� ���� ���ͷ����ͷ� �̵�
			else
				++itor;
		}		
	}

	// 6. ""ģ������ ��û ���" ��Ŷ ����
	CProtocolBuff header(dfHEADER_SIZE);
	CProtocolBuff Payload;

	Network_Res_FriendRemove((char*)&header, (char*)&Payload, RemoveAccountNo, result);

	// 7. ���� ��Ŷ�� �ش� ������, SendBuff�� �ֱ�
	bool check = SendPacket(sock, &header, &Payload);
	if (check == false)
		return false;

	return true;
}

// "ģ�� ��û" ��Ŷ ó��
bool Network_Req_FriendRequest(SOCKET sock, char* Packet)
{
	// 1. ���ڷ� ���� sock�� �̿��� ģ�� ��û�� ���� ���� �˾ƿ���
	stAcceptUser* AcceptUser = ClientSearch_AcceptList(sock);

	// 2. ���� ó��
	// ���� ������ ���(���⼭ ���� ������, Accept�� ���� ����), ������ ���´�.
	// �̷���Ȳ�� �߻������� �𸣰�����..Ȥ�ø𸣴� ó����
	if (AcceptUser == nullptr)
	{
		_tprintf(_T("Network_Req_FriendRequest(). Accept���� ���� ������ ģ����û�� ����. (SOCKET : %lld). ���� ����\n"), sock);
		return false;
	}

	// 3. ģ�� ��û�� ���� ȸ���� AccountNo �˾Ƴ���
	CProtocolBuff* RequestPacekt = (CProtocolBuff*)Packet;

	UINT64 RequestAccountNo;
	*RequestPacekt >> RequestAccountNo;

	// 4. ģ�� ��û�� ���� ȸ���� �����ϴ��� üũ
	stAccount* NowAccount = ClientSearch_AccountList(RequestAccountNo);

	// 5. �̹� ģ����û�� ���� ȸ������ üũ
	// �̹� ģ����û�� ���� ȸ���̶��, FriendRequestitor�� end�� �ƴϴ�.
	map<UINT64, stFromTo*>::iterator FriendRequestitor;
	for (FriendRequestitor = map_FriendRequestList.begin(); FriendRequestitor != map_FriendRequestList.end(); ++FriendRequestitor)
		if (FriendRequestitor->second->m_FromAccount == AcceptUser->m_AccountNo)			// ���� ���� ģ����û�� ã�Ҵٰ�
			if (FriendRequestitor->second->m_ToAccount == RequestAccountNo)					// ��û�� ���� ������ �̹��� ģ����û�� �������� ������� for�� ����
				break;
	

	// 6. �̹� ģ���� ȸ������ üũ
	// �̹� ģ���� ȸ���̶��, Frienditor�� end�� �ƴϴ�
	map<UINT64, stFromTo*>::iterator Frienditor;
	for (Frienditor = map_FriendList.begin(); Frienditor != map_FriendList.end(); ++Frienditor)
		if (Frienditor->second->m_FromAccount == AcceptUser->m_AccountNo)	// �� ģ���� ã�Ҵµ�
			if (Frienditor->second->m_ToAccount == RequestAccountNo)		// �� ģ����, �̹��� ģ����û�� �������� ������� for�� ����
				break;

	// 7. ��� ����
	BYTE result;

	// 7-1. Accept ������ �α��� ���� �ƴ϶��, �α��� �� �ƴ϶�� ��� ����
	if (AcceptUser->m_AccountNo == 0)
		result = df_RESULT_FRIEND_REQUEST_AREADY;

	// 7-2. �α����� �Ǿ������� ģ�� ��û�� ���� ȸ���� ���ų�, �� �ڽſ��� ģ����û�� ���� ���, ȸ�� ���ٰ� ��� ����
	else if (NowAccount == nullptr || RequestAccountNo == AcceptUser->m_AccountNo)
		result = df_RESULT_FRIEND_REQUEST_NOTFOUND;

	// 7-3. �̹� ģ�� ��û�� ���� �������� �� ģ����û�� ���� ���, ���� �޽��� ����
	else if(FriendRequestitor != map_FriendRequestList.end())
		result = df_RESULT_FRIEND_REQUEST_NOTFOUND;

	// 7-4. �̹� ģ���� �������� �� ģ����û�� ���� ���, ���� �޽��� ����
	else if (Frienditor != map_FriendList.end())
		result = df_RESULT_FRIEND_REQUEST_NOTFOUND;
	
	// 7-5. ��� �ش���� �ʴ´ٸ�, ���� ��� ����
	// �׸���, [���� ģ����û ���], [���� ģ����û ���]�� �߰�
	else
	{
		result = df_RESULT_FRIEND_REQUEST_OK;

		typedef pair<UINT64, stFromTo*> Itn_pair;

		// [���� ģ����û ���]�� �߰�
		stFromTo* MyRequest = new stFromTo;
		MyRequest->m_FromAccount = AcceptUser->m_AccountNo;
		MyRequest->m_ToAccount = RequestAccountNo;
		MyRequest->m_Time = 0;
		map_FriendRequestList.insert(Itn_pair(g_uiRequestIndex, MyRequest));
		g_uiRequestIndex++;

		// "��"�� ���� ģ����û ī��Ʈ ����
		stAccount* MyAccount = ClientSearch_AccountList(AcceptUser->m_AccountNo);
		MyAccount->m_RequestCount++;

		// [���� ģ����û ���]�� �߰�
		stFromTo* OtherRequest = new stFromTo;
		OtherRequest->m_FromAccount = RequestAccountNo;
		OtherRequest->m_ToAccount = AcceptUser->m_AccountNo;
		OtherRequest->m_Time = 0;
		map_FriendReplyList.insert(Itn_pair(g_uiRequestIndex, OtherRequest));
		g_uiRequestIndex++;

		// "���"�� ���� ģ����û ī��Ʈ ����
		NowAccount->m_ReplyCount++;
	}

	// 8. "ģ�� ��û ���" ��Ŷ ����
	CProtocolBuff header(dfHEADER_SIZE);
	CProtocolBuff Payload;

	Network_Res_FriendRequest((char*)&header, (char*)&Payload, RequestAccountNo, result);

	// 8. ���� ��Ŷ�� �ش� ������, SendBuff�� �ֱ�
	bool check = SendPacket(sock, &header, &Payload);
	if (check == false)
		return false;

	return true;
}

// "ģ����û ���" ��Ŷ ó��
bool Network_Req_RequestCancel(SOCKET sock, char* Packet)
{
	// 1. ���ڷ� ���� sock�� �̿��� ģ�� ��û�� ���� ���� �˾ƿ���
	stAcceptUser* AcceptUser = ClientSearch_AcceptList(sock);

	// 2. ���� ó��
	// ���� ������ ���(���⼭ ���� ������, Accept�� ���� ����), ������ ���´�.
	// �̷���Ȳ�� �߻������� �𸣰�����..Ȥ�ø𸣴� ó����
	if (AcceptUser == nullptr)
	{
		_tprintf(_T("Network_Req_RequestCancel(). Accept���� ���� ������ ģ����û ��� ����. (SOCKET : %lld). ���� ����\n"), sock);
		return false;
	}

	// 3. ���� ģ�� ��û�� ����� ȸ���� AccountNo �˾Ƴ���
	CProtocolBuff* CancelPacekt = (CProtocolBuff*)Packet;

	UINT64 CancelAccountNo;
	*CancelPacekt >> CancelAccountNo;

	// 4. ���� ģ����û�� ���´� ������ ���� ȸ������ �����ϴ��� üũ
	// �α����� ���..ū �ǹ̰� ���°� ���� �ѵ� �׳� �غ�.
	stAccount* NowAccount = ClientSearch_AccountList(CancelAccountNo);

	// 5. �̹��� ����Ϸ��� ��û�� [ģ����û ���]�� �����ϴ��� üũ
	// ��Ͽ� �����ϸ�, Requestitor�� end()�� �ƴ�.
	map<UINT64, stFromTo*>::iterator Requestitor;
	for (Requestitor = map_FriendRequestList.begin(); Requestitor != map_FriendRequestList.end(); ++Requestitor)
		if (Requestitor->second->m_FromAccount == AcceptUser->m_AccountNo)			// ���� ���� ��û�� ã�Ұ�
			if (Requestitor->second->m_ToAccount == CancelAccountNo)				// ��밡 �̹��� �����Ϸ��� ����No���, [ģ����û ���]�� �����ϴ� ��. break;
				break;

	// 6. Ȥ�� �̹��� ����Ϸ��� ������ �̹� ģ���� �Ǿ����� üũ
	// ���� �߻����� ������.. Ȥ�� ��Ŷ ó���� �����ٸ� �̹� ģ���� �Ǿ��ִ� ��쵵 ������.
	// ģ�� ��Ͽ� �����ϸ�, Friendito�� end()�� �ƴ�
	map<UINT64, stFromTo*>::iterator Frienditor;
	for (Frienditor = map_FriendList.begin(); Frienditor != map_FriendList.end(); ++Frienditor)
		if (Frienditor->second->m_FromAccount == AcceptUser->m_AccountNo)				// �� ģ���� ã�Ұ�
			if (Frienditor->second->m_ToAccount == CancelAccountNo)						// ��밡 �̹��� �����Ϸ��� �������, [ģ�� ���]�� �ִ� ��. break;
				break;


	// 7. ��� ����
	BYTE result;

	// 7-1. Accept ������ �α��� ���� �ƴ϶��, Fail��� ����
	if (AcceptUser->m_AccountNo == 0)
		result = df_RESULT_FRIEND_CANCEL_FAIL;

	// 7-2. �α����� �Ǿ������� ģ�� ��û�� �޾Ҵ� ȸ���� ���ų�, �� �ڽſ��� ����(?) ģ����û�� ����Ϸ��� ���, ȸ�� ���ٰ� ��� ����
	else if (NowAccount == nullptr || CancelAccountNo == AcceptUser->m_AccountNo)
		result = df_RESULT_FRIEND_REQUEST_NOTFOUND;

	// 7-3. �̹��� ����Ϸ��� ��û�� [ģ����û ���]�� �����ϴ��� �ʴ´ٸ�, ȸ�� ���ٰ� ��� ����
	else if (Requestitor == map_FriendRequestList.end())
		result = df_RESULT_FRIEND_REQUEST_NOTFOUND;

	// 7-4. �� ��Ȳ�� �ش���� �ʴ´ٸ�, ���� ��� ����
	else
	{
		stAccount* MyAccount = ClientSearch_AccountList(AcceptUser->m_AccountNo);

		// 1. �ٵ� ����, ���� ���װ� �߻��ؼ� �̹� ģ���� �������� ���´� ��û�� ���� �����ִٸ�, Ŭ�󿡴� ������ ������ [���� ģ����û ���]���� �����Ѵ�.
		if (Frienditor != map_FriendList.end())
			result = df_RESULT_FRIEND_CANCEL_FAIL;

		// �̰� �ƴ϶�� ������ ���� ��� ����
		else
			result = df_RESULT_FRIEND_CANCEL_OK;

		// 2. [������û ���]���� ��û ���� ("��"�� ���� ��û�� ����)
		map<UINT64, stFromTo*>::iterator itor;
		itor = map_FriendRequestList.begin();

		while (itor != map_FriendRequestList.end())
		{
			// ���� ���� ��û�� ã�Ҵµ�
			if (itor->second->m_FromAccount == AcceptUser->m_AccountNo)
			{
				// ��밡 �̹��� ��û�� ����ϴ� �����
				if (itor->second->m_ToAccount == CancelAccountNo)
				{
					// �ش� ��û ����
					itor = map_FriendRequestList.erase(itor);

					// �׸��� ���� ���� ��û �� 1����
					MyAccount->m_RequestCount--;
				}

				// �̹� ��� ��밡 �ƴ϶�� itor ����
				else
					++itor;
			}

			// ���� ���� ��û�� �ƴϸ�, itor ����
			else
				++itor;
		}

		// 3. [������û ���]���� ��û ���� ("���"�� ���� ��û�� ����)
		itor = map_FriendReplyList.begin();
		while (itor != map_FriendReplyList.end())
		{
			// ��밡 ���� ��û�� ã�Ҵµ�
			if (itor->second->m_FromAccount == CancelAccountNo)
			{
				// ���� ���´� ��û�̶��
				if (itor->second->m_ToAccount == AcceptUser->m_AccountNo)
				{
					// �ش� ��û ����
					itor = map_FriendReplyList.erase(itor);

					// ����� ���� ��û �� 1����
					NowAccount->m_ReplyCount--;
				}

				// ���� ���´� ��û�� �ƴϸ� itor ����
				else
					++itor;
			}

			// ��밡 ���� ��û�� �ƴϸ�, itor����
			else
				++itor;
		}

	}

	// 6. "ģ����û ��� ���" ��Ŷ ����
	CProtocolBuff header(dfHEADER_SIZE);
	CProtocolBuff Payload;

	Network_Res_RequestCancel((char*)&header, (char*)&Payload, CancelAccountNo, result);

	// 7. ���� ��Ŷ�� �ش� ������, SendBuff�� �ֱ�
	bool check = SendPacket(sock, &header, &Payload);
	if (check == false)
		return false;

	return true;

}

// "������û �ź�" ��Ŷ ó��
bool Network_Req_RequestDeny(SOCKET sock, char* Packet)
{
	// 1. ���ڷ� ���� sock�� �̿��� ģ�� ��û�� ���� ���� �˾ƿ���
	stAcceptUser* AcceptUser = ClientSearch_AcceptList(sock);

	// 2. ���� ó��
	// ���� ������ ���(���⼭ ���� ������, Accept�� ���� ����), ������ ���´�.
	// �̷���Ȳ�� �߻������� �𸣰�����..Ȥ�ø𸣴� ó����
	if (AcceptUser == nullptr)
	{
		_tprintf(_T("Network_Req_RequestDeny(). Accept���� ���� ������ ������û �ź� ����. (SOCKET : %lld). ���� ����\n"), sock);
		return false;
	}

	// 3. ���� ��û�� ������ ȸ���� AccountNo �˾Ƴ���
	CProtocolBuff* DenyPacekt = (CProtocolBuff*)Packet;

	UINT64 DenyAccountNo;
	*DenyPacekt >> DenyAccountNo;

	// 4. ������ ģ����û�� ���´� ������ ���� ȸ������ �����ϴ��� üũ
	// ȸ�� Ż�� ���..ū �ǹ̰� ���°� ���� �ѵ� �׳� �غ�.
	stAccount* NowAccount = ClientSearch_AccountList(DenyAccountNo);

	// 5. �̹��� �����Ϸ��� ��û�� [������û ���]�� �����ϴ��� üũ
	// ��Ͽ� �����ϸ�, Denyitor�� end()�� �ƴ�.
	map<UINT64, stFromTo*>::iterator Denyitor;
	for (Denyitor = map_FriendReplyList.begin(); Denyitor != map_FriendReplyList.end(); ++Denyitor)
		if (Denyitor->second->m_FromAccount == AcceptUser->m_AccountNo)			// ���� ���� ��û�� ã�Ұ�
			if (Denyitor->second->m_ToAccount == DenyAccountNo)					// ��밡 �̹��� ��û�� �����Ϸ��� ����No���, [������û ���]�� �����ϴ� ��. break;
				break;

	// 6. Ȥ�� �̹��� ����Ϸ��� ������ �̹� ģ���� �Ǿ����� üũ
	// ���� �߻����� ������.. Ȥ�� ��Ŷ ó���� �����ٸ� �̹� ģ���� �Ǿ��ִ� ��쵵 ������.
	// ģ�� ��Ͽ� �����ϸ�, Friendito�� end()�� �ƴ�
	map<UINT64, stFromTo*>::iterator Frienditor;
	for (Frienditor = map_FriendList.begin(); Frienditor != map_FriendList.end(); ++Frienditor)
		if (Frienditor->second->m_FromAccount == AcceptUser->m_AccountNo)				// �� ģ���� ã�Ұ�
			if (Frienditor->second->m_ToAccount == DenyAccountNo)						// ��밡 �̹��� �����Ϸ��� �������, [ģ�� ���]�� �ִ� ��. break;
				break;

	// 7. ��� ����
	BYTE result;

	// 7-1. Accept ������ �α��� ���� �ƴ϶��, Fail��� ����
	if (AcceptUser->m_AccountNo == 0)
		result = df_RESULT_FRIEND_DENY_FAIL;

	// 7-2. �α����� �Ǿ������� ģ�� ��û�� �޾Ҵ� ȸ���� ���ų�, ���� ������ ����(?) ģ�� ��û�� �����Ϸ��� ���, ȸ�� ���ٰ� ��� ����
	else if (NowAccount == nullptr || DenyAccountNo == AcceptUser->m_AccountNo)
		result = df_RESULT_FRIEND_DENY_NOTFRIEND;

	// 7-3. �̹��� �����Ϸ��� ��û�� [������û ���]�� �����ϴ��� �ʴ´ٸ�, ȸ�� ���ٰ� ��� ����
	else if (Denyitor == map_FriendRequestList.end())
		result = df_RESULT_FRIEND_DENY_NOTFRIEND;

	// 7-4. �� ��Ȳ�� �ش���� �ʴ´ٸ�, ���� ��� ����
	else
	{
		stAccount* MyAccount = ClientSearch_AccountList(AcceptUser->m_AccountNo);

		// 1. �ٵ� ����, ���� ���װ� �߻��ؼ� �̹� ģ���� �������� ���´� ��û�� ���� �����־��ٸ�, Ŭ�󿡴� ������ ������ [���� ģ����û ���]���� �����Ѵ�.
		if (Frienditor != map_FriendList.end())
			result = df_RESULT_FRIEND_DENY_FAIL;

		// �̰� �ƴ϶�� ������ ���� ��� ����
		else
			result = df_RESULT_FRIEND_DENY_OK;

		// 3. [������û ���]���� ��û ���� ("��"�� ���� ��û�� ����)
		map<UINT64, stFromTo*>::iterator itor;
		itor = map_FriendReplyList.begin();

		while (itor != map_FriendReplyList.end())
		{
			// ���� ���� ��û�� ã�Ҵµ�
			if (itor->second->m_FromAccount == AcceptUser->m_AccountNo)
			{
				// �̹��� �����ϴ� ��밡 ���´� ��û�̶��
				if (itor->second->m_ToAccount == DenyAccountNo)
				{
					// �ش� ��û ����
					itor = map_FriendRequestList.erase(itor);

					// ���� ���� ��û �� 1����
					MyAccount->m_ReplyCount--;
				}

				// ��밡 ���´� ��û�� �ƴϸ� itor ����
				else
					++itor;
			}

			// ���� ���� ��û�� �ƴϸ�, itor����
			else
				++itor;
		}


		// 2. [������û ���]���� ��û ���� ("���"�� ���� ��û�� ����)
		itor = map_FriendRequestList.begin();

		while (itor != map_FriendRequestList.end())
		{
			// ��밡 ���� ��û�� ã�Ҵµ�
			if (itor->second->m_FromAccount == DenyAccountNo)
			{
				// ������ ���� ��û�̶��
				if (itor->second->m_ToAccount == AcceptUser->m_AccountNo)
				{
					// �ش� ��û ����
					itor = map_FriendRequestList.erase(itor);

					// �׸��� ����� ���� ��û �� 1����
					NowAccount->m_RequestCount--;
				}

				// ������ ���� ��û�� �ƴ϶�� itor ����
				else
					++itor;
			}

			// ��밡 ���� ��û�� �ƴ϶��, itor ����
			else
				++itor;
		}		
	}

	// 7. "������û ��� ���" ��Ŷ ����
	CProtocolBuff header(dfHEADER_SIZE);
	CProtocolBuff Payload;

	Network_Res_RequestDeny((char*)&header, (char*)&Payload, DenyAccountNo, result);

	// 8. ���� ��Ŷ�� �ش� ������, SendBuff�� �ֱ�
	bool check = SendPacket(sock, &header, &Payload);
	if (check == false)
		return false;

	return true;
}

// "ģ����û ����" ��Ŷ ó��
bool Network_Req_FriendAgree(SOCKET sock, char* Packet)
{
	// 1. ���ڷ� ���� sock�� �̿��� ģ�� ��û�� ���� ���� �˾ƿ���
	stAcceptUser* AcceptUser = ClientSearch_AcceptList(sock);

	// 2. ���� ó��
	// ���� ������ ���(���⼭ ���� ������, Accept�� ���� ����), ������ ���´�.
	// �̷���Ȳ�� �߻������� �𸣰�����..Ȥ�ø𸣴� ó����
	if (AcceptUser == nullptr)
	{
		_tprintf(_T("Network_Req_Login(). Accept���� ���� ������ �α��ο�û. (SOCKET : %lld). ���� ����\n"), sock);
		return false;
	}

	// 3. ģ����û�� ������ ȸ���� AccountNo �˾Ƴ���
	CProtocolBuff* AgreePacekt = (CProtocolBuff*)Packet;

	UINT64 AgreePacektAccountNo;
	*AgreePacekt >> AgreePacektAccountNo;

	// 4. ������ ģ�� ��û�� ���� ȸ���� �����ϴ��� üũ (�߻��� ���� ������ ������.. Ȥ�ø��� �ص�)
	stAccount* NowAccount = ClientSearch_AccountList(AgreePacektAccountNo);

	// 5. ������ �� ģ����û�� �����Ѱ� �´��� üũ
	// �´ٸ�, ReplyListitor�� end�� �ƴ�.
	map<UINT64, stFromTo*>::iterator ReplyListitor;
	for (ReplyListitor = map_FriendReplyList.begin(); ReplyListitor != map_FriendReplyList.end(); ++ReplyListitor)
		if (ReplyListitor->second->m_FromAccount == AcceptUser->m_AccountNo)	// ���� ���� ��û�߿�
			if (ReplyListitor->second->m_ToAccount == AgreePacektAccountNo)		// AgreePacektAccountNo ������ ���� ��û�� ������, ������ �� ģ����û�� ���� ������ �Ѱ� �´°�. for�� break;
				break;


	// 6. ��� ����
	BYTE result;

	// 6-1. Accept ������ �α��� ���� �ƴ϶��, Fail ��� ����
	if (AcceptUser->m_AccountNo == 0)
		result = df_RESULT_FRIEND_AGREE_FAIL;

	// 6-2. Ȥ��, �α����� �Ǿ������� ģ����û ������ ȸ���� ���� ȸ���̰ų�, �� �ڽ���(?) ������ ģ����û�� ���� ���, ȸ�� ���ٰ� ��� ����
	// �� �ڽ��� ������ ģ����û�� ���� ���� ���ʿ� ģ����û�Ҷ� ������, ������ �߻����� �𸣴� ���⼭�� ����ó��.
	else if (NowAccount == nullptr || AgreePacektAccountNo == AcceptUser->m_AccountNo)
		result = df_RESULT_FRIEND_AGREE_NOTFRIEND;

	// 6-3. ������ �� ģ����û�� �����Ѱ� �ƴ϶��, Fail��� ����
	else if(ReplyListitor == map_FriendReplyList.end())
		result = df_RESULT_FRIEND_AGREE_FAIL;

	// 6-3. ��� �ش���� �ʴ´ٸ�, ���� ��� ����
	// �׸���, [ģ�� ���]�� �߰�, [���� ģ����û ���]���� ����, [���� ģ����û ���]���� ����
	else
	{
		result = df_RESULT_FRIEND_AGREE_OK;
		stAccount* MyAccount = ClientSearch_AccountList(AcceptUser->m_AccountNo);

		// 1. [ģ�� ���]�� �߰�
		// ������ ����, [1. A->B�� ģ����. 2. B->A�� ģ����] �� �� �߰��ؾ���
		typedef pair<UINT64, stFromTo*> Itn_pair;

		stFromTo* MyFriend = new stFromTo;
		MyFriend->m_FromAccount = AcceptUser->m_AccountNo;
		MyFriend->m_ToAccount = AgreePacektAccountNo;
		MyFriend->m_Time = 0;
		map_FriendList.insert(Itn_pair(g_uiFriendIndex, MyFriend));
		g_uiFriendIndex++;

		// ���� ģ�� �� ����
		MyAccount->m_FriendCount++;

		stFromTo* OtherFriend = new stFromTo;
		OtherFriend->m_FromAccount = AgreePacektAccountNo;
		OtherFriend->m_ToAccount = AcceptUser->m_AccountNo;
		OtherFriend->m_Time = 0;
		map_FriendList.insert(Itn_pair(g_uiFriendIndex, OtherFriend));
		g_uiFriendIndex++;

		// ����� ģ�� �� ����
		NowAccount->m_FriendCount++;

		// 2. [���� ģ����û ���]���� ����
		map<UINT64, stFromTo*>::iterator itor;
		itor = map_FriendReplyList.begin();

		while (itor != map_FriendReplyList.end())
		{
			// ���� ���� ��û�� ã�Ҵµ�
			if (itor->second->m_FromAccount == AcceptUser->m_AccountNo)
			{
				// ��밡, �̹��� ģ���� �߰��� �������
				if (itor->second->m_ToAccount == AgreePacektAccountNo)
				{
					// �ش� ���� [���� ģ����û ���]���� ����.
					itor = map_FriendReplyList.erase(itor);

					// �� ������û �� ����
					MyAccount->m_ReplyCount--;
				}

				// �̹��� �߰��� ������ �ƴ϶��, itor �������� ����.
				else
					itor++;

			}

			// ��밡 ���� ��û�� ã�Ҵµ�
			else if (itor->second->m_FromAccount == AgreePacektAccountNo)
			{
				// ���� ���� ��û�̾��ٸ�
				if (itor->second->m_ToAccount == AcceptUser->m_AccountNo)
				{
					// �ش� ���� [���� ģ����û ���]���� ����.
					itor = map_FriendReplyList.erase(itor);

					// ����� ������û �� ����
					NowAccount->m_ReplyCount--;
				}

				// �� �ƴ� �ٸ������ ���� ��û�̾��ٸ� itor �������� ����
				else
					itor++;
			}

			// ���� ���� ��û, ��밡 ���� ��û �� �� �ƴ϶�� itor �������� ����
			else
				itor++;
		}

		// 3. [���� ģ����û ���]���� ����
		itor = map_FriendRequestList.begin();
		while (itor != map_FriendRequestList.end())
		{
			// ���� ���� ��û�� ã�Ҵµ�
			if (itor->second->m_FromAccount == AcceptUser->m_AccountNo)
			{
				// ��밡, �̹��� ģ���� �߰��� �������
				if (itor->second->m_ToAccount == AgreePacektAccountNo)
				{
					// �ش� ���� [���� ģ����û ���]���� ����.
					itor = map_FriendRequestList.erase(itor);

					// ���� ���� ��û �� ����
					MyAccount->m_RequestCount--;
				}

				// �̹��� �߰��� ������ �ƴ϶��, itor �������� ����.
				else
					itor++;
			}

			// ��밡 ���� ��û�� ã�Ҵµ�
			else if (itor->second->m_FromAccount == AgreePacektAccountNo)
			{
				// ��밡, �����
				if (itor->second->m_ToAccount == AcceptUser->m_AccountNo)
				{
					// �ش� ���� [���� ģ����û ���]���� ����.
					itor = map_FriendRequestList.erase(itor);

					// ����� ���� ��û �� ����
					NowAccount->m_RequestCount--;
				}

				// ��밡 ���� �ƴ϶��, itor �������� ����.
				else
					itor++;
			}

			// ���� ���� ��û, ��밡 ���� ��û �� �� �ƴ϶�� itor �������� ����
			else
				itor++;
		}
	}

	// 7. "ģ����û ���� ���" ��Ŷ ����
	CProtocolBuff header(dfHEADER_SIZE);
	CProtocolBuff Payload;

	Network_Res_FriendAgree((char*)&header, (char*)&Payload, AgreePacektAccountNo, result);

	// 8. ���� ��Ŷ�� �ش� ������, SendBuff�� �ֱ�
	bool check = SendPacket(sock, &header, &Payload);
	if (check == false)
		return false;

	return true;
}

// ��Ʈ���� �׽�Ʈ�� ��Ŷ ó��
bool Network_Req_StressTest(SOCKET sock, char* Packet)
{
	// 1. ���ڷ� ���� sock�� �̿��� ������(Accept)�� �������� üũ
	stAcceptUser* AcceptUser = ClientSearch_AcceptList(sock);

	// 2. ���� ó��
	// ���� ������ ���(���⼭ ���� ������, Accept�� ���� ����), ������ ���´�.
	// �̷���Ȳ�� �߻������� �𸣰�����..Ȥ�ø𸣴� ó����
	if (AcceptUser == nullptr)
	{
		_tprintf(_T("Network_Req_StressTest(). Accept���� ���� ������ ��Ʈ���� �׽�Ʈ ��Ŷ ����. (SOCKET : %lld). ���� ����\n"), sock);
		return false;
	}

	// 3. ���� ���ڿ� ������
	CProtocolBuff* StressTestBuff = (CProtocolBuff*)Packet;

	// �ϴ� ���ڿ� ������ ������ (����Ʈ ����)
	WORD wStringSize;
	*StressTestBuff >> wStringSize;

	// ������ ��ŭ ���ڿ� ��������
	TCHAR* TestString = new TCHAR[(wStringSize / 2) + 1];
	for(int i=0; i<wStringSize/2; ++i)
		*StressTestBuff >> TestString[i];
	
	// 3. "��Ʈ���� �׽�Ʈ�� ��Ŷ ���" ����
	CProtocolBuff header(dfHEADER_SIZE);
	CProtocolBuff Payload;

	Network_Res_StressTest((char*)&header, (char*)&Payload, wStringSize, TestString);
	
	// 4. ���� ��Ŷ�� �ش� ������, SendBuff�� �ֱ�
	bool check = SendPacket(sock, &header, &Payload);
	if (check == false)
		return false;

	delete TestString;

	return true;
}




/////////////////////////
// ��Ŷ ���� �Լ�
/////////////////////////
// "ȸ������ ��û(ȸ����� �߰�) ���" ��Ŷ ����
void Network_Res_AccountAdd(char* header, char* Packet, UINT64 AccountNo)
{
	// 1. ���̷ε� ����
	CProtocolBuff* Payload = (CProtocolBuff*)Packet;
	*Payload << AccountNo;

	// 2. ��� ����
	CreateHeader((CProtocolBuff*)header, df_RES_ACCOUNT_ADD, Payload->GetUseSize());
}

// "�α��� ��û ���" ��Ŷ ����
void Network_Res_Login(char* header, char* Packet, UINT64 AccountNo, TCHAR* Nick)
{
	// 1. ���̷ε� ����
	CProtocolBuff* Payload = (CProtocolBuff*)Packet;
	*Payload << AccountNo;

	for(int i=0; i<dfNICK_MAX_LEN; ++i)
		*Payload << Nick[i];

	// 2. ��� ����
	CreateHeader((CProtocolBuff*)header, df_RES_LOGIN, Payload->GetUseSize());
}

// "ȸ�� ��� ��û ���" ��Ŷ ����
void Network_Res_AccountList(char* header, char* Packet)
{
	// 1. ���̷ε� ����
	CProtocolBuff* Payload = (CProtocolBuff*)Packet;

	// ���� ���� ȸ�� �� �˾ƿ���
	UINT AccountCount = (UINT)map_AccountList.size();
	*Payload << AccountCount;

	// ���� �� ��� ������ AccountNo, �г��� �߰�
	map<UINT64, stAccount*>::iterator itor;
	for (itor = map_AccountList.begin(); itor != map_AccountList.end(); ++itor)
	{
		// AccountNo �߰�
		*Payload << itor->first;

		// �г��� �߰�
		for (int i = 0; i < dfNICK_MAX_LEN; ++i)
			*Payload << itor->second->m_NickName[i];
	}

	// 2. ��� ����
	CreateHeader((CProtocolBuff*)header, df_RES_ACCOUNT_LIST, Payload->GetUseSize());
}

// "ģ�� ��� ��û ���" ��Ŷ ����
void Network_Res_FriendList(char* header, char* Packet, UINT64 AccountNo, UINT FriendCount)
{
	// 1. ���̷ε� ����
	CProtocolBuff* Payload = (CProtocolBuff*)Packet;

	// 2. �� ģ�� �� ����
	*Payload << FriendCount;

	// �� ģ�� ���� 0�� �ƴ� ���,
	// ó������ ������ [ģ�� ���]�� ���鼭, ���� ģ���� ������ AccountNo, �г��� ����
	map <UINT64, stFromTo*>::iterator itor;
	if (FriendCount != 0)
	{
		for (itor = map_FriendList.begin(); itor != map_FriendList.end(); ++itor)
		{
			// �� ģ���� ã������
			if (itor->second->m_FromAccount == AccountNo)
			{
				// �� ģ���� ������ �����´�.
				map <UINT64, stAccount*>::iterator Accountitor;
				Accountitor = map_AccountList.find(itor->second->m_ToAccount);

				// ����� AccountNo, �г��� ����
				*Payload << Accountitor->second->m_AccountNo;

				for (int i = 0; i<dfNICK_MAX_LEN; ++i)
					*Payload << Accountitor->second->m_NickName[i];

			}
		}
	}

	// 2. ��� ����
	CreateHeader((CProtocolBuff*)header, df_RES_FRIEND_LIST, Payload->GetUseSize());
}

// "���� ģ����û ��� ���" ��Ŷ ����
void Network_Res_RequestList(char* header, char* Packet, UINT64 AccountNo, UINT ReplyCount)
{
	// 1. ���̷ε� ����
	CProtocolBuff* Payload = (CProtocolBuff*)Packet;

	// ���� ģ����û ��(ī��Ʈ) ����	
	*Payload << ReplyCount;

	// ���� ���� ģ����û ���� 0�� �ƴ� ���,
	// ó������ ������ [���� ģ����û ���]�� ���鼭, ���� ģ����û�� ���� ������ AccountNo, �г��� ����
	map <UINT64, stFromTo*>::iterator itor;
	if (ReplyCount != 0)
	{
		for (itor = map_FriendRequestList.begin(); itor != map_FriendRequestList.end(); ++itor)
		{
			// ���� ���� ��û�� ã������
			if (itor->second->m_FromAccount == AccountNo)
			{
				// ��û�� ���� ����� ������ �����´�.
				map <UINT64, stAccount*>::iterator Accountitor;
				Accountitor = map_AccountList.find(itor->second->m_ToAccount);

				// ����� AccountNo, �г��� ����
				*Payload << Accountitor->second->m_AccountNo;

				for (int i = 0; i<dfNICK_MAX_LEN; ++i)
					*Payload << Accountitor->second->m_NickName[i];

			}
		}
	}

	// 2. ��� ����
	CreateHeader((CProtocolBuff*)header, df_RES_FRIEND_REQUEST_LIST, Payload->GetUseSize());
}

// "���� ģ����û ��� ���" ��Ŷ ó��
void Network_Res_ReplytList(char* header, char* Packet, UINT64 AccountNo, UINT ReplyCount)
{
	// 1. ���̷ε� ����
	CProtocolBuff* Payload = (CProtocolBuff*)Packet;

	//  ���� ģ����û ��(ī��Ʈ) ����	
	*Payload << ReplyCount;

	// ���� ���� ģ����û ���� 0�� �ƴ� ���,
	// ó������ ������ [���� ģ����û ���]�� ���鼭, ������ ģ����û�� ���� ������ AccountNo, �г��� ����
	multimap <UINT64, stFromTo*>::iterator itor;
	if (ReplyCount != 0)
	{
		for (itor = map_FriendReplyList.begin(); itor != map_FriendReplyList.end(); ++itor)
		{
			// ������ ���� ��û�� ã������
			if (itor->second->m_FromAccount == AccountNo)
			{
				// ��û�� ���� ����� ������ �����´�.
				map <UINT64, stAccount*>::iterator Accountitor;
				Accountitor = map_AccountList.find(itor->second->m_ToAccount);

				// ����� AccountNo, �г��� ����
				*Payload << Accountitor->second->m_AccountNo;

				for (int i = 0; i<dfNICK_MAX_LEN; ++i)
					*Payload << Accountitor->second->m_NickName[i];

			}
		}
	}

	// 2. ��� ����
	CreateHeader((CProtocolBuff*)header, df_RES_FRIEND_REPLY_LIST, Payload->GetUseSize());
}

// "ģ������ ��û ���" ��Ŷ ����
void Network_Res_FriendRemove(char* header, char* Packet, UINT64 AccountNo, BYTE Result)
{
	// 1. ���̷ε� ����
	CProtocolBuff* Payload = (CProtocolBuff*)Packet;

	// AccountNo, Result ����
	*Payload << AccountNo;
	*Payload << Result;

	// 2. ��� ����
	CreateHeader((CProtocolBuff*)header, df_RES_FRIEND_REMOVE, Payload->GetUseSize());

}

// "ģ�� ��û ���" ��Ŷ ����
void Network_Res_FriendRequest(char* header, char* Packet, UINT64 AccountNo, BYTE Result)
{
	// 1. ���̷ε� ����
	CProtocolBuff* Payload = (CProtocolBuff*)Packet;

	// AccountNo, Result ����
	*Payload << AccountNo;
	*Payload << Result;

	// 2. ��� ����
	CreateHeader((CProtocolBuff*)header, df_RES_FRIEND_REQUEST, Payload->GetUseSize());
}

// "ģ����û ��� ���" ��Ŷ ����
void Network_Res_RequestCancel(char* header, char* Packet, UINT64 AccountNo, BYTE Result)
{
	// 1. ���̷ε� ����
	CProtocolBuff* Payload = (CProtocolBuff*)Packet;

	// AccountNo, Result ����
	*Payload << AccountNo;
	*Payload << Result;

	// 2. ��� ����
	CreateHeader((CProtocolBuff*)header, df_RES_FRIEND_CANCEL, Payload->GetUseSize());
}

// "������û ��� ���" ��Ŷ ����
void Network_Res_RequestDeny(char* header, char* Packet, UINT64 AccountNo, BYTE Result)
{
	// 1. ���̷ε� ����
	CProtocolBuff* Payload = (CProtocolBuff*)Packet;

	// AccountNo, Result ����
	*Payload << AccountNo;
	*Payload << Result;

	// 2. ��� ����
	CreateHeader((CProtocolBuff*)header, df_RES_FRIEND_DENY, Payload->GetUseSize());

}

// "ģ����û ���� ���" ��Ŷ ����
void Network_Res_FriendAgree(char* header, char* Packet, UINT64 AccountNo, BYTE Result)
{
	// 1. ���̷ε� ����
	CProtocolBuff* Payload = (CProtocolBuff*)Packet;

	// AccountNo, Result ����
	*Payload << AccountNo;
	*Payload << Result;

	// 2. ��� ����
	CreateHeader((CProtocolBuff*)header, df_RES_FRIEND_AGREE, Payload->GetUseSize());
}

// "��Ʈ���� �׽�Ʈ ���" ��Ŷ ����
void Network_Res_StressTest(char* header, char* Packet, WORD StringSize, TCHAR* TestString)
{
	// 1. ���̷ε� ����
	CProtocolBuff* Payload = (CProtocolBuff*)Packet;

	*Payload << StringSize;

	for (int i = 0; i < StringSize / 2; ++i)
		*Payload << TestString[i];

	// 2. ��� ����
	CreateHeader((CProtocolBuff*)header, df_RES_STRESS_ECHO, Payload->GetUseSize());
}




/////////////////////////
// Send ó��
/////////////////////////
// Send���ۿ� ������ �ֱ�
bool SendPacket(SOCKET sock, CProtocolBuff* headerBuff, CProtocolBuff* payloadBuff)
{
	// ���ڷ� ���� sock�� �������� Accept ���� ��Ͽ��� ���� �˾ƿ���.
	stAcceptUser* NowAccount = ClientSearch_AcceptList(sock);

	// ���ܻ��� üũ
	// 1. �ش� ������ Accept ���� �����ΰ�.
	// NowAccount�� ���� nullptr�̸�, �ش� ������ ��ã�� ��. false�� �����Ѵ�.
	if (NowAccount == nullptr)
	{
		_tprintf(_T("SendPacket(). �α��� ���� �ƴ� ������ ������� �Լ��� ȣ���. ���� ����\n"));
		return false;
	}

	// 1. ���� q�� ��� �ֱ�
	int Size = headerBuff->GetUseSize();
	DWORD BuffArray = 0;
	int a = 0;
	while (Size > 0)
	{
		int EnqueueCheck = NowAccount->m_SendBuff.Enqueue(headerBuff->GetBufferPtr() + BuffArray, Size);
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
		int EnqueueCheck = NowAccount->m_SendBuff.Enqueue(payloadBuff->GetBufferPtr() + BuffArray, PayloadLen);
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
bool SendProc(SOCKET sock)
{
	// ���ڷ� ���� sock�� �������� Accept ���� ��Ͽ��� ���� �˾ƿ���.
	stAcceptUser* NowAccount = ClientSearch_AcceptList(sock);

	// ���ܻ��� üũ
	// 1. �ش� ������ Accept ���� �����ΰ�.
	// NowAccount�� ���� nullptr�̸�, �ش� ������ ��ã�� ��. false�� �����Ѵ�.
	if (NowAccount == nullptr)
	{
		_tprintf(_T("SendProc(). �α��� ���� �ƴ� ������ ������� SendProc�� ȣ���. ���� ����\n"));
		return false;
	}

	// 2. SendBuff�� ���� �����Ͱ� �ִ��� Ȯ��.
	if (NowAccount->m_SendBuff.GetUseSize() == 0)
		return true;

	// 3. ���� �������� �������� ���� ������
	char* sendbuff = NowAccount->m_SendBuff.GetBufferPtr();

	// 4. SendBuff�� �� �������� or Send����(����)���� ��� send
	while (1)
	{
		// 5. SendBuff�� �����Ͱ� �ִ��� Ȯ��(���� üũ��)
		if (NowAccount->m_SendBuff.GetUseSize() == 0)
			break;

		// 6. �� ���� ���� �� �ִ� �������� ���� ���� ���
		int Size = NowAccount->m_SendBuff.GetNotBrokenGetSize();

		// 7. ���� ������ 0�̶�� 1�� ����. send() �� 1����Ʈ�� �õ��ϵ��� �ϱ� ���ؼ�.
		// �׷��� send()�� ������ �ߴ��� Ȯ�� ����
		if (Size == 0)
			Size = 1;

		// 8. front ������ ����
		int *front = (int*)NowAccount->m_SendBuff.GetReadBufferPtr();

		// 9. front�� 1ĭ �� index�� �˾ƿ���.
		int BuffArrayCheck = NowAccount->m_SendBuff.NextIndex(*front, 1);

		// 10. Send()
		int SendSize = send(NowAccount->m_sock, &sendbuff[BuffArrayCheck], Size, 0);

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
		NowAccount->m_SendBuff.RemoveData(SendSize);
	}

	return true;
}
