#include "pch.h"
#include "BattleServer_Room_Version.h"
#include "Parser\Parser_Class.h"
#include "Protocol_Set\CommonProtocol_2.h"		// ���� �Ϲ����� ���� �ʿ�
#include "Log\Log.h"
#include "CPUUsage\CPUUsage.h"
#include "PDHClass\PDHCheck.h"

#include "shDB_Communicate.h"

#include "rapidjson\document.h"
#include "rapidjson\writer.h"
#include "rapidjson\stringbuffer.h"

#include <process.h>
#include <strsafe.h>

using namespace rapidjson;

extern ULONGLONG g_ullAcceptTotal_MMO;
extern LONG	  g_lAcceptTPS_MMO;
extern LONG	g_lSendPostTPS_MMO;
extern LONG	g_lRecvTPS_MMO;

extern LONG g_lAllocNodeCount;
extern LONG g_lAllocNodeCount_Lan;

extern LONG g_lAuthModeUserCount;
extern LONG g_lGameModeUserCount;

extern LONG g_lAuthFPS;
extern LONG g_lGameFPS;

LONG g_lShowAuthFPS;
LONG g_lShowGameFPS;

// ��Ʋ���� ���� ��ū ����
LONG g_lBattleEnterTokenError;

// auth�� �α��� ��Ŷ����, DB ���� �� ����� -10(����� ����)�� �� ���
LONG g_lQuery_Result_Not_Find;

// auth�� �α��� ��Ŷ����, DB ���� �� ����� -10�� �ƴѵ� 1�� �ƴ� ������ ��� 
LONG g_lTempError;

// auth�� �α��� ��Ŷ����, ������ SessionKey(��ū)�� �ٸ� ���
LONG g_lTokenError;

// auth�� �α��� ��Ŷ����, ������ ��� �� ������ �ٸ� ���
LONG g_lVerError;

// auth�� �α��� ��Ŷ����, �ߺ� �α��� ��
LONG g_DuplicateCount;

// GQCS���� �������� ���� �� 1 ����
extern LONG g_SemCount;



// !! �ӽ�. ���� ������� �ֽ� ���Ͽ��� ���� ���� !!
LONG g_Cartridge_Bullet = 10;





// ------------------
// CGameSession�� �Լ�
// (CBattleServer_Room�� �̳�Ŭ����)
// ------------------
namespace Library_Jingyu
{
	// ������
	CCrashDump* g_BattleServer_Room_Dump = CCrashDump::GetInstance();

	// �α׿�
	CSystemLog* g_BattleServer_RoomLog = CSystemLog::GetInstance();


	// -----------------------
	// �����ڿ� �Ҹ���
	// -----------------------

	// ������
	CBattleServer_Room::CGameSession::CGameSession()
		:CMMOServer::cSession()
	{
		// ClientKey �ʱⰪ ����
		m_int64ClientKey = -1;

		// �ڷᱸ���� �� �÷���
		m_bStructFlag = false;

		// �α��� ��Ŷ �Ϸ� ó��
		m_bLoginFlag = false;

		// �� ����� ���� üũ �÷���
		m_bLogoutFlag = false;

		// �������� �� �ʱ� ��ȣ
		m_iRoomNo = -1;

		// �α��� HTTP ��û Ƚ��
		m_lLoginHTTPCount = 0;

		// ���� ����
		m_bAliveFlag = false;	
	}

	// �Ҹ���
	CBattleServer_Room::CGameSession::~CGameSession()
	{
		// �Ұ� ����
	}

	// -----------------
	// �����Լ�
	// -----------------

	// --------------- AUTH ���� �Լ�

	// ������ Auth ���� �����
	//
	// Parameter : ����
	// return : ����
	void CBattleServer_Room::CGameSession::OnAuth_ClientJoin()
	{
		// �Ұ� ����
	}

	// ������ Auth ��忡�� ����
	//
	// Parameter : Game���� ����Ȱ����� �˷��ִ� Flag. ����Ʈ false(������� ����)
	// return : ����
	void CBattleServer_Room::CGameSession::OnAuth_ClientLeave(bool bGame)
	{
		// ��� ������ ���
		if (bGame == true)
		{
			
		}

		// ���� ���� ������ ���
		else
		{
			// ClientKey �ʱⰪ���� ����.
			// !! �̰� ���ϸ�, Release�Ǵ� �߿� HTTP������ �� ��� !!
			// !! Auth_Update���� �̹� ����ť�� ��� ������ �������� �� SendPacket ���ɼ� !!
			// !! �� ���, ���� ������ �������� Send�� �� �� ���� !!
			m_int64ClientKey = -1;

			// �ش� ������ �ִ� �� �ȿ��� ���� ����
			// -1�� �ƴϸ� �뿡 �ִ°�.
			if (m_iRoomNo != -1)
			{
				AcquireSRWLockShared(&m_pParent->m_Room_Umap_srwl);		// ----- Room �ڷᱸ�� Shard �� 

				// 1. �� �˻�
				auto FindRoom = m_pParent->m_Room_Umap.find(m_iRoomNo);

				// ���� ������ Crash
				if (FindRoom == m_pParent->m_Room_Umap.end())
				{
					ReleaseSRWLockShared(&m_pParent->m_Room_Umap_srwl);		// ----- Room �ڷᱸ�� Shard ��� 
					g_BattleServer_Room_Dump->Crash();
				}

				stRoom* NowRoom = FindRoom->second;

				// �� ���°� Play�� Crash. ���� �ȵ�
				// Auth��忡�� ����Ǵ� ������ �ִ� ����, ������ ���/�غ� ���̾�� ��				
				if (NowRoom->m_iRoomState == eu_ROOM_STATE::PLAY_ROOM)
				{
					ReleaseSRWLockShared(&m_pParent->m_Room_Umap_srwl);		// ----- Room �ڷᱸ�� Shard ��� 
					g_BattleServer_Room_Dump->Crash();
				}

				// ������� ���� �� ��尡 WAIT_ROOM Ȥ�� READY_ROOM Ȯ��.
				// ��, Auth������ �����ϴ� �� Ȯ��. �� Ǭ��.
				// Ȥ��, �� Ǯ���µ� ������ ���ɼ��� ����.
				// Auth���� �������ε� ���� �����Ǵ°��� ���� �ȵ�. 
				// ���� Play��忡�� �����ȴ�
				ReleaseSRWLockShared(&m_pParent->m_Room_Umap_srwl);		// ----- Room �ڷᱸ�� Shard ��� 


				// 2. ���� �� �ȿ� �ִ� ���� �� ����
				--NowRoom->m_iJoinUserCount;

				// ���� ��, �� ���� ���� ���� 0���� ������ ���� �ȵ�.
				if (NowRoom->m_iJoinUserCount < 0)				
					g_BattleServer_Room_Dump->Crash();


				// 3. �� ���� �ڷᱸ������ ���� ����
				if (NowRoom->Erase(this) == false)
					g_BattleServer_Room_Dump->Crash();

				m_iRoomNo = -1;


				// 4. �� ���� ��� ��������, �濡�� ������ �����ٰ� �˷���
				// BroadCast�ص� �ȴ�. ������ �� �ȿ� ���� ���� ������ ����.
				CProtocolBuff_Net* SendBuff = CProtocolBuff_Net::Alloc();

				WORD Type = en_PACKET_CS_GAME_RES_REMOVE_USER;

				SendBuff->PutData((char*)&Type, 2);
				SendBuff->PutData((char*)&m_iRoomNo, 4);
				SendBuff->PutData((char*)&m_Int64AccountNo, 8);

				// ���⼭, false�� ���ϵ� �� ����. (�� ���� �ڷᱸ���� ������ 0��)
				// ������ ���¿��� ��� ������ ���� �� �ֱ� ������ ������ �Ⱥ���.
				// ������ return �ȹ���
				NowRoom->SendPacket_BroadCast(SendBuff);


				// 5. ������ �������� �ش� ���� �����ٰ� ��Ŷ ������� ��.
				m_pParent->m_Master_LanClient->Packet_RoomLeave_Req(m_iRoomNo, m_Int64AccountNo);
			}			
		}

	}

	// Auth ����� �������� packet�� ��
	//
	// Parameter : ��Ŷ (CProtocolBuff_Net*)
	// return : ����
	void CBattleServer_Room::CGameSession::OnAuth_Packet(CProtocolBuff_Net* Packet)
	{
		// ��Ŷ ó��
		try
		{
			// 1. Ÿ�� ����
			WORD Type;
			Packet->GetData((char*)&Type, 2);

			// 2. Ÿ�Կ� ���� �б� ó��
			switch (Type)
			{
				// �α��� ��û
			case en_PACKET_CS_GAME_REQ_LOGIN:
				Auth_LoginPacket(Packet);
				break;

				// �� ���� ��û
			case en_PACKET_CS_GAME_REQ_ENTER_ROOM:
				break;

				// ��Ʈ��Ʈ
			case en_PACKET_CS_GAME_REQ_HEARTBEAT:
				break;

				// �� �ܿ��� ��������.
			default:
				throw CException(_T("OnAuth_Packet() --> Type Error!!"));
				break;
			}
		}
		catch (CException& exc)
		{
			// �α� ��� (�α� ���� : ����)
			g_BattleServer_RoomLog->LogSave(L"CBattleServer_Room", CSystemLog::en_LogLevel::LEVEL_ERROR, L"%s",
				(TCHAR*)exc.GetExceptionText());

			// ����
			g_BattleServer_Room_Dump->Crash();
		}

	}



	// --------------- GAME ���� �Լ�

	// ������ Game���� �����
	//
	// Parameter : ����
	// return : ����
	void CBattleServer_Room::CGameSession::OnGame_ClientJoin()
	{
		// �Ұ� ����
	}

	// ������ Game��忡�� ����
	//
	// Parameter : ����
	// return : ����
	void CBattleServer_Room::CGameSession::OnGame_ClientLeave()
	{
		// ClientKey �ʱⰪ���� ����.
		// !! �̰� ���ϸ�, Release�Ǵ� �߿� HTTP������ �� ��� !!
		// !! Game_Update���� �̹� ����ť�� ��� ������ �������� �� SendPacket ���ɼ� !!
		// !! �� ���, ���� ������ �������� Send�� �� �� ���� !!
		m_int64ClientKey = -1;


		// �ش� ������ �ִ� �� �ȿ��� ���� ����
		// -1�� ���� ���� �ȵ�. ������ �濡 �־�� ��
		if(m_iRoomNo == -1)
			g_BattleServer_Room_Dump->Crash();


		AcquireSRWLockShared(&m_pParent->m_Room_Umap_srwl);		// ----- Room �ڷᱸ�� Shard �� 

		// 1. �� �˻�
		auto FindRoom = m_pParent->m_Room_Umap.find(m_iRoomNo);

		// ���� ������ Crash
		if (FindRoom == m_pParent->m_Room_Umap.end())
		{
			ReleaseSRWLockShared(&m_pParent->m_Room_Umap_srwl);		// ----- Room �ڷᱸ�� Shard ��� 
			g_BattleServer_Room_Dump->Crash();
		}

		stRoom* NowRoom = FindRoom->second;

		// �� ���°� Play�� �ƴϸ� Crash. ���� �ȵ�
		// Game��忡�� ����Ǵ� ������ �ִ� ����, ������ �÷��� ���̾�� ��
		if (NowRoom->m_iRoomState != eu_ROOM_STATE::PLAY_ROOM)
		{
			ReleaseSRWLockShared(&m_pParent->m_Room_Umap_srwl);		// ----- Room �ڷᱸ�� Shard ��� 
			g_BattleServer_Room_Dump->Crash();
		}

		// ������� ���� �� ��尡 PLAY_ROOM Ȯ��.
		// ��, Game������ �����ϴ� �� Ȯ��. �� Ǭ��.
		// �� Ǯ���µ� ���� ������ ���ɼ��� ���� ����. 
		// �� ������ Game �����尡 �ϰ�, �ش� �Լ��� Game �����忡�� ȣ��ȴ�. 
		ReleaseSRWLockShared(&m_pParent->m_Room_Umap_srwl);		// ----- Room �ڷᱸ�� Shard ��� 


		// 2. ���� �� �ȿ� �ִ� ���� �� ����
		--NowRoom->m_iJoinUserCount;

		// ���� ��, �� ���� ���� ���� 0���� ������ ���� �ȵ�.
		if (NowRoom->m_iJoinUserCount < 0)
			g_BattleServer_Room_Dump->Crash();

		// 3. ������ ���� �� ����.
		// ������ ���� ���� �¸����θ� �Ǵ��ؾ� �ϱ� ������ 
		// ������ ���ӿ��� �����ų� HP�� 0�̵Ǿ� ����� ��� ���ҽ��Ѿ� ��
		--NowRoom->m_iAliveUserCount;

		// ���� ��, 0���� ������ ���� �ȵ�.
		if (NowRoom->m_iAliveUserCount < 0)
			g_BattleServer_Room_Dump->Crash();


		// 4. �� ���� �ڷᱸ������ ���� ����
		if (NowRoom->Erase(this) == false)
			g_BattleServer_Room_Dump->Crash();

		m_iRoomNo = -1;

	}

