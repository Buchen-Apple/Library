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

			// �� ���� ���� ��Ŷ�� �޾Ҵ��� üũ. true�� ����
			bool m_bBattleRoomEnterCheck;

			// ������ �����κ��� �� ������, ���������� ���� ����. true�� ����
			bool m_bRoomInfoOK;

			stPlayer()
			{
				m_bLoginCheck = false;	// ���� �����ÿ��� flase�� ����
				m_bBattleRoomEnterCheck = false;	// ���� �����ÿ��� flase�� ����
				m_bRoomInfoOK = false;
			}
		};
		

		// ------------
		// ��� ����
		// ------------

		// �Ľ̿� ����
		stConfigFile m_stConfig;	

		// HTTP�� ����ϴ� ����
		HTTP_Exchange* m_HTTP_Post;

		// DB_Connector TLS ����
		// MatchmakingDB�� ����
		CBConnectorTLS* m_MatchDBcon;

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
			  

		// ------------
		// ��¿�
		// ------------

		// �α��� ��Ŷ �޾��� ��, ����.
		LONG m_lTokenError;
		LONG m_lAccountError;
		LONG m_lTempError;
		LONG m_lVerError;

		// �α��ο� ������ ���� ��
		LONG m_lLoginUser;

		// �÷��̾� ����ü �Ҵ� ��
		LONG m_lstPlayer_AllocCount;

		// ��Ʋ �� ���� ���� ��Ŷ�� �Ⱥ����� ���� Ŭ���̾�Ʈ�� ��
		LONG m_lNot_BattleRoom_Enter;




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

		// ��ġ����ŷ DB��, �ʱ� ������ Insert�ϴ� �Լ�.
		// �̹�, �����Ͱ� �����ϴ� ���, ������ Update�Ѵ�.
		// 
		// Parameter : ����
		// return : ����
		void ServerInfo_DBInsert();

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

#endif // !__MATCH_MAKING_SERVER_H__
