#ifndef __CHAT_LANCLIENT_H__
#define __CHAT_LANCLIENT_H__

#include "NetworkLib\NetworkLib_LanClinet.h"
#include "Protocol_Set\CommonProtocol.h"
#include "ObjectPool\Object_Pool_LockFreeVersion.h"
#include <unordered_map>

namespace Library_Jingyu
{
	// CLanClient�� ��ӹ޴� Chat_LanClient
	// !! ä�� ������ �α��� ������ �����ϱ� ���� �뵵 !!

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
		// Parameter : AccountNo, stToken*
		// return : �߰� ���� ��, true
		//		  : AccountNo�� �ߺ��� �� false
		bool InsertTokenFunc(INT64 AccountNo, stToken* isnertToken);


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

		// -----------------------
		// �ܺο��� ȣ���ϴ� �Լ�
		// -----------------------
		// Start �Լ�




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






#endif // !__CHAT_LANCLIENT_H__