	// Game ����� �������� packet�� ��
	//
	// Parameter : ��Ŷ (CProtocolBuff_Net*)
	// return : ����
	void CBattleServer_Room::CGameSession::OnGame_Packet(CProtocolBuff_Net* Packet)
	{
		// Ÿ�Կ� ���� ��Ŷ ó��
		try
		{
			// 1. Ÿ�� ����
			WORD Type;
			Packet->GetData((char*)&Type, 2);

			// 2. Ÿ�Կ� ���� �б⹮ ó��
			switch (Type)
			{	

				// ��Ʈ��Ʈ
			case en_PACKET_CS_GAME_REQ_HEARTBEAT:
				break;

				// �� �ܿ��� ���� ����
			default:
				throw CException(_T("OnGame_Packet() --> Type Error!!"));
				break;
			}

		}
		catch (CException& exc)
		{
			// �α� ��� (�α� ���� : ����)
			g_BattleServer_RoomLog->LogSave(L"CBattleServer_Room", CSystemLog::en_LogLevel::LEVEL_ERROR, L"%s",
				(TCHAR*)exc.GetExceptionText());

			// ����
			g_BattleServer_Room_Dump->Crash();
		}

	}



	// --------------- Release ���� �Լ�

	// Release�� ����.
	//
	// Parameter : ����
	// return : ����
	void CBattleServer_Room::CGameSession::OnGame_ClientRelease()
	{
		// m_bStructFlag�� true���, �ڷᱸ���� �� ����
		if (m_bStructFlag == true)
		{
			if (m_pParent->EraseAccountNoFunc(m_Int64AccountNo) == false)
				g_BattleServer_Room_Dump->Crash();

			m_bStructFlag = false;
		}
		
		m_bLoginFlag = false;	
		m_bLogoutFlag = false;
		m_bAliveFlag = false;
		m_lLoginHTTPCount = 0;		
	}




	// -----------------
	// Auth��� ��Ŷ ó�� �Լ�
	// -----------------

	// �α��� ��û 
	//
	// Parameter : CProtocolBuff_Net*
	// return : ����
	void CBattleServer_Room::CGameSession::Auth_LoginPacket(CProtocolBuff_Net* Packet)
	{
		// 1. ���� ClientKey�� �ʱⰪ�� �ƴϸ� Crash
		if (m_int64ClientKey != -1)
			g_BattleServer_Room_Dump->Crash();


		// 2. AccountNo, ClientKey ������
		INT64 AccountNo;
		INT64 ClinetKey;

		Packet->GetData((char*)&AccountNo, 8);
		Packet->GetData((char*)&ClinetKey, 8);



		// 3. AccountNo, ClientKey ����
		m_Int64AccountNo = AccountNo;
		m_int64ClientKey = ClinetKey;



		// 4. AccountNo �ڷᱸ���� �߰�.
		// �̹� ������(false ����) �ߺ� �α������� ó��
		// ���� �������Դ� ���� ��Ŷ, ���� ���� ������ DIsconnect.
		if (m_pParent->InsertAccountNoFunc(AccountNo, this) == false)
		{
			InterlockedIncrement(&g_DuplicateCount);			

			// �α��� ���� ��Ŷ ������ ����.
			CProtocolBuff_Net* SendBuff = CProtocolBuff_Net::Alloc();

			// Ÿ��
			WORD Type = en_PACKET_CS_GAME_RES_LOGIN;
			SendBuff->PutData((char*)&Type, 2);

			// Status
			BYTE Status = 6;		// �ߺ� �α���
			SendBuff->PutData((char*)&Status, 1);

			// AccountNo
			SendBuff->PutData((char*)&AccountNo, 8);			

			// SendPacket
			SendPacket(SendBuff);

			// �ߺ� �α��� ó���� ���� ���� ���� ��û
			m_pParent->DisconnectAccountNoFunc(AccountNo);

			return;
		}

		// �ڷᱸ���� �� �÷��� ����
		m_bStructFlag = true;



		// 5. �α��� ����ó���� ���� HTTP ���
		// ��� ��, �ش� ��Ŷ�� ���� ��ó���� Auth_Update���� �ѱ��.
		DB_WORK_LOGIN* Send_A = (DB_WORK_LOGIN*)m_pParent->m_shDB_Communicate.m_pDB_Work_Pool->Alloc();

		Send_A->m_wWorkType = eu_DB_READ_TYPE::eu_LOGIN_AUTH;
		Send_A->m_i64UniqueKey = ClinetKey;
		Send_A->pPointer = this;

		// ����ȭ ���� ���� ����, ���۷��� ī��Ʈ Add
		Packet->Add();
		Send_A->m_pBuff = Packet;

		Send_A->AccountNo = AccountNo;

		// Select_Account.php ��û
		m_pParent->m_shDB_Communicate.DBReadFunc((DB_WORK*)Send_A, en_PHP_TYPE::SELECT_ACCOUNT);


		// 6. ���� ���� ������ ���� HTTP ���
		// ��� ��, �ش� ��Ŷ�� ���� ��ó���� Auth_Update���� �ѱ��.
		DB_WORK_LOGIN_CONTENTS* Send_B = (DB_WORK_LOGIN_CONTENTS*)m_pParent->m_shDB_Communicate.m_pDB_Work_Pool->Alloc();
	
		Send_A->m_wWorkType = eu_DB_READ_TYPE::eu_LOGIN_INFO;
		Send_A->m_i64UniqueKey = ClinetKey;
		Send_A->pPointer = this;

		Send_A->AccountNo = AccountNo;

		// Select_Contents.php ��û
		m_pParent->m_shDB_Communicate.DBReadFunc((DB_WORK*)Send_A, en_PHP_TYPE::SELECT_CONTENTS);	
	}


	// �� ���� ��û
	// 
	// Parameter : CProtocolBuff_Net*
	// return : ����
	void CBattleServer_Room::CGameSession::Auth_RoomEnterPacket(CProtocolBuff_Net* Packet)
	{
		// �α��� ���� üũ
		if(m_bLoginFlag == false)
			g_BattleServer_Room_Dump->Crash();

		// 1. ������
		INT64 AccountNo;
		int RoomNo;
		char EnterToken[32];

		Packet->GetData((char*)&AccountNo, 8);
		Packet->GetData((char*)&RoomNo, 4);
		Packet->GetData(EnterToken, 32);		

		// 2. ���� üũ
		if (AccountNo != m_Int64AccountNo)
			g_BattleServer_Room_Dump->Crash();
		
		AcquireSRWLockShared(&m_pParent->m_Room_Umap_srwl);	// ----- Room umap Shared ��

		
		// 3. �����ϰ��� �ϴ� �� �˻�
		auto ret = m_pParent->m_Room_Umap.find(RoomNo);

		// 4. �� ���翩�� üũ
		if (ret == m_pParent->m_Room_Umap.end())
		{
			ReleaseSRWLockShared(&m_pParent->m_Room_Umap_srwl);	// ----- Room umap Shared ���

			// ���� ������, ���� ����
			CProtocolBuff_Net* SendBuff = CProtocolBuff_Net::Alloc();

			WORD Type = en_PACKET_CS_GAME_RES_ENTER_ROOM;
			BYTE Result = 4;
			BYTE MaxUser = 0;	// �� ���� ��Ŷ���� �ǹ̰� ������.. �׳� �ƹ��ų� ����.

			SendBuff->PutData((char*)&Type, 2);
			SendBuff->PutData((char*)&AccountNo, 8);
			SendBuff->PutData((char*)&RoomNo, 4);
			SendBuff->PutData((char*)&MaxUser, 1);
			SendBuff->PutData((char*)&Result, 1);

			SendPacket(SendBuff);

			return;
		}

		stRoom* NowRoom = ret->second;

		// 5. ���� ������ �ƴϸ� ���� 3 ���� (�غ�� or �÷��� ������ ���� ���)
		if (NowRoom->m_iRoomState != eu_ROOM_STATE::WAIT_ROOM)
		{
			ReleaseSRWLockShared(&m_pParent->m_Room_Umap_srwl);	// ----- Room umap Shared ���

			// ������ �ƴϸ�, ���� ����
			CProtocolBuff_Net* SendBuff = CProtocolBuff_Net::Alloc();

			WORD Type = en_PACKET_CS_GAME_RES_ENTER_ROOM;
			BYTE Result = 3;
			BYTE MaxUser = 0;	// ���� �ƴ� ��Ŷ���� �ǹ̰� ������.. �׳� �ƹ��ų� ����.

			SendBuff->PutData((char*)&Type, 2);
			SendBuff->PutData((char*)&AccountNo, 8);
			SendBuff->PutData((char*)&RoomNo, 4);
			SendBuff->PutData((char*)&MaxUser, 1);
			SendBuff->PutData((char*)&Result, 1);

			SendPacket(SendBuff);

			return;
		}
		
		// ������� ����, [�����ϴ� ����]�̶�°� Ȯ��. (Auth��忡�� �������� �� Ȯ��)
		// �� Ǭ��. ������ Auth�� �����ϴ� ���̴�.
		// �� Ǯ�� �߰��� ������ ���ɼ��� ����. 
		// �����Ǳ� ���ؼ��� Play����� ���� �Ǿ�� �ϴµ�, ��� ������ Auth �����尡 �Ѵ�.
		// �׸���, �ش� �Լ��� Auth �����忡�� ȣ��ȴ�.
		// ��, �ش� �Լ��� ������ ���� ���� ����� ���ɼ��� ����.
		ReleaseSRWLockShared(&m_pParent->m_Room_Umap_srwl);	// ----- Room umap Shared ���


		// 6. �� �ο��� üũ
		if (NowRoom->m_iJoinUserCount == NowRoom->m_iMaxJoinCount)
		{
			BYTE MaxUser = NowRoom->m_iJoinUserCount;		

			// �̹� �ִ� �ο������, ���� ����			
			CProtocolBuff_Net* SendBuff = CProtocolBuff_Net::Alloc();

			WORD Type = en_PACKET_CS_GAME_RES_ENTER_ROOM;
			BYTE Result = 5;

			SendBuff->PutData((char*)&Type, 2);
			SendBuff->PutData((char*)&AccountNo, 8);
			SendBuff->PutData((char*)&RoomNo, 4);
			SendBuff->PutData((char*)&MaxUser, 1);
			SendBuff->PutData((char*)&Result, 1);

			SendPacket(SendBuff);

			return;
		}

		// 7. �� ��ū üũ
		if (memcmp(EnterToken, NowRoom->m_cEnterToken, 32) != 0)
		{
			BYTE MaxUser = NowRoom->m_iJoinUserCount;		

			// ��ū�� �ٸ���, ���� ����
			CProtocolBuff_Net* SendBuff = CProtocolBuff_Net::Alloc();

			WORD Type = en_PACKET_CS_GAME_RES_ENTER_ROOM;
			BYTE Result = 2;

			SendBuff->PutData((char*)&Type, 2);
			SendBuff->PutData((char*)&AccountNo, 8);
			SendBuff->PutData((char*)&RoomNo, 4);
			SendBuff->PutData((char*)&MaxUser, 1);
			SendBuff->PutData((char*)&Result, 1);

			SendPacket(SendBuff);

			return;
		}


		// 8. ������� ���� ����
		// �뿡 �ο� �߰�
		NowRoom->m_iJoinUserCount++;
		NowRoom->Insert(this);

		int NowUser = NowRoom->m_iJoinUserCount;


		// 9. �������� �� ���� ���� ������
		CProtocolBuff_Net* SendBuff_A = CProtocolBuff_Net::Alloc();

		WORD Type = en_PACKET_CS_GAME_RES_ENTER_ROOM;
		BYTE Result = 1;

		SendBuff_A->PutData((char*)&Type, 2);
		SendBuff_A->PutData((char*)&AccountNo, 8);
		SendBuff_A->PutData((char*)&RoomNo, 4);
		SendBuff_A->PutData((char*)&NowUser, 1);
		SendBuff_A->PutData((char*)&Result, 1);

		SendPacket(SendBuff_A);


		// 10. �濡 �ִ� ��� �������� "���� �߰���" ��Ŷ ������
		// �̹��� ������ �ڱ� �ڽ� ����.
		Type = en_PACKET_CS_GAME_RES_ADD_USER;

		CProtocolBuff_Net* SendBuff_B = CProtocolBuff_Net::Alloc();

		SendBuff_B->PutData((char*)&Type, 2);
		SendBuff_B->PutData((char*)&RoomNo, 4);
		SendBuff_B->PutData((char*)&AccountNo, 8);
		SendBuff_B->PutData((char*)m_tcNickName, 40);

		SendBuff_B->PutData((char*)&m_iRecord_PlayCount, 4);
		SendBuff_B->PutData((char*)&m_iRecord_PlayTime, 4);
		SendBuff_B->PutData((char*)&m_iRecord_Kill, 4);
		SendBuff_B->PutData((char*)&m_iRecord_Die, 4);
		SendBuff_B->PutData((char*)&m_iRecord_Win, 4);

		if (NowRoom->SendPacket_BroadCast(SendBuff_B) == false)
			g_BattleServer_Room_Dump->Crash();



		// 11. �� �ο����� Ǯ ���� �Ǿ��� ���
		if (NowUser == NowRoom->m_iMaxJoinCount)
		{
			// 1) �����Ϳ��� �� ���� ��Ŷ ������
			m_pParent->m_Master_LanClient->Packet_RoomClose_Req(RoomNo);

			// 2) ��� �������� ī��Ʈ�ٿ� ��Ŷ ������
			CProtocolBuff_Net* SendBuff = CProtocolBuff_Net::Alloc();

			WORD Type = en_PACKET_CS_GAME_RES_PLAY_READY;
			BYTE ReadySec = m_pParent->m_stConst.m_bCountDownSec;		// �� ����

			SendBuff->PutData((char*)&Type, 2);
			SendBuff->PutData((char*)&RoomNo, 4);
			SendBuff->PutData((char*)&ReadySec, 1);

			if (NowRoom->SendPacket_BroadCast(SendBuff) == false)
				g_BattleServer_Room_Dump->Crash();

			// 3) �� ī��Ʈ�ٿ� ���� ����
			// �� �ð� + �������� ���� ī��Ʈ�ٿ�(10��)�� �Ǹ� �� ��带 Play�� �����Ű��
			// ������ AUTH_TO_GAME���� �����Ų��.
			NowRoom->m_dwCountDown = timeGetTime();

			// 4) ���� ���¸� Ready�� ����
			NowRoom->m_iRoomState = eu_ROOM_STATE::READY_ROOM;

			// 5) ���� �� ����
			// �ش� ī��Ʈ�� Auth ��忡���� �����ϱ� ������ ���Ͷ� �ʿ� ����.
			--m_pParent->m_lNowWaitRoomCount;
		}
	}



