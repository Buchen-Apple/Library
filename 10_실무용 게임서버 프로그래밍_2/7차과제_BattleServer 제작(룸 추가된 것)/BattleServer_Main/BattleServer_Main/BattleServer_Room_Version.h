#ifndef __BATTLESERVER_ROOM_VERSION_H__
#define __BATTLESERVER_ROOM_VERSION_H__

#include "NetworkLib/NetworkLib_MMOServer.h"
#include "NetworkLib/NetworkLib_LanClinet.h"
#include "NetworkLib/NetworkLib_LanServer.h"

#include "Http_Exchange/HTTP_Exchange.h"

#include <unordered_set>
#include <unordered_map>
#include <forward_list>

using namespace std;

// --------------------------------------------
// shDB ���ο��� �޸�Ǯ�� �����Ǵ� ����ü��
// --------------------------------------------
namespace Library_Jingyu
{
	// Auth �����忡�� �α��� ������ ���� ��, ����Ű üũ�� ���� ��������
	struct DB_WORK_LOGIN
	{
		WORD m_wWorkType;
		INT64 m_i64UniqueKey;

		// Player ������
		LPVOID pPointer;
		TCHAR m_tcResponse[420];

		INT64 AccountNo;
	};

	// Auth �����忡�� �α��� ��û�� ���� ��, ���� ���� ������ �����ϱ� ���� ��������.
	struct DB_WORK_LOGIN_CONTENTS
	{
		WORD m_wWorkType;
		INT64 m_i64UniqueKey;

		// Player ������
		LPVOID pPointer;
		TCHAR m_tcResponse[200];

		INT64 AccountNo;
	};

	// ContentDB�� ���� ������ Write�� �� ���
	struct DB_WORK_CONTENT_UPDATE
	{
		WORD m_wWorkType;

		// Update�� res�� ������ ª��. ���� result : 1��
		TCHAR m_tcResponse[30];

		int		m_iCount;	// ������ ī��Ʈ		

		INT64 AccountNo;
	};

	// Content DB�� ���� ���� 2���� Write�� �� ���
	struct DB_WORK_CONTENT_UPDATE_2
	{
		WORD m_wWorkType;

		// Update�� res�� ������ ª��. ���� result : 1��
		TCHAR m_tcResponse[30];

		int		m_iCount1;	// ������ ī��Ʈ1
		int		m_iCount2;	// ������ ī��Ʈ2

		INT64 AccountNo;
	};

	// shDB�� � API�� ȣ���� ������. API Type
	enum en_PHP_TYPE
	{
		// Seelct_account.php
		SELECT_ACCOUNT = 1,

		// Seelct_contents.php
		SELECT_CONTENTS,

		// Update_account.php
		UPDATE_ACCOUN,

		// Updated_contents.php
		UPDATE_CONTENTS,

		// DB_Read ������ ���� ��ȣ
		EXIT = 9999
	};

	// DB ��û�� ���� ��ó���� ����, Type
	enum eu_DB_AFTER_TYPE
	{
		// �α��� ��Ŷ�� ���� ���� ó��
		eu_LOGIN_AUTH = 0,

		// �α��� ��Ŷ�� ���� ���� ��������
		eu_LOGIN_INFO,

		// PlayCount ����
		eu_PLAYCOUNT_UPDATE,

		// Kill ����
		eu_KILL_UPDATE,

		// Die, �÷���Ÿ�� ����
		eu_DIE_UPDATE,

		// Win, �÷���Ÿ�� ����
		eu_WIN_UPDATE
	};
}

// -----------------------
// shDB�� ����ϴ� Ŭ����
// -----------------------
namespace Library_Jingyu
{
	// �޸�Ǯ���� ������ ����ü.
	// ���� ũ�Ⱑ ū �� 1���� �����Ѵ�.
#define DB_WORK	DB_WORK_LOGIN

	class shDB_Communicate
	{
		friend class CBattleServer_Room;

		// ---------------
		// ��� ����
		// ---------------		

		// ����
		CCrashDump* m_Dump;

		// DB_Read�� ����� �Ϸ���Ʈ �ڵ�
		HANDLE m_hDB_Read;

		// DB_Read�� ��Ŀ ������ �ڵ�
		HANDLE* m_hDB_Read_Thread;

		// ��Ʋ���� this
		CBattleServer_Room* m_pBattleServer;

		// ---------------		

		// DB Write �����忡�� �Ͻ�Ű�� �� ť(Normal ť)
		CNormalQueue<DB_WORK*>* m_pDB_Wirte_Start_Queue;

		// DB Write�� ������� �Ͻ�Ű�� ���̺�Ʈ.
		HANDLE m_hDBWrite_Event;

		// DB Write�� ������ ����� �̺�Ʈ
		HANDLE m_hDBWrite_Exit_Event;

		// DB_Write�� ������ �ڵ�
		HANDLE m_hDB_Write_Thread;


	public:
		// ---------------
		// �ܺο��� ���� ������ ��� ����
		// ---------------		

		// DB_WORK�� �����ϴ� �޸� Ǯ
		CMemoryPoolTLS<DB_WORK>* m_pDB_Work_Pool;

		// Read�� �Ϸ�� DB_WORK* �� �����صδ� ������ ť
		CLF_Queue<DB_WORK*> *m_pDB_ReadComplete_Queue;

		// DBWrite�� TPS
		LONG m_lDBWriteTPS;

		// �ʴ� DBWrite ť�� ������ �������� ��
		LONG m_lDBWriteCountTPS;


	private:
		// --------------
		// ������
		// --------------

		// DB_Read ������
		static UINT WINAPI DB_ReadThread(LPVOID lParam);

		// DB_Write ������
		static UINT WINAPI DB_WriteThread(LPVOID lParam);


	public:
		// ---------------
		// ��� �Լ�. �ܺο��� ȣ�� ����
		// ---------------

		// DB�� Read�� ���� ���� �� ȣ��Ǵ� �Լ�
		// ���ڷ� ���� ����ü�� ������ Ȯ���� ���� ó��
		// 
		// Parameter : DB_WORK*, APIType
		// return : ����
		void DBReadFunc(DB_WORK* Protocol, WORD APIType);

		// DB�� Write �� ���� ���� �� ȣ��Ǵ� �Լ�.
		// ���ڷ� ���� ����ü�� ������ Ȯ���� ���� ó��
		//
		// Parameter : DB_WORK*
		// return : ����
		void DBWriteFunc(DB_WORK* Protocol);

		// Battle���� this�� �޾Ƶд�.
		void ParentSet(CBattleServer_Room* Parent);


	public:
		// ---------------
		// �����ڿ� �Ҹ���
		// ---------------

		// ������
		shDB_Communicate();

		// �Ҹ���
		virtual ~shDB_Communicate();
	};
}

// ----------------------------------------
// 
// MMOServer�� �̿��� ��Ʋ ����. �� ����
//
// ----------------------------------------
namespace Library_Jingyu
{
	class CBattleServer_Room :public CMMOServer
	{
		friend class CGame_MinitorClient;
		friend class CBattle_Master_LanClient;
		friend class CBattle_Chat_LanServer;	
		friend class shDB_Communicate;
		friend class CGameSession;


		// -----------------------
		// �̳� Ŭ����
		// -----------------------

		// ������ Ÿ��
		enum eu_ITEM_TYPE
		{
			CARTRIDGE = 0,	// źâ
			HELMET = 1,		// ���
			MEDKIT = 2		// �޵�Ŷ
		};

		// ������ Ÿ��
		enum eu_REDZONE_TYPE
		{
			LEFT = 0,	// ���� ������
			RIGHT,		// ������
			TOP,		// ��
			BOTTOM,		// �Ʒ�
		};

		// �÷��̾��� ���� ���
		enum eu_PLATER_MODE
		{
			AUTH = 0,	// �
			GAME,		// ����
			LOG_OUT,	// �α׾ƿ�
		};

		// ������ ����ü
		struct stRoomItem
		{
			UINT m_uiID;
			eu_ITEM_TYPE m_euType;
			float m_fPosX;
			float m_fPosY;

