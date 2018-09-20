#ifndef __CHAT_SERVER_H__
#define __CHAT_SERVER_H__

#include "NetworkLib\NetworkLib_NetServer.h"
#include "NetworkLib\NetworkLib_LanClinet.h"

#include <vector>
#include <unordered_map>

using namespace std;


// ---------------------------------------------
// 
// LoginServer�� LanServer�� ����ϴ� LanClient
// 
// ---------------------------------------------
namespace Library_Jingyu
{	
	class Chat_LanClient :public CLanClient
	{
		friend class CChatServer;

		// -----------------------
		// ���� ����ü
		// -----------------------
		// ��ū ����ü
		struct stToken
		{
			char	m_cToken[64];
			ULONGLONG m_ullInsertTime;	// �μ�Ʈ �� �ð�
		};

	private:
		// -----------------------
		// ��� ����
		// -----------------------

		// ��ū ����ü�� �����ϴ� TLS
		CMemoryPoolTLS< stToken >* m_MTokenTLS;

		// ��ū �ڷᱸ�� Lock
		SRWLOCK srwl;

		// ��ūŰ�� �����ϴ� �ڷᱸ��
		//
		// Key : AccountNO
		// Value : ��ū ����ü
		unordered_map<INT64, stToken*> m_umapTokenCheck;

	private:
		// -----------------------
		// ��Ŷ ó�� �Լ�
		// -----------------------

		// ���ο� ������ �α��� ������ ���� ��, �α��� �����κ��� ��ūŰ�� �޴´�.
		// �� ��ūŰ�� ������ �� ������ ������ �Լ�
		//
		// Parameter : ����Ű, CProtocolBuff_Lan*
		// return : ����
		void NewUserJoin(ULONGLONG SessionID, CProtocolBuff_Lan* Payload);


		// -----------------------
		// ��� �Լ�
		// -----------------------

		// ��ū ���� �ڷᱸ����, ���� ������ ��ū �߰�
		// ���� umap���� ������
		// 
		// Parameter : AccountNo, Token(char*)
		// return : ����
		void InsertTokenFunc(INT64 AccountNo, char* Token);


		// ��ū ���� �ڷᱸ������, ��ū �˻�
		// ���� umap���� ������
		// 
		// Parameter : AccountNo
		// return : �˻� ���� ��, stToken*
		//		  : �˻� ���� �� nullptr
		stToken* FindTokenFunc(INT64 AccountNo);


		// ��ū ���� �ڷᱸ������, ��ū ����
		// ���� umap���� ������
		// 
		// Parameter : AccountNo
		// return : ���� ��, ���ŵ� ��ū stToken*
		//		  : �˻� ���� �� nullptr
		stToken* EraseTokenFunc(INT64 AccountNo);


	public:
		// -----------------------
		// �����ڿ� �Ҹ���
		// -----------------------
		Chat_LanClient();
		virtual ~Chat_LanClient();



	public:
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


// ---------------------------------------------
// 
// ChatServer
// 
// ---------------------------------------------
namespace Library_Jingyu
{
	// 1�� ���� ���� ��
#define SECTOR_X_COUNT	50
#define SECTOR_Y_COUNT	50
	
	class CChatServer :public CNetServer
	{
		// -------------------------------------
		// inner ����ü
		// -------------------------------------
		// �̸� ���� 9���� ���صδ� ����ü
		struct st_SecotrSaver
		{
			POINT m_Sector[9];
			LONG m_dwCount;
		};

		// �ϰ� ����ü
		struct st_WorkNode
		{
			WORD m_wType;
			ULONGLONG m_ullSessionID;
			CProtocolBuff_Net* m_pPacket;
		};

		// ���Ͽ��� �о���� �� ����ü
		struct stConfigFile
		{
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

			TCHAR LoginServerIP[20];
			int LoginServerPort;
			int Client_CreateWorker;
			int Client_ActiveWorker;
			int Client_Nodelay;
		};

		// �÷��̾� ����ü
		struct stPlayer
		{
			// ���� ID (��Ʈ��ũ�� ��� �� ����ϴ� Ű��)
			ULONGLONG m_ullSessionID;

