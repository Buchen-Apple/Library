#include "pch.h"
#include "ChatServer_Room.h"
#include "Protocol_Set/CommonProtocol_2.h"
#include "CrashDump/CrashDump.h"
#include "Log/Log.h"

#include <strsafe.h>


namespace Library_Jingyu
{
	// ����ȭ ���� 1���� ũ��
	// �� ������ ���� ������ �����ؾ� ��.
	LONG g_lNET_BUFF_SIZE;

	// ������ 
	CCrashDump* g_ChatDump = CCrashDump::GetInstance();
	CSystemLog* g_ChatLog = CSystemLog::GetInstance();


	// ----------------------------
	// �÷��̾� ���� �ڷᱸ�� �Լ�
	// ----------------------------

	// �÷��̾� ���� �ڷᱸ���� Insert
	//
	// Parameter : SessionID, stPlayer*
	// return : ���� �߰� �� true
	//		  : Ű �ߺ� �� false
	bool CChatServer_Room::InsertPlayerFunc(ULONGLONG SessionID, stPlayer* InsertPlayer)
	{
		AcquireSRWLockExclusive(&m_Player_Umap_srwl);	 // ----- Player Umap Exclusive ��

		// 1. �߰�
		auto ret = m_Player_Umap.insert(make_pair(SessionID, InsertPlayer));

		// �ߺ�Ű�� false ����
		if (ret.second == false)
		{
			ReleaseSRWLockExclusive(&m_Player_Umap_srwl);	 // ----- Player Umap Exclusive ���
			return false;
		}


		ReleaseSRWLockExclusive(&m_Player_Umap_srwl);	 // ----- Player Umap Exclusive ���

		return true;
	}

	// �÷��̾� ���� �ڷᱸ������ �˻�
	//
	// Parameter : SessionID
	// return :  �� ã���� stPlayer*
	//		  :  ���� ������ �� nullptr	
	CChatServer_Room::stPlayer* CChatServer_Room::FindPlayerFunc(ULONGLONG SessionID)
	{
		AcquireSRWLockShared(&m_Player_Umap_srwl);	 // ----- Player Umap Shared ��

		// 1. �˻�
		auto FindPlayer = m_Player_Umap.find(SessionID);

		// ���� ������� false ����
		if (FindPlayer == m_Player_Umap.end())
		{
			ReleaseSRWLockShared(&m_Player_Umap_srwl);	 // ----- Player Umap Shared ���
			return false;
		}

		// 2. �ִٸ� �޾Ƶд�.
		stPlayer* RetPlayer = FindPlayer->second;

		ReleaseSRWLockShared(&m_Player_Umap_srwl);	 // ----- Player Umap Shared ���

		// 3. ����
		return RetPlayer;
	}

	// �÷��̾� ���� �ڷᱸ������ ����
	//
	// Parameter : SessionID
	// return :  Erase �� Second(stPlayer*)
	//		  : ���� ������ �� nullptr
	CChatServer_Room::stPlayer* CChatServer_Room::ErasePlayerFunc(ULONGLONG SessionID)
	{
		AcquireSRWLockExclusive(&m_Player_Umap_srwl);	 // ----- Player Umap Exclusive ��

		// 1. �˻�
		auto FindPlayer = m_Player_Umap.find(SessionID);

		// ��ã������ nullptr ����
		if (FindPlayer == m_Player_Umap.end())
		{
			ReleaseSRWLockExclusive(&m_Player_Umap_srwl);	 // ----- Player Umap Exclusive ���
			return nullptr;
		}

		// 2. ã������ ���� �� �޾Ƶΰ� Erase
		stPlayer* RetPlayer = FindPlayer->second;

		m_Player_Umap.erase(FindPlayer);


		ReleaseSRWLockExclusive(&m_Player_Umap_srwl);	 // ----- Player Umap Exclusive ���

		return RetPlayer;
	}





	// ----------------------------
	// �÷��̾� ���� �ڷᱸ�� �Լ�
	// ----------------------------