			// �������� �����ϴ� ����
			// 0 : ���� ������� ���� ������ ������
			// 1 : ������ ������ ������ (��Ʈ ������ ����)
			// 2 : ������ ���� ���� ������ (4�� ����)
			BYTE m_bItemArea;		
		};

		// CMMOServer�� cSession�� ��ӹ޴� ���� Ŭ����
		class CGameSession :public CMMOServer::cSession
		{
			friend class CBattleServer_Room;	

			// -----------------------
			// ��� ����
			// -----------------------

			// ȸ�� ��ȣ
			INT64 m_Int64AccountNo;

			// �г���
			TCHAR m_tcNickName[20];

			// ����Ű
			char m_cSessionKey[64];

			// ������ ClientKey
			// ��Ī������ �߱�.
			// ���� ��ο��� ����
			INT64 m_int64ClientKey;

			// �ش� ������ ���� ���� �� ��ȣ
			int m_iRoomNo;

			// CBattleServer_Room�� ������ (�θ�)
			CBattleServer_Room* m_pParent;		

			// AccountNo �ڷᱸ���� �� üũ �÷���
			// true�� �� ����
			bool m_bStructFlag;

			// �α��� ��Ŷ ó�� �÷���
			// true�� ����Ű, ������ �������� ���� ���� ó����.
			bool m_bLoginFlag;

			// �� ����� ���� üũ �÷���
			// true�� �� ����� ����.
			// �α��� HTTP ó�� ��, �̹� ���� ��Ŷ�� ���� ������ �� �÷��׸� true�� �����Ѵ�.
			bool m_bLogoutFlag;

			// ���� �÷���
			// true�� ����ִ� ����.
			// Game��忡���� ��ȿ�ϴ�.
			bool m_bAliveFlag;

			// �α��� �� �ʿ��� ������ ��� ���õǾ��� üũ�ϴ� Flag
			// �� 2���� HTTP�� ȣ��. �� ���� 2��� ��� ���õ� ��.
			LONG m_lLoginHTTPCount;

			// ������ ��� Ÿ��. ������ ������ Ÿ��
			eu_PLATER_MODE m_euModeType;


			// -----------------------
			// ���� ����
			// -----------------------
			int		m_iRecord_PlayCount;	// �÷��� Ƚ��
			int		m_iRecord_PlayTime;		// �÷��� �ð� �ʴ���
			int		m_iRecord_Kill;			// ���� Ƚ��
			int		m_iRecord_Die;			// ���� Ƚ��
			int		m_iRecord_Win;			// �����¸� Ƚ��

			// ���� ������ ���۵� �ð�. �и������� ����.
			// DB�� �����ϰų� �������� ������ ���� �ʴ����� ��ȯ �� ������.
			DWORD m_dwGameStartTime;		

			// ������ ������ ����Ǿ����� üũ�ϴ� �÷���
			// ���� ����ÿ��� �����ؾ� �ϱ� ������, LastDBWriteFlag�� �ϳ� �д�.
			bool m_bLastDBWriteFlag;	


			// -----------------------
			// ������ ����
			// -----------------------
			float	m_fPosX;
			float	m_fPosY;

			int		m_iHP;

			// �Ѿ� ��
			int		m_iBullet;

			// źâ ��
			int		m_iCartridge;
					   
			// ��� ��
			int m_iHelmetCount;


			// -----------------------
			// ���� Ÿ�� üũ
			// -----------------------

			// Fire_1(�� �߻�)�� ������ �ð�.
			// �� �ð����� 100m/s�̳��� ������ ��Ŷ�� �;� �������� ó������.
			// ��Ŷ�� ���� ���� ���� ����, ������ ��Ŷ�� ���� �ٽ� 0���� �ʱ�ȭ
			// ��, �ش� ���� 0�̸� Fire_1 ������ �ȿ� ����
			DWORD m_dwFire1_StartTime;
			
		private:
			// -----------------------
			// �����ڿ� �Ҹ���
			// -----------------------

			CGameSession();
			virtual ~CGameSession();

		public:
			// -----------------------
			// �ܺο��� ȣ�� ������ �Լ�
			// (����)
			// -----------------------

			// ���� ���� ����
			//
			// Parameter : ����
			// return : ���� �� true, ��� �� false
			bool GetAliveState();	


		public:
			// -----------------------
			// �ܺο��� ȣ�� ������ �Լ�
			// (����)
			// -----------------------

			// ĳ���� ���� �� ���õǴ� ��
			//
			// Parameter : ������ X, Y��ǥ,ĳ���� ������ �ð�(DWORD)
			// return : ���� 
			void StartSet(float PosX, float PosY, DWORD NowTime);

			// ���� ���� ����
			//
			// Parameter : ������ ���� ����(true�� ����, false�� ���)
			// return : ����
			void AliveSet(bool Flag);			

			// ���� ������ ó��
			// !! HP�� 0�� �Ǹ� �ڵ����� ������°� �ȴ� !!
			//
			// Parameter : ������ ���� ������
			// return : ���� �� ���� HP
			int Damage(int Damage);
			

		public:
			// -----------------------
			// ���� ����
			// -----------------------

			// ������ �¸� ī��Ʈ 1 ����
			// !! �¸� ī��Ʈ�� �÷��� �ð��� ���� ���� !!
			// !! ���ο����� �¸�ī��Ʈ ����, �÷���Ÿ�� ���� ��, DB�� ������� �Ѵ�. !!
			// 
			// Parameter : ����
			// return : ����
			void Record_Win_Add();

			// ������ ��� ī��Ʈ 1 ����
			// !! ��� ī��Ʈ�� �÷��� �ð��� ���� ���� !!
			// !! ���ο����� ��� ī��Ʈ ����, �÷���Ÿ�� ���� ��, DB�� ������� �Ѵ�. !!
			// 
			// Parameter : ����
			// return : ����
			void Recored_Die_Add();

			// ������ ų ī��Ʈ 1 ����
			// !! ���ο��� ų ī��Ʈ ���� ��, DB�� ������� �Ѵ� !!
			//
			// Parameter : ����
			// return : ����
			void Record_Kill_Add();

			// ������ �÷��� Ƚ�� 1 ����
			// !! ���ο��� �÷��� Ƚ�� ���� ��, DB�� ������� �Ѵ� !!
			//
			// Parameter : ����
			// return : ����
			void Record_PlayCount_Add();

			


		private:
			// -----------------
			// �����Լ�
			// -----------------

			// Auth �����忡�� ó��
			virtual void OnAuth_ClientJoin();
			virtual void OnAuth_ClientLeave(bool bGame = false);
			virtual void OnAuth_Packet(CProtocolBuff_Net* Packet);
			virtual void OnAuth_HeartBeat();

			// Game �����忡�� ó��
			virtual void OnGame_ClientJoin();
			virtual void OnGame_ClientLeave();
			virtual void OnGame_Packet(CProtocolBuff_Net* Packet);
			virtual void OnGame_HeartBeat();

			// Release��
			virtual void OnGame_ClientRelease();			

			// GQCS���� 121���� �߻� �� ȣ��Ǵ� �Լ�
			virtual void OnSemaphore();


			// -----------------
			// Auth��� ��Ŷ ó�� �Լ�
			// -----------------

			// �α��� ��û 
			// 
			// Parameter : CProtocolBuff_Net*
			// return : ����
			void Auth_LoginPacket(CProtocolBuff_Net* Packet);	

			// �� ���� ��û
			// 
			// Parameter : CProtocolBuff_Net*
			// return : ����
			void Auth_RoomEnterPacket(CProtocolBuff_Net* Packet);



			// -----------------
			// Game��� ��Ŷ ó�� �Լ�
			// -----------------

			// ������ �̵��� �� ������ ��Ŷ.
			//
			// Parameter : CProtocolBuff_Net*
			// return : ����
			void Game_MovePacket(CProtocolBuff_Net* Packet);

