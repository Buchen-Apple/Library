#ifndef __MATCH_MAKING_SERVER_H__
#define __MATCH_MAKING_SERVER_H__

#include "NetworkLib/NetworkLib_NetServer.h"
#include "NetworkLib/NetworkLib_LanClinet.h"
#include "DB_Connector/DB_Connector.h"
#include "Http_Exchange/HTTP_Exchange.h"

#include <unordered_map>

using namespace std;

// ---------------------------------------------
// 
// ��ġ����ŷ Net����
//
// ---------------------------------------------
namespace Library_Jingyu
{
	class Matchmaking_Net_Server :public CNetServer
	{
		// LanŬ��� friend����
		friend class Matchmaking_Lan_Client;
		friend class Monitor_Lan_Clinet;

		// ------------
		// ���� ����ü
		// ------------
		// ���Ͽ��� �о���� �� ����ü
		struct stConfigFile
		{
			// ��ġ����ŷ Net���� ����
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
			int HeartBeat;

			// ��ġ����ŷ DB ����
			TCHAR DB_IP[20];
			TCHAR DB_User[40];
			TCHAR DB_Password[40];
			TCHAR DB_Name[40];
			int  DB_Port;
			int MatchDBHeartbeat;	// ��ġ����ŷ DB�� �� �и������帶�� ��Ʈ��Ʈ�� �� ���ΰ�.			

			// ��ġ����ŷ LanClient ���� (�����Ϳ� ����)
			TCHAR MasterServerIP[20];
			int MasterServerPort;
			int ClientCreateWorker;
			int ClientActiveWorker;
			int ClientNodelay;

			// ����͸� LanClient ���� (����͸� ������ ����)
			TCHAR MonitorServerIP[20];
			int MonitorServerPort;
			int MonitorClientCreateWorker;
			int MonitorClientActiveWorker;
			int MonitorClientNodelay;

		};

		// ���� ������ ����ü
		struct stPlayer
		{
			// ��ū �ƴ�. Net������ ��ſ� ���Ǵ� Ű.
			ULONGLONG m_ullSessionID;

			// �ش� ������ AccountNo
			INT64 m_i64AccountNo;

			// ������ Ŭ���̾�Ʈ�� �����ϰ� üũ�ϱ� ����. ������ �������� ���
			UINT64	m_ui64ClientKey;					

			// ���� �α��� ��Ŷ�� �޾Ҵ��� üũ. true�� ����. false�� �ȹ���
			bool m_bLoginCheck;			

			// ��Ʋ���� �� ���� ���� ��Ŷ�� ���� �������� üũ. true�� ��������.
			bool m_bBattleRoomEnterCheck;

			// ������ ������ �� ���� ��û�� ���� ����. true�� ���� ����.
			bool m_bSendMaster_RoomInfo;

			// ���������� ��Ŷ�� ���� �ð� ���� ����
			DWORD m_dwLastPacketTime;

			stPlayer()
			{
				m_i64AccountNo = -1;	// ���� �����ÿ��� -1�� ����
				m_bLoginCheck = false;	// ���� �����ÿ��� flase�� ����
				m_bBattleRoomEnterCheck = false;	// ���� �����ÿ��� flase�� ����
				m_bSendMaster_RoomInfo = false;
			}
		};
		

		// ------------
		// ��� ����
		// ------------

		// �Ľ̿� ����
		stConfigFile m_stConfig;	

		// HTTP�� ����ϴ� ����
		HTTP_Exchange* m_HTTP_Post;

		// �ش� ��Ī������ No.
		// �Ľ����� �о�´�.
		int m_iServerNo;

		// ������ ������ ��� ��ū
		// �Ľ����� �о�´�.
		char MasterToken[32];

		// ClinetKey�� ����� ���� ����.
		// ���� 1���� ������ �� ���� ++�Ѵ�.
		// 
		// �ʱ� �� : 0 (�����ڿ��� ����)
		UINT64 m_ClientKeyAdd;

