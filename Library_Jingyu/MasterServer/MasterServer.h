#ifndef __MASTER_SERVER_H__
#define __MASTER_SERVER_H__

#include "NetworkLib/NetworkLib_LanServer.h"
#include "ObjectPool/Object_Pool_LockFreeVersion.h"
#include "Parser/Parser_Class.h"
#include "Protocol_Set/CommonProtocol_2.h"		// �ñ������δ� CommonProtocol.h�� �̸� ���� �ʿ�. ������ ä�ü������� �α��� ������ �̿��ϴ� ���������� �־ _2�� ����.

#include <unordered_map>
#include <unordered_set>
#include <queue>

// -----------------------
//
// ������ Match ����
// 
// -----------------------
namespace Library_Jingyu
{
	class CMatchServer_Lan :public CLanServer
	{
		friend class CBattleServer_Lan;


		// -------------------
		// �̳� ����ü
		// -------------------

		// ���Ͽ��� �о���� �� ����ü
		struct stConfigFile
		{
			// ��ġ����ŷ�� Lan ���� ����
			TCHAR BindIP[20];
			int Port;
			int CreateWorker;
			int ActiveWorker;
			int CreateAccept;
			int Nodelay;
			int MaxJoinUser;
			int LogLevel;
			char EnterToken[32];


			// ��Ʋ�� Lan ���� ����
			TCHAR BattleBindIP[20];
			int BattlePort;
			int BattleCreateWorker;
			int BattleActiveWorker;
			int BattleCreateAccept;
			int BattleNodelay;
			int BattleMaxJoinUser;
			int BattleLogLevel;
			char BattleEnterToken[32];
		};

		// ��Ī���� ����ü
		struct stMatching
		{
			ULONGLONG m_ullSessionID;
			int m_iServerNo;

			// �α��� ����. true�� �α��� ��
			bool m_bLoginCheck;

			stMatching()
			{
				m_bLoginCheck = false;
			}
		};
		


	private:
		// -----------------
		// ��� ����
		// -----------------

		// !! Battle ���� !!
		CBattleServer_Lan* pBattleServer;

		// Config�� ����
		stConfigFile m_stConfig;



	private:
		// -----------------
		// �÷��̾� ������ �ڷᱸ��
		// -----------------	

		// ��Ī �� �������� ���� ���� Player �ڷᱸ��
		// umap ���
		// ClientKey �̿�
		//
		// Key : ClientKey, Value : AccountNo
		unordered_map<UINT64, UINT64> m_Player_ClientKey_Umap;

		// m_Player_ClientKey_Umap������ SRWLOCK
		SRWLOCK m_srwl_Player_ClientKey_Umap;



	private:
		// -----------------
		// Match ���� ������ �ڷᱸ��
		// -----------------	

		// ������ ��ġ����ŷ ���� ������ �ڷᱸ��
		// uamp ���
		//
		// Key : SessionID, Value : stMatching*
		unordered_map<ULONGLONG, stMatching*> m_MatchServer_Umap;

		// m_MatchServer_Umap������ SRWLOCK
		SRWLOCK m_srwl_MatchServer_Umap;	

		// stMatching�� �����ϴ� TLSPool
		CMemoryPoolTLS<stMatching> *m_TLSPool_MatchServer;

		// ------------------------

		// Match ���� ��, �α��� ��Ŷ���� ���� ���� ������ �ڷᱸ��.
		// ���� ���� �Ǵܿ뵵.
		// uset ���
		//
		// Key : ServerNo (��Ī���� No)
		unordered_set<int> m_LoginMatServer_Uset;

		// m_LoginMatServer_Uset������ SRWLOCK
		SRWLOCK m_srwl_LoginMatServer_Uset;

			  

	private:
		// -------------------------------------
		// ���ο����� ����ϴ� �Լ�
		// -------------------------------------

		// ���Ͽ��� Config ���� �о����
		// 
		// Parameter : config ����ü
		// return : ���������� ���� �� true
		//		  : �� �ܿ��� false
		bool SetFile(stConfigFile* pConfig);