			// HitPoint ����
			//
			// Parameter : CProtocolBuff_Net*
			// return : ����
			void Game_HitPointPacket(CProtocolBuff_Net* Packet);

			// Fire 1 ��Ŷ (�� �߻�)
			//
			// Parameter : CProtocolBuff_Net*
			// return : ����
			void Game_Frie_1_Packet(CProtocolBuff_Net* Packet);

			// HitDamage
			//
			// Parameter : CProtocolBuff_Net*
			// return : ����
			void Game_HitDamage_Packet(CProtocolBuff_Net* Packet);
			
			// Frie 2 ��Ŷ (�� ����)
			//
			// Parameter : CProtocolBuff_Net*
			// return : ����
			void Game_Fire_2_Packet(CProtocolBuff_Net* Packet);

			// KickDamage
			//
			// Parameter : CProtocolBuff_Net*
			// return : ����
			void Game_KickDamage_Packet(CProtocolBuff_Net* Packet);

			// Reload Request
			//
			// Parameter : CProtocolBuff_Net*
			// return : ����
			void Game_Reload_Packet(CProtocolBuff_Net* Packet);

			// ������ ȹ�� ��û
			//
			// Parameter : CProtocolBuff_Net*, �������� Type
			// return : ����
			void Game_GetItem_Packet(CProtocolBuff_Net* Packet, int Type);	
		};
		
		// ���Ͽ��� �о���� �� ����ü
		struct stConfigFile
		{
			// �� ����
			TCHAR BindIP[20];
			int Port;
			int CreateWorker;
			int ActiveWorker;
			int CreateAccept;
			int HeadCode;
			int XORCode;
			int Nodelay;
			int MaxJoinUser;
			int LogLevel;

			// ������ Ŭ��
			TCHAR MasterServerIP[20];
			int MasterServerPort;
			int MasterClientCreateWorker;
			int MasterClientActiveWorker;
			int MasterClientNodelay;


			// ����͸� Ŭ��
			TCHAR MonitorServerIP[20];
			int MonitorServerPort;
			int MonitorClientCreateWorker;
			int MonitorClientActiveWorker;
			int MonitorClientNodelay;

			
			// ä�� ������
			TCHAR ChatLanServerIP[20];
			int ChatPort;
			int ChatCreateWorker;
			int ChatActiveWorker;
			int ChatCreateAccept;
			int ChatNodelay;
			int ChatMaxJoinUser;
			int ChatLogLevel;
		};
				
		// �� ����ü
		struct stRoom
		{	
			// ��Ʋ���� ��ȣ
			int m_iBattleServerNo;

			// �� ��ȣ
			int m_iRoomNo;

			// ���� ����
			// 0 : ����
			// 1 : �غ��
			// 2 : �÷��� ��
			int m_iRoomState;

			// ���� �� �ȿ� �ִ� ���� ��
			int m_iJoinUserCount;			

			// ������ ���� ��. 
			// HP�� 0 �̻��� ����.
			int m_iAliveUserCount;

			// ���� ���� ��ȯ�� ���� ��.
			// �� ���� m_iJoinUserCount�� ����������, �� ���� ��ο���
			// ĳ���� ���� ��Ŷ�� ������ (��, �ٸ���� ����)
			int m_iGameModeUser;

			// Play���·� ������ ���� ī��Ʈ �ٿ�. �и������� ����
			// timeGetTime()�� ���� ����.
			DWORD m_dwCountDown;			

			// ------------

			// ������ ����Ǿ����� üũ�ϴ� Flag.
			// �¸��ڰ� ��������, Game ���� ��Ŷ�� ���� �� ����ȴ�.
			// true�� ����� ����.
			bool m_bGameEndFlag;	

			// ���� ���ᰡ üũ�� ������ �и�������
			// m_bGameEndFlag�� true�� ����� �ٷ� 
			DWORD m_dwGameEndMSec;

			// ------------

			// ������ ����Ǿ�, �� ���� ��� �����鿡�� Shutdown�� ���� ����.
			// �������� Shutdown�� ������ �ʵ��� �ϱ� ����.
			// true�� �̹�, �� ���� ��� �������� �˴ٿ��� ��.
			bool m_bShutdownFlag;

			// �� ���� ��ū (��Ʋ���� ���� ��ū���� �ٸ�)
			char m_cEnterToken[32];	

			// ------------

			// �濡 ������ ������ ���� �ڷᱸ��.
			vector<CGameSession*> m_JoinUser_Vector;
					   
			// ���� ������ �ִ� �ο� ��. ���� ��
			const int m_iMaxJoinCount = 5;	

			// ------------

			// �� ����, �����ϰ� ������ID�� �ο��ϱ� ���� ����.
			// ������ ���� �� ++�Ѵ�.
			UINT m_uiItemID;		

			// �ش� �濡 ������ ������ �ڷᱸ��
			// Key : ItemID, Value : stRoomTiem*
			unordered_map<UINT, stRoomItem*> m_RoomItem_umap;

			// ------------

			CBattleServer_Room* m_pBattleServer;


			// ------------

			// ������ Ȱ��ȭ ����.
			// eu_REDZONE_TYPE�� ���� üũ�Ѵ�.
			int m_arrayRedZone[4];

			// ������ Ȱ��ȭ üũ�� ���� �ð�.
			// �� �ð����κ��� 40�ʰ� �������� ������ Ȱ��ȭ.
			DWORD m_dwReaZoneTime;

			// ������ ������ Tick üũ�� ���� �ð�.
			// 1�ʴ����� ����
			DWORD m_dwTick;

			// ���� Ȱ��ȭ�� �������� ��.
			int m_iRedZoneCount;

			// �������� X,Y ��ǥ.
			// �������� Ȱ��ȭ �� �� ���� ����ȴ�.
			float m_fSafePos[2][2];

			// �̹��� ������, �������� ���� ��� ��Ŷ ���� ����.
			// ������ Ȱ��ȭ �� �ٽ� false�� �ǵ�����.
			bool m_bRedZoneWarningFlag;

			// Last ������ ��, ��� �������밡 �� ������.
			// �� ���� �� �̹� �����ȴ�.
			BYTE m_bLastRedZoneSafeType;
		
			// ------------


			// ------------

			// ������ �� ������ 10�ʿ� 1ȸ ������ �����ϱ�
			// ���� ��ġ(4����)�� ���� �迭
			//
			// �ش� ���� ������ �� : 0
			// �������� ���� �� : 0
			// �������� ���� �� : ������ �������� ȹ���� �ð�
			//
			// �� ������ ���鼭 ������ ȹ���� �ð� ���� 10�ʰ� ������
			// źâ�� �����ؾ� �Ѵ�.
			DWORD m_dwItemCreateTick[4];



			// ------------
			// ��� �Լ�
			// ------------

			// ������
			stRoom();

			// �Ҹ���
			virtual ~stRoom();

			// �ڷᱸ�� ���� ��� �������� ���ڷ� ���� ��Ŷ ������
			//
			// Parameter : CProtocolBuff_Net*
			// return : �ڷᱸ�� ���� ������ 0���� ��� false
			//		  : �� �ܿ��� true
			bool SendPacket_BroadCast(CProtocolBuff_Net* SendBuff);

			// �ڷᱸ�� ���� ��� �������� ���ڷ� ���� ��Ŷ ������
			// ���ڷ� ���� AccountNo�� �����ϰ� ������
			//
			// Parameter : CProtocolBuff_Net*, AccountNo
			// return : �ڷᱸ�� ���� ������ 0���� ��� false
			//		  : �� �ܿ��� true
			bool SendPacket_BroadCast(CProtocolBuff_Net* SendBuff, INT64 AccountNo);
			
			// �� ���� ��� ������ Auth_To_Game���� ����
			//
			// Parameter : ����
			// return : ����
			void ModeChange();			

			// �� ���� ��� ������ �������·� ����
			//
			// Parameter : ����
			// return : �ڷᱸ�� ���� ������ 0���� ��� false
			//		  : �� �ܿ��� true
			bool AliveFlag_True();

