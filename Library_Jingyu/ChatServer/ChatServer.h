#ifndef __CHAT_SERVER_H__
#define __CHAT_SERVER_H__

#include "NetworkLib\NetworkLib_NetServer.h"
#include "ObjectPool\Object_Pool_LockFreeVersion.h"
#include "CrashDump\CrashDump.h"

#include "LockFree_Queue\LockFree_Queue.h"

#include <vector>
#include <unordered_map>

using namespace std;


namespace Library_Jingyu
{

	// 1�� ���� ���� ��
#define SECTOR_X_COUNT	50
#define SECTOR_Y_COUNT	50


	////////////////////////////////////////////
	// !! ä�ü��� !!
	////////////////////////////////////////////
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

			// ��ū (���� Ű)
			char m_cToken[64];
		};

	private:
		// -------------------------------------
		// ��� ����
		// -------------------------------------
		// !! ĳ�� ��Ʈ�� ����� ���� ���Ǵ� �͵鳢�� �������Ѽ� ����
		// !! ��κ�, ���� �� �����ϴ�(Write)�ϴ� �������̱� ������ �б�����, �������� �������� ������ ����.

		// ����
		CCrashDump* m_ChatDump = CCrashDump::GetInstance();

		// �޽��� ����ü�� �ٷ� TLS�޸�Ǯ
		// �ϰ� ����ü�� �ٷ��.
		CMemoryPoolTLS<st_WorkNode> *m_MessagePool;

		// �޽����� ���� ������ ť
		// �ּ�(8����Ʈ)�� �ٷ��.
		CLF_Queue<st_WorkNode*> *m_LFQueue;

		// �÷��̾� ����ü�� �ٷ� TLS�޸�Ǯ
		CMemoryPoolTLS<stPlayer> *m_PlayerPool;

		// �÷��̾� ����ü�� �ٷ�� map
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

		// ������Ʈ ������ �ڵ�
		HANDLE hUpdateThraed;

		// ������Ʈ ������ ����� �뵵 Event
		HANDLE UpdateThreadEvent;

		// ������Ʈ ������ ���� �뵵 Event
		HANDLE UpdateThreadEXITEvent;

	private:
		// -------------------------------------
		// Ŭ���� ���ο����� ����ϴ� ��� �Լ�
		// -------------------------------------	

		void SecotrSave(int SectorX, int SectorY, st_SecotrSaver* Sector);

		// ���Ͽ��� Config ���� �о����
		// 
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

		// ���ڷ� ���� ���� X,Y �ֺ� 9�� ������ ������(������ ��Ŷ�� ���� Ŭ�� ����)���� SendPacket ȣ��
		//
		// parameter : ���� x,y, ���� ����
		// return : ����
		void SendPacket_Sector(int SectorX, int SectorY, CProtocolBuff_Net* SendBuff);

		// ������Ʈ ������
		static UINT	WINAPI	UpdateThread(LPVOID lParam);



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
	};
}



#endif // !__CHAT_SERVER_H__