	// -----------------
	// Game��� ��Ŷ ó�� �Լ�
	// -----------------



}


// ------------------
// stRoom�� �Լ�
// (CBattleServer_Room�� �̳�Ŭ����)
// ------------------
namespace Library_Jingyu
{

	// ------------
	// ��� �Լ�
	// ------------

	// ������
	CBattleServer_Room::stRoom::stRoom()
	{
		// �̸� �޸� ���� ��Ƶα�
		m_JoinUser_Vector.reserve(10);

		// ������ ����Ǿ����� üũ�ϴ� �÷���
		m_bGameEndFlag = false;

		// ���� �˴ٿ� ������.
		m_bShutdownFlag = false;
	}

	// �ڷᱸ�� ���� ��� �������� ���ڷ� ���� ��Ŷ ������
	//
	// Parameter : CProtocolBuff_Net*
	// return : �ڷᱸ�� ���� ������ 0���� ��� false
	//		  : �� �ܿ��� true
	bool CBattleServer_Room::stRoom::SendPacket_BroadCast(CProtocolBuff_Net* SendBuff)
	{
		// 1. �ڷᱸ�� ���� ���� �� �ޱ�
		size_t Size = m_JoinUser_Vector.size();

		// 2. ���� ���� 0���̰ų� ���� ���, return false
		if (Size <= 0)
			return false;

		// !! while�� ���� ���� ī��Ʈ��, ���� �� ��ŭ ���� !!
		// �����ʿ���, �Ϸ� ������ ���� Free�� �ϱ� ������ Add�ؾ� �Ѵ�.
		SendBuff->Add((int)Size);

		// 3. ���� �� ��ŭ ���鼭 ��Ŷ ����.
		size_t Index = 0;

		while (Index < Size)
		{
			m_JoinUser_Vector[Index]->SendPacket(SendBuff);
			++Index;
		}

		// 4. ��Ŷ Free
		// !! ������ ���뿡�� Free ������, ���۷��� ī��Ʈ�� �ο� �� ��ŭ Add �߱� ������ 1���� �� ������ ����. !!	
		CProtocolBuff_Net::Free(SendBuff);

		return true;
	}

	// �� ���� ��� ������ Auth_To_Game���� ����
	//
	// Parameter : ����
	// return : ����
	void CBattleServer_Room::stRoom::ModeChange()
	{
		// 1. �ڷᱸ�� ���� ���� �� �ޱ�
		size_t Size = m_JoinUser_Vector.size();

		// 2. ���� ���� 0���� ���ɼ� ����.
		// ī��Ʈ �ٿ� �߿� ��� ������ ���� ���.
		// ������ �Ⱥ���.
		if (Size == 0)
			return;

		// 3. 0���� �ƴ϶��, ���ο� �ִ� ��� ������ AUTH_TO_GAME���� ����
		size_t Index = 0;

		while (Index < Size)
		{
			m_JoinUser_Vector[Index]->SetMode_GAME();
			++Index;
		}
	}

	// �� ���� ��� ������ �������·� ����
	//
	// Parameter : ����
	// return : �ڷᱸ�� ���� ������ 0���� ��� false
	//		  : �� �ܿ��� true
	bool CBattleServer_Room::stRoom::AliveFalg_True()
	{
		// 1. �ڷᱸ�� ���� ���� �� �ޱ�
		size_t Size = m_JoinUser_Vector.size();

		// 2. ���� ���� 0���̰ų� 0������ ���, return false
		if (Size <= 0)
			return false;

		// 3. ���� �� ��ŭ ���鼭 ���� �÷��� ����
		size_t Index = 0;

		while (Index < Size)
		{
			m_JoinUser_Vector[Index]->m_bAliveFlag = true;
			++Index;
		}

		return true;
	}

	// �� ���� �����鿡�� ���� ���� ��Ŷ ������
	//
	// Parameter : ����
	// return : ����
	void CBattleServer_Room::stRoom::GameOver()
	{
		// 1. �� �ȿ� ������ 0���� ��� Crash
		if (m_iJoinUserCount == 0)
			g_BattleServer_Room_Dump->Crash();

		// 2. �� �ο��� üũ ������ vector ���� ����� �޶� Crash
		size_t Size = m_JoinUser_Vector.size();

		if(Size == 0 || Size != m_iJoinUserCount)
			g_BattleServer_Room_Dump->Crash();		
			   

		// 3. �¸��ڿ��� ���� �¸���Ŷ �����
		CProtocolBuff_Net* winPacket = CProtocolBuff_Net::Alloc();

		WORD Type = en_PACKET_CS_GAME_RES_WINNER;
		winPacket->PutData((char*)&Type, 2);


		// 4. �й��ڿ��� ���� �й� ��Ŷ �����
		CProtocolBuff_Net* LosePacket = CProtocolBuff_Net::Alloc();

		Type = en_PACKET_CS_GAME_RES_GAMEOVER;
		LosePacket->PutData((char*)&Type, 2);


		// 5. ��ȸ�ϸ鼭 ��Ŷ ������
		size_t Index = 0;
		int WinUserCount = 0;

		while (Index < Size)
		{
			// ������ �¸����� ��� (������)
			if (m_JoinUser_Vector[Index]->m_bAliveFlag == true)
			{
				// �� ���ӿ� �¸��ڴ� 1��.
				// WinUserCount�� 1�ε� ���� ���Դٸ�, �¸��ڰ� 1�� �̻��� �Ǿ��ٴ� �ǹ�.
				// ���� �ȵ�.
				if(WinUserCount == 1)
					g_BattleServer_Room_Dump->Crash();

				// �¸� ��Ŷ ������
				m_JoinUser_Vector[Index]->SendPacket(winPacket);

				// �¸���Ŷ ���� �� ����
				++WinUserCount;
			}

			// ������ �й����� ��� (�����)
			else
			{
				// ���۷��� ī��Ʈ ����.
				LosePacket->Add();

				// �й� ��Ŷ ������
				m_JoinUser_Vector[Index]->SendPacket(LosePacket);
			}

			++Index;
		}

		// 6. �й� ��Ŷ�� ���۷��� ī��Ʈ ����.
		CProtocolBuff_Net::Free(LosePacket);
	}

	// �� ���� �����鿡�� �˴ٿ� ������
	//
	// Parameter : ����
	// return : ����
	void CBattleServer_Room::stRoom::Shutdown_All()
	{
		// 1. �� �ȿ� ������ 0���̸� ���� �ȵ�. �ۿ��� �̹� 0�� �ƴ϶�°��� �˰� �Ա� ������.
		if (m_iJoinUserCount <= 0)
			g_BattleServer_Room_Dump->Crash();

		// 2. ���� �ȿ� 0���̸� ũ����
		size_t Size = m_JoinUser_Vector.size();
		if(Size <= 0)
			g_BattleServer_Room_Dump->Crash();

		// 3. ���� ���� ������ ������ �ٸ��� ũ����
		if(m_iJoinUserCount != Size)
			g_BattleServer_Room_Dump->Crash();

		// 4. ��ȸ�ϸ� Shutdown
		size_t Index = 0;

		while (Index < Size)
		{
			m_JoinUser_Vector[Index]->Disconnect();

			++Index;
		}	
	}


	// ------------
	// �ڷᱸ�� �Լ�
	// ------------

	// �ڷᱸ���� Insert
	//
	// Parameter : �߰��ϰ��� �ϴ� CGameSession*
	// return : ����
	void CBattleServer_Room::stRoom::Insert(CGameSession* InsertPlayer)
	{
		m_JoinUser_Vector.push_back(InsertPlayer);
	}

	// �ڷᱸ������ Erase
	//
	// Parameter : �����ϰ��� �ϴ� CGameSession*
	// return : ���� �� true
	//		  : ���� ��  false
	bool CBattleServer_Room::stRoom::Erase(CGameSession* InsertPlayer)
	{
		size_t Size = m_JoinUser_Vector.size();
		bool Flag = false;

		// 1. �ڷᱸ�� �ȿ� ������ 0���� �۰ų� ������ return false
		if (Size <= 0)
			return false;

		// 2. �ڷᱸ�� �ȿ� ������ 1���̰ų�, ã���� �ϴ� ������ �������� �ִٸ� �ٷ� ����
		if (Size == 1 || m_JoinUser_Vector[Size - 1] == InsertPlayer)
		{
			Flag = true;
			m_JoinUser_Vector.pop_back();
		}

		// 3. �ƴ϶�� Swap �Ѵ�
		else
		{
			size_t Index = 0;
			while (Index < Size)
			{
				// ���� ã���� �ϴ� ������ ã�Ҵٸ�
				if (m_JoinUser_Vector[Index] == InsertPlayer)
				{
					Flag = true;

					CGameSession* Temp = m_JoinUser_Vector[Size - 1];
					m_JoinUser_Vector[Size - 1] = m_JoinUser_Vector[Index];
					m_JoinUser_Vector[Index] = Temp;

					m_JoinUser_Vector.pop_back();

					break;
				}

				++Index;
			}
		}

		// 4. ����, ���� ���ߴٸ� return false
		if (Flag == false)
			return false;

		return true;
	}

}


// ----------------------------------------
// 
// MMOServer�� �̿��� ��Ʋ ����. �� ����
//
// ----------------------------------------
namespace Library_Jingyu
{
	// Net ����ȭ ���� 1���� ũ�� (Byte)
	LONG g_lNET_BUFF_SIZE = 512;
		   	  


	// -----------------------
	// ���ο����� ����ϴ� �Լ�
	// -----------------------