			// �� ���� �����鿡�� ���� ���� ��Ŷ ������
			//
			// Parameter : ����
			// return : ����
			void GameOver();

			// �� ���� �����鿡�� �˴ٿ� ������
			//
			// Parameter : ����
			// return : ����
			void Shutdown_All();

			// �� ���� ��� �����鿡�� �� ������Ŷ�� �ٸ� ���� ���� ��Ŷ ������
			// 
			// Parameter : ����
			// return : ����
			void CreateCharacter();

			// �¸��ڿ��� ���� ������
			//
			// Parameter : ����
			// return : ����
			void WInRecodeSend();

			// �ش� �濡, ������ ���� (���� ���� ���� �� ����)
			// ���� ��, �� ���� �������� ������ ���� ��Ŷ ����
			//
			// Parameter : ����
			// return : ����
			void StartCreateItem();

			// ������ ����� ��ġ�� ������ ����
			// ���� ��, �� ���� �������� ������ ���� ��Ŷ ����
			//
			// Parameter : CGameSession* (����� ����)
			// return : ����
			void PlayerDieCreateItem(CGameSession* DiePlayer);

			// ��ǥ�� ���� ��ġ�� ������ 1�� ����
			//
			// Parameter : ������ XY��ǥ, ������ Type, ������ ����
			// return : ����
			void CreateItem(float PosX, float PosY, int Type, BYTE Area);

			// �� ���� ��� �������� "���� �߰���" ��Ŷ ������
			//
			// Parameter : �̹��� ������ ���� CGameSession*
			// return : ����
			void Send_AddUser(CGameSession* NowPlayer);

			// �������� ��������, �� ���� ���� ��� �� ó�� �Լ�
			//
			// Parameter : ������(CGameSession*), �����(CGameSession*) 
			// return : ����
			void Player_Die(CGameSession* AttackPlayer, CGameSession* DiePlayer);


			// ------------
			// �ڷᱸ�� �Լ�
			// ------------

			// �ڷᱸ���� Insert
			//
			// Parameter : �߰��ϰ��� �ϴ� CGameSession*
			// return : ����
			void Insert(CGameSession* InsertPlayer);

			// �ڷᱸ���� ������ �ֳ� üũ
			//
			// Parameter : AccountNo
			// return : ã�� ������ CGameSession*
			//		  : ������ ���� �� nullptr
			CGameSession* Find(INT64 AccountNo);

			// �ڷᱸ������ Erase
			//
			// Parameter : �����ϰ��� �ϴ� CGameSession*
			// return : ���� �� true
			//		  : ���� ��  false
			bool Erase(CGameSession* InsertPlayer);


			// ------------
			// ������ �ڷᱸ�� �Լ�
			// ------------

			// ������ �ڷᱸ���� Insert
			//
			// Parameter : ItemID, stRoomItem*
			// return : ����
			void Item_Insert(UINT ID, stRoomItem* InsertItem);

			// ������ �ڷᱸ���� �������� �ֳ� üũ
			//
			// Parameter : itemID
			// return : ã�� �������� stRoomItem*
			//		  : �������� ���� �� nullptr
			stRoomItem* Item_Find(UINT ID);

			// ������ �ڷᱸ������ Erase
			//
			// Parameter : �����ϰ��� �ϴ� stRoomItem*
			// return : ���� �� true
			//		  : ���� ��  false
			bool Item_Erase(stRoomItem* InsertPlayer);


			// ------------
			// ������ ���� �Լ�
			// ------------
			
			// ������ ���
			// �� �Լ���, ������ ��� ������ �� ������ ȣ��ȴ�.
			// �̹� �ۿ���, ȣ�� ���� �� üũ �� �� �Լ� ȣ��.
			//
			// Parameter : ��� �ð�(BYTE). 
			// return : ����
			void RedZone_Warning(BYTE AlertTimeSec);

			// ������ Ȱ��ȭ
			// �� �Լ���, �������� Ȱ��ȭ �� �������� ȣ��ȴ�
			// ��, Ȱ��ȭ ���� ������ �ۿ��� ���� ������ �� �Լ� ȣ��
			//
			// Parameter : ����
			// return : ����
			void RedZone_Active();

			// ������ ������ üũ
			// �� �Լ���, �������� �������� ��� �� ������ ȣ��
			// ��, ���� ������ �ۿ��� ���� ������ �� �Լ� ȣ��
			//
			// Parameter : ����
			// return : ����
			void RedZone_Damage();
		};

		// �� ���� state
		enum eu_ROOM_STATE
		{
			// ����
			WAIT_ROOM = 0,

			// �غ��
			READY_ROOM,

			// �÷��̹�
			PLAY_ROOM
		};
				
		// ���� ���� �Ұ����� Config ���� ���� ����ü
		struct stCONFIG
		{
			// Auth_Update���� �� �����ӿ� ó���� HTTP��� ��ó��
			// ���� ��
			const int m_iHTTP_MAX = 50;

			// �ִ� ������ �� �ִ� �� ��
			// ���� ��
			const LONG m_lMaxTotalRoomCount = 1200;

			// �ִ� ������ �� �ִ� ���� ��
			// ���� ��
			const LONG m_lMaxWaitRoomCount = 500;

			// Auth_Update���� �� �����ӿ� ���� ������ �� ��
			// ���� ��
			const int m_iLoopCreateRoomCount = 500;

			// Auth_Update���� �� �����ӿ�, Game���� �ѱ�� �� ��
			// ��, Ready ������ ���� Play�� �����ϴ� ��
			// ���� ��
			const int m_iLoopRoomModeChange = 50;

			// ��ū ��߱� �ð�
			// �и������� ����.
			// ex) 1000�� ���, 1�� ������ ��ū ��߱�.
			const int m_iTokenChangeSlice = 20000;

			// ���� Ready���°� �Ǿ��� �� ī��Ʈ�ٿ�.
			// �� ����
			const BYTE m_bCountDownSec = 10;

			// ���� Ready���°� �Ǿ��� �� ī��Ʈ�ٿ�.
			// �и������� ����
			const int m_iCountDownMSec = 10000;

			// Play������ ����, ���� ���� �� �� �ʵ��� ����ϴ���.
			// �и������� ����
			const UINT m_iRoomCloseDelay = 5000;

			// ������ ȹ�� ��ǥ ����
			const float m_fGetItem_Correction = 3.0f;

			// ������ Ȱ��ȭ �ð�.
			// ���� ���� ��, �Ʒ� �ð����� �ϳ��� Ȱ��ȭ
			const DWORD m_dwReaZoneActiveTime = 40000; // (���� 40��)
		
			// ������ ��� ������ �ð�.
			// ������ Ȱ��ȭ �ð���, �� ������ŭ ������ ���, ��� ��Ŷ�� ������.
			const DWORD m_dwRedZoneWarningTime = 20000; // (���� 20��)

			// ������ �ִ� ��.
			// �� Ƚ�� �̻��� ������ Ȱ��ȭ �Ұ���
			const int m_iRedZoneActiveLimit = 5;
		};




		// -----------------------
		// ��� ����
		// -----------------------

		// ����. CMMOServer�� ����.
		CGameSession* m_cGameSession;

		// Config ����
		stConfigFile m_stConfig;

		// ������ �������� �����ϴ� ����ü ����
		stCONFIG m_stConst;

		// ����͸� Ŭ��
		CGame_MinitorClient* m_Monitor_LanClient;

		// �����Ϳ� ����� Ŭ��
		CBattle_Master_LanClient* m_Master_LanClient;

		// ä�ü������� ���� �޴� ������
		CBattle_Chat_LanServer* m_Chat_LanServer;

		// ���� �ڵ�
		// Ŭ�� ���� ���� ��. �Ľ����� �о��.
		int m_uiVer_Code;

		// shDB�� ����ϴ� ����
		shDB_Communicate m_shDB_Communicate;	

		// ���� ���� No
		// ������ ������ �Ҵ��� �ش�.
		int m_iServerNo;


		// -----------------------
		// ������ ���� ����
		// -----------------------

