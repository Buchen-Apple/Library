#include "pch.h"
#include "BattleServer_Room_Version.h"
#include "Parser\Parser_Class.h"
#include "Protocol_Set\CommonProtocol_2.h"		// ���� �Ϲ����� ���� �ʿ�
#include "Log\Log.h"
#include "CPUUsage\CPUUsage.h"
#include "PDHClass\PDHCheck.h"

#include "rapidjson\document.h"
#include "rapidjson\writer.h"
#include "rapidjson\stringbuffer.h"

#include <process.h>

using namespace rapidjson;

extern ULONGLONG g_ullAcceptTotal_MMO;
extern LONG	  g_lAcceptTPS_MMO;
extern LONG	g_lSendPostTPS_MMO;
extern LONG	g_lRecvTPS_MMO;

extern LONG g_lAllocNodeCount;
extern LONG g_lAllocNodeCount_Lan;

extern LONG g_lAuthModeUserCount;
extern LONG g_lGameModeUserCount;

extern LONG g_lAuthFPS;
extern LONG g_lGameFPS;

LONG g_lShowAuthFPS;
LONG g_lShowGameFPS;

// ��Ʋ���� ���� ��ū ����
LONG g_lBattleEnterTokenError;

// auth�� �α��� ��Ŷ����, DB ���� �� ����� -10(����� ����)�� �� ���
LONG g_lQuery_Result_Not_Find;

// auth�� �α��� ��Ŷ����, DB ���� �� ����� -10�� �ƴѵ� 1�� �ƴ� ������ ��� 
LONG g_lTempError;

// auth�� �α��� ��Ŷ����, ������ SessionKey(��ū)�� �ٸ� ���
LONG g_lTokenError;

// auth�� �α��� ��Ŷ����, ������ ��� �� ������ �ٸ� ���
LONG g_lVerError;

// GQCS���� �������� ���� �� 1 ����
extern LONG g_SemCount;





// ------------------
// CGameSession�� �Լ�
// (CBattleServer_Room�� �̳�Ŭ����)
// ------------------
namespace Library_Jingyu
{
	// ������
	CCrashDump* g_BattleServer_Room_Dump = CCrashDump::GetInstance();

	// �α׿�
	CSystemLog* g_BattleServer_RoomLog = CSystemLog::GetInstance();


	// -----------------------
	// �����ڿ� �Ҹ���
	// -----------------------

	// ������
	CBattleServer_Room::CGameSession::CGameSession()
		:CMMOServer::cSession()
	{
		// AccountNo �ʱⰪ ����
		m_Int64AccountNo = 0x0fffffffffffffff;
	}

	// �Ҹ���
	CBattleServer_Room::CGameSession::~CGameSession()
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
	void CBattleServer_Room::CGameSession::OnAuth_ClientJoin()
	{
		// �Ұ� ����
	}

	// ������ Auth ��忡�� ����
	//
	// Parameter : Game���� ����Ȱ����� �˷��ִ� Flag. ����Ʈ false.
	// return : ����
	void CBattleServer_Room::CGameSession::OnAuth_ClientLeave(bool bGame)
	{

		// ���� ���� ���ᰡ �ƴ϶� ��� ������ ���, �α��� ���� ��Ŷ ����
		if (bGame == true)
		{
			// Alloc
			CProtocolBuff_Net* SendBuff = CProtocolBuff_Net::Alloc();

			// Ÿ��
			WORD Type = en_PACKET_CS_GAME_RES_LOGIN;
			SendBuff->PutData((char*)&Type, 2);

			// Status
			BYTE Status = 1;
			SendBuff->PutData((char*)&Status, 1);

			// AccountNo
			SendBuff->PutData((char*)&m_Int64AccountNo, 8);

			// SendPacket
			SendPacket(SendBuff);
		}

	}

	// Auth ����� �������� packet�� ��
	//
	// Parameter : ��Ŷ (CProtocolBuff_Net*)
	// return : ����
	void CBattleServer_Room::CGameSession::OnAuth_Packet(CProtocolBuff_Net* Packet)
	{
		// ��Ŷ ó��
		try
		{
			// 1. Ÿ�� ����
			WORD Type;
			Packet->GetData((char*)&Type, 2);

			// 2. Ÿ�Կ� ���� �б� ó��
			switch (Type)
			{
				// �α��� ��û
			case en_PACKET_CS_GAME_REQ_LOGIN:
				Auth_LoginPacket(Packet);
				break;

				// ��Ʈ��Ʈ
			case en_PACKET_CS_GAME_REQ_HEARTBEAT:
				break;

				// �� �ܿ��� ��������.
			default:
				throw CException(_T("OnAuth_Packet() --> Type Error!!"));
				break;
			}
		}
		catch (CException& exc)
		{
			// �α� ��� (�α� ���� : ����)
			g_BattleServer_RoomLog->LogSave(L"CBattleServer_Room", CSystemLog::en_LogLevel::LEVEL_ERROR, L"%s",
				(TCHAR*)exc.GetExceptionText());

			// ����
			g_BattleServer_Room_Dump->Crash();
		}

	}