		// ���� �ڵ�
		// Ŭ�� ���� ���� ��. �Ľ����� �о��.
		int m_uiVer_Code;

		// �ش� PC�� ���� IP
		char m_cServerIP[20];

		// ��ġ����ŷ DB��, �� ���� ��ȭ�� ���� �� ���� ������ ���ΰ�.
		int m_iMatchDBConnectUserChange; 

		// !! ������ lan������ ����ϴ� lanŬ�� !!
		Matchmaking_Lan_Client* m_pLanClient;

		// !! ����͸� lan ������ ����ϴ� lan Ŭ�� !!
		Monitor_Lan_Clinet* m_pMonitorLanClient;



		// -----------------------------------

		// DB ��Ʈ��Ʈ�� �� ��, �ش� �� ���� +-100�̸� ���.
		// ���������� ���� ���۵� ������, �ο��� üũ�� �Ѵ�.
		size_t m_ChangeConnectUser;

		// m_ChangeConnectUser�� �����ϴ� LOCK
		SRWLOCK m_srwlChangeConnectUser;

		// -----------------------------------



		// -----------------------------------

		// DBHeartbeatThread�� �ڵ�
		HANDLE hDB_HBThread;

		// DBHeartbeatThread ����� �̺�Ʈ
		HANDLE hDB_HBThread_ExitEvent;

		// DBHeartbeatThread���� �� ��Ű��� �̺�Ʈ
		HANDLE hDB_HBThread_WorkerEvent;
		
		// -----------------------------------

		// HeartbeatThread�� �ڵ�
		HANDLE m_hHBThread;

		// HeartbeatThread ����� �̺�Ʈ
		HANDLE m_hHBThreadExitEvent;

		// -----------------------------------
		


		// -----------------------------------

		// ������ �������� �����ϴ� �ڷᱸ��
		// umap���� ���� 
		//
		// ���� : ���԰� ������ ����ϱ� ������, �Ϲ� map(Red Black Tree)�� ����ϸ� ���� ������ ���� ������尡 ũ��.
		// ������, O(1)�� �ؽ��� ����ϴ� umap ���
		//
		// ���� : Net�� SessionKey, stPlayer*
		unordered_map<ULONGLONG, stPlayer*>	m_umapPlayer;

		// ClientKey�� �̿��� ������ �������� �����ϴ� �ڷᱸ��
		// umap���� ���� 
		//
		// �÷��̾� ���� �ڷᱸ���� 2�� ����ϴ� ���� : ������ ������ Lan��� ��, ClientKey�� �̿��� ����Ѵ�.
		// ������ LanServer���� ������ ����, ClinetKey�� �̿��� ������ ã�Ƴ��� �Ѵ�. �� �뵵.
		//
		// ���� : ClientKey, stPlayer*
		unordered_map<UINT64, stPlayer*>	m_umapPlayer_ClientKey;

		// m_umapPlayer�� m_umapPlayer_ClientKey�� �����ϴ� LOCK
		SRWLOCK m_srwlPlayer;

		// stPlayer ����ü�� �ٷ�� TLS
		CMemoryPoolTLS<stPlayer> *m_PlayerPool;

		// -----------------------------------


		// -----------------------------------

		// �α��� �� ������ �����ϴ� �ڷᱸ��
		// umap
		//
		// Key : AccountNo / Value : SessionID
		unordered_map<UINT64, ULONGLONG> m_umapLoginPlayer;

		// m_umapLoginPlayer�� �����ϴ� LOCK
		SRWLOCK m_srwlLoginPlayer;

		// -----------------------------------
			  

		// ------------
		// ��¿�
		// ------------

		// �α��� ��Ŷ �޾��� ��, ����.
		LONG m_lTokenError;
		LONG m_lAccountError;
		LONG m_lTempError;
		LONG m_lVerError;
		LONG m_lOverlapError;

		// �α��ο� ������ ���� ��
		LONG m_lLoginUser;