	// �α��� �� �÷��̾� ���� �ڷᱸ���� Insert
	//
	// Parameter : AccountNo, stPlayer*
	// return : ���� �߰� �� true
	//		  : Ű �ߺ� �� flase
	bool CChatServer_Room::InsertLoginPlayerFunc(INT64 AccountNo, stPlayer* InsertPlayer)
	{
		AcquireSRWLockExclusive(&m_LoginPlayer_Umap_srwl);	 // ----- �α��� Player Umap Exclusive ��

		// 1. �߰�
		auto ret = m_LoginPlayer_Umap.insert(make_pair(AccountNo, InsertPlayer));

		// �ߺ�Ű�� false ����
		if (ret.second == false)
		{
			ReleaseSRWLockExclusive(&m_LoginPlayer_Umap_srwl);	 // ----- �α��� Player Umap Exclusive ���
			return false;
		}

		ReleaseSRWLockExclusive(&m_LoginPlayer_Umap_srwl);	 // ----- �α��� Player Umap Exclusive ���

		return true;

	}

	// �α��� �� �÷��̾� ���� �ڷᱸ������ ����
	//
	// Parameter : AccountNo
	// return : ���������� Erase �� true
	//		  : ����  �˻� ���� �� false
	bool CChatServer_Room::EraseLoginPlayerFunc(ULONGLONG AccountNo)
	{
		AcquireSRWLockExclusive(&m_LoginPlayer_Umap_srwl);	 // ----- �α��� Player Umap Exclusive ��

		// 1. �˻�
		auto FindPlayer = m_LoginPlayer_Umap.find(AccountNo);

		// ��ã������ nullptr ����
		if (FindPlayer == m_LoginPlayer_Umap.end())
		{
			ReleaseSRWLockExclusive(&m_LoginPlayer_Umap_srwl);	 // ----- �α��� Player Umap Exclusive ���
			return false;
		}

		// 2. ã������ Erase
		m_LoginPlayer_Umap.erase(FindPlayer);

		ReleaseSRWLockExclusive(&m_LoginPlayer_Umap_srwl);	 // ----- �α��� Player Umap Exclusive ���

		return true;
	}







	// ------------------
	// ��Ŷ ó�� �Լ�
	// ------------------

	// �α��� ��Ŷ
	//
	// Parameter : SessionID, CProtocolBuff_Net*
	// return : ����
	void CChatServer_Room::Packet_Login(ULONGLONG SessionID, CProtocolBuff_Net* Packet)
	{
		// 1. �÷��̾� �˻�
		stPlayer* NowPlayer = FindPlayerFunc(SessionID);

		// ������ ũ����
		if (NowPlayer == nullptr)
			g_ChatDump->Crash();

		// 2. ������
		INT64	AccountNo;
		char	ConnectToken[32];

		Packet->GetData((char*)&AccountNo, 8);
		Packet->GetData((char*)NowPlayer->m_tcID, 40);
		Packet->GetData((char*)NowPlayer->m_tcNick, 40);
		Packet->GetData(ConnectToken, 32);


		// 3. �α��� ���� üũ. 
		// �̹� �α������̸� �α��� ��Ŷ�� �Ǻ�����? Ȥ�� ���� �Ǽ�..
		if (NowPlayer->m_bLoginCheck == true)
		{
			g_ChatDump->Crash();

			/*
			TCHAR str[100];
			StringCchPrintf(str, 100, _T("Packet_Login(). LoginFlag True. SessionID : %lld, AccountNo : %lld"), 
				SessionID, AccountNo);

			throw CException(str);	
			*/
		}		



		// 4. ��ū üũ
		if (memcmp(m_cConnectToken, ConnectToken, 32) != 0)
		{
			// �ٸ��� ���� ��Ŷ
			CProtocolBuff_Net* SendBuff = CProtocolBuff_Net::Alloc();

			WORD Type = en_PACKET_CS_CHAT_RES_LOGIN;
			BYTE Status = 0;

			SendBuff->PutData((char*)&Type, 2);
			SendBuff->PutData((char*)&Status, 1);
			SendBuff->PutData((char*)&AccountNo, 8);

			SendPacket(SessionID, SendBuff);

			return;
		}
		
			

		// 5. AccountNo ����
		NowPlayer->m_i64AccountNo = AccountNo;



		// 6. �α��� �÷��̾� ������ �ڷᱸ���� �߰� (�ߺ� �α��� üũ)
		if (InsertLoginPlayerFunc(AccountNo, NowPlayer) == false)
		{
			// ���� ��Ŷ -------
			CProtocolBuff_Net* SendBuff = CProtocolBuff_Net::Alloc();

			WORD Type = en_PACKET_CS_CHAT_RES_LOGIN;
			BYTE Status = 0;

			SendBuff->PutData((char*)&Type, 2);
			SendBuff->PutData((char*)&Status, 1);
			SendBuff->PutData((char*)&AccountNo, 8);

			SendPacket(SessionID, SendBuff);


			// ������ �������̴� ���� ���� ����	-------

			AcquireSRWLockShared(&m_LoginPlayer_Umap_srwl);	// ----- �α��� Player Shared ��

			// 1) �˻�
			auto FindPlayer = m_LoginPlayer_Umap.find(SessionID);

			// 2) ���� �� ����. �� ������ ó���ϴµ� ������� ���ɼ�
			if (FindPlayer == m_LoginPlayer_Umap.end())
			{
				ReleaseSRWLockShared(&m_LoginPlayer_Umap_srwl);	// ----- �α��� Player Shared ���
				return;
			}

			// 3) ������ ���� ���� ��û
			Disconnect(FindPlayer->second->m_ullSessionID);

			ReleaseSRWLockShared(&m_LoginPlayer_Umap_srwl);	// ----- �α��� Player Shared ���

			return;
		}

		// 7. �� �߰������� �÷��� ����.
		NowPlayer->m_bLoginCheck = true;


		// 8. ���� ���� ������
		CProtocolBuff_Net* SendBuff = CProtocolBuff_Net::Alloc();

		WORD Type = en_PACKET_CS_CHAT_RES_LOGIN;
		BYTE Status = 1;

		SendBuff->PutData((char*)&Type, 2);
		SendBuff->PutData((char*)&Status, 1);
		SendBuff->PutData((char*)&AccountNo, 8);

		SendPacket(SessionID, SendBuff);
	}

