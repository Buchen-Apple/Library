#ifndef __MASTER_SERVER_H__
#define __MASTER_SERVER_H__

#include "NetworkLib/NetworkLib_LanServer.h"
#include "Log/Log.h"
#include "ObjectPool/Object_Pool_LockFreeVersion.h"
#include "Parser/Parser_Class.h"
#include "Protocol_Set/CommonProtocol_2.h"		// �ñ������δ� CommonProtocol.h�� �̸� ���� �ʿ�. ������ ä�ü������� �α��� ������ �̿��ϴ� ���������� �־ _2�� ����.

#include <list>
#include <unordered_map>
#include <unordered_set>
#include <queue>


using namespace std;


// -----------------------
//
// ������ Lan����
// �ܺο����� �ش� Ŭ������ �����ϸ�, �ش� Ŭ������ �� Lan������ ����.
// 
// -----------------------
namespace Library_Jingyu
{
	class CMasterServer
	{

		friend class CMatchServer_Lan;
		friend class CBattleServer_Lan;

	private:
		// -----------------
		// �̳� ����ü
		// -----------------	

		// CRoom ���漱��
		struct CRoom;

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
		
		// ��Ʋ���� ����ü
		struct CBattleServer
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

		// ��Ī���� ����ü
		struct CMatchingServer
		{
			ULONGLONG m_ullSessionID;
			int m_iServerNo;

			// �α��� ����. true�� �α��� ��
			bool m_bLoginCheck;

			CMatchingServer()
			{
				m_bLoginCheck = false;
			}
		};

		// Room_Priority_Queue���� ����� �񱳿�����
		struct RoomCMP
		{
			// m_iEmptyCount�� ���� ������ pop
			bool operator()(CRoom* t, CRoom* u)
			{
				return t->m_iEmptyCount > u->m_iEmptyCount;
			}
		};

		// Room ����ü
		struct CRoom
		{
			int m_iRoomNo;

			// �濡, ���� ������ ������.
			// ex) ������ 3/5���¶��, �ش� ���� 2��.
			int m_iEmptyCount;

			// ���� �����ϴ� ��Ʋ ������ Key
			int m_iBattleServerNo;

			// ��Ʋ���� �� ���� ��ū(��Ʋ���� ����)
			char	m_cEnterToken[32];

			// �ش� ���� ��
			SRWLOCK m_srwl_Room;

			CRoom()
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

	private:
		// -----------------
		// Battle ���� ������ �ڷᱸ��
		// -----------------	

		// ������ ��Ʋ ���� ������ �ڷᱸ��
		// uamp ���
		//
		// Key : SessionID, Value : CBattleServer*
		unordered_map<ULONGLONG, CMasterServer::CBattleServer*> m_BattleServer_Umap;

		// m_BattleServer_Umap������ SRWLOCK
		SRWLOCK m_srwl_BattleServer_Umap;

		// CBattleServer�� �����ϴ� TLSPool
		CMemoryPoolTLS<CMasterServer::CBattleServer> *m_TLSPool_BattleServer;
	

	private:
		// -----------------
		// Player ������ �ڷᱸ��
		// -----------------			   

		// ClientKey�� �̿���, �÷��̾ �����ϴ� �ڷᱸ��
		// uset ���
		//
		// Key : ClientKey, Value : CPlayer*
		unordered_set<UINT64> m_ClientKey_Uset;

		// m_ClientKey_Umap������ SRWLOCK
		SRWLOCK m_srwl_ClientKey_Umap;

		// -----------------------

		// AccountNo�� �̿���, �÷��̾ �����ϴ� �ڷᱸ��
		// uset ���
		//
		// Key : AccountNo
		unordered_set<UINT64>m_AccountNo_Uset;

		// m_AccountNo_Umap������ SRWLOCK
		SRWLOCK m_srwl_m_AccountNo_Umap;

		// -----------------------

	private:
		// -----------------
		// Room ������ �ڷᱸ��
		// -----------------

		// RoomNo�� �̿���, ���� �����ϴ� �ڷᱸ��
		// umap ���
		//
		// Key : RoomNo, Value : CPlayer*
		unordered_map<int, CRoom*> m_Room_Umap;

		// m_Room_Umap������ SRWLOCK
		SRWLOCK m_srwl_Room_Umap;

		// -----------------------			

		// ���� �����ϴ� �ڷᱸ��2
		// Priority_Queue ���
		//
		//  <�ڷ���, ����ü, �񱳿�����> ������ ����
		// �ڷ��� : CRoom*, ����ü : Vector<CRoom*>. �� ������ : ���� ����(��������)
		priority_queue <CRoom*, vector<CRoom*>, RoomCMP> m_Room_pq;

		// m_Room_pq������ SRWLOCK
		SRWLOCK m_srwl_Room_pq;

		// -----------------------

		// CRoom�� �����ϴ� TLSPool
		CMemoryPoolTLS<CRoom> *m_TLSPool_Room;


	private:
		// -----------------
		// ��� ����
		// -----------------

		// ���� �о���� �� ����ü
		stConfigFile m_stConfig;

		// ----------------------------

		// !! ��ġ����ŷ�� Lan���� !!
		CMatchServer_Lan* m_pMatchServer;

		// !! ��Ʋ�� Lan ���� !!
		CBattleServer_Lan* m_pBattleServer;



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


	public:
		// -------------------------------------
		// �ܺο��� ��� ������ �Լ�
		// -------------------------------------

		// ������ ���� ����
		// ���� Lan������ �����Ѵ�.
		// 
		// Parameter : ����
		// return : ���� �� false ����
		bool ServerStart();

		// ������ ���� ����
		// ���� Lan������ ����
		// 
		// Parameter : ����
		// return : ����
		void ServerStop();