	// --------------- GAME ���� �Լ�

	// ������ Game���� �����
	//
	// Parameter : ����
	// return : ����
	void CBattleServer_Room::CGameSession::OnGame_ClientJoin()
	{
		// �Ұ� ����
	}

	// ������ Game��忡�� ����
	//
	// Parameter : ����
	// return : ����
	void CBattleServer_Room::CGameSession::OnGame_ClientLeave()
	{
		// �Ұ� ����
	}

	// Game ����� �������� packet�� ��
	//
	// Parameter : ��Ŷ (CProtocolBuff_Net*)
	// return : ����
	void CBattleServer_Room::CGameSession::OnGame_Packet(CProtocolBuff_Net* Packet)
	{
		// Ÿ�Կ� ���� ��Ŷ ó��
		try
		{
			// 1. Ÿ�� ����
			WORD Type;
			Packet->GetData((char*)&Type, 2);

			// 2. Ÿ�Կ� ���� �б⹮ ó��
			switch (Type)
			{	

				// ��Ʈ��Ʈ
			case en_PACKET_CS_GAME_REQ_HEARTBEAT:
				break;

				// �� �ܿ��� ���� ����
			default:
				throw CException(_T("OnGame_Packet() --> Type Error!!"));
				break;
			}

		}
		catch (CException& exc)
		{
			// �α� ��� (�α� ���� : ����)
			g_BattleServer_RoomLog->LogSave(L"CBattleServer_Room", CSystemLog::en_LogLevel::LEVEL_ERROR, L"%s",
				(TCHAR*)exc.GetExceptionText());

			// ����
			g_BattleServer_Room_Dump->Crash();
		}

	}



	// --------------- Release ���� �Լ�

	// Release�� ����.
	//
	// Parameter : ����
	// return : ����
	void CBattleServer_Room::CGameSession::OnGame_ClientRelease()
	{
		// �ڷᱸ������ ����
		//m_pParent->AccountNO_Erase(m_Int64AccountNo);

		// AccountNo �ʱ�ȭ
		m_Int64AccountNo = 0x0fffffffffffffff;
	}




	// -----------------
	// ��Ŷ ó�� �Լ�
	// -----------------