	// ���Ͽ��� Config ���� �о����
	// 
	// Parameter : config ����ü
	// return : ���������� ���� �� true
	//		  : �� �ܿ��� false
	bool CBattleServer_Room::SetFile(stConfigFile* pConfig)
	{
		Parser Parser;

		// ���� �ε�
		try
		{
			Parser.LoadFile(_T("MMOGameServer_Config.ini"));
		}
		catch (int expn)
		{
			if (expn == 1)
			{
				printf("File Open Fail...\n");
				return false;
			}
			else if (expn == 2)
			{
				printf("FileR Read Fail...\n");
				return false;
			}
		}



		////////////////////////////////////////////////////////
		// BATTLESERVER config �о����
		////////////////////////////////////////////////////////

		// ���� ���� -------------------------
		if (Parser.AreaCheck(_T("BATTLESERVER")) == false)
			return false;

		// IP
		if (Parser.GetValue_String(_T("BindIP"), pConfig->BindIP) == false)
			return false;

		// Port
		if (Parser.GetValue_Int(_T("Port"), &pConfig->Port) == false)
			return false;

		// ���� ��Ŀ ��
		if (Parser.GetValue_Int(_T("CreateWorker"), &pConfig->CreateWorker) == false)
			return false;

		// Ȱ��ȭ ��Ŀ ��
		if (Parser.GetValue_Int(_T("ActiveWorker"), &pConfig->ActiveWorker) == false)
			return false;

		// ���� ����Ʈ
		if (Parser.GetValue_Int(_T("CreateAccept"), &pConfig->CreateAccept) == false)
			return false;

		// ��� �ڵ�
		if (Parser.GetValue_Int(_T("HeadCode"), &pConfig->HeadCode) == false)
			return false;

		// xorcode1
		if (Parser.GetValue_Int(_T("XorCode1"), &pConfig->XORCode1) == false)
			return false;

		// xorcode2
		if (Parser.GetValue_Int(_T("XorCode2"), &pConfig->XORCode2) == false)
			return false;

		// Nodelay
		if (Parser.GetValue_Int(_T("Nodelay"), &pConfig->Nodelay) == false)
			return false;

		// �ִ� ���� ���� ���� ��
		if (Parser.GetValue_Int(_T("MaxJoinUser"), &pConfig->MaxJoinUser) == false)
			return false;

		// �α� ����
		if (Parser.GetValue_Int(_T("LogLevel"), &pConfig->LogLevel) == false)
			return false;




		////////////////////////////////////////////////////////
		// �⺻ config �о����
		////////////////////////////////////////////////////////

		// ���� ���� -------------------------
		if (Parser.AreaCheck(_T("CONFIG")) == false)
			return false;

		// VerCode
		if (Parser.GetValue_Int(_T("VerCode"), &m_uiVer_Code) == false)
			return false;

		// BattleNetServer�� IP
		if (Parser.GetValue_String(_T("BattleNetServerIP"), m_Master_LanClient->m_tcBattleNetServerIP) == false)
			return false;

		// MasterNetServer�� Port ����.
		m_Master_LanClient->m_iBattleNetServerPort = pConfig->Port;

		// ChatNetServer�� IP
		if (Parser.GetValue_String(_T("ChatNetServerIP"), m_Master_LanClient->m_tcChatNetServerIP) == false)
			return false;

		// ChatNetServer�� Port
		if (Parser.GetValue_Int(_T("ChatNetServerPort"), &m_Master_LanClient->m_iChatNetServerPort) == false)
			return false;

		// ������ ������ ���� �� ��� ��ū
		TCHAR m_tcMasterToken[33];
		if (Parser.GetValue_String(_T("MasterEnterToken"), m_tcMasterToken) == false)
			return false;

		// ��� ���� char������ ����� ������ ��ȯ�ؼ� ������ �־���Ѵ�.
		int len = WideCharToMultiByte(CP_UTF8, 0, m_tcMasterToken, lstrlenW(m_tcMasterToken), 0, 0, 0, 0);
		WideCharToMultiByte(CP_UTF8, 0, m_tcMasterToken, lstrlenW(m_tcMasterToken), m_Master_LanClient->m_cMasterToken, len, 0, 0);


		





		////////////////////////////////////////////////////////
		// ������ LanClient config �о����
		////////////////////////////////////////////////////////

		// ���� ���� -------------------------
		if (Parser.AreaCheck(_T("MASTERLANCLIENT")) == false)
			return false;

		// IP
		if (Parser.GetValue_String(_T("MasterServerIP"), pConfig->MasterServerIP) == false)
			return false;

		// Port
		if (Parser.GetValue_Int(_T("MasterServerPort"), &pConfig->MasterServerPort) == false)
			return false;

		// ���� ��Ŀ ��
		if (Parser.GetValue_Int(_T("MasterClientCreateWorker"), &pConfig->MasterClientCreateWorker) == false)
			return false;

		// Ȱ��ȭ ��Ŀ ��
		if (Parser.GetValue_Int(_T("MasterClientActiveWorker"), &pConfig->MasterClientActiveWorker) == false)
			return false;


		// Nodelay
		if (Parser.GetValue_Int(_T("MasterClientNodelay"), &pConfig->MasterClientNodelay) == false)
			return false;




		////////////////////////////////////////////////////////
		// ����͸� LanClient config �о����
		////////////////////////////////////////////////////////

		// ���� ���� -------------------------
		if (Parser.AreaCheck(_T("MONITORLANCLIENT")) == false)
			return false;

		// IP
		if (Parser.GetValue_String(_T("MonitorServerIP"), pConfig->MonitorServerIP) == false)
			return false;

		// Port
		if (Parser.GetValue_Int(_T("MonitorServerPort"), &pConfig->MonitorServerPort) == false)
			return false;

		// ���� ��Ŀ ��
		if (Parser.GetValue_Int(_T("MonitorClientCreateWorker"), &pConfig->MonitorClientCreateWorker) == false)
			return false;

		// Ȱ��ȭ ��Ŀ ��
		if (Parser.GetValue_Int(_T("MonitorClientActiveWorker"), &pConfig->MonitorClientActiveWorker) == false)
			return false;


		// Nodelay
		if (Parser.GetValue_Int(_T("MonitorClientNodelay"), &pConfig->MonitorClientNodelay) == false)
			return false;


		return true;
	}

	// Start
	// ���������� CMMOServer�� Start, ���� ���ñ��� �Ѵ�.
	//
	// Parameter : ����
	// return : ���� �� false
	bool CBattleServer_Room::BattleServerStart()
	{
		// 1. ���� ����
		m_cGameSession = new CGameSession[m_stConfig.MaxJoinUser];

		int i = 0;
		while (i < m_stConfig.MaxJoinUser)
		{
			// GameServer�� ������ ����
			m_cGameSession[i].m_pParent = this;

			// ������ ���� ����
			SetSession(&m_cGameSession[i], m_stConfig.MaxJoinUser);
			++i;
		}

		// 2. ����͸� ������ ����Ǵ�, �� Ŭ���̾�Ʈ ����
		if (m_Monitor_LanClient->ClientStart(m_stConfig.MonitorServerIP, m_stConfig.MonitorServerPort, m_stConfig.MonitorClientCreateWorker,
			m_stConfig.MonitorClientActiveWorker, m_stConfig.MonitorClientNodelay) == false)
			return false;

		// 3. ������ ������ ����Ǵ�, �� Ŭ���̾�Ʈ ����
		if(m_Master_LanClient->ClientStart(m_stConfig.MasterServerIP, m_stConfig.MasterServerPort, m_stConfig.MasterClientCreateWorker,
			m_stConfig.MasterClientActiveWorker, m_stConfig.MasterClientNodelay) == false)
			return false;

		// 4. Battle ���� ����
		if (Start(m_stConfig.BindIP, m_stConfig.Port, m_stConfig.CreateWorker, m_stConfig.ActiveWorker, m_stConfig.CreateAccept,
			m_stConfig.Nodelay, m_stConfig.MaxJoinUser, m_stConfig.HeadCode, m_stConfig.XORCode1, m_stConfig.XORCode2) == false)
			return false;		

		// ���� ���� �α� ���		
		g_BattleServer_RoomLog->LogSave(L"CBattleServer_Room", CSystemLog::en_LogLevel::LEVEL_SYSTEM, L"ServerOpen...");

		return true;

	}

	// Stop
	// ���������� Stop ����
	//
	// Parameter : ����
	// return : ����
	void CBattleServer_Room::BattleServerStop()
	{
		// 1. ����͸� Ŭ�� ����
		if (m_Monitor_LanClient->GetClinetState() == true)
			m_Monitor_LanClient->ClientStop();

		// 2. ������ Lan Ŭ�� ����
		if (m_Master_LanClient->GetClinetState() == true)
			m_Master_LanClient->ClientStop();

		// 3. ���� ����
		if (GetServerState() == true)
			Stop();

		// 3. ���� ����
		delete[] m_cGameSession;
	}

	// ��¿� �Լ�
	//
	// Parameter : ����
	// return : ����
	void CBattleServer_Room::ShowPrintf()
	{

	}



	// -----------------------
	// ��Ŷ ��ó�� �Լ�
	// -----------------------

	// Login ��Ŷ�� ���� ���� ó�� (��ū üũ ��..)
	//
	// Parameter : DB_WORK_LOGIN*
	// return : ����
	void CBattleServer_Room::Auth_LoginPacket_AUTH(DB_WORK_LOGIN* DBData)
	{
		// 1. ClientKey üũ
		CGameSession* NowPlayer = (CGameSession*)DBData->pPointer;

		// �ٸ��� �̹� ������ ������ �Ǵ�. ���ɼ� �ִ� ��Ȳ
		if (NowPlayer->m_int64ClientKey != DBData->m_i64UniqueKey)
			return;


		// 2. Json������ �Ľ��ϱ� (UTF-16)
		GenericDocument<UTF16<>> Doc;
		Doc.Parse(DBData->m_tcResponse);

		int iResult = Doc[_T("result")].GetInt();


		// 3. DB ��û ��� Ȯ��
		// ����� 1�� �ƴ϶��, 
		if (iResult != 1)
		{
			// �̹� ���� ��Ŷ�� �� �������� üũ
			if (NowPlayer->m_bLogoutFlag == true)
				return;

			// �ٸ� HTTP ��û�� ���� ��Ŷ�� �� ������ ���ϵ��� �÷��� ����.
			NowPlayer->m_bLogoutFlag = true;

			// ���� �α� ���
			// �α� ��� (�α� ���� : ����)
			g_BattleServer_RoomLog->LogSave(L"CBattleServer_Room", CSystemLog::en_LogLevel::LEVEL_ERROR, 
				L"Auth_LoginPacket_AUTH()--> DB Result Error!! AccoutnNo : %lld, Error : %d", NowPlayer->m_Int64AccountNo, iResult);

			WORD Type = en_PACKET_CS_GAME_RES_LOGIN;
			BYTE Result = 2;			

			// ����� -10�� ��� (ȸ������ ��ü�� �ȵǾ� ����)
			if (iResult == -10)
				InterlockedIncrement(&g_lQuery_Result_Not_Find);

			// �� �� ��Ÿ ������ ���
			else
				InterlockedIncrement(&g_lTempError);

			CProtocolBuff_Net* SendBuff = CProtocolBuff_Net::Alloc();

			SendBuff->PutData((char*)&Type, 2);
			SendBuff->PutData((char*)&DBData->AccountNo, 8);
			SendBuff->PutData((char*)&Result, 1);

			NowPlayer->SendPacket(SendBuff);
			return;
		}


		// 4. SessionKey, ConnectToekn, Ver_Code ������
		char SessionKey[64];
		char ConnectToken[32];
		UINT VerCode;

		DBData->m_pBuff->GetData(SessionKey, 64);
		DBData->m_pBuff->GetData(ConnectToken, 32);
		DBData->m_pBuff->GetData((char*)&VerCode, 4);


		// 5. ��ūŰ ��
		const TCHAR* tDBToekn = Doc[_T("sessionkey")].GetString();

		char DBToken[64];
		int len = (int)_tcslen(tDBToekn);
		WideCharToMultiByte(CP_UTF8, 0, tDBToekn, (int)_tcslen(tDBToekn), DBToken, len, NULL, NULL);

		if (memcmp(DBToken, SessionKey, 64) != 0)
		{
			// �̹� ���� ��Ŷ�� �� �������� üũ
			if (NowPlayer->m_bLogoutFlag == true)
				return;

			// ���� �α� ���
			// �α� ��� (�α� ���� : ����)
			g_BattleServer_RoomLog->LogSave(L"CBattleServer_Room", CSystemLog::en_LogLevel::LEVEL_ERROR,
				L"Auth_LoginPacket_AUTH()--> SessionKey Error!! AccoutnNo : %lld", NowPlayer->m_Int64AccountNo);

			// �ٸ� HTTP ��û�� ���� ��Ŷ�� �� ������ ���ϵ��� �÷��� ����.
			NowPlayer->m_bLogoutFlag = true;

			// ��ū�� �ٸ���� Result 3(����Ű ����)�� ������.
			InterlockedIncrement(&g_lTokenError);

			WORD Type = en_PACKET_CS_MATCH_RES_LOGIN;
			BYTE Result = 3;

			CProtocolBuff_Net* SendBuff = CProtocolBuff_Net::Alloc();

			SendBuff->PutData((char*)&Type, 2);
			SendBuff->PutData((char*)&DBData->AccountNo, 8);
			SendBuff->PutData((char*)&Result, 1);

			NowPlayer->SendPacket(SendBuff);
			return;
		}


		// 6. ��Ʋ���� ���� ��ū ��
		// "����" ��ū�� ���� ��
		if (memcmp(ConnectToken, m_cConnectToken_Now, 32) != 0)
		{
			// �ٸ��ٸ� "����" ��ū�� ��
			if (memcpy(ConnectToken, m_cConnectToken_Before, 32) != 0)
			{
				// �̹� ���� ��Ŷ�� �� �������� üũ
				if (NowPlayer->m_bLogoutFlag == true)
					return;

				// ���� �α� ���
				// �α� ��� (�α� ���� : ����)
				g_BattleServer_RoomLog->LogSave(L"CBattleServer_Room", CSystemLog::en_LogLevel::LEVEL_ERROR,
					L"Auth_LoginPacket_AUTH()--> BattleEnterToken Error!! AccoutnNo : %lld", NowPlayer->m_Int64AccountNo);

				// �ٸ� HTTP ��û�� ���� ��Ŷ�� �� ������ ���ϵ��� �÷��� ����.
				NowPlayer->m_bLogoutFlag = true;

				InterlockedIncrement(&g_lBattleEnterTokenError);

				// �׷��� �ٸ��ٸ� �̻��� ������ �Ǵ�.
				// ��Ʋ���� ���� ��ū�� �ٸ��ٴ� ��Ŷ ������.
				CProtocolBuff_Net* SendBuff = CProtocolBuff_Net::Alloc();

				WORD Type = en_PACKET_CS_GAME_RES_LOGIN;
				BYTE Result = 3;	// ����� ����Ű ������ ���� ���. ���� �����ɱ�?

				SendBuff->PutData((char*)&Type, 2);
				SendBuff->PutData((char*)&DBData->AccountNo, 8);
				SendBuff->PutData((char*)&Result, 1);

				NowPlayer->SendPacket(SendBuff);

				return;
			}
		}


		// 7. ���� �� 
		if (m_uiVer_Code != VerCode)
		{
			// �̹� ���� ��Ŷ�� �� �������� üũ
			if (NowPlayer->m_bLogoutFlag == true)
				return;

			// ���� �α� ���
			// �α� ��� (�α� ���� : ����)
			g_BattleServer_RoomLog->LogSave(L"CBattleServer_Room", CSystemLog::en_LogLevel::LEVEL_ERROR,
				L"Auth_LoginPacket_AUTH()--> VerCode Error!! AccoutnNo : %lld, Code : %d", NowPlayer->m_Int64AccountNo, VerCode);

			// �ٸ� HTTP ��û�� ���� ��Ŷ�� �� ������ ���ϵ��� �÷��� ����.
			NowPlayer->m_bLogoutFlag = true;

			// ������ �ٸ���� Result 5(���� ����)�� ������.
			InterlockedIncrement(&g_lVerError);

			WORD Type = en_PACKET_CS_MATCH_RES_LOGIN;
			BYTE Result = 5;

			CProtocolBuff_Net* SendBuff = CProtocolBuff_Net::Alloc();

			SendBuff->PutData((char*)&Type, 2);
			SendBuff->PutData((char*)&DBData->AccountNo, 8);
			SendBuff->PutData((char*)&Result, 1);

			NowPlayer->SendPacket(SendBuff);
			return;
		}


		// 8. �г��� ����
		const TCHAR* TempNick = Doc[_T("nick")].GetString();
		StringCchCopy(NowPlayer->m_tcNickName, _Mycountof(NowPlayer->m_tcNickName), TempNick);


		// 9. Contents ������ ���õƴٸ� ���� ��Ŷ ����
		NowPlayer->m_lLoginHTTPCount++;

		if (NowPlayer->m_lLoginHTTPCount == 2)
		{
			// �α��� ��Ŷ ó�� Flag ����
			NowPlayer->m_bLoginFlag = true;

			// ���� ���� ��Ŷ ����
			CProtocolBuff_Net* SendBuff = CProtocolBuff_Net::Alloc();

			WORD Type = en_PACKET_CS_MATCH_RES_LOGIN;
			BYTE Result = 1;

			SendBuff->PutData((char*)&Type, 2);
			SendBuff->PutData((char*)&DBData->AccountNo, 8);
			SendBuff->PutData((char*)&Result, 1);

			NowPlayer->SendPacket(SendBuff);
		}
	}

