#ifndef __LOGIN_SERVER_H__
#define __LOGIN_SERVER_H__

#include "NetworkLib\NetworkLib_NetServer.h"
#include "NetworkLib\NetworkLib_LanServer.h"
#include "NetworkLib\NetworkLib_LanClinet.h"
#include "CPUUsage\CPUUsage.h"
#include "PDHClass\PDHCheck.h"
#include "DB_Connector\DB_Connector.h"

#include <unordered_map>

using namespace std;

// ----------------------------------
// LoginServer (NetServer)
// ----------------------------------
namespace Library_Jingyu
{
	class CLogin_NetServer :public CNetServer
	{
		friend class CLogin_LanServer;
		friend class CMoniter_Clinet;

		///////////////////////
		// ���� ����ü
		///////////////////////

		// ������ ���� ���� ����ü
		struct stPlayer
		{
			INT64 m_i64AccountNo;
			WCHAR m_wcID[20];
			WCHAR m_wcNickName[20];

			bool m_bLoginCheck;		// �α��� ����. true�� �α��� �� ����.
		};

		// ���Ͽ��� �о���� �� ����ü
		struct stConfigFile
		{
			// �α��� Net���� ����
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

			// ���Ӽ����� ä�ü����� ����.
			TCHAR GameServerIP[16];
			int GameServerPort;

			TCHAR ChatServerIP[16];
			int ChatServerPort;

			// AccountDB�� ���� �� �ʿ��� ����
			TCHAR DB_IP[20];
			TCHAR DB_User[40];
			TCHAR DB_Password[40];
			TCHAR DB_Name[40];
			int  DB_Port;

			// �α��� Lan���� ����
			TCHAR LanBindIP[20];
			int LanPort;
			int LanCreateWorker;
			int LanActiveWorker;
			int LanCreateAccept;
			int LanNodelay;
			int LanMaxJoinUser;
			int LanLogLevel;

			// ����͸� Ŭ�� ����
			TCHAR MonitorServerIP[20];
			int	MonitorServerPort;
			int ClientCreateWorker;
			int ClientActiveWorker;
			int ClientNodelay;
		};

		
	private:

		/////////////////////////////
		// ��� ����
		/////////////////////////////

		stConfigFile m_stConfig;

		// Chat, Game�� LanClient�� ����� LanServer
		CLogin_LanServer* m_cLanS;

		// ����͸� �������� ������ ������ LanClient
		CMoniter_Clinet* m_LanMonitorC;

		// ���� ����ü ���� TLS
		CMemoryPoolTLS< stPlayer >  *m_MPlayerTLS;

		// ������ ���� ���� �ڷᱸ��
		// umap ���
		//
		// Key : Net������ SessionID
		// Value : stPlayer*
		unordered_map<ULONGLONG, stPlayer*> m_umapJoinUser;

		// DB_Connector TLS ����
		//
		// AccountDB�� ����
		CBConnectorTLS* m_AcountDB_Connector;

		// ���� ���� �ڷᱸ���� ���Ǵ� ��
		SRWLOCK srwl;

	private:	

		/////////////////////////////
		// ��� ó�� �Լ�
		/////////////////////////////

		// ���Ͽ��� Config ���� �о����
		// 
		// 
		// Parameter : config ����ü
		// return : ���������� ���� �� true
		//		  : �� �ܿ��� false
		bool SetFile(stConfigFile* pConfig);



		/////////////////////////////
		// �ڷᱸ�� ���� �Լ�
		/////////////////////////////

		// ���� ���� �ڷᱸ������ ���� �˻�
		// ���� umap���� ������
		// 
		// Parameter : SessionID
		// return : �˻� ���� �� stPlayer*
		//		  : ���� �� nullptr
		stPlayer* FindPlayerFunc(ULONGLONG SessionID);

		// ���� ���� �ڷᱸ����, ���� ������ ���� �߰�
		// ���� umap���� ������
		// 
		// Parameter : SessionID, stPlayer*
		// return : �߰� ���� ��, true
		//		  : SessionID�� �ߺ��� �� false
		bool InsertPlayerFunc(ULONGLONG SessionID, stPlayer* InsertPlayer);

		// ���� ���� �ڷᱸ���� �ִ�, ������ ���� ����
		// ���� umap���� ������
		// 
		// Parameter : SessionID, AccountNo, UserID(cahr*), NickName(char*)
		// return : ����
		void SetPlayerFunc(ULONGLONG SessionID, INT64 AccountNo, char* UserID, char* NickName);

		// ���� ���� �ڷᱸ���� �ִ�, ���� ����
		// ���� umap���� ������
		// 
		// Parameter : SessionID
		// return : ���� ��, stPlayer*
		//		  : ���� ��, nullptr
		stPlayer* ErasePlayerFunc(ULONGLONG SessionID);


		/////////////////////////////
		// Lan�� ��Ŷ ó�� �Լ�
		/////////////////////////////		