	// �α��� ��û 
	//
	// Parameter : CProtocolBuff_Net*
	// return : ����
	void CBattleServer_Room::CGameSession::Auth_LoginPacket(CProtocolBuff_Net* Packet)
	{
		// 1. ������
		INT64 AccountNo;
		char SessionKey[64];
		char ConnectToken[32];
		UINT Version;

		Packet->GetData((char*)&AccountNo, 8);
		Packet->GetData(SessionKey, 64);
		Packet->GetData(ConnectToken, 32);
		Packet->GetData((char*)&Version, 4);

		// 2. ��Ʋ���� ���� ��ū ��
		// "����" ��ū�� ���� ��
		if (memcmp(ConnectToken, m_pParent->cConnectToken_Now, 32) != 0)
		{
			// �ٸ��ٸ� "����" ��ū�� ��
			if (memcpy(ConnectToken, m_pParent->m_cConnectToken_Before, 32) != 0)
			{
				InterlockedIncrement(&g_lBattleEnterTokenError);

				// �׷��� �ٸ��ٸ� �̻��� ������ �Ǵ�.
				// ��Ʋ���� ���� ��ū�� �ٸ��ٴ� ��Ŷ ������.
				CProtocolBuff_Net* SendBuff = CProtocolBuff_Net::Alloc();

				WORD Type = en_PACKET_CS_GAME_RES_LOGIN;
				BYTE Result = 3;	// ����� ����Ű ������ ���� ���. ���� �����ɱ�?

				SendBuff->PutData((char*)&Type, 2);
				SendBuff->PutData((char*)&AccountNo, 8);
				SendBuff->PutData((char*)&Result, 1);

				SendPacket(SendBuff);

				return;
			}
		}		

		// 3. ���� �� 
		if (m_pParent->m_uiVer_Code != Version)
		{
			// ������ �ٸ���� Result 5(���� ����)�� ������.
			InterlockedIncrement(&g_lVerError);

			WORD Type = en_PACKET_CS_MATCH_RES_LOGIN;
			BYTE Result = 5;

			CProtocolBuff_Net* SendBuff = CProtocolBuff_Net::Alloc();

			SendBuff->PutData((char*)&Type, 2);
			SendBuff->PutData((char*)&AccountNo, 8);
			SendBuff->PutData((char*)&Result, 1);

			SendPacket(SendBuff);
			return;
		}


		// AccountNo �ڷᱸ���� �߰�.	----------------------
		// �̹� ������(false ����) �ߺ� �α������� ó��
		/*
		if (m_pParent->AccountNO_Insert(AccountNo) == false)
		{
			// �α��� ���� ��Ŷ ������ ����.
			CProtocolBuff_Net* SendBuff = CProtocolBuff_Net::Alloc();

			// Ÿ��
			WORD Type = en_PACKET_CS_GAME_RES_LOGIN;
			SendBuff->PutData((char*)&Type, 2);

			// Status
			BYTE Status = 0;		// ����
			SendBuff->PutData((char*)&Status, 1);

			// AccountNo
			SendBuff->PutData((char*)&AccountNo, 8);

			InterlockedIncrement(&g_DuplicateCount);

			// SendPacket
			SendPacket(SendBuff, TRUE);

			return;
		}
		*/


		// 4. m_Int64AccountNo�� ���� 0x0fffffffffffffff�� �ƴϸ�, ���� �̻���.
		// �ڵ� �߸��̶�� �ۿ� �� ��.
		if (m_Int64AccountNo != 0x0fffffffffffffff)
		{
			throw CException(_T("Auth_LoginPacket() --> AccountNo Error!!"));
		}

		// 5. �� ����
		m_Int64AccountNo = AccountNo;

		// 6. ����Ű Ȯ���� ���� DB_Read�����忡�� �� ��Ŵ
		DB_Read_Start();
		//PostQueuedCompletionStatus(m_pParent->m_hDBRead_IOCPHandle, 0, 0, 0); << 3��° ���ڿ� DB_WORK ������ ��

		// 5. �����̸�, ��带 AUTH_TO_GAME���� ���� ��û.	
		//SetMode_GAME();
	}
}


// ----------------------------------------
// 
// MMOServer�� �̿��� ��Ʋ ����. �� ����
//
// ----------------------------------------
namespace Library_Jingyu
{
	// Net ����ȭ ���� 1���� ũ�� (Byte)
	LONG g_lNET_BUFF_SIZE = 512;


	// -----------------------
	// ������
	// -----------------------