	// Login ��Ŷ�� ���� Contents ���� ��������
	//
	// Parameter : DB_WORK_LOGIN*
	// return : ����
	void CBattleServer_Room::Auth_LoginPacket_Info(DB_WORK_LOGIN_CONTENTS* DBData)
	{
		// 1. ClientKey üũ
		CGameSession* NowPlayer = (CGameSession*)DBData->pPointer;

		// �ٸ��� �̹� ������ ������ �Ǵ�. ���ɼ� �ִ� ��Ȳ
		if (NowPlayer->m_int64ClientKey != DBData->m_i64UniqueKey)
			return;


		// 2. Json������ �Ľ��ϱ� (UTF-16)
		GenericDocument<UTF16<>> Doc;
		Doc.Parse(DBData->m_tcResponse);

		int iResult = Doc[_T("result")].GetInt();


		// 3. DB ��û ��� Ȯ��
		// ����� 1�� �ƴ϶��, 
		if (iResult != 1)
		{
			// �̹� ���� ��Ŷ�� �� �������� üũ
			if (NowPlayer->m_bLogoutFlag == true)
				return;

			// ���� �α� ���
			// �α� ��� (�α� ���� : ����)
			g_BattleServer_RoomLog->LogSave(L"CBattleServer_Room", CSystemLog::en_LogLevel::LEVEL_ERROR,
				L"Auth_LoginPacket_Info()--> DB Result Error!! AccoutnNo : %lld, Error : %d", NowPlayer->m_Int64AccountNo, iResult);

			// �ٸ� HTTP ��û�� ���� ��Ŷ�� �� ������ ���ϵ��� �÷��� ����.
			NowPlayer->m_bLogoutFlag = true;

			WORD Type = en_PACKET_CS_GAME_RES_LOGIN;
			BYTE Result = 2;

			// ����� -10�� ��� (ȸ������ ��ü�� �ȵǾ� ����)
			if (iResult == -10)
				InterlockedIncrement(&g_lQuery_Result_Not_Find);

			// �� �� ��Ÿ ������ ���
			else
				InterlockedIncrement(&g_lTempError);

			CProtocolBuff_Net* SendBuff = CProtocolBuff_Net::Alloc();

			SendBuff->PutData((char*)&Type, 2);
			SendBuff->PutData((char*)&DBData->AccountNo, 8);
			SendBuff->PutData((char*)&Result, 1);

			NowPlayer->SendPacket(SendBuff);
			return;
		}

		// 4. ������� ���� �������� ��Ŷ ã�ƿ� ��.

		// ���� ����
		NowPlayer->m_iRecord_PlayCount = Doc[_T("playcount")].GetInt();	// �÷��� Ƚ��
		NowPlayer->m_iRecord_PlayTime = Doc[_T("playtime")].GetInt();	// �÷��� �ð� �ʴ���
		NowPlayer->m_iRecord_Kill = Doc[_T("kill")].GetInt();			// ���� Ƚ��
		NowPlayer->m_iRecord_Die = Doc[_T("die")].GetInt();				// ���� Ƚ��
		NowPlayer->m_iRecord_Win = Doc[_T("win")].GetInt();				// �����¸� Ƚ��

		// 5. ������ �α��� HTTP ��û�� �Ϸ�Ǿ��ٸ� ���� ��Ŷ ����
		NowPlayer->m_lLoginHTTPCount++;

		if (NowPlayer->m_lLoginHTTPCount == 2)
		{
			// �α��� ��Ŷ ó�� Flag ����
			NowPlayer->m_bLoginFlag = true;

			// ���� ���� ��Ŷ ����
			CProtocolBuff_Net* SendBuff = CProtocolBuff_Net::Alloc();

			WORD Type = en_PACKET_CS_MATCH_RES_LOGIN;
			BYTE Result = 1;

			SendBuff->PutData((char*)&Type, 2);
			SendBuff->PutData((char*)&DBData->AccountNo, 8);
			SendBuff->PutData((char*)&Result, 1);

			NowPlayer->SendPacket(SendBuff);
		}

	}




	// -----------------------
	// AccountNo �ڷᱸ�� ���� �Լ�
	// -----------------------

	// AccountNo �ڷᱸ���� ������ �߰��ϴ� �Լ�
	//
	// Parameter : AccountNo, CGameSession*
	// return : ���� �� true
	//		  : ���� �� false
	bool CBattleServer_Room::InsertAccountNoFunc(INT64 AccountNo, CGameSession* InsertPlayer)
	{

		AcquireSRWLockExclusive(&m_AccountNo_Umap_srwl);		// AccountNo uamp Exclusive ��

		// 1. Insert
		auto ret = m_AccountNo_Umap.insert(make_pair(AccountNo, InsertPlayer));

		ReleaseSRWLockExclusive(&m_AccountNo_Umap_srwl);		// AccountNo uamp Exclusive ���

		// 2. �ߺ�Ű�� �� false ����
		if (ret.second == false)
			return false;

		return true;
	}

	// AccountNo �ڷᱸ������ ���� �˻� ��, �ش� �������� Disconenct �ϴ� �Լ�
	//
	// Parameter : AccountNo
	// return : ����
	void CBattleServer_Room::DisconnectAccountNoFunc(INT64 AccountNo)
	{
		AcquireSRWLockShared(&m_AccountNo_Umap_srwl);		// AccountNo uamp Shared ��

		// 1. �˻�
		auto ret = m_AccountNo_Umap.find(AccountNo);

		// 2. ���� ������ �� �׳� ����.
		// ���� �� �ɰ� ������ ����, �̹� ������ �������� ���� �ֱ� ������
		// �������� ��Ȳ���� �Ǵ�.
		if (ret == m_AccountNo_Umap.end())
		{
			ReleaseSRWLockShared(&m_AccountNo_Umap_srwl);		// AccountNo uamp Shared ���
			return;
		}

		// 3. �ִ� ������� Disconnect
		ret->second->Disconnect();

		ReleaseSRWLockShared(&m_AccountNo_Umap_srwl);		// AccountNo uamp Shared ���
	}


	// AccountNo �ڷᱸ������ ������ �����ϴ� �Լ�
	//
	// Parameter : AccountNo
	// return : ���� �� true
	//		  : ���� �� false
	bool CBattleServer_Room::EraseAccountNoFunc(INT64 AccountNo)
	{
		AcquireSRWLockExclusive(&m_AccountNo_Umap_srwl);		// AccountNo uamp Exclusive ��

		// 1. �˻�
		auto ret = m_AccountNo_Umap.find(AccountNo);

		// 2. ���� ������ �� false ����
		if (ret == m_AccountNo_Umap.end())
		{
			ReleaseSRWLockExclusive(&m_AccountNo_Umap_srwl);		// AccountNo uamp Exclusive ���
			return false;
		}

		// 3. �ִ� ������� Erase
		m_AccountNo_Umap.erase(ret);

		ReleaseSRWLockExclusive(&m_AccountNo_Umap_srwl);		// AccountNo uamp Shared ���

		return true;
	}






	// ---------------------------------
	// Auth����� �� ���� �ڷᱸ�� ����
	// ---------------------------------

	// ���� Room �ڷᱸ���� Insert�ϴ� �Լ�
	//
	// Parameter : RoomNo, stRoom*
	// return : ���� �� true
	//		  : ����(�ߺ� Ű) �� false	
	bool CBattleServer_Room::InsertRoomFunc(int RoomNo, stRoom* InsertRoom)
	{
		AcquireSRWLockExclusive(&m_Room_Umap_srwl);		// ----- Room �ڷᱸ�� Exclusive �� 

		// 1. insert
		auto ret = m_Room_Umap.insert(make_pair(RoomNo, InsertRoom));

		ReleaseSRWLockExclusive(&m_Room_Umap_srwl);		// ----- Room �ڷᱸ�� Exclusive ��� 

		// 2. �ߺ�Ű��� false ����
		if (ret.second == false)
			return false;

		return true;
	}






	// -----------------------
	// �����Լ�
	// -----------------------

