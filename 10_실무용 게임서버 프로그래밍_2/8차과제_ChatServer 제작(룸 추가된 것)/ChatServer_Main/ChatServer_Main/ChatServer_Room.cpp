#include "pch.h"
#include "ChatServer_Room.h"
#include "Protocol_Set/CommonProtocol_2.h"
#include "CrashDump/CrashDump.h"
#include "Log/Log.h"
#include "Parser/Parser_Class.h"
#include "CPUUsage/CPUUsage.h"
#include "PDHClass/PDHCheck.h"

#include <strsafe.h>
#include <unordered_set>

// ------------------------
//
// Chat_Net����
//
// ------------------------
namespace Library_Jingyu
{

	// ����ȭ ���� 1���� ũ��
	// �� ������ ���� ������ �����ؾ� ��.
	LONG g_lNET_BUFF_SIZE = 512;

	// ������ 
	CCrashDump* g_ChatDump = CCrashDump::GetInstance();
	CSystemLog* g_ChatLog = CSystemLog::GetInstance();


	// -----------------------
	// ������
	// -----------------------

	// ��Ʈ��Ʈ ������
	UINT WINAPI CChatServer_Room::HeartBeatThread(LPVOID lParam)
	{
		CChatServer_Room* g_this = (CChatServer_Room*)lParam;

		// [���� ��ȣ] ���ڷ� �޾Ƶα�
		HANDLE hEvent = g_this->m_hHBThreadExitEvent;

		// �˴ٿ� ��ų SessionID�� �޾Ƶ� Array.
		// ���� Array�� �δ� ����
		// 1. �� ���� ����
		// 2. ����� ���� ���ϱ�
		// --> Disconnect() �ȿ���, �˴ٿ� �Ŀ�, GetSessionUnLOCK()�Լ����� InDisconnect()�� �� �� �ִ�
		// --> InDisconenct()������ OnClientLeave()�� ȣ���ϴµ�, �� �ȿ��� Erase�ϱ� ���� �ٽ� ���� �Ǵ�.
		// --> ��, ����� �߻� ���ɼ�
		ULONGLONG* SessionArray = new ULONGLONG[g_this->m_Paser.MaxJoinUser];

		while (1)
		{
			// ��� (1�ʿ� 1ȸ �����)
			DWORD Check = WaitForSingleObject(hEvent, 1000);

			// �̻��� ��ȣ���
			if (Check == WAIT_FAILED)
			{
				DWORD Error = GetLastError();
				printf("JobAddThread Exit Error!!! (%d) \n", Error);
				break;
			}

			// ����, ���� ��ȣ�� �Դٸ� ��Ʈ��Ʈ ������ ����.
			else if (Check == WAIT_OBJECT_0)
				break;

			// �� ������ �ƴ϶��, ���� �Ѵ�.	

			int Size = 0;

			AcquireSRWLockShared(&g_this->m_Player_Umap_srwl);	// �� -----------

			// ���� �ڷᱸ���� ����� 0 �̻����� üũ �� ���� ����
			if (g_this->m_Player_Umap.size() > 0)
			{
				auto itor_Begin = g_this->m_Player_Umap.begin();
				auto itor_End = g_this->m_Player_Umap.end();

				while (itor_Begin != itor_End)
				{
					// ������ ��Ŷ�� ������ 30�� �̻��� �Ǿ��ٸ�
					// !! ��� ��� ������ ���� �� �ֱ� ������, ������ üũ�Ǿ�� �� !!
					// !! �׷��� ��� ��� ���� int������ �޴´� !!
					int Time = (timeGetTime() - itor_Begin->second->m_dwLastPacketTime);
					if (Time > 30000)
					{
						SessionArray[Size] = itor_Begin->second->m_ullSessionID;

						// ��Ʈ��Ʈ �α� �����
						// �α� ��� (�α� ���� : ����)
						g_ChatLog->LogSave(false, L"ChatServer", CSystemLog::en_LogLevel::LEVEL_ERROR, L"HeartBeat!! AccountNo : %lld, Time : %d", 
							itor_Begin->second->m_i64AccountNo, Time);

						Size++;
					}

					++itor_Begin;
				}
			}

			ReleaseSRWLockShared(&g_this->m_Player_Umap_srwl);	// ��� -----------

			// Size�� �ִٸ�, �ݺ��� ���鼭 �˴ٿ� ����
			if (Size > 0)
			{
				// ���� ���, SessionArray�ȿ� 10���� �����Ͱ� ��������� Size�� 10
				// Size�� �ε����� ����� ���̱� ������ 1 ����
				// ���� ������ �δ� �� ����, '0'�� ���� �������� ���ϴ� ���� �ξ� �ӵ��� ������ ������ 
				// �̷��� ó��.
				Size--;

				while (Size >= 0)
				{
					g_this->Disconnect(SessionArray[Size]);
					
					// �ش� ������ ���⿡���� �����ϱ� ������, ���Ͷ� ���ص� ����.
					g_this->m_lHeartBeat++;

					Size--;
				}
			}
		}

		printf("HB_Thread Exit!!\n");

		delete[] SessionArray;

		return 0;
	}





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
	// Parameter : AccountNo, SessionID
	// return : ���� �߰� �� true
	//		  : Ű �ߺ� �� flase
	bool CChatServer_Room::InsertLoginPlayerFunc(INT64 AccountNo, ULONGLONG SessionID)
	{
		AcquireSRWLockExclusive(&m_LoginPlayer_Umap_srwl);	 // ----- �α��� Player Umap Exclusive ��

		// 1. �߰�
		auto ret = m_LoginPlayer_Umap.insert(make_pair(AccountNo, SessionID));

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

		// ��ã������ false ����
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




	// ----------------------------
	// �� ���� �ڷᱸ�� �Լ�
	// ----------------------------

	// �� �ڷᱸ���� Insert
	//
	// Parameter : RoonNo, stROom*
	// return : ���� �߰� �� true
	//		  : Ű �ߺ� �� flase
	bool CChatServer_Room::InsertRoomFunc(int RoomNo, stRoom* InsertRoom)
	{
		AcquireSRWLockExclusive(&m_Room_Umap_srwl);	// ----- �� Exclusive ��

		// 1. �߰�
		auto ret = m_Room_Umap.insert(make_pair(RoomNo, InsertRoom));

		if (ret.second == false)
		{
			ReleaseSRWLockExclusive(&m_Room_Umap_srwl);	// ----- �� Exclusive ���
			return false;
		}

		ReleaseSRWLockExclusive(&m_Room_Umap_srwl);	// ----- �� Exclusive ���

		return true;
	}
	
	// �� �ڷᱸ������ Erase
	//
	// Parameter : RoonNo
	// return : ���������� ���� �� stRoom*
	//		  : �˻� ���� �� nullptr
	CChatServer_Room::stRoom* CChatServer_Room::EraseRoomFunc(int RoomNo)
	{
		AcquireSRWLockExclusive(&m_Room_Umap_srwl);		// ----- �� Exclusive ��

		// 1. �˻�
		auto FindRoom = m_Room_Umap.find(RoomNo);

		if (FindRoom == m_Room_Umap.end())
		{
			ReleaseSRWLockExclusive(&m_Room_Umap_srwl);		// ----- �� Exclusive ���
			return nullptr;
		}

		// 2. �� ã������ Erase
		stRoom* RetRoom = FindRoom->second;

		m_Room_Umap.erase(FindRoom);

		ReleaseSRWLockExclusive(&m_Room_Umap_srwl);		// ----- �� Exclusive ���

		return RetRoom;
	}

	// �� �ڷᱸ���� ��� �� ����
	//
	// Parameter : ����
	// return : ����
	void CChatServer_Room::RoomClearFunc()
	{
		// 1. �˴ٿ� ���� ���� ���� ���.
		// !! �� �ɰ� �˴ٿ��� ������ ����� ���� !!
		ULONGLONG* SessionArray = new ULONGLONG[m_Paser.MaxJoinUser];
		int Count = 0;

		AcquireSRWLockExclusive(&m_Room_Umap_srwl);		// ----- �� Exclusive ��		

		// 2. ���� ���� ��� �� Ŭ���� ����
		if (m_Room_Umap.size() > 0)
		{
			auto itor_Now = m_Room_Umap.begin();
			auto itor_End = m_Room_Umap.end();

			while (itor_Now != itor_End)
			{
				// �ش� �濡 ������ �ִ� ���, �� ���� ��� �������� �˴ٿ� ����.
				// �� ����, OnClientLeave���� �� �ı�
				if (itor_Now->second->m_iJoinUser > 0)
				{
					stRoom* NowRoom = itor_Now->second;

					NowRoom->m_bDeleteFlag = true;

					size_t Size = NowRoom->m_JoinUser_vector.size();

					if (Size != NowRoom->m_iJoinUser)
						g_ChatDump->Crash();

					size_t Index = 0;
					while (Index < Size)
					{
						// �˴ٿ� �� SessionID�� �޾Ƶд�.
						SessionArray[Count] = NowRoom->m_JoinUser_vector[Index];
						++Index;
						++Count;
					}

					++itor_Now;
				}

				// �濡 ������ ���� ���, ��� ����
				else
				{
					m_pRoom_Pool->Free(itor_Now->second);
					InterlockedDecrement(&m_lRoomCount);
					itor_Now = m_Room_Umap.erase(itor_Now);
				}
			}
		}

		ReleaseSRWLockExclusive(&m_Room_Umap_srwl);		// ----- �� Exclusive ���

		// 3. �˴ٿ� ������ �������, �˴ٿ� ����
		if (Count > 0)
		{
			// Count�� �ε��� ������ �ؾ��ϱ� ������ --�Ѵ�.
			// ��������, 0������ ���������� ���ϴ°��� ������ ������.
			--Count;

			while (Count >=0)
			{
				Disconnect(SessionArray[Count]);
				--Count;
			}
			
		}
		
		delete[] SessionArray;
	}





	// --------------
	// �ܺο��� ȣ�� ������ �Լ�
	// --------------

	// ��¿� �Լ�
	void  CChatServer_Room::ShowPrintf()
	{
		// �ش� ���μ����� ��뷮 üũ�� Ŭ����
		static CCpuUsage_Process ProcessUsage;

		// ȭ�� ����� �� ����
		/*
		MonitorConnect : %d, BattleConnect : %d	- ����͸� ������ ���� ����, ��Ʋ �� ������ ���� ����. 1�̸� ������.
		SessionNum : 	- NetServer �� ���Ǽ�
		PacketPool_Net : 	- �ܺο��� ��� ���� Net ����ȭ ������ ��
		HeartBeat Flag :			- ��Ʈ��Ʈ ������. 1�̸� ��Ʈ��Ʈ ��

		PlayerData_Pool :	- Player ����ü �Ҵ緮
		Player Count : 		- Contents ��Ʈ Player ����

		Accept Total :		- Accept ��ü ī��Ʈ (accept ���Ͻ� +1)
		Accept TPS :		- Accept ó�� Ƚ��
		Send TPS			- �ʴ� Send�Ϸ� Ƚ��. (�Ϸ��������� ����)		

		Net_BuffChunkAlloc_Count : - Net ����ȭ ���� �� Alloc�� ûũ �� (�ۿ��� ������� ûũ ��)
		Chat_PlayerChunkAlloc_Count : - �÷��̾� �� Alloc�� ûũ �� (�ۿ��� ������� ûũ ��)

		TotalRoom_Pool :	- �� Umap�� Size
		TotalRoom :			- �� ��
		Room_ChunkAlloc_Count : - �� �� Alloc�� ûũ ��(�ܺο��� ��� ûũ ��)

		Server_EnterToken_Miss : 		- ä�ü��� ���� ��ū �̽�
		Room_EnterTokenNot_Miss : 	- �� ������ū �̽�
		Sem Count :					- 121 ���� �߻� ��
		Login_Overlap :				- �ߺ��α��� ��
		HeartBeat_Count :			- ��Ʈ��Ʈ�� ���� ���� ī��Ʈ

		----------------------------------------------------
		PacketPool_Lan : 	- �ܺο��� ��� ���� Lan ����ȭ ������ ��

		Lan_BuffChunkAlloc_Count : - Lan ����ȭ ���� �� Alloc�� ûũ �� (�ۿ��� ������� ûũ ��)

		----------------------------------------------------
		CPU usage [ChatServer:%.1f%% U:%.1f%% K:%.1f%%] - ���μ��� ��뷮.

		*/

		// ��� ����, ���μ��� ��뷮 ����
		ProcessUsage.UpdateCpuTime();

		printf("==================== Chat Server =====================\n"
			"MonitorConnect : %d, BattleConnect : %d\n"
			"SessionNum : %lld\n"
			"PacketPool_Net : %d\n"
			"HeartBeat Flag : %d\n\n"

			"PlayerData_Pool : %d\n"
			"Player Count : %lld\n\n"

			"Accept Total : %lld\n"
			"Accept TPS : %d\n"
			"Send TPS : %d\n"
			"Recv TPS : %d\n\n"

			"Net_BuffChunkAlloc_Count : %d (Out : %d)\n"
			"Chat_PlayerChunkAlloc_Count : %d (Out : %d)\n\n"

			"Server_EnterToken_Miss : %d\n"
			"Room_EnterTokenNot_Miss : %d\n"
			"Sem_Count : %d\n"
			"Login_Overlap : %d\n"
			"HeartBeat_Count : %d\n\n"

			"------------------------------------------------\n"
			"TotalRoom_Pool : %lld\n"
			"TotalRoom : %d\n"
			"Room_ChunkAlloc_Count : %d (Out : %d)\n\n"

			"------------------------------------------------\n"
			"PacketPool_Lan : %d\n\n"

			"Lan_BuffChunkAlloc_Count : %d (Out : %d)\n\n"

			"========================================================\n\n"
			"CPU usage [ChatServer:%.1f%% U:%.1f%% K:%.1f%%]\n",

			// ----------- ä�� ������
			m_pMonitor_Client->GetClinetState(), m_pBattle_Client->GetClinetState(),
			GetClientCount(),
			CProtocolBuff_Net::GetNodeCount(),
			m_Paser.HeartBeat,

			m_lUpdateStruct_PlayerCount,
			m_Player_Umap.size(),

			GetAcceptTotal(),
			GetAccpetTPS(),
			GetSendTPS(),
			GetRecvTPS(),			

			CProtocolBuff_Net::GetChunkCount(), CProtocolBuff_Net::GetOutChunkCount(),
			m_pPlayer_Pool->GetAllocChunkCount(), m_pPlayer_Pool->GetOutChunkCount(),

			m_lEnterTokenMiss,
			m_lRoom_EnterTokenMiss,
			GetSemCount(),
			m_lLoginOverlap,
			m_lHeartBeat,

			m_Room_Umap.size(),
			m_lRoomCount,
			m_pRoom_Pool->GetAllocChunkCount(), m_pRoom_Pool->GetOutChunkCount(),

			// ----------- ��Ʋ �� Ŭ���̾�Ʈ��
			CProtocolBuff_Lan::GetNodeCount(),
			CProtocolBuff_Lan::GetChunkCount(), CProtocolBuff_Lan::GetOutChunkCount(),

			// ----------- ���μ��� ��뷮 
			ProcessUsage.ProcessTotal(), ProcessUsage.ProcessUser(), ProcessUsage.ProcessKernel());

	}
	   	  
	// ���� ����
	//
	// Parameter : ����
	// return : ���� �� true
	//		  : ���� �� false
	bool CChatServer_Room::ServerStart()
	{
		// ���� �ʱ�ȭ
		m_lChatLoginCount = 0;
		m_lUpdateStruct_PlayerCount = 0;
		m_lEnterTokenMiss = 0;
		m_lRoom_EnterTokenMiss = 0;
		m_lRoomCount = 0;
		m_lLoginOverlap = 0;
		m_lHeartBeat = 0;


		// ����͸� ������ ����Ǵ� �� Ŭ�� ����
		if (m_pMonitor_Client->ClientStart(m_Paser.MonitorServerIP, m_Paser.MonitorServerPort, m_Paser.MonitorClientCreateWorker,
			m_Paser.MonitorClientActiveWorker, m_Paser.MonitorClientNodelay) == false)
		{
			return false;
		}

		// ��Ʋ�� ����Ǵ� �� Ŭ�� ����
		if (m_pBattle_Client->ClientStart(m_Paser.BattleServerIP, m_Paser.BattleServerPort, m_Paser.BattleClientCreateWorker,
			m_Paser.BattleClientActiveWorker, m_Paser.BattleClientNodelay) == false)
		{
			return false;
		}

		// ä�� Net ���� ����
		if (Start(m_Paser.BindIP, m_Paser.Port, m_Paser.CreateWorker, m_Paser.ActiveWorker, m_Paser.CreateAccept,
			m_Paser.Nodelay, m_Paser.MaxJoinUser, m_Paser.HeadCode, m_Paser.XORCode) == false)
		{
			// �α� ��� (�α� ���� : ����)
			g_ChatLog->LogSave(false, L"ChatServer", CSystemLog::en_LogLevel::LEVEL_ERROR, L"Net ServerOpenError");
			
			return false;
		}	

		// ���Ǳ� ���Ͽ��� ��Ʈ��Ʈ ���ΰ� TRUE��� ��Ʈ��Ʈ ������ ����
		if (m_Paser.HeartBeat != 0)
		{
			// ��Ʈ��Ʈ ������ ����
			m_hHBthreadHandle = (HANDLE)_beginthreadex(NULL, 0, HeartBeatThread, this, 0, 0);
		}
		
		// ���� ���� �α� ��� (�α� ���� : ����)
		g_ChatLog->LogSave(true, L"ChatServer", CSystemLog::en_LogLevel::LEVEL_SYSTEM, L"ServerOpen...");

		return true;
	}

	// ���� ����
	//
	// Parameter : ����
	// return : ����
	void CChatServer_Room::ServerStop()
	{
		// ����͸� ������ ����Ǵ� �� Ŭ�� ����
		m_pMonitor_Client->Stop();

		// ��Ʋ�� ����Ǵ� �� Ŭ�� ����
		m_pBattle_Client->Stop();		

		// ä�� Net ���� ����
		Stop();

		// ��Ʈ��Ʈ�� üũ�߾��ٸ�, ��Ʈ��Ʈ ������ ����
		if (m_Paser.HeartBeat != 0)
		{
			// ��Ʈ��Ʈ ������ ����
			SetEvent(m_hHBThreadExitEvent);

			// ��Ʈ��Ʈ �����尡 �̻��� ��ȣ�� ����Ǿ��ٸ�, 
			if (WaitForSingleObject(m_hHBthreadHandle, INFINITE) != WAIT_OBJECT_0)
			{
				// ���ῡ �����Ѵٸ�, ���� ���
				DWORD Error = GetLastError();
				g_ChatLog->LogSave(false, L"MatchServer", CSystemLog::en_LogLevel::LEVEL_ERROR,
					L"ServerStop() --> HBThread Exit Error!!!(%d)", Error);

				g_ChatDump->Crash();
			}
		}

		// ���� ���� �α� ��� (�α� ���� : ����)
		g_ChatLog->LogSave(true, L"ChatServer", CSystemLog::en_LogLevel::LEVEL_SYSTEM, L"All Server Stop...");
	}






	// ------------------
	// ���ο����� ����ϴ� �Լ�
	// ------------------

	// �� ���� ��� �������� ���ڷ� ���� ��Ŷ ������.
	//
	// Parameter : ULONGLONG �迭, �迭�� ��,  CProtocolBuff_Net*
	// return : �ڷᱸ���� 0���̸� false. �� �ܿ��� true
	bool CChatServer_Room::Room_BroadCast(ULONGLONG Array[], int ArraySize, CProtocolBuff_Net* SendBuff)
	{
		// ����ȭ ���� ���۷��� ī��Ʈ ����
		SendBuff->Add((int)ArraySize);

		// ó������ ��ȸ�ϸ� ������.
		int Index = 0;

		while (Index < ArraySize)
		{
			SendPacket(Array[Index], SendBuff);

			++Index;
		}

		return true;
	}

	// Config ����
	//
	// Parameter : stParser*
	// return : ���� �� false
	bool CChatServer_Room::SetFile(stParser* pConfig)
	{
		Parser Parser;

		// ���� �ε�
		try
		{
			Parser.LoadFile(_T("ChatServer_Config.ini"));
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
		// ä�� �� ������ config �о����
		////////////////////////////////////////////////////////

		// ���� ���� -------------------------
		if (Parser.AreaCheck(_T("CHATSERVER")) == false)
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

		// xorcode
		if (Parser.GetValue_Int(_T("XorCode"), &pConfig->XORCode) == false)
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

		// ��Ʈ��Ʈ ����
		if (Parser.GetValue_Int(_T("HeartBeat"), &pConfig->HeartBeat) == false)
			return false;



		////////////////////////////////////////////////////////
		// config �о����
		////////////////////////////////////////////////////////
			   		 
		// ���� ���� -------------------------
		if (Parser.AreaCheck(_T("CONFIG")) == false)
			return false;

		// �ܺ� IP
		if (Parser.GetValue_String(_T("ChatIP"), pConfig->ChatIP) == false)
			return false;




		////////////////////////////////////////////////////////
		// ä�� LanClient config �о����
		////////////////////////////////////////////////////////

		// ���� ���� -------------------------
		if (Parser.AreaCheck(_T("CHATLANCLIENT")) == false)
			return false;

		// IP
		if (Parser.GetValue_String(_T("BattleServerIP"), pConfig->BattleServerIP) == false)
			return false;

		// Port
		if (Parser.GetValue_Int(_T("BattleServerPort"), &pConfig->BattleServerPort) == false)
			return false;

		// ���� ��Ŀ ��
		if (Parser.GetValue_Int(_T("ClientCreateWorker"), &pConfig->BattleClientCreateWorker) == false)
			return false;

		// Ȱ��ȭ ��Ŀ ��
		if (Parser.GetValue_Int(_T("ClientActiveWorker"), &pConfig->BattleClientActiveWorker) == false)
			return false;


		// Nodelay
		if (Parser.GetValue_Int(_T("ClientNodelay"), &pConfig->BattleClientNodelay) == false)
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


		// ���������� ��Ŷ �����ð� ����
		NowPlayer->m_dwLastPacketTime = timeGetTime();



		// 4. "����" ��ū üũ
		// ��ū �� �� ����� ���ɼ��� �ֱ� ������ ���� �Ǵ�.

		AcquireSRWLockShared(&m_ServerEnterToken_srwl);		// ----- ��ū Shared ��

		if (memcmp(m_cConnectToken_Now, ConnectToken, 32) != 0)
		{
			// �ٸ��� "����" ��Ŷ�� ��
			if (memcmp(m_cConnectToken_Before, ConnectToken, 32) != 0)
			{
				ReleaseSRWLockShared(&m_ServerEnterToken_srwl);		// ----- ��ū Shared ���
				InterlockedIncrement(&m_lEnterTokenMiss);

				// ���� ���
				g_ChatLog->LogSave(false, L"ChatServer", CSystemLog::en_LogLevel::LEVEL_ERROR, L"Server Token Error !!  AccountNo : %lld", AccountNo);

				// �׷��� �ٸ���, ���� ��Ŷ
				CProtocolBuff_Net* SendBuff = CProtocolBuff_Net::Alloc();

				WORD Type = en_PACKET_CS_CHAT_RES_LOGIN;
				BYTE Status = 0;

				SendBuff->PutData((char*)&Type, 2);
				SendBuff->PutData((char*)&Status, 1);
				SendBuff->PutData((char*)&AccountNo, 8);

				SendPacket(SessionID, SendBuff);

				return;
			}
		}
		
		ReleaseSRWLockShared(&m_ServerEnterToken_srwl);		// ----- ��ū Shared ���
			

		// 5. AccountNo ����
		NowPlayer->m_i64AccountNo = AccountNo;



		// 6. �α��� �÷��̾� ������ �ڷᱸ���� �߰� (�ߺ� �α��� üũ)
		if (InsertLoginPlayerFunc(AccountNo, SessionID) == false)
		{
			InterlockedIncrement(&m_lLoginOverlap);

			// �ߺ��α��� �α� �����
			g_ChatLog->LogSave(false, L"ChatServer", CSystemLog::en_LogLevel::LEVEL_ERROR, L"Overlapped Login!! AccountNo : %lld", AccountNo);

			// ���� ��Ŷ -------
			CProtocolBuff_Net* SendBuff = CProtocolBuff_Net::Alloc();

			WORD Type = en_PACKET_CS_CHAT_RES_LOGIN;
			BYTE Status = 2;

			SendBuff->PutData((char*)&Type, 2);
			SendBuff->PutData((char*)&Status, 1);
			SendBuff->PutData((char*)&AccountNo, 8);

			// ������ ����
			SendPacket(SessionID, SendBuff, TRUE);


			// ������ �������̴� ���� ���� ����	-------

			AcquireSRWLockShared(&m_LoginPlayer_Umap_srwl);	// ----- �α��� Player Shared ��

			// 1) �˻�
			auto FindPlayer = m_LoginPlayer_Umap.find(AccountNo);

			// 2) ���� �� ����. �� ������ ó���ϴµ� ������� ���ɼ�
			if (FindPlayer == m_LoginPlayer_Umap.end())
			{
				ReleaseSRWLockShared(&m_LoginPlayer_Umap_srwl);	// ----- �α��� Player Shared ���
				return;
			}

			ULONGLONG TempSessionID = FindPlayer->second;

			// ���� ��Ǯ�� Disconnect�ϸ� ����� ���ɼ�
			ReleaseSRWLockShared(&m_LoginPlayer_Umap_srwl);	// ----- �α��� Player Shared ���

			// 3) ���� ���� ��û
			Disconnect(TempSessionID);

			return;
		}

		// 7. �� �߰������� �÷��� ����.
		NowPlayer->m_bLoginCheck = true;

		// �α��� ��Ŷ���� ó���� ���� ī��Ʈ 1 ����
		InterlockedIncrement(&m_lChatLoginCount);


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

		// ���������� ��Ŷ �����ð� ����
		NowPlayer->m_dwLastPacketTime = timeGetTime();

		// 5. �� �˻�
		AcquireSRWLockShared(&m_Room_Umap_srwl);		// ----- �� Shared ��

		auto FindRoom = m_Room_Umap.find(RoomNo);

		if (FindRoom == m_Room_Umap.end())
		{
			ReleaseSRWLockShared(&m_Room_Umap_srwl);		// ----- �� Shared ���

			// ���� ���
			g_ChatLog->LogSave(false, L"ChatServer", CSystemLog::en_LogLevel::LEVEL_ERROR, L"Not Find Room !!  AccountNo : %lld, TryRoomNo : %d", AccountNo, RoomNo);

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

		// 6. ��ū �˻�
		// !! �� �� �ɰ� �� �ʿ� ����. !!
		// !! �� ����, ���� ���� ��ūó�� �߰��� ����Ǵ� ��찡 ���� ������, �� ���̺� ���� �ɸ� �����ϴ� !!
		if (memcmp(EnterToken, NowRoom->m_cEnterToken, 32) != 0)
		{	
			ReleaseSRWLockShared(&m_Room_Umap_srwl);		// ----- �� Shared ���

			InterlockedIncrement(&m_lRoom_EnterTokenMiss);

			// ���� ���
			g_ChatLog->LogSave(false, L"ChatServer", CSystemLog::en_LogLevel::LEVEL_ERROR, L"Room Token Error !!  AccountNo : %lld", AccountNo);

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

		// 7. ���� ��Ŷ ����
		// !! �濡 ������ �߰��Ǵ� ����, ä�� �޽����� ���� �� �ִ� ���°� �ȴ�. !!
		// !! ��, Ŭ�� ���忡���� �� ���� ���� ��Ŷ�� �ȹ޾Ҵµ� �濡 �ִ� �޽����� �޴´�. !!
		// !! ������, �̸� ������Ŷ�� ���� ������ �濡 �߰��ؾ� �Ѵ� !!
		CProtocolBuff_Net* SendBuff = CProtocolBuff_Net::Alloc();

		WORD Type = en_PACKET_CS_CHAT_RES_ENTER_ROOM;
		BYTE Status = 1;	// ����

		SendBuff->PutData((char*)&Type, 2);
		SendBuff->PutData((char*)&AccountNo, 8);
		SendBuff->PutData((char*)&RoomNo, 4);
		SendBuff->PutData((char*)&Status, 1);

		SendPacket(SessionID, SendBuff);


		// 8. �� ��ū �˻� �� �ڷᱸ���� �߰�
		NowRoom->ROOM_LOCK();	// ----- �� ��		

		// ����, ���� ������ ���̶�� �� ���� ��Ŷ ����
		if (NowRoom->m_bDeleteFlag == true)
		{
			NowRoom->ROOM_UNLOCK();	// ----- �� ���	
			ReleaseSRWLockShared(&m_Room_Umap_srwl);		// ----- �� Shared ���		

			// ���� ���
			g_ChatLog->LogSave(false, L"ChatServer", CSystemLog::en_LogLevel::LEVEL_ERROR, L"Not Find Room(2) !!  AccountNo : %lld, TryRoomNo : %d", AccountNo, RoomNo);

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

		// �� �� �ڷᱸ���� ���� �߰�
		NowRoom->Insert(SessionID);

		// �� �� ���� �� ����
		++NowRoom->m_iJoinUser;

		// �������� �� ��ȣ �Ҵ�
		NowPlayer->m_iRoomNo = RoomNo;		

		NowRoom->ROOM_UNLOCK();	// ----- �� ���	

		ReleaseSRWLockShared(&m_Room_Umap_srwl);		// ----- �� Shared ���		
		
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

		// ���������� ��Ŷ �����ð� ����
		NowPlayer->m_dwLastPacketTime = timeGetTime();



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
		// SessionID�� �޾Ƶд�. ���� �ɰ� Room_BroadCast�� �ϸ� ����� ���ɼ�
		ULONGLONG SessionArray[10];
		int ArraySize = 0;

		NowRoom->ROOM_LOCK();	// ----- �� ��	

		size_t Size = NowRoom->m_JoinUser_vector.size();
		if (Size == 0)
			g_ChatDump->Crash();

		while (ArraySize < Size)
		{
			SessionArray[ArraySize] = NowRoom->m_JoinUser_vector[ArraySize];
			ArraySize++;
		}

		NowRoom->ROOM_UNLOCK();	// ----- �� ���	
		ReleaseSRWLockShared(&m_Room_Umap_srwl);		// ----- �� Shared ���

		// BoradCast		
		if(Room_BroadCast(SessionArray, ArraySize, SendBuff) == false)
			g_ChatDump->Crash();		

		// BroadCast�ߴ� ��Ŷ Free
		// Room_BroadCast() �Լ�����, ���� �� ��ŭ ���۷��� ī��Ʈ�� �������ױ� ������
		// ���⼭ 1 ���ҽ������ �Ѵ� 
		CProtocolBuff_Net::Free(SendBuff);
	}

	// ��Ʈ��Ʈ
	//
	// Parameter : SessionID
	// return : ����
	void CChatServer_Room::Packet_HeartBeat(ULONGLONG SessionID)
	{
		// 1. �÷��̾� �˻�
		stPlayer* NowPlayer = FindPlayerFunc(SessionID);

		// ������ ũ����
		if (NowPlayer == nullptr)
			g_ChatDump->Crash();

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

		// ���������� ��Ŷ �����ð� ����
		NowPlayer->m_dwLastPacketTime = timeGetTime();
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
		InterlockedIncrement(&m_lUpdateStruct_PlayerCount);

		// 2. �ʱ� ����
		NewPlayer->m_ullSessionID = SessionID;

		// ������ ��Ŷ ���� �ð� ����
		NewPlayer->m_dwLastPacketTime = timeGetTime();

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
			DeletePlayer->m_bLoginCheck = false;

			// �α��� ���� ī��Ʈ ����
			InterlockedDecrement(&m_lChatLoginCount);

			// �α��� �ڷᱸ������ ����
			if(EraseLoginPlayerFunc(DeletePlayer->m_i64AccountNo) == false)
				g_ChatDump->Crash();
		}		


		// 3. �뿡 �� �ִٸ�, �뿡�� ����
		if (RoomNo != -1)
		{
			int DeleteRoomNo = -1;

			// �� ��ȣ �ʱ�ȭ
			DeletePlayer->m_iRoomNo = -1;

			AcquireSRWLockShared(&m_Room_Umap_srwl);		// ----- �� Shared ��

			// �� �˻�
			auto FindRoom = m_Room_Umap.find(RoomNo);
			if (FindRoom == m_Room_Umap.end())
				g_ChatDump->Crash();

			stRoom* NowRoom = FindRoom->second;

			NowRoom->ROOM_LOCK();	// ----- �� ��

			// �� �� �ڷᱸ������ ���� ����
			if (NowRoom->Erase(SessionID) == false)
				g_ChatDump->Crash();

			// ���� ���� �� ����
			--NowRoom->m_iJoinUser;

			// ���� ���� ���� 0���̰�, �����÷��װ� true�� ���, ������ ��
			if (NowRoom->m_iJoinUser == 0 && NowRoom->m_bDeleteFlag == true)
				DeleteRoomNo = RoomNo;

			NowRoom->ROOM_UNLOCK();	// ----- �� ���

			ReleaseSRWLockShared(&m_Room_Umap_srwl);		// ----- �� Shared ���


			// �̹��� ������ ���� ����, ������ ���̶�� Erase
			if (DeleteRoomNo != -1)
			{
				// Erase
				stRoom* DeleteRoom = EraseRoomFunc(DeleteRoomNo);

				if (DeleteRoom == nullptr)
					g_ChatDump->Crash();

				InterlockedDecrement(&m_lRoomCount);

				// �� Free
				m_pRoom_Pool->Free(DeleteRoom);				
			}
		}

		// 4. stPlayer* Free
		m_pPlayer_Pool->Free(DeletePlayer);

		InterlockedDecrement(&m_lUpdateStruct_PlayerCount);
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
				Packet_HeartBeat(SessionID);
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
			g_ChatLog->LogSave(false, L"ChatServer", CSystemLog::en_LogLevel::LEVEL_ERROR, L"%s",
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
		g_ChatLog->LogSave(false, L"ChatServer", CSystemLog::en_LogLevel::LEVEL_ERROR, L"%s (ErrorCode : %d)",
			errorStr, error);

		g_ChatDump->Crash();
	}

	// �������� �߻� �� ȣ��Ǵ� �Լ�
	//
	// parameter : ����
	// return : ����
	void CChatServer_Room::OnSemaphore(ULONGLONG SessionID)
	{
		AcquireSRWLockShared(&m_Player_Umap_srwl);	 // ----- Player Umap Shared ��

		// 1. �˻�
		auto FindPlayer = m_Player_Umap.find(SessionID);

		// ���� ������� Crash
		if (FindPlayer == m_Player_Umap.end())
		{
			ReleaseSRWLockShared(&m_Player_Umap_srwl);	 // ----- Player Umap Shared ���
			g_ChatDump->Crash();
		}

		// 2. �ִٸ� AccountNo�� �޴´�.
		INT64 AccountNo = FindPlayer->second->m_i64AccountNo;

		ReleaseSRWLockShared(&m_Player_Umap_srwl);	 // ----- Player Umap Shared ���


		// 3. �α� ��� (�α� ���� : ����)
		g_ChatLog->LogSave(false, L"ChatServer", CSystemLog::en_LogLevel::LEVEL_ERROR, L"Semahore!! AccountNo : %lld", AccountNo);
	}





	// --------------
	// �����ڿ� �Ҹ���
	// --------------

	// ������
	CChatServer_Room::CChatServer_Room()
		: CNetServer()
	{
		// config
		if (SetFile(&m_Paser) == false)
			g_ChatDump->Crash();

		// �α�
		g_ChatLog->SetDirectory(L"ChatServer");
		g_ChatLog->SetLogLeve((CSystemLog::en_LogLevel)m_Paser.LogLevel);


		// ����� �� Ŭ�� �����Ҵ�
		m_pMonitor_Client = new CChat_MonitorClient;
		m_pMonitor_Client->SetNetServer(this);

		// ��Ʋ ��Ŭ�� �����Ҵ�
		m_pBattle_Client = new CChat_LanClient;
		m_pBattle_Client->SetNetServer(this);
		
		// �ڷᱸ�� ���� �̸� ��Ƶα�
		m_Player_Umap.reserve(5000);
		m_LoginPlayer_Umap.reserve(5000);

		// �� �ʱ�ȭ
		InitializeSRWLock(&m_Player_Umap_srwl);
		InitializeSRWLock(&m_LoginPlayer_Umap_srwl);
		InitializeSRWLock(&m_ServerEnterToken_srwl);

		// TLS Pool �����Ҵ�
		m_pPlayer_Pool = new CMemoryPoolTLS<stPlayer>(0, false);
		m_pRoom_Pool = new CMemoryPoolTLS<stRoom>(0, false);

		// ��Ʈ��Ʈ ������ ���� �뵵 Event
		// 
		// �ڵ� ���� Event 
		// ���� ���� �� non-signalled ����
		// �̸� ���� Event	
		m_hHBThreadExitEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	}

	// �Ҹ���
	CChatServer_Room::~CChatServer_Room()
	{
		// TLS Pool ��������
		delete m_pPlayer_Pool;
		delete m_pRoom_Pool;

		// �� Ŭ�� ��������
		delete m_pMonitor_Client;
		delete m_pBattle_Client;
	}
}


// -----------------------
//
// ��Ʋ������ ����Ǵ� Lan Ŭ��
//
// -----------------------
namespace Library_Jingyu
{
	// ----------------------
	// ���ο����� ����ϴ� �Լ�
	// ----------------------

	// ä�� �ݼ��� ����
	//
	// Parameter : CChatServer_Room*
	// return : ����
	void CChat_LanClient::SetNetServer(CChatServer_Room* NetServer)
	{
		m_pNetServer = NetServer;
	}





	// ----------------------
	// ��Ŷ ó�� �Լ�
	// ----------------------

	// �ű� ���� ����
	//
	// Parameter : ClientID, CProtocolBuff_Lan*
	// return : ����
	void CChat_LanClient::Packet_RoomCreate(ULONGLONG ClinetID, CProtocolBuff_Lan* Payload)
	{
		// �α��� ��Ŷ ó���ƴ��� Ȯ��
		if (m_bLoginCheck == false)
		{
			// ������ ���� ����
			TCHAR str[100];
			StringCchPrintf(str, 100, _T("Packet_RoomCreate(). Not LoginUser. SessionID : %lld"), ClinetID);

			throw CException(str);
		}

		// 1. �� Alloc
		CChatServer_Room::stRoom* NewRoom = m_pNetServer->m_pRoom_Pool->Alloc();


		// 2. ������ �� �� ����
		UINT	ReqSequence;	
		int RoomNo;

		Payload->GetData((char*)&NewRoom->m_iBattleServerNo, 4);
		Payload->GetData((char*)&RoomNo, 4);
		Payload->GetData((char*)&NewRoom->m_iMaxUser, 4);
		Payload->GetData(NewRoom->m_cEnterToken, 32);

		Payload->GetData((char*)&ReqSequence, 4);

		NewRoom->m_iRoomNo = RoomNo;
		NewRoom->m_iJoinUser = 0;
		NewRoom->m_bDeleteFlag = false;

		if(NewRoom->m_JoinUser_vector.size() != 0)
			g_ChatDump->Crash();

		InterlockedIncrement(&m_pNetServer->m_lRoomCount);

		// 3. �� �ڷᱸ���� �߰�
		if (m_pNetServer->InsertRoomFunc(RoomNo, NewRoom) == false)
			g_ChatDump->Crash();


		// 4. ������Ŷ ������
		CProtocolBuff_Lan* SendBuff = CProtocolBuff_Lan::Alloc();

		WORD Type = en_PACKET_CHAT_BAT_RES_CREATED_ROOM;

		SendBuff->PutData((char*)&Type, 2);
		SendBuff->PutData((char*)&RoomNo, 4);
		SendBuff->PutData((char*)&ReqSequence, 4);

		SendPacket(ClinetID, SendBuff);		   		 	  
	}

	// ���� ��ū �����
	//
	// Parameter : ClientID, CProtocolBuff_Lan*
	// return : ����
	void CChat_LanClient::Packet_TokenChange(ULONGLONG ClinetID, CProtocolBuff_Lan* Payload)
	{
		// �α��� ��Ŷ ó���ƴ��� Ȯ��
		if (m_bLoginCheck == false)
		{
			// ������ ���� ����
			TCHAR str[100];
			StringCchPrintf(str, 100, _T("Packet_TokenChange(). Not LoginUser. SessionID : %lld"), ClinetID);

			throw CException(str);
		}

		// 1. ������
		char Token[32];
		UINT ReqSequence;

		Payload->GetData((char*)Token, 32);
		Payload->GetData((char*)&ReqSequence, 4);		

		// ��ū ���� �� �α��� ��, �߸��� ��ū�� ���� �� �ֱ� ������ �� �Ǵ�.
		AcquireSRWLockExclusive(&m_pNetServer->m_ServerEnterToken_srwl);		// ----- ��ū Exclusve ��
	
		// 2. "����" ��ū�� "����" ��ū�� ����
		// ���ʷ� �ش� ��Ŷ�� �޾��� ��쿡��, �׳� memcpy �Ѵ�. 
		memcpy_s(m_pNetServer->m_cConnectToken_Before, 32, m_pNetServer->m_cConnectToken_Now, 32);

		// ��Ŷ���� ���� ��ū�� ���� ��ū�� ����
		memcpy_s(m_pNetServer->m_cConnectToken_Now, 32, Token, 32);

		ReleaseSRWLockExclusive(&m_pNetServer->m_ServerEnterToken_srwl);		// ----- ��ū Exclusve ���


		// 3. ���� ������
		CProtocolBuff_Lan* SendBuff = CProtocolBuff_Lan::Alloc();

		WORD Type = en_PACKET_CHAT_BAT_RES_CONNECT_TOKEN;

		SendBuff->PutData((char*)&Type, 2);
		SendBuff->PutData((char*)&ReqSequence, 8);


		SendPacket(ClinetID, SendBuff);
	}

	// �α��� ��Ŷ�� ���� ����
	//
	// Parameter : ClientID, CProtocolBuff_Lan*
	// return : ����
	void CChat_LanClient::Packet_Login(CProtocolBuff_Lan* Payload)
	{
		// �α��� ��Ŷ ó���ƴ��� Ȯ��
		// ������ �ȵ�.
		if (m_bLoginCheck == true)
		{
			// ������ ���� ����
			TCHAR str[100];
			StringCchPrintf(str, 100, _T("Packet_Login(). LoginUser"));

			throw CException(str);
		}

		// �� �÷��׷� �α��� ó���� �Ǿ����� Ȯ��.
		m_bLoginCheck = true;		
	}
	
	// �� ���� 
	//
	// Parameter : ClientID, CProtocolBuff_Lan*
	// return : ����
	void CChat_LanClient::Packet_RoomErase(ULONGLONG ClinetID, CProtocolBuff_Lan* Payload)
	{
		// �α��� ��Ŷ ó���ƴ��� Ȯ��
		if (m_bLoginCheck == false)
		{
			// ������ ���� ����
			TCHAR str[100];
			StringCchPrintf(str, 100, _T("Packet_RoomErase(). Not LoginUser. SessionID : %lld"), ClinetID);

			throw CException(str);
		}


		// 1. ������
		int		BattleServerNo;
		int		RoomNo;
		UINT	ReqSequence;			// �޽��� ������ ��ȣ (REQ / RES ¦���� �뵵)

		Payload->GetData((char*)&BattleServerNo, 4);
		Payload->GetData((char*)&RoomNo, 4);
		Payload->GetData((char*)&ReqSequence, 4);


		// 2. �� �˻�
		// �˴ٿ� ���� �迭
		ULONGLONG DisconnectArray[10];
		int ArraySize = 0;

		AcquireSRWLockExclusive(&m_pNetServer->m_Room_Umap_srwl);	 // ----- �� Exclusive ��

		auto FindRoom = m_pNetServer->m_Room_Umap.find(RoomNo);
		
		// �� �˻� ��, ���� ���� ���� ����.
		// ä���� ���� ���, ��Ʋ������ �����ϰ� �ִ� Wait ���� ��� �ı���Ų��. Play���� �����ִ�.
		// �ٽ� ä���� ������, ��Ʋ�� Play���� �ְ� ä���� ���� �ϳ��� ����.
		// �� ���¿��� Play���� ����Ǹ�, ä�ÿ��� �� ���� ��Ŷ�� �����µ� 
		// ä���� �ٽ� ������ ������ ������ �÷��� ���� ����.
		if (FindRoom == m_pNetServer->m_Room_Umap.end())
		{
			ReleaseSRWLockExclusive(&m_pNetServer->m_Room_Umap_srwl);	 // ----- �� Exclusive ���
			return;
		}

		CChatServer_Room::stRoom* NowRoom = FindRoom->second;


		// 3. ���� üũ
		
		// ��Ʋ���� ��ȣ 
		if(NowRoom->m_iBattleServerNo != BattleServerNo)
			g_ChatDump->Crash();

		// �� ��ȣ
		if(NowRoom->m_iRoomNo != RoomNo)
			g_ChatDump->Crash();


		// 4. �˴ٿ� ������
		// ���� ������ ���� 1�� �̻��̸�, �濡 �ִ� ��� �������� Shutdown ȣ��
		// �׸��� ������ �� �÷��� üũ
		if(NowRoom->m_iJoinUser < 0)
			g_ChatDump->Crash();

		if (NowRoom->m_iJoinUser > 0)
		{	
			NowRoom->m_bDeleteFlag = true;

			size_t Size = NowRoom->m_JoinUser_vector.size();

			if (Size != NowRoom->m_iJoinUser)
				g_ChatDump->Crash();

			ArraySize = Size;

			size_t Index = 0;
			while (Size > Index)
			{
				// ���⼭ �˴ٿ� ������ ����� ����.
				// �޾Ƶд�.
				DisconnectArray[Index] = NowRoom->m_JoinUser_vector[Index];	
				++Index;
			}

			ReleaseSRWLockExclusive(&m_pNetServer->m_Room_Umap_srwl);	 // ----- �� Exclusive ���
		}

		// 5. ������ ���� 0���̸� �� ��� ����
		else
		{	
			m_pNetServer->m_Room_Umap.erase(FindRoom);
			ReleaseSRWLockExclusive(&m_pNetServer->m_Room_Umap_srwl);	 // ----- �� Exclusive ���

			InterlockedDecrement(&m_pNetServer->m_lRoomCount);
		}		

		// 6. ����, ArraySize�� ������, �˴ٿ� ��ų ������ �ִ� ��.
		if (ArraySize > 0)
		{
			ArraySize--;

			while (ArraySize >=0)
			{
				m_pNetServer->Disconnect(DisconnectArray[ArraySize]);
				ArraySize--;
			}
		}



		// 7. ���� ��Ŷ
		// �� ��, ���� ���� �������� �ʾ��� ���� ������, �� ������ �����̱� ������ �������.
		CProtocolBuff_Lan* SendBuff = CProtocolBuff_Lan::Alloc();

		WORD Type = en_PACKET_CHAT_BAT_RES_DESTROY_ROOM;

		SendBuff->PutData((char*)&Type, 2);
		SendBuff->PutData((char*)&RoomNo, 4);
		SendBuff->PutData((char*)&ReqSequence, 4);

		SendPacket(ClinetID, SendBuff);
	}





	// ----------------------
	// �ܺο��� ��� ������ �Լ�
	// ----------------------

	// ���� �Լ�
	// ����������, ��ӹ��� CLanClient�� Startȣ��.
	//
	// Parameter : ������ ������ IP, ��Ʈ, ��Ŀ������ ��, Ȱ��ȭ��ų ��Ŀ������ ��, TCP_NODELAY ��� ����(true�� ���)
	// return : ���� �� true , ���� �� falsel 
	bool CChat_LanClient::ClientStart(TCHAR* ConnectIP, int Port, int CreateWorker, int ActiveWorker, int Nodelay)
	{
		// ��Ʋ �������� ����
		if (Start(ConnectIP, Port, CreateWorker, ActiveWorker, Nodelay) == false)
		{
			// ��Ʋ ��Ŭ�� ���� ����
			g_ChatLog->LogSave(false, L"ChatServer", CSystemLog::en_LogLevel::LEVEL_ERROR, L"Battle LanClient Start Error");

			return false;
		}
		

		// ��Ʋ ��Ŭ�� ���� �α�
		g_ChatLog->LogSave(true, L"ChatServer", CSystemLog::en_LogLevel::LEVEL_SYSTEM, L"Battle LanClient Start...");

		return true;
	}

	// Ŭ���̾�Ʈ ����
	//
	// Parameter : ����
	// return : ����
	void CChat_LanClient::ClientStop()
	{
		Stop();

		// ��Ʋ ��Ŭ�� ���� �α�
		g_ChatLog->LogSave(true, L"ChatServer", CSystemLog::en_LogLevel::LEVEL_SYSTEM, L"Battle LanClient Stop...");
	}






	// -----------------------
	// �����Լ�
	// -----------------------

	// ��ǥ ������ ���� ���� ��, ȣ��Ǵ� �Լ� (ConnectFunc���� ���� ���� �� ȣ��)
	//
	// parameter : ����Ű
	// return : ����
	void CChat_LanClient::OnConnect(ULONGLONG ClinetID)
	{
		// ���� ����, Ȥ�� �����ڿ��� �ʱ�ȭ�߱� ������,
		// ������ 0xffffffffffffffff�̿��� �Ѵ�.
		if (m_ullSessionID != 0xffffffffffffffff)
			g_ChatDump->Crash();

		m_ullSessionID = ClinetID;


		// ��Ʋ�������� ���� ��Ŷ ����
		CProtocolBuff_Lan* SendBuff = CProtocolBuff_Lan::Alloc();

		WORD Type = en_PACKET_CHAT_BAT_REQ_SERVER_ON;

		SendBuff->PutData((char*)&Type, 2);
		SendBuff->PutData((char*)m_pNetServer->m_Paser.ChatIP, 32);
		SendBuff->PutData((char*)&m_pNetServer->m_Paser.Port, 2);

		SendPacket(ClinetID, SendBuff);
	}

	// ��ǥ ������ ���� ���� �� ȣ��Ǵ� �Լ� (InDIsconnect �ȿ��� ȣ��)
	//
	// parameter : ����Ű
	// return : ����
	void CChat_LanClient::OnDisconnect(ULONGLONG ClinetID)
	{
		m_ullSessionID = 0xffffffffffffffff;
		m_bLoginCheck = false;

		// ��Ʋ������ ������ ����� ��� �� ����
		m_pNetServer->RoomClearFunc();
	}

	// ��Ŷ ���� �Ϸ� �� ȣ��Ǵ� �Լ�.
	//
	// parameter : ����Ű, ���� ��Ŷ
	// return : ����
	void CChat_LanClient::OnRecv(ULONGLONG ClinetID, CProtocolBuff_Lan* Payload)
	{
		// ���� ���� Ȯ��
		if (m_ullSessionID == 0xffffffffffffffff ||
			m_ullSessionID != ClinetID)
		{
			g_ChatDump->Crash();
		}

		// Ÿ�� Ȯ��
		WORD Type;
		Payload->GetData((char*)&Type, 2);

		try
		{
			// Ÿ�Կ� ���� �б�ó��
			switch (Type)
			{
				// �ű� ���� ����
			case en_PACKET_CHAT_BAT_REQ_CREATED_ROOM:
				Packet_RoomCreate(ClinetID, Payload);
				break;

				// �� ����
			case en_PACKET_CHAT_BAT_REQ_DESTROY_ROOM:
				Packet_RoomErase(ClinetID, Payload);
				break;

				// ���� ��ū �����
			case en_PACKET_CHAT_BAT_REQ_CONNECT_TOKEN:
				Packet_TokenChange(ClinetID, Payload);
				break;
			
				// ���� ������ ���� ����(�̹� ���´� �α��� ��û�� ���� ����)
			case en_PACKET_CHAT_BAT_RES_SERVER_ON:
				Packet_Login(Payload);
			break;

				// ���� Ÿ���̸� ũ����
			default:
				
				TCHAR str[100];
				StringCchPrintf(str, 100, _T("BattleLanClient. OnRecv(). TypeError. SessionID : %lld, Type : %d"), ClinetID, Type);

				throw CException(str);
				break;
			}

		}
		catch (CException& exc)
		{
			// �α� ��� (�α� ���� : ����)
			g_ChatLog->LogSave(false, L"ChatServer", CSystemLog::en_LogLevel::LEVEL_ERROR, L"%s",
				(TCHAR*)exc.GetExceptionText());

			g_ChatDump->Crash();

			// ���� ���� ��û
			//Disconnect(SessionID);
		}
	}

	// ��Ŷ �۽� �Ϸ� �� ȣ��Ǵ� �Լ�
	//
	// parameter : ����Ű, Send �� ������
	// return : ����
	void CChat_LanClient::OnSend(ULONGLONG ClinetID, DWORD SendSize)
	{

	}

	// ��Ŀ �����尡 ��� �� ȣ��Ǵ� �Լ�.
	// GQCS �ٷ� �ϴܿ��� ȣ��
	// 
	// parameter : ����
	// return : ����
	void CChat_LanClient::OnWorkerThreadBegin()
	{

	}

	// ��Ŀ �����尡 ���� �� ȣ��Ǵ� �Լ�
	// GQCS �ٷ� ������ ȣ��
	// 
	// parameter : ����
	// return : ����
	void CChat_LanClient::OnWorkerThreadEnd()
	{

	}

	// ���� �߻� �� ȣ��Ǵ� �Լ�.
	//
	// parameter : ���� �ڵ�(���� ������ �����ڵ�� WinGetLastError() �Լ��� ��� ����. ���� ��� 0�� ���ϵ�)
	//			 : ���� �ڵ忡 ���� ��Ʈ��
	// return : ����
	void CChat_LanClient::OnError(int error, const TCHAR* errorStr)
	{

	}



	// -------------------
	// �����ڿ� �Ҹ���
	// -------------------

	// ������
	CChat_LanClient::CChat_LanClient()
		:CLanClient()
	{
		m_ullSessionID = 0xffffffffffffffff;
		m_bLoginCheck = false;
	}

	// �Ҹ���
	CChat_LanClient::~CChat_LanClient()
	{

	}

}


// ---------------------------------------------
// 
// ����͸� LanClient
// 
// ---------------------------------------------
namespace Library_Jingyu
{

	// -----------------------
	// �����ڿ� �Ҹ���
	// -----------------------
	CChat_MonitorClient::CChat_MonitorClient()
		:CLanClient()
	{
		// ����͸� ���� �������� �����带 �����ų �̺�Ʈ ����
		//
		// �ڵ� ���� Event 
		// ���� ���� �� non-signalled ����
		// �̸� ���� Event	
		m_hMonitorThreadExitEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

		m_ullSessionID = 0xffffffffffffffff;
	}

	CChat_MonitorClient::~CChat_MonitorClient()
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
	bool CChat_MonitorClient::ClientStart(TCHAR* ConnectIP, int Port, int CreateWorker, int ActiveWorker, int Nodelay)
	{
		// ����͸� ������ ����
		if (Start(ConnectIP, Port, CreateWorker, ActiveWorker, Nodelay) == false)
		{
			// ����� ��Ŭ�� ���� ����
			g_ChatLog->LogSave(false, L"ChatServer", CSystemLog::en_LogLevel::LEVEL_ERROR, L"Monitor LanClient Start Error");

			return false;
		}

		// ����͸� ������ ���� ������ ������ ����
		m_hMonitorThread = (HANDLE)_beginthreadex(NULL, 0, MonitorThread, this, 0, NULL);

		// ����� ��Ŭ�� ���� �α�
		g_ChatLog->LogSave(true, L"ChatServer", CSystemLog::en_LogLevel::LEVEL_SYSTEM, L"Monitor LanClient Start...");

		return true;
	}

	// ���� �Լ�
	// ����������, ��ӹ��� CLanClient�� Stopȣ��.
	// �߰���, ���ҽ� ���� ��
	//
	// Parameter : ����
	// return : ����
	void CChat_MonitorClient::ClientStop()
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

	// ä�ü����� this�� �Է¹޴� �Լ�
	// 
	// Parameter : �� ������ this
	// return : ����
	void CChat_MonitorClient::SetNetServer(CChatServer_Room* ChatThis)
	{
		m_ChatServer_this = ChatThis;
	}




	// -----------------------
	// ���ο����� ����ϴ� ��� �Լ�
	// -----------------------

	// ���� �ð����� ����͸� ������ ������ �����ϴ� ������
	UINT	WINAPI CChat_MonitorClient::MonitorThread(LPVOID lParam)
	{
		// this �޾Ƶα�
		CChat_MonitorClient* g_This = (CChat_MonitorClient*)lParam;

		// ���� ��ȣ �̺�Ʈ �޾Ƶα�
		HANDLE hEvent = g_This->m_hMonitorThreadExitEvent;

		// CPU ����� üũ Ŭ���� (ä�ü��� ����Ʈ����)
		CCpuUsage_Process CProcessCPU;

		// PDH�� Ŭ����
		CPDH	CPdh;

		while (1)
		{
			// 1�ʿ� �ѹ� ����� ������ ������.
			DWORD Check = WaitForSingleObject(hEvent, 1000);

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
			// ����͸� ������ �������� ����!
			if (g_This->GetClinetState() == false)
				continue;

			// ���μ��� CPU �����, PDH ���� ����
			CProcessCPU.UpdateCpuTime();
			CPdh.SetInfo();

			// ä�ü����� On�� ���, ��Ŷ�� ������.
			if (g_This->m_ChatServer_this->GetServerState() == true)
			{
				// Ÿ�ӽ����� ���ϱ�
				int TimeStamp = (int)(time(NULL));

				// 1. ä�ü��� ON		
				g_This->InfoSend(dfMONITOR_DATA_TYPE_CHAT_SERVER_ON, TRUE, TimeStamp);

				// 2. ä�ü��� CPU ���� (Ŀ�� + ����)
				g_This->InfoSend(dfMONITOR_DATA_TYPE_CHAT_CPU, (int)CProcessCPU.ProcessTotal(), TimeStamp);

				// 3. ä�ü��� �޸� ���� Ŀ�� ��뷮 (Private) MByte
				int Data = (int)(CPdh.Get_UserCommit() / 1024 / 1024);
				g_This->InfoSend(dfMONITOR_DATA_TYPE_CHAT_MEMORY_COMMIT, Data, TimeStamp);

				// 4. ä�ü��� ��ŶǮ ��뷮
				g_This->InfoSend(dfMONITOR_DATA_TYPE_CHAT_PACKET_POOL, CProtocolBuff_Net::GetNodeCount() + CProtocolBuff_Lan::GetNodeCount(), TimeStamp);

				// 5. ä�ü��� ���� ������ü
				g_This->InfoSend(dfMONITOR_DATA_TYPE_CHAT_SESSION, (int)g_This->m_ChatServer_this->GetClientCount(), TimeStamp);

				// 6. ä�ü��� �α����� ������ ��ü �ο�				
				g_This->InfoSend(dfMONITOR_DATA_TYPE_CHAT_PLAYER, g_This->m_ChatServer_this->m_lChatLoginCount, TimeStamp);

				// 7. ��Ʋ���� �� ��				
				g_This->InfoSend(dfMONITOR_DATA_TYPE_CHAT_ROOM, g_This->m_ChatServer_this->m_lRoomCount, TimeStamp);
			}

		}

		return 0;
	}

	// ����͸� ������ ������ ����
	//
	// Parameter : DataType(BYTE), DataValue(int), TimeStamp(int)
	// return : ����
	void CChat_MonitorClient::InfoSend(BYTE DataType, int DataValue, int TimeStamp)
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
	void CChat_MonitorClient::OnConnect(ULONGLONG SessionID)
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
	void CChat_MonitorClient::OnDisconnect(ULONGLONG SessionID)
	{
		m_ullSessionID = 0xffffffffffffffff;
	}

	// ��Ŷ ���� �Ϸ� �� ȣ��Ǵ� �Լ�.
	//
	// parameter : ���� ����Ű, CProtocolBuff_Lan*
	// return : ����
	void CChat_MonitorClient::OnRecv(ULONGLONG SessionID, CProtocolBuff_Lan* Payload)
	{}

	// ��Ŷ �۽� �Ϸ� �� ȣ��Ǵ� �Լ�
	//
	// parameter : ���� ����Ű, Send �� ������
	// return : ����
	void CChat_MonitorClient::OnSend(ULONGLONG SessionID, DWORD SendSize)
	{}

	// ��Ŀ �����尡 ��� �� ȣ��Ǵ� �Լ�.
	// GQCS �ٷ� �ϴܿ��� ȣ��
	// 
	// parameter : ����
	// return : ����
	void CChat_MonitorClient::OnWorkerThreadBegin()
	{}

	// ��Ŀ �����尡 ���� �� ȣ��Ǵ� �Լ�
	// GQCS �ٷ� ������ ȣ��
	// 
	// parameter : ����
	// return : ����
	void CChat_MonitorClient::OnWorkerThreadEnd()
	{}

	// ���� �߻� �� ȣ��Ǵ� �Լ�.
	//
	// parameter : ���� �ڵ�(���� ������ �����ڵ�� WinGetLastError() �Լ��� ��� ����. ���� ��� 0�� ���ϵ�)
	//			 : ���� �ڵ忡 ���� ��Ʈ��
	// return : ����
	void CChat_MonitorClient::OnError(int error, const TCHAR* errorStr)
	{}
}