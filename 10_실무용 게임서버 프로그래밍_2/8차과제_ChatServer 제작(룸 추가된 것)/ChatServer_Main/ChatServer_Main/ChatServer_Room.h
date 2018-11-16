#ifndef __CHAT_SERVER_ROOM_H__
#define __CHAT_SERVER_ROOM_H__

#include "NetworkLib/NetworkLib_NetServer.h"
#include "ObjectPool/Object_Pool_LockFreeVersion.h"

#include <unordered_map>
#include <vector>

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
		};

		// �� ����ü
		struct stRoom
		{
			// �� ��ȣ
			int m_iRoomNo;

			// �� ���� ��ū
			char m_cEnterToken[32];

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




		// --------------
		// ��� ����
		// --------------

		// ä�� Net���� ���� ��ū
		// Ŭ�� ��� �´�.
		char m_cConnectToken[32];





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
		unordered_map<INT64, stPlayer*> m_LoginPlayer_Umap;

		//m_Player_Umap�� srw��
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
		// �÷��̾� ���� �ڷᱸ�� �Լ�
		// ----------------------------

		// �α��� �� �÷��̾� ���� �ڷᱸ���� Insert
		//
		// Parameter : AccountNo, stPlayer*
		// return : ���� �߰� �� true
		//		  : Ű �ߺ� �� flase
		bool InsertLoginPlayerFunc(INT64 AccountNo, stPlayer* InsertPlayer);	

		// �α��� �� �÷��̾� ���� �ڷᱸ������ ����
		//
		// Parameter : AccountNo
		// return : ���������� Erase �� true
		//		  : ����  �˻� ���� �� false
		bool EraseLoginPlayerFunc(ULONGLONG AccountNo);




	public:
		// --------------
		// �ܺο��� ȣ�� ������ �Լ�
		// --------------

		// ���� ����
		//
		// Parameter : ����
		// return : ���� �� true
		//		  : ���� �� false



		// ���� ����
		//
		// Parameter : ����
		// return : ����



	private:
		// ------------------
		// ���ο����� ����ϴ� �Լ�
		// ------------------

		// �� ���� ��� �������� ���ڷ� ���� ��Ŷ ������.
		//
		// Parameter : stRoom* CProtocolBuff_Net*
		// return : �ڷᱸ���� 0���̸� false. �� �ܿ��� true
		bool Room_BroadCast(stRoom* NowRoom, CProtocolBuff_Net* SendBuff)
		{
			// �ڷᱸ�� ����� 0�̸� false
			size_t Size = NowRoom->m_JoinUser_vector.size();
			if (Size == 0)
				return false;

			// ����ȭ ���� ���۷��� ī��Ʈ ����
			SendBuff->Add((int)Size);

			// ó������ ��ȸ�ϸ� ������.
			size_t Index = 0;

			while (Index < Size)
			{
				SendPacket(NowRoom->m_JoinUser_vector[0], SendBuff);

				++Index;				
			}			

			return true;
		}



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


#endif // !__CHAT_SERVER_ROOM_H__
