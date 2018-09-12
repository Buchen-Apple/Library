#include "pch.h"
#include "LoginServer.h"

// ----------------------------------
// LoginServer(LanServer)
// ----------------------------------
namespace Library_Jingyu
{
	// �θ� ����
	//
	// Parameter : CLogin_NetServer*
	// return : ����
	void CLogin_LanServer::ParentSet(CLogin_NetServer* parent)
	{
		m_cParentS = parent;
	}


	// GameServer, ChatServer�� LanClient�� ���ο� ���� ���� Send 
	// --- CLogin_NetServer�� "OnClientJoin"���� ȣ�� ---
	// 
	// Parameter : AccountNo, Token(64Byte)
	// return : ����
	bool CLogin_LanServer::UserJoinSend(INT64 AccountNo, char* Token)
	{

	}


	// ������
	CLogin_LanServer::CLogin_LanServer()
	{
		// �ϴ°� ����
	}

	// �Ҹ���
	CLogin_LanServer::~CLogin_LanServer()
	{
		// �ϴ°� ����
	}




	// -----------------------------------
	// �����Լ�
	// -----------------------------------

	// Accept ����, ȣ��ȴ�.
	//
	// parameter : ������ ������ IP, Port
	// return false : Ŭ���̾�Ʈ ���� �ź�
	// return true : ���� ���
	bool CLogin_LanServer::OnConnectionRequest(TCHAR* IP, USHORT port)
	{
		return true;
	}

	// ���� �� ȣ��Ǵ� �Լ� (AcceptThread���� Accept �� ȣ��)
	//
	// parameter : ������ �������� �Ҵ�� ����Ű
	// return : ����
	void CLogin_LanServer::OnClientJoin(ULONGLONG ClinetID)
	{

	}

	// ���� ���� �� ȣ��Ǵ� �Լ� (InDIsconnect �ȿ��� ȣ��)
	//
	// parameter : ���� ����Ű
	// return : ����
	void CLogin_LanServer::OnClientLeave(ULONGLONG ClinetID)
	{

	}

	// ��Ŷ ���� �Ϸ� �� ȣ��Ǵ� �Լ�.
	//
	// parameter : ���� ����Ű, ���� ��Ŷ
	// return : ����
	void CLogin_LanServer::OnRecv(ULONGLONG ClinetID, CProtocolBuff_Lan* Payload)
	{
		// 1. ���� ��Ŷ ������
		INT64 AccountNo;
		char Token[64];
		Payload->GetData((char*)&AccountNo, 8);
		Payload->GetData(Token, 64);

		// 2. AccountNo�� �ڷᱸ������ �˻�



		// 3. �Ķ���� ����



		// 4. NetServer�� UserLoginPacketSend�Լ� ȣ��
		m_cParentS->UserLoginPacketSend(AccountNo);
	}

	// ��Ŷ �۽� �Ϸ� �� ȣ��Ǵ� �Լ�
	//
	// parameter : ���� ����Ű, Send �� ������
	// return : ����
	void CLogin_LanServer::OnSend(ULONGLONG ClinetID, DWORD SendSize)
	{

	}

	// ��Ŀ �����尡 ��� �� ȣ��Ǵ� �Լ�.
	// GQCS �ٷ� �ϴܿ��� ȣ��
	// 
	// parameter : ����
	// return : ����
	void CLogin_LanServer::OnWorkerThreadBegin()
	{

	}

	// ��Ŀ �����尡 ���� �� ȣ��Ǵ� �Լ�
	// GQCS �ٷ� ������ ȣ��
	// 
	// parameter : ����
	// return : ����
	void CLogin_LanServer::OnWorkerThreadEnd()
	{

	}

	// ���� �߻� �� ȣ��Ǵ� �Լ�.
	//
	// parameter : ���� �ڵ�(���� ������ �����ڵ�� WinGetLastError() �Լ��� ��� ����. ���� ��� 0�� ���ϵ�)
	//			 : ���� �ڵ忡 ���� ��Ʈ��
	// return : ����
	void CLogin_LanServer::OnError(int error, const TCHAR* errorStr)
	{

	}




}



// ----------------------------------
// LoginServer(NetServer)
// ----------------------------------
namespace Library_Jingyu
{
	// ������
	CLogin_NetServer::CLogin_NetServer()
	{
		m_cLanS = new CLogin_LanServer;

		// LanServer�� this ����
		m_cLanS->ParentSet(this);
	}

	// �Ҹ���
	CLogin_NetServer::~CLogin_NetServer()
	{

	}



	// �������� �α��� �Ϸ� ������ Send
	// --- CLogin_LanServer�� "OnRecv"���� ȣ�� --- 
	//
	// Parameter : AccountNo
	// return : ����
	bool CLogin_NetServer::UserLoginPacketSend(INT64 AccountNo)
	{
		
		return true;
	}


	// Accept ����, ȣ��ȴ�.
	//
	// parameter : ������ ������ IP, Port
	// return false : Ŭ���̾�Ʈ ���� �ź�
	// return true : ���� ���
	bool CLogin_NetServer::OnConnectionRequest(TCHAR* IP, USHORT port)
	{
		return true;
	}

	// ���� �� ȣ��Ǵ� �Լ� (AcceptThread���� Accept �� ȣ��)
	//
	// parameter : ������ �������� �Ҵ�� ����Ű
	// return : ����
	void CLogin_NetServer::OnClientJoin(ULONGLONG SessionID)
	{
		
	}

	// ���� ���� �� ȣ��Ǵ� �Լ� (InDIsconnect �ȿ��� ȣ��)
	//
	// parameter : ���� ����Ű
	// return : ����
	void CLogin_NetServer::OnClientLeave(ULONGLONG SessionID)
	{}

	// ��Ŷ ���� �Ϸ� �� ȣ��Ǵ� �Լ�.
	//
	// parameter : ���� ����Ű, ���� ��Ŷ
	// return : ����
	void CLogin_NetServer::OnRecv(ULONGLONG SessionID, CProtocolBuff_Net* Payload)
	{
		// 1. ������


		// �������� ����, �α��μ����� ä�ü����� ���ο� ���� ������Ŷ ����
		m_cLanS->UserJoinSend();


	}

	// ��Ŷ �۽� �Ϸ� �� ȣ��Ǵ� �Լ�
	//
	// parameter : ���� ����Ű, Send �� ������
	// return : ����
	void CLogin_NetServer::OnSend(ULONGLONG SessionID, DWORD SendSize)
	{}

	// ��Ŀ �����尡 ��� �� ȣ��Ǵ� �Լ�.
	// GQCS �ٷ� �ϴܿ��� ȣ��
	// 
	// parameter : ����
	// return : ����
	void CLogin_NetServer::OnWorkerThreadBegin()
	{}

	// ��Ŀ �����尡 ���� �� ȣ��Ǵ� �Լ�
	// GQCS �ٷ� ������ ȣ��
	// 
	// parameter : ����
	// return : ����
	void CLogin_NetServer::OnWorkerThreadEnd()
	{}

	// ���� �߻� �� ȣ��Ǵ� �Լ�.
	//
	// parameter : ���� �ڵ�(���� ������ �����ڵ�� WinGetLastError() �Լ��� ��� ����. ���� ��� 0�� ���ϵ�)
	//			 : ���� �ڵ忡 ���� ��Ʈ��
	// return : ����
	void CLogin_NetServer::OnError(int error, const TCHAR* errorStr)
	{}





}