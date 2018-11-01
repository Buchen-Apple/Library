#ifndef __MATCH_MAKING_SERVER_H__
#define __MATCH_MAKING_SERVER_H__

#include "NetworkLib/NetworkLib_NetServer.h"
#include "DB_Connector/DB_Connector.h"

// ---------------------------------------------
// 
// ��ġ����ŷ Net����
//
// ---------------------------------------------
namespace Library_Jingyu
{
	class Matchmaking_Net_Server :public CNetServer
	{

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
			int XORCode1;
			int XORCode2;
			int Nodelay;
			int MaxJoinUser;
			int LogLevel;			


			// ��ġ����ŷ DB�� ����
			TCHAR DB_IP[20];
			TCHAR DB_User[40];
			TCHAR DB_Password[40];
			TCHAR DB_Name[40];
			int  DB_Port;
			int MatchDBHeartbeat;	// ��ġ����ŷ DB�� �� �и������帶�� ��Ʈ��Ʈ�� �� ���ΰ�.

			// ��ġ����ŷ Lan Ŭ�� ����.
			int ServerNo;	// �ش� ������, Net���������� ������(DB ��Ʈ��Ʈ), LanŬ�󿡰� �� �߿��� �����̱� ������ LanŬ�� ������ �з�
			char MasterToken[32];
		};
		

		// ------------
		// ��� ����
		// ------------

		// �Ľ̿� ����
		stConfigFile m_stConfig;

		// DBHeartbeatThread�� �ڵ�
		HANDLE hDB_HBThread;

		// DBHeartbeatThread ����� �̺�Ʈ
		HANDLE hDB_HBThread_ExitEvent;

		// DB_Connector TLS ����
		//
		// MatchmakingDB�� ����
		CBConnectorTLS* m_MatchDB_Connector;




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


		// -------------------------------------
		// ������
		// -------------------------------------

		// matchmakingDB�� ���� �ð����� ��Ʈ��Ʈ�� ��� ������.
		static UINT WINAPI DBHeartbeatThread(LPVOID lParam);



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


#endif // !__MATCH_MAKING_SERVER_H__