		// �÷��̾� ����ü �Ҵ� ��
		LONG m_lstPlayer_AllocCount;

		// ��Ʋ �� ���� ���� ��Ŷ�� �Ⱥ����� ���� Ŭ���̾�Ʈ�� ��
		LONG m_lNot_BattleRoom_Enter;

		// �� ������ ������ ���� ��(�ʴ�)
		LONG m_lRoomEnter_OK;

		// ��Ʈ��Ʈ�� ���� ����
		LONG m_lHeartBeatCount;



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

		// ClientKey ����� �Լ�
		//
		// Parameter : ����
		// return : ClientKey(UINT64)
		UINT64 CreateClientKey();

		// ������ ��� �������� shutdown �ϴ� �Լ�
		//
		// Parameter : ����
		// return : ����
		void AllShutdown();



	private:
		// -------------------------------------
		// ������
		// -------------------------------------

		// matchmakingDB�� ���� �ð����� ��Ʈ��Ʈ�� ��� ������.
		static UINT WINAPI DBHeartbeatThread(LPVOID lParam);

		// ������ ��Ʈ��Ʈ�� üũ�ϴ� ������
		static UINT WINAPI HeartbeatThread(LPVOID lParam);



	private:
		// -------------------------------------
		// �ڷᱸ�� �߰�,����,�˻��� �Լ�
		// -------------------------------------

		// Player ���� �ڷᱸ�� "2��"��, ���� �߰�
		// ���� umap���� ������
		// 
		// Parameter : SessionID, ClientKey, stPlayer*
		// return : �߰� ���� ��, true
		//		  : SessionID�� �ߺ��� ��(�̹� �������� ����) false
		bool InsertPlayerFunc(ULONGLONG SessionID, UINT64 ClientKey, stPlayer* insertPlayer);

		// Player ���� �ڷᱸ������, ���� �˻�
		// !!SessionID!! �� �̿��� �˻�
		// ���� umap���� ������
		// 
		// Parameter : SessionID
		// return : �˻� ���� ��, stPalyer*
		//		  : �˻� ���� �� nullptr
		stPlayer* FindPlayerFunc(ULONGLONG SessionID);

		// Player ���� �ڷᱸ������, ���� �˻�
		// !!ClientKey!! �� �̿��� �˻�
		// ���� umap���� ������
		// 
		// Parameter : ClientKey
		// return : �˻� ���� ��, stPalyer*
		//		  : �˻� ���� �� nullptr
		stPlayer* FindPlayerFunc_ClientKey(UINT64 ClientKey);

		// Player ���� �ڷᱸ�� "2��"����, ���� ���� (�˻� �� ����)
		// ���� uumap���� ������
		// 
		// Parameter : SessionID
		// return : ���� ��, ���ŵ� ���� stPalyer*
		//		  : �˻� ���� ��(���������� ���� ����) nullptr
		stPlayer* ErasePlayerFunc(ULONGLONG SessionID);



	private:
		// -------------------------------------
		// �α����� ���� ���� �ڷᱸ��
		// -------------------------------------

		// �α��� ���� ���� �ڷᱸ���� �߰�
		//
		// Parameter : AccountNo, SessionID
		// return : ���� �� true / ���� �� false
		bool InsertLoginPlayerFunc(INT64 AccountNo, ULONGLONG SessionID);

		// �α��� ���� ���� �ڷᱸ������ �˻�
		//
		// Parameter : AccountNo, (out)SessionID
		// return : ���� �� true�� �Բ� ���ڷ� ���� SessionID�� ä����
		//		  : ���� �� false�� �Բ� ���ڷ� ���� SessionID ä���� ����
		bool FindLoginPlayerFunc(INT64 AccountNo, ULONGLONG* SessionID);

		// �α��� �� ���� ���� �ڷᱸ������ ���� ����
		//
		// Parameter : AccountNo
		// return : ���� ���� �� true / ���� �� false
		bool EraseLoginPlayerFunc(INT64 AccountNo);



