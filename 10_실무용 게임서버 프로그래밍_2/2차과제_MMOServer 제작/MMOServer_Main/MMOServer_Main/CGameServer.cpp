#include "pch.h"
#include "CGameServer.h"
#include "Parser\Parser_Class.h"



// ------------------
// CGameSession�� �Լ�
// (CGameServer�� �̳�Ŭ����)
// ------------------
namespace Library_Jingyu
{
	// -----------------------
	// �����ڿ� �Ҹ���
	// -----------------------

	// ������
	CGameServer::CGameSession::CGameSession()
		:CMMOServer::cSession()
	{
		// �Ұ� ����
	}

	// �Ҹ���
	CGameServer::CGameSession::~CGameSession()
	{
		// �Ұ� ����
	}

	// -----------------
	// �����Լ�
	// -----------------


	// --------------- AUTH ���� �Լ�

	// ������ Auth ���� �����
	//
	// Parameter : ����
	// return : ����
	void CGameServer::CGameSession::OnAuth_ClientJoin()
	{

	}

	// ������ Auth ��忡�� ����
	//
	// Parameter : ����
	// return : ����
	void CGameServer::CGameSession::OnAuth_ClientLeave()
	{

	}

	// Auth ����� �������� packet�� ��
	//
	// Parameter : ��Ŷ (CProtocolBuff_Net*)
	// return : ����
	void CGameServer::CGameSession::OnAuth_Packet(CProtocolBuff_Net* Packet)
	{
		// � ��Ŷ�� ������ GAME���� ����.
	}



	// --------------- GAME ���� �Լ�

	// ������ Game���� �����
	//
	// Parameter : ����
	// return : ����
	void CGameServer::CGameSession::OnGame_ClientJoin()
	{

	}

	// ������ Game��忡�� ����
	//
	// Parameter : ����
	// return : ����
	void CGameServer::CGameSession::OnGame_ClientLeave()
	{

	}

	// Game ����� �������� packet�� ��
	//
	// Parameter : ��Ŷ (CProtocolBuff_Net*)
	// return : ����
	void CGameServer::CGameSession::OnGame_Packet(CProtocolBuff_Net* Packet)
	{

	}



	// --------------- Release ���� �Լ�
	
	// Release�� ����.
	//
	// Parameter : ����
	// return : ����
	void CGameServer::CGameSession::OnGame_ClientRelease()
	{

	}
}

// ---------------
// CGameServer
// CMMOServer�� ��ӹ޴� ���� ����
// ---------------
namespace Library_Jingyu
{
	// -----------------
	// �����ڿ� �Ҹ���
	// -----------------
	CGameServer::CGameServer()
		:CMMOServer()
	{
		m_GameServerDump = CCrashDump::GetInstance();

		// ���� �о����
		// ------------------- Config���� ����		
		if (SetFile(&m_stConfig) == false)
			m_GameServerDump->Crash();
	}

	CGameServer::~CGameServer()
	{
	}

	// GameServerStart
	// ���������� CMMOServer�� Start, ���� ���ñ��� �Ѵ�.
	//
	// Parameter : ����
	// return : ���� �� false
	bool CGameServer::GameServerStart()
	{
		// 1. ���� ����
		m_cGameSession = new CGameSession[m_stConfig.MaxJoinUser];
		SetSession(m_cGameSession, m_stConfig.MaxJoinUser, m_stConfig.HeadCode, m_stConfig.XORCode1, m_stConfig.XORCode2);
		
		// 2. ���� ����
		if (Start(m_stConfig.BindIP, m_stConfig.Port, m_stConfig.CreateWorker, m_stConfig.ActiveWorker, m_stConfig.CreateAccept, 
			m_stConfig.Nodelay, m_stConfig.MaxJoinUser, m_stConfig.HeadCode, m_stConfig.XORCode1, m_stConfig.XORCode2) == false)
			return false;			

		return true;
	}

	// GameServerStop
	// ���������� Stop ����
	//
	// Parameter : ����
	// return : ����
	void CGameServer::GameServerStop()
	{
		// 1. ���� ����
		if (GetServerState() == true)
			Stop();

		// 2. ���� ����
		delete[] m_cGameSession;
	}





	// -----------------------
	// ���ο����� ����ϴ� �Լ�
	// -----------------------

