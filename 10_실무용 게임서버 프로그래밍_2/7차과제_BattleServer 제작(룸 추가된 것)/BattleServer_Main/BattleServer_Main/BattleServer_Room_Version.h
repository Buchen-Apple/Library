#ifndef __BATTLESERVER_ROOM_VERSION_H__
#define __BATTLESERVER_ROOM_VERSION_H__

#include "NetworkLib/NetworkLib_MMOServer.h"
#include "NetworkLib/NetworkLib_LanClinet.h"

#include "Http_Exchange/HTTP_Exchange.h"
#include "shDB_Communicate.h"

#include <unordered_set>
#include <unordered_map>

using namespace std;

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



		// -----------------------
		// �̳� Ŭ����
		// -----------------------

		// CMMOServer�� cSession�� ��ӹ޴� ���� Ŭ����
		class CGameSession :public CMMOServer::cSession
		{
			friend class CBattleServer_Room;	

			// -----------------------
			// ��� ����
			// -----------------------

			// ȸ�� ��ȣ
			INT64 m_Int64AccountNo;

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
			// true�� ����Ű���� ���������� ó����
			bool m_bLoginFlag;


			
		private:
			// -----------------------
			// �����ڿ� �Ҹ���
			// -----------------------

			CGameSession();
			virtual ~CGameSession();


		private:
			// -----------------
			// �����Լ�
			// -----------------

			// Auth �����忡�� ó��
			virtual void OnAuth_ClientJoin();
			virtual void OnAuth_ClientLeave(bool bGame = false);
			virtual void OnAuth_Packet(CProtocolBuff_Net* Packet);

			// Game �����忡�� ó��
			virtual void OnGame_ClientJoin();
			virtual void OnGame_ClientLeave();
			virtual void OnGame_Packet(CProtocolBuff_Net* Packet);

			// Release��
			virtual void OnGame_ClientRelease();


			// -----------------
			// ��Ŷ ó�� �Լ�
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

		};

		friend class CGameSession;

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
			int XORCode1;
			int XORCode2;
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
		};
				
		// �� ����ü
		struct stRoom
		{
			// �� ��ȣ
			int m_iRoomNo;

			// ���� ����
			// 0 : ����
			// 1 : �غ��
			// 2 : �÷��� ��
			int m_iRoomState;

			// ���� �� �ȿ� �ִ� ���� ��
			int m_iJoinUserCount;			

			// Game��� ������ ���� ��
			int m_iGameModeUserCount;

			// �ش� ���� ������ Play������.
			// ���� ��� ������ Game��尡 �Ǿ� ������ ���� �� ���
			// �� ���ʹ� �ٸ�
			bool m_bGameStart;	

			// ī��Ʈ �ٿ�. �и������� ����
			// timeGetTime()�� ���� ����.
			DWORD m_dwCountDown;

			// �� ���� ��ū (��Ʋ���� ���� ��ū���� �ٸ�)
			char m_cEnterToken[32];


			// ------------

			// �濡 ������ ������ ���� �ڷᱸ��.
			vector<CGameSession*> m_JoinUser_Vector;
					   
			// ���� ������ �ִ� �ο� ��. ���� ��
			const int m_iMaxJoinCount = 5;

			// ������
			stRoom()
			{
				// �̸� �޸� ���� ��Ƶα�
				m_JoinUser_Vector.reserve(10);
			}

			// �ڷᱸ���� Insert
			//
			// Parameter : �߰��ϰ��� �ϴ� CGameSession*
			// return : ����
			void Insert(CGameSession* InsertPlayer)
			{
				m_JoinUser_Vector.push_back(InsertPlayer);
			}

			// �ڷᱸ������ Erase
			//
			// Parameter : �����ϰ��� �ϴ� CGameSession*
			// return : ���� �� true
			//		  : ���� ��  false
			bool Erase(CGameSession* InsertPlayer)
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

			// �ڷᱸ�� ���� ��� �������� ���ڷ� ���� ��Ŷ ������
			//
			// Parameter : CProtocolBuff_Net*
			// return : ���� �� true
			//		  : ������ 0���� �� false.
			bool SendPacket_BroadCast(CProtocolBuff_Net* SendBuff)
			{
				// 1. �ڷᱸ�� ���� ���� �� �ޱ�
				size_t Size = m_JoinUser_Vector.size();

				// 2. ���� ���� 0���̰ų� ���� ���, �������� return false
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

		// DB ��û�� ���� ��ó���� ����, Type
		enum eu_DB_READ_TYPE
		{
			// �α��� ��Ŷ�� ���� ó��
			eu_LOGIN	= 0			
		};
		
		// ���� ���� �Ұ����� Config ���� ���� ����ü
		struct stCONFIG
		{
			// Auth_Update���� �� �����ӿ� ó���� HTTP��� ��ó��
			// ���� ��
			const int m_iHTTP_MAX = 50;

			// �ִ� ������ �� �ִ� �� ��
			// ���� ��
			const LONG m_lMaxTotalRoomCount = 500;

			// �ִ� ������ �� �ִ� ���� ��
			// ���� ��
			const LONG m_lMaxWaitRoomCount = 200;

			// Auth_Update���� �� �����ӿ� ���� ������ �� ��
			// ���� ��
			const int m_iLoopCreateRoomCount = 30;

			// ��ū ��߱� �ð�
			// �и������� ����.
			// ex) 1000�� ���, 1�� ������ ��ū ��߱�.
			const int m_iTokenChangeSlice = 12000;
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

		// ���� �ڵ�
		// Ŭ�� ���� ���� ��. �Ľ����� �о��.
		int m_uiVer_Code;

		// shDB�� ����ϴ� ����
		shDB_Communicate m_shDB_Communicate;	

		// ���� ���� No
		// ������ ������ �Ҵ��� �ش�.
		int m_iServerNo;

		


		

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




		// -----------------------
		// �� ���� ����
		// -----------------------

		// �� ��ȣ�� �Ҵ��� ����
		// ���Ͷ����� ++
		LONG m_lGlobal_RoomNo;
		
		// ���� ���� �� (5/5�� ���� ���� ��)
		LONG m_lNowWaitRoomCount;

		// ���� ���� ���� ������� �� �� (��� �� ���ļ�)
		LONG m_lNowTotalRoomCount;

		// �� ���� ��ū
		// �̸� 64������ ����� �ΰ�, ����� �� �������� ������.
		char m_cRoomEnterToken[64][32];

		// stRoom ���� Pool
		CMemoryPoolTLS<stRoom> *m_Room_Pool;

		

		


		// -----------------------
		// �� ���� �ڷᱸ�� ����
		// -----------------------

		// Auth ��忡 �ִ� �� ���� umap
		//
		// Key : RoomNo, Value : stRoom*
		unordered_map<int, stRoom*> m_Room_Umap;

		// m_AuthRoom_Umap SRW��
		SRWLOCK m_Room_Umap_srwl;






		// -----------------------
		// ���� ���� �ڷᱸ�� ����
		// -----------------------

		// AccountNo�� �̿��� CGameSession* ����
		//
		// ���� ���� ������ AccountNo�� �������� CGameSession* ����
		// Key : AccountNo, Value : CGameSession*
		unordered_map<INT64, CGameSession*> m_AccountNo_Umap;

		// m_AccountNo_Umap�� SRW��
		SRWLOCK m_AccountNo_Umap_srwl;			  



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

		// Start
		// ���������� CMMOServer�� Start, ���� ���ñ��� �Ѵ�.
		//
		// Parameter : ����
		// return : ���� �� false
		bool BattleServerStart();

		// Stop
		// ���������� Stop ����
		//
		// Parameter : ����
		// return : ����
		void BattleServerStop();

		// ��¿� �Լ�
		//
		// Parameter : ����
		// return : ����
		void ShowPrintf();


		

	private:
		// -----------------------
		// ��Ŷ ��ó�� �Լ�
		// -----------------------

		// Login ��Ŷ ��ó��
		//
		// Parameter : DB_WORK_LOGIN*
		// return : ����
		void Auth_LoginPacket_Last(DB_WORK_LOGIN* DBData);




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
		// ---------------------------------
		// Auth����� �� ���� �ڷᱸ�� ����
		// ---------------------------------

		// ���� Room �ڷᱸ���� Insert�ϴ� �Լ�
		//
		// Parameter : RoomNo, stRoom*
		// return : ���� �� true
		//		  : ����(�ߺ� Ű) �� false	
		bool InsertRoomFunc(int RoomNo, stRoom* InsertRoom);

		




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


		// ------------- 
		// ��� ����
		// ------------- 

		// ���� ������ ������ ����� ���� ID
		ULONGLONG m_ullSessionID;

		// ������ �� ������ IP�� Port
		TCHAR m_tcBattleNetServerIP[30];
		int m_iBattleNetServerPort;

		// ä�� �� ������ IP�� Port
		TCHAR m_tcChatNetServerIP[30];
		int m_iChatNetServerPort;

		// ������ ������ ��� ���� ��ū
		char m_cMasterToken[32];

		// ������ �������� ��Ŷ�� �ϳ� ���� �� 1�� �����Ǵ� ��.
		UINT uiReqSequence;

		// ��ū �߱� �ð� �����ϱ�
		// timeGetTime() �Լ��� ���ϰ�.
		// �и� ������ ����
		DWORD m_dwTokenSendTime;



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




	private:
		// -----------------------
		// Battle Net ������ ȣ���ϴ� �Լ�
		// -----------------------

		// �����Ϳ���, �ű� ���� ���� ��Ŷ ������
		//
		// Parameter : ��Ʋ���� No, stRoom*
		// return : ����
		void Packet_NewRoomCreate_Req(int BattleServerNo, CBattleServer_Room::stRoom* NewRoom);

		// ��ū ��߱� �Լ�
		//
		// Parameter : ����
		// return : ����
		void Packet_TokenChange_Req();

		// �����Ϳ���, �� ���� ��Ŷ ������
		//
		// Parameter : RoomNo
		// return : ����
		void Packet_RoomClose(int RoomNo);


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
			dfSERVER_NO = 2	// ��Ʋ������ 2���̴�
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



#endif // !__BATTLESERVER_ROOM_VERSION_H__