	private:
		// -----------------
		// �÷��̾� ���� �ڷᱸ�� �Լ�
		// -----------------

		// ��Ī���� �����Ǵ� �÷��̾� �ڷᱸ���� insert
		//
		// Parameter : ClinetKey, AccountNo
		// return : ���������� ���� ��, true
		//		  : ���� �� false;	
		bool InsertPlayerFunc(UINT64 ClientKey, UINT64 AccountNo);




	private:
		// -----------------
		// ������ ��Ī ���� ������ �ڷᱸ�� �Լ� (umap)
		// -----------------

		// ������ ���� �ڷᱸ���� Insert
		// umap���� ������
		//
		// Parameter : SessionID, stMatching*
		// return : ���� �� false ����
		bool InsertMatchServerFunc(ULONGLONG SessionID, stMatching* insertServer);
		
		// ������ ���� �ڷᱸ������, ���� �˻�
		// ���� umap���� ������
		// 
		// Parameter : SessionID
		// return : �˻� ���� ��, stMatchingr*
		//		  : �˻� ���� �� nullptr
		stMatching* FindMatchServerFunc(ULONGLONG SessionID);
		
		// ������ ���� �ڷᱸ������ Erase
		// umap���� ������
		//
		// Parameter : SessionID
		// return : ���� ��, ���ŵ� stMatching*
		//		  : �˻� ���� ��(���������� ���� ����) nullptr
		stMatching* EraseMatchServerFunc(ULONGLONG SessionID);



	
	private:
		// -----------------
		// ������ ��Ī ���� ������ �ڷᱸ�� �Լ�(�α��� �� ���� ����)
		// -----------------

		// �α��� �� ���� ���� �ڷᱸ���� Insert
		// uset���� ������
		//
		// Parameter : ServerNo
		// return : ���� �� false ����
		bool InsertLoginMatchServerFunc(int ServerNo);

		// �α��� �� ���� ���� �ڷᱸ������ Erase
		// uset���� ������
		//
		// Parameter : ServerNo
		// return : ���� �� false ����
		bool EraseLoginMatchServerFunc(int ServerNo);
			   		 	  


	private:
		// -----------------
		// ��Ŷ ó���� �Լ�
		// -----------------

		// Login��Ŷ ó��
		//
		// Parameter : SessionID, Payload
		// return : ����
		void Packet_Login(ULONGLONG SessionID, CProtocolBuff_Lan* Payload);

		// �� ���� ��û
		// ���� ���� ��, Battle�� ����
		//
		// Parameter : SessionID, Payload
		// return : ����
		void Relay_RoomInfo(ULONGLONG SessionID, CProtocolBuff_Lan* Payload);

		// �� ���� ����
		//
		// Parameter : SessionID, Payload
		// return : ����
		void Packet_RoomEnter_OK(ULONGLONG SessionID, CProtocolBuff_Lan* Payload);




	public:
		// -----------------------
		// �ܺο��� ��� ������ �Լ�
		// -----------------------

		// ���� ����
		//
		// Parameter : ����
		// return : ���� �� false.
		bool ServerStart();

		// ���� ����
		//
		// Parameter : ����
		// return : ����
		void ServerStop();





	private:
		// -----------------------
		// �����Լ�
		// -----------------------

		bool OnConnectionRequest(TCHAR* IP, USHORT port);

		void OnClientJoin(ULONGLONG SessionID);

		void OnClientLeave(ULONGLONG SessionID);

		void OnRecv(ULONGLONG SessionID, CProtocolBuff_Lan* Payload);

		void OnSend(ULONGLONG SessionID, DWORD SendSize);

		void OnWorkerThreadBegin();

		void OnWorkerThreadEnd();

		void OnError(int error, const TCHAR* errorStr);


	public:
		// -----------------------
		// �����ڿ� �Ҹ���
		// -----------------------

