#ifndef __CHAT_SERVER_ROOM_H__
#define __CHAT_SERVER_ROOM_H__

#include "NetworkLib/NetworkLib_NetServer.h"
#include "NetworkLib/NetworkLib_LanClinet.h"
#include "ObjectPool/Object_Pool_LockFreeVersion.h"

#include <unordered_map>
#include <vector>
#include <process.h>

using namespace std;


// ------------------------
//
// Chat_Net����
//
// ------------------------
namespace Library_Jingyu
{
	class CChatServer_Room	:public CNetServer
	{
		friend class CChat_LanClient;
		friend class CChat_MonitorClient;

		// --------------
		// �̳� Ŭ����
		// --------------

		// �÷��̾� ����ü
		struct stPlayer
		{
			// ���� ID
			// ������ ��� �� ���
			ULONGLONG m_ullSessionID;

			// ȸ�� ��ȣ
			INT64 m_i64AccountNo;

			// �������� �� ��ȣ
			int m_iRoomNo;

			// ID
			TCHAR m_tcID[20];

			// Nickname
			TCHAR m_tcNick[20];

			// �α��� ���� üũ
			// true�� �α��� ��.
			bool m_bLoginCheck;

			// ���������� ��Ŷ�� ���� �ð�
			DWORD m_dwLastPacketTime;

			stPlayer()
			{
				// ���� ��, �� ��ȣ�� -1
				m_iRoomNo = -1;
			}
		};

		// �� ����ü
		struct stRoom
		{
			// ��Ʋ���� ��ȣ
			int m_iBattleServerNo;

			// �� ��ȣ
			int m_iRoomNo;		

			// ���� ���� ���� ��
			int m_iMaxUser;

			// �濡 ���� ���� ���� ��
			int m_iJoinUser;

			// �� ���� ��ū
			char m_cEnterToken[32];

			// ������ �� �÷���
			// true�� ������ ���̴�.
			// true�鼭 �濡 ���� ���� 0���̸� ���� �����ȴ�.
			bool m_bDeleteFlag;

			// �뿡 ������ ������ ���� �ڷᱸ��
			//
			// SessionID ����
			vector<ULONGLONG> m_JoinUser_vector;

			// �� ��
			SRWLOCK m_Room_srwl;

			stRoom()
			{
				// �� �ʱ�ȭ
				InitializeSRWLock(&m_Room_srwl);
			}



			// --------------
			// ��� �Լ�
			// --------------

			// �� ��
			//
			// Parameter : ����
			// return : ����
			void ROOM_LOCK()
			{
				AcquireSRWLockExclusive(&m_Room_srwl);
			}

			// �� ���
			//
			// Parameter : ����
			// return : ����
			void ROOM_UNLOCK()
			{
				ReleaseSRWLockExclusive(&m_Room_srwl);
			}

			// ������ �ڷᱸ���� ���ڷ� ���� ClientKey �߰�
			//
			// Parameter : ClientKey
			// return : ����
			void Insert(ULONGLONG ClientKey)
			{
				m_JoinUser_vector.push_back(ClientKey);
			}

