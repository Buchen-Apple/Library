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

			// CBattleServer_Room�� ������
			CBattleServer_Room* m_pParent;

			// ClientKey�� �ʱ� ��
			const INT64 m_i64Default_CK = -1;


			
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

			// �뿡 ������ �ο� ��
			int m_iJoinUserCount;

			// �� �ִ� �ο� ��. 
			// �� �ο��� �Ǹ� �� �������� �Ǵ�
			const int m_iMaxJoinCount = 5;

			// �� ���� ��ū (��Ʋ���� ���� ��ū���� �ٸ�)
			char m_cEnterToken[32];
		};

		enum eu_DB_READ_TYPE
		{
			// �α��� ��Ŷ�� ���� ó��
			eu_LOGIN	= 0			
		};





		// -----------------------
		// ��� ����
		// -----------------------

		// ����. CMMOServer�� ����.
		CGameSession* m_cGameSession;

		// Config ����
		stConfigFile m_stConfig;

		// ����͸� Ŭ��
		CGame_MinitorClient* m_Monitor_LanClient;

		// ���� �ڵ�
		// Ŭ�� ���� ���� ��. �Ľ����� �о��.
		int m_uiVer_Code;

		// shDB�� ����ϴ� ����
		shDB_Communicate m_shDB_Communicate;	


		

		// -----------------------
		// ��ū���� ����
		// -----------------------

		// ���� ��ū ����
		// ��Ʋ������ �� 2�� �� �ϳ��� ��ġ�Ѵٸ� �´ٰ� �����Ų��.

		// ��Ʋ���� ���� ��ū 1��
		// "����" ��ū�� �����Ѵ�.
		// �����Ϳ� ����� �� Ŭ�� ���� �� �����Ѵ�.
		char cConnectToken_Now[32];

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

		// �ִ� ������ �� �ִ� �� ��
		// ���� ��
		const int m_iMaxTotalRoomCount = 500;

		// �ִ� ������ �� �ִ� ���� ��
		// ���� ��
		const int m_iMaxWaitRoomCount = 200;


		// ���� ���� �� (5/5�� ���� ���� ��)
		int m_iNowWaitRoomCount;

		// ���� ���� ���� ������� �� �� (��� �� ���ļ�)
		int m_iNowTotalRoomCount;

		// �� ���� ��ū
		// �̸� 64������ ����� �ΰ�, ����� �� �������� ������.
		char m_cRoomEnterToken[64][32];

		// stRoom ���� Pool
		CMemoryPoolTLS<stRoom> *m_Room_Pool;

		

		


		// -----------------------
		// �� ���� �ڷᱸ�� ����
		// -----------------------

		// �� ���� umap
		//
		// Key : RoomNo, Value : stRoom*
		unordered_map<int, stRoom*> m_Room_Umap;






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

		// AccountNo �ڷᱸ������ ������ �˻��ϴ� �Լ�
		//
		// Parameter : AccountNo
		// return : ���� �� ClientKey
		//		  : ���� �� nullptr
		CGameSession* FindAccountNoFunc(INT64 AccountNo);

		// AccountNo �ڷᱸ������ ������ �����ϴ� �Լ�
		//
		// Parameter : AccountNo
		// return : ���� �� true
		//		  : ���� �� false
		bool EraseAccountNoFunc(INT64 AccountNo);


	protected:
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