			// ���� ��ǥ X,Y
			WORD m_wSectorX;
			WORD m_wSectorY;

			// ���� ���� ��
			INT64 m_i64AccountNo;

			// ���� ID (�α��� �� ����ϴ� ID)
			WCHAR m_tLoginID[20];

			// �г���
			WCHAR m_tNickName[20];

			// ���������� ��Ŷ�� ���� �ð� (GetTickCount64)
			ULONGLONG m_ullLastPacketTime;

			// ��ū (���� Ű)
			//char m_cToken[64];
		};
			

	private:
		// -------------------------------------
		// ��� ����
		// -------------------------------------
		// !! ĳ�� ��Ʈ�� ����� ���� ���Ǵ� �͵鳢�� �������Ѽ� ����
		// !! ��κ�, ���� �� �����ϴ�(Write)�ϴ� �������̱� ������ �б�����, �������� �������� ������ ����.

		// ����
		CCrashDump* m_ChatDump = CCrashDump::GetInstance();

		////////////////////////////////////////////////
		// !! �α��� ������ ����ϱ� ���� LanClient !!
		////////////////////////////////////////////////
		Chat_LanClient m_Logn_LanClient;

		// �Ľ��� ���� ����ü
		stConfigFile m_stConfig;

		// �޽��� ����ü�� �ٷ� TLS�޸�Ǯ
		// �ϰ� ����ü�� �ٷ��.
		CMemoryPoolTLS<st_WorkNode> *m_MessagePool;

		// �޽����� ���� ������ ť
		// �ּ�(8����Ʈ)�� �ٷ��.
		CLF_Queue<st_WorkNode*> *m_LFQueue;

		// �÷��̾� ����ü�� �ٷ� TLS�޸�Ǯ
		CMemoryPoolTLS<stPlayer> *m_PlayerPool;

		// �÷��̾� ����ü�� �ٷ�� umap
		// Key : SessionID
		// Value : stPlayer*
		// 
		// ���� ���� ---
		// �ش� �ڷᱸ���� �̿��� [����, ����, �˻�]�� �ַ� ���.
		// �� ��, �÷��̾� ����, ��������(����, ����) �ÿ��� ���� ������ �����ٰ� �Ǵ�.
		// �ٽ��� �÷��� ���� ������ ������ �󸶳� ������ ó���ϴ°��� �ٽ�. 
		// �߰��� ���� �����Ͱ� ���ĵǾ� ���� �ʿ䵵 ����
		// ������ umap�� ����
		unordered_map<ULONGLONG, stPlayer*> m_mapPlayer;

		// ���� vector
		// ��� : SessionID
		//
		// ���� ���� ---
		// �ش� �ڷᱸ���� �̿��� [����, ����, ��ȸ]�� �ַ� ���
		// �� ��, �ֺ� ���Ϳ� ä�� �޽����� �߼��ϴ� "��ȸ"�� �ٽ����� �Ǵ�.
		// ����, ������ ���, push_pack, pop_pack ���� �ϸ� O(1)�� �ӵ�
		// ���� ���ĵ� �ʿ䰡 ���� ������, ���� �ÿ��� �׳� push_back���� ������ �ȴ�.
		// ������ pop_back�ε�, �� ���, ���� ��ȸ�� ����, (�迭�� ���, ó�� ���� ��ġ���� +�ϸ鼭 ��ġ�� �̵��ϱ⶧���� ��ȸ�� ������ ������.)
		// �����ϰ��� �ϴ� ��ҿ� ������ ��Ҹ� swap ��, ������ ��Ҹ� pop_back�ϴ� ���·� �ذ�.
		vector<ULONGLONG> m_vectorSecotr[SECTOR_Y_COUNT][SECTOR_X_COUNT];

		//  X,Y ����, 9���� ���͸� �̸� ���ؼ� �����صδ� �迭
		st_SecotrSaver* m_stSectorSaver[SECTOR_Y_COUNT][SECTOR_X_COUNT];

		// ���������� ��Ŷ�� ���� �ð��� �����ϴ� umap
		// Key : SessionID
		// Value : ���������� ��Ŷ�� ���� �ð� (GetTickCount64)
		//
		// ��ȸ�� �־, vector�� �ϰ� �;�����, key / value ������ �Ǿ�� �ϱ� ������ umap���� ����
		unordered_map<ULONGLONG, ULONGLONG> m_mapLastPacketTime;