	private:
		// -------------------------------------
		// Player ������ �ڷᱸ�� �Լ�
		// -------------------------------------

		// Player ���� �ڷᱸ�� "2��"�� Isnert
		//
		// Parameter : ClinetKey, AccountNo
		// return : �ߺ��� �� false.
		bool InsertPlayerFunc(UINT64 ClinetKey, UINT64 AccountNo);


		// ClinetKey ������ �ڷᱸ������ Erase
		// umap���� ������
		//
		// Parameter : ClinetKey
		// return : ���� ������ �� false
		//		  : �ִ� ������ �� true
		bool ErasePlayerFunc(UINT64 ClinetKey);
		


	public:
		// -------------------------------------
		// �����ڿ� �Ҹ���
		// -------------------------------------

		// ������
		CMasterServer();

		// �Ҹ���
		virtual ~CMasterServer();

	};
}



// -----------------------
//
// ��ġ����ŷ�� ����Ǵ� Lan ����
// 
// -----------------------
namespace Library_Jingyu
{
	class CMatchServer_Lan :public CLanServer
	{
		friend class CMasterServer;			


	private:
		// -----------------
		// Match ���� ������ �ڷᱸ��
		// -----------------	

		// ������ ��ġ����ŷ ���� ������ �ڷᱸ��
		// uamp ���
		//
		// Key : SessionID, Value : CMatchingServer*
		unordered_map<ULONGLONG, CMasterServer::CMatchingServer*> m_MatchServer_Umap;

		// m_MatchServer_Umap������ SRWLOCK
		SRWLOCK m_srwl_MatchServer_Umap;

		// CMatchingServer�� �����ϴ� TLSPool
		CMemoryPoolTLS<CMasterServer::CMatchingServer> *m_TLSPool_MatchServer;
		
		// ------------------------
		
		// Match ���� ��, �α��� ��Ŷ���� ���� ���� ������ �ڷᱸ��.
		// ���� ���� �Ǵܿ뵵.
		// uset ���
		//
		// Key : ServerNo (��Ī���� No)
		unordered_set<int> m_LoginMatServer_Uset;

		// m_LoginMatServer_Uset������ SRWLOCK
		SRWLOCK m_srwl_LoginMatServer_Umap;



	private:
		// -----------------
		// ��� ����
		// -----------------

		// !! �θ� ����?�� ������ ���� !!
		CMasterServer* pMasterServer;
			   
	private:
		// -----------------
		// umap�� �ڷᱸ�� ���� �Լ�
		// -----------------

		// ������ ���� �ڷᱸ���� Insert
		// umap���� ������
		//
		// Parameter : SessionID, CMatchingServer*
		// return : ���� �� false ����
		bool InsertPlayerFunc(ULONGLONG SessionID, CMasterServer::CMatchingServer* insertServer);

		// ������ ���� �ڷᱸ������, ���� �˻�
		// ���� umap���� ������
		// 
		// Parameter : SessionID
		// return : �˻� ���� ��, CMatchingServer*
		//		  : �˻� ���� �� nullptr
		CMasterServer::CMatchingServer* FindPlayerFunc(ULONGLONG SessionID);
			   
		// ������ ���� �ڷᱸ������ Erase
		// umap���� ������
		//
		// Parameter : SessionID
		// return : ���� ��, ���ŵ� CMatchingServer*
		//		  : �˻� ���� ��(���������� ���� ����) nullptr
		CMasterServer::CMatchingServer* ErasePlayerFunc(ULONGLONG SessionID);


	private:
		// -----------------
		// uset�� �ڷᱸ�� ���� �Լ�
		// -----------------

		// �α��� �� ���� ���� �ڷᱸ���� Insert
		// uset���� ������
		//
		// Parameter : ServerNo
		// return : ���� �� false ����
		bool InsertLoginPlayerFunc(int ServerNo);

		// �α��� �� ���� ���� �ڷᱸ������ Erase
		// uset���� ������
		//
		// Parameter : ServerNo
		// return : ���� �� false ����
		bool EraseLoginPlayerFunc(int ServerNo);




	private:
		// -----------------
		// ���ο����� ����ϴ� �Լ�
		// -----------------

		// ������ ������ ���� ����
		//
		// Parameter : CMasterServer* 
		// return : ����
		void SetParent(CMasterServer* Parent);


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
		//
		// Parameter : SessionID, Payload
		// return : ����
		void Packet_RoomInfo(ULONGLONG SessionID, CProtocolBuff_Lan* Payload);
		
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


	private:
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
// ��Ʋ�� ����Ǵ� Lan ����
// 
// -----------------------
namespace Library_Jingyu
{
	class CBattleServer_Lan :public CLanServer
	{
		friend class CMasterServer;

	private:
		// -----------------
		// ��� ����
		// -----------------

		// !! �θ� ����?�� ������ ���� !!
		CMasterServer* pMasterServer; 

	private:
		// -----------------
		// ���ο����� ����ϴ� �Լ�
		// -----------------

		// ������ ������ ���� ����
		//
		// Parameter : CMasterServer* 
		// return : ����
		void SetParent(CMasterServer* Parent);


	public:
		// -------------------------------------
		// �ܺο��� ��� ������ �Լ�
		// -------------------------------------

		// ���� ����
		// ���� Lan������ �����Ѵ�.
		// 
		// Parameter : ����
		// return : ���� �� false ����
		bool ServerStart();

		// ���� ����
		// ���� Lan������ ����
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
		// -------------------------
		// �����ڿ� �Ҹ���
		// -------------------------

		// ������
		CBattleServer_Lan();

		// �Ҹ���
		virtual ~CBattleServer_Lan();

	};
}

#endif // ! __MASTER_SERVER_H__