	// �� ���� 
	//
	// Parameter : SessionID, CProtocolBuff_Net*
	// return : ����
	void CChatServer_Room::Packet_Room_Enter(ULONGLONG SessionID, CProtocolBuff_Net* Packet)
	{
		// 1. �÷��̾� �˻�
		stPlayer* NowPlayer = FindPlayerFunc(SessionID);

		// ������ ũ����
		if (NowPlayer == nullptr)
			g_ChatDump->Crash();


		// 2. ������
		INT64	AccountNo;
		int		RoomNo;
		char	EnterToken[32];

		Packet->GetData((char*)&AccountNo, 8);
		Packet->GetData((char*)&RoomNo, 4);
		Packet->GetData(EnterToken, 32);


		// 3. �α��� ���� üũ. 
		// �α��� ���� �ƴϸ� Crash
		if (NowPlayer->m_bLoginCheck == false)
		{
			g_ChatDump->Crash();

			/*
			TCHAR str[100];
			StringCchPrintf(str, 100, _T("Packet_Room_Enter(). LoginFlag True. SessionID : %lld, AccountNo : %lld"),
				SessionID, AccountNo);

			throw CException(str);
			*/
		}
		


		// 4. AccountNo ����
		if (AccountNo != NowPlayer->m_i64AccountNo)
		{
			g_ChatDump->Crash();

			/*
			TCHAR str[100];
			StringCchPrintf(str, 100, _T("Packet_Room_Enter(). AccountNo Error. SessionID : %lld, AccountNo : %lld"), 
				SessionID, AccountNo);

			throw CException(str);
			*/
		}


		// 5. �������� �� ����
		if (RoomNo != NowPlayer->m_iRoomNo)
		{
			g_ChatDump->Crash();

			/*
			TCHAR str[100];
			StringCchPrintf(str, 100, _T("Packet_Room_Enter(). RoomNo Error. SessionID : %lld, AccountNo : %lld, RoomNo : %d"),
				SessionID, AccountNo, RoomNo);

			throw CException(str);
			*/
		}	



		// 6. �� �˻�
		AcquireSRWLockShared(&m_Room_Umap_srwl);		// ----- �� Shared ��

		auto FindRoom = m_Room_Umap.find(RoomNo);

		if (FindRoom == m_Room_Umap.end())
		{
			ReleaseSRWLockShared(&m_Room_Umap_srwl);		// ----- �� Shared ���

			// ���� ���� �� ����. ���� ��Ŷ ����
			CProtocolBuff_Net* SendBuff = CProtocolBuff_Net::Alloc();

			WORD Type = en_PACKET_CS_CHAT_RES_ENTER_ROOM;
			BYTE Status = 3;	// �� ����

			SendBuff->PutData((char*)&Type, 2);
			SendBuff->PutData((char*)&AccountNo, 8);
			SendBuff->PutData((char*)&RoomNo, 4);
			SendBuff->PutData((char*)&Status, 1);

			SendPacket(SessionID, SendBuff);
			return;
		}

		stRoom* NowRoom = FindRoom->second;


		// 7. �� ��ū �˻� �� �ڷᱸ���� �߰�
		NowRoom->ROOM_LOCK();	// ----- �� ��	

		// �� ��ū �˻�
		if (memcmp(EnterToken, NowRoom->m_cEnterToken, 32) != 0)
		{
			ReleaseSRWLockShared(&m_Room_Umap_srwl);		// ----- �� Shared ���
			NowRoom->ROOM_UNLOCK();							// ----- �� ���	

			// ��ū	����ġ ��Ŷ
			CProtocolBuff_Net* SendBuff = CProtocolBuff_Net::Alloc();

			WORD Type = en_PACKET_CS_CHAT_RES_ENTER_ROOM;
			BYTE Status = 2;	// ��ū �ٸ�

			SendBuff->PutData((char*)&Type, 2);
			SendBuff->PutData((char*)&AccountNo, 8);
			SendBuff->PutData((char*)&RoomNo, 4);
			SendBuff->PutData((char*)&Status, 1);

			SendPacket(SessionID, SendBuff);
			return;
		}

		// �� �� �ڷᱸ���� ���� �߰�
		NowRoom->Insert(SessionID);

		NowRoom->ROOM_UNLOCK();	// ----- �� ���	

		ReleaseSRWLockShared(&m_Room_Umap_srwl);		// ----- �� Shared ���



		// 8. ���� ��Ŷ ����
		CProtocolBuff_Net* SendBuff = CProtocolBuff_Net::Alloc();

		WORD Type = en_PACKET_CS_CHAT_RES_ENTER_ROOM;
		BYTE Status = 1;	// ����

		SendBuff->PutData((char*)&Type, 2);
		SendBuff->PutData((char*)&AccountNo, 8);
		SendBuff->PutData((char*)&RoomNo, 4);
		SendBuff->PutData((char*)&Status, 1);

		SendPacket(SessionID, SendBuff);
	}