		// ���������� ��Ŷ�� ���� �ð��� �����ϴ� umap�� ��
		SRWLOCK m_LastPacketsrwl;

		// ������Ʈ ������ �ڵ�
		HANDLE hUpdateThraed;

		// ������Ʈ ������ ����� �뵵 Event
		HANDLE UpdateThreadEvent;

		// ������Ʈ ������ ���� �뵵 Event
		HANDLE UpdateThreadEXITEvent;


		// �ϰ� �߰� ������ �ڵ�
		HANDLE hJobThraed;

		// �ϰ� �߰� ������ ���� �뵵 Event
		HANDLE JobThreadEXITEvent;


	private:
		// -------------------------------------
		// Ŭ���� ���ο����� ����ϴ� ��� �Լ�
		// -------------------------------------	

		// ���ڷ� ���� ���� X,Y ����, ��ε� ĳ��Ʈ ��, ������ �� ���� 9���� ���صд�.
		// 
		// Parameter : ���� ���� X, Y, ���ͱ���ü(out)
		// return : ����
		void SecotrSave(int SectorX, int SectorY, st_SecotrSaver* Sector);

		// ���Ͽ��� Config ���� �о����
		// 
		// Parameter : config ����ü
		// return : ���������� ���� �� true
		//		  : �� �ܿ��� false
		bool SetFile(stConfigFile* pConfig);

		// Player ���� �ڷᱸ����, ���� �߰�
		// ���� map���� ������
		// 
		// Parameter : SessionID, stPlayer*
		// return : �߰� ���� ��, true
		//		  : SessionID�� �ߺ��� ��(�̹� �������� ����) false
		bool InsertPlayerFunc(ULONGLONG SessionID, stPlayer* insertPlayer);

		// Player ���� �ڷᱸ������, ���� �˻�
		// ���� map���� ������
		// 
		// Parameter : SessionID
		// return : �˻� ���� ��, stPalyer*
		//		  : �˻� ���� �� nullptr
		stPlayer* FindPlayerFunc(ULONGLONG SessionID);

		// Player ���� �ڷᱸ������, ���� ���� (�˻� �� ����)
		// ���� map���� ������
		// 
		// Parameter : SessionID
		// return : ���� ��, ���ŵ� ���� stPalyer*
		//		  : �˻� ���� ��(���������� ���� ����) nullptr
		stPlayer* ErasePlayerFunc(ULONGLONG SessionID);



		// ������ ��Ŷ ���� �ð� ���� �ڷᱸ����, ������ �ð� �߰�
		// �̹� �ִ� ������� ������ ������ �����Ѵ�.
		// ���� umap���� ������
		// 
		// Parameter : SessionID
		// return : ����
		void InsertLastTime(ULONGLONG SessionID);

		// ������ ��Ŷ ���� �ð� ���� �ڷᱸ������, ���� ����
		// ���� umap���� ������
		// 
		// Parameter : SessionID
		// return : ����
		void EraseLastTime(ULONGLONG SessionID);



		// ���ڷ� ���� ���� X,Y �ֺ� 9�� ������ ������(������ ��Ŷ�� ���� Ŭ�� ����)���� SendPacket ȣ��
		//
		// parameter : ���� x,y, ���� ����
		// return : ����
		void SendPacket_Sector(int SectorX, int SectorY, CProtocolBuff_Net* SendBuff);

		// ������Ʈ ������
		static UINT	WINAPI	UpdateThread(LPVOID lParam);

		// �ϰ� �߰� ������
		//
		// ���� ��Ʈ��Ʈ ��...
		static UINT	WINAPI	JobAddThread(LPVOID lParam);


	private:
		// -------------------------------------
		// Ŭ���� ���ο����� ����ϴ� ��Ŷ ó�� �Լ�
		// -------------------------------------

		// ���� ��Ŷó�� �Լ�
		// OnClientJoin���� ȣ��
		// 
		// Parameter : SessionID
		// return : ����
		void Packet_Join(ULONGLONG SessionID);