	// DB_Read�� ������
	// IOCP�� ������ Ǯ�� �̿��� �����ȴ�.
	UINT WINAPI	CBattleServer_Room::DB_Read_Thread(LPVOID lParam)
	{
		CBattleServer_Room* gThis = (CBattleServer_Room*)lParam;

		HANDLE hIOCP = gThis->m_hDBRead_IOCPHandle;

		DWORD cbTransferred;
		DB_WORK* stNowWork;
		OVERLAPPED* overlapped;

		// HTTP�� ����ϴ� ����
		HTTP_Exchange m_HTTP_Post((TCHAR*)_T("127.0.0.1"), 80);

		while (1)
		{
			// GQCS�� ���
			GetQueuedCompletionStatus(hIOCP, &cbTransferred, (PULONG_PTR)&stNowWork, &overlapped, INFINITE);

			// ���� ��ȣ�� ��� ó�� 


			// ���� ��ȣ�� �ƴ϶�� �ϰ� ó��
			try
			{
				switch (stNowWork->m_wType)
				{
					// �α��� ��Ŷ�� ���� ó��
					// ���� Ȯ�� �� �������.
				case eu_DB_READ_TYPE::eu_LOGIN:
				{
					// 1. ������
					INT64 AccountNo;
					char SessionKey[32];
					stNowWork->m_pWorkBuff->GetData((char*)&AccountNo, 8);
					stNowWork->m_pWorkBuff->GetData(SessionKey, 32);


					// 2. AccountNo�� DB���� ã�ƿ���
					// DB�� ���� ������ Ȯ��.
					TCHAR RequestBody[2000];
					TCHAR Body[1000];

					ZeroMemory(RequestBody, sizeof(RequestBody));
					ZeroMemory(Body, sizeof(Body));

					swprintf_s(Body, _Mycountof(Body), L"{\"accountno\" : %lld}", AccountNo);
					if (m_HTTP_Post.HTTP_ReqANDRes((TCHAR*)_T("Contents/Select_account.php"), Body, RequestBody) == false)
						g_BattleServer_Room_Dump->Crash();

					// Json������ �Ľ��ϱ� (UTF-16)
					GenericDocument<UTF16<>> Doc;
					Doc.Parse(RequestBody);



					// 3. DB ��� üũ
					int iResult = Doc[_T("result")].GetInt();

					// ����� 1�� �ƴ϶��, 
					if (iResult != 1)
					{
						WORD Type = en_PACKET_CS_GAME_RES_LOGIN;
						BYTE Result;

						// ����� -10�� ��� (ȸ������ ��ü�� �ȵǾ� ����)
						if (iResult == -10)
						{
							Result = 2;	// ����� ����
							InterlockedIncrement(&g_lQuery_Result_Not_Find);
						}

						// �� �� ��Ÿ ������ ���
						else
						{
							Result = 2;
							InterlockedIncrement(&g_lTempError);
						}

						CProtocolBuff_Net* SendBuff = CProtocolBuff_Net::Alloc();

						SendBuff->PutData((char*)&Type, 2);
						SendBuff->PutData((char*)&AccountNo, 8);
						SendBuff->PutData((char*)&Result, 1);

						stNowWork->m_pNowSession->SendPacket(SendBuff);
						break;
					}


					// 4. ����� 1�̶��, ��ūŰ �� 
					const TCHAR* tDBToekn = Doc[_T("sessionkey")].GetString();

					char DBToken[64];
					int len = (int)_tcslen(tDBToekn);
					WideCharToMultiByte(CP_UTF8, 0, tDBToekn, (int)_tcslen(tDBToekn), DBToken, len, NULL, NULL);

					if (memcmp(DBToken, SessionKey, 64) != 0)
					{
						// ��ū�� �ٸ���� Result 3(����Ű ����)�� ������.
						InterlockedIncrement(&g_lTokenError);

						WORD Type = en_PACKET_CS_MATCH_RES_LOGIN;
						BYTE Result = 3;

						CProtocolBuff_Net* SendBuff = CProtocolBuff_Net::Alloc();

						SendBuff->PutData((char*)&Type, 2);
						SendBuff->PutData((char*)&AccountNo, 8);
						SendBuff->PutData((char*)&Result, 1);

						stNowWork->m_pNowSession->SendPacket(SendBuff);
						break;
					}


					// 5. �����̸� ���� ���� ��Ŷ ����
					CProtocolBuff_Net* SendBuff = CProtocolBuff_Net::Alloc();

					WORD Type = en_PACKET_CS_MATCH_RES_LOGIN;
					BYTE Result = 1;

					SendBuff->PutData((char*)&Type, 2);
					SendBuff->PutData((char*)&AccountNo, 8);
					SendBuff->PutData((char*)&Result, 1);

					stNowWork->m_pNowSession->SendPacket(SendBuff);
				}
				break;

				default:
					break;
				}

			}

			catch (CException& exc)
			{

			}	

			// DB_Read_End()�Լ� ȣ��. ���ο����� I/Oī��Ʈ ����.

			// DB_WORK ������ ����ȭ ���� FREE
			CProtocolBuff_Net::Free(stNowWork->m_pWorkBuff);

			// DB_WORK Free

		}

		return 0;
	}






	// -----------------------
	// ���ο����� ����ϴ� �Լ�
	// -----------------------