	// ä�� ������
	//
	// Parameter : SessionID, CProtocolBuff_Net*
	// return : ����
	void CChatServer_Room::Packet_Message(ULONGLONG SessionID, CProtocolBuff_Net* Packet)
	{
		// 1. �÷��̾� �˻�
		stPlayer* NowPlayer = FindPlayerFunc(SessionID);

		// ������ ũ����
		if (NowPlayer == nullptr)
			g_ChatDump->Crash();



		// 2. ������
		INT64	AccountNo;
		WORD	MessageLen;
		WCHAR	Message[512];		// null ������

		Packet->GetData((char*)&AccountNo, 8);
		Packet->GetData((char*)&MessageLen, 2);
		Packet->GetData((char*)Message, MessageLen);
		


		// 3. �α��� ���� üũ. 
		// �α��� ���� �ƴϸ� Crash
		if (NowPlayer->m_bLoginCheck == false)
		{
			g_ChatDump->Crash();

			/*
			TCHAR str[100];
			StringCchPrintf(str, 100, _T("Packet_Message(). LoginFlag True. SessionID : %lld, AccountNo : %lld"),
				SessionID, AccountNo);

			throw CException(str);
			*/
		}


		// 4. AccountNo ����
		if (AccountNo != NowPlayer->m_i64AccountNo)
		{
			g_ChatDump->Crash();

			/*
			TCHAR str[100];
			StringCchPrintf(str, 100, _T("Packet_Message(). AccountNo Error. SessionID : %lld, AccountNo : %lld"),
				SessionID, AccountNo);

			throw CException(str);
			*/
		}		


		// 5.  ä�� ���� ��Ŷ �����
		CProtocolBuff_Net* SendBuff = CProtocolBuff_Net::Alloc();

		WORD Type = en_PACKET_CS_CHAT_RES_MESSAGE;

		SendBuff->PutData((char*)&Type, 2);

		SendBuff->PutData((char*)&AccountNo, 8);
		SendBuff->PutData((char*)NowPlayer->m_tcID, 40);
		SendBuff->PutData((char*)NowPlayer->m_tcNick, 40);

		SendBuff->PutData((char*)&MessageLen, 2);
		SendBuff->PutData((char*)Message, MessageLen);
			   	


		// 6. �� �˻�
		int RoomNo = NowPlayer->m_iRoomNo;
		AcquireSRWLockShared(&m_Room_Umap_srwl);		// ----- �� Shared ��

		auto FindRoom = m_Room_Umap.find(RoomNo);

		// �� ���°� ���� �ȵ�. Crash
		if (FindRoom == m_Room_Umap.end())
			g_ChatDump->Crash();

		stRoom* NowRoom = FindRoom->second;


		// 7. �� ���� ��� �������� ä�� ���� ��Ŷ BroadCast	
		// �ڱ� �ڽ� ����

		NowRoom->ROOM_LOCK();	// ----- �� ��			

		// BoradCast		
		if(Room_BroadCast(NowRoom, SendBuff) == false)
			g_ChatDump->Crash();

		NowRoom->ROOM_UNLOCK();	// ----- �� ���	

		ReleaseSRWLockShared(&m_Room_Umap_srwl);		// ----- �� Shared ���
	}