	// ���Ͽ��� Config ���� �о����
	// 
	// Parameter : config ����ü
	// return : ���������� ���� �� true
	//		  : �� �ܿ��� false
	bool CGameServer::SetFile(stConfigFile* pConfig)
	{
		Parser Parser;

		// ���� �ε�
		try
		{
			Parser.LoadFile(_T("MMOGameServer_Config.ini"));
		}
		catch (int expn)
		{
			if (expn == 1)
			{
				printf("File Open Fail...\n");
				return false;
			}
			else if (expn == 2)
			{
				printf("FileR Read Fail...\n");
				return false;
			}
		}

		////////////////////////////////////////////////////////
		// ChatServer config �о����
		////////////////////////////////////////////////////////

		// ���� ���� -------------------------
		if (Parser.AreaCheck(_T("MMOGAMESERVER")) == false)
			return false;

		// IP
		if (Parser.GetValue_String(_T("BindIP"), pConfig->BindIP) == false)
			return false;

		// Port
		if (Parser.GetValue_Int(_T("Port"), &pConfig->Port) == false)
			return false;

		// ���� ��Ŀ ��
		if (Parser.GetValue_Int(_T("CreateWorker"), &pConfig->CreateWorker) == false)
			return false;

		// Ȱ��ȭ ��Ŀ ��
		if (Parser.GetValue_Int(_T("ActiveWorker"), &pConfig->ActiveWorker) == false)
			return false;

		// ���� ����Ʈ
		if (Parser.GetValue_Int(_T("CreateAccept"), &pConfig->CreateAccept) == false)
			return false;

		// ��� �ڵ�
		if (Parser.GetValue_Int(_T("HeadCode"), &pConfig->HeadCode) == false)
			return false;

		// xorcode1
		if (Parser.GetValue_Int(_T("XorCode1"), &pConfig->XORCode1) == false)
			return false;

		// xorcode2
		if (Parser.GetValue_Int(_T("XorCode2"), &pConfig->XORCode2) == false)
			return false;

		// Nodelay
		if (Parser.GetValue_Int(_T("Nodelay"), &pConfig->Nodelay) == false)
			return false;

		// �ִ� ���� ���� ���� ��
		if (Parser.GetValue_Int(_T("MaxJoinUser"), &pConfig->MaxJoinUser) == false)
			return false;

		// �α� ����
		if (Parser.GetValue_Int(_T("LogLevel"), &pConfig->LogLevel) == false)
			return false;

		return true;
	}





	// -----------------------
	// �����Լ�
	// -----------------------

	// AuthThread���� 1Loop���� 1ȸ ȣ��.
	// 1�������� ���������� ó���ؾ� �ϴ� ���� �Ѵ�.
	// 
	// parameter : ����
	// return : ����
	void CGameServer::OnAuth_Update()
	{

	}

	// GameThread���� 1Loop���� 1ȸ ȣ��.
	// 1�������� ���������� ó���ؾ� �ϴ� ���� �Ѵ�.
	// 
	// parameter : ����
	// return : ����
	void CGameServer::OnGame_Update()
	{}

	// ���ο� ���� ���� ��, Auth���� ȣ��ȴ�.
	//
	// parameter : ������ ������ IP, Port
	// return false : Ŭ���̾�Ʈ ���� �ź�
	// return true : ���� ���
	bool CGameServer::OnConnectionRequest(TCHAR* IP, USHORT port)
	{
		return true;
	}

	// ��Ŀ �����尡 ��� �� ȣ��Ǵ� �Լ�.
	// GQCS �ٷ� �ϴܿ��� ȣ��
	// 
	// parameter : ����
	// return : ����
	void CGameServer::OnWorkerThreadBegin()
	{}

	// ��Ŀ �����尡 ���� �� ȣ��Ǵ� �Լ�
	// GQCS �ٷ� ������ ȣ��
	// 
	// parameter : ����
	// return : ����
	void CGameServer::OnWorkerThreadEnd()
	{}

	// ���� �߻� �� ȣ��Ǵ� �Լ�.
	//
	// parameter : ���� �ڵ�(���� ������ �����ڵ�� WinGetLastError() �Լ��� ��� ����. ���� ��� 0�� ���ϵ�)
	//			 : ���� �ڵ忡 ���� ��Ʈ��
	// return : ����
	void CGameServer::OnError(int error, const TCHAR* errorStr)
	{}



}