		// ������
		CMatchServer_Lan();

		// �Ҹ���
		virtual ~CMatchServer_Lan();


	};
}


// -----------------------
//
// ������ Battle ����
// 
// -----------------------
namespace Library_Jingyu
{
	class CBattleServer_Lan :public CLanServer
	{
		friend class CMatchServer_Lan;

		// --------------
		// �̳� ����ü
		// --------------

		// CRoom ���漱��
		struct stRoom;

		// Room_Priority_Queue���� ����� �񱳿�����
		struct RoomCMP
		{
			// m_iEmptyCount�� ���� ������ pop
			bool operator()(stRoom* t, stRoom* u)
			{
				return t->m_iEmptyCount > u->m_iEmptyCount;
			}
		};

		// ��Ʋ���� ����ü
		struct stBattle
		{
			ULONGLONG m_ullSessionID;

			// ��Ʋ���� ������ȣ
			int m_iServerNo;

			// ��Ʋ���� ���� IP
			WCHAR	m_tcBattleIP[16];

			// ��Ʋ���� ���� Port
			WORD	m_wBattlePort;

			// ��Ʋ���� ���� ��ū(��Ʋ���� ����)
			char	m_cConnectToken[32];

			// �ش� ��Ʋ������ ����� ä�ü���
			WCHAR	m_tcChatIP[16];
			WORD	m_wChatPort;
		};
		
		// Room ����ü
		struct stRoom
		{
			int m_iRoomNo;

			// �濡, ���� ������ ������.
			// ex) ������ 3/5���¶��, �ش� ���� 2��.
			int m_iEmptyCount;

			// ���� �����ϴ� ��Ʋ ������ No
			int m_iBattleServerNo;

			// ��Ʋ���� �� ���� ��ū(��Ʋ���� ����)
			char	m_cEnterToken[32];

			// �ش� ���� ��
			SRWLOCK m_srwl_Room;

			stRoom()
			{
				InitializeSRWLock(&m_srwl_Room);
			}

			// �ش� ���� ��
			void RoomLOCK()
			{
				AcquireSRWLockExclusive(&m_srwl_Room);
			}

			// �ش� ���� ���
			void RoomUNLOCK()
			{
				ReleaseSRWLockExclusive(&m_srwl_Room);
			}
		};

		// �÷��̾� ����ü
		struct stPlayer
		{
			UINT64 m_ui64AccountNo;

			// �ش� ������ ������ ���� �����ϴ� ��Ʋ ������ No
			int m_iBattleServerNo;

			// ���� ���� �� ��ȣ.
			int m_iJoinRoomNo;
		};


	private:
		// --------------
		// ��� ����
		// --------------

		// !! Matching ���� !!
		CMatchServer_Lan* pMatchServer;
		

	private:
		// -----------------
		// �÷��̾� ������ �ڷᱸ��
		// -----------------
		
		// ��Ʋ �� �������� ���� ���� Player �ڷᱸ��
		// umap ���
		// AccountNo �̿�
		//
		// Key : AccountNo, Value : stPlayer*
		unordered_map<UINT64, stPlayer*> m_Player_AccountNo_Umap;

		// m_Player_AccountNo_Umap������ SRWLOCK
		SRWLOCK m_srwl_Player_AccountNo_Umap;

		// stPlayer�� �����ϴ� TLSPool
		CMemoryPoolTLS<stPlayer> *m_TLSPool_Player;



	private:
		// -----------------
		// ������ Battle ���� ������ �ڷᱸ��
		// -----------------	

		// ������ ��Ʋ ���� ������ �ڷᱸ��
		// uamp ���
		//
		// Key : SessionID, Value : stBattle*
		unordered_map<ULONGLONG, stBattle*> m_BattleServer_Umap;

		// m_BattleServer_Umap������ SRWLOCK
		SRWLOCK m_srwl_BattleServer_Umap;

		// stBattle�� �����ϴ� TLSPool
		CMemoryPoolTLS<stBattle> *m_TLSPool_BattleServer;