	// -----------------------
	// �����Լ�
	// -----------------------

	//Accept ����, ȣ��ȴ�.
	//
	// parameter : ������ ������ IP, Port
	// return false : Ŭ���̾�Ʈ ���� �ź�
	// return true : ���� ���
	bool CChatServer_Room::OnConnectionRequest(TCHAR* IP, USHORT port)
	{
		return true;
	}

	// ���� �� ȣ��Ǵ� �Լ� (AcceptThread���� Accept �� ȣ��)
	//
	// parameter : ������ �������� �Ҵ�� ����Ű
	// return : ����
	void CChatServer_Room::OnClientJoin(ULONGLONG SessionID)
	{
		// 1. stPlayer Alloc
		stPlayer* NewPlayer = m_pPlayer_Pool->Alloc();

		// 2. �ʱ� ����
		NewPlayer->m_ullSessionID = SessionID;

		// 3. �ڷᱸ���� �߰�
		InsertPlayerFunc(SessionID, NewPlayer);
	}

	// ���� ���� �� ȣ��Ǵ� �Լ� (InDIsconnect �ȿ��� ȣ��)
	//
	// parameter : ���� ����Ű
	// return : ����
	void CChatServer_Room::OnClientLeave(ULONGLONG SessionID)
	{
		// 1. �ڷᱸ������ ����
		stPlayer* DeletePlayer = ErasePlayerFunc(SessionID);

		// ������ Crash
		if (DeletePlayer == nullptr)
			g_ChatDump->Crash();

		int RoomNo = DeletePlayer->m_iRoomNo;


		// 2. �α��� ������ ���
		if (DeletePlayer->m_bLoginCheck == true)
		{
			// �̸� �ʱ�ȭ
			DeletePlayer->m_bLoginCheck = false;

			// �α��� �ڷᱸ������ ����
			if(EraseLoginPlayerFunc(DeletePlayer->m_i64AccountNo) == false)
				g_ChatDump->Crash();
		}


		// 3. stPlayer* Free
		m_pPlayer_Pool->Free(DeletePlayer);


		// 4. �뿡�� ����

		AcquireSRWLockShared(&m_Room_Umap_srwl);		// ----- �� Shared ��

		// �� �˻�
		auto FindRoom = m_Room_Umap.find(RoomNo);
		if(FindRoom == m_Room_Umap.end())
			g_ChatDump->Crash();

		stRoom* NowRoom = FindRoom->second;
		
		NowRoom->ROOM_LOCK();	// ----- �� ��

		// �� �� �ڷᱸ������ ���� ����
		if(NowRoom->Erase(SessionID) == false)
			g_ChatDump->Crash();
		
		NowRoom->ROOM_UNLOCK();	// ----- �� ���

		ReleaseSRWLockShared(&m_Room_Umap_srwl);		// ----- �� Shared ���
	}