	// AuthThread���� 1Loop���� 1ȸ ȣ��.
	// 1�������� ���������� ó���ؾ� �ϴ� ���� �Ѵ�.
	// 
	// parameter : ����
	// return : ����
	void CBattleServer_Room::OnAuth_Update()
	{
		// ------------------- HTTP ��� ��, ��ó��
		// 1. Q ������ Ȯ��
		int iQSize = m_shDB_Communicate.m_pDB_ReadComplete_Queue->GetInNode();

		// �� �����ӿ� �ִ� m_iHTTP_MAX���� ��ó��.
		if (iQSize > m_stConst.m_iHTTP_MAX)
			iQSize = m_stConst.m_iHTTP_MAX;

		// 2. ������ ���� ����
		DB_WORK* DBData;
		while (iQSize > 0)
		{
			// �ϰ� ��ť		
			if (m_shDB_Communicate.m_pDB_ReadComplete_Queue->Dequeue(DBData) == -1)
				g_BattleServer_Room_Dump->Crash();

			try
			{
				// �ϰ� Ÿ�Կ� ���� �ϰ� ó��
				switch (DBData->m_wWorkType)
				{
					// �α��� ��Ŷ�� ���� ����ó��
				case eu_DB_READ_TYPE::eu_LOGIN_AUTH:
					Auth_LoginPacket_AUTH((DB_WORK_LOGIN*)DBData);
					break;

					// �α��� ��Ŷ�� ���� ���� ������ ó��
				case eu_DB_READ_TYPE::eu_LOGIN_INFO:
					Auth_LoginPacket_Info((DB_WORK_LOGIN_CONTENTS*)DBData);
					break;

					// ���� �ϰ� Ÿ���̸� ����.
				default:
					TCHAR str[200];
					StringCchPrintf(str, 200, _T("OnAuth_Update(). Type Error. Type : %d"), DBData->m_wWorkType);

					throw CException(str);
				}

			}
			catch (CException& exc)
			{
				// �α� ��� (�α� ���� : ����)
				g_BattleServer_RoomLog->LogSave(L"CBattleServer_Room", CSystemLog::en_LogLevel::LEVEL_ERROR, L"%s",
					(TCHAR*)exc.GetExceptionText());

				// ����
				g_BattleServer_Room_Dump->Crash();
			}

			// ��忡 ���� ����ȭ ���� ��ȯ
			if (DBData->m_wWorkType == eu_DB_READ_TYPE::eu_LOGIN_AUTH)
			{
				// DB_WORK ���� ����ȭ ���� ��ȯ
				CProtocolBuff_Net::Free(DBData->m_pBuff);
			}

			// DB_WORK ��ȯ
			m_shDB_Communicate.m_pDB_Work_Pool->Free(DBData);

			--iQSize;
		}


		// ------------------- �� ���� ó��

		// �� �����ӿ� �ִ� m_iLoopRoomModeChange���� �� ��� ����
		int ModeChangeCount = m_stConst.m_iLoopRoomModeChange;
		DWORD CmpTime = timeGetTime();

		AcquireSRWLockShared(&m_Room_Umap_srwl);	// ----- Room umap Shared ��

		auto itor_Now = m_Room_Umap.begin();
		auto itor_End = m_Room_Umap.end();

		// �� �ڷᱸ���� ó������ ������ ��ȸ
		while (itor_Now != itor_End)
		{
			// ����, �̹� �����ӿ�, ������ ��ŭ�� �� ��� ������ �߻��ߴٸ� �׸��Ѵ�.
			if (ModeChangeCount == 0)
				break;

			stRoom* NowRoom = itor_Now->second;

			// �ش� ���� �غ���� ���
			if (NowRoom->m_iRoomState == eu_ROOM_STATE::READY_ROOM)
			{
				// ī��Ʈ �ٿ��� �Ϸ�Ǿ����� üũ
				if ( (NowRoom->m_dwCountDown + m_stConst.m_iCountDownMSec) <= CmpTime )
				{
					// 1. ������ ���� �� ����
					if (NowRoom->m_iJoinUserCount != NowRoom->m_JoinUser_Vector.size())
						g_BattleServer_Room_Dump->Crash();

					NowRoom->m_iAliveUserCount = NowRoom->m_iJoinUserCount;


					// 2. �� �� ��� ������ ���� �÷��� ����
					// false�� ���ϵ� �� ����. �ڷᱸ�� ���� ������ 0���� �� �ֱ� ������.
					// ������ ���ϰ� �ȹ޴´�.
					NowRoom->AliveFalg_True();


					// 3. �� ���� ��� ��������, ��Ʋ���� ���� �÷��� ���� ��Ŷ ������
					CProtocolBuff_Net* SendBuff = CProtocolBuff_Net::Alloc();

					WORD Type = en_PACKET_CS_GAME_RES_PLAY_START;

					SendBuff->PutData((char*)&Type, 2);
					SendBuff->PutData((char*)&NowRoom->m_iRoomNo, 4);

					// ���⼭�� false�� ���ϵ� �� ����(�ڷᱸ�� ���� ������ 0���� �� ����)
					// ī��Ʈ�ٿ��� ������ ���� ��� ������ ���� ���ɼ�.
					// �׷��� ���ϰ� �ȹ޴´�.
					NowRoom->SendPacket_BroadCast(SendBuff);


					// 4. �� ���¸� Play�� ����
					NowRoom->m_iRoomState = eu_ROOM_STATE::PLAY_ROOM;


					// 5. ��� ������ AUTH_TO_GAME���� ����
					NowRoom->ModeChange();


					// 6. �̹� �����ӿ��� �� ��� ���� ���� ī��Ʈ ����
					--ModeChangeCount;					
				}
			}

			++itor_Now;
		}

		ReleaseSRWLockShared(&m_Room_Umap_srwl);	// ----- Room umap Shared ���





		// ------------------- �����
		// !! ���� ���� ���� !!
		// - [���� ���� ���� ������� �� �� �� < �ִ� ������ �� �ִ� �� ��] ����
		// - [���� ���� �� < �ִ� ������ �� �ִ� ���� ��] ����
			
		// �� �����ӿ� �ִ� m_iLoopCreateRoomCount���� �� ����
		int LoopCount = m_stConst.m_iLoopCreateRoomCount;

		while (LoopCount > 0)
		{
			// 1. ���� ������� �� ��� üũ
			if (m_lNowTotalRoomCount < m_stConst.m_lMaxTotalRoomCount)
			{
				// 2. ���� ���� �� üũ
				if (m_lNowWaitRoomCount < m_stConst.m_lMaxWaitRoomCount)
				{
					// ������� ���� ����� ���� ����					

					// 1) ���� �� �� �� ����
					InterlockedIncrement(&m_lNowTotalRoomCount);


					// 2) ���� �� ����.
					// Auth������ �����ϴ� �����̱� ������ ���Ͷ� �ʿ� ����.
					++m_lNowWaitRoomCount;


					// 3) �� Alloc
					stRoom* NowRoom = m_Room_Pool->Alloc();

					// ����, �� �ȿ� ���� ���� 0���� �ƴϸ� Crash
					if (NowRoom->m_iJoinUserCount != 0 || NowRoom->m_JoinUser_Vector.size() != 0)
						g_BattleServer_Room_Dump->Crash();


					// 4) ����
					LONG RoomNo = InterlockedIncrement(&m_lGlobal_RoomNo);
					NowRoom->m_iRoomNo = RoomNo;
					NowRoom->m_iRoomState = eu_ROOM_STATE::WAIT_ROOM;
					NowRoom->m_iJoinUserCount = 0;
					NowRoom->m_iAliveUserCount = 0;
					NowRoom->m_dwCountDown = 0;		
					NowRoom->m_bGameEndFlag = false;
					NowRoom->m_dwGameEndMSec = 0;
					NowRoom->m_bShutdownFlag = false;

					// ��ū ����				
					WORD Index = rand() % 64;	// 0 ~ 63 �� �ε��� ��󳻱�				
					memcpy_s(NowRoom->m_cEnterToken, 32, m_cRoomEnterToken[Index], 32);


					// 5) �� ���� �ڷᱸ���� �߰�
					if (InsertRoomFunc(RoomNo, NowRoom) == false)
						g_BattleServer_Room_Dump->Crash();


					// 6) ������ �������� [�ű� ���� ���� �˸�] ��Ŷ ����
					// �� Ŭ�� ���� ������.
					m_Master_LanClient->Packet_NewRoomCreate_Req(m_iServerNo, NowRoom);

					--LoopCount;
				}

				// ���� ���� ���� �̹� max��� �� �ȸ���� ������.
				else
					break;
			}

			// ���� ������� �� ����� �̹� max��� �� �ȸ���� ������.
			else
				break;			
		}

		


		// ------------------- ��ū ��߱�
		// ������ �� Ŭ���̾�Ʈ�� �Լ� ȣ��
		m_Master_LanClient->Packet_TokenChange_Req();	
	}

	// GameThread���� 1Loop���� 1ȸ ȣ��.
	// 1�������� ���������� ó���ؾ� �ϴ� ���� �Ѵ�.
	// 
	// parameter : ����
	// return : ����
	void CBattleServer_Room::OnGame_Update()
	{
		// ------------------- �� üũ

		// �̹� �����ӿ� ������ �� �޾Ƶα�.
		// �� �����ӿ� �ִ� 100���� �� ���� ����
		int DeleteRoomNo[100];
		int Index = 0;

		AcquireSRWLockShared(&m_Room_Umap_srwl);	// ----- Room Umap Shared ��
		
		auto itor_Now = m_Room_Umap.begin();
		auto itor_End = m_Room_Umap.end();

		// Step 1. ������ �� üũ
		// Step 2. ���� ������ �ִٸ�, ���� ����� �� ó�� 	
		// Setp 3. ���� ����� ���� �ƴϰ� ������ �浵 �ƴ϶��, ���� ���� üũ

		// ��� ���� ��ȸ�ϸ�, PLAY ����� �濡 ���� �۾� ����
		while (itor_Now != itor_End)
		{
			// �� ��� üũ. 
			if (itor_Now->second->m_iRoomState == eu_ROOM_STATE::PLAY_ROOM)
			{
				// ������� ���� ���� �����ϴ� ��.
				stRoom* NowRoom = itor_Now->second;

				// Step 1. ������ �� üũ
				// ������ �ο� ���� 0���� ��. (���� �� �ƴ�)
				// ���� ���� ��, ��� ������ �i�Ƴ��� �۾����� �Ϸ�� ��.
				if (NowRoom->m_iJoinUserCount == 0)
				{
					if (Index < 100)
					{
						DeleteRoomNo[Index] = NowRoom->m_iRoomNo;
						++Index;
					}
				}

				// Step 2. ���� ������ �ִٸ�, ���� ����� �� ó�� 
				// �˴ٿ��� ���� ����, ���� �����ӿ� Step 1. ���� �ɸ��� ����Ѵ�.
				else if (NowRoom->m_bGameEndFlag == true)
				{
					// �˴ٿ��� ���� �ʾ��� ��쿡�� ���� ����.
					// �̹� ���� �����ӿ� �˴ٿ��� ���ȴµ�, ���� ��� ������ ������� �ʾ�
					// �� �� ������ Ż ���ɼ��� �ֱ� ������.
					if (NowRoom->m_bShutdownFlag == false)
					{
						// ���� �ð��� �Ǿ����� üũ
						if ((NowRoom->m_dwGameEndMSec + m_stConst.m_iRoomCloseDelay) <= timeGetTime())
						{		
							// �˴ٿ� ���ȴٴ� �÷��� ����.
							NowRoom->m_bShutdownFlag = true;

							// ���� �濡 �ִ� �������� Shutdown
							NowRoom->Shutdown_All();
						}
					}
				}				

				// Setp 3. ���� ����� ���� �ƴϰ� ������ �浵 �ƴ϶��, ���� ���� üũ
				// �����ڰ� 1���̸� ���� ����.
				else if (NowRoom->m_iAliveUserCount == 1)
				{
					// ���� ���� ��Ŷ ������
					// - ������ 1���� �¸� ��Ŷ
					// - ������ �����ڵ鿡�� �й� ��Ŷ
					NowRoom->GameOver();

					// ���� �� ���� �÷��� ����
					NowRoom->m_bGameEndFlag = true;

					// �� ������ �ð� ����
					NowRoom->m_dwGameEndMSec = timeGetTime();

					// �� �������� ���� ���� �ð��� 
				}
			}

			++itor_End;
		}

		ReleaseSRWLockShared(&m_Room_Umap_srwl);	// ----- Room Umap Shared ���




		// ------------------- �� ����

		// Step 4. �� ����

		// Index�� 0���� ũ�ٸ�, ������ ���� �ִ� ��. 		
		if (Index > 0)
		{
			int Index_B = 0;

			AcquireSRWLockExclusive(&m_Room_Umap_srwl);	// ----- Room Umap Exclusive ��		

			while (Index_B < Index)
			{
				// 1. �˻�
				auto FindRoom = m_Room_Umap.find(DeleteRoomNo[Index_B]);

				// ���⼭ ������ �ȵ�. �� ������ �� �������� �ۿ� ���ϱ� ������
				if (FindRoom == m_Room_Umap.end())
					g_BattleServer_Room_Dump->Crash();

				// 2. stRoom* ����
				m_Room_Pool->Free(FindRoom->second);

				// 3. Erase
				m_Room_Umap.erase(FindRoom);

				++Index_B;
			}

			ReleaseSRWLockExclusive(&m_Room_Umap_srwl);	// ----- Room Umap Exclusive ���
		}
	}

	// ���ο� ���� ���� ��, Auth���� ȣ��ȴ�.
	//
	// parameter : ������ ������ IP, Port
	// return false : Ŭ���̾�Ʈ ���� �ź�
	// return true : ���� ���
	bool CBattleServer_Room::OnConnectionRequest(TCHAR* IP, USHORT port)
	{
		return true;
	}

	// ��Ŀ �����尡 ��� �� ȣ��Ǵ� �Լ�.
	// GQCS �ٷ� �ϴܿ��� ȣ��
	// 
	// parameter : ����
	// return : ����
	void CBattleServer_Room::OnWorkerThreadBegin()
	{

	}

	// ��Ŀ �����尡 ���� �� ȣ��Ǵ� �Լ�
	// GQCS �ٷ� ������ ȣ��
	// 
	// parameter : ����
	// return : ����
	void CBattleServer_Room::OnWorkerThreadEnd()
	{

	}

	// ���� �߻� �� ȣ��Ǵ� �Լ�.
	//
	// parameter : ���� �ڵ�(���� ������ �����ڵ�� WinGetLastError() �Լ��� ��� ����. ���� ��� 0�� ���ϵ�)
	//			 : ���� �ڵ忡 ���� ��Ʈ��
	// return : ����
	void CBattleServer_Room::OnError(int error, const TCHAR* errorStr)
	{
		// �α� ��� (�α� ���� : ����)
		g_BattleServer_RoomLog->LogSave(L"CBattleServer_Room", CSystemLog::en_LogLevel::LEVEL_ERROR, L"%s (ErrorCode : %d)",
			errorStr, error);
	}

	   	  

	// -----------------
	// �����ڿ� �Ҹ���
	// -----------------
	CBattleServer_Room::CBattleServer_Room()
		:CMMOServer()
	{	
		srand((UINT)time(NULL));

		m_lNowWaitRoomCount = 0;
		m_lNowTotalRoomCount = 0;
		m_lGlobal_RoomNo = 0;
		m_iServerNo = -1;

		// �� ���� ��ū �����α�
		for (int i = 0; i < 64; ++i)
		{
			for (int h = 0; h < 32; ++h)
			{
				m_cRoomEnterToken[i][h] = (rand() % 128) + 1;
			}
		}	

		// ��Ʋ���� ���� ���� ��ū ����� �α�
		for (int i = 0; i < 32; ++i)
		{
			m_cConnectToken_Now[i] = (rand() % 128) + 1;
		}

		// ����͸� ������ ����ϱ� ���� LanClient �����Ҵ�
		m_Monitor_LanClient = new CGame_MinitorClient;
		m_Monitor_LanClient->ParentSet(this);

		// ������ ������ ����ϱ� ���� LanClient �����Ҵ�
		m_Master_LanClient = new CBattle_Master_LanClient;
		m_Master_LanClient->ParentSet(this);


		// ------------------- Config���� ����		
		if (SetFile(&m_stConfig) == false)
			g_BattleServer_Room_Dump->Crash();

		// ------------------- �α� ������ ���� ����
		g_BattleServer_RoomLog->SetDirectory(L"CBattleServer_Room");
		g_BattleServer_RoomLog->SetLogLeve((CSystemLog::en_LogLevel)m_stConfig.LogLevel);
				


		// TLS �����Ҵ�
		m_Room_Pool = new CMemoryPoolTLS<stRoom>(0, false);		

		// reserve ����.
		m_AccountNo_Umap.reserve(m_stConfig.MaxJoinUser);
		m_Room_Umap.reserve(m_stConst.m_lMaxTotalRoomCount);

		//SRW�� �ʱ�ȭ
		InitializeSRWLock(&m_AccountNo_Umap_srwl);
		InitializeSRWLock(&m_Room_Umap_srwl);
	}

