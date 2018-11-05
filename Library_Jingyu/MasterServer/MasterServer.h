#ifndef __MASTER_SERVER_H__
#define __MASTER_SERVER_H__

#include "NetworkLib/NetworkLib_LanServer.h"
#include "Log/Log.h"
#include "ObjectPool/Object_Pool_LockFreeVersion.h"
#include "Parser/Parser_Class.h"
#include "Protocol_Set/CommonProtocol_2.h"		// �ñ������δ� CommonProtocol.h�� �̸� ���� �ʿ�. ������ ä�ü������� �α��� ������ �̿��ϴ� ���������� �־ _2�� ����.

#include <list>
#include <unordered_map>
#include <queue>


using namespace std;

// -----------------------
//
// ������ Lan ����
// 
// -----------------------
namespace Library_Jingyu
{
	class CMasterServer_Lan :public CLanServer
	{

	private:
		// -----------------
		// �̳� Ŭ����
		// -----------------	

		// CRoom ���漱��
		class CRoom;

		// ���Ͽ��� �о���� �� ����ü
		struct stConfigFile
		{
			// ������ Lan ���� ����
			TCHAR BindIP[20];
			int Port;
			int CreateWorker;
			int ActiveWorker;
			int CreateAccept;
			int Nodelay;
			int MaxJoinUser;
			int LogLevel;
		};		

		// ��Ʋ���� Ŭ����
		class CBattleServer
		{
			ULONGLONG m_ullSessionID;
			int m_iServerNo;
		};

		// ��Ī���� Ŭ����
		class CMatchingServer
		{
			ULONGLONG m_ullSessionID;
			int m_iServerNo;
		};			

		// Room_Priority_Queue���� �񱳿����ڷ� ����� Ŭ����
		class RoomCMP
		{
			// m_iEmptyCount�� ���� ������ pop
			bool operator()(CRoom* t, CRoom* u)
			{
				return t->m_iEmptyCount > u->m_iEmptyCount;
			}
		};

		// ���� Ŭ����
		class CPlayer
		{
			UINT64	m_ui64ClinetKey;
			UINT64	m_ui64AccountNo;

			// ������ �����ϰ� �ִ� Room
			CRoom* pJoinRoom = nullptr;
		};

		// Room Ŭ����
		class CRoom
		{
			friend class RoomCMP;

			int m_iRoomNo;

			// �濡, ���� ������ ������.
			// ex) ������ 3/5���¶��, �ش� ���� 2��.
			int m_iEmptyCount;

			// ���� �����ϴ� ��Ʋ ����
			CBattleServer* pBattle = nullptr;

			// �뿡 Join�� ������ �����ϴ� list
			list<CPlayer*> m_JoinUser_List;
		};			


	private:
		// -----------------
		// Match ���� ������ �ڷᱸ��
		// -----------------	

		// ������ ��ġ����ŷ ���� ������ �ڷᱸ��
		// uamp ���
		//
		// Key : SessionID, Value : CMatchingServer*
		unordered_map<ULONGLONG, CMatchingServer*> m_MatchServer_Umap;

		// m_MatchServer_Umap������ SRWLOCK
		SRWLOCK m_srwl_MatchServer_Umap;

		// CMatchingServer�� �����ϴ� TLSPool
		CMemoryPoolTLS<CMatchingServer> *m_TLSPool_MatchServer;



	private:
		// -----------------
		// Battle ���� ������ �ڷᱸ��
		// -----------------	

		// ������ ��Ʋ ���� ������ �ڷᱸ��
		// uamp ���
		//
		// Key : SessionID, Value : CBattleServer*
		unordered_map<ULONGLONG, CBattleServer*> m_BattleServer_Umap;

		// m_BattleServer_Umap������ SRWLOCK
		SRWLOCK m_srwl_BattleServer_Umap;

		// CBattleServer�� �����ϴ� TLSPool
		CMemoryPoolTLS<CBattleServer> *m_TLSPool_BattleServer;



	private:
		// -----------------
		// Player ������ �ڷᱸ��
		// -----------------			   

		// ClientKey�� �̿���, �÷��̾ �����ϴ� �ڷᱸ��
		// umap ���
		//
		// Key : ClientKey, Value : CPlayer*
		unordered_map<UINT64, CPlayer*> m_ClientKey_Umap;

		// m_ClientKey_Umap������ SRWLOCK
		SRWLOCK m_srwl_ClientKey_Umap;	

		// -----------------------

		// AccountNo�� �̿���, �÷��̾ �����ϴ� �ڷᱸ��
		// umap ���
		//
		// Key : AccountNo, Value : CPlayer*
		unordered_map<UINT64, CPlayer*> m_AccountNo_Umap;

		// m_AccountNo_Umap������ SRWLOCK
		SRWLOCK m_srwl_AccountNo_Umap;

		// -----------------------

		// CPlayer�� �����ϴ� TLSPool
		CMemoryPoolTLS<CPlayer> *m_TLSPool_Player;



	private:
		// -----------------
		// Room ������ �ڷᱸ��
		// -----------------

		// RoomNo�� �̿���, ���� �����ϴ� �ڷᱸ��
		// umap ���
		// !! ���� ���� ���� 0�� �Ǿ�, ������ ������ Room�� �����Ѵ� !!
		//
		// Key : RoomNo, Value : CPlayer*
		unordered_map<int, CRoom*> m_Room_Umap;

		// m_Room_Umap������ SRWLOCK
		SRWLOCK m_srwl_Room_Umap;

		// -----------------------			

		// ���� �����ϴ� �ڷᱸ��2
		// Priority_Queue ���
		// �� ���� ��û ��, ���� ���� ���� 0�� �ƴ� ��� ���� ����.
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

		// ������
		CCrashDump* m_CDump;

		// �α� �����
		CSystemLog* m_CLog;

		// ���� �о���� �� ����ü
		stConfigFile m_stConfig;


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
		CMasterServer_Lan();

		// �Ҹ���
		virtual ~CMasterServer_Lan();

	};
}


#endif // ! __MASTER_SERVER_H__