		// LanClient���� ���� ��Ŷ ó�� �Լ� (����)
		//
		// Parameter : ��Ŷ Ÿ��, AccountNo, SessionID(LanClient���� ���´� Parameter)
		// return : ����
		void LanClientPacketFunc(WORD Type, INT64 AccountNo, ULONGLONG SessionID);


		/////////////////////////////
		// Net�� ��Ŷ ó�� �Լ�
		/////////////////////////////

		// �α��� ��û
		// 
		// Parameter : SessionID, Packet*
		// return : ����
		void LoginPacketFunc(ULONGLONG SessionID, CProtocolBuff_Net* Packet);		
		
		// "����" ��Ŷ ����� ������
		//
		// Parameter : SessionID, AccountNo, Status, UserID, NickName
		// return : ����
		void Success_Packet(ULONGLONG SessionID, INT64 AccountNo, BYTE Status);

		// "����" ��Ŷ ����� ������
		//
		// Parameter : SessionID, AccountNo, Status, UserID, NickName
		// return : ����
		void Fail_Packet(ULONGLONG SessionID, INT64 AccountNo, BYTE Status, char* UserID = nullptr, char* NickName = nullptr);
			   

	public:
		// ��¿� �Լ�
		void ShowPrintf();
	

	public:
		// ������
		CLogin_NetServer();

		// �Ҹ���
		virtual ~CLogin_NetServer();


	public:
		/////////////////////////////
		// �ܺο��� ȣ�� ������ ��� �Լ�
		/////////////////////////////

		// �α��� ���� ���� �Լ�
		// ���������� NetServer�� Start ȣ��
		// 
		// return false : ���� �߻� ��. �����ڵ� ���� �� false ����
		// return true : ����
		bool ServerStart();

		// �α��� ���� ���� �Լ�
		//
		// Parameter : ����
		// return : ����
		void ServerStop();


	private:
		/////////////////////////////
		// �����Լ�
		/////////////////////////////

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
		virtual void OnRecv(ULONGLONG SessionID, CProtocolBuff_Net* Payload);

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


// ----------------------------------
// LoginServer (LanServer)
// ----------------------------------
namespace Library_Jingyu
{	
	class CLogin_LanServer :public CLanServer
	{
		friend class CLogin_NetServer;

		// �� �θ�(CLogin_NetServer)�� ������
		CLogin_NetServer* m_cParentS;

		// ������ ���� ������ �迭
		// SessionID�� ��� �ִ�.
		ULONGLONG m_arrayJoinServer[10];

		// �迭 �ȿ� �ִ� ���� ��
		int m_iArrayCount;

		// �迭�� ��
		SRWLOCK srwl;

	private:
		// �θ� ����
		//
		// Parameter : CLogin_NetServer*
		// return : ����
		void ParentSet(CLogin_NetServer* parent);

		// GameServer, ChatServer�� LanClient�� ���ο� ���� ���� �˸�
		// --- CLogin_NetServer�� "OnRecv"���� ȣ�� ---
		// 
		// Parameter : AccountNo, Token(64Byte), ULONGLONG Parameter
		// return : ����
		void UserJoinSend(INT64 AccountNo, char* Token, ULONGLONG Parameter);		

	public:
		// ������
		CLogin_LanServer();

		// �Ҹ���
		virtual ~CLogin_LanServer();


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

		
	};
}


// ----------------------------------
// ����͸� Ŭ���̾�Ʈ (LanClient)
// ----------------------------------
namespace Library_Jingyu
{
	// ������ �����ؼ� ����͸� ����(LanServer�� �����Ѵ�)
	class CMoniter_Clinet :public CLanClient
	{
		friend class CLogin_NetServer;

		// ������ ������ ��Ƶα�
		enum en_MonitorClient
		{
			dfSERVER_NO = 2	// �ϵ��� �����ϴ� ������ No�� 2���̴�.
		};

		// ����͸� ������ ���� ������ �������� �ڵ�.
		HANDLE m_hMonitorThread;

		// ����͸� ������ �����ų �̺�Ʈ
		HANDLE m_hMonitorThreadExitEvent;

		// ���� ����͸� ������ ����� ���� ID
		ULONGLONG m_ullSessionID;	


	private:
		// -----------------------
		// ���ο����� ����ϴ� ��� �Լ�
		// -----------------------

		// ���� �ð����� ����͸� ������ ������ �����ϴ� ������
		static UINT	WINAPI MonitorThread(LPVOID lParam);

		// ����͸� ������ ������ ����
		//
		// Parameter : DataType(BYTE), DataValue(int), TimeStamp(int)
		// return : ����
		void InfoSend(BYTE DataType, int DataValue, int TimeStamp);

	public:
		// -----------------------
		// �����ڿ� �Ҹ���
		// -----------------------
		CMoniter_Clinet();
		virtual ~CMoniter_Clinet();

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


	public:
		// -----------------------
		// �����Լ�
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


#endif // !__LOGIN_SERVER_H__