		// ������ ���� ������ ���� ����. �����ڿ��� next_permutation�� �̿��� ����
		// 4!
		int m_arrayRedZoneCreate[24][4];

		// ������ Ÿ�Կ� ���� Ȱ�� ��ġ
		float m_arrayRedZoneRange[4][2][2];

		// ������ �������� ��������.
		// �迭 0���� 1�� Ÿ���� �������� ��ǥ�� �������.
		float m_arrayLastRedZoneSafeRange[4][2][2];



		// -----------------------
		// ��¿� ����
		// -----------------------
		
		// ��Ʋ���� ���� ��ū ����
		LONG m_lBattleEnterTokenError;

		// �� ���� ��ū ����
		LONG m_lRoomEnterTokenError;

		// auth�� �α��� ��Ŷ����, DB ���� �� ����� -10(����� ����)�� �� ���
		LONG m_lQuery_Result_Not_Find;

		// auth�� �α��� ��Ŷ����, DB ���� �� ����� -10�� �ƴѵ� 1�� �ƴ� ������ ��� 
		LONG m_lTempError;

		// auth�� �α��� ��Ŷ����, ������ SessionKey(��ū)�� �ٸ� ���
		LONG m_lTokenError;

		// auth�� �α��� ��Ŷ����, ������ ��� �� ������ �ٸ� ���
		LONG m_lVerError;

		// auth�� �α��� ��Ŷ���� ���� DBWrite�� ��
		LONG m_OverlapLoginCount_DB;

		// auth�� �α��� ��Ŷ���� �ߺ��α��� ��
		LONG m_OverlapLoginCount;

		// Ready �� ��
		LONG m_lReadyRoomCount;

		// Play �� ��
		LONG m_lPlayRoomCount;

		// �Ѿ� ���� �� �������� �������ϴ� ������(�Ÿ� 1�� ������)
		float m_fFire1_Damage;

		// AuthFPS, GameFPS ������
		// 2������ �ش� ������ �ʿ��ϱ� ������, ����Լ����� �޾Ƶд�.
		// �� ���� ����͸� Ŭ�� ������ ����.
		LONG m_lAuthFPS;
		LONG m_lGameFPS;



		

		// -----------------------
		// ��ū���� ����
		// -----------------------

		// ���� ��ū ����
		// ��Ʋ������ �� 2�� �� �ϳ��� ��ġ�Ѵٸ� �´ٰ� �����Ų��.

		// ��Ʋ���� ���� ��ū 1��
		// "����" ��ū�� �����Ѵ�.
		// �����Ϳ� ����� �� Ŭ�� ���� �� �����Ѵ�.
		char m_cConnectToken_Now[32];

		// ��Ʋ���� ���� ��ū 2�� 
		// "����" ��ū�� �����Ѵ�.
		// ��ū�� ���Ҵ� �� ���, ���� ��ū�� ����� ������ ��
		// ���ο� ��ū�� "����" ��ū�� �ִ´�.
		char m_cConnectToken_Before[32];

		// ��ū�� ��
		SRWLOCK m_ServerEnterToken_srwl;


		// -----------------------
		// �� ���� ����
		// -----------------------

		// �� ��ȣ�� �Ҵ��� ����
		// ���Ͷ����� ++
		LONG m_lGlobal_RoomNo;
		
		// Wait Room ī��Ʈ
		LONG m_lNowWaitRoomCount;

		// ���� ���� ���� ������� �� �� (��� �� ���ļ�)
		LONG m_lNowTotalRoomCount;

		// �� ���� ��ū
		// �̸� 64������ ����� �ΰ�, ����� �� �������� ������.
		char m_cRoomEnterToken[64][32];

		// stRoom ���� Pool
		CMemoryPoolTLS<stRoom> *m_Room_Pool;


		// -----------------------
		// ������ ���� ����
		// -----------------------

		// ������ ����ü ���� �޸�Ǯ TLS
		CMemoryPoolTLS<stRoomItem> *m_Item_Pool;
		

		


		// -----------------------
		// �� ���� �ڷᱸ�� ����
		// -----------------------

		// �� ���� umap
		//
		// Key : RoomNo, Value : stRoom*
		unordered_map<int, stRoom*> m_Room_Umap;

		// m_Room_Umap SRW��
		SRWLOCK m_Room_Umap_srwl;






		// -----------------------
		// ���� ���� �ڷᱸ�� ����
		// -----------------------

		// AccountNo�� �̿��� CGameSession* ����
		// �ߺ� �α��� üũ �뵵
		//
		// ���� ���� ������ AccountNo�� �������� CGameSession* ����
		// Key : AccountNo, Value : CGameSession*
		unordered_map<INT64, CGameSession*> m_AccountNo_Umap;

		// m_AccountNo_Umap�� SRW��
		SRWLOCK m_AccountNo_Umap_srwl;	



		// -----------------------
		// ���� DB�� Write ���� ���� + �ߺ��α��� ���� �ڷᱸ�� ����
		// -----------------------

		// AccountNo�� �̿��� DBWrite ���� ī��Ʈ ����.
		//
		// Key : AccountNo, Value : WriteCount(int)
		unordered_map<INT64, int> m_DBWrite_Umap;

		// m_DBWrite_Umap�� SRW��
		SRWLOCK m_DBWrite_Umap_srwl;



		// -----------------------
		// �ߺ� �α������� ���� ���ܾ� �ϴ� Game �����(������ ����) ���� ���� �ڷᱸ�� ����
		// -----------------------

		// ClientKey, CGameSession*�� �ٷ�� forward_list
		//
		// pair�� �̿��� ä��
		forward_list<pair<INT64, CGameSession*>> m_Overlap_list;

		// m_Overlap_list�� SRW��
		SRWLOCK m_Overlap_list_srwl;


	public:
		// -----------------------
		// �ܺο��� ��� ������ �Լ�
		// -----------------------

		// Start
		// ���������� CMMOServer�� Start, ���� ���ñ��� �Ѵ�.
		//
		// Parameter : ����
		// return : ���� �� false
		bool ServerStart();

		// Stop
		// ���������� Stop ����
		//
		// Parameter : ����
		// return : ����
		void ServerStop();

		// ��¿� �Լ�
		//
		// Parameter : ����
		// return : ����
		void ShowPrintf();


	private:
		// -----------------------
		// ���ο����� ����ϴ� �Լ�
		// -----------------------

		// ���Ͽ��� Config ���� �о����
		// 
		// Parameter : config ����ü
		// return : ���������� ���� �� true
		//		  : �� �ܿ��� false
		bool SetFile(stConfigFile* pConfig);

		// �Ѿ� ������ ��� �� ���Ǵ� �Լ� (������ ���ÿ��� ������ ����.)
		// ���� ���ҽ��Ѿ� �ϴ� HP�� ����Ѵ�.
		//
		// Parameter : �����ڿ� �������� �Ÿ�
		// return : ���ҽ��Ѿ��ϴ� HP
		int GunDamage(float Range);

		// �� ���� �Լ�
		//
		// Parameter : ����
		// return : ����
		void RoomCreate();

		// HTTP ��� �� ��ó��
		// Auth �����忡�� �������� ȣ��ȴ�.
		//
		// Parameter : ����
		// return : ����
		void AuthLoop_HTTP();

		// ������ ���� Ȥ�� ä�ü����� �׾��� ���
		// Auth �����忡�� ȣ��ȴ�.
		//
		// Parameter : ����
		// return : ���� 
		void AuthLoop_ServerDie();

		// �� ���� ó��
		// ���� game���� �����ϱ� ���..
		// Auth �����忡�� �������� ȣ��ȴ�.
		//
		// Parameter : ����
		// return : ����
		void AuthLoop_RoomLogic();

		// ���Ӹ���� �ߺ� �α��� ���� ����
		// Game �����忡�� �������� ȣ��ȴ�.
		// 
		// Parameter : ����
		// return : ����
		void GameLoop_OverlapLogin();

		// ���� ����� �� üũ
		// Game �����忡�� �������� ȣ��ȴ�.
		//
		// Parameter : (out)int�� �迭(������ �� ��ȣ ������), (out)������ ���� ��
		// return : ����
		void GameLoop_RoomLogic(int DeleteRoomNo[], int* Index);

		// ���� ����� �� ����
		// Game �����忡�� �������� ȣ��ȴ�.
		//
		// Parameter : (out)int�� �迭(������ �� ��ȣ ������), (out)������ ���� ��
		// return : ����
		void GameLoop_RoomDelete(int DeleteRoomNo[], int* Index);

		
		

		

	private:
		// -----------------------
		// ��Ŷ ��ó�� �Լ�
		// -----------------------

		// Login ��Ŷ�� ���� ���� ó�� (��ū üũ ��..)
		//
		// Parameter : DB_WORK_LOGIN*
		// return : ����
		void Auth_LoginPacket_AUTH(DB_WORK_LOGIN* DBData);

		// Login ��Ŷ�� ���� Contents ���� ��������
		//
		// Parameter : DB_WORK_LOGIN*
		// return : ����
		void Auth_LoginPacket_Info(DB_WORK_LOGIN_CONTENTS* DBData);



	private:
		// -----------------------
		// AccountNo �ڷᱸ�� ���� �Լ�
		// -----------------------

		// AccountNo �ڷᱸ���� ������ �߰��ϴ� �Լ�
		//
		// Parameter : AccountNo, CGameSession*
		// return : ���� �� true
		//		  : ���� �� false
		bool InsertAccountNoFunc(INT64 AccountNo, CGameSession* InsertPlayer);

		// AccountNo �ڷᱸ������ ���� �˻� ��, �ش� �������� Disconenct �ϴ� �Լ�
		//
		// Parameter : AccountNo
		// return : ����
		void DisconnectAccountNoFunc(INT64 AccountNo);

		// AccountNo �ڷᱸ������ ������ �����ϴ� �Լ�
		//
		// Parameter : AccountNo
		// return : ���� �� true
		//		  : ���� �� false
		bool EraseAccountNoFunc(INT64 AccountNo);



	private:
		// -----------------------
		// DBWrite ī��Ʈ �ڷᱸ�� ���� �Լ�
		// -----------------------

		// DBWrite ī��Ʈ ���� �ڷᱸ���� ������ �߰��ϴ� �Լ�
		//
		// Parameter : AccountNo
		// return : ���� �� true
		//		  : ����(Ű �ߺ�) �� false
		bool InsertDBWriteCountFunc(INT64 AccountNo);

		// DBWrite ī��Ʈ ���� �ڷᱸ������ ������ �˻� ��, 
		// ī��Ʈ(Value)�� 1 �����ϴ� �Լ�
		//
		// Parameter : AccountNo
		// return : ����
		void AddDBWriteCountFunc(INT64 AccountNo);

		// DBWrite ī��Ʈ ���� �ڷᱸ������ ������ �˻� ��, 
		// ī��Ʈ(Value)�� 1 ���ҽ�Ű�� �Լ�
		// ���� �� 0�̵Ǹ� Erase�Ѵ�.
		//
		// Parameter : AccountNo
		// return : ���� �� true
		//		  : �˻� ���� �� false
		bool MinDBWriteCountFunc(INT64 AccountNo);
		


	private:
		// ---------------------------------
		// �� �ڷᱸ�� ���� �Լ�
		// ---------------------------------

		// ���� Room �ڷᱸ���� Insert�ϴ� �Լ�
		//
		// Parameter : RoomNo, stRoom*
		// return : ���� �� true
		//		  : ����(�ߺ� Ű) �� false	
		bool InsertRoomFunc(int RoomNo, stRoom* InsertRoom);



	private:
		// -----------------------
		// �ߺ��α��� �ڷᱸ��(list) ���� �Լ�
		// -----------------------

		// �ߺ��α��� �ڷᱸ��(list)�� Insert
		//
		// Parameter : ClientKey, CGameSession*
		// return : ����
		void InsertOverlapFunc(INT64 ClientKey, CGameSession* InsertPlayer);		




	private:
		// -----------------------
		// �����Լ�
		// -----------------------

		// AuthThread���� 1Loop���� 1ȸ ȣ��.
		// 1�������� ���������� ó���ؾ� �ϴ� ���� �Ѵ�.
		// 
		// parameter : ����
		// return : ����
		virtual void OnAuth_Update();

		// GameThread���� 1Loop���� 1ȸ ȣ��.
		// 1�������� ���������� ó���ؾ� �ϴ� ���� �Ѵ�.
		// 
		// parameter : ����
		// return : ����
		virtual void OnGame_Update();

		// ���ο� ���� ���� ��, Auth���� ȣ��ȴ�.
		//
		// parameter : ������ ������ IP, Port
		// return false : Ŭ���̾�Ʈ ���� �ź�
		// return true : ���� ���
		virtual bool OnConnectionRequest(TCHAR* IP, USHORT port);

		// ��Ŀ �����尡 ��� �� ȣ��Ǵ� �Լ�.
		// GQCS �ٷ� �ϴܿ��� ȣ��
		// 
		// parameter : ����
		// return : ����
		virtual void OnWorkerThreadBegin();

		// ��Ŀ �����尡 ���� �� ȣ��Ǵ� �Լ�
		// GQCS �ٷ� ������ ȣ��
		// 
		// parameter : ����
		// return : ����
		virtual void OnWorkerThreadEnd();

		// ���� �߻� �� ȣ��Ǵ� �Լ�.
		//
		// parameter : ���� �ڵ�(���� ������ �����ڵ�� WinGetLastError() �Լ��� ��� ����. ���� ��� 0�� ���ϵ�)
		//			 : ���� �ڵ忡 ���� ��Ʈ��
		// return : ����
		virtual void OnError(int error, const TCHAR* errorStr);


	public:
		// -----------------------
		// �����ڿ� �Ҹ���
		// -----------------------

		// ������
		CBattleServer_Room();

		// �Ҹ���
		virtual ~CBattleServer_Room();
	};

}

// ----------------------------------------
// 
// ������ ������ ����Ǵ� LanClient
//
// ----------------------------------------
namespace Library_Jingyu
{
	class CBattle_Master_LanClient :public CLanClient
	{
		friend class CBattleServer_Room;
		friend class CBattle_Chat_LanServer;


		// ------------- 
		// ��� ����
		// ------------- 

		// ���� ������ ������ ����� ���� ID
		ULONGLONG m_ullSessionID;

		// ������ �� ������ IP�� Port
		TCHAR m_tcBattleNetServerIP[30];
		int m_iBattleNetServerPort;

		// ä�� �� ������ IP�� Port
		// ä�� Lan Ŭ�󿡰� �޴´�.
		TCHAR m_tcChatNetServerIP[30];
		int m_iChatNetServerPort;

		// ������ ������ ��� ���� ��ū
		char m_cMasterToken[32];

		// ������ �������� ��Ŷ�� �ϳ� ���� �� 1�� �����Ǵ� ��.
		UINT uiReqSequence;

		// �α��� üũ
		// true�� �α��� ��Ŷ�� ���� ����
		bool m_bLoginCheck;




		// ----------------------
		// !!Battle ������ this !!
		// ----------------------
		CBattleServer_Room* m_BattleServer_this;


		



	private:
		// -----------------------
		// ��Ŷ ó�� �Լ�
		// -----------------------

		// �����Ϳ��� ���� �α��� ��û ����
		//
		// Parameter : SessionID, CProtocolBuff_Lan*
		// return : ����
		void Packet_Login_Res(ULONGLONG SessionID, CProtocolBuff_Lan* Payload);

		// �ű� ���� ���� ����
		//
		// Parameter : SessionID, CProtocolBuff_Lan*
		// return : ����
		void Packet_NewRoomCreate_Res(ULONGLONG SessionID, CProtocolBuff_Lan* Payload);
		
		// ��ū ����� ����
		//
		// Parameter : SessionID, CProtocolBuff_Lan*
		// return : ����
		void Packet_TokenChange_Res(ULONGLONG SessionID, CProtocolBuff_Lan* Payload);

		// �� ���� ����
		//
		// Parameter : SessionID, CProtocolBuff_Lan*
		// return : ����
		void Packet_RoomClose_Res(ULONGLONG SessionID, CProtocolBuff_Lan* Payload);

		// �� ���� ����
		//
		// Parameter : RoomNo, AccountNo
		// return : ����
		void Packet_RoomLeave_Res(ULONGLONG SessionID, CProtocolBuff_Lan* Payload);


	private:
		// -----------------------
		// ä�� Lan ������ ȣ���ϴ� �Լ�
		// -----------------------

		// �����Ϳ���, �ű� ���� ���� ��Ŷ ������
		//
		// Parameter : RoomNo
		// return : ����
		void Packet_NewRoomCreate_Req(int RoomNo);

		// ��ū ��߱� �Լ�
		//
		// Parameter : ����
		// return : ����
		void Packet_TokenChange_Req();


	private:	
		// -----------------------
		// Battle Net ������ ȣ���ϴ� �Լ�
		// -----------------------		

		// �����Ϳ���, �� ���� ��Ŷ ������
		//
		// Parameter : RoomNo
		// return : ����
		void Packet_RoomClose_Req(int RoomNo);

		// �����Ϳ���, �� ���� ��Ŷ ������
		//
		// Parameter : RoomNo, ClientKey
		// return : ����
		void Packet_RoomLeave_Req(int RoomNo, INT64 ClientKey);



	public:
		// -----------------------
		// �ܺο��� ��� ������ �Լ�
		// -----------------------

		// ���� �Լ�
		// ����������, ��ӹ��� CLanClient�� Startȣ��.
		//
		// Parameter : ������ ������ IP, ��Ʈ, ��Ŀ������ ��, Ȱ��ȭ��ų ��Ŀ������ ��, TCP_NODELAY ��� ����(true�� ���)
		// return : ���� �� true , ���� �� falsel 
		bool ClientStart(TCHAR* ConnectIP, int Port, int CreateWorker, int ActiveWorker, int Nodelay);

		// ���� �Լ�
		// ����������, ��ӹ��� CLanClient�� Stopȣ��.
		// �߰���, ���ҽ� ���� ��
		//
		// Parameter : ����
		// return : ����
		void ClientStop();

		// ��Ʋ������ this�� �Է¹޴� �Լ�
		// 
		// Parameter : ��Ʋ ������ this
		// return : ����
		void ParentSet(CBattleServer_Room* ChatThis);



	private:
		// -----------------------
		// ���� �����Լ�
		// -----------------------

		// ��ǥ ������ ���� ���� ��, ȣ��Ǵ� �Լ� (ConnectFunc���� ���� ���� �� ȣ��)
		//
		// parameter : ����Ű
		// return : ����
		virtual void OnConnect(ULONGLONG SessionID);

		// ��ǥ ������ ���� ���� �� ȣ��Ǵ� �Լ� (InDIsconnect �ȿ��� ȣ��)
		//
		// parameter : ����Ű
		// return : ����
		virtual void OnDisconnect(ULONGLONG SessionID);

		// ��Ŷ ���� �Ϸ� �� ȣ��Ǵ� �Լ�.
		//
		// parameter : ���� ����Ű, CProtocolBuff_Lan*
		// return : ����
		virtual void OnRecv(ULONGLONG SessionID, CProtocolBuff_Lan* Payload);

		// ��Ŷ �۽� �Ϸ� �� ȣ��Ǵ� �Լ�
		//
		// parameter : ���� ����Ű, Send �� ������
		// return : ����
		virtual void OnSend(ULONGLONG SessionID, DWORD SendSize);

		// ��Ŀ �����尡 ��� �� ȣ��Ǵ� �Լ�.
		// GQCS �ٷ� �ϴܿ��� ȣ��
		// 
		// parameter : ����
		// return : ����
		virtual void OnWorkerThreadBegin();

		// ��Ŀ �����尡 ���� �� ȣ��Ǵ� �Լ�
		// GQCS �ٷ� ������ ȣ��
		// 
		// parameter : ����
		// return : ����
		virtual void OnWorkerThreadEnd();

		// ���� �߻� �� ȣ��Ǵ� �Լ�.
		//
		// parameter : ���� �ڵ�(���� ������ �����ڵ�� WinGetLastError() �Լ��� ��� ����. ���� ��� 0�� ���ϵ�)
		//			 : ���� �ڵ忡 ���� ��Ʈ��
		// return : ����
		virtual void OnError(int error, const TCHAR* errorStr);

		

	public:
		// -----------------------
		// �����ڿ� �Ҹ���
		// -----------------------
		CBattle_Master_LanClient();
		virtual ~CBattle_Master_LanClient();

	};
}

// ---------------
// CGame_MonitorClient
// CLanClient�� ��ӹ޴� ����͸� Ŭ��. ����͸� LanServer�� ���� ����
// ---------------
namespace Library_Jingyu
{
	class CGame_MinitorClient :public CLanClient
	{
		friend class CGameServer;

		// ������ ������ ��Ƶα�
		enum en_MonitorClient
		{
			dfSERVER_NO = 22	// ��Ʋ������ 22���̴�
		};

		// ����͸� ������ ���� ������ �������� �ڵ�.
		HANDLE m_hMonitorThread;

		// ����͸� �����带 �����ų �̺�Ʈ
		HANDLE m_hMonitorThreadExitEvent;

		// ���� ����͸� ������ ����� ���� ID
		ULONGLONG m_ullSessionID;

		// ----------------------
		// !!Battle ������ this !!
		// ----------------------
		CBattleServer_Room* m_BattleServer_this;



	private:
		// -----------------------
		// ������
		// -----------------------

		// ���� �ð����� ����͸� ������ ������ �����ϴ� ������
		static UINT	WINAPI MonitorThread(LPVOID lParam);




	private:
		// -----------------------
		// ���ο����� ����ϴ� ��� �Լ�
		// -----------------------	

		// ����͸� ������ ������ ����
		//
		// Parameter : DataType(BYTE), DataValue(int), TimeStamp(int)
		// return : ����
		void InfoSend(BYTE DataType, int DataValue, int TimeStamp);
			   		 

	public:
		// -----------------------
		// �����ڿ� �Ҹ���
		// -----------------------
		CGame_MinitorClient();
		virtual ~CGame_MinitorClient();


	public:
		// -----------------------
		// �ܺο��� ��� ������ �Լ�
		// -----------------------

		// ���� �Լ�
		// ����������, ��ӹ��� CLanClient�� Startȣ��.
		//
		// Parameter : ������ ������ IP, ��Ʈ, ��Ŀ������ ��, Ȱ��ȭ��ų ��Ŀ������ ��, TCP_NODELAY ��� ����(true�� ���)
		// return : ���� �� true , ���� �� falsel 
		bool ClientStart(TCHAR* ConnectIP, int Port, int CreateWorker, int ActiveWorker, int Nodelay);

		// ���� �Լ�
		// ����������, ��ӹ��� CLanClient�� Stopȣ��.
		// �߰���, ���ҽ� ���� ��
		//
		// Parameter : ����
		// return : ����
		void ClientStop();

		// ��Ʋ������ this�� �Է¹޴� �Լ�
		// 
		// Parameter : ��Ʋ ������ this
		// return : ����
		void ParentSet(CBattleServer_Room* ChatThis);




	private:
		// -----------------------
		// ���� �����Լ�
		// -----------------------

		// ��ǥ ������ ���� ���� ��, ȣ��Ǵ� �Լ� (ConnectFunc���� ���� ���� �� ȣ��)
		//
		// parameter : ����Ű
		// return : ����
		virtual void OnConnect(ULONGLONG SessionID);

		// ��ǥ ������ ���� ���� �� ȣ��Ǵ� �Լ� (InDIsconnect �ȿ��� ȣ��)
		//
		// parameter : ����Ű
		// return : ����
		virtual void OnDisconnect(ULONGLONG SessionID);

		// ��Ŷ ���� �Ϸ� �� ȣ��Ǵ� �Լ�.
		//
		// parameter : ���� ����Ű, CProtocolBuff_Lan*
		// return : ����
		virtual void OnRecv(ULONGLONG SessionID, CProtocolBuff_Lan* Payload);

		// ��Ŷ �۽� �Ϸ� �� ȣ��Ǵ� �Լ�
		//
		// parameter : ���� ����Ű, Send �� ������
		// return : ����
		virtual void OnSend(ULONGLONG SessionID, DWORD SendSize);

		// ��Ŀ �����尡 ��� �� ȣ��Ǵ� �Լ�.
		// GQCS �ٷ� �ϴܿ��� ȣ��
		// 
		// parameter : ����
		// return : ����
		virtual void OnWorkerThreadBegin();

		// ��Ŀ �����尡 ���� �� ȣ��Ǵ� �Լ�
		// GQCS �ٷ� ������ ȣ��
		// 
		// parameter : ����
		// return : ����
		virtual void OnWorkerThreadEnd();

		// ���� �߻� �� ȣ��Ǵ� �Լ�.
		//
		// parameter : ���� �ڵ�(���� ������ �����ڵ�� WinGetLastError() �Լ��� ��� ����. ���� ��� 0�� ���ϵ�)
		//			 : ���� �ڵ忡 ���� ��Ʈ��
		// return : ����
		virtual void OnError(int error, const TCHAR* errorStr);

	};
}

// ---------------------------
//
// ä�ü����� ��Ŭ��� ����Ǵ� Lan ����
//
// ---------------------------
namespace Library_Jingyu
{
	class CBattle_Chat_LanServer	:public CLanServer
	{
		friend class CBattle_Master_LanClient;
		friend class CBattleServer_Room;



		// ------------
		// ��� ����
		// ------------

		// ������ �������� ����Ǵ� ��Ŭ��
		CBattle_Master_LanClient* m_pMasterClient;

		// ��Ʋ Net����
		CBattleServer_Room* m_BattleServer;

		// ������ ����.
		// �ʱⰪ�� 0xffffffffffffffff
		ULONGLONG m_ullSessionID;
		
		// �α��� ����.
		// �α��� ��Ŷ�� ������ true�� ����
		bool m_bLoginCheck;

		// ä�� �� Ŭ�󿡰� ��Ŷ�� �ϳ� ���� �� 1�� �����Ǵ� ��.
		UINT m_uiReqSequence;

		// ��ū �߱� �ð� �����ϱ�
		// timeGetTime() �Լ��� ���ϰ�.
		// �и� ������ ����
		DWORD m_dwTokenSendTime;



	private:
		// -----------------------
		// Battle Net ������ ȣ���ϴ� �Լ�
		// -----------------------

		// ä�� ��Ŭ�󿡰�, �ű� ���� ���� ��Ŷ ������
		//
		// Parameter : stRoom*
		// return : ����
		void Packet_NewRoomCreate_Req(CBattleServer_Room::stRoom* NewRoom);

		// ä�� ��Ŭ�󿡰�, ��ū �߱�
		//
		// Parameter : ����
		// return : ����
		void Packet_TokenChange_Req();


	public:
		// ------------------------
		// �ܺο��� ȣ�� ������ �Լ�
		// ------------------------

		// ���� ����
		//
		// Parameter : IP, Port, ���� ��Ŀ, Ȱ��ȭ ��Ŀ, ���� ����Ʈ ������, �������, �ִ� ������ ��
		// return : ���� �� false
		bool ServerStart(TCHAR* IP, int Port, int CreateWorker, int ActiveWorker, int CreateAccept, int Nodelay, int MaxUser);

		// ���� ����
		//
		// Parameter : ����
		// return : ����
		void ServerStop();


	private:
		// -----------------------
		// ��Ŷ ó�� �Լ�
		// -----------------------

	
		// �ű� ���� ���� ��Ŷ ����.
		// �� �ȿ��� �����Ϳ��Ե� �����ش�.
		//
		// Parameter : SessinID, CProtocolBuff_Lan*
		// return : ����
		void Packet_NewRoomCreate_Res(ULONGLONG SessionID, CProtocolBuff_Lan* Packet);

		// �� ������ ���� ����
		//
		// Parameter : SessinID, CProtocolBuff_Lan*
		// return : ����
		void Packet_RoomClose_Res(ULONGLONG SessionID, CProtocolBuff_Lan* Packet);


		// ��ū ����࿡ ���� ����
		// �� �ȿ��� �����Ϳ��Ե� �����ش�.
		//
		// Parameter : SessionID, CProtocolBuff_Lan*
		// return : ����
		void Packet_TokenChange_Res(ULONGLONG SessionID, CProtocolBuff_Lan* Packet);

		// �α��� ��û
		//
		// Parameter : SessinID, CProtocolBuff_Lan*
		// return : ����
		void Packet_Login(ULONGLONG SessionID, CProtocolBuff_Lan* Packet);



	private:
		// --------------------------
		// ���ο����� ����ϴ� �Լ�
		// --------------------------

		// ������ �� Ŭ��, ��Ʋ Net ���� ����
		//
		// Parameter : CBattle_Master_LanClient*
		// return : ����
		void SetMasterClient(CBattle_Master_LanClient* SetPoint, CBattleServer_Room* SetPoint2);



	private:
		// -----------------------
		// �����Լ�
		// -----------------------

		// Accept ����, ȣ��ȴ�.
		//
		// parameter : ������ ������ IP, Port
		// return false : Ŭ���̾�Ʈ ���� �ź�
		// return true : ���� ���
		virtual bool OnConnectionRequest(TCHAR* IP, USHORT port);

		// ���� �� ȣ��Ǵ� �Լ� (AcceptThread���� Accept �� ȣ��)
		//
		// parameter : ������ �������� �Ҵ�� ����Ű
		// return : ����
		virtual void OnClientJoin(ULONGLONG SessionID);

		// ���� ���� �� ȣ��Ǵ� �Լ� (InDIsconnect �ȿ��� ȣ��)
		//
		// parameter : ���� ����Ű
		// return : ����
		virtual void OnClientLeave(ULONGLONG SessionID);

		// ��Ŷ ���� �Ϸ� �� ȣ��Ǵ� �Լ�.
		//
		// parameter : ���� ����Ű, ���� ��Ŷ
		// return : ����
		virtual void OnRecv(ULONGLONG SessionID, CProtocolBuff_Lan* Payload);

		// ��Ŷ �۽� �Ϸ� �� ȣ��Ǵ� �Լ�
		//
		// parameter : ���� ����Ű, Send �� ������
		// return : ����
		virtual void OnSend(ULONGLONG SessionID, DWORD SendSize);

		// ��Ŀ �����尡 ��� �� ȣ��Ǵ� �Լ�.
		// GQCS �ٷ� �ϴܿ��� ȣ��
		// 
		// parameter : ����
		// return : ����
		virtual void OnWorkerThreadBegin();

		// ��Ŀ �����尡 ���� �� ȣ��Ǵ� �Լ�
		// GQCS �ٷ� ������ ȣ��
		// 
		// parameter : ����
		// return : ����
		virtual void OnWorkerThreadEnd();

		// ���� �߻� �� ȣ��Ǵ� �Լ�.
		//
		// parameter : ���� �ڵ�(���� ������ �����ڵ�� WinGetLastError() �Լ��� ��� ����. ���� ��� 0�� ���ϵ�)
		//			 : ���� �ڵ忡 ���� ��Ʈ��
		// return : ����
		virtual void OnError(int error, const TCHAR* errorStr);


	public:
		// ---------------
		// �����ڿ� �Ҹ���
		// ----------------

		// ������
		CBattle_Chat_LanServer();

		// �Ҹ���
		virtual ~CBattle_Chat_LanServer();

	};
}

#endif // !__BATTLESERVER_ROOM_VERSION_H__
