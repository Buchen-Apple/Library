#ifndef __MONITOR_CLIENT_H__
#define __MONITOR_CLIENT_H__

#include "NetworkLib\NetworkLib_NetClient.h"
#include "LockFree_Queue\LockFree_Queue.h"
#include "ObjectPool\Object_Pool_LockFreeVersion.h"
#include "MonitorView.h"

// ----------------------
// ����͸� Ŭ���̾�Ʈ
// ----------------------
namespace Library_Jingyu
{
	class  CMonitorClient :public CNetClient
	{
		// ������ ���� �� ���� No
		enum en_Monitor_Type
		{
			dfMONITOR_ETC = 2,		// ä�ü��� �� ��� ����.
			dfMONITOR_CHATSERVER = 3		// ä�� ����
		};

		// ȭ�鿡 ����� ������ ���� ����ü
		struct stLastData
		{
			int m_ZeroCount = 0;	// �����͸� �������� 0�� ��µ�, �̰� ����ߴ��� üũ.
			BYTE m_ServerNo = 0;
			int m_Value = -1;
			
		};

		// �ϰ� ����ü
		struct st_WorkNode
		{
			WORD Type;	
			ULONGLONG SessionID;
			CProtocolBuff_Net* m_pPacket;
		};

		// �ϰ� TLS
		CMemoryPoolTLS<st_WorkNode>* m_WorkPool;

		// �ϰ� ���� ť
		CLF_Queue<st_WorkNode*> *m_LFQueue;

		// ���� ���� ID
		ULONGLONG m_ullSessionID;

		// �α��� ��Ŷ���� �޾Ƽ� �α��� ó���� ��������
		// true�� �α��� ��.
		bool m_bLoginCheck;

		// ����͸� ��� �迭
		// Ÿ�� �� ��ŭ ������ ����.
		// ���ʿ���, ��� nullptr
		CMonitorGraphUnit* m_pMonitor[dfMONITOR_DATA_TYPE_END-1] = { nullptr, };

		// �� ����ϱ� ��, ���������� ���� �����͸� �����ϴ� ���.
		stLastData m_LastData[dfMONITOR_DATA_TYPE_END - 1];

	public:
		// -----------------------
		// �����ڿ� �Ҹ���
		// -----------------------
		CMonitorClient();
		virtual ~CMonitorClient();


	public:
		// -----------------------
		// �ܺο��� ��� ������ ��� �Լ�
		// -----------------------

		// ����� Ŭ���� ����.
		// �ܺ���, WM_CREATE���� ȣ��
		//
		// Parameter : �ν��Ͻ�, ������ �ڵ�
		// retrun : ����
		void SetMonitorClass(HINSTANCE hInst, HWND hWnd);

		// 1�ʿ� 1ȸ ȣ��Ǵ� ������Ʈ ����
		//
		// Parameter : ����
		// return : ����
		void Update();

		// ����͸� Ŭ���̾�Ʈ ��ŸƮ
		// ���ο����� NetClient�� Start �Լ� ȣ��
		//
		// Parameter : ����
		// return : ���� �� false
		bool ClientStart();

		// ����͸� Ŭ���̾�Ʈ ��ž
		// ���ο����� NetClient�� Stop �Լ� ȣ��
		//
		// Parameter : ����
		// return : ����
		void ClientStop();


	private:
		// -----------------------
		// ���ο����� ��� ������ ��� �Լ�
		// -----------------------

		// �α��� ��û�� ���� ���� ó��
		//
		// Parameter : SessionID, Packet(Net)
		// return : ����
		void Login_Packet(ULONGLONG SessionID, CProtocolBuff_Net* Packet);

		// ������ ���� ��Ŷ �޾��� �� �ϴ� ��
		//
		// Parameter : SessionID, Packet(Net), stLastData*
		// return : ����
		void Data_Packet(ULONGLONG SessionID, CProtocolBuff_Net* Packet, stLastData* LastData);



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

#endif // !__MONITOR_CLIENT_H__