	CBattleServer_Room::~CBattleServer_Room()
	{
		// ����͸� ������ ����ϴ� LanClient ��������
		delete m_Monitor_LanClient;

		// ������ ������ ����ϴ� LanClient ���� ����
		delete m_Master_LanClient;

		// TLS ���� ����
		delete m_Room_Pool;
	}

}


// ----------------------------------------
// 
// ������ ������ ����Ǵ� LanClient
//
// ----------------------------------------
namespace Library_Jingyu
{

	// -----------------------
	// ��Ŷ ó�� �Լ�
	// -----------------------

	// �����Ϳ��� ���� �α��� ��û ����
	//
	// Parameter : SessionID, CProtocolBuff_Lan*
	// return : ����
	void CBattle_Master_LanClient::Packet_Login_Res(ULONGLONG SessionID, CProtocolBuff_Lan* Payload)
	{
		// 1. Connect �������� Ȯ��
		if (SessionID != m_ullSessionID)
			g_BattleServer_Room_Dump->Crash();

		// 2. ������
		int ServerNo;
		Payload->GetData((char*)&ServerNo, 4);

		// 3. ���� ��ȣ ����
		// ����, ��Ʋ Net ������ ��ȣ�� -1(�ʱ� ��)�� �ƴ϶�� 2�� �α��� ��û?�� ���� ��.
		if(m_BattleServer_this->m_iServerNo != -1)
			g_BattleServer_Room_Dump->Crash();

		m_BattleServer_this->m_iServerNo = ServerNo;
	}

	// �ű� ���� ���� ����
	//
	// Parameter : SessionID, CProtocolBuff_Lan*
	// return : ����
	void CBattle_Master_LanClient::Packet_NewRoomCreate_Res(ULONGLONG SessionID, CProtocolBuff_Lan* Payload)
	{
		// ���� �ϴ°� ����..
	}

	// ��ū ����� ����
	//
	// Parameter : SessionID, CProtocolBuff_Lan*
	// return : ����
	void CBattle_Master_LanClient::Packet_TokenChange_Res(ULONGLONG SessionID, CProtocolBuff_Lan* Payload)
	{
		// ���� �ϴ°� ����..
	}

	// �� ���� ����
	//
	// Parameter : SessionID, CProtocolBuff_Lan*
	// return : ����
	void CBattle_Master_LanClient::Packet_RoomClose_Res(ULONGLONG SessionID, CProtocolBuff_Lan* Payload)
	{
		// ���� �ϴ°� ����..
	}

	// �� ���� ����
	//
	// Parameter : RoomNo, AccountNo
	// return : ����
	void CBattle_Master_LanClient::Packet_RoomLeave_Res(ULONGLONG SessionID, CProtocolBuff_Lan* Payload)
	{
		// ���� �ϴ°� ����..
	}




	// -----------------------
	// Battle Net ������ ȣ���ϴ� �Լ�
	// -----------------------

	// �����Ϳ���, �ű� ���� ���� ��Ŷ ������
	//
	// Parameter : ��Ʋ���� No, stRoom*
	// return : ����
	void CBattle_Master_LanClient::Packet_NewRoomCreate_Req(int BattleServerNo, CBattleServer_Room::stRoom* NewRoom)
	{
		// 1. ��Ŷ �����
		WORD Type = en_PACKET_BAT_MAS_REQ_CREATED_ROOM;
		UINT ReqSequence = InterlockedIncrement(&uiReqSequence);

		CProtocolBuff_Lan* SendBuff = CProtocolBuff_Lan::Alloc();

		SendBuff->PutData((char*)&Type, 2);
		SendBuff->PutData((char*)&BattleServerNo, 4);
		SendBuff->PutData((char*)&NewRoom->m_iRoomNo, 4);
		SendBuff->PutData((char*)&NewRoom->m_iMaxJoinCount, 4);
		SendBuff->PutData(NewRoom->m_cEnterToken, 32);

		SendBuff->PutData((char*)&ReqSequence, 4);

		// 2. ��ū �߱� �ð� �����ϱ�
		m_dwTokenSendTime = timeGetTime();

		// 3. �����Ϳ��� Send�ϱ�
		SendPacket(m_ullSessionID, SendBuff);
	}
	
	// ��ū ��߱� �Լ�
	//
	// Parameter : ����
	// return : ����
	void CBattle_Master_LanClient::Packet_TokenChange_Req()
	{
		// �����Ϳ� ������ ���� ���¶�� ��Ŷ �Ⱥ�����.
		if (m_ullSessionID == 0xffffffffffffffff)
			return;


		// ��ū ��߱� ������ �Ǿ����� Ȯ���ϱ�.
		DWORD NowTime = timeGetTime();

		if ((m_dwTokenSendTime + m_BattleServer_this->m_stConst.m_iTokenChangeSlice) <= NowTime)
		{
			// 1. ��ū �߱� �ð� ����
			m_dwTokenSendTime = NowTime;

			// 2. "����" ��ū�� "����" ��ū�� ����
			memcpy_s(m_BattleServer_this->m_cConnectToken_Now, 32, m_BattleServer_this->m_cConnectToken_Before, 32);

			// 3. "����" ��ū �ٽ� �����
			int i = 0;
			while (i < 32)
			{
				m_BattleServer_this->m_cConnectToken_Now[i] = (rand() % 128) + 1;

				++i;
			}

			// 4. ��ū ��߱� ��Ŷ �����
			WORD Type = en_PACKET_BAT_MAS_REQ_CONNECT_TOKEN;
			UINT ReqSequence = InterlockedIncrement(&uiReqSequence);

			CProtocolBuff_Lan* SendBuff = CProtocolBuff_Lan::Alloc();

			SendBuff->PutData((char*)&Type, 2);
			SendBuff->PutData(m_BattleServer_this->m_cConnectToken_Now, 32);
			SendBuff->PutData((char*)&ReqSequence, 4);


			// 5. �����Ϳ��� ��Ŷ ������
			SendPacket(m_ullSessionID, SendBuff);		
		}
	}

	// �����Ϳ���, �� ���� ��Ŷ ������
	//
	// Parameter : RoomNo
	// return : ����
	void CBattle_Master_LanClient::Packet_RoomClose_Req(int RoomNo)
	{
		// �����Ϳ� ������ ���� ���¶�� ��Ŷ �Ⱥ�����.
		if (m_ullSessionID == 0xffffffffffffffff)
			return;

		// 1. ��Ŷ �����
		WORD Type = en_PACKET_BAT_MAS_REQ_CLOSED_ROOM;
		UINT ReqSequence = InterlockedIncrement(&uiReqSequence);

		CProtocolBuff_Lan* SendBuff = CProtocolBuff_Lan::Alloc();

		SendBuff->PutData((char*)&Type, 2);
		SendBuff->PutData((char*)&RoomNo, 4);
		SendBuff->PutData((char*)&ReqSequence, 4);

		SendPacket(m_ullSessionID, SendBuff);
	}

	// �����Ϳ���, �� ���� ��Ŷ ������
	//
	// Parameter : RoomNo, AccountNo
	// return : ����
	void CBattle_Master_LanClient::Packet_RoomLeave_Req(int RoomNo, INT64 AccountNo)
	{
		// �����Ϳ� ������ ���� ���¶�� ��Ŷ �Ⱥ�����.
		if (m_ullSessionID == 0xffffffffffffffff)
			return;

		// 1. ��Ŷ �����
		WORD Type = en_PACKET_BAT_MAS_REQ_LEFT_USER;
		UINT ReqSequence = InterlockedIncrement(&uiReqSequence);

		CProtocolBuff_Lan* SendBuff = CProtocolBuff_Lan::Alloc();

		SendBuff->PutData((char*)&Type, 2);
		SendBuff->PutData((char*)&RoomNo, 4);
		SendBuff->PutData((char*)&AccountNo, 8);
		SendBuff->PutData((char*)&ReqSequence, 4);

		SendPacket(m_ullSessionID, SendBuff);
	}



	// -----------------------
	// �ܺο��� ��� ������ �Լ�
	// -----------------------

	// ���� �Լ�
	// ����������, ��ӹ��� CLanClient�� Startȣ��.
	//
	// Parameter : ������ ������ IP, ��Ʈ, ��Ŀ������ ��, Ȱ��ȭ��ų ��Ŀ������ ��, TCP_NODELAY ��� ����(true�� ���)
	// return : ���� �� true , ���� �� falsel 
	bool CBattle_Master_LanClient::ClientStart(TCHAR* ConnectIP, int Port, int CreateWorker, int ActiveWorker, int Nodelay)
	{
		// ������ ������ ����
		if (Start(ConnectIP, Port, CreateWorker, ActiveWorker, Nodelay) == false)
			return false;

		return true;
	}

	// ���� �Լ�
	// ����������, ��ӹ��� CLanClient�� Stopȣ��.
	// �߰���, ���ҽ� ���� ��
	//
	// Parameter : ����
	// return : ����
	void CBattle_Master_LanClient::ClientStop()
	{
		// ������ ������ ���� ����
		Stop();
	}

	// ��Ʋ������ this�� �Է¹޴� �Լ�
	// 
	// Parameter : ��Ʋ ������ this
	// return : ����
	void CBattle_Master_LanClient::ParentSet(CBattleServer_Room* ChatThis)
	{
		m_BattleServer_this = ChatThis;
	}




	// -----------------------
	// ���� �����Լ�
	// -----------------------

	// ��ǥ ������ ���� ���� ��, ȣ��Ǵ� �Լ� (ConnectFunc���� ���� ���� �� ȣ��)
	//
	// parameter : ����Ű
	// return : ����
	void CBattle_Master_LanClient::OnConnect(ULONGLONG SessionID)
	{
		m_ullSessionID = SessionID;

		// ������ ����(Lan)�� �α��� ��Ŷ ����
		CProtocolBuff_Lan* SendBuff = CProtocolBuff_Lan::Alloc();

		WORD Type = en_PACKET_BAT_MAS_REQ_SERVER_ON;

		SendBuff->PutData((char*)&Type, 2);

		SendBuff->PutData((char*)m_tcBattleNetServerIP, 32);
		SendBuff->PutData((char*)&m_iBattleNetServerPort, 2);
		SendBuff->PutData(m_BattleServer_this->m_cConnectToken_Now, 32);
		SendBuff->PutData(m_cMasterToken, 32);

		SendBuff->PutData((char*)m_tcChatNetServerIP, 32);
		SendBuff->PutData((char*)&m_iChatNetServerPort, 2);


		SendPacket(SessionID, SendBuff);
	}

	// ��ǥ ������ ���� ���� �� ȣ��Ǵ� �Լ� (InDIsconnect �ȿ��� ȣ��)
	//
	// parameter : ����Ű
	// return : ����
	void CBattle_Master_LanClient::OnDisconnect(ULONGLONG SessionID)
	{
		// SessionID�� �ʱⰪ���� ����
		m_ullSessionID = 0xffffffffffffffff;

		// ��Ʋ Net ������ ServerNo�� �ʱⰪ���� ����
		m_BattleServer_this->m_iServerNo = -1;
	}

	// ��Ŷ ���� �Ϸ� �� ȣ��Ǵ� �Լ�.
	//
	// parameter : ���� ����Ű, CProtocolBuff_Lan*
	// return : ����
	void CBattle_Master_LanClient::OnRecv(ULONGLONG SessionID, CProtocolBuff_Lan* Payload)
	{
		// Type �и�
		WORD Type;
		Payload->GetData((char*)&Type, 2);


		// Ÿ�Կ� ���� �б� ó��
		try
		{
			switch (Type)
			{
				// �α��� ��û�� ���� ����
			case en_PACKET_BAT_MAS_RES_SERVER_ON:
				Packet_Login_Res(SessionID, Payload);

				// �ű� ���� ���� ����
			case en_PACKET_BAT_MAS_RES_CREATED_ROOM:
				Packet_NewRoomCreate_Res(SessionID, Payload);

				// ��ū ����� ����
			case en_PACKET_BAT_MAS_RES_CONNECT_TOKEN:
				Packet_TokenChange_Res(SessionID, Payload);

				// �� ���� ����
			case en_PACKET_BAT_MAS_RES_CLOSED_ROOM:
				Packet_RoomClose_Res(SessionID, Payload);

				// �� ���� ����
			case en_PACKET_BAT_MAS_RES_LEFT_USER:
				Packet_RoomLeave_Res(SessionID, Payload);

				break;
			default:
				break;
			}

		}
		catch (CException& exc)
		{

		}
	}

	// ��Ŷ �۽� �Ϸ� �� ȣ��Ǵ� �Լ�
	//
	// parameter : ���� ����Ű, Send �� ������
	// return : ����
	void CBattle_Master_LanClient::OnSend(ULONGLONG SessionID, DWORD SendSize)
	{}

	// ��Ŀ �����尡 ��� �� ȣ��Ǵ� �Լ�.
	// GQCS �ٷ� �ϴܿ��� ȣ��
	// 
	// parameter : ����
	// return : ����
	void CBattle_Master_LanClient::OnWorkerThreadBegin()
	{}