	private:
		// -----------------
		// Room ������ �ڷᱸ��
		// -----------------

		// RoomNo�� �̿���, ���� �����ϴ� �ڷᱸ��
		// umap ���
		//
		// Key : RoomNo, Value : CPlayer*
		unordered_map<int, stRoom*> m_Room_Umap;

		// m_Room_Umap������ SRWLOCK
		SRWLOCK m_srwl_Room_Umap;

		// -----------------------			

		// ���� �����ϴ� �ڷᱸ��2
		// Priority_Queue ���
		//
		//  <�ڷ���, ����ü, �񱳿�����> ������ ����
		// �ڷ��� : CRoom*, ����ü : Vector<CRoom*>. �� ������ : ���� ����(��������)
		priority_queue <stRoom*, vector<stRoom*>, RoomCMP> m_Room_pq;

		// m_Room_pq������ SRWLOCK
		SRWLOCK m_srwl_Room_pq;

		// -----------------------

		// stRoom�� �����ϴ� TLSPool
		CMemoryPoolTLS<stRoom> *m_TLSPool_Room;



	private:
		// -----------------
		// �÷��̾� ������ �ڷᱸ�� �Լ�
		// -----------------

		// �÷��̾� ���� �ڷᱸ���� �÷��̾� Insert
		//
		// Parameter : AccountNo, stPlayer*
		// return : ���������� Insert �� true
		//		  : ���� �� false
		bool InsertPlayerFunc(UINT64 AccountNo, stPlayer* InsertPlayer);

		// ���ڷ� ���� AccountNo�� �̿��� ���� �˻�.
		// 1. RoomNo, BattleServerNo�� ���� ��ġ�ϴ��� Ȯ��
		// 2. �ڷᱸ������ Erase
		// 3. stPlayer*�� �޸�Ǯ�� Free
		//
		// !! ��Ī �� �������� ȣ�� !!
		//
		// Parameter : AccountNo, RoomNo, BattleServerNo
		//
		// return Code
		// 0 : ���������� ������
		// 1 : �������� �ʴ� ���� (AccountNo�� ���� �˻� ����
		// 2 : RoomNo ����ġ
		// 3 : BattleServerNo ����ġ
		int ErasePlayerFunc(UINT64 AccountNo, int RoomNo, int BattleServerNo);



	public:
		// -----------------------
		// �ܺο��� ��� ������ �Լ�
		// -----------------------

		// ���� ����
		//
		// Parameter : ����
		// return : ���� �� false.
		bool ServerStart();

		// ���� ����
		//
		// Parameter : ����
		// return : ����
		void ServerStop();





	private:
		// -------------------------------
		// ��Ŷ ó�� �Լ�
		// -------------------------------

		// �� ���� ��û ��Ŷ ó��
		// !! ��Ī �� �������� ȣ�� !!
		//
		// Parameter : SessionID(��Ī ���� SessionID), ClientKey, AccountNo
		// return : ����
		void Relay_Battle_Room_Info(ULONGLONG SessionID, UINT64 ClientKey, UINT64 AccountNo);
			
		


	private:
		// -----------------------
		// �����Լ�
		// -----------------------

		bool OnConnectionRequest(TCHAR* IP, USHORT port);

		void OnClientJoin(ULONGLONG SessionID);

		void OnClientLeave(ULONGLONG SessionID);

		void OnRecv(ULONGLONG SessionID, CProtocolBuff_Lan* Payload);

		void OnSend(ULONGLONG SessionID, DWORD SendSize);

		void OnWorkerThreadBegin();

		void OnWorkerThreadEnd();

		void OnError(int error, const TCHAR* errorStr);


	public:
		// -----------------------
		// �����ڿ� �Ҹ���
		// -----------------------

		// ������
		CBattleServer_Lan();

		// �Ҹ���
		virtual ~CBattleServer_Lan();
	};
}


#endif // !__MASTER_SERVER_H