	// ���Ͽ��� Config ���� �о����
	// 
	// Parameter : config ����ü
	// return : ���������� ���� �� true
	//		  : �� �ܿ��� false
	bool CBattleServer_Room::SetFile(stConfigFile* pConfig)
	{
		Parser Parser;

		// ���� �ε�
		try
		{
			Parser.LoadFile(_T("BattleServer.ini"));
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
		// BATTLESERVER config �о����
		////////////////////////////////////////////////////////

		// ���� ���� -------------------------
		if (Parser.AreaCheck(_T("BATTLESERVER")) == false)
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




		////////////////////////////////////////////////////////
		// �⺻ config �о����
		////////////////////////////////////////////////////////

		// ���� ���� -------------------------
		if (Parser.AreaCheck(_T("CONFIG")) == false)
			return false;

		// VerCode
		if (Parser.GetValue_Int(_T("VerCode"), &m_uiVer_Code) == false)
			return false;






		////////////////////////////////////////////////////////
		// ������ LanClient config �о����
		////////////////////////////////////////////////////////

		// ���� ���� -------------------------
		if (Parser.AreaCheck(_T("MASTERLANCLIENT")) == false)
			return false;

		// IP
		if (Parser.GetValue_String(_T("MasterServerIP"), pConfig->MasterServerIP) == false)
			return false;

		// Port
		if (Parser.GetValue_Int(_T("MasterServerPort"), &pConfig->MasterServerPort) == false)
			return false;

		// ���� ��Ŀ ��
		if (Parser.GetValue_Int(_T("MasterClientCreateWorker"), &pConfig->MasterClientCreateWorker) == false)
			return false;

		// Ȱ��ȭ ��Ŀ ��
		if (Parser.GetValue_Int(_T("MasterClientActiveWorker"), &pConfig->MasterClientActiveWorker) == false)
			return false;


		// Nodelay
		if (Parser.GetValue_Int(_T("MasterClientNodelay"), &pConfig->MasterClientNodelay) == false)
			return false;




		////////////////////////////////////////////////////////
		// ����͸� LanClient config �о����
		////////////////////////////////////////////////////////

		// ���� ���� -------------------------
		if (Parser.AreaCheck(_T("MONITORLANCLIENT")) == false)
			return false;

		// IP
		if (Parser.GetValue_String(_T("MonitorServerIP"), pConfig->MonitorServerIP) == false)
			return false;

		// Port
		if (Parser.GetValue_Int(_T("MonitorServerPort"), &pConfig->MonitorServerPort) == false)
			return false;

		// ���� ��Ŀ ��
		if (Parser.GetValue_Int(_T("MonitorClientCreateWorker"), &pConfig->MonitorClientCreateWorker) == false)
			return false;

		// Ȱ��ȭ ��Ŀ ��
		if (Parser.GetValue_Int(_T("MonitorClientActiveWorker"), &pConfig->MonitorClientActiveWorker) == false)
			return false;


		// Nodelay
		if (Parser.GetValue_Int(_T("MonitorClientNodelay"), &pConfig->MonitorClientNodelay) == false)
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
	void CBattleServer_Room::OnAuth_Update()
	{


	}

	// GameThread���� 1Loop���� 1ȸ ȣ��.
	// 1�������� ���������� ó���ؾ� �ϴ� ���� �Ѵ�.
	// 
	// parameter : ����
	// return : ����
	void CBattleServer_Room::OnGame_Update()
	{

	}

	// ���ο� ���� ���� ��, Auth���� ȣ��ȴ�.
	//
	// parameter : ������ ������ IP, Port
	// return false : Ŭ���̾�Ʈ ���� �ź�
	// return true : ���� ���
	bool CBattleServer_Room::OnConnectionRequest(TCHAR* IP, USHORT port)
	{
		return true;
	}

	// ��Ŀ �����尡 ��� �� ȣ��Ǵ� �Լ�.
	// GQCS �ٷ� �ϴܿ��� ȣ��
	// 
	// parameter : ����
	// return : ����
	void CBattleServer_Room::OnWorkerThreadBegin()
	{

	}

	// ��Ŀ �����尡 ���� �� ȣ��Ǵ� �Լ�
	// GQCS �ٷ� ������ ȣ��
	// 
	// parameter : ����
	// return : ����
	void CBattleServer_Room::OnWorkerThreadEnd()
	{

	}

	// ���� �߻� �� ȣ��Ǵ� �Լ�.
	//
	// parameter : ���� �ڵ�(���� ������ �����ڵ�� WinGetLastError() �Լ��� ��� ����. ���� ��� 0�� ���ϵ�)
	//			 : ���� �ڵ忡 ���� ��Ʈ��
	// return : ����
	void CBattleServer_Room::OnError(int error, const TCHAR* errorStr)
	{
		// �α� ��� (�α� ���� : ����)
		g_BattleServer_RoomLog->LogSave(L"CBattleServer_Room", CSystemLog::en_LogLevel::LEVEL_ERROR, L"%s (ErrorCode : %d)",
			errorStr, error);
	}

	   	  

	// -----------------
	// �����ڿ� �Ҹ���
	// -----------------
	CBattleServer_Room::CBattleServer_Room()
		:CMMOServer()
	{		
		// ------------------- Config���� ����		
		if (SetFile(&m_stConfig) == false)
			g_BattleServer_Room_Dump->Crash();

		// ------------------- �α� ������ ���� ����
		g_BattleServer_RoomLog->SetDirectory(L"CBattleServer_Room");
		g_BattleServer_RoomLog->SetLogLeve((CSystemLog::en_LogLevel)m_stConfig.LogLevel);
				
		// ����͸� ������ ����ϱ� ���� LanClient �����Ҵ�
		m_Monitor_LanClient = new CGame_MinitorClient;
		m_Monitor_LanClient->ParentSet(this);

		// U_set�� reserve ����.
		// �̸� ���� ��Ƶд�.
		m_setAccount.reserve(m_stConfig.MaxJoinUser);

		// U_set�� SRW�� �ʱ�ȭ
		InitializeSRWLock(&m_setSrwl);
	}

	CBattleServer_Room::~CBattleServer_Room()
	{
		// ����͸� ������ ����ϴ� LanClient ��������
		delete m_Monitor_LanClient;
	}

}


// ---------------
// CGame_MinitorClient
// CLanClienet�� ��ӹ޴� ����͸� Ŭ��
// ---------------
namespace Library_Jingyu
{
	// -----------------------
	// �����ڿ� �Ҹ���
	// -----------------------
	CGame_MinitorClient::CGame_MinitorClient()
		:CLanClient()
	{
		// ����͸� ���� �������� �����带 �����ų �̺�Ʈ ����
		//
		// �ڵ� ���� Event 
		// ���� ���� �� non-signalled ����
		// �̸� ���� Event	
		m_hMonitorThreadExitEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

	}

	CGame_MinitorClient::~CGame_MinitorClient()
	{
		// ���� ������ �Ǿ�������, ���� ����
		if (GetClinetState() == true)
			ClientStop();

		// �̺�Ʈ ����
		CloseHandle(m_hMonitorThreadExitEvent);
	}



	// -----------------------
	// �ܺο��� ��� ������ �Լ�
	// -----------------------

	// ���� �Լ�
	// ����������, ��ӹ��� CLanClient�� Startȣ��.
	//
	// Parameter : ������ ������ IP, ��Ʈ, ��Ŀ������ ��, Ȱ��ȭ��ų ��Ŀ������ ��, TCP_NODELAY ��� ����(true�� ���)
	// return : ���� �� true , ���� �� falsel 
	bool CGame_MinitorClient::ClientStart(TCHAR* ConnectIP, int Port, int CreateWorker, int ActiveWorker, int Nodelay)
	{
		// ����͸� ������ ����
		if (Start(ConnectIP, Port, CreateWorker, ActiveWorker, Nodelay) == false)
			return false;

		// ����͸� ������ ���� ������ ������ ����
		m_hMonitorThread = (HANDLE)_beginthreadex(NULL, 0, MonitorThread, this, 0, NULL);

		return true;
	}

	// ���� �Լ�
	// ����������, ��ӹ��� CLanClient�� Stopȣ��.
	// �߰���, ���ҽ� ���� ��
	//
	// Parameter : ����
	// return : ����
	void CGame_MinitorClient::ClientStop()
	{
		// 1. ����͸� ���� �������� ������ ����
		SetEvent(m_hMonitorThreadExitEvent);

		// ���� ���
		if (WaitForSingleObject(m_hMonitorThread, INFINITE) == WAIT_FAILED)
		{
			DWORD Error = GetLastError();
			printf("MonitorThread Exit Error!!! (%d) \n", Error);
		}

		// 2. ������ �ڵ� ��ȯ
		CloseHandle(m_hMonitorThread);

		// 3. ����͸� ������ ���� ����
		Stop();
	}

	// ��Ʋ������ this�� �Է¹޴� �Լ�
	// 
	// Parameter : ��Ʋ ������ this
	// return : ����
	void CGame_MinitorClient::ParentSet(CBattleServer_Room* ChatThis)
	{
		m_BattleServer_this = ChatThis;
	}




	// -----------------------
	// ���ο����� ����ϴ� ��� �Լ�
	// -----------------------

	// ���� �ð����� ����͸� ������ ������ �����ϴ� ������
	UINT	WINAPI CGame_MinitorClient::MonitorThread(LPVOID lParam)
	{
		// this �޾Ƶα�
		CGame_MinitorClient* g_This = (CGame_MinitorClient*)lParam;

		// ���� ��ȣ �̺�Ʈ �޾Ƶα�
		HANDLE* hEvent = &g_This->m_hMonitorThreadExitEvent;

		// CPU ����� üũ Ŭ���� (ä�ü��� ����Ʈ����)
		CCpuUsage_Process CProcessCPU;

		// CPU ����� üũ Ŭ���� (�ϵ����)
		CCpuUsage_Processor CProcessorCPU;

		// PDH�� Ŭ����
		CPDH	CPdh;

		while (1)
		{
			// 1�ʿ� �ѹ� ����� ������ ������.
			DWORD Check = WaitForSingleObject(*hEvent, 1000);

			// �̻��� ��ȣ���
			if (Check == WAIT_FAILED)
			{
				DWORD Error = GetLastError();
				printf("MoniterThread Exit Error!!! (%d) \n", Error);
				break;
			}

			// ����, ���� ��ȣ�� �Դٸ� ������ ����.
			else if (Check == WAIT_OBJECT_0)
				break;


			// �װ� �ƴ϶��, ���� �Ѵ�.

			// ���μ���, ���μ��� CPU �����, PDH ���� ����
			CProcessorCPU.UpdateCpuTime();
			CProcessCPU.UpdateCpuTime();
			CPdh.SetInfo();

			// ----------------------------------
			// �ϵ���� ���� ������ (���μ���)
			// ----------------------------------
			int TimeStamp = (int)(time(NULL));

			// 1. �ϵ���� CPU ���� ��ü
			g_This->InfoSend(dfMONITOR_DATA_TYPE_SERVER_CPU_TOTAL, (int)CProcessorCPU.ProcessorTotal(), TimeStamp);

			// 2. �ϵ���� ��밡�� �޸� (MByte)
			g_This->InfoSend(dfMONITOR_DATA_TYPE_SERVER_AVAILABLE_MEMORY, (int)CPdh.Get_AVA_Mem(), TimeStamp);

			// 3. �ϵ���� �̴��� ���� ����Ʈ (KByte)
			int iData = (int)(CPdh.Get_Net_Recv() / 1024);
			g_This->InfoSend(dfMONITOR_DATA_TYPE_SERVER_NETWORK_RECV, iData, TimeStamp);

			// 4. �ϵ���� �̴��� �۽� ����Ʈ (KByte)
			iData = (int)(CPdh.Get_Net_Send() / 1024);
			g_This->InfoSend(dfMONITOR_DATA_TYPE_SERVER_NETWORK_SEND, iData, TimeStamp);

			// 5. �ϵ���� �������� �޸� ��뷮 (MByte)
			iData = (int)(CPdh.Get_NonPaged_Mem() / 1024 / 1024);
			g_This->InfoSend(dfMONITOR_DATA_TYPE_SERVER_NONPAGED_MEMORY, iData, TimeStamp);



			// ----------------------------------
			// ���Ӽ��� ���� ������
			// ----------------------------------

			// ���Ӽ����� On�� ���, ���Ӽ��� ���� ������.
			if (g_This->m_BattleServer_this->GetServerState() == true)
			{
				// 1. ���Ӽ��� ON		
				g_This->InfoSend(dfMONITOR_DATA_TYPE_BATTLE_SERVER_ON, TRUE, TimeStamp);

				// 2. ���Ӽ��� CPU ���� (Ŀ�� + ����)
				g_This->InfoSend(dfMONITOR_DATA_TYPE_BATTLE_CPU, (int)CProcessCPU.ProcessTotal(), TimeStamp);

				// 3. ���Ӽ��� �޸� ���� Ŀ�� ��뷮 (Private) MByte
				int Data = (int)(CPdh.Get_UserCommit() / 1024 / 1024);
				g_This->InfoSend(dfMONITOR_DATA_TYPE_BATTLE_MEMORY_COMMIT, Data, TimeStamp);

				// 4. ���Ӽ��� ��ŶǮ ��뷮
				g_This->InfoSend(dfMONITOR_DATA_TYPE_BATTLE_PACKET_POOL, g_lAllocNodeCount + g_lAllocNodeCount_Lan, TimeStamp);

				// 5. Auth ������ �ʴ� ���� ��
				g_This->InfoSend(dfMONITOR_DATA_TYPE_BATTLE_AUTH_FPS, g_lShowAuthFPS, TimeStamp);

				// 6. Game ������ �ʴ� ���� ��
				g_This->InfoSend(dfMONITOR_DATA_TYPE_BATTLE_GAME_FPS, g_lShowGameFPS, TimeStamp);

				// 7. ���Ӽ��� ���� ������ü
				g_This->InfoSend(dfMONITOR_DATA_TYPE_BATTLE_SESSION_ALL, (int)g_This->m_BattleServer_this->GetClientCount(), TimeStamp);

				// 8. Auth ������ ��� �ο�
				g_This->InfoSend(dfMONITOR_DATA_TYPE_BATTLE_SESSION_AUTH, g_lAuthModeUserCount, TimeStamp);

				// 9. Game ������ ��� �ο�
				g_This->InfoSend(dfMONITOR_DATA_TYPE_BATTLE_SESSION_GAME, g_lGameModeUserCount, TimeStamp);

				// 10. ���� ��

				// 11. �÷��� �� ��
			}

		}

		return 0;
	}

	// ����͸� ������ ������ ����
	//
	// Parameter : DataType(BYTE), DataValue(int), TimeStamp(int)
	// return : ����
	void CGame_MinitorClient::InfoSend(BYTE DataType, int DataValue, int TimeStamp)
	{
		WORD Type = en_PACKET_SS_MONITOR_DATA_UPDATE;

		CProtocolBuff_Lan* SendBuff = CProtocolBuff_Lan::Alloc();

		SendBuff->PutData((char*)&Type, 2);
		SendBuff->PutData((char*)&DataType, 1);
		SendBuff->PutData((char*)&DataValue, 4);
		SendBuff->PutData((char*)&TimeStamp, 4);

		SendPacket(m_ullSessionID, SendBuff);
	}


	// -----------------------
	// ���� �����Լ�
	// -----------------------

	// ��ǥ ������ ���� ���� ��, ȣ��Ǵ� �Լ� (ConnectFunc���� ���� ���� �� ȣ��)
	//
	// parameter : ����Ű
	// return : ����
	void CGame_MinitorClient::OnConnect(ULONGLONG SessionID)
	{
		m_ullSessionID = SessionID;

		// ����͸� ����(Lan)�� �α��� ��Ŷ ����
		CProtocolBuff_Lan* SendBuff = CProtocolBuff_Lan::Alloc();

		WORD Type = en_PACKET_SS_MONITOR_LOGIN;
		int ServerNo = dfSERVER_NO;

		SendBuff->PutData((char*)&Type, 2);
		SendBuff->PutData((char*)&ServerNo, 4);

		SendPacket(SessionID, SendBuff);
	}

	// ��ǥ ������ ���� ���� �� ȣ��Ǵ� �Լ� (InDIsconnect �ȿ��� ȣ��)
	//
	// parameter : ����Ű
	// return : ����
	void CGame_MinitorClient::OnDisconnect(ULONGLONG SessionID)
	{}

	// ��Ŷ ���� �Ϸ� �� ȣ��Ǵ� �Լ�.
	//
	// parameter : ���� ����Ű, CProtocolBuff_Lan*
	// return : ����
	void CGame_MinitorClient::OnRecv(ULONGLONG SessionID, CProtocolBuff_Lan* Payload)
	{}

	// ��Ŷ �۽� �Ϸ� �� ȣ��Ǵ� �Լ�
	//
	// parameter : ���� ����Ű, Send �� ������
	// return : ����
	void CGame_MinitorClient::OnSend(ULONGLONG SessionID, DWORD SendSize)
	{}

	// ��Ŀ �����尡 ��� �� ȣ��Ǵ� �Լ�.
	// GQCS �ٷ� �ϴܿ��� ȣ��
	// 
	// parameter : ����
	// return : ����
	void CGame_MinitorClient::OnWorkerThreadBegin()
	{}

	// ��Ŀ �����尡 ���� �� ȣ��Ǵ� �Լ�
	// GQCS �ٷ� ������ ȣ��
	// 
	// parameter : ����
	// return : ����
	void CGame_MinitorClient::OnWorkerThreadEnd()
	{}

	// ���� �߻� �� ȣ��Ǵ� �Լ�.
	//
	// parameter : ���� �ڵ�(���� ������ �����ڵ�� WinGetLastError() �Լ��� ��� ����. ���� ��� 0�� ���ϵ�)
	//			 : ���� �ڵ忡 ���� ��Ʈ��
	// return : ����
	void CGame_MinitorClient::OnError(int error, const TCHAR* errorStr)
	{}



}