			// ������ �ڷᱸ������ ���ڷ� ���� ClientKey ����
			//
			// Parameter : ClientKey
			// return : ���� �� true
			//		  : �ش� ������ ���� �� false
			bool Erase(ULONGLONG ClientKey)
			{
				size_t Size = m_JoinUser_vector.size();
				bool Flag = false;

				// 1. �ڷᱸ�� �ȿ� ������ 0�̶�� return false
				if (Size == 0)
					return false;

				// 2. �ڷᱸ�� �ȿ� ������ 1���̰ų�, ã���� �ϴ� ������ �������� �ִٸ� �ٷ� ����
				if (Size == 1 || m_JoinUser_vector[Size - 1] == ClientKey)
				{
					Flag = true;
					m_JoinUser_vector.pop_back();
				}

				// 3. �ƴ϶�� Swap �Ѵ�
				else
				{
					size_t Index = 0;
					while (Index < Size)
					{
						// ���� ã���� �ϴ� ������ ã�Ҵٸ�
						if (m_JoinUser_vector[Index] == ClientKey)
						{
							Flag = true;

							ULONGLONG Temp = m_JoinUser_vector[Size - 1];
							m_JoinUser_vector[Size - 1] = m_JoinUser_vector[Index];
							m_JoinUser_vector[Index] = Temp;

							m_JoinUser_vector.pop_back();

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
								
		};

		// �Ľ� ����ü
		struct stParser
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

			// �ܺο��� ������ IP
			TCHAR ChatIP[20];

			// ��Ʋ�� ����Ǵ� �� Ŭ��
			TCHAR BattleServerIP[20];
			int BattleServerPort;
			int BattleClientCreateWorker;
			int BattleClientActiveWorker;
			int BattleClientNodelay;

			// ����͸��� ����Ǵ� �� Ŭ��
			TCHAR MonitorServerIP[20];
			int MonitorServerPort;
			int MonitorClientCreateWorker;
			int MonitorClientActiveWorker;
			int MonitorClientNodelay;
		};



		// --------------
		// ��� ����
		// --------------

		// �ļ� ����
		stParser m_Paser;

		// ����͸� �� Ŭ�� 
		CChat_MonitorClient* m_pMonitor_Client;

		// ��Ʋ �� Ŭ��
		CChat_LanClient* m_pBattle_Client;


		// ---- ī��Ʈ �� ----

		// ���� �α��� ��Ŷ���� ó���� ���� ī��Ʈ.
		LONG m_lChatLoginCount;

		// Player ����ü �Ҵ緮
		// Alloc�� 1 ����, Free �� 1 ����
		LONG m_lUpdateStruct_PlayerCount;

		// ä�ü��� ���� ��ū�� �ٸ� �� 1 ����
		LONG m_lEnterTokenMiss;

		// �� ���� ��ū�� �ٸ� �� 1 ����
		LONG m_lRoom_EnterTokenMiss;

		// ä�ü��� �� �� �� (��� ��� ����)
		LONG m_lRoomCount;

		// �ߺ� �α��� Ƚ��
		LONG m_lLoginOverlap;





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





		// --------------------------------
		// �÷��̾� ���� �ڷᱸ�� ����
		// --------------------------------

		// �÷��̾� ���� �ڷᱸ��
		//
		// Key : �������� ���� SessionID, Value : stPlayer*
		unordered_map<ULONGLONG, stPlayer*> m_Player_Umap;

		//m_Player_Umap�� srw��
		SRWLOCK m_Player_Umap_srwl;

		// stPlayer ���� �޸�Ǯ
		CMemoryPoolTLS<stPlayer> *m_pPlayer_Pool;






		// --------------------------------
		// �α��� �� �÷��̾� ���� �ڷᱸ�� ����
		// --------------------------------

		// �α��� �� �÷��̾� ���� �ڷᱸ��
		//
		// Key : AccountNo, Value : stPlayer*
		unordered_map<INT64, ULONGLONG> m_LoginPlayer_Umap;

		//m_Player_map�� srw��
		SRWLOCK m_LoginPlayer_Umap_srwl;




		// --------------------------
		// �� ���� �ڷᱸ�� ����
		// --------------------------

		// �� ���� �ڷᱸ��
		//
		// Key : RoomNo, Value : stRoom*
		unordered_map<int, stRoom*> m_Room_Umap;

		//m_Player_Umap�� srw��
		SRWLOCK m_Room_Umap_srwl;

		// stRoom ���� �޸�Ǯ
		CMemoryPoolTLS<stRoom> *m_pRoom_Pool;



		// --------------------------
		// ��Ʈ��Ʈ ������ ����
		// --------------------------

		// ��Ʈ��Ʈ ������ �ڵ�
		HANDLE m_hHBthreadHandle;

		// ��Ʈ��Ʈ ������ ���� �̺�Ʈ
		HANDLE m_hHBThreadExitEvent;


	private:
		// -----------------------
		// ������
		// -----------------------

		// ��Ʈ��Ʈ ������
		static UINT WINAPI HeartBeatThread(LPVOID lParam);




	private:
		// ----------------------------
		// �÷��̾� ���� �ڷᱸ�� �Լ�
		// ----------------------------

		// �÷��̾� ���� �ڷᱸ���� Insert
		//
		// Parameter : SessionID, stPlayer*
		// return : ���� �߰� �� true
		//		  : Ű �ߺ� �� false
		bool InsertPlayerFunc(ULONGLONG SessionID, stPlayer* InsertPlayer);

		// �÷��̾� ���� �ڷᱸ������ �˻�
		//
		// Parameter : SessionID
		// return :  �� ã���� stPlayer*
		//		  :  ���� ������ �� nullptr	
		stPlayer* FindPlayerFunc(ULONGLONG SessionID);

		// �÷��̾� ���� �ڷᱸ������ ����
		//
		// Parameter : SessionID
		// return :  Erase �� Second(stPlayer*)
		//		  : ���� ������ �� nullptr
		stPlayer* ErasePlayerFunc(ULONGLONG SessionID);
			   



	private:
		// ----------------------------
		// �α��� �� �÷��̾� ���� �ڷᱸ�� �Լ�
		// ----------------------------

		// �α��� �� �÷��̾� ���� �ڷᱸ���� Insert
		//
		// Parameter : AccountNo, SessionID
		// return : ���� �߰� �� true
		//		  : Ű �ߺ� �� flase
		bool InsertLoginPlayerFunc(INT64 AccountNo, ULONGLONG SessionID);	

		// �α��� �� �÷��̾� ���� �ڷᱸ������ ����
		//
		// Parameter : AccountNo
		// return : ���������� Erase �� true
		//		  : ����  �˻� ���� �� false
		bool EraseLoginPlayerFunc(ULONGLONG AccountNo);



	private:
		// ----------------------------
		// �� ���� �ڷᱸ�� �Լ�
		// ----------------------------

		// �� �ڷᱸ���� Insert
		//
		// Parameter : RoonNo, stROom*
		// return : ���� �߰� �� true
		//		  : Ű �ߺ� �� flase
		bool InsertRoomFunc(int RoomNo, stRoom* InsertRoom);

		// �� �ڷᱸ������ Erase
		//
		// Parameter : RoonNo
		// return : ���������� ���� �� stRoom*
		//		  : �˻� ���� �� nullptr
		stRoom* EraseRoomFunc(int RoomNo);

		// �� �ڷᱸ���� ��� �� ����
		//
		// Parameter : ����
		// return : ����
		void RoomClearFunc();



	public:
		// --------------
		// �ܺο��� ȣ�� ������ �Լ�
		// --------------

		// ��¿� �Լ�
		void ShowPrintf();


		// ���� ����
		//
		// Parameter : ����
		// return : ���� �� true
		//		  : ���� �� false
		bool ServerStart();

		// ���� ����
		//
		// Parameter : ����
		// return : ����
		void ServerStop();



	private:
		// ------------------
		// ���ο����� ����ϴ� �Լ�
		// ------------------

		// �� ���� ��� �������� ���ڷ� ���� ��Ŷ ������.
		//
		// Parameter : ULONGLONG �迭, �迭�� ��,  CProtocolBuff_Net*
		// return : �ڷᱸ���� 0���̸� false. �� �ܿ��� true
		bool Room_BroadCast(ULONGLONG Array[], int ArraySize, CProtocolBuff_Net* SendBuff);

		// Config ����
		//
		// Parameter : stParser*
		// return : ���� �� false
		bool SetFile(stParser* pConfig);		



	private:
		// ------------------
		// ��Ŷ ó�� �Լ�
		// ------------------

		// �α��� ��Ŷ
		//
		// Parameter : SessionID, CProtocolBuff_Net*
		// return : ����
		void Packet_Login(ULONGLONG SessionID, CProtocolBuff_Net* Packet);

		// �� ���� 
		//
		// Parameter : SessionID, CProtocolBuff_Net*
		// return : ����
		void Packet_Room_Enter(ULONGLONG SessionID, CProtocolBuff_Net* Packet);

		// ä�� ������
		//
		// Parameter : SessionID, CProtocolBuff_Net*
		// return : ����
		void Packet_Message(ULONGLONG SessionID, CProtocolBuff_Net* Packet);

		// ��Ʈ��Ʈ
		//
		// Parameter : SessionID
		// return : ����
		void Packet_HeartBeat(ULONGLONG SessionID);



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




	public:
		// --------------
		// �����ڿ� �Ҹ���
		// --------------

		// ������
		CChatServer_Room();

		// �Ҹ���
		virtual ~CChatServer_Room();
	};
}


// -----------------------
//
// ��Ʋ������ ����Ǵ� Lan Ŭ��
//
// -----------------------
namespace Library_Jingyu
{
	class CChat_LanClient	:public CLanClient
	{
		friend class CChatServer_Room;


