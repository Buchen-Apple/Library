#ifndef __BATTLESERVER_ROOM_VERSION_H__
#define __BATTLESERVER_ROOM_VERSION_H__

#include "NetworkLib/NetworkLib_MMOServer.h"
#include "NetworkLib/NetworkLib_LanClinet.h"

#include "Http_Exchange/HTTP_Exchange.h"

#include <unordered_set>

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
			// �����ڿ� �Ҹ���
			// -----------------------
			CGameSession();
			virtual ~CGameSession();

			// ȸ�� ��ȣ
			INT64 m_Int64AccountNo;

			// ������ ClientKey
			// ��Ī������ �߱�.
			// ���� ��ο��� ����
			INT64 m_int64ClientKey;

			// CBattleServer_Room�� ������
			CBattleServer_Room* m_pParent;


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

		// DB_Read �����忡�� �Ͻ�Ű�� �� ����ü
		struct DB_WORK
		{
			WORD m_wType;
			CGameSession* m_pNowSession;
			CProtocolBuff_Net* m_pWorkBuff;
		};
		
		enum eu_DB_READ_TYPE
		{
			eu_LOGIN	= 0			// �α��� ��Ŷ�� ���� ó��
		};

		// -----------------------
		// ��� ����
		// -----------------------





		// -----------------------
		// ��� ����
		// -----------------------

		// ����. CMMOServer�� ����.
		CGameSession* m_cGameSession;

		// Config ����
		stConfigFile m_stConfig;

		// ����͸� Ŭ��
		CGame_MinitorClient* m_Monitor_LanClient;

		// U_set �ڷᱸ��.
		// ���� ���� ������ AccountNo ������.
		// �ִ��� ������ Ȯ�θ� �ϱ� ������ Set ���
		unordered_set<INT64> m_setAccount;

		// U_set �� ��
		SRWLOCK m_setSrwl;

		// ���� �ڵ�
		// Ŭ�� ���� ���� ��. �Ľ����� �о��.
		int m_uiVer_Code;

		// DB_read ������ �� IOCP
		HANDLE m_hDBRead_IOCPHandle;


		// ----------------

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

		// ----------------



	private:
		// -----------------------
		// ������
		// -----------------------

		// DB_Read�� ������
		// IOCP�� ������ Ǯ�� �̿��� �����ȴ�.
		static UINT WINAPI	DB_Read_Thread(LPVOID lParam);


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