	// ��Ŷ ���� �Ϸ� �� ȣ��Ǵ� �Լ�.
	//
	// parameter : ���� ����Ű, ���� ��Ŷ
	// return : ����
	void CChatServer_Room::OnRecv(ULONGLONG SessionID, CProtocolBuff_Net* Payload)
	{
		// Ÿ�� ������
		WORD Type;
		Payload->GetData((char*)&Type, 2);


		// Ÿ�Կ� ���� �б� ó��
		try
		{
			switch (Type)
			{
				// ä�� ������
			case en_PACKET_CS_CHAT_REQ_MESSAGE:
				Packet_Message(SessionID, Payload);
				break;


				// �α��� ��û
			case en_PACKET_CS_CHAT_REQ_LOGIN:
				Packet_Login(SessionID, Payload);
				break;


				// �� ���� ��û
			case en_PACKET_CS_CHAT_REQ_ENTER_ROOM:
				Packet_Room_Enter(SessionID, Payload);
				break;


				// ��Ʈ��Ʈ
			case en_PACKET_CS_CHAT_REQ_HEARTBEAT:
				break;


				// ���� Ÿ���̸� ũ����
			default:
				TCHAR str[100];
				StringCchPrintf(str, 100, _T("OnRecv(). TypeError. SessionID : %lld, Type : %d"), SessionID, Type);

				throw CException(str);
			}
		}
		catch (CException& exc) 
		{
			// �α� ��� (�α� ���� : ����)
			g_ChatLog->LogSave(L"ChatServer", CSystemLog::en_LogLevel::LEVEL_ERROR, L"%s",
				(TCHAR*)exc.GetExceptionText());

			g_ChatDump->Crash();

			// ���� ���� ��û
			//Disconnect(SessionID);
		}

	}

	// ��Ŷ �۽� �Ϸ� �� ȣ��Ǵ� �Լ�
	//
	// parameter : ���� ����Ű, Send �� ������
	// return : ����
	void CChatServer_Room::OnSend(ULONGLONG SessionID, DWORD SendSize)
	{

	}

	// ��Ŀ �����尡 ��� �� ȣ��Ǵ� �Լ�.
	// GQCS �ٷ� �ϴܿ��� ȣ��
	// 
	// parameter : ����
	// return : ����
	void CChatServer_Room::OnWorkerThreadBegin()
	{

	}

	// ��Ŀ �����尡 ���� �� ȣ��Ǵ� �Լ�
	// GQCS �ٷ� ������ ȣ��
	// 
	// parameter : ����
	// return : ����
	void CChatServer_Room::OnWorkerThreadEnd()
	{

	}

	// ���� �߻� �� ȣ��Ǵ� �Լ�.
	//
	// parameter : ���� �ڵ�(���� ������ �����ڵ�� WinGetLastError() �Լ��� ��� ����. ���� ��� 0�� ���ϵ�)
	//			 : ���� �ڵ忡 ���� ��Ʈ��
	// return : ����
	void CChatServer_Room::OnError(int error, const TCHAR* errorStr)
	{
		// �α� ��� (�α� ���� : ����)
		g_ChatLog->LogSave(L"ChatServer", CSystemLog::en_LogLevel::LEVEL_ERROR, L"%s (ErrorCode : %d)",
			errorStr, error);

		g_ChatDump->Crash();
	}






	// --------------
	// �����ڿ� �Ҹ���
	// --------------

	// ������
	CChatServer_Room::CChatServer_Room()
	{
		// config


		// �α�
		g_ChatLog->SetDirectory(L"ChatServer");
		g_ChatLog->SetLogLeve((CSystemLog::en_LogLevel)CSystemLog::en_LogLevel::LEVEL_DEBUG);
		//g_ChatLog->SetLogLeve((CSystemLog::en_LogLevel)m_stConfig.LogLevel);




		// �ڷᱸ�� ���� �̸� ��Ƶα�
		m_Player_Umap.reserve(5000);
		m_LoginPlayer_Umap.reserve(5000);

		// �� �ʱ�ȭ
		InitializeSRWLock(&m_Player_Umap_srwl);
		InitializeSRWLock(&m_LoginPlayer_Umap_srwl);

		// TLS Pool �����Ҵ�
		m_pPlayer_Pool = new CMemoryPoolTLS<stPlayer>(0, false);
		m_pRoom_Pool = new CMemoryPoolTLS<stRoom>(0, false);
	}

	// �Ҹ���
	CChatServer_Room::~CChatServer_Room()
	{
		// TLS Pool ��������
		delete m_pPlayer_Pool;
		delete m_pRoom_Pool;
	}
}