		// ���� ��Ŷó�� �Լ�
		// OnClientLeave���� ȣ��
		// 
		// Parameter : SessionID
		// return : ����
		void Packet_Leave(ULONGLONG SessionID);

		// �Ϲ� ��Ŷó�� �Լ�
		// 
		// Parameter : SessionID, CProtocolBuff_Net*
		// return : ����
		void Packet_Normal(ULONGLONG SessionID, CProtocolBuff_Net* Packet);



		// -------------------------------------
		// '�Ϲ� ��Ŷ ó�� �Լ�'���� ó���Ǵ� �Ϲ� ��Ŷ��
		// -------------------------------------

		// ���� �̵���û ��Ŷ ó��
		//
		// Parameter : SessionID, CProtocolBuff_Net*
		// return : ����
		void Packet_Sector_Move(ULONGLONG SessionID, CProtocolBuff_Net* Packet);

		// ä�� ������ ��û
		//
		// Parameter : SessionID, CProtocolBuff_Net*
		// return : ����
		void Packet_Chat_Message(ULONGLONG SessionID, CProtocolBuff_Net* Packet);

		// �α��� ��û
		//
		// Parameter : SessionID, CProtocolBuff_Net*
		// return : ����
		void Packet_Chat_Login(ULONGLONG SessionID, CProtocolBuff_Net* Packet);


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
		// Ŭ���� �ܺο��� ��� ������ �Լ�
		// -------------------------------------

		// ������
		CChatServer();

		//�Ҹ���
		virtual ~CChatServer();

		// ä�� ���� ���� �Լ�
		// ���������� NetServer�� Start�� ���� ȣ��
		//
		// return false : ���� �߻� ��. �����ڵ� ���� �� false ����
		// return true : ����
		bool ServerStart();

		// ä�� ���� ���� �Լ�
		//
		// Parameter : ����
		// return : ����
		void ServerStop();

		// �׽�Ʈ��. ť ��� �� ���
		LONG GetQueueInNode()
		{
			return m_LFQueue->GetInNode();
		}

		// �׽�Ʈ��. �� ���� �÷��̾� �� ���
		LONG JoinPlayerCount()
		{
			return (LONG)m_mapPlayer.size();
		}

		// !! �׽�Ʈ�� !!
		// �ϰ� TLS�� �� �Ҵ�� ûũ �� ��ȯ
		LONG GetWorkChunkCount()
		{
			return m_MessagePool->GetAllocChunkCount();
		}

		// !! �׽�Ʈ�� !!
		// �ϰ� TLS�� ���� �ۿ��� ������� ûũ �� ��ȯ
		LONG GetWorkOutChunkCount()
		{
			return m_MessagePool->GetOutChunkCount();
		}


		// !! �׽�Ʈ�� !!
		// �÷��̾� TLS�� �� �Ҵ�� ûũ �� ��ȯ
		LONG GetPlayerChunkCount()
		{
			return m_PlayerPool->GetAllocChunkCount();
		}

		// !! �׽�Ʈ�� !!
		// �÷��̾� TLS�� ���� �ۿ��� ������� ûũ �� ��ȯ
		LONG GetPlayerOutChunkCount()
		{
			return m_PlayerPool->GetOutChunkCount();
		}

		// ----------------------
		// ----------------------
		// �� Ŭ���̾�Ʈ�� 

		// !! �׽�Ʈ�� !!
		// ��ū umap ���� �� ��ȯ
		LONG GetTokenUmapSize()
		{
			return (LONG)m_Logn_LanClient.m_umapTokenCheck.size();
		}

		// !! �׽�Ʈ�� !!
		// ��ū TLS�� �� �Ҵ�� ûũ �� ��ȯ
		LONG GetTokenChunkCount()
		{
			return m_Logn_LanClient.m_MTokenTLS->GetAllocChunkCount();
		}

		// !! �׽�Ʈ�� !!
		// ��ū TLS�� ���� �ۿ��� ������� ûũ �� ��ȯ
		LONG GetTokenOutChunkCount()
		{
			return m_Logn_LanClient.m_MTokenTLS->GetOutChunkCount();
		}
	};
}



#endif // !__CHAT_SERVER_H__