	private:
		// -------------------------------------
		// ��Ŷ ó���� �Լ�
		// -------------------------------------

		// Ŭ���� �α��� ��û ����
		//
		// Parameter : SessionID, Payload
		// return : ����. ������ �����, ���ο��� throw ����
		void Packet_Match_Login(ULONGLONG SessionID, CProtocolBuff_Net* Payload);

		// �� ���� ����
		// �����Ϳ��� �� ���� ���� ��Ŷ ����
		//
		// Parameter : SessionID, Payload
		// return : ����. ������ �����, ���ο��� throw ����
		void Packet_Battle_EnterOK(ULONGLONG SessionID, CProtocolBuff_Net* Payload);

		// �� ���� ����
		// �����Ϳ��� �� ���� ���� ��Ŷ ����
		//
		// Parameter : ClinetKey
		// return : ����
		void Packet_Battle_EnterFail(UINT64 ClientKey);

	public:
		// -------------------------------------
		// �ܺο��� ��� ������ �Լ�
		// -------------------------------------

		// ��ġ����ŷ ���� Start �Լ�
		//
		// Parameter : ����
		// return : ���� �� false ����
		bool ServerStart();

		// ��ġ����ŷ ���� ���� �Լ�
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
		// -------------------------------------
		// NetServer�� �����Լ�
		// -------------------------------------

		virtual bool OnConnectionRequest(TCHAR* IP, USHORT port);

		virtual void OnClientJoin(ULONGLONG SessionID);

		virtual void OnClientLeave(ULONGLONG SessionID);

		virtual void OnRecv(ULONGLONG SessionID, CProtocolBuff_Net* Payload);

		virtual void OnSend(ULONGLONG SessionID, DWORD SendSize);

		virtual void OnWorkerThreadBegin();

		virtual void OnWorkerThreadEnd();

		virtual void OnError(int error, const TCHAR* errorStr);

		virtual void OnSemaphore(ULONGLONG SessionID);



	public:	
		// -------------------------------------
		// �����ڿ� �Ҹ���
		// -------------------------------------

		// ������
		Matchmaking_Net_Server();

		// �Ҹ���
		virtual ~Matchmaking_Net_Server();

	};
}


// ---------------------------------------------
// 
// ��ġ����ŷ LanClient(Master������ LanServer�� ���)
//
// ---------------------------------------------
namespace Library_Jingyu
{
	class Matchmaking_Lan_Client :public CLanClient
	{
		// Net������ friend����
		friend class Matchmaking_Net_Server;


		// ------------
		// ��� ����
		// ------------

		// !! ��Ī Net ���� !!
		Matchmaking_Net_Server* m_pParent;

		// ������ ������ ����� SessionID
		ULONGLONG m_ullClientID;

		// �α��� ����. �α��� ��Ŷ�� ���������� ������ true�� ����
		// false�� �α��� �ȵ� ����
		bool m_bLoginCheck;

	private:
		// -------------------------------------
		// Net ������ ȣ���ϴ� �Լ�
		// -------------------------------------

		// �� ���� ��û
		// �����Ϳ��� ��Ŷ ����
		//
		// Parameter : SessionID
		// return : ����. ������ �����, ���ο��� throw ����
		void Packet_Battle_Info(ULONGLONG SessionID);


	private:
		// -------------------------------------
		// �����Ϳ��� ���� ��Ŷ ó���� �Լ�
		// -------------------------------------

		// �� ���� ��û�� ���� ����
		// ���ο���, Net������ SendPacket()���� ȣ���Ѵ�.
		// 
		// Parameter : CProtocolBuff_Lan*
		// return : ����. ������ �����, ���ο��� throw ����
		void Response_Battle_Info(CProtocolBuff_Lan* payload);

		// �α��� ��û�� ���� ����
		// 
		// Parameter : CProtocolBuff_Lan*
		// return : ����. ������ �����, ���ο��� throw ����
		void Response_Login(CProtocolBuff_Lan* payload);


	public:
		// -------------------------------------
		// �ܺο��� ��� ������ �Լ�
		// -------------------------------------

		// ��ġ����ŷ LanClient ���� �Լ�
		//
		// Parameter : ����
		// return : ���� �� false ����
		bool ClientStart();

		// ��ġ����ŷ LanClient ���� �Լ�
		//
		// Parameter : ����
		// return : ����
		void ClientStop();


	protected:
		// -------------------------------------
		// ��� ���迡���� ��� ������ ��� �Լ�
		// -------------------------------------
		
		// �� �θ� ä���ִ� �Լ�
		// 
		// Parameter : Net������ this
		// return : ����
		void SetParent(Matchmaking_Net_Server* NetServer);


	private:
		// -----------------------
		// Lan Ŭ���� ���� �Լ�
		// -----------------------

		void OnConnect(ULONGLONG ClinetID);

		void OnDisconnect(ULONGLONG ClinetID);

		void OnRecv(ULONGLONG ClinetID, CProtocolBuff_Lan* Payload);

		void OnSend(ULONGLONG ClinetID, DWORD SendSize);

		void OnWorkerThreadBegin();

		void OnWorkerThreadEnd();

		void OnError(int error, const TCHAR* errorStr);



	public:
		// -------------------------------------
		// �����ڿ� �Ҹ���
		// -------------------------------------

		// ������
		Matchmaking_Lan_Client();

		// �Ҹ���
		virtual ~Matchmaking_Lan_Client();	
	};
}


// ---------------------------------------------
// 
// ����͸� LanClient
//
// ---------------------------------------------
namespace Library_Jingyu
{
	class Monitor_Lan_Clinet	:public CLanClient
	{
		// Net������ Friend ����
		friend class Matchmaking_Net_Server;

		// ������ ������ ��Ƶα�
		enum en_MonitorClient
		{
			dfSERVER_NO = 4	// ��ġ����ŷ ������ 4���̴�
		};

		// -------------
		// ��� ����
		// -------------

		// ����͸� ������ ���� ������ �������� �ڵ�.
		HANDLE m_hMonitorThread;

		// ����͸� ������ �����ų �̺�Ʈ
		HANDLE m_hMonitorThreadExitEvent;

		// ���� ����͸� ������ ����� ���� ID
		ULONGLONG m_ullSessionID;


		// -------------
		// !! ��Ī������ this !!
		// -------------
		Matchmaking_Net_Server* m_MatchServer_this;

	private:
		// ----------------------------
		// ���ο����� ����ϴ� ��� �Լ�
		// ----------------------------

		// ���� �ð����� ����͸� ������ ������ �����ϴ� ������
		static UINT	WINAPI MonitorThread(LPVOID lParam);

		// ����͸� ������ ������ ����
		//
		// Parameter : DataType(BYTE), DataValue(int), TimeStamp(int)
		// return : ����
		void InfoSend(BYTE DataType, int DataValue, int TimeStamp);

	public:

		// -----------------------
		// �ܺο��� ��� ������ �Լ�
		// -----------------------

		// ���� �Լ�
		// ����������, ��ӹ��� CLanClient�� Startȣ��.
		//
		// Parameter : ����
		// return : ���� �� true , ���� �� falsel 
		bool ClientStart();

		// ���� �Լ�
		// ����������, ��ӹ��� CLanClient�� Stopȣ��.
		// �߰���, ���ҽ� ���� ��
		//
		// Parameter : ����
		// return : ����
		void ClientStop();

		// ��Ī������ this�� �Է¹޴� �Լ�
		// 
		// Parameter : �� ������ this
		// return : ����
		void SetParent(Matchmaking_Net_Server* ChatThis);


	private:
		// -----------
		// ���� �Լ�
		// -----------

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
		// �����ڿ� �Ҹ���
		Monitor_Lan_Clinet();
		virtual ~Monitor_Lan_Clinet();
	};
}

#endif // !__MATCH_MAKING_SERVER_H__