	// ��Ŀ �����尡 ���� �� ȣ��Ǵ� �Լ�
	// GQCS �ٷ� ������ ȣ��
	// 
	// parameter : ����
	// return : ����
	void CBattle_Master_LanClient::OnWorkerThreadEnd()
	{}

	// ���� �߻� �� ȣ��Ǵ� �Լ�.
	//
	// parameter : ���� �ڵ�(���� ������ �����ڵ�� WinGetLastError() �Լ��� ��� ����. ���� ��� 0�� ���ϵ�)
	//			 : ���� �ڵ忡 ���� ��Ʈ��
	// return : ����
	void CBattle_Master_LanClient::OnError(int error, const TCHAR* errorStr)
	{}






	// -----------------------
	// �����ڿ� �Ҹ���
	// -----------------------

	// ������
	CBattle_Master_LanClient::CBattle_Master_LanClient()
	{
		// SessionID �ʱ�ȭ
		m_ullSessionID = 0xffffffffffffffff;

		// Req������ �ʱ�ȭ
		uiReqSequence = 0;
	}

	// �Ҹ���
	CBattle_Master_LanClient::~CBattle_Master_LanClient()
	{
		// ���� ������ �Ǿ�������, ���� ����
		if (GetClinetState() == true)
			ClientStop();
	}
}


// ---------------
// CGame_MinitorClient
// CLanClienet�� ��ӹ޴� ����͸� Ŭ��
// ---------------
namespace Library_Jingyu
{
	// -----------------------
	// �����ڿ� �Ҹ���
	// -----------------------
	CGame_MinitorClient::CGame_MinitorClient()
		:CLanClient()
	{
		// ����͸� ���� �������� �����带 �����ų �̺�Ʈ ����
		//
		// �ڵ� ���� Event 
		// ���� ���� �� non-signalled ����
		// �̸� ���� Event	
		m_hMonitorThreadExitEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	}

	CGame_MinitorClient::~CGame_MinitorClient()
	{
		// ���� ������ �Ǿ�������, ���� ����
		if (GetClinetState() == true)
			ClientStop();

		// �̺�Ʈ ����
		CloseHandle(m_hMonitorThreadExitEvent);
	}



	// -----------------------
	// �ܺο��� ��� ������ �Լ�
	// -----------------------

	// ���� �Լ�
	// ����������, ��ӹ��� CLanClient�� Startȣ��.
	//
	// Parameter : ������ ������ IP, ��Ʈ, ��Ŀ������ ��, Ȱ��ȭ��ų ��Ŀ������ ��, TCP_NODELAY ��� ����(true�� ���)
	// return : ���� �� true , ���� �� falsel 
	bool CGame_MinitorClient::ClientStart(TCHAR* ConnectIP, int Port, int CreateWorker, int ActiveWorker, int Nodelay)
	{
		// ����͸� ������ ����
		if (Start(ConnectIP, Port, CreateWorker, ActiveWorker, Nodelay) == false)
			return false;

		// ����͸� ������ ���� ������ ������ ����
		m_hMonitorThread = (HANDLE)_beginthreadex(NULL, 0, MonitorThread, this, 0, NULL);

		return true;
	}

	// ���� �Լ�
	// ����������, ��ӹ��� CLanClient�� Stopȣ��.
	// �߰���, ���ҽ� ���� ��
	//
	// Parameter : ����
	// return : ����
	void CGame_MinitorClient::ClientStop()
	{
		// 1. ����͸� ���� �������� ������ ����
		SetEvent(m_hMonitorThreadExitEvent);

		// ���� ���
		if (WaitForSingleObject(m_hMonitorThread, INFINITE) == WAIT_FAILED)
		{
			DWORD Error = GetLastError();
			printf("MonitorThread Exit Error!!! (%d) \n", Error);
		}

		// 2. ������ �ڵ� ��ȯ
		CloseHandle(m_hMonitorThread);

		// 3. ����͸� ������ ���� ����
		Stop();
	}

	// ��Ʋ������ this�� �Է¹޴� �Լ�
	// 
	// Parameter : ��Ʋ ������ this
	// return : ����
	void CGame_MinitorClient::ParentSet(CBattleServer_Room* ChatThis)
	{
		m_BattleServer_this = ChatThis;
	}




	// -----------------------
	// ���ο����� ����ϴ� ��� �Լ�
	// -----------------------

	// ���� �ð����� ����͸� ������ ������ �����ϴ� ������
	UINT	WINAPI CGame_MinitorClient::MonitorThread(LPVOID lParam)
	{
		// this �޾Ƶα�
		CGame_MinitorClient* g_This = (CGame_MinitorClient*)lParam;

		// ���� ��ȣ �̺�Ʈ �޾Ƶα�
		HANDLE* hEvent = &g_This->m_hMonitorThreadExitEvent;

		// CPU ����� üũ Ŭ���� (ä�ü��� ����Ʈ����)
		CCpuUsage_Process CProcessCPU;

		// CPU ����� üũ Ŭ���� (�ϵ����)
		CCpuUsage_Processor CProcessorCPU;

		// PDH�� Ŭ����
		CPDH	CPdh;

		while (1)
		{
			// 1�ʿ� �ѹ� ����� ������ ������.
			DWORD Check = WaitForSingleObject(*hEvent, 1000);

			// �̻��� ��ȣ���
			if (Check == WAIT_FAILED)
			{
				DWORD Error = GetLastError();
				printf("MoniterThread Exit Error!!! (%d) \n", Error);
				break;
			}

			// ����, ���� ��ȣ�� �Դٸ� ������ ����.
			else if (Check == WAIT_OBJECT_0)
				break;


			// �װ� �ƴ϶��, ���� �Ѵ�.

			// ���μ���, ���μ��� CPU �����, PDH ���� ����
			CProcessorCPU.UpdateCpuTime();
			CProcessCPU.UpdateCpuTime();
			CPdh.SetInfo();

			// ----------------------------------
			// �ϵ���� ���� ������ (���μ���)
			// ----------------------------------
			int TimeStamp = (int)(time(NULL));

			// 1. �ϵ���� CPU ���� ��ü
			g_This->InfoSend(dfMONITOR_DATA_TYPE_SERVER_CPU_TOTAL, (int)CProcessorCPU.ProcessorTotal(), TimeStamp);

			// 2. �ϵ���� ��밡�� �޸� (MByte)
			g_This->InfoSend(dfMONITOR_DATA_TYPE_SERVER_AVAILABLE_MEMORY, (int)CPdh.Get_AVA_Mem(), TimeStamp);

			// 3. �ϵ���� �̴��� ���� ����Ʈ (KByte)
			int iData = (int)(CPdh.Get_Net_Recv() / 1024);
			g_This->InfoSend(dfMONITOR_DATA_TYPE_SERVER_NETWORK_RECV, iData, TimeStamp);

			// 4. �ϵ���� �̴��� �۽� ����Ʈ (KByte)
			iData = (int)(CPdh.Get_Net_Send() / 1024);
			g_This->InfoSend(dfMONITOR_DATA_TYPE_SERVER_NETWORK_SEND, iData, TimeStamp);

			// 5. �ϵ���� �������� �޸� ��뷮 (MByte)
			iData = (int)(CPdh.Get_NonPaged_Mem() / 1024 / 1024);
			g_This->InfoSend(dfMONITOR_DATA_TYPE_SERVER_NONPAGED_MEMORY, iData, TimeStamp);



			// ----------------------------------
			// ��Ʋ���� ���� ������
			// ----------------------------------

			// ��Ʋ������ On�� ���, ���Ӽ��� ���� ������.
			if (g_This->m_BattleServer_this->GetServerState() == true)
			{
				// 1. ��Ʋ���� ON		
				g_This->InfoSend(dfMONITOR_DATA_TYPE_BATTLE_SERVER_ON, TRUE, TimeStamp);

				// 2. ��Ʋ���� CPU ���� (Ŀ�� + ����)
				g_This->InfoSend(dfMONITOR_DATA_TYPE_BATTLE_CPU, (int)CProcessCPU.ProcessTotal(), TimeStamp);

				// 3. ��Ʋ���� �޸� ���� Ŀ�� ��뷮 (Private) MByte
				int Data = (int)(CPdh.Get_UserCommit() / 1024 / 1024);
				g_This->InfoSend(dfMONITOR_DATA_TYPE_BATTLE_MEMORY_COMMIT, Data, TimeStamp);

				// 4. ��Ʋ���� ��ŶǮ ��뷮
				g_This->InfoSend(dfMONITOR_DATA_TYPE_BATTLE_PACKET_POOL, g_lAllocNodeCount + g_lAllocNodeCount_Lan, TimeStamp);

				// 5. Auth ������ �ʴ� ���� ��
				g_This->InfoSend(dfMONITOR_DATA_TYPE_BATTLE_AUTH_FPS, g_lShowAuthFPS, TimeStamp);

				// 6. Game ������ �ʴ� ���� ��
				g_This->InfoSend(dfMONITOR_DATA_TYPE_BATTLE_GAME_FPS, g_lShowGameFPS, TimeStamp);

				// 7. ��Ʋ���� ���� ������ü
				g_This->InfoSend(dfMONITOR_DATA_TYPE_BATTLE_SESSION_ALL, (int)g_This->m_BattleServer_this->GetClientCount(), TimeStamp);

				// 8. Auth ������ ��� �ο�
				g_This->InfoSend(dfMONITOR_DATA_TYPE_BATTLE_SESSION_AUTH, g_lAuthModeUserCount, TimeStamp);

				// 9. Game ������ ��� �ο�
				g_This->InfoSend(dfMONITOR_DATA_TYPE_BATTLE_SESSION_GAME, g_lGameModeUserCount, TimeStamp);

				// 10. ���� ��

				// 11. �÷��� �� ��
			}

		}

		return 0;
	}

	// ����͸� ������ ������ ����
	//
	// Parameter : DataType(BYTE), DataValue(int), TimeStamp(int)
	// return : ����
	void CGame_MinitorClient::InfoSend(BYTE DataType, int DataValue, int TimeStamp)
	{
		WORD Type = en_PACKET_SS_MONITOR_DATA_UPDATE;

		CProtocolBuff_Lan* SendBuff = CProtocolBuff_Lan::Alloc();

		SendBuff->PutData((char*)&Type, 2);
		SendBuff->PutData((char*)&DataType, 1);
		SendBuff->PutData((char*)&DataValue, 4);
		SendBuff->PutData((char*)&TimeStamp, 4);

		SendPacket(m_ullSessionID, SendBuff);
	}





	// -----------------------
	// ���� �����Լ�
	// -----------------------

	// ��ǥ ������ ���� ���� ��, ȣ��Ǵ� �Լ� (ConnectFunc���� ���� ���� �� ȣ��)
	//
	// parameter : ����Ű
	// return : ����
	void CGame_MinitorClient::OnConnect(ULONGLONG SessionID)
	{
		m_ullSessionID = SessionID;

		// ����͸� ����(Lan)�� �α��� ��Ŷ ����
		CProtocolBuff_Lan* SendBuff = CProtocolBuff_Lan::Alloc();

		WORD Type = en_PACKET_SS_MONITOR_LOGIN;
		int ServerNo = dfSERVER_NO;

		SendBuff->PutData((char*)&Type, 2);
		SendBuff->PutData((char*)&ServerNo, 4);

		SendPacket(SessionID, SendBuff);
	}

	// ��ǥ ������ ���� ���� �� ȣ��Ǵ� �Լ� (InDIsconnect �ȿ��� ȣ��)
	//
	// parameter : ����Ű
	// return : ����
	void CGame_MinitorClient::OnDisconnect(ULONGLONG SessionID)
	{}

	// ��Ŷ ���� �Ϸ� �� ȣ��Ǵ� �Լ�.
	//
	// parameter : ���� ����Ű, CProtocolBuff_Lan*
	// return : ����
	void CGame_MinitorClient::OnRecv(ULONGLONG SessionID, CProtocolBuff_Lan* Payload)
	{}

	// ��Ŷ �۽� �Ϸ� �� ȣ��Ǵ� �Լ�
	//
	// parameter : ���� ����Ű, Send �� ������
	// return : ����
	void CGame_MinitorClient::OnSend(ULONGLONG SessionID, DWORD SendSize)
	{}

	// ��Ŀ �����尡 ��� �� ȣ��Ǵ� �Լ�.
	// GQCS �ٷ� �ϴܿ��� ȣ��
	// 
	// parameter : ����
	// return : ����
	void CGame_MinitorClient::OnWorkerThreadBegin()
	{}

	// ��Ŀ �����尡 ���� �� ȣ��Ǵ� �Լ�
	// GQCS �ٷ� ������ ȣ��
	// 
	// parameter : ����
	// return : ����
	void CGame_MinitorClient::OnWorkerThreadEnd()
	{}

	// ���� �߻� �� ȣ��Ǵ� �Լ�.
	//
	// parameter : ���� �ڵ�(���� ������ �����ڵ�� WinGetLastError() �Լ��� ��� ����. ���� ��� 0�� ���ϵ�)
	//			 : ���� �ڵ忡 ���� ��Ʈ��
	// return : ����
	void CGame_MinitorClient::OnError(int error, const TCHAR* errorStr)
	{}
}