#include "pch.h"
#include "Chat_LanClient.h"

namespace Library_Jingyu
{

	// -----------------------
	// �����ڿ� �Ҹ���
	// -----------------------
	Chat_LanClient::Chat_LanClient()
		:CLanClient()
	{
		// ��ū�� �����ϴ� umap�� �뷮�� �Ҵ��صд�.
		m_umapTokenCheck.reserve(10000);

		// ��� ����ü ���� TLS �����Ҵ�
		m_MTokenTLS = new CMemoryPoolTLS< stToken >(500, false);

		// �� �ʱ�ȭ
		InitializeSRWLock(&srwl);
	}

	Chat_LanClient::~Chat_LanClient()
	{
		// ��� ����ü ���� TLS ��������
		delete m_MTokenTLS;
	}




	// -----------------------
	// ��Ŷ ó�� �Լ�
	// -----------------------

	// ���ο� ������ �α��� ������ ���� ��, �α��� �����κ��� ��ūŰ�� �޴´�.
	// �� ��ūŰ�� ������ �� ������ ������ �Լ�
	void Chat_LanClient::NewUserJoin(ULONGLONG SessionID, CProtocolBuff_Lan* Payload)
	{
		// 1. ��ū TLS���� Alloc
		// !! ��ū TLS  Free��, ChatServer����, ���� ������ ������ ��(OnClientLeave)���� �Ѵ� !!
		stToken* NewToken = m_MTokenTLS->Alloc();

		// 2. AccountNo ���´�.
		INT64 AccountNo;
		Payload->GetData((char*)&AccountNo, 8);

		// 3. ��ūŰ ���´�.	
		// NewToken�� ���� �ִ´�.
		Payload->GetData((char*)NewToken->m_cToken, 64);

		// 4. �Ķ���� ���´�.
		INT64 Parameter;
		Payload->GetData((char*)&Parameter, 8);

		// 5. �ڷᱸ���� ��ū ����
		InsertTokenFunc(AccountNo, NewToken);
			   

		// ������Ŷ ���� �� ������ -------------------
		
		// 1. ����ȭ���� Alloc
		CProtocolBuff_Lan* SendBuff = CProtocolBuff_Lan::Alloc();

		// 2. Ÿ�� ����
		WORD Type = en_PACKET_SS_RES_NEW_CLIENT_LOGIN;
				
		// 3. ���� �����͸� SendBUff�� �ִ´�
		SendBuff->PutData((char*)&Type, 2);
		SendBuff->PutData((char*)&AccountNo, 8);
		SendBuff->PutData((char*)&Parameter, 8);

		// 4. Send �Ѵ�.
		SendPacket(SessionID, SendBuff);
	}




	// -----------------------
	// ��� �Լ�
	// -----------------------

	// ��ū ���� �ڷᱸ����, ���� ������ ��ū �߰�
	// ���� umap���� ������
	// 
	// Parameter : AccountNo, stToken*
	// return : �߰� ���� ��, true
	//		  : AccountNo�� �ߺ��� �� false
	bool Chat_LanClient::InsertTokenFunc(INT64 AccountNo, stToken* isnertToken)
	{
		// umap�� �߰�
		AcquireSRWLockExclusive(&srwl);		// ---------------- Lock
		auto ret = m_umapTokenCheck.insert(make_pair(AccountNo, isnertToken));
		ReleaseSRWLockExclusive(&srwl);		// ---------------- Unock

		// �ߺ��� Ű�� �� false ����.
		if (ret.second == false)
			return false;

		return true;
	}


	// ��ū ���� �ڷᱸ������, ��ū �˻�
	// ���� umap���� ������
	// 
	// Parameter : AccountNo
	// return : �˻� ���� ��, stToken*
	//		  : �˻� ���� �� nullptr
	Chat_LanClient::stToken* Chat_LanClient::FindTokenFunc(INT64 AccountNo)
	{
		AcquireSRWLockShared(&srwl);	// ---------------- Shared Lock
		auto FindToken = m_umapTokenCheck.find(AccountNo);
		ReleaseSRWLockShared(&srwl);	// ---------------- Shared UnLock

		if (FindToken == m_umapTokenCheck.end())
			return nullptr;

		return FindToken->second;
	}


	// ��ū ���� �ڷᱸ������, ��ū ����
	// ���� umap���� ������
	// 
	// Parameter : AccountNo
	// return : ���� ��, ���ŵ� ��ū stToken*
	//		  : �˻� ���� �� nullptr
	Chat_LanClient::stToken* Chat_LanClient::EraseTokenFunc(INT64 AccountNo)
	{
		// 1) map���� ���� �˻�

		// erase������ �� �۾��̱� ������, Exclusive �� ���.
		AcquireSRWLockExclusive(&srwl);		// ---------------- Lock

		auto FindToken = m_umapTokenCheck.find(AccountNo);
		if (FindToken == m_umapTokenCheck.end())
		{
			ReleaseSRWLockExclusive(&srwl);	// ---------------- Unlock
			return nullptr;
		}

		stToken* ret = FindToken->second;

		// 2) �ʿ��� ����
		m_umapTokenCheck.erase(FindToken);

		ReleaseSRWLockExclusive(&srwl);	// ---------------- Unlock

		return ret;
	}





	// -----------------------
	// ���� �����Լ�
	// -----------------------

	// ��ǥ ������ ���� ���� ��, ȣ��Ǵ� �Լ� (ConnectFunc���� ���� ���� �� ȣ��)
	//
	// parameter : ����Ű
	// return : ����
	void Chat_LanClient::OnConnect(ULONGLONG SessionID)
	{
		// �α��� ������, �α��� ��Ŷ ����.

		// 1. Ÿ�� ����
		WORD Type = en_PACKET_SS_LOGINSERVER_LOGIN;

		// 2. �α��ο��� �˷���, �����ϴ� ������ Ÿ��
		BYTE ServerType = en_PACKET_SS_LOGINSERVER_LOGIN::dfSERVER_TYPE_CHAT;

		// 3. ���� �̸� ����. �ƹ��ų�
		WCHAR ServerName[32] = L"Chat_LanClient";

		// 4. ����ȭ ���۸� Alloc�޾Ƽ� ����
		CProtocolBuff_Lan* SendBuff = CProtocolBuff_Lan::Alloc();

		SendBuff->PutData((char*)&Type, 2);
		SendBuff->PutData((char*)&ServerType, 1);
		SendBuff->PutData((char*)&ServerName, 64);

		// 5. Send�Ѵ�.
		SendPacket(SessionID, SendBuff);
	}

	// ��ǥ ������ ���� ���� �� ȣ��Ǵ� �Լ� (InDIsconnect �ȿ��� ȣ��)
	//
	// parameter : ����Ű
	// return : ����
	void Chat_LanClient::OnDisconnect(ULONGLONG SessionID)
	{
		// ���� �ϴ°� ����. 
		// ������ �����, LanClient���� �̹� ���� �ٽ� �õ���.
	}

	// ��Ŷ ���� �Ϸ� �� ȣ��Ǵ� �Լ�.
	//
	// parameter : ���� ����Ű, ���� ��Ŷ
	// return : ����
	void Chat_LanClient::OnRecv(ULONGLONG SessionID, CProtocolBuff_Lan* Payload)
	{
		// ��ūŰ�� �����ϴ� �ڷᱸ���� ��ū ����

		// 1. Type ����
		WORD Type;
		Payload->GetData((char*)&Type, 2);

		// 2. Ÿ�Կ� ���� ���� ó��
		switch (Type)
		{
			// ���ο� ������ �������� (�α��� �����κ��� ����)
		case en_PACKET_SS_REQ_NEW_CLIENT_LOGIN:
			NewUserJoin(SessionID, Payload);
			break;


		default:
			break;
		}
	}

	// ��Ŷ �۽� �Ϸ� �� ȣ��Ǵ� �Լ�
	//
	// parameter : ���� ����Ű, Send �� ������
	// return : ����
	void Chat_LanClient::OnSend(ULONGLONG SessionID, DWORD SendSize)
	{
		// ���� �Ұ� ����
	}

	// ��Ŀ �����尡 ��� �� ȣ��Ǵ� �Լ�.
	// GQCS �ٷ� �ϴܿ��� ȣ��
	// 
	// parameter : ����
	// return : ����
	void Chat_LanClient::OnWorkerThreadBegin()
	{
		// ���� �Ұ� ����
	}

	// ��Ŀ �����尡 ���� �� ȣ��Ǵ� �Լ�
	// GQCS �ٷ� ������ ȣ��
	// 
	// parameter : ����
	// return : ����
	void Chat_LanClient::OnWorkerThreadEnd()
	{
		// ���� �Ұ� ����
	}

	// ���� �߻� �� ȣ��Ǵ� �Լ�.
	//
	// parameter : ���� �ڵ�(���� ������ �����ڵ�� WinGetLastError() �Լ��� ��� ����. ���� ��� 0�� ���ϵ�)
	//			 : ���� �ڵ忡 ���� ��Ʈ��
	// return : ����
	void Chat_LanClient::OnError(int error, const TCHAR* errorStr)
	{
		// ���� �Ұ� ����
	}


}