		// ----------------------
		// ��� ����
		// ----------------------

		// !! ä�� �ݼ��� !!
		CChatServer_Room* m_pNetServer;

		// ��Ʋ �������� ������ ���Ǿ��̵�
		ULONGLONG m_ullSessionID;

		// ��Ʋ �������� �α��� ����
		// true�� �α��� �� ����
		// ���� ������ ���� �� ������ ������ true�� ����
		bool m_bLoginCheck;





	private:
		// ----------------------
		// ���ο����� ����ϴ� �Լ�
		// ----------------------

		// ä�� �ݼ��� ����
		//
		// Parameter : CChatServer_Room*
		// return : ����
		void SetNetServer(CChatServer_Room* NetServer);

		



	private:
		// ----------------------
		// ��Ŷ ó�� �Լ�
		// ----------------------

		// �ű� ���� ����
		//
		// Parameter : ClientID, CProtocolBuff_Lan*
		// return : ����
		void Packet_RoomCreate(ULONGLONG ClinetID, CProtocolBuff_Lan* Payload);

		// ���� ��ū �����
		//
		// Parameter : ClientID, CProtocolBuff_Lan*
		// return : ����
		void Packet_TokenChange(ULONGLONG ClinetID, CProtocolBuff_Lan* Payload);

		// �α��� ��Ŷ�� ���� ����
		//
		// Parameter : CProtocolBuff_Lan*
		// return : ����
		void Packet_Login(CProtocolBuff_Lan* Payload);

		// �� ���� 
		//
		// Parameter : ClientID, CProtocolBuff_Lan*
		// return : ����
		void Packet_RoomErase(ULONGLONG ClinetID, CProtocolBuff_Lan* Payload);


	public:
		// ----------------------
		// �ܺο��� ��� ������ �Լ�
		// ----------------------

		// ���� �Լ�
		// ����������, ��ӹ��� CLanClient�� Startȣ��.
		//
		// Parameter : ������ ������ IP, ��Ʈ, ��Ŀ������ ��, Ȱ��ȭ��ų ��Ŀ������ ��, TCP_NODELAY ��� ����(true�� ���)
		// return : ���� �� true , ���� �� falsel 
		bool ClientStart(TCHAR* ConnectIP, int Port, int CreateWorker, int ActiveWorker, int Nodelay);

		// Ŭ���̾�Ʈ ����
		//
		// Parameter : ����
		// return : ����
		void ClientStop();




	private:
		// -----------------------
		// �����Լ�
		// -----------------------

		// ��ǥ ������ ���� ���� ��, ȣ��Ǵ� �Լ� (ConnectFunc���� ���� ���� �� ȣ��)
		//
		// parameter : ����Ű
		// return : ����
		virtual void OnConnect(ULONGLONG ClinetID);

		// ��ǥ ������ ���� ���� �� ȣ��Ǵ� �Լ� (InDIsconnect �ȿ��� ȣ��)
		//
		// parameter : ����Ű
		// return : ����
		virtual void OnDisconnect(ULONGLONG ClinetID);

		// ��Ŷ ���� �Ϸ� �� ȣ��Ǵ� �Լ�.
		//
		// parameter : ����Ű, ���� ��Ŷ
		// return : ����
		virtual void OnRecv(ULONGLONG ClinetID, CProtocolBuff_Lan* Payload);

		// ��Ŷ �۽� �Ϸ� �� ȣ��Ǵ� �Լ�
		//
		// parameter : ����Ű, Send �� ������
		// return : ����
		virtual void OnSend(ULONGLONG ClinetID, DWORD SendSize);

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
		// -------------------
		// �����ڿ� �Ҹ���
		// -------------------

		// ������
		CChat_LanClient();

		// �Ҹ���
		virtual ~CChat_LanClient();

	};
}


// ---------------------------------------------
// 
// ����͸� LanClient
// 
// ---------------------------------------------
namespace Library_Jingyu
{
	class CChat_MonitorClient :public CLanClient
	{
		friend class CChatServer_Room;

		// ������ ������ ��Ƶα�
		enum en_MonitorClient
		{
			dfSERVER_NO = 3	// ä�ü����� 3���̴�
		};

		// -----------------------
		// ��� ����
		// -----------------------

		// ����͸� ������ ���� ������ �������� �ڵ�.
		HANDLE m_hMonitorThread;

		// ����͸� ������ �����ų �̺�Ʈ
		HANDLE m_hMonitorThreadExitEvent;

		// ���� ����͸� ������ ����� ���� ID
		ULONGLONG m_ullSessionID;

		// ----------------------
		// !! ä�� ������ this !!
		// ----------------------
		CChatServer_Room* m_ChatServer_this;


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
		CChat_MonitorClient();
		virtual ~CChat_MonitorClient();


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

		// ä�ü����� this�� �Է¹޴� �Լ�
		// 
		// Parameter : �� ������ this
		// return : ����
		void SetNetServer(CChatServer_Room* ChatThis);


	private:
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







#endif // !__CHAT_SERVER_ROOM_H__
