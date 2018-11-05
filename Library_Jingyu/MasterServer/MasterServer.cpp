#include "pch.h"
#include "MasterServer.h"


// -----------------------
//
// ������ Lan ����
// 
// -----------------------
namespace Library_Jingyu
{

	// -------------------------------------
	// ���ο����� ����ϴ� �Լ�
	// -------------------------------------

	// ���Ͽ��� Config ���� �о����
	// 
	// Parameter : config ����ü
	// return : ���������� ���� �� true
	//		  : �� �ܿ��� false
	bool CMasterServer_Lan::SetFile(stConfigFile* pConfig)
	{
		Parser Parser;

		// ���� �ε�
		try
		{
			Parser.LoadFile(_T("MasterServer_Config.ini"));
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
		// Matchmaking Net������ config �о����
		////////////////////////////////////////////////////////

		// ���� ���� -------------------------
		if (Parser.AreaCheck(_T("MASTERSERVER")) == false)
			return false;

		// BindIP
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
	// �ܺο��� ��� ������ �Լ�
	// -----------------------

	// ���� ����
	//
	// Parameter : ����
	// return : ���� �� false.
	bool CMasterServer_Lan::ServerStart()
	{
		// LanServer.Start �Լ� ȣ��
		if (Start(m_stConfig.BindIP, m_stConfig.Port, m_stConfig.CreateWorker, m_stConfig.ActiveWorker,
			m_stConfig.CreateAccept, m_stConfig.Nodelay, m_stConfig.MaxJoinUser) == false)
		{
			return false;
		}

		return true;		
	}

	// ���� ����
	//
	// Parameter : ����
	// return : ����
	void CMasterServer_Lan::ServerStop()
	{
		// LanServer.Stop()�Լ� ȣ��
		Stop();

		// ������ �� ������ �����Ѵ�.
	}





	// -----------------------
	// �����Լ�
	// -----------------------

	bool CMasterServer_Lan::OnConnectionRequest(TCHAR* IP, USHORT port)
	{
		return true;
	}

	void CMasterServer_Lan::OnClientJoin(ULONGLONG SessionID)
	{}

	void CMasterServer_Lan::OnClientLeave(ULONGLONG SessionID)
	{}

	void CMasterServer_Lan::OnRecv(ULONGLONG SessionID, CProtocolBuff_Lan* Payload)
	{}

	void CMasterServer_Lan::OnSend(ULONGLONG SessionID, DWORD SendSize)
	{}

	void CMasterServer_Lan::OnWorkerThreadBegin()
	{}

	void CMasterServer_Lan::OnWorkerThreadEnd()
	{}

	void CMasterServer_Lan::OnError(int error, const TCHAR* errorStr)
	{}




	// -----------------------
	// �����ڿ� �Ҹ���
	// -----------------------

	// ������
	CMasterServer_Lan::CMasterServer_Lan()
		:CLanServer()
	{
		// �̱��� �ޱ�
		m_CDump = CCrashDump::GetInstance();
		m_CLog = CSystemLog::GetInstance();

		//  Config���� ����		
		if (SetFile(&m_stConfig) == false)
			m_CDump->Crash();

		// �α� ������ ���� ����
		m_CLog->SetDirectory(L"MasterServer");
		m_CLog->SetLogLeve((CSystemLog::en_LogLevel)m_stConfig.LogLevel);

		// �� �ʱ�ȭ
		InitializeSRWLock(&m_srwl_ClientKey_Umap);
		InitializeSRWLock(&m_srwl_AccountNo_Umap);
		InitializeSRWLock(&m_srwl_MatchServer_Umap);
		InitializeSRWLock(&m_srwl_BattleServer_Umap);

		// TLS �����Ҵ�
		m_TLSPool_MatchServer = new CMemoryPoolTLS<CMatchingServer>(0, false);
		m_TLSPool_BattleServer = new CMemoryPoolTLS<CBattleServer>(0, false);
		m_TLSPool_Player = new CMemoryPoolTLS<CPlayer>(0, false);
		m_TLSPool_Room = new CMemoryPoolTLS<CRoom>(0, false);
	}

	// �Ҹ���
	CMasterServer_Lan::~CMasterServer_Lan()
	{
		// TLS ���� ����
		delete m_TLSPool_MatchServer;
		delete m_TLSPool_BattleServer;
		delete m_TLSPool_Player;
		delete m_TLSPool_Room;
	}

}