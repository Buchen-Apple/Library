#ifndef __MONITOR_SERVER_H__
#define __MONITOR_SERVER_H__

#include "NetworkLib\NetworkLib_NetServer.h"
#include "NetworkLib\NetworkLib_LanServer.h"
#include "Protocol_Set/CommonProtocol_2.h"
#include "CrashDump\CrashDump.h"

#include <vector>


// ----------------------
// ����͸� Net ����
// ----------------------
namespace Library_Jingyu
{
	class CNet_Monitor_Server :public CNetServer
	{
		// Config ���� ���� ����ü
		struct stConfigFile
		{
			// ����͸� Net ����
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

			// ����͸� Lan ����
			TCHAR LanBindIP[20];
			int LanPort;
			int LanCreateWorker;
			int LanActiveWorker;
			int LanCreateAccept;
			int LanNodelay;
			int LanMaxJoinUser;
			int LanLogLevel;

			// DB
			TCHAR DB_IP[20];
			TCHAR DB_User[40];
			TCHAR DB_Password[40];
			TCHAR DB_Name[40];
			int  DB_Port;

		};
		
		friend class CLan_Monitor_Server;

		// �÷��̾� ����ü
		struct stPlayer
		{
			ULONGLONG m_ullSessionID;
			bool m_bLoginCheck;		// true�� �α��� ��Ŷ ó���� �÷��̾�
		};

	private:
		// -----------------------
		// ��� ����
		// -----------------------

		// �α��� ���� ������ Ű		
		char m_cLoginKey[32];

		// Config ���� ����
		stConfigFile m_stConfig;

		// �÷��̾� ����ü ���� TLS
		// stPlayer�� �ٷ��.
		CMemoryPoolTLS<stPlayer> *m_PlayerTLS;

		// �÷��̾� ���� �ڷᱸ�� 
		// Vector�� ������
		// ��� : stPlayer*
		//
		// ���� : �ش� �ڷᱸ���� �̿��� �ַ� [��ȸ]�� �Ѵ�.
		// �׸���, �����ϴ� Ŭ���̾�Ʈ�� ����͸� ���̱� ������, �ƹ��� ���� ���͵� 100 �̻� �ȵ��´�.
		// ������ ��ȸ�� ������ ���� ���
		vector<stPlayer*> m_vectorPlayer;
		SRWLOCK	m_vectorSrwl;

		// !! ����͸� �� ���� !!
		CLan_Monitor_Server* m_CLanServer;


	public:
		// ������, �Ҹ���
		CNet_Monitor_Server();
		virtual ~CNet_Monitor_Server();

	private:
		// -----------------------
		// ���ο����� ��� ������ ����Լ�
		// -----------------------

		// ���Ͽ��� Config ���� �о����
		// 
		// Parameter : config ����ü
		// return : ���������� ���� �� true
		//		  : �� �ܿ��� false
		bool SetFile(stConfigFile* pConfig);
			
		// �÷��̾� �ڷᱸ���� �÷��̾� �߰�.
		//
		// Parameter : SessionID
		// return : ����
		void InsertPlayer(ULONGLONG SessionID);

		// �÷��̾� �ڷᱸ������ �÷��̾� ����
		//
		// Parameter : SessionID
		// return : ���� �÷��̾� �� �� false.
		bool ErasePlayer(ULONGLONG SessionID);
		
		// ������ Ŭ��鿡�� ���� �Ѹ��� (��ε� ĳ����)
		//
		// Parameter : ����No, ������ Ÿ��, ������ ��, Ÿ�ӽ�����
		// return : ����
		void DataBroadCasting(BYTE ServerNo, BYTE DataType, int DataValue, int TimeStamp);
		

		// �α��� ��û ��Ŷ ó��
		//
		// Parameter : ����ID, Net ����ȭ����
		void LoginPakcet(ULONGLONG SessionID, CProtocolBuff_Net* Payload);



	public:
		// -----------------------
		// �ܺο��� ��� ������ �Լ�
		// -----------------------

		// ����͸� Net���� ����
		// ���ο����� CNetServer�� Start �Լ� ȣ��
		//
		// Parameter : ����
		// return : ����
		bool ServerStart();

		// ����͸� Net���� ����
		// ���ο����� CNetServer�� Stop �Լ� ȣ��
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

		// �������� �߻� �� ȣ��Ǵ� �Լ�
		//
		// parameter : SessionID
		// return : ����
		virtual void OnSemaphore(ULONGLONG SessionID);

	};
}



// ----------------------
// ����͸� Lan ����
// ----------------------
namespace Library_Jingyu
{
	class CLan_Monitor_Server :public CLanServer
	{
		friend class CNet_Monitor_Server;

		// DB�� ���� ���� ���� ����ü
		struct stDBWriteInfo
		{
			int m_iType = 0;
			char m_cServerName[64] = { 0, };
			int m_iValue = 0;
			int m_iMin = 0x0fffffff;
			int m_iMax = 0;
			ULONGLONG m_iTotal = 0;
			int m_iTotalCount = 0;

			float m_iAvr = 0;
			int m_iServerNo = 0;

			// �ʱ�ȭ
			void init()
			{
				m_iValue = 0;
				m_iMin = 0x0fffffff;
				m_iMax = 0;
				m_iTotal = 0;
				m_iTotalCount = 0;
			}
		};

		// ������ �� Ŭ���̾�Ʈ ���� ����ü
		struct stServerInfo
		{
			ULONGLONG m_ullSessionID;
			int m_bServerNo;
		};


	private:
		// -----------------------
		// ��� ����
		// -----------------------

		// !! ����͸� �� ����(�θ�)�� this !!
		CNet_Monitor_Server* m_ParentThis;


		// ������ ��Ŭ�� ������ ����ü �迭
		// SessionID, ServerNo ����
		stServerInfo m_arrayJoinServer[10];

		// ����ü �迭 �ȿ� �ִ� ���� �� (������ �� Ŭ�� ��)
		int m_iArrayCount;

		// ����ü �迭 ��
		SRWLOCK srwl;

		// DB ���� ���� ����ü�� ��
		stDBWriteInfo m_stDBInfo[dfMONITOR_DATA_TYPE_END - 1];
		SRWLOCK DBInfoSrwl;

		// DBWriteThread �ڵ�
		HANDLE m_hDBWriteThread;

		// DBWriteThread ����� �̺�Ʈ
		HANDLE m_hDBWriteThreadExitEvent;


	private:
		// -----------------------
		// ���� ��� �Լ�
		// -----------------------
		
		// �θ�(NetServer)�� This ���� �Լ�
		//
		// Parameter : ����͸� �ݼ����� This
		void ParentSet(CNet_Monitor_Server* NetServer);


		// ����͸� Ŭ���̾�Ʈ�κ��� ���� ���� ����
		// ���� ���� ��, �� ���鿡�� ����
		//
		// Parameter : Lan����ȭ ����
		// return : ����
		void DataUpdatePacket(CProtocolBuff_Lan* Payload);
		
		// �α��� ��û ó��
		//
		// Parameter : SessionID, Lan����ȭ ����
		// return : ����
		void LoginPacket(ULONGLONG SessionID, CProtocolBuff_Lan* Payload);

		// DB�� ���� ���� ������
		// 1�п� 1ȸ DB�� Insert
		static UINT	WINAPI DBWriteThread(LPVOID lParam);


	public:
		// -----------------------
		// �ܺο��� ���� ������ ��� �Լ�
		// -----------------------

		// ����͸� Lan ���� ����
		// ���������� CLanServer�� Start�Լ� ȣ��
		//
		// Parameter : ����
		// return :  ����
		bool LanServerStart();

		// ����͸� Lan ���� ����
		// ���������� CLanServer�� stop�Լ� ȣ��
		//
		// Parameter : ����
		// return :  ����
		void LanServerStop();




	public:
		// ������
		CLan_Monitor_Server();

		// �Ҹ���
		virtual ~CLan_Monitor_Server();

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
	};
}


#endif // !__MONITOR_SERVER_H__
