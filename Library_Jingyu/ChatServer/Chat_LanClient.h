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

		// ��ūŰ�� �����ϴ� �ڷᱸ��
		// Key : AccountNO
		// Value : ��ū ����ü
		unordered_map<INT64, stToken*>* m_umapTokenCheck;

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
		virtual void OnConnect(ULONGLONG SessionID) = 0;

		// ��ǥ ������ ���� ���� �� ȣ��Ǵ� �Լ� (InDIsconnect �ȿ��� ȣ��)
		//
		// parameter : ����Ű
		// return : ����
		virtual void OnDisconnect(ULONGLONG SessionID) = 0;

		// ��Ŷ ���� �Ϸ� �� ȣ��Ǵ� �Լ�.
		//
		// parameter : ���� ����Ű, CProtocolBuff_Lan*
		// return : ����
		virtual void OnRecv(ULONGLONG SessionID, CProtocolBuff_Lan* Payload) = 0;

		// ��Ŷ �۽� �Ϸ� �� ȣ��Ǵ� �Լ�
		//
		// parameter : ���� ����Ű, Send �� ������
		// return : ����
		virtual void OnSend(ULONGLONG SessionID, DWORD SendSize) = 0;

		// ��Ŀ �����尡 ��� �� ȣ��Ǵ� �Լ�.
		// GQCS �ٷ� �ϴܿ��� ȣ��
		// 
		// parameter : ����
		// return : ����
		virtual void OnWorkerThreadBegin() = 0;

		// ��Ŀ �����尡 ���� �� ȣ��Ǵ� �Լ�
		// GQCS �ٷ� ������ ȣ��
		// 
		// parameter : ����
		// return : ����
		virtual void OnWorkerThreadEnd() = 0;

		// ���� �߻� �� ȣ��Ǵ� �Լ�.
		//
		// parameter : ���� �ڵ�(���� ������ �����ڵ�� WinGetLastError() �Լ��� ��� ����. ���� ��� 0�� ���ϵ�)
		//			 : ���� �ڵ忡 ���� ��Ʈ��
		// return : ����
		virtual void OnError(int error, const TCHAR* errorStr) = 0;

	};
}






#endif // !__CHAT_LANCLIENT_H__
