#include "pch.h"
#include "BattleServer_Room_Version.h"
#include "Parser\Parser_Class.h"
#include "Protocol_Set\CommonProtocol_2.h"		// 차후 일반으로 변경 필요
#include "Log\Log.h"
#include "CPUUsage\CPUUsage.h"
#include "PDHClass\PDHCheck.h"

#include "shDB_Communicate.h"

#include "rapidjson\document.h"
#include "rapidjson\writer.h"
#include "rapidjson\stringbuffer.h"

#include <process.h>
#include <strsafe.h>

using namespace rapidjson;

// !! 임시. 차후 강사님이 주신 파일에서 읽을 예정 !!
LONG g_Cartridge_Bullet = 10;


// ------------------
// CGameSession의 함수
// (CBattleServer_Room의 이너클래스)
// ------------------
namespace Library_Jingyu
{
	// 덤프용
	CCrashDump* g_BattleServer_Room_Dump = CCrashDump::GetInstance();

	// 로그용
	CSystemLog* g_BattleServer_RoomLog = CSystemLog::GetInstance();


	// -----------------------
	// 생성자와 소멸자
	// -----------------------

	// 생성자
	CBattleServer_Room::CGameSession::CGameSession()
		:CMMOServer::cSession()
	{
		// ClientKey 초기값 셋팅
		m_int64ClientKey = -1;

		// 자료구조에 들어감 플래그
		m_bStructFlag = false;

		// 로그인 패킷 완료 처리
		m_bLoginFlag = false;

		// 곧 종료될 유저 체크 플래그
		m_bLogoutFlag = false;

		// 입장중인 방 초기 번호
		m_iRoomNo = -1;

		// 로그인 HTTP 요청 횟수
		m_lLoginHTTPCount = 0;

		// 생존 상태
		m_bAliveFlag = false;	

		// 마지막 전적 저장 상태 확인
		m_bLastDBWriteFlag = false;

		// DBWrite 카운트 초기화
		m_iDBWriteCount = 0;

		// 헬멧 수
		m_iHelmetCount = 0;
	}

	// 소멸자
	CBattleServer_Room::CGameSession::~CGameSession()
	{
		// 할거 없음
	}



	// -----------------
	// 가상함수
	// -----------------

	// --------------- AUTH 모드용 함수

	// 유저가 Auth 모드로 변경됨
	//
	// Parameter : 없음
	// return : 없음
	void CBattleServer_Room::CGameSession::OnAuth_ClientJoin()
	{
		// 할거 없음
	}

	// 유저가 Auth 모드에서 나감
	//
	// Parameter : Game모드로 변경된것인지 알려주는 Flag. 디폴트 false(변경되지 않음)
	// return : 없음
	void CBattleServer_Room::CGameSession::OnAuth_ClientLeave(bool bGame)
	{
		// 모드 변경일 경우
		if (bGame == true)
		{
			
		}

		// 실제 게임 종료일 경우
		else
		{
			// ClientKey 초기값으로 셋팅.
			// !! 이걸 안하면, Release되는 중에 HTTP응답이 올 경우 !!
			// !! Auth_Update에서 이미 샌드큐가 모두 정리된 유저에게 또 SendPacket 가능성 !!
			// !! 이 경우, 전혀 엉뚱한 유저에게 Send가 될 수 있음 !!
			m_int64ClientKey = -1;

			// 해당 유저가 있는 룸 안에서 유저 제거
			// -1이 아니면 룸에 있는것.
			if (m_iRoomNo != -1)
			{
				AcquireSRWLockShared(&m_pParent->m_Room_Umap_srwl);		// ----- Room 자료구조 Shard 락 

				// 1. 룸 검색
				auto FindRoom = m_pParent->m_Room_Umap.find(m_iRoomNo);

				// 룸이 없으면 Crash
				if (FindRoom == m_pParent->m_Room_Umap.end())
					g_BattleServer_Room_Dump->Crash();

				stRoom* NowRoom = FindRoom->second;

				// 룸 상태가 Play면 Crash. 말도 안됨
				// Auth모드에서 종료되는 유저가 있는 방은, 무조건 대기/준비 방이어야 함				
				if (NowRoom->m_iRoomState == eu_ROOM_STATE::PLAY_ROOM)
					g_BattleServer_Room_Dump->Crash();			
				


				// 2. 현재 룸 안에 있는 유저 수 감소
				--NowRoom->m_iJoinUserCount;

				// 감소 후, 룸 안의 유저 수가 0보다 적으면 말도 안됨.
				if (NowRoom->m_iJoinUserCount < 0)				
					g_BattleServer_Room_Dump->Crash();


				// 3. 룸 안의 자료구조에서 유저 제거
				if (NowRoom->Erase(this) == false)
					g_BattleServer_Room_Dump->Crash();

				m_iRoomNo = -1;


				// 4. 방 안의 모든 유저에게, 방에서 유저가 나갔다고 알려줌
				// BroadCast해도 된다. 어차피 방 안에 지금 나간 유저는 없음.
				CProtocolBuff_Net* SendBuff = CProtocolBuff_Net::Alloc();

				WORD Type = en_PACKET_CS_GAME_RES_REMOVE_USER;

				SendBuff->PutData((char*)&Type, 2);
				SendBuff->PutData((char*)&m_iRoomNo, 4);
				SendBuff->PutData((char*)&m_Int64AccountNo, 8);

				// 여기서, false가 리턴될 수 있음. (방 안의 자료구조에 유저가 0명)
				// 대기방인 상태에서 모든 유저가 나갈 수 있기 때문에 문제로 안본다.
				// 때문에 return 안받음
				NowRoom->SendPacket_BroadCast(SendBuff);


				// 5. 마스터 서버에게 해당 유저 나갔다고 패킷 보내줘야 함.
				m_pParent->m_Master_LanClient->Packet_RoomLeave_Req(m_iRoomNo, m_Int64AccountNo);


				// 6. 방 안의 유저 수가 0명이라면, m_bWaitDelete 플래그 체크 후, 방 모드를 Play로 변경
				if (NowRoom->m_iJoinUserCount == 0 && NowRoom->m_bWaitDelete == true)
				{
					InterlockedDecrement(&m_pParent->m_lNowWaitRoomCount);
					InterlockedIncrement(&m_pParent->m_lPlayRoomCount);

					NowRoom->m_iRoomState = eu_ROOM_STATE::PLAY_ROOM;
				}

				// Auth 스레드와, Master_LanClient 스레드가, Wait모드인 방에 동시 접근하기 때문에, 크기 락을 묶어야 함.
				ReleaseSRWLockShared(&m_pParent->m_Room_Umap_srwl);		// ----- Room 자료구조 Shard 언락 
			}			
		}

	}

	// Auth 모드의 유저에게 packet이 옴
	//
	// Parameter : 패킷 (CProtocolBuff_Net*)
	// return : 없음
	void CBattleServer_Room::CGameSession::OnAuth_Packet(CProtocolBuff_Net* Packet)
	{
		// 패킷 처리
		try
		{
			// 1. 타입 빼기
			WORD Type;
			Packet->GetData((char*)&Type, 2);

			// 2. 타입에 따라 분기 처리
			switch (Type)
			{
				// 로그인 요청
			case en_PACKET_CS_GAME_REQ_LOGIN:
				Auth_LoginPacket(Packet);
				break;

				// 방 입장 요청
			case en_PACKET_CS_GAME_REQ_ENTER_ROOM:
				break;

				// 하트비트
			case en_PACKET_CS_GAME_REQ_HEARTBEAT:
				break;

				// 그 외에는 문제있음.
			default:
				throw CException(_T("OnAuth_Packet() --> Type Error!!"));
				break;
			}
		}
		catch (CException& exc)
		{
			// 로그 찍기 (로그 레벨 : 에러)
			g_BattleServer_RoomLog->LogSave(L"CBattleServer_Room", CSystemLog::en_LogLevel::LEVEL_ERROR, L"%s",
				(TCHAR*)exc.GetExceptionText());

			// 덤프
			g_BattleServer_Room_Dump->Crash();
		}

	}



	// --------------- GAME 모드용 함수

	// 유저가 Game모드로 변경됨
	//
	// Parameter : 없음
	// return : 없음
	void CBattleServer_Room::CGameSession::OnGame_ClientJoin()
	{
		// 로그인 여부 체크
		if (m_bLoginFlag == false)
			g_BattleServer_Room_Dump->Crash();

		AcquireSRWLockShared(&m_pParent->m_Room_Umap_srwl);		// ----- Room Umap Shared 락

		// 1. 유저가 있는 방 알아오기
		auto FindRoom = m_pParent->m_Room_Umap.find(m_iRoomNo);

		// 없으면 말도 안됨.

		if (FindRoom == m_pParent->m_Room_Umap.end())
			g_BattleServer_Room_Dump->Crash();

		stRoom* NowRoom = FindRoom->second;

		// 방 모드가 Play가 아니면 말도 안됨.
		if(NowRoom->m_iRoomState != eu_ROOM_STATE::PLAY_ROOM)
			g_BattleServer_Room_Dump->Crash();

		// 락 푼다
		ReleaseSRWLockShared(&m_pParent->m_Room_Umap_srwl);		// ----- Room Umap Shared 언락



		// 2. 게임 모드로 전환된 유저 카운트 증가.

		// 증가 전, 이미 JoinUser와 같다면 말도 안됨. 한 명이 방에 더 들어온것. 크래시
		if(NowRoom->m_iGameModeUser == NowRoom->m_iJoinUserCount)
			g_BattleServer_Room_Dump->Crash();

		++NowRoom->m_iGameModeUser;



		// 3. 증가 후, JoinUser와 같아진다면 방 내의 모든 유저에게 [내 캐릭터 생성] 과 [다른 유저 캐릭터 생성]을 보낸다.
		// OnGame_ClientJoin 마다 호출하는게 아니라, 모두 접속했는지 확인 후 보낸다. 동시에 캐릭터 생성을 하기 위해서.
		if (NowRoom->m_iGameModeUser == NowRoom->m_iJoinUserCount)
			NowRoom->CreateCharacter();


		// 전적 중, 플레이 횟수 증가. 그리고 DB에 저장
		++m_iRecord_PlayCount;

		DB_WORK_CONTENT_UPDATE* CountWrite = (DB_WORK_CONTENT_UPDATE*)m_pParent->m_shDB_Communicate.m_pDB_Work_Pool->Alloc();

		CountWrite->m_wWorkType = eu_DB_AFTER_TYPE::eu_WRITE;

		CountWrite->m_iRecord_PlayCount = m_iRecord_PlayCount;
		CountWrite->m_iRecord_PlayTime = m_iRecord_PlayTime;
		CountWrite->m_iRecord_Kill = m_iRecord_Kill;
		CountWrite->m_iRecord_Die = m_iRecord_Die;
		CountWrite->m_iRecord_Win = m_iRecord_Win;

		CountWrite->AccountNo = m_Int64AccountNo;

		// Write 하기 전에, WriteCount 증가.
		m_pParent->AddDBWriteCountFunc(m_Int64AccountNo);

		// DBWrite
		m_pParent->m_shDB_Communicate.DBWriteFunc((DB_WORK*)CountWrite, en_PHP_TYPE::UPDATE_CONTENTS);
	}

	// 유저가 Game모드에서 나감
	//
	// Parameter : 없음
	// return : 없음
	void CBattleServer_Room::CGameSession::OnGame_ClientLeave()
	{
		// ClientKey 초기값으로 셋팅.
		// !! 이걸 안하면, Release되는 중에 HTTP응답이 올 경우 !!
		// !! Game_Update에서 이미 샌드큐가 모두 정리된 유저에게 또 SendPacket 가능성 !!
		// !! 이 경우, 전혀 엉뚱한 유저에게 Send가 될 수 있음 !!
		m_int64ClientKey = -1;


		// 해당 유저가 있는 룸 안에서 유저 제거
		// -1인 것은 말도 안됨. 무조건 방에 있어야 함
		if(m_iRoomNo == -1)
			g_BattleServer_Room_Dump->Crash();


		AcquireSRWLockShared(&m_pParent->m_Room_Umap_srwl);		// ----- Room 자료구조 Shard 락 

		// 1. 룸 검색
		auto FindRoom = m_pParent->m_Room_Umap.find(m_iRoomNo);

		// 룸이 없으면 Crash
		if (FindRoom == m_pParent->m_Room_Umap.end())
			g_BattleServer_Room_Dump->Crash();

		stRoom* NowRoom = FindRoom->second;

		// 룸 상태가 Play가 아니면 Crash. 말도 안됨
		// Game모드에서 종료되는 유저가 있는 방은, 무조건 플레이 방이어야 함
		if (NowRoom->m_iRoomState != eu_ROOM_STATE::PLAY_ROOM)
			g_BattleServer_Room_Dump->Crash();

		// 여기까지 오면 방 모드가 PLAY_ROOM 확정.
		// 즉, Game에서만 접근하는 방 확정. 락 푼다.
		// 락 풀었는데 방이 삭제될 가능성은 전혀 없음. 
		// 방 삭제는 Game 스레드가 하고, 해당 함수도 Game 스레드에서 호출된다. 
		ReleaseSRWLockShared(&m_pParent->m_Room_Umap_srwl);		// ----- Room 자료구조 Shard 언락 



		// 2. 현재 룸 안에 있는 유저 수 감소
		--NowRoom->m_iJoinUserCount;

		// 감소 후, 룸 안의 유저 수가 0보다 적으면 말도 안됨.
		if (NowRoom->m_iJoinUserCount < 0)
			g_BattleServer_Room_Dump->Crash();



		// 3. 생존한 유저 수 감소.
		// 생존한 유저 수로 승리여부를 판단해야 하기 때문에 
		// 유저가 게임에서 나가거나 HP가 0이되어 사망할 경우 감소시켜야 함
		--NowRoom->m_iAliveUserCount;

		// 감소 후, 0보다 적으면 말도 안됨.
		if (NowRoom->m_iAliveUserCount < 0)
			g_BattleServer_Room_Dump->Crash();




		// 4. 룸 안의 자료구조에서 유저 제거
		if (NowRoom->Erase(this) == false)
			g_BattleServer_Room_Dump->Crash();



		// 5. 방 안의 모든 유저에게, 방에서 유저가 나갔다고 알려줌
		// BroadCast해도 된다. 어차피 방 안에 지금 나간 유저는 없음.
		CProtocolBuff_Net* SendBuff = CProtocolBuff_Net::Alloc();

		WORD Type = en_PACKET_CS_GAME_RES_LEAVE_USER;

		SendBuff->PutData((char*)&Type, 2);
		SendBuff->PutData((char*)&m_iRoomNo, 4);
		SendBuff->PutData((char*)&m_Int64AccountNo, 8);

		// 여기서, false가 리턴될 수 있음. (방 안의 자료구조에 유저가 0명)
		// 대기방인 상태에서 모든 유저가 나갈 수 있기 때문에 문제로 안본다.
		// 때문에 return 안받음
		NowRoom->SendPacket_BroadCast(SendBuff);

		m_iRoomNo = -1;



		// 6. 마지막 전적 저장 패킷을 안보냈다면, 여기서 보낸다.
		if (m_bLastDBWriteFlag == false)
		{
			// 플레이 타임 갱신
			int AddTime = (timeGetTime() - m_dwGameStartTime) / 1000;
			m_iRecord_PlayTime = m_iRecord_PlayTime + AddTime;

			// DB에 Write 준비
			DB_WORK_CONTENT_UPDATE* WriteWork = (DB_WORK_CONTENT_UPDATE*)m_pParent->m_shDB_Communicate.m_pDB_Work_Pool->Alloc();

			WriteWork->m_wWorkType = eu_DB_AFTER_TYPE::eu_WRITE;

			WriteWork->m_iRecord_PlayCount = m_iRecord_PlayCount;
			WriteWork->m_iRecord_PlayTime = m_iRecord_PlayTime;
			WriteWork->m_iRecord_Kill = m_iRecord_Kill;
			WriteWork->m_iRecord_Die = m_iRecord_Die;
			WriteWork->m_iRecord_Win = m_iRecord_Win;

			WriteWork->AccountNo = m_Int64AccountNo;

			// Write 하기 전에, WriteCount 증가.
			m_pParent->AddDBWriteCountFunc(m_Int64AccountNo);

			// DBWrite
			m_pParent->m_shDB_Communicate.DBWriteFunc((DB_WORK*)WriteWork, en_PHP_TYPE::UPDATE_CONTENTS);
		}
	}

	// Game 모드의 유저에게 packet이 옴
	//
	// Parameter : 패킷 (CProtocolBuff_Net*)
	// return : 없음
	void CBattleServer_Room::CGameSession::OnGame_Packet(CProtocolBuff_Net* Packet)
	{
		// 타입에 따라 패킷 처리
		try
		{
			// 1. 타입 빼기
			WORD Type;
			Packet->GetData((char*)&Type, 2);

			// 2. 타입에 따라 분기문 처리
			switch (Type)
			{	
				// 캐릭터 이동
			case en_PACKET_CS_GAME_REQ_MOVE_PLAYER:
				Game_MovePacket(Packet);
				break;

				// Fire1 발사 (총 발사)
			case en_PACKET_CS_GAME_REQ_FIRE1:
				Game_Frie_1_Packet(Packet);
				break;

				// HitDamage
			case en_PACKET_CS_GAME_REQ_HIT_DAMAGE:
				Game_HitDamage_Packet(Packet);
				break;

				// Fire 2 공격 (발차기)
			case en_PACKET_CS_GAME_REQ_FIRE2:
				Game_Fire_2_Packet(Packet);
				break;

				// KickDamage
			case en_PACKET_CS_GAME_RES_KICK_DAMAGE:
				Game_KickDamage_Packet(Packet);
				break;

				// 캐릭터 HitPoint 갱신
			case en_PACKET_CS_GAME_REQ_HIT_POINT:
				Game_HitPointPacket(Packet);
				break;

				// 하트비트
			case en_PACKET_CS_GAME_REQ_HEARTBEAT:
				break;

				// 이 외에는 문제 있음
			default:
				throw CException(_T("OnGame_Packet() --> Type Error!!"));
				break;
			}

		}
		catch (CException& exc)
		{
			// 로그 찍기 (로그 레벨 : 에러)
			g_BattleServer_RoomLog->LogSave(L"CBattleServer_Room", CSystemLog::en_LogLevel::LEVEL_ERROR, L"%s",
				(TCHAR*)exc.GetExceptionText());

			// 덤프
			g_BattleServer_Room_Dump->Crash();
		}

	}



	// --------------- Release 모드용 함수

	// Release된 유저.
	//
	// Parameter : 없음
	// return : 없음
	void CBattleServer_Room::CGameSession::OnGame_ClientRelease()
	{
		// m_bStructFlag가 true라면, 자료구조에 들어간 유저
		if (m_bStructFlag == true)
		{
			if (m_pParent->EraseAccountNoFunc(m_Int64AccountNo) == false)
				g_BattleServer_Room_Dump->Crash();

			m_bStructFlag = false;
		}

		// DBWrite에서 제거 시도.
		// Auth모드에서, 로그인 패킷을 안보내고 나갈 경우, 없을 수도 있음 (리턴값이 false가 나올 수 있음)
		// 그래서 리턴값 안받음.
		m_pParent->MinDBWriteCountFunc(m_Int64AccountNo);
		
		m_bLoginFlag = false;	
		m_bLogoutFlag = false;
		m_lLoginHTTPCount = 0;		
		m_bLastDBWriteFlag = false;		
		m_iHelmetCount = 0;
	}




	// -----------------
	// Auth모드 패킷 처리 함수
	// -----------------

	// 로그인 요청 
	//
	// Parameter : CProtocolBuff_Net*
	// return : 없음
	void CBattleServer_Room::CGameSession::Auth_LoginPacket(CProtocolBuff_Net* Packet)
	{
		// 1. 현재 ClientKey가 초기값이 아니면 Crash
		if (m_int64ClientKey != -1)
			g_BattleServer_Room_Dump->Crash();


		// 2. AccountNo, ClientKey 마샬링
		INT64 AccountNo;
		INT64 ClinetKey;

		Packet->GetData((char*)&AccountNo, 8);
		Packet->GetData((char*)&ClinetKey, 8);


		// 3. 혹시 아직 DBWrite 중인지 체크
		if (m_pParent->InsertDBWriteCountFunc(AccountNo, 1) == false)
		{
			InterlockedIncrement(&m_pParent->m_DBWrie_LoginCount);

			// 중복 로그인 패킷 보내기
			CProtocolBuff_Net* SendBuff = CProtocolBuff_Net::Alloc();

			// 타입
			WORD Type = en_PACKET_CS_GAME_RES_LOGIN;
			SendBuff->PutData((char*)&Type, 2);

			// Status
			BYTE Status = 6;		// 중복 로그인
			SendBuff->PutData((char*)&Status, 1);

			// AccountNo
			SendBuff->PutData((char*)&AccountNo, 8);

			// SendPacket
			SendPacket(SendBuff);

			return;
		}



		// 4. AccountNo, ClientKey 셋팅
		m_Int64AccountNo = AccountNo;
		m_int64ClientKey = ClinetKey;
			   		



		// 5. AccountNo 자료구조에 추가.	
		// 이미 있으면(false 리턴) 중복 로그인으로 처리
		// 현재 유저에게는 실패 패킷, 접속 중인 유저는 DIsconnect.
		if (m_pParent->InsertAccountNoFunc(AccountNo, this) == false)
		{	
			InterlockedIncrement(&m_pParent->m_DuplicateCount);

			// 중복 로그인 패킷 보내기
			CProtocolBuff_Net* SendBuff = CProtocolBuff_Net::Alloc();

			// 타입
			WORD Type = en_PACKET_CS_GAME_RES_LOGIN;
			SendBuff->PutData((char*)&Type, 2);

			// Status
			BYTE Status = 6;		// 중복 로그인
			SendBuff->PutData((char*)&Status, 1);

			// AccountNo
			SendBuff->PutData((char*)&AccountNo, 8);			

			// SendPacket
			SendPacket(SendBuff);

			// 중복 로그인 처리된 유저 접속 종료 요청
			m_pParent->DisconnectAccountNoFunc(AccountNo);

			return;
		}

		// 자료구조에 들어감 플래그 변경
		m_bStructFlag = true;





		// 6. 로그인 인증처리를 위한 HTTP 통신
		// 통신 후, 해당 패킷에 대한 후처리는 Auth_Update에게 넘긴다.
		DB_WORK_LOGIN* Send_A = (DB_WORK_LOGIN*)m_pParent->m_shDB_Communicate.m_pDB_Work_Pool->Alloc();

		Send_A->m_wWorkType = eu_DB_AFTER_TYPE::eu_LOGIN_AUTH;
		Send_A->m_i64UniqueKey = ClinetKey;
		Send_A->pPointer = this;

		// 직렬화 버퍼 전달 전에, 레퍼런스 카운트 Add
		Packet->Add();
		Send_A->m_pBuff = Packet;

		Send_A->AccountNo = AccountNo;

		// Select_Account.php 요청
		m_pParent->m_shDB_Communicate.DBReadFunc((DB_WORK*)Send_A, en_PHP_TYPE::SELECT_ACCOUNT);


		// 7. 유저 전적 정보를 위한 HTTP 통신
		// 통신 후, 해당 패킷에 대한 후처리는 Auth_Update에게 넘긴다.
		DB_WORK_LOGIN_CONTENTS* Send_B = (DB_WORK_LOGIN_CONTENTS*)m_pParent->m_shDB_Communicate.m_pDB_Work_Pool->Alloc();
	
		Send_A->m_wWorkType = eu_DB_AFTER_TYPE::eu_LOGIN_INFO;
		Send_A->m_i64UniqueKey = ClinetKey;
		Send_A->pPointer = this;

		Send_A->AccountNo = AccountNo;

		// Select_Contents.php 요청
		m_pParent->m_shDB_Communicate.DBReadFunc((DB_WORK*)Send_A, en_PHP_TYPE::SELECT_CONTENTS);	
	}


	// 방 입장 요청
	// 
	// Parameter : CProtocolBuff_Net*
	// return : 없음
	void CBattleServer_Room::CGameSession::Auth_RoomEnterPacket(CProtocolBuff_Net* Packet)
	{
		// 로그인 여부 체크
		if(m_bLoginFlag == false)
			g_BattleServer_Room_Dump->Crash();

		// 1. 마샬링
		INT64 AccountNo;
		int RoomNo;
		char EnterToken[32];

		Packet->GetData((char*)&AccountNo, 8);
		Packet->GetData((char*)&RoomNo, 4);
		Packet->GetData(EnterToken, 32);		

		// 2. 에러 체크
		if (AccountNo != m_Int64AccountNo)
			g_BattleServer_Room_Dump->Crash();
		
		AcquireSRWLockShared(&m_pParent->m_Room_Umap_srwl);	// ----- Room umap Shared 락

		
		// 3. 입장하고자 하는 방 검색
		auto ret = m_pParent->m_Room_Umap.find(RoomNo);

		// 4. 방 존재여부 체크
		if (ret == m_pParent->m_Room_Umap.end())
		{
			ReleaseSRWLockShared(&m_pParent->m_Room_Umap_srwl);	// ----- Room umap Shared 언락

			// 방이 없으면, 에러 리턴
			CProtocolBuff_Net* SendBuff = CProtocolBuff_Net::Alloc();

			WORD Type = en_PACKET_CS_GAME_RES_ENTER_ROOM;
			BYTE Result = 4;
			BYTE MaxUser = 0;	// 방 없음 패킷에는 의미가 없을듯.. 그냥 아무거나 보냄.

			SendBuff->PutData((char*)&Type, 2);
			SendBuff->PutData((char*)&AccountNo, 8);
			SendBuff->PutData((char*)&RoomNo, 4);
			SendBuff->PutData((char*)&MaxUser, 1);
			SendBuff->PutData((char*)&Result, 1);

			SendPacket(SendBuff);

			return;
		}

		stRoom* NowRoom = ret->second;

		// 5. 방이 대기방이 아니거나, Wait이지만 삭제될 방일 경우 (m_bWaitDelete가 true) 에러 3 리턴
		if (NowRoom->m_iRoomState != eu_ROOM_STATE::WAIT_ROOM || 
			NowRoom->m_bWaitDelete == true)
		{
			ReleaseSRWLockShared(&m_pParent->m_Room_Umap_srwl);	// ----- Room umap Shared 언락

			// 대기방이 아니면, 에러 리턴
			CProtocolBuff_Net* SendBuff = CProtocolBuff_Net::Alloc();

			WORD Type = en_PACKET_CS_GAME_RES_ENTER_ROOM;
			BYTE Result = 3;
			BYTE MaxUser = 0;	// 대기방 아님 패킷에는 의미가 없을듯.. 그냥 아무거나 보냄.

			SendBuff->PutData((char*)&Type, 2);
			SendBuff->PutData((char*)&AccountNo, 8);
			SendBuff->PutData((char*)&RoomNo, 4);
			SendBuff->PutData((char*)&MaxUser, 1);
			SendBuff->PutData((char*)&Result, 1);

			SendPacket(SendBuff);

			return;
		}			
		
		// 6. 방 인원수 체크
		if (NowRoom->m_iJoinUserCount == NowRoom->m_iMaxJoinCount)
		{
			BYTE MaxUser = NowRoom->m_iJoinUserCount;

			ReleaseSRWLockShared(&m_pParent->m_Room_Umap_srwl);	// ----- Room umap Shared 언락				

			// 이미 최대 인원수라면, 에러 리턴			
			CProtocolBuff_Net* SendBuff = CProtocolBuff_Net::Alloc();

			WORD Type = en_PACKET_CS_GAME_RES_ENTER_ROOM;
			BYTE Result = 5;

			SendBuff->PutData((char*)&Type, 2);
			SendBuff->PutData((char*)&AccountNo, 8);
			SendBuff->PutData((char*)&RoomNo, 4);
			SendBuff->PutData((char*)&MaxUser, 1);
			SendBuff->PutData((char*)&Result, 1);

			SendPacket(SendBuff);

			return;
		}

		// 7. 방 토큰 체크
		if (memcmp(EnterToken, NowRoom->m_cEnterToken, 32) != 0)
		{
			BYTE MaxUser = NowRoom->m_iJoinUserCount;

			ReleaseSRWLockShared(&m_pParent->m_Room_Umap_srwl);	// ----- Room umap Shared 언락

			InterlockedIncrement(&m_pParent->m_lRoomEnterTokenError);		

			// 토큰이 다르면, 에러 리턴
			CProtocolBuff_Net* SendBuff = CProtocolBuff_Net::Alloc();

			WORD Type = en_PACKET_CS_GAME_RES_ENTER_ROOM;
			BYTE Result = 2;

			SendBuff->PutData((char*)&Type, 2);
			SendBuff->PutData((char*)&AccountNo, 8);
			SendBuff->PutData((char*)&RoomNo, 4);
			SendBuff->PutData((char*)&MaxUser, 1);
			SendBuff->PutData((char*)&Result, 1);

			SendPacket(SendBuff);

			return;
		}


		// 8. 여기까지 오면 정상
		// 룸에 인원 추가
		NowRoom->m_iJoinUserCount++;
		NowRoom->Insert(this);

		int NowUser = NowRoom->m_iJoinUserCount;


		// 9. 정상적인 방 입장 응답 보내기
		CProtocolBuff_Net* SendBuff_A = CProtocolBuff_Net::Alloc();

		WORD Type = en_PACKET_CS_GAME_RES_ENTER_ROOM;
		BYTE Result = 1;

		SendBuff_A->PutData((char*)&Type, 2);
		SendBuff_A->PutData((char*)&AccountNo, 8);
		SendBuff_A->PutData((char*)&RoomNo, 4);
		SendBuff_A->PutData((char*)&NowUser, 1);
		SendBuff_A->PutData((char*)&Result, 1);

		SendPacket(SendBuff_A);


		// 10. 방에 있는 모든 유저에게 "유저 추가됨" 패킷 보내기
		// 이번에 입장한 자기 자신 포함.
		Type = en_PACKET_CS_GAME_RES_ADD_USER;

		CProtocolBuff_Net* SendBuff_B = CProtocolBuff_Net::Alloc();

		SendBuff_B->PutData((char*)&Type, 2);
		SendBuff_B->PutData((char*)&RoomNo, 4);
		SendBuff_B->PutData((char*)&AccountNo, 8);
		SendBuff_B->PutData((char*)m_tcNickName, 40);

		SendBuff_B->PutData((char*)&m_iRecord_PlayCount, 4);
		SendBuff_B->PutData((char*)&m_iRecord_PlayTime, 4);
		SendBuff_B->PutData((char*)&m_iRecord_Kill, 4);
		SendBuff_B->PutData((char*)&m_iRecord_Die, 4);
		SendBuff_B->PutData((char*)&m_iRecord_Win, 4);

		if (NowRoom->SendPacket_BroadCast(SendBuff_B) == false)
			g_BattleServer_Room_Dump->Crash();


		// 11. 룸 인원수가 풀 방이 되었을 경우
		if (NowUser == NowRoom->m_iMaxJoinCount)
		{
			// 1) 마스터에게 방 닫힘 패킷 보내기
			m_pParent->m_Master_LanClient->Packet_RoomClose_Req(RoomNo);

			// 2) 모든 유저에게 카운트다운 패킷 보내기
			CProtocolBuff_Net* SendBuff = CProtocolBuff_Net::Alloc();

			WORD Type = en_PACKET_CS_GAME_RES_PLAY_READY;
			BYTE ReadySec = m_pParent->m_stConst.m_bCountDownSec;		// 초 단위

			SendBuff->PutData((char*)&Type, 2);
			SendBuff->PutData((char*)&RoomNo, 4);
			SendBuff->PutData((char*)&ReadySec, 1);

			if (NowRoom->SendPacket_BroadCast(SendBuff) == false)
				g_BattleServer_Room_Dump->Crash();

			// 3) 방 카운트다운 변수 갱신
			// 이 시간 + 유저에게 보낸 카운트다운(10초)가 되면 방 모드를 Play로 변경시키고
			// 유저도 AUTH_TO_GAME으로 변경시킨다.
			NowRoom->m_dwCountDown = timeGetTime();

			// 4) 방의 상태를 Ready로 변경
			NowRoom->m_iRoomState = eu_ROOM_STATE::READY_ROOM;

			InterlockedDecrement(&m_pParent->m_lNowWaitRoomCount);
			InterlockedIncrement(&m_pParent->m_lReadyRoomCount);

			// 5) 대기방 수 감소
			InterlockedDecrement(&m_pParent->m_lNowWaitRoomCount);
		}

		ReleaseSRWLockShared(&m_pParent->m_Room_Umap_srwl);	// ----- Room umap Shared 언락
	}



	// -----------------
	// Game모드 패킷 처리 함수
	// -----------------

	// 유저가 이동할 시 보내는 패킷.
	//
	// Parameter : CProtocolBuff_Net*
	// return : 없음
	void CBattleServer_Room::CGameSession::Game_MovePacket(CProtocolBuff_Net* Packet)
	{
		// 로그인 여부 체크
		if (m_bLoginFlag == false)
			g_BattleServer_Room_Dump->Crash();

		// 생존 여부 체크
		// 죽은 유저의 이동 요청이면 그냥 무시.
		if (m_bAliveFlag == false)
			return;

		// 1. 내가있는 방 알아오기
		AcquireSRWLockShared(&m_pParent->m_Room_Umap_srwl);		// ----- Room Umap Shared 락

		auto FindRoom = m_pParent->m_Room_Umap.find(m_iRoomNo);

		// 없으면 크래시
		if (FindRoom == m_pParent->m_Room_Umap.end())
			g_BattleServer_Room_Dump->Crash();

		stRoom* NowRoom = FindRoom->second;

		// 플레이모드가 아니면 크래시
		if (NowRoom->m_iRoomState != eu_ROOM_STATE::PLAY_ROOM)
			g_BattleServer_Room_Dump->Crash();

		ReleaseSRWLockShared(&m_pParent->m_Room_Umap_srwl);		// ----- Room Umap Shared 언락


		// 2. 마샬링
		float MoveTargetX;
		float MoveTargetY;
		float MoveTargetZ;

		float HitPointX;
		float HitPointY;
		float HitPointZ;

		Packet->GetData((char*)&MoveTargetX, 4);
		Packet->GetData((char*)&MoveTargetY, 4);
		Packet->GetData((char*)&MoveTargetZ, 4);

		Packet->GetData((char*)&HitPointX, 4);
		Packet->GetData((char*)&HitPointY, 4);
		Packet->GetData((char*)&HitPointZ, 4);


		// 2. 정보 갱신
		// Hit 포인트는 저장할 필요 없음. 그냥 릴레이만 한다.
		// MoveTargetX,Z만 저장한다. Z가 실제 높이 개념.
		// 즉, 서버는 X,Y만 저장하면 된다.
		m_fPosX = MoveTargetX;
		m_fPosY = MoveTargetZ;



		// 3. 방 내의 모든 유저에게 받은 패킷 Send하기(본인 제외)
		CProtocolBuff_Net* SendBuff = CProtocolBuff_Net::Alloc();
		WORD Type = en_PACKET_CS_GAME_RES_MOVE_PLAYER;

		SendBuff->PutData((char*)&Type, 2);
		SendBuff->PutData((char*)&m_Int64AccountNo, 8);

		SendBuff->PutData((char*)&MoveTargetX, 4);
		SendBuff->PutData((char*)&MoveTargetY, 4);
		SendBuff->PutData((char*)&MoveTargetZ, 4);

		SendBuff->PutData((char*)&HitPointX, 4);
		SendBuff->PutData((char*)&HitPointY, 4);
		SendBuff->PutData((char*)&HitPointZ, 4);

		NowRoom->SendPacket_BroadCast(SendBuff, m_Int64AccountNo);
	}
	
	// HitPoint 갱신
	//
	// Parameter : CProtocolBuff_Net*
	// return : 없음
	void CBattleServer_Room::CGameSession::Game_HitPointPacket(CProtocolBuff_Net* Packet)
	{
		// 로그인 여부 체크
		if (m_bLoginFlag == false)
			g_BattleServer_Room_Dump->Crash();

		// 생존 여부 체크
		// 죽은 유저의 히트 포인트 갱신은 무시
		if (m_bAliveFlag == false)
			return;


		// 1. 내가있는 방 알아오기
		AcquireSRWLockShared(&m_pParent->m_Room_Umap_srwl);		// ----- Room Umap Shared 락

		auto FindRoom = m_pParent->m_Room_Umap.find(m_iRoomNo);

		// 없으면 크래시
		if (FindRoom == m_pParent->m_Room_Umap.end())
			g_BattleServer_Room_Dump->Crash();

		stRoom* NowRoom = FindRoom->second;

		// 플레이모드가 아니면 크래시
		if (NowRoom->m_iRoomState != eu_ROOM_STATE::PLAY_ROOM)
			g_BattleServer_Room_Dump->Crash();

		ReleaseSRWLockShared(&m_pParent->m_Room_Umap_srwl);		// ----- Room Umap Shared 언락



		// 2. 마샬링
		float HitPointX;
		float HitPointY;
		float HitPointZ;

		Packet->GetData((char*)&HitPointX, 4);
		Packet->GetData((char*)&HitPointY, 4);
		Packet->GetData((char*)&HitPointZ, 4);



		// 3. 해당 방 내의 유저에게 HitPoint 갱신 패킷 보내기.
		// 자기 자신 제외

		CProtocolBuff_Net* SendBuff = CProtocolBuff_Net::Alloc();

		WORD Type = en_PACKET_CS_GAME_RES_HIT_POINT;

		SendBuff->PutData((char*)&Type, 2);

		SendBuff->PutData((char*)&m_Int64AccountNo, 8);

		SendBuff->PutData((char*)&HitPointX, 4);
		SendBuff->PutData((char*)&HitPointY, 4);
		SendBuff->PutData((char*)&HitPointZ, 4);

		NowRoom->SendPacket_BroadCast(SendBuff, m_Int64AccountNo);		
	}

	// Fire 1 패킷 (총 발사)
	//
	// Parameter : CProtocolBuff_Net*
	// return : 없음
	void CBattleServer_Room::CGameSession::Game_Frie_1_Packet(CProtocolBuff_Net* Packet)
	{
		// 로그인 여부 체크
		if (m_bLoginFlag == false)
			g_BattleServer_Room_Dump->Crash();

		// 생존 여부 체크
		// 죽은 유저의 공격 요청은 무시
		if (m_bAliveFlag == false)
			return;


		// 1. 내가있는 방 알아오기
		AcquireSRWLockShared(&m_pParent->m_Room_Umap_srwl);		// ----- Room Umap Shared 락

		auto FindRoom = m_pParent->m_Room_Umap.find(m_iRoomNo);

		// 없으면 크래시
		if (FindRoom == m_pParent->m_Room_Umap.end())
			g_BattleServer_Room_Dump->Crash();

		stRoom* NowRoom = FindRoom->second;

		// 플레이모드가 아니면 크래시
		if (NowRoom->m_iRoomState != eu_ROOM_STATE::PLAY_ROOM)
			g_BattleServer_Room_Dump->Crash();

		ReleaseSRWLockShared(&m_pParent->m_Room_Umap_srwl);		// ----- Room Umap Shared 언락



		// 2. 마샬링
		float HitPointX;
		float HitPointY;
		float HitPointZ;

		Packet->GetData((char*)&HitPointX, 4);
		Packet->GetData((char*)&HitPointY, 4);
		Packet->GetData((char*)&HitPointZ, 4);




		// 3. 총알 수가 0개일 경우 패킷 무시함
		if (m_iBullet == 0)
			return;

		// 총알 수 감소
		--m_iBullet;		




		// 4. 방 안의 유저들에게(자기자신 포함) Fire 1에 대한 응답패킷 보내기
		CProtocolBuff_Net* SendBuff = CProtocolBuff_Net::Alloc();

		WORD Type = en_PACKET_CS_GAME_RES_FIRE1;

		SendBuff->PutData((char*)&Type, 2);
		SendBuff->PutData((char*)&m_Int64AccountNo, 8);

		SendBuff->PutData((char*)&HitPointX, 4);
		SendBuff->PutData((char*)&HitPointY, 4);
		SendBuff->PutData((char*)&HitPointZ, 4);

		NowRoom->SendPacket_BroadCast(SendBuff);



		// 5. 패킷을 받은 시간 갱신
		// 패킷을 브로드캐스트 하는 시간은 100m/s에 안잡히게 하기 위해 여기서 시간 갱신
		m_dwFire1_StartTime = timeGetTime();
	}

	// HitDamage
	//
	// Parameter : CProtocolBuff_Net*
	// return : 없음
	void CBattleServer_Room::CGameSession::Game_HitDamage_Packet(CProtocolBuff_Net* Packet)
	{
		// 로그인 여부 체크
		if (m_bLoginFlag == false)
			g_BattleServer_Room_Dump->Crash();

		// 생존 여부 체크
		// 죽은 유저가 보낸 데미지 패킷은 무시
		if (m_bAliveFlag == false)
			return;


		// 1. 내가있는 방 알아오기
		AcquireSRWLockShared(&m_pParent->m_Room_Umap_srwl);		// ----- Room Umap Shared 락

		auto FindRoom = m_pParent->m_Room_Umap.find(m_iRoomNo);

		// 없으면 크래시
		if (FindRoom == m_pParent->m_Room_Umap.end())
			g_BattleServer_Room_Dump->Crash();

		stRoom* NowRoom = FindRoom->second;

		// 플레이모드가 아니면 크래시
		if (NowRoom->m_iRoomState != eu_ROOM_STATE::PLAY_ROOM)
			g_BattleServer_Room_Dump->Crash();

		ReleaseSRWLockShared(&m_pParent->m_Room_Umap_srwl);		// ----- Room Umap Shared 언락



		// 2. Fire_1 패킷을 받은 시간 체크
		DWORD FireTime = m_dwFire1_StartTime;

		// 기존 시간은 0으로 초기화.
		m_dwFire1_StartTime = 0;

		// 100m/s 이내로 왔는지 체크
		if ((timeGetTime() - FireTime) > 100)
		{
			// 100m/s가 지난 후에 왔다면 패킷 무시
			return;
		}



		// 3. 100 m/s 이내로 왔다면 마샬링
		INT64 TargetAccountNo; // 피해자 AccountNo

		Packet->GetData((char*)&TargetAccountNo, 8);


		// 4. 해당 방에 피해자 AccountNo가 있나 체크
		CGameSession* Target = NowRoom->Find(TargetAccountNo);

		if (Target == nullptr)
		{
			// 없다면 패킷 무시
			// 없을 가능성 있음. 100 m/s 이내에 이미 나갔을 가능성

			//로그 남긴다.
			g_BattleServer_RoomLog->LogSave(L"CBattleServer_Room", CSystemLog::en_LogLevel::LEVEL_DEBUG,
				L"Game_HitDamage_Packet() --> Target Not Find (Attack : %lld, Target : %lld)", m_Int64AccountNo, TargetAccountNo);

			return;
		}

		
		// 5. hp 차감 처리		
		int TargetHP, HelmetCount;
		BYTE HelmetHit;

		// 타겟에게 헬멧이 있는 경우, 헬멧만 1 차감
		if (Target->m_iHelmetCount > 0)
		{
			--Target->m_iHelmetCount;

			// Send할 변수 셋팅
			HelmetCount = Target->m_iHelmetCount;

			TargetHP = Target->m_iHP;
			HelmetHit = 1;
		}

		// 헬멧이 없는 경우, 데미지 적용
		else
		{
			// 피타고라스 정리 (a2 + b2 = c2)
			// 두 점간의 거리 구함 (c)
			float a = (m_fPosX - Target->m_fPosX);
			float b = (m_fPosY - Target->m_fPosY);

			float c = sqrtf((a*a) + (b*b));

			// 거리 기준으로 HP 감소
			// 지금은 그냥 무조건 -10 감소. 
			// 차후 강사님에게 프로토콜 받으면 그때 정상 처리			
			TargetHP = Target->m_iHP - 10;
			if (TargetHP < 0)
				TargetHP = 0;

			Target->m_iHP = TargetHP;			

			// Send할 변수 셋팅
			HelmetCount = 0;

			HelmetHit = 0;
		}
		


		// 6. 방 내의 모든 유저에게 응답패킷 보내기
		// 방 안의 모든 유저에게 보낸다.(자기 자신 포함)
		CProtocolBuff_Net* SendBuff = CProtocolBuff_Net::Alloc();

		WORD Type = en_PACKET_CS_GAME_RES_HIT_DAMAGE;

		SendBuff->PutData((char*)&Type, 2);

		SendBuff->PutData((char*)&m_Int64AccountNo, 8);
		SendBuff->PutData((char*)&TargetAccountNo, 8);
		SendBuff->PutData((char*)&TargetHP, 4);

		SendBuff->PutData((char*)&HelmetHit, 1);
		SendBuff->PutData((char*)&HelmetCount, 4);

		NowRoom->SendPacket_BroadCast(SendBuff);


		// 7. 사망했다면 해당 유저 사망 패킷을 방 전체에 뿌린다.
		if (TargetHP == 0)
		{
			// 피해자의 생존 플래그를 false로 변경
			Target->m_bAliveFlag = false;

			// 룸의 생존 유저 수가 이미 0이었으면 문제 있음. 
			// 모든 유저가 죽었는데 또 죽었다고 한 것.
			if (NowRoom->m_iAliveUserCount == 0)
				g_BattleServer_Room_Dump->Crash();
			
			// 룸의 생존 유저 수 카운트 1 감소
			--NowRoom->m_iAliveUserCount;

			// 패킷 보내기
			CProtocolBuff_Net* SendBuff = CProtocolBuff_Net::Alloc();

			WORD Type = en_PACKET_CS_GAME_RES_DIE;

			SendBuff->PutData((char*)&Type, 2);
			SendBuff->PutData((char*)&TargetAccountNo, 8);			

			NowRoom->SendPacket_BroadCast(SendBuff);



			// DB 저장 파트 -----------------------------------

			// 공격자의 Kill 카운트 증가
			++m_iRecord_Kill;

			// DBWrite 구조체 셋팅
			DB_WORK_CONTENT_UPDATE* KillWrite = (DB_WORK_CONTENT_UPDATE*)m_pParent->m_shDB_Communicate.m_pDB_Work_Pool->Alloc();
		
			KillWrite->m_wWorkType = eu_DB_AFTER_TYPE::eu_WRITE;

			KillWrite->m_iRecord_PlayCount = m_iRecord_PlayCount;
			KillWrite->m_iRecord_PlayTime = m_iRecord_PlayTime;
			KillWrite->m_iRecord_Kill = m_iRecord_Kill;
			KillWrite->m_iRecord_Die = m_iRecord_Die;
			KillWrite->m_iRecord_Win = m_iRecord_Win;

			KillWrite->AccountNo = m_Int64AccountNo;

			// 요청하기
			m_pParent->AddDBWriteCountFunc(m_Int64AccountNo);
			m_pParent->m_shDB_Communicate.DBWriteFunc((DB_WORK*)KillWrite, en_PHP_TYPE::UPDATE_CONTENTS);	


			

			// 사망자의 Die 카운트 증가
			++Target->m_iRecord_Die;

			// DBWrite 구조체 셋팅
			DB_WORK_CONTENT_UPDATE* DieWrite = (DB_WORK_CONTENT_UPDATE*)m_pParent->m_shDB_Communicate.m_pDB_Work_Pool->Alloc();

			DieWrite->m_wWorkType = eu_DB_AFTER_TYPE::eu_WRITE;

			DieWrite->m_iRecord_PlayCount = Target->m_iRecord_PlayCount;
			DieWrite->m_iRecord_PlayTime = Target->m_iRecord_PlayTime;
			DieWrite->m_iRecord_Kill = Target->m_iRecord_Kill;
			DieWrite->m_iRecord_Die = Target->m_iRecord_Die;
			DieWrite->m_iRecord_Win = Target->m_iRecord_Win;

			DieWrite->AccountNo = TargetAccountNo;

			// 요청하기
			m_pParent->AddDBWriteCountFunc(TargetAccountNo);
			m_pParent->m_shDB_Communicate.DBWriteFunc((DB_WORK*)DieWrite, en_PHP_TYPE::UPDATE_CONTENTS);
		}
	}

	// Frie 2 패킷 (발 차기)
	//
	// Parameter : CProtocolBuff_Net*
	// return : 없음
	void CBattleServer_Room::CGameSession::Game_Fire_2_Packet(CProtocolBuff_Net* Packet)
	{
		// 로그인 여부 체크
		if (m_bLoginFlag == false)
			g_BattleServer_Room_Dump->Crash();

		// 생존 여부 체크
		// 죽은 유저의 공격 요청은 무시
		if (m_bAliveFlag == false)
			return;


		// 1. 내가있는 방 알아오기
		AcquireSRWLockShared(&m_pParent->m_Room_Umap_srwl);		// ----- Room Umap Shared 락

		auto FindRoom = m_pParent->m_Room_Umap.find(m_iRoomNo);

		// 없으면 크래시
		if (FindRoom == m_pParent->m_Room_Umap.end())
			g_BattleServer_Room_Dump->Crash();

		stRoom* NowRoom = FindRoom->second;

		// 플레이모드가 아니면 크래시
		if (NowRoom->m_iRoomState != eu_ROOM_STATE::PLAY_ROOM)
			g_BattleServer_Room_Dump->Crash();

		ReleaseSRWLockShared(&m_pParent->m_Room_Umap_srwl);		// ----- Room Umap Shared 언락



		// 2. 마샬링
		float HitPointX;
		float HitPointY;
		float HitPointZ;

		Packet->GetData((char*)&HitPointX, 4);
		Packet->GetData((char*)&HitPointY, 4);
		Packet->GetData((char*)&HitPointZ, 4);



		// 3. 방 안의 유저들에게(자기자신 포함) Fire 2에 대한 응답패킷 보내기
		CProtocolBuff_Net* SendBuff = CProtocolBuff_Net::Alloc();

		WORD Type = en_PACKET_CS_GAME_RES_FIRE2;

		SendBuff->PutData((char*)&Type, 2);
		SendBuff->PutData((char*)&m_Int64AccountNo, 8);

		SendBuff->PutData((char*)&HitPointX, 4);
		SendBuff->PutData((char*)&HitPointY, 4);
		SendBuff->PutData((char*)&HitPointZ, 4);

		NowRoom->SendPacket_BroadCast(SendBuff);



		// 5. 패킷을 받은 시간 갱신
		// 패킷을 브로드캐스트 하는 시간은 100m/s에 안잡히게 하기 위해 여기서 시간 갱신
		m_dwFire2_StartTime = timeGetTime();
	}

	// KickDamage
	//
	// Parameter : CProtocolBuff_Net*
	// return : 없음
	void CBattleServer_Room::CGameSession::Game_KickDamage_Packet(CProtocolBuff_Net* Packet)
	{
		// 로그인 여부 체크
		if (m_bLoginFlag == false)
			g_BattleServer_Room_Dump->Crash();

		// 생존 여부 체크
		// 죽은 유저의 공격 요청은 무시
		if (m_bAliveFlag == false)
			return;


		// 1. 내가있는 방 알아오기
		AcquireSRWLockShared(&m_pParent->m_Room_Umap_srwl);		// ----- Room Umap Shared 락

		auto FindRoom = m_pParent->m_Room_Umap.find(m_iRoomNo);

		// 없으면 크래시
		if (FindRoom == m_pParent->m_Room_Umap.end())
			g_BattleServer_Room_Dump->Crash();

		stRoom* NowRoom = FindRoom->second;

		// 플레이모드가 아니면 크래시
		if (NowRoom->m_iRoomState != eu_ROOM_STATE::PLAY_ROOM)
			g_BattleServer_Room_Dump->Crash();

		ReleaseSRWLockShared(&m_pParent->m_Room_Umap_srwl);		// ----- Room Umap Shared 언락



		// 2. Fire_2 패킷을 받은 시간 체크
		DWORD FireTime = m_dwFire2_StartTime;

		// 기존 시간은 0으로 초기화.
		m_dwFire2_StartTime = 0;

		// 100m/s 이내로 왔는지 체크
		if ((timeGetTime() - FireTime) > 100)
		{
			// 100m/s가 지난 후에 왔다면 패킷 무시
			return;
		}



		// 3. 100 m/s 이내로 왔다면 마샬링
		INT64 TargetAccountNo; // 피해자 AccountNo

		Packet->GetData((char*)&TargetAccountNo, 8);



		// 4. 해당 방에 피해자 AccountNo가 있나 체크
		CGameSession* Target = NowRoom->Find(TargetAccountNo);

		if (Target == nullptr)
		{
			// 없다면 패킷 무시
			// 없을 가능성 있음. 100 m/s 이내에 이미 나갔을 가능성

			//로그 남긴다.
			g_BattleServer_RoomLog->LogSave(L"CBattleServer_Room", CSystemLog::en_LogLevel::LEVEL_DEBUG,
				L"Game_KickDamage_Packet() --> Target Not Find (Attack : %lld, Target : %lld)", m_Int64AccountNo, TargetAccountNo);

			return;
		}


		// 5. 공격자와 피격자의 거리 구하기.

		// 피타고라스 정리 (a2 + b2 = c2)
		// 두 점간의 거리 구함 (c)
		float a = (m_fPosX - Target->m_fPosX);
		float b = (m_fPosY - Target->m_fPosY);

		float c = sqrtf((a*a) + (b*b));

		// 두 점의 거리가 2보다 멀다면, 공격 무시
		if (c > 2)
			return;


		// 6. 타겟의 hp 감소 및 갱신
		int TargetHP = Target->m_iHP - 10;

		if (TargetHP < 0)
			TargetHP = 0;

		Target->m_iHP = TargetHP;


		// HP 감소 패킷 보내기
		CProtocolBuff_Net* SendBuff = CProtocolBuff_Net::Alloc();

		WORD Type = en_PACKET_CS_GAME_RES_KICK_DAMAGE;

		SendBuff->PutData((char*)&Type, 2);

		SendBuff->PutData((char*)&m_Int64AccountNo, 8);
		SendBuff->PutData((char*)&TargetAccountNo, 8);
		SendBuff->PutData((char*)&TargetHP, 4);

		NowRoom->SendPacket_BroadCast(SendBuff);


		// 7. 피격자가 사망했다면 해당 유저 사망 패킷을 방 전체에 뿌린다.
		if (TargetHP == 0)
		{
			// 피해자의 생존 플래그를 false로 변경
			Target->m_bAliveFlag = false;

			// 룸의 생존 유저 수가 이미 0이었으면 문제 있음. 
			// 모든 유저가 죽었는데 또 죽었다고 한 것.
			if (NowRoom->m_iAliveUserCount == 0)
				g_BattleServer_Room_Dump->Crash();

			// 룸의 생존 유저 수 카운트 1 감소
			--NowRoom->m_iAliveUserCount;

			// 패킷 보내기
			CProtocolBuff_Net* SendBuff = CProtocolBuff_Net::Alloc();

			WORD Type = en_PACKET_CS_GAME_RES_DIE;

			SendBuff->PutData((char*)&Type, 2);
			SendBuff->PutData((char*)&TargetAccountNo, 8);

			NowRoom->SendPacket_BroadCast(SendBuff);



			// DB 저장 파트 -----------------------------------

			// 공격자의 Kill 카운트 증가
			++m_iRecord_Kill;

			// DBWrite 구조체 셋팅
			DB_WORK_CONTENT_UPDATE* KillWrite = (DB_WORK_CONTENT_UPDATE*)m_pParent->m_shDB_Communicate.m_pDB_Work_Pool->Alloc();

			KillWrite->m_wWorkType = eu_DB_AFTER_TYPE::eu_WRITE;

			KillWrite->m_iRecord_PlayCount = m_iRecord_PlayCount;
			KillWrite->m_iRecord_PlayTime = m_iRecord_PlayTime;
			KillWrite->m_iRecord_Kill = m_iRecord_Kill;
			KillWrite->m_iRecord_Die = m_iRecord_Die;
			KillWrite->m_iRecord_Win = m_iRecord_Win;

			KillWrite->AccountNo = m_Int64AccountNo;

			// 요청하기
			m_pParent->AddDBWriteCountFunc(m_Int64AccountNo);
			m_pParent->m_shDB_Communicate.DBWriteFunc((DB_WORK*)KillWrite, en_PHP_TYPE::UPDATE_CONTENTS);




			// 사망자의 Die 카운트 증가
			++Target->m_iRecord_Die;

			// DBWrite 구조체 셋팅
			DB_WORK_CONTENT_UPDATE* DieWrite = (DB_WORK_CONTENT_UPDATE*)m_pParent->m_shDB_Communicate.m_pDB_Work_Pool->Alloc();

			DieWrite->m_wWorkType = eu_DB_AFTER_TYPE::eu_WRITE;

			DieWrite->m_iRecord_PlayCount = Target->m_iRecord_PlayCount;
			DieWrite->m_iRecord_PlayTime = Target->m_iRecord_PlayTime;
			DieWrite->m_iRecord_Kill = Target->m_iRecord_Kill;
			DieWrite->m_iRecord_Die = Target->m_iRecord_Die;
			DieWrite->m_iRecord_Win = Target->m_iRecord_Win;

			DieWrite->AccountNo = TargetAccountNo;

			// 요청하기
			m_pParent->AddDBWriteCountFunc(TargetAccountNo);
			m_pParent->m_shDB_Communicate.DBWriteFunc((DB_WORK*)DieWrite, en_PHP_TYPE::UPDATE_CONTENTS);
		}
	}
}

// ------------------
// stRoom의 함수
// (CBattleServer_Room의 이너클래스)
// ------------------
namespace Library_Jingyu
{

	// ------------
	// 멤버 함수
	// ------------

	// 생성자
	CBattleServer_Room::stRoom::stRoom()
	{
		// 미리 메모리 공간 잡아두기
		m_JoinUser_Vector.reserve(10);

		m_iJoinUserCount = 0;
	}
	
	// 소멸자
	CBattleServer_Room::stRoom::~stRoom()
	{
		// 할거 없음. 차후 필요하면 추가.
	}



	// 자료구조 내의 모든 유저에게 인자로 받은 패킷 보내기
	//
	// Parameter : CProtocolBuff_Net*
	// return : 자료구조 내에 유저가 0명일 경우 false
	//		  : 그 외에는 true
	bool CBattleServer_Room::stRoom::SendPacket_BroadCast(CProtocolBuff_Net* SendBuff)
	{
		// 1. 자료구조 내의 유저 수 받기
		size_t Size = m_JoinUser_Vector.size();

		// 2. 유저 수가 0명일 경우, return false
		if (Size == 0)
		{
			CProtocolBuff_Net::Free(SendBuff);
			return false;
		}

		// !! while문 돌기 전에 카운트를, 유저 수 만큼 증가 !!
		// 엔진쪽에서, 완료 통지가 오면 Free를 하기 때문에 Add해야 한다.
		SendBuff->Add((int)Size);

		// 3. 유저 수 만큼 돌면서 패킷 전송.
		size_t Index = 0;

		while (Index < Size)
		{
			m_JoinUser_Vector[Index]->SendPacket(SendBuff);
			++Index;
		}

		// 4. 패킷 Free
		// !! 엔진쪽 완통에서 Free 하지만, 레퍼런스 카운트를 인원 수 만큼 Add 했기 때문에 1개가 더 증가한 상태. !!	
		CProtocolBuff_Net::Free(SendBuff);

		return true;
	}

	// 자료구조 내의 모든 유저에게 인자로 받은 패킷 보내기
	// 인자로 받은 AccountNo를 제외하고 보낸다
	//
	// Parameter : CProtocolBuff_Net*, AccountNo(패킷 안보낼 유저)
	// return : 자료구조 내에 유저가 0명일 경우 false
	//		  : 그 외에는 true
	bool CBattleServer_Room::stRoom::SendPacket_BroadCast(CProtocolBuff_Net* SendBuff, INT64 AccountNo)
	{
		// 1. 자료구조 내의 유저 수 받기
		size_t Size = m_JoinUser_Vector.size();

		// 2. 유저 수가 0명일 경우, return false
		if (Size == 0)
		{
			CProtocolBuff_Net::Free(SendBuff);
			return false;
		}

		// !! while문 돌기 전에 카운트를, 유저 수 - 1 만큼 증가 !!
		// 엔진쪽에서, 완료 통지가 오면 Free를 하기 때문에 Add해야 한다.
		// 자기 자신에게는 안보내기 때문에, 나를 제외한 유저 수만 ++ 한다
		SendBuff->Add((int)Size - 1);

		// 3. 유저 수 만큼 돌면서 패킷 전송.
		size_t Index = 0;

		while (Index < Size)
		{
			// 자기 자신이 아닐 경우에만 보낸다.
			if(m_JoinUser_Vector[Index]->m_Int64AccountNo != AccountNo)
				m_JoinUser_Vector[Index]->SendPacket(SendBuff);

			++Index;
		}

		// 4. 패킷 Free
		// !! 엔진쪽 완통에서 Free 하지만, 레퍼런스 카운트를 인원 수 만큼 Add 했기 때문에 1개가 더 증가한 상태. !!	
		CProtocolBuff_Net::Free(SendBuff);

		return true;
	}



	// 방 내의 모든 유저를 Auth_To_Game으로 변경
	//
	// Parameter : 없음
	// return : 없음
	void CBattleServer_Room::stRoom::ModeChange()
	{
		// 1. 자료구조 내의 유저 수 받기
		size_t Size = m_JoinUser_Vector.size();

		// 2. 유저 수가 0명일 가능성 있음.
		// 카운트 다운 중에 모든 유저가 나갈 경우.
		// 문제로 안본다.
		if (Size == 0)
			return;

		// 3. 0명이 아니라면, 내부에 있는 모든 유저를 AUTH_TO_GAME으로 변경
		size_t Index = 0;

		while (Index < Size)
		{
			m_JoinUser_Vector[Index]->SetMode_GAME();
			++Index;
		}
	}

	// 룸 안의 모든 유저를 생존상태로 변경
	//
	// Parameter : 없음
	// return : 자료구조 내에 유저가 0명일 경우 false
	//		  : 그 외에는 true
	bool CBattleServer_Room::stRoom::AliveFlag_True()
	{
		// 1. 자료구조 내의 유저 수 받기
		size_t Size = m_JoinUser_Vector.size();

		// 2. 유저 수가 0명이거나 0이하일 경우, return false
		if (Size <= 0)
			return false;

		// 3. 유저 수 만큼 돌면서 생존 플래그 변경
		size_t Index = 0;

		while (Index < Size)
		{
			m_JoinUser_Vector[Index]->m_bAliveFlag = true;
			++Index;
		}

		return true;
	}

	// 방 안의 유저들에게 게임 종료 패킷 보내기
	//
	// Parameter : 없음
	// return : 없음
	void CBattleServer_Room::stRoom::GameOver()
	{
		// 1. 방 안에 유저가 0명인 경우 Crash
		if (m_iJoinUserCount == 0)
			g_BattleServer_Room_Dump->Crash();

		// 2. 방 인원수 체크 변수와 vector 안의 사이즈가 달라도 Crash
		size_t Size = m_JoinUser_Vector.size();

		if(Size == 0 || Size != m_iJoinUserCount)
			g_BattleServer_Room_Dump->Crash();		
			   

		// 3. 승리자에게 보낼 승리패킷 만들기
		CProtocolBuff_Net* winPacket = CProtocolBuff_Net::Alloc();

		WORD Type = en_PACKET_CS_GAME_RES_WINNER;
		winPacket->PutData((char*)&Type, 2);


		// 4. 패배자에게 보낼 패배 패킷 만들기
		CProtocolBuff_Net* LosePacket = CProtocolBuff_Net::Alloc();

		Type = en_PACKET_CS_GAME_RES_GAMEOVER;
		LosePacket->PutData((char*)&Type, 2);


		// 5. 순회하면서 패킷 보내기
		size_t Index = 0;
		int WinUserCount = 0;

		while (Index < Size)
		{
			// 유저가 승리자일 경우 (생존자)
			if (m_JoinUser_Vector[Index]->m_bAliveFlag == true)
			{
				// 미리 초기화.
				m_JoinUser_Vector[Index]->m_bAliveFlag = false;

				// 한 게임에 승리자는 1명.
				// WinUserCount가 1인데 여기 들어왔다면, 승리자가 1명 이상이 되었다는 의미.
				// 말도 안됨.
				if(WinUserCount == 1)
					g_BattleServer_Room_Dump->Crash();

				// 전적 카운트 중, 승리 카운트 증가
				++m_JoinUser_Vector[Index]->m_iRecord_Win;

				// 승리 패킷 보내기
				m_JoinUser_Vector[Index]->SendPacket(winPacket);

				// 승리패킷 보낸 수 증가
				++WinUserCount;				
			}

			// 유저가 패배자일 경우 (사망자)
			else
			{
				// 래퍼런스 카운트 증가.
				LosePacket->Add();

				// 패배 패킷 보내기
				m_JoinUser_Vector[Index]->SendPacket(LosePacket);
			}

			++Index;
		}

		// 6. 패배 패킷의 래퍼런스 카운트 감소.
		CProtocolBuff_Net::Free(LosePacket);
	}

	// 방 안의 유저들에게 셧다운 날리기
	//
	// Parameter : 없음
	// return : 없음
	void CBattleServer_Room::stRoom::Shutdown_All()
	{
		// 1. 방 안에 유저가 0명이면 말이 안됨. 밖에서 이미 0이 아니라는것을 알고 왔기 때문에.
		if (m_iJoinUserCount <= 0)
			g_BattleServer_Room_Dump->Crash();

		// 2. 백터 안에 0명이면 크래시
		size_t Size = m_JoinUser_Vector.size();
		if(Size <= 0)
			g_BattleServer_Room_Dump->Crash();

		// 3. 백터 안의 유저와 변수가 다르면 크래시
		if(m_iJoinUserCount != Size)
			g_BattleServer_Room_Dump->Crash();

		// 4. 순회하며 Shutdown
		size_t Index = 0;

		while (Index < Size)
		{
			m_JoinUser_Vector[Index]->Disconnect();

			++Index;
		}	
	}
	
	// 방 안의 모든 유저들에게 나 생성패킷과 다른 유저 생성 패킷 보내기
	// 
	// Parameter : 없음
	// return : 없음
	void CBattleServer_Room::stRoom::CreateCharacter()
	{
		// 1. 방 안에 유저가 0명이면 말이 안됨. 밖에서 이미 0이 아니라는것을 알고 왔기 때문에.
		if (m_iJoinUserCount <= 0)
			g_BattleServer_Room_Dump->Crash();

		// 2. 백터 안에 0명이면 크래시
		size_t Size = m_JoinUser_Vector.size();
		if (Size <= 0)
			g_BattleServer_Room_Dump->Crash();

		// 3. 백터 안의 유저와 변수가 다르면 크래시
		if (m_iJoinUserCount != Size)
			g_BattleServer_Room_Dump->Crash();

		// 4. 캐릭터들의 초기 정보 셋팅
		// OnGame_ClientJoin에서 셋팅하는게 아니라 여기서 셋팅하는 이유는, 생성 위치를 어떤 범위를 지정 후
		// 그 범위의 1, 2, 3, 4 순서대로 지정해줘야 하기 때문에.
		// 만약 OnGame_ClientJoin에서 하게되면 어떤 범위에 몇 번까지 했는지 모름..
		size_t Index = 0;

		DWORD NowTime = timeGetTime();

		while (Index < Size)
		{
			m_JoinUser_Vector[Index]->m_fPosX = 1;
			m_JoinUser_Vector[Index]->m_fPosY = 1;

			m_JoinUser_Vector[Index]->m_iHP = 100;

			// 총알 수
			m_JoinUser_Vector[Index]->m_iBullet = 10;	

			// 탄창 수
			m_JoinUser_Vector[Index]->m_iCartridge = 3;			

			// 게임 시작 시간.
			// 밀리 세컨드로 보관.
			m_JoinUser_Vector[Index]->m_dwGameStartTime = NowTime;	

			++Index;
		}


		// 5. 순회하며 패킷 보냄.		
		int NowUserIndex = 0;
		while (NowUserIndex < Size)
		{
			size_t Index = 0;

			while (Index < Size)
			{
				// 내 캐릭터일 경우, 내 캐릭터 생성 패킷 보냄
				if (NowUserIndex == Index)
				{	
					// 패킷 만들기
					CProtocolBuff_Net* MySendBuff = CProtocolBuff_Net::Alloc();

					WORD Type = en_PACKET_CS_GAME_RES_CREATE_MY_CHARACTER;

					MySendBuff->PutData((char*)&Type, 2);
					MySendBuff->PutData((char*)&m_JoinUser_Vector[Index]->m_fPosX, 4);
					MySendBuff->PutData((char*)&m_JoinUser_Vector[Index]->m_fPosY, 4);
					MySendBuff->PutData((char*)&m_JoinUser_Vector[Index]->m_iHP, 4);
					MySendBuff->PutData((char*)&m_JoinUser_Vector[Index]->m_iBullet, 4);

					// 패킷 보내기	
					m_JoinUser_Vector[Index]->SendPacket(MySendBuff);
				}


				// 다른 사람 캐릭터일 경우, 다른 사람 캐릭터 생성 패킷 보냄.
				else
				{
					// 패킷 만들기
					CProtocolBuff_Net* MySendBuff = CProtocolBuff_Net::Alloc();

					WORD Type = en_PACKET_CS_GAME_RES_CREATE_OTHER_CHARACTER;

					MySendBuff->PutData((char*)&Type, 2);
					MySendBuff->PutData((char*)&m_JoinUser_Vector[Index]->m_Int64AccountNo, 8);
					MySendBuff->PutData((char*)m_JoinUser_Vector[Index]->m_tcNickName, 40);
					MySendBuff->PutData((char*)&m_JoinUser_Vector[Index]->m_fPosX, 4);
					MySendBuff->PutData((char*)&m_JoinUser_Vector[Index]->m_fPosY, 4);
					MySendBuff->PutData((char*)&m_JoinUser_Vector[Index]->m_iHP, 4);
					MySendBuff->PutData((char*)&m_JoinUser_Vector[Index]->m_iBullet, 4);

					// 패킷 보내기	
					m_JoinUser_Vector[Index]->SendPacket(MySendBuff);
				}

				++Index;
			}

			++NowUserIndex;
		}

		
	}
	
	// 방 안의 모든 유저들에게 전적 변경내용 보내기.
	//
	// Parameter : 없음
	// return : 없음
	void CBattleServer_Room::stRoom::RecodeSend()
	{
		// 1. 방 안에 유저가 0명이면 말이 안됨. 밖에서 이미 0이 아니라는것을 알고 왔기 때문에.
		if (m_iJoinUserCount <= 0)
			g_BattleServer_Room_Dump->Crash();

		// 2. 백터 안에 0명이면 크래시
		size_t Size = m_JoinUser_Vector.size();
		if (Size <= 0)
			g_BattleServer_Room_Dump->Crash();

		// 3. 백터 안의 유저와 변수가 다르면 크래시
		if (m_iJoinUserCount != Size)
			g_BattleServer_Room_Dump->Crash();

		// 4. 순회하면서 모든 유저에게 전적 내용 보냄.
		// 동시에 플레이 타임도 갱신. 초단위

		size_t Index = 0;

		while (Index < Size)
		{
			CGameSession* NowPlayer = m_JoinUser_Vector[Index];

			// 1. 플레이 타임 갱신
			int AddTime = (timeGetTime() - NowPlayer->m_dwGameStartTime) / 1000;
			NowPlayer->m_iRecord_PlayTime = NowPlayer->m_iRecord_PlayTime + AddTime;


			// 2. 전적 패킷 만들어서 보내기.
			CProtocolBuff_Net* SendBuff = CProtocolBuff_Net::Alloc();

			WORD Type = en_PACKET_CS_GAME_RES_RECORD;

			SendBuff->PutData((char*)&Type, 2);

			SendBuff->PutData((char*)&NowPlayer->m_iRecord_PlayCount, 4);
			SendBuff->PutData((char*)&NowPlayer->m_iRecord_PlayTime, 4);
			SendBuff->PutData((char*)&NowPlayer->m_iRecord_Kill, 4);
			SendBuff->PutData((char*)&NowPlayer->m_iRecord_Die, 4);
			SendBuff->PutData((char*)&NowPlayer->m_iRecord_Win, 4);

			NowPlayer->SendPacket(SendBuff);


			// 3. 마지막 전적을 DB에 저장하기
			// 강제 종료시(OnGame_ClientLeave)에도 저장해야 하기 때문에, LastDBWriteFlag를 하나 두고 저장 했나 안했나 체크한다.
			NowPlayer->m_bLastDBWriteFlag = true;

			DB_WORK_CONTENT_UPDATE* WriteWork = (DB_WORK_CONTENT_UPDATE*)NowPlayer->m_pParent->m_shDB_Communicate.m_pDB_Work_Pool->Alloc();

			WriteWork->m_wWorkType = eu_DB_AFTER_TYPE::eu_WRITE;

			WriteWork->m_iRecord_PlayCount = NowPlayer->m_iRecord_PlayCount;
			WriteWork->m_iRecord_PlayTime = NowPlayer->m_iRecord_PlayTime;
			WriteWork->m_iRecord_Kill = NowPlayer->m_iRecord_Kill;
			WriteWork->m_iRecord_Die = NowPlayer->m_iRecord_Die;
			WriteWork->m_iRecord_Win = NowPlayer->m_iRecord_Win;

			WriteWork->AccountNo = NowPlayer->m_Int64AccountNo;

			// Write 하기 전에, DBWrite카운트 올려야함.
			NowPlayer->m_pParent->AddDBWriteCountFunc(NowPlayer->m_Int64AccountNo);

			// DBWrite 시도
			NowPlayer->m_pParent->m_shDB_Communicate.DBWriteFunc((DB_WORK*)WriteWork, en_PHP_TYPE::UPDATE_CONTENTS);
		}
	}




	// ------------
	// 자료구조 함수
	// ------------

	// 자료구조에 Insert
	//
	// Parameter : 추가하고자 하는 CGameSession*
	// return : 없음
	void CBattleServer_Room::stRoom::Insert(CGameSession* InsertPlayer)
	{
		m_JoinUser_Vector.push_back(InsertPlayer);
	}

	// 자료구조에 유저가 있나 체크
	//
	// Parameter : AccountNo
	// return : 찾은 유저의 CGameSession*
	//		  : 유저가 없을 시 nullptr
	CBattleServer_Room::CGameSession* CBattleServer_Room::stRoom::Find(INT64 AccountNo)
	{
		size_t Size = m_JoinUser_Vector.size();

		// 1. 자료구조 안에 유저가 0이라면 Crash
		if (Size == 0)
			g_BattleServer_Room_Dump->Crash();


		// 2. 룸 안에 있는 유저 수와 다르다면 Crash
		if (m_iJoinUserCount != Size)
			g_BattleServer_Room_Dump->Crash();


		// 3. 순회하면서 있나 없나 검색
		size_t Index = 0;
		while (Index < Size)
		{
			if (m_JoinUser_Vector[Index]->m_Int64AccountNo == AccountNo)
				return m_JoinUser_Vector[Index];

			++Index;
		}

		// 4. 여기까지 오면 없는것. 
		return nullptr;
	}

	// 자료구조에서 Erase
	//
	// Parameter : 제거하고자 하는 CGameSession*
	// return : 성공 시 true
	//		  : 실패 시  false
	bool CBattleServer_Room::stRoom::Erase(CGameSession* InsertPlayer)
	{
		size_t Size = m_JoinUser_Vector.size();
		bool Flag = false;

		// 1. 자료구조 안에 유저가 0이라면 return false
		if (Size == 0)
			return false;

		// 2. 자료구조 안에 유저가 1명이거나, 찾고자 하는 유저가 마지막에 있다면 바로 제거
		if (Size == 1 || m_JoinUser_Vector[Size - 1] == InsertPlayer)
		{
			Flag = true;
			m_JoinUser_Vector.pop_back();
		}

		// 3. 아니라면 Swap 한다
		else
		{
			size_t Index = 0;
			while (Index < Size)
			{
				// 내가 찾고자 하는 유저를 찾았다면
				if (m_JoinUser_Vector[Index] == InsertPlayer)
				{
					Flag = true;

					CGameSession* Temp = m_JoinUser_Vector[Size - 1];
					m_JoinUser_Vector[Size - 1] = m_JoinUser_Vector[Index];
					m_JoinUser_Vector[Index] = Temp;

					m_JoinUser_Vector.pop_back();

					break;
				}

				++Index;
			}
		}

		// 4. 만약, 제거 못했다면 return false
		if (Flag == false)
			return false;

		return true;
	}

}

// ----------------------------------------
// 
// MMOServer를 이용한 배틀 서버. 룸 버전
//
// ----------------------------------------
namespace Library_Jingyu
{
	// Net 직렬화 버퍼 1개의 크기 (Byte)
	LONG g_lNET_BUFF_SIZE = 512;
		   	  


	// -----------------------
	// 외부에서 사용 가능한 함수
	// -----------------------

	// Start
	// 내부적으로 CMMOServer의 Start, 세션 셋팅까지 한다.
	//
	// Parameter : 없음
	// return : 실패 시 false
	bool CBattleServer_Room::ServerStart()
	{
		// 카운트 값 초기화
		m_lNowWaitRoomCount = 0;
		m_lNowTotalRoomCount = 0;
		m_lGlobal_RoomNo = 0;

		m_lBattleEnterTokenError = 0;
		m_lRoomEnterTokenError = 0;
		m_lQuery_Result_Not_Find = 0;
		m_lTempError = 0;
		m_lTokenError = 0;
		m_lVerError = 0;
		m_DuplicateCount = 0;
		m_DBWrie_LoginCount = 0;
		m_lReadyRoomCount = 0;
		m_lPlayRoomCount = 0;

		// 1. 세션 셋팅
		m_cGameSession = new CGameSession[m_stConfig.MaxJoinUser];

		int i = 0;
		while (i < m_stConfig.MaxJoinUser)
		{
			// GameServer의 포인터 셋팅
			m_cGameSession[i].m_pParent = this;

			// 엔진에 세션 셋팅
			SetSession(&m_cGameSession[i], m_stConfig.MaxJoinUser);
			++i;
		}

		// 2. 모니터링 서버와 연결되는, 랜 클라이언트 가동
		if (m_Monitor_LanClient->ClientStart(m_stConfig.MonitorServerIP, m_stConfig.MonitorServerPort, m_stConfig.MonitorClientCreateWorker,
			m_stConfig.MonitorClientActiveWorker, m_stConfig.MonitorClientNodelay) == false)
			return false;

		// 3. 채팅 랜클라와 연결되는 랜서버 가동
		if (m_Chat_LanServer->ServerStart(m_stConfig.ChatLanServerIP, m_stConfig.ChatPort, m_stConfig.ChatCreateWorker, m_stConfig.ChatActiveWorker,
			m_stConfig.ChatCreateAccept, m_stConfig.ChatNodelay, m_stConfig.ChatMaxJoinUser) == false)
			return false;

		// 4. 마스터 서버와 연결되는, 랜 클라이언트 가동
		if (m_Master_LanClient->ClientStart(m_stConfig.MasterServerIP, m_stConfig.MasterServerPort, m_stConfig.MasterClientCreateWorker,
			m_stConfig.MasterClientActiveWorker, m_stConfig.MasterClientNodelay) == false)
			return false;

		// 4. Battle 서버 시작
		if (Start(m_stConfig.BindIP, m_stConfig.Port, m_stConfig.CreateWorker, m_stConfig.ActiveWorker, m_stConfig.CreateAccept,
			m_stConfig.Nodelay, m_stConfig.MaxJoinUser, m_stConfig.HeadCode, m_stConfig.XORCode1, m_stConfig.XORCode2) == false)
			return false;

		// 서버 오픈 로그 찍기		
		g_BattleServer_RoomLog->LogSave(L"CBattleServer_Room", CSystemLog::en_LogLevel::LEVEL_SYSTEM, L"ServerOpen...");

		return true;

	}

	// Stop
	// 내부적으로 Stop 실행
	//
	// Parameter : 없음
	// return : 없음
	void CBattleServer_Room::ServerStop()
	{
		// 1. 모니터링 클라 종료
		if (m_Monitor_LanClient->GetClinetState() == true)
			m_Monitor_LanClient->ClientStop();

		// 2. 마스터 Lan 클라 종료
		if (m_Master_LanClient->GetClinetState() == true)
			m_Master_LanClient->ClientStop();

		// 3. 서버 종료
		if (GetServerState() == true)
			Stop();

		// 3. 세션 삭제
		delete[] m_cGameSession;
	}

	// 출력용 함수
	//
	// Parameter : 없음
	// return : 없음
	void CBattleServer_Room::ShowPrintf()
	{
		// 화면 출력할 것 셋팅
		/*
		Total SessionNum : 					- MMOServer 의 세션수
		AuthMode SessionNum :				- Auth 모드의 유저 수
		GameMode SessionNum (Auth + Game) :	- Game 모드의 유저 수 (Auth + Game모드 유저 수)

		PacketPool_Net : 		- 외부에서 사용 중인 Net 직렬화 버퍼의 수
		Accept Socket Queue :	- Accept Socket Queue 안의 일감 수

		Accept Total :		- Accept 전체 카운트 (accept 리턴시 +1)
		Accept TPS :		- 초당 Accept 처리 횟수
		Auth FPS :			- 초당 Auth 스레드 처리 횟수
		Game FPS :			- 초당 Game 스레드 처리 횟수

		Send TPS:			- 초당 Send완료 횟수. (완료통지에서 증가)
		Recv TPS:			- 초당 Recv완료 횟수. (패킷 1개가 완성되었을 때 증가. RecvProc에서 패킷에 넣기 전에 1씩 증가)

		WaitRoom :			- 대기방 수
		ReadyRoom :			- 준비방 수
		PlayRoom :			- 플레이방 수
		TotalRoom :			- 총 방 수

		Net_BuffChunkAlloc_Count : - Net 직렬화 버퍼 총 Alloc한 청크 수 (밖에서 사용중인 청크 수)
		ASQPool_ChunkAlloc_Count : - Accept Socket Queue에 들어가는 일감 총 Alloc한 청크 수 (밖에서 사용중인 청크 수)


		------------------ Error -------------------

		Battle_EnterTokenError:		- 배틀서버 입장 토큰 에러
		Room_EnterTokenError:		- 방 입장 토큰 에러
		Login_Query_Not_Find :		- auth의 로그인 패킷에서, DB 쿼리시 -10이 결과로 옴.
		Login_Query_Temp :			- auth의 로그인 패킷에서, DB 쿼리 시 -10이 아닌 에러가 옴.
		Login_UserTokenError :		- Auth의 로그인 패킷에서, 유저 토큰이 다름
		Login_VerError :			- auth의 로그인 패킷에서, 유저가 들고온 버전이 다름
		Login_Duplicate :			- 중복 로그인
		Login_DBWrite :				- DBWrite중인데 또 들어올 경우

		---------- Battle LanServer(Chat) ---------

		SessionNum :				- 배틀 랜서버 (채팅서버)에 접속한 세션 수
		PacketPool_Lan :			- 사용중인 직렬화버퍼 랜 버전. 토탈.

		Lan_BuffChunkAlloc_Count :	- 사용중인 직렬화 버퍼의 청크 수(밖에서 사용중인 수)

		-------------- LanClient -------------------

		Monitor Connect :			- 모니터 서버와 연결되는 랜 클라. 접속 여부
		Master Connect :			- 마스터 서버와 연결되는 랜 클라. 접속여부

		*/

		LONG AuthUser = GetAuthModeUserCount();
		LONG GameUser = GetGameModeUserCount();

		printf("========================================================\n"
			"Total SessionNum : %lld\n"
			"AuthMode SessionNum : %d\n"
			"GameMode SessionNum : %d (Auth + Game : %d)\n\n"

			"PacketPool_Net : %d\n"
			"Accept Socket Queue : %d\n\n"

			"Accept Total : %lld\n"
			"Accept TPS : %d\n"
			"Send TPS : %d\n"
			"Recv TPS : %d\n\n"

			"Auth FPS : %d\n"
			"Game FPS : %d\n\n"

			"Net_BuffChunkAlloc_Count : %d (Out : %d)\n"
			"ASQPool_ChunkAlloc_Count : %d (Out : %d)\n\n"

			"------------------ Room -------------------\n\n"

			"WaitRoom : %d\n"
			"ReadyRoom : %d\n"
			"PlayRoom : %d\n"
			"TotalRoom : %d\n"
			"TotalRoom_Pool : %lld\n\n"

			"Room_ChunkAlloc_Count : %d (Out : %d)\n\n"

			"------------------ Error -------------------\n\n"

			"Battle_EnterTokenError : %d\n"
			"Room_EnterTokenError : %d\n"
			"Login_Query_Not_Find : %d\n"
			"Login_Query_Temp : %d\n"
			"Login_UserTokenError : %d\n"
			"Login_VerError : %d\n"
			"Login_Duplicate :%d\n"
			"Login_DBWrite :%d\n\n"

			"---------- Battle LanServer(Chat) ---------\n\n"

			"SessionNum : %lld\n"
			"PacketPool_Lan : %d\n\n"

			"Lan_BuffChunkAlloc_Count : %d (Out : %d)\n\n"

			"-------------- LanClient -----------\n\n"

			"Monitor Connect : %d\n"
			"Master Connect : %d\n\n"

			"========================================================\n\n",

			// ----------- 게임 서버용
			GetClientCount(),
			AuthUser,
			GameUser, AuthUser + GameUser,

			CProtocolBuff_Net::GetNodeCount(),
			GetASQ_Count(),

			GetAccpetTotal(),
			GetAccpetTPS(),
			GetSendTPS(),
			GetRecvTPS(),

			GetAuthFPS(),
			GetGameFPS(),

			CProtocolBuff_Net::GetChunkCount(), CProtocolBuff_Net::GetOutChunkCount(),
			GetChunkCount(), GetOutChunkCount(),

			m_lNowWaitRoomCount,
			m_lReadyRoomCount,
			m_lPlayRoomCount,
			m_lNowTotalRoomCount,		
			m_Room_Umap.size(),
			m_Room_Pool->GetAllocChunkCount(), m_Room_Pool->GetOutChunkCount(),

			// ----------- 에러
			m_lBattleEnterTokenError,
			m_lRoomEnterTokenError,
			m_lQuery_Result_Not_Find,
			m_lTempError,
			m_lTokenError,
			m_lVerError,
			m_DuplicateCount,
			m_DBWrie_LoginCount,

			// ----------- 배틀 랜서버
			m_Chat_LanServer->GetClientCount(),
			CProtocolBuff_Lan::GetNodeCount(),
			CProtocolBuff_Lan::GetChunkCount(), CProtocolBuff_Lan::GetOutChunkCount(),


			// ----------- 랜클라 (마스터, 모니터)
			m_Monitor_LanClient->GetClinetState(),
			m_Master_LanClient->GetClinetState()
		);
	}

	   


	// -----------------------
	// 내부에서만 사용하는 함수
	// -----------------------

	// 파일에서 Config 정보 읽어오기
	// 
	// Parameter : config 구조체
	// return : 정상적으로 셋팅 시 true
	//		  : 그 외에는 false
	bool CBattleServer_Room::SetFile(stConfigFile* pConfig)
	{
		Parser Parser;

		// 파일 로드
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
		// BATTLESERVER config 읽어오기
		////////////////////////////////////////////////////////

		// 구역 지정 -------------------------
		if (Parser.AreaCheck(_T("BATTLESERVER")) == false)
			return false;

		// IP
		if (Parser.GetValue_String(_T("BindIP"), pConfig->BindIP) == false)
			return false;

		// Port
		if (Parser.GetValue_Int(_T("Port"), &pConfig->Port) == false)
			return false;

		// 생성 워커 수
		if (Parser.GetValue_Int(_T("CreateWorker"), &pConfig->CreateWorker) == false)
			return false;

		// 활성화 워커 수
		if (Parser.GetValue_Int(_T("ActiveWorker"), &pConfig->ActiveWorker) == false)
			return false;

		// 생성 엑셉트
		if (Parser.GetValue_Int(_T("CreateAccept"), &pConfig->CreateAccept) == false)
			return false;

		// 헤더 코드
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

		// 최대 접속 가능 유저 수
		if (Parser.GetValue_Int(_T("MaxJoinUser"), &pConfig->MaxJoinUser) == false)
			return false;

		// 로그 레벨
		if (Parser.GetValue_Int(_T("LogLevel"), &pConfig->LogLevel) == false)
			return false;




		////////////////////////////////////////////////////////
		// 기본 config 읽어오기
		////////////////////////////////////////////////////////

		// 구역 지정 -------------------------
		if (Parser.AreaCheck(_T("CONFIG")) == false)
			return false;

		// VerCode
		if (Parser.GetValue_Int(_T("VerCode"), &m_uiVer_Code) == false)
			return false;

		// BattleNetServer의 IP
		if (Parser.GetValue_String(_T("BattleNetServerIP"), m_Master_LanClient->m_tcBattleNetServerIP) == false)
			return false;

		// MasterNetServer의 Port 셋팅.
		m_Master_LanClient->m_iBattleNetServerPort = pConfig->Port;		

		// 마스터 서버에 입장 시 들고갈 토큰
		TCHAR m_tcMasterToken[33];
		if (Parser.GetValue_String(_T("MasterEnterToken"), m_tcMasterToken) == false)
			return false;

		// 들고갈 때는 char형으로 들고가기 때문에 변환해서 가지고 있어야한다.
		int len = WideCharToMultiByte(CP_UTF8, 0, m_tcMasterToken, lstrlenW(m_tcMasterToken), 0, 0, 0, 0);
		WideCharToMultiByte(CP_UTF8, 0, m_tcMasterToken, lstrlenW(m_tcMasterToken), m_Master_LanClient->m_cMasterToken, len, 0, 0);



		
		////////////////////////////////////////////////////////
		// 채팅 LanServer Config 읽어오기
		////////////////////////////////////////////////////////

		// 구역 지정 -------------------------
		if (Parser.AreaCheck(_T("CHATLANSERVER")) == false)
			return false;

		// IP
		if (Parser.GetValue_String(_T("ChatBindIP"), pConfig->ChatLanServerIP) == false)
			return false;

		// Port
		if (Parser.GetValue_Int(_T("ChatPort"), &pConfig->ChatPort) == false)
			return false;

		// 생성 워커 수
		if (Parser.GetValue_Int(_T("ChatCreateWorker"), &pConfig->ChatCreateWorker) == false)
			return false;

		// 활성화 워커 수
		if (Parser.GetValue_Int(_T("ChatActiveWorker"), &pConfig->ChatActiveWorker) == false)
			return false;

		// 생성 엑셉트
		if (Parser.GetValue_Int(_T("ChatCreateAccept"), &pConfig->ChatCreateAccept) == false)
			return false;

		// Nodelay
		if (Parser.GetValue_Int(_T("ChatNodelay"), &pConfig->ChatNodelay) == false)
			return false;

		// 최대 접속 가능 유저 수
		if (Parser.GetValue_Int(_T("ChatMaxJoinUser"), &pConfig->ChatMaxJoinUser) == false)
			return false;

		// 로그 레벨
		if (Parser.GetValue_Int(_T("ChatLogLevel"), &pConfig->ChatLogLevel) == false)
			return false;






		////////////////////////////////////////////////////////
		// 마스터 LanClient config 읽어오기
		////////////////////////////////////////////////////////

		// 구역 지정 -------------------------
		if (Parser.AreaCheck(_T("MASTERLANCLIENT")) == false)
			return false;

		// IP
		if (Parser.GetValue_String(_T("MasterServerIP"), pConfig->MasterServerIP) == false)
			return false;

		// Port
		if (Parser.GetValue_Int(_T("MasterServerPort"), &pConfig->MasterServerPort) == false)
			return false;

		// 생성 워커 수
		if (Parser.GetValue_Int(_T("MasterClientCreateWorker"), &pConfig->MasterClientCreateWorker) == false)
			return false;

		// 활성화 워커 수
		if (Parser.GetValue_Int(_T("MasterClientActiveWorker"), &pConfig->MasterClientActiveWorker) == false)
			return false;

		// Nodelay
		if (Parser.GetValue_Int(_T("MasterClientNodelay"), &pConfig->MasterClientNodelay) == false)
			return false;




		////////////////////////////////////////////////////////
		// 모니터링 LanClient config 읽어오기
		////////////////////////////////////////////////////////

		// 구역 지정 -------------------------
		if (Parser.AreaCheck(_T("MONITORLANCLIENT")) == false)
			return false;

		// IP
		if (Parser.GetValue_String(_T("MonitorServerIP"), pConfig->MonitorServerIP) == false)
			return false;

		// Port
		if (Parser.GetValue_Int(_T("MonitorServerPort"), &pConfig->MonitorServerPort) == false)
			return false;

		// 생성 워커 수
		if (Parser.GetValue_Int(_T("MonitorClientCreateWorker"), &pConfig->MonitorClientCreateWorker) == false)
			return false;

		// 활성화 워커 수
		if (Parser.GetValue_Int(_T("MonitorClientActiveWorker"), &pConfig->MonitorClientActiveWorker) == false)
			return false;


		// Nodelay
		if (Parser.GetValue_Int(_T("MonitorClientNodelay"), &pConfig->MonitorClientNodelay) == false)
			return false;


		return true;
	}

	


	// -----------------------
	// 마스터 랜 클라에서 호출하는 함수
	// -----------------------

	// 배틀서버 내에 존재하는 모든 방 중, Wait 상태의 방을 찾아낸다.
	// 찾은 Wait모드 방에 유저가 있는 경우, 모두 접속 종료 시킨다.
	//
	// Parameter : 없음
	// return : 없음
	void CBattleServer_Room::RoomClear()
	{
		// Auth 스레드와 동일하게, Wait모드의 룸 동시접근.
		// 때문에 Exclusive락을 걸어야 한다.
		// Game 스레드와는 동일한 방 접근 없음.
		AcquireSRWLockExclusive(&m_Room_Umap_srwl);		// ----- 룸 Exclusive 락

		// 방이 하나 이상 있다면
		if (m_Room_Umap.size() > 0)
		{
			auto itor_Now = m_Room_Umap.begin();
			auto itor_End = m_Room_Umap.end();

			// 방 순회
			while (itor_Now != itor_End)
			{
				// wait모드의 방인 경우
				if (itor_Now->second->m_iRoomState == eu_ROOM_STATE::WAIT_ROOM)
				{
					stRoom* NowRoom = itor_Now->second;

					// 유저가 있는 방의 경우
					if (NowRoom->m_iJoinUserCount > 0)
					{
						// m_bWaitDelete 플래그를 true로 변경
						NowRoom->m_bWaitDelete = true;

						// 방 안의 모든 유저 Shutdown
						NowRoom->Shutdown_All();
					}

					// 유저가 없는 방의 경우
					else if (NowRoom->m_iJoinUserCount == 0)
					{
						// 방 모드를 Play로 변경. 바로 삭제되도록
						NowRoom->m_iRoomState = eu_ROOM_STATE::PLAY_ROOM;

						InterlockedDecrement(&m_lNowWaitRoomCount);
						InterlockedIncrement(&m_lPlayRoomCount);
					}				
				}

				++itor_Now;
			}

		}

		ReleaseSRWLockExclusive(&m_Room_Umap_srwl);		// ----- 룸 Exclusive 언락
	}


	   	  



	// -----------------------
	// 패킷 후처리 함수
	// -----------------------

	// Login 패킷에 대한 인증 처리 (토큰 체크 등..)
	//
	// Parameter : DB_WORK_LOGIN*
	// return : 없음
	void CBattleServer_Room::Auth_LoginPacket_AUTH(DB_WORK_LOGIN* DBData)
	{
		// 1. ClientKey 체크
		CGameSession* NowPlayer = (CGameSession*)DBData->pPointer;

		// 다르면 이미 종료한 유저로 판단. 가능성 있는 상황
		if (NowPlayer->m_int64ClientKey != DBData->m_i64UniqueKey)
			return;


		// 2. Json데이터 파싱하기 (UTF-16)
		GenericDocument<UTF16<>> Doc;
		Doc.Parse(DBData->m_tcResponse);

		int iResult = Doc[_T("result")].GetInt();


		// 3. DB 요청 결과 확인
		// 결과가 1이 아니라면, 
		if (iResult != 1)
		{
			// 이미 종료 패킷이 간 유저인지 체크
			if (NowPlayer->m_bLogoutFlag == true)
				return;

			// 다른 HTTP 요청이 종료 패킷을 또 보내지 못하도록 플래그 변경.
			NowPlayer->m_bLogoutFlag = true;

			// 에러 로그 찍기
			// 로그 찍기 (로그 레벨 : 에러)
			g_BattleServer_RoomLog->LogSave(L"CBattleServer_Room", CSystemLog::en_LogLevel::LEVEL_ERROR, 
				L"Auth_LoginPacket_AUTH()--> DB Result Error!! AccoutnNo : %lld, Error : %d", NowPlayer->m_Int64AccountNo, iResult);

			WORD Type = en_PACKET_CS_GAME_RES_LOGIN;
			BYTE Result = 2;			

			// 결과가 -10일 경우 (회원가입 자체가 안되어 있음)
			if (iResult == -10)
				InterlockedIncrement(&m_lQuery_Result_Not_Find);

			// 그 외 기타 에러일 경우
			else
				InterlockedIncrement(&m_lTempError);

			CProtocolBuff_Net* SendBuff = CProtocolBuff_Net::Alloc();

			SendBuff->PutData((char*)&Type, 2);
			SendBuff->PutData((char*)&DBData->AccountNo, 8);
			SendBuff->PutData((char*)&Result, 1);

			// 보내고 끊기
			NowPlayer->SendPacket(SendBuff, TRUE);
			return;
		}


		// 4. SessionKey, ConnectToekn, Ver_Code 마샬링
		char SessionKey[64];
		char ConnectToken[32];
		UINT VerCode;

		DBData->m_pBuff->GetData(SessionKey, 64);
		DBData->m_pBuff->GetData(ConnectToken, 32);
		DBData->m_pBuff->GetData((char*)&VerCode, 4);


		// 5. 토큰키 비교
		const TCHAR* tDBToekn = Doc[_T("sessionkey")].GetString();

		char DBToken[64];
		int len = (int)_tcslen(tDBToekn);
		WideCharToMultiByte(CP_UTF8, 0, tDBToekn, (int)_tcslen(tDBToekn), DBToken, len, NULL, NULL);

		if (memcmp(DBToken, SessionKey, 64) != 0)
		{
			// 이미 종료 패킷이 간 유저인지 체크
			if (NowPlayer->m_bLogoutFlag == true)
				return;

			// 에러 로그 찍기
			// 로그 찍기 (로그 레벨 : 에러)
			g_BattleServer_RoomLog->LogSave(L"CBattleServer_Room", CSystemLog::en_LogLevel::LEVEL_ERROR,
				L"Auth_LoginPacket_AUTH()--> SessionKey Error!! AccoutnNo : %lld", NowPlayer->m_Int64AccountNo);

			// 다른 HTTP 요청이 종료 패킷을 또 보내지 못하도록 플래그 변경.
			NowPlayer->m_bLogoutFlag = true;

			// 토큰이 다를경우 Result 3(세션키 오류)를 보낸다.
			InterlockedIncrement(&m_lTokenError);

			WORD Type = en_PACKET_CS_MATCH_RES_LOGIN;
			BYTE Result = 3;

			CProtocolBuff_Net* SendBuff = CProtocolBuff_Net::Alloc();

			SendBuff->PutData((char*)&Type, 2);
			SendBuff->PutData((char*)&DBData->AccountNo, 8);
			SendBuff->PutData((char*)&Result, 1);

			// 보내고 끊기
			NowPlayer->SendPacket(SendBuff, TRUE);
			return;
		}


		// 6. 배틀서버 입장 토큰 비교
		// "현재" 토큰과 먼저 비교
		if (memcmp(ConnectToken, m_cConnectToken_Now, 32) != 0)
		{
			// 다르다면 "이전" 토큰과 비교
			if (memcpy(ConnectToken, m_cConnectToken_Before, 32) != 0)
			{
				// 이미 종료 패킷이 간 유저인지 체크
				if (NowPlayer->m_bLogoutFlag == true)
					return;

				// 에러 로그 찍기
				// 로그 찍기 (로그 레벨 : 에러)
				g_BattleServer_RoomLog->LogSave(L"CBattleServer_Room", CSystemLog::en_LogLevel::LEVEL_ERROR,
					L"Auth_LoginPacket_AUTH()--> BattleEnterToken Error!! AccoutnNo : %lld", NowPlayer->m_Int64AccountNo);

				// 다른 HTTP 요청이 종료 패킷을 또 보내지 못하도록 플래그 변경.
				NowPlayer->m_bLogoutFlag = true;

				InterlockedIncrement(&m_lBattleEnterTokenError);

				// 그래도 다르다면 이상한 유저로 판단.
				// 배틀서버 입장 토큰이 다르다는 패킷 보낸다.
				CProtocolBuff_Net* SendBuff = CProtocolBuff_Net::Alloc();

				WORD Type = en_PACKET_CS_GAME_RES_LOGIN;
				BYTE Result = 3;	// 현재는 세션키 오류랑 같이 취급. 차후 수정될까?

				SendBuff->PutData((char*)&Type, 2);
				SendBuff->PutData((char*)&DBData->AccountNo, 8);
				SendBuff->PutData((char*)&Result, 1);

				// 보내고 끊기
				NowPlayer->SendPacket(SendBuff, TRUE);

				return;
			}
		}


		// 7. 버전 비교 
		if (m_uiVer_Code != VerCode)
		{
			// 이미 종료 패킷이 간 유저인지 체크
			if (NowPlayer->m_bLogoutFlag == true)
				return;

			// 에러 로그 찍기
			// 로그 찍기 (로그 레벨 : 에러)
			g_BattleServer_RoomLog->LogSave(L"CBattleServer_Room", CSystemLog::en_LogLevel::LEVEL_ERROR,
				L"Auth_LoginPacket_AUTH()--> VerCode Error!! AccoutnNo : %lld, Code : %d", NowPlayer->m_Int64AccountNo, VerCode);

			// 다른 HTTP 요청이 종료 패킷을 또 보내지 못하도록 플래그 변경.
			NowPlayer->m_bLogoutFlag = true;

			// 버전이 다를경우 Result 5(버전 오류)를 보낸다.
			InterlockedIncrement(&m_lVerError);

			WORD Type = en_PACKET_CS_MATCH_RES_LOGIN;
			BYTE Result = 5;

			CProtocolBuff_Net* SendBuff = CProtocolBuff_Net::Alloc();

			SendBuff->PutData((char*)&Type, 2);
			SendBuff->PutData((char*)&DBData->AccountNo, 8);
			SendBuff->PutData((char*)&Result, 1);

			// 보내고 끊기
			NowPlayer->SendPacket(SendBuff, TRUE);
			return;
		}


		// 8. 닉네임 복사
		const TCHAR* TempNick = Doc[_T("nick")].GetString();
		StringCchCopy(NowPlayer->m_tcNickName, _Mycountof(NowPlayer->m_tcNickName), TempNick);


		// 9. Contents 정보도 셋팅됐다면 정상 패킷 보냄
		NowPlayer->m_lLoginHTTPCount++;

		if (NowPlayer->m_lLoginHTTPCount == 2)
		{
			// 로그인 패킷 처리 Flag 변경
			NowPlayer->m_bLoginFlag = true;

			// 정상 응답 패킷 보냄
			CProtocolBuff_Net* SendBuff = CProtocolBuff_Net::Alloc();

			WORD Type = en_PACKET_CS_MATCH_RES_LOGIN;
			BYTE Result = 1;

			SendBuff->PutData((char*)&Type, 2);
			SendBuff->PutData((char*)&DBData->AccountNo, 8);
			SendBuff->PutData((char*)&Result, 1);

			NowPlayer->SendPacket(SendBuff);
		}
	}

	// Login 패킷에 대한 Contents 정보 가져오기
	//
	// Parameter : DB_WORK_LOGIN*
	// return : 없음
	void CBattleServer_Room::Auth_LoginPacket_Info(DB_WORK_LOGIN_CONTENTS* DBData)
	{
		// 1. ClientKey 체크
		CGameSession* NowPlayer = (CGameSession*)DBData->pPointer;

		// 다르면 이미 종료한 유저로 판단. 가능성 있는 상황
		if (NowPlayer->m_int64ClientKey != DBData->m_i64UniqueKey)
			return;


		// 2. Json데이터 파싱하기 (UTF-16)
		GenericDocument<UTF16<>> Doc;
		Doc.Parse(DBData->m_tcResponse);

		int iResult = Doc[_T("result")].GetInt();


		// 3. DB 요청 결과 확인
		// 결과가 1이 아니라면, 
		if (iResult != 1)
		{
			// 이미 종료 패킷이 간 유저인지 체크
			if (NowPlayer->m_bLogoutFlag == true)
				return;

			// 에러 로그 찍기
			// 로그 찍기 (로그 레벨 : 에러)
			g_BattleServer_RoomLog->LogSave(L"CBattleServer_Room", CSystemLog::en_LogLevel::LEVEL_ERROR,
				L"Auth_LoginPacket_Info()--> DB Result Error!! AccoutnNo : %lld, Error : %d", NowPlayer->m_Int64AccountNo, iResult);

			// 다른 HTTP 요청이 종료 패킷을 또 보내지 못하도록 플래그 변경.
			NowPlayer->m_bLogoutFlag = true;

			WORD Type = en_PACKET_CS_GAME_RES_LOGIN;
			BYTE Result = 2;

			// 결과가 -10일 경우 (회원가입 자체가 안되어 있음)
			if (iResult == -10)
				InterlockedIncrement(&m_lQuery_Result_Not_Find);

			// 그 외 기타 에러일 경우
			else
				InterlockedIncrement(&m_lTempError);

			CProtocolBuff_Net* SendBuff = CProtocolBuff_Net::Alloc();

			SendBuff->PutData((char*)&Type, 2);
			SendBuff->PutData((char*)&DBData->AccountNo, 8);
			SendBuff->PutData((char*)&Result, 1);

			// 보내고 끊기
			NowPlayer->SendPacket(SendBuff, TRUE);
			return;
		}

		// 4. 여기까지 오면 정상으로 패킷 찾아온 것.

		// 정보 복사
		NowPlayer->m_iRecord_PlayCount = Doc[_T("playcount")].GetInt();	// 플레이 횟수
		NowPlayer->m_iRecord_PlayTime = Doc[_T("playtime")].GetInt();	// 플레이 시간 초단위
		NowPlayer->m_iRecord_Kill = Doc[_T("kill")].GetInt();			// 죽인 횟수
		NowPlayer->m_iRecord_Die = Doc[_T("die")].GetInt();				// 죽은 횟수
		NowPlayer->m_iRecord_Win = Doc[_T("win")].GetInt();				// 최종승리 횟수

		// 5. 인증용 로그인 HTTP 요청이 완료되었다면 정상 패킷 보냄
		NowPlayer->m_lLoginHTTPCount++;

		if (NowPlayer->m_lLoginHTTPCount == 2)
		{
			// 로그인 패킷 처리 Flag 변경
			NowPlayer->m_bLoginFlag = true;

			// 정상 응답 패킷 보냄
			CProtocolBuff_Net* SendBuff = CProtocolBuff_Net::Alloc();

			WORD Type = en_PACKET_CS_MATCH_RES_LOGIN;
			BYTE Result = 1;

			SendBuff->PutData((char*)&Type, 2);
			SendBuff->PutData((char*)&DBData->AccountNo, 8);
			SendBuff->PutData((char*)&Result, 1);

			NowPlayer->SendPacket(SendBuff);
		}

	}

	// DB_Write에 대한 작업 후처리
	//
	// Parameter : DB_WORK_CONTENT_UPDATE*
	// return : 없음
	void CBattleServer_Room::Game_DBWrite(DB_WORK_CONTENT_UPDATE* DBData)
	{
		// 1. Json데이터 파싱하기 (UTF-16)
		GenericDocument<UTF16<>> Doc;
		Doc.Parse(DBData->m_tcResponse);

		int iResult = Doc[_T("result")].GetInt();


		// 2. DB 요청 결과 확인
		// 결과가 1이 아니라면 Crash.
		// Write는 무조건 성공한다는 가정
		if (iResult != 1)
			g_BattleServer_Room_Dump->Crash();


		// 3. DBWriteCount 1 줄이기
		MinDBWriteCountFunc(DBData->AccountNo);		
	}







	// -----------------------
	// AccountNo 자료구조 관리 함수
	// -----------------------

	// AccountNo 자료구조에 유저를 추가하는 함수
	//
	// Parameter : AccountNo, CGameSession*
	// return : 성공 시 true
	//		  : 실패 시 false
	bool CBattleServer_Room::InsertAccountNoFunc(INT64 AccountNo, CGameSession* InsertPlayer)
	{

		AcquireSRWLockExclusive(&m_AccountNo_Umap_srwl);		// AccountNo uamp Exclusive 락

		// 1. Insert
		auto ret = m_AccountNo_Umap.insert(make_pair(AccountNo, InsertPlayer));

		ReleaseSRWLockExclusive(&m_AccountNo_Umap_srwl);		// AccountNo uamp Exclusive 언락

		// 2. 중복키일 시 false 리턴
		if (ret.second == false)
			return false;

		return true;
	}

	// AccountNo 자료구조에서 유저 검색 후, 해당 유저에게 Disconenct 하는 함수
	//
	// Parameter : AccountNo
	// return : 없음
	void CBattleServer_Room::DisconnectAccountNoFunc(INT64 AccountNo)
	{
		AcquireSRWLockShared(&m_AccountNo_Umap_srwl);		// AccountNo uamp Shared 락

		// 1. 검색
		auto ret = m_AccountNo_Umap.find(AccountNo);

		// 2. 없는 유저일 시 그냥 리턴.
		// 여기 락 걸고 들어오기 전에, 이미 유저가 종료했을 수도 있기 때문에
		// 정상적인 상황으로 판단.
		if (ret == m_AccountNo_Umap.end())
		{
			ReleaseSRWLockShared(&m_AccountNo_Umap_srwl);		// AccountNo uamp Shared 언락
			return;
		}

		// 3. 있는 유저라면 Disconnect
		ret->second->Disconnect();

		ReleaseSRWLockShared(&m_AccountNo_Umap_srwl);		// AccountNo uamp Shared 언락
	}


	// AccountNo 자료구조에서 유저를 제거하는 함수
	//
	// Parameter : AccountNo
	// return : 성공 시 true
	//		  : 실패 시 false
	bool CBattleServer_Room::EraseAccountNoFunc(INT64 AccountNo)
	{
		AcquireSRWLockExclusive(&m_AccountNo_Umap_srwl);		// AccountNo uamp Exclusive 락

		// 1. 검색
		auto ret = m_AccountNo_Umap.find(AccountNo);

		// 2. 없는 유저일 시 false 리턴
		if (ret == m_AccountNo_Umap.end())
		{
			ReleaseSRWLockExclusive(&m_AccountNo_Umap_srwl);		// AccountNo uamp Exclusive 언락
			return false;
		}

		// 3. 있는 유저라면 Erase
		m_AccountNo_Umap.erase(ret);

		ReleaseSRWLockExclusive(&m_AccountNo_Umap_srwl);		// AccountNo uamp Shared 언락

		return true;
	}







	// -----------------------
	// DBWrite 카운트 자료구조 관리 함수
	// -----------------------

	// DBWrite 카운트 관리 자료구조에 유저를 추가하는 함수
	//
	// Parameter : AccountNo, Count(int)
	// return : 성공 시 true
	//		  : 실패(키 중복) 시 false
	bool CBattleServer_Room::InsertDBWriteCountFunc(INT64 AccountNo, int WriteCount)
	{
		AcquireSRWLockExclusive(&m_DBWrite_Umap_srwl);		// ----- DBWrite Umap Exclusive 락

		// 1. 추가
		auto ret = m_DBWrite_Umap.insert(make_pair(AccountNo, WriteCount));

		// 추가 실패 시 (중복 키) return false
		if (ret.second == false)
		{
			ReleaseSRWLockExclusive(&m_DBWrite_Umap_srwl);		// ----- DBWrite Umap Exclusive 언락
			return false;
		}

		ReleaseSRWLockExclusive(&m_DBWrite_Umap_srwl);		// ----- DBWrite Umap Exclusive 언락
		return true;
	}

	// DBWrite 카운트 관리 자료구조에서 유저를 검색 후, 
	// 카운트(Value)를 1 증가하는 함수
	//
	// Parameter : AccountNo
	// return : 없음
	void CBattleServer_Room::AddDBWriteCountFunc(INT64 AccountNo)
	{
		AcquireSRWLockExclusive(&m_DBWrite_Umap_srwl);		// ----- DBWrite Umap Exclusive 락

		// 1. 검색
		auto ret = m_DBWrite_Umap.find(AccountNo);

		// 없으면 크래시
		if (ret == m_DBWrite_Umap.end())
			g_BattleServer_Room_Dump->Crash();

	
		// 2. Value의 값 1 증가
		++ret->second;

		ReleaseSRWLockExclusive(&m_DBWrite_Umap_srwl);		// ----- DBWrite Umap Exclusive 언락
	}

	// DBWrite 카운트 관리 자료구조에서 유저를 검색 후, 
	// 카운트(Value)를 1 감소시키는 함수
	// 감소 후 0이되면 Erase한다.
	//
	// Parameter : AccountNo
	// return : 성공 시 true
	//		  : 검색 실패 시 false
	bool CBattleServer_Room::MinDBWriteCountFunc(INT64 AccountNo)
	{
		AcquireSRWLockExclusive(&m_DBWrite_Umap_srwl);		// ----- DBWrite Umap Exclusive 락

		// 1. 검색
		auto ret = m_DBWrite_Umap.find(AccountNo);

		// 없으면 return false
		if (ret == m_DBWrite_Umap.end())
		{
			ReleaseSRWLockExclusive(&m_DBWrite_Umap_srwl);		// ----- DBWrite Umap Exclusive 언락
			return false;
		}

		// 이미 Value가 0이면 크래시
		if(ret->second == 0)
			g_BattleServer_Room_Dump->Crash();


		// 2. Value의 값 1 감소
		--ret->second;


		// 3. 감소 후 0이면 Erase
		if (ret->second == 0)
			m_DBWrite_Umap.erase(ret);

		ReleaseSRWLockExclusive(&m_DBWrite_Umap_srwl);		// ----- DBWrite Umap Exclusive 언락

		return true;
	}








	// ---------------------------------
	// Auth모드의 방 관리 자료구조 변수
	// ---------------------------------

	// 방을 Room 자료구조에 Insert하는 함수
	//
	// Parameter : RoomNo, stRoom*
	// return : 성공 시 true
	//		  : 실패(중복 키) 시 false	
	bool CBattleServer_Room::InsertRoomFunc(int RoomNo, stRoom* InsertRoom)
	{
		AcquireSRWLockExclusive(&m_Room_Umap_srwl);		// ----- Room 자료구조 Exclusive 락 

		// 1. insert
		auto ret = m_Room_Umap.insert(make_pair(RoomNo, InsertRoom));

		ReleaseSRWLockExclusive(&m_Room_Umap_srwl);		// ----- Room 자료구조 Exclusive 언락 

		// 2. 중복키라면 false 리턴
		if (ret.second == false)
			return false;

		return true;
	}






	// -----------------------
	// 가상함수
	// -----------------------

	// AuthThread에서 1Loop마다 1회 호출.
	// 1루프마다 정기적으로 처리해야 하는 일을 한다.
	// 
	// parameter : 없음
	// return : 없음
	void CBattleServer_Room::OnAuth_Update()
	{
		// ------------------- HTTP 통신 후, 후처리
		// 1. Q 사이즈 확인
		int iQSize = m_shDB_Communicate.m_pDB_ReadComplete_Queue->GetInNode();

		// 한 프레임에 최대 m_iHTTP_MAX개의 후처리.
		if (iQSize > m_stConst.m_iHTTP_MAX)
			iQSize = m_stConst.m_iHTTP_MAX;

		// 2. 있으면 로직 진행
		DB_WORK* DBData;
		while (iQSize > 0)
		{
			// 일감 디큐		
			if (m_shDB_Communicate.m_pDB_ReadComplete_Queue->Dequeue(DBData) == -1)
				g_BattleServer_Room_Dump->Crash();

			try
			{
				// 일감 타입에 따라 일감 처리
				switch (DBData->m_wWorkType)
				{
					// 로그인 패킷에 대한 인증처리
				case eu_DB_AFTER_TYPE::eu_LOGIN_AUTH:
					Auth_LoginPacket_AUTH((DB_WORK_LOGIN*)DBData);
					break;

					// 로그인 패킷에 대한 정보 얻어오기 처리
				case eu_DB_AFTER_TYPE::eu_LOGIN_INFO:
					Auth_LoginPacket_Info((DB_WORK_LOGIN_CONTENTS*)DBData);
					break;

					// 없는 일감 타입이면 에러.
				default:
					TCHAR str[200];
					StringCchPrintf(str, 200, _T("OnAuth_Update(). HTTP Type Error. Type : %d"), DBData->m_wWorkType);

					throw CException(str);
				}

			}
			catch (CException& exc)
			{
				// 로그 찍기 (로그 레벨 : 에러)
				g_BattleServer_RoomLog->LogSave(L"CBattleServer_Room", CSystemLog::en_LogLevel::LEVEL_ERROR, L"%s",
					(TCHAR*)exc.GetExceptionText());

				// 덤프
				g_BattleServer_Room_Dump->Crash();
			}

			// 모드에 따라 직렬화 버퍼 반환
			if (DBData->m_wWorkType == eu_DB_AFTER_TYPE::eu_LOGIN_AUTH)
			{
				// DB_WORK 안의 직렬화 버퍼 반환
				CProtocolBuff_Net::Free(DBData->m_pBuff);
			}

			// DB_WORK 반환
			m_shDB_Communicate.m_pDB_Work_Pool->Free(DBData);

			--iQSize;
		}


		// ------------------- 방 상태 처리

		// 한 프레임에 최대 m_iLoopRoomModeChange개의 방 모드 변경
		int ModeChangeCount = m_stConst.m_iLoopRoomModeChange;
		DWORD CmpTime = timeGetTime();

		AcquireSRWLockShared(&m_Room_Umap_srwl);	// ----- Room umap Shared 락

		auto itor_Now = m_Room_Umap.begin();
		auto itor_End = m_Room_Umap.end();

		// 방 자료구조를 처음부터 끝까지 순회
		while (itor_Now != itor_End)
		{
			// 만약, 이번 프레임에, 지정한 만큼의 방 모드 변경이 발생했다면 그만한다.
			if (ModeChangeCount == 0)
				break;

			stRoom* NowRoom = itor_Now->second;

			// 해당 방이 준비방일 경우
			if (NowRoom->m_iRoomState == eu_ROOM_STATE::READY_ROOM)
			{
				// 카운트 다운이 완료되었는지 체크
				if ( (NowRoom->m_dwCountDown + m_stConst.m_iCountDownMSec) <= CmpTime )
				{
					// 1. 생존한 유저 수 갱신
					if (NowRoom->m_iJoinUserCount != NowRoom->m_JoinUser_Vector.size())
						g_BattleServer_Room_Dump->Crash();

					NowRoom->m_iAliveUserCount = NowRoom->m_iJoinUserCount;


					// 2. 룸 내 모든 유저의 생존 플래그 변경
					// false가 리턴될 수 있음. 자료구조 내에 유저가 0명일 수 있기 때문에.
					// 때문에 리턴값 안받는다.
					NowRoom->AliveFlag_True();


					// 3. 방 안의 모든 유저에게, 배틀서버 대기방 플레이 시작 패킷 보내기
					CProtocolBuff_Net* SendBuff = CProtocolBuff_Net::Alloc();

					WORD Type = en_PACKET_CS_GAME_RES_PLAY_START;

					SendBuff->PutData((char*)&Type, 2);
					SendBuff->PutData((char*)&NowRoom->m_iRoomNo, 4);

					// 여기서는 false가 리턴될 수 있음(자료구조 내에 유저가 0명일 수 있음)
					// 카운트다운이 끝나기 전에 모든 유저가 나갈 가능성.
					// 그래서 리턴값 안받는다.
					NowRoom->SendPacket_BroadCast(SendBuff);

					InterlockedDecrement(&m_lReadyRoomCount);
					InterlockedIncrement(&m_lPlayRoomCount);

					// 4. 방 상태를 Play로 변경
					NowRoom->m_iRoomState = eu_ROOM_STATE::PLAY_ROOM;

					// 5. 모든 유저를 AUTH_TO_GAME으로 변경
					NowRoom->ModeChange();

					// 6. 이번 프레임에서 방 모드 변경 남은 카운트 감소
					--ModeChangeCount;					
				}
			}

			++itor_Now;
		}

		ReleaseSRWLockShared(&m_Room_Umap_srwl);	// ----- Room umap Shared 언락





		// ------------------- 방생성
		// !! 대기방 생성 조건 !!
		// - [현재 게임 내에 만들어진 총 방 수 < 최대 존재할 수 있는 방 수] 만족
		// - [현재 대기방 수 < 최대 존재할 수 있는 대기방 수] 만족

		// 마스터 서버와 로그인, 채팅 랜 클라가 로그인 상태일 때만 보낸다.
		// 일단, 마스터에게 연결되어 있어야 배틀서버 No를 받는다.
		// 그리고 채팅서버에게도 전달이 된 후에야 정상적으로 방이 생성되었다고 볼 수 있다.
		// 즉, 방 생성 가능한 시점이 되어야만 생성.
		if (m_Master_LanClient->m_bLoginCheck == true &&
			m_Chat_LanServer->m_bLoginCheck == true)
		{
			// 한 프레임에 최대 m_iLoopCreateRoomCount개의 방 생성
			int LoopCount = m_stConst.m_iLoopCreateRoomCount;

			while (LoopCount > 0)
			{
				// 1. 현재 만들어진 총 방수 체크
				if (m_lNowTotalRoomCount < m_stConst.m_lMaxTotalRoomCount)
				{
					// 2. 현재 대기방 수 체크
					if (m_lNowWaitRoomCount < m_stConst.m_lMaxWaitRoomCount)
					{
						// 여기까지 오면 방생성 조건 만족					

						// 1) 현재 총 방 수 증가
						InterlockedIncrement(&m_lNowTotalRoomCount);


						// 2) 대기방 수 증가.
						InterlockedIncrement(&m_lNowWaitRoomCount);


						// 3) 방 Alloc
						stRoom* NowRoom = m_Room_Pool->Alloc();

						// 만약, 방 안에 유저 수가 0명이 아니면 Crash
						if (NowRoom->m_iJoinUserCount != 0 || NowRoom->m_JoinUser_Vector.size() != 0)
							g_BattleServer_Room_Dump->Crash();


						// 4) 셋팅
						LONG RoomNo = InterlockedIncrement(&m_lGlobal_RoomNo);
						NowRoom->m_iBattleServerNo = m_iServerNo;
						NowRoom->m_iRoomNo = RoomNo;
						NowRoom->m_iRoomState = eu_ROOM_STATE::WAIT_ROOM;
						NowRoom->m_iGameModeUser = 0;
						NowRoom->m_bGameEndFlag = false;
						NowRoom->m_dwGameEndMSec = 0;
						NowRoom->m_bShutdownFlag = false;
						NowRoom->m_bWaitDelete = false;

						// 방 입장 토큰 셋팅				
						WORD Index = rand() % 64;	// 0 ~ 63 중 인덱스 골라내기				
						memcpy_s(NowRoom->m_cEnterToken, 32, m_cRoomEnterToken[Index], 32);


						// 5) 방 관리 자료구조에 추가
						if (InsertRoomFunc(RoomNo, NowRoom) == false)
							g_BattleServer_Room_Dump->Crash();


						// 6) 채팅 서버에게 [신규 대기방 생성 알림] 패킷 보냄
						// 랜 클라를 통해 보낸다.
						// 채팅 응답이 오면 마스터에게도 보낸다.
						m_Chat_LanServer->Packet_NewRoomCreate_Req(NowRoom);

						--LoopCount;
					}

					// 현재 대기방 수가 이미 max라면 방 안만들고 나간다.
					else
						break;
				}

				// 현재 만들어진 총 방수가 이미 max라면 방 안만들고 나간다.
				else
					break;
			}
		}

		


		// ------------------- 토큰 재발급
		// 채팅 랜 서버를 통해 채팅 서버로 토큰 전달.
		// 이게 완료되면, 알아서 마스터 서버에게도 토큰이 전달된다.
		m_Chat_LanServer->Packet_TokenChange_Req();
	}

	// GameThread에서 1Loop마다 1회 호출.
	// 1루프마다 정기적으로 처리해야 하는 일을 한다.
	// 
	// parameter : 없음
	// return : 없음
	void CBattleServer_Room::OnGame_Update()
	{

		// ------------------- HTTP 통신 후, 후처리
		// 1. Q 사이즈 확인
		int iQSize = m_shDB_Communicate.m_pDB_Wirte_End_Queue->GetNodeSize();

		// 한 프레임에 최대 m_iHTTP_MAX개의 후처리.
		if (iQSize > m_stConst.m_iHTTP_MAX)
			iQSize = m_stConst.m_iHTTP_MAX;

		// 2. 있으면 로직 진행
		DB_WORK* DBData;
		while (iQSize > 0)
		{
			// 일감 디큐		
			if (m_shDB_Communicate.m_pDB_Wirte_End_Queue->Dequeue(DBData) == -1)
				g_BattleServer_Room_Dump->Crash();

			try
			{
				// 일감 타입에 따라 일감 처리
				switch (DBData->m_wWorkType)
				{
					// DB_Write 작업 후처리
				case eu_DB_AFTER_TYPE::eu_WRITE:
					Game_DBWrite((DB_WORK_CONTENT_UPDATE*)DBData);
					break;

					// 없는 일감 타입이면 에러.
				default:
					TCHAR str[200];
					StringCchPrintf(str, 200, _T("OnGame_Update(). HTTP Type Error. Type : %d"), DBData->m_wWorkType);

					throw CException(str);
				}

			}
			catch (CException& exc)
			{
				// 로그 찍기 (로그 레벨 : 에러)
				g_BattleServer_RoomLog->LogSave(L"CBattleServer_Room", CSystemLog::en_LogLevel::LEVEL_ERROR, L"%s",
					(TCHAR*)exc.GetExceptionText());

				// 덤프
				g_BattleServer_Room_Dump->Crash();
			}

			// DB_WORK 반환
			m_shDB_Communicate.m_pDB_Work_Pool->Free(DBData);

			--iQSize;			 
		}





		// ------------------- 방 체크

		// 이번 프레임에 삭제될 방 받아두기.
		// 한 프레임에 최대 100개의 방 삭제 가능
		int DeleteRoomNo[100];
		int Index = 0;

		AcquireSRWLockShared(&m_Room_Umap_srwl);	// ----- Room Umap Shared 락
		
		auto itor_Now = m_Room_Umap.begin();
		auto itor_End = m_Room_Umap.end();

		// Step 1. 삭제될 방 체크
		// Step 2. 게임 종료된 방 처리 	
		// Setp 3. 게임 종료 체크
		// Step 4. 생존자가 0명인 경우에도 게임 종료.

		// 모든 방을 순회하며, PLAY 모드인 방에 한해 작업 진행
		while (itor_Now != itor_End)
		{
			// 방 모드 체크. 
			if (itor_Now->second->m_iRoomState == eu_ROOM_STATE::PLAY_ROOM)
			{
				// 여기까지 오면 나만 접근하는 방.
				stRoom* NowRoom = itor_Now->second;

				// Step 1. 삭제될 방 체크
				// 접속한 인원 수가 0명인 방. (생존 수 아님)
				// 게임 종료 후, 모든 유저를 쫒아내는 작업까지 완료된 방.
				if (NowRoom->m_iJoinUserCount == 0)
				{
					if (Index < 100)
					{
						DeleteRoomNo[Index] = NowRoom->m_iRoomNo;
						++Index;
					}
				}

				// Step 2. 게임 종료된 방 처리 
				else if (NowRoom->m_bGameEndFlag == true)
				{
					// 셧다운을 하지 않았을 경우에만 로직 진행.
					// 이미 이전 프레임에 셧다운을 날렸는데, 아직 모든 유저가 종료되지 않아
					// 또 이 로직을 탈 가능성이 있기 때문에.
					if (NowRoom->m_bShutdownFlag == false)
					{
						// 삭제 시간이 되었는지 체크
						if ((NowRoom->m_dwGameEndMSec + m_stConst.m_iRoomCloseDelay) <= timeGetTime())
						{		
							// 셧다운 날렸다는 플래그 변경.
							NowRoom->m_bShutdownFlag = true;

							// 아직 방에 있는 유저에게 Shutdown
							NowRoom->Shutdown_All();
						}
					}
				}				

				// Setp 3. 게임 종료 체크
				// 생존자가 1명이면 게임 종료.
				else if (NowRoom->m_iAliveUserCount == 1)
				{
					// 게임 종료 패킷 보내기
					// - 생존자 1명에겐 승리 패킷
					// - 나머지 접속자들에겐 패배 패킷
					NowRoom->GameOver();

					// 방 내 유저에게 전적 변경내용 보내기
					NowRoom->RecodeSend();

					// 게임 방 종료 플래그 변경
					NowRoom->m_bGameEndFlag = true;

					// 현 시점의 시간 저장
					// 방 파괴 체크 용도
					NowRoom->m_dwGameEndMSec = timeGetTime();
				}

				// Step 4. 생존자가 0명인 경우에도 게임 종료.
				// 방 내의 생존자들이 같은 프레임에 hp가 0이되어 죽을 경우.
				else if (NowRoom->m_iAliveUserCount == 0)
				{
					// 게임 종료 패킷 보내기
					// - 모든 유저에게 패배 패킷 보냄. 승리자 없음
					CProtocolBuff_Net* SendBuff = CProtocolBuff_Net::Alloc();

					WORD Type = en_PACKET_CS_GAME_RES_GAMEOVER;

					SendBuff->PutData((char*)&Type, 2);

					NowRoom->SendPacket_BroadCast(SendBuff);

					// 방 내 유저에게 전적 변경내용 보내기
					NowRoom->RecodeSend();

					// 게임 방 종료 플래그 변경
					NowRoom->m_bGameEndFlag = true;

					// 현 시점의 시간 저장
					// 방 파괴 체크 용도
					NowRoom->m_dwGameEndMSec = timeGetTime();
				}
			}

			++itor_End;
		}

		ReleaseSRWLockShared(&m_Room_Umap_srwl);	// ----- Room Umap Shared 언락




		// ------------------- 방 삭제		

		// Step 1. 방 삭제
		// Step 2. 채팅서버로 방 삭제 패킷 보내기

		// Index가 0보다 크다면, 삭제할 방이 있는 것. 		
		if (Index > 0)
		{
			// Step 1. 방 삭제
			int Index_B = 0;

			AcquireSRWLockExclusive(&m_Room_Umap_srwl);	// ----- Room Umap Exclusive 락		

			while (Index_B < Index)
			{
				// 1. 검색
				auto FindRoom = m_Room_Umap.find(DeleteRoomNo[Index_B]);

				// 여기서 없으면 안됨. 방 삭제는 이 로직에서 밖에 안하기 때문에
				if (FindRoom == m_Room_Umap.end())
					g_BattleServer_Room_Dump->Crash();

				// 2. stRoom* 해제
				m_Room_Pool->Free(FindRoom->second);

				// 3. Erase
				m_Room_Umap.erase(FindRoom);

				InterlockedDecrement(&m_lPlayRoomCount);
				InterlockedDecrement(&m_lNowTotalRoomCount);

				++Index_B;
			}

			ReleaseSRWLockExclusive(&m_Room_Umap_srwl);	// ----- Room Umap Exclusive 언락


			// Step 2. 채팅서버로 방 삭제 패킷 보내기
			// 채팅서버와 연결되어 있을 경우
			if (m_Chat_LanServer->m_bLoginCheck == true)
			{
				Index_B = 0;

				// 채팅서버 세션 ID
				ULONGLONG ChatSessionID = m_Chat_LanServer->m_ullSessionID;

				while (Index_B < Index)
				{
					UINT ReqSequence = InterlockedIncrement(&m_Chat_LanServer->m_uiReqSequence);

					// 패킷 만들기
					CProtocolBuff_Lan* SendBuff = CProtocolBuff_Lan::Alloc();

					WORD Type = en_PACKET_CHAT_BAT_REQ_DESTROY_ROOM;

					SendBuff->PutData((char*)&Type, 2);
					SendBuff->PutData((char*)&m_iServerNo, 4);
					SendBuff->PutData((char*)&DeleteRoomNo[Index_B], 4);
					SendBuff->PutData((char*)&ReqSequence, 4);

					// 패킷 보내기
					m_Chat_LanServer->SendPacket(ChatSessionID, SendBuff);

					++Index_B;
				}
			}

		}
	}

	// 새로운 유저 접속 시, Auth에서 호출된다.
	//
	// parameter : 접속한 유저의 IP, Port
	// return false : 클라이언트 접속 거부
	// return true : 접속 허용
	bool CBattleServer_Room::OnConnectionRequest(TCHAR* IP, USHORT port)
	{
		return true;
	}

	// 워커 스레드가 깨어날 시 호출되는 함수.
	// GQCS 바로 하단에서 호출
	// 
	// parameter : 없음
	// return : 없음
	void CBattleServer_Room::OnWorkerThreadBegin()
	{

	}

	// 워커 스레드가 잠들기 전 호출되는 함수
	// GQCS 바로 위에서 호출
	// 
	// parameter : 없음
	// return : 없음
	void CBattleServer_Room::OnWorkerThreadEnd()
	{

	}

	// 에러 발생 시 호출되는 함수.
	//
	// parameter : 에러 코드(실제 윈도우 에러코드는 WinGetLastError() 함수로 얻기 가능. 없을 경우 0이 리턴됨)
	//			 : 에러 코드에 대한 스트링
	// return : 없음
	void CBattleServer_Room::OnError(int error, const TCHAR* errorStr)
	{
		// 로그 찍기 (로그 레벨 : 에러)
		g_BattleServer_RoomLog->LogSave(L"CBattleServer_Room", CSystemLog::en_LogLevel::LEVEL_ERROR, L"%s (ErrorCode : %d)",
			errorStr, error);
	}

	   	  

	// -----------------
	// 생성자와 소멸자
	// -----------------
	CBattleServer_Room::CBattleServer_Room()
		:CMMOServer()
	{	
		srand((UINT)time(NULL));	

		// 방 입장 토큰 만들어두기
		for (int i = 0; i < 64; ++i)
		{
			for (int h = 0; h < 32; ++h)
			{
				m_cRoomEnterToken[i][h] = (rand() % 128) + 1;
			}
		}	

		// 배틀서버 최초 입장 토큰 만들어 두기
		for (int i = 0; i < 32; ++i)
		{
			m_cConnectToken_Now[i] = (rand() % 128) + 1;
		}

		// 모니터링 서버와 통신하기 위한 LanClient 동적할당
		m_Monitor_LanClient = new CGame_MinitorClient;
		m_Monitor_LanClient->ParentSet(this);

		// 마스터 서버와 통신하기 위한 LanClient 동적할당
		m_Master_LanClient = new CBattle_Master_LanClient;
		m_Master_LanClient->ParentSet(this);

		// 채팅서버와 연결되는 LanServer 동적할당
		m_Chat_LanServer = new CBattle_Chat_LanServer;
		m_Chat_LanServer->SetMasterClient(m_Master_LanClient, this);


		// ------------------- Config정보 셋팅		
		if (SetFile(&m_stConfig) == false)
			g_BattleServer_Room_Dump->Crash();

		// ------------------- 로그 저장할 파일 셋팅
		g_BattleServer_RoomLog->SetDirectory(L"CBattleServer_Room");
		g_BattleServer_RoomLog->SetLogLeve((CSystemLog::en_LogLevel)m_stConfig.LogLevel);
				


		// TLS 동적할당
		m_Room_Pool = new CMemoryPoolTLS<stRoom>(0, false);		

		// reserve 셋팅
		m_AccountNo_Umap.reserve(m_stConfig.MaxJoinUser);
		m_Room_Umap.reserve(m_stConst.m_lMaxTotalRoomCount);
		m_DBWrite_Umap.reserve(m_stConfig.MaxJoinUser);

		//SRW락 초기화
		InitializeSRWLock(&m_AccountNo_Umap_srwl);
		InitializeSRWLock(&m_Room_Umap_srwl);
		InitializeSRWLock(&m_DBWrite_Umap_srwl);
	}

	CBattleServer_Room::~CBattleServer_Room()
	{
		// 모니터링 서버와 통신하는 LanClient 동적해제
		delete m_Monitor_LanClient;

		// 마스터 서버와 통신하는 LanClient 동적 해제
		delete m_Master_LanClient;

		// TLS 동적 해제
		delete m_Room_Pool;
	}

}

// ----------------------------------------
// 
// 마스터 서버와 연결되는 LanClient
//
// ----------------------------------------
namespace Library_Jingyu
{

	// -----------------------
	// 패킷 처리 함수
	// -----------------------

	// 마스터에게 보낸 로그인 요청 응답
	//
	// Parameter : SessionID, CProtocolBuff_Lan*
	// return : 없음
	void CBattle_Master_LanClient::Packet_Login_Res(ULONGLONG SessionID, CProtocolBuff_Lan* Payload)
	{
		// 1. Connect 상태인지 확인
		if (SessionID != m_ullSessionID)
			g_BattleServer_Room_Dump->Crash();

		if(m_ullSessionID == 0xffffffffffffffff)
			g_BattleServer_Room_Dump->Crash();

		if (m_bLoginCheck == true)
			g_BattleServer_Room_Dump->Crash();

		// 2. 마샬링
		int ServerNo;
		Payload->GetData((char*)&ServerNo, 4);


		// 3. 서버 번호 셋팅
		m_BattleServer_this->m_iServerNo = ServerNo;	


		// 4. 최초 로그인이기 때문에 배틀서버 입장 토큰 보내기
		UINT ReqSequence = uiReqSequence;

		CProtocolBuff_Lan* SendBuff = CProtocolBuff_Lan::Alloc();

		WORD Type = en_PACKET_BAT_MAS_REQ_CONNECT_TOKEN;

		SendBuff->PutData((char*)&Type, 2);
		SendBuff->PutData((char*)m_BattleServer_this->m_cConnectToken_Now, 32);
		SendBuff->PutData((char*)&uiReqSequence, 4);

		SendPacket(SessionID, SendBuff);

		
		// 5. 기존에 방이 있었다면, 마스터 서버로 방 생성 패킷 보냄. (지금은, 연결 끊기면 모두 파괴중.)
		// Shared로 접근해도 안전한 이유!
		// 1. OnAuth_Update의 방 상태 체크 부분은, Ready상태의 방만 접근하기 때문에 동일한 방 접근 가능성 X
		// 2. 마찬가지로 OnGame_Update는 Play상태의 방만 접근.
		// 3. 클라이언트가 방 입장 패킷을 배틀로 보내기 위해서는, 해당 방이 마스터에 있어야 함. 
		// 근데, 이 패킷이 마스터에게 방을 보내는 로직이기 때문에, 여기서 패킷을 보내기 전에 방은 없음.
		/*
		AcquireSRWLockShared(&m_BattleServer_this->m_Room_Umap_srwl);	// ----- 룸 Shared 락

		// 방이 존재한다면
		if (m_BattleServer_this->m_Room_Umap.size() > 0)
		{
			auto itor_Now = m_BattleServer_this->m_Room_Umap.begin();
			auto itor_End = m_BattleServer_this->m_Room_Umap.end();

			WORD Type = en_PACKET_BAT_MAS_REQ_CREATED_ROOM;

			// 순회
			while (itor_Now != itor_End)
			{
				// 대기 상태의 방이라면, 마스터에게 방 생성 패킷 보냄
				if (itor_Now->second->m_iRoomState == CBattleServer_Room::eu_ROOM_STATE::WAIT_ROOM)
				{
					CBattleServer_Room::stRoom* NowRoom = itor_Now->second;

					UINT ReqSequence = InterlockedIncrement(&uiReqSequence);

					// 패킷 만들기
					CProtocolBuff_Lan* SendBuff = CProtocolBuff_Lan::Alloc();

					SendBuff->PutData((char*)&Type, 2);
					SendBuff->PutData((char*)&NowRoom->m_iBattleServerNo, 4);
					SendBuff->PutData((char*)&NowRoom->m_iRoomNo, 4);
					SendBuff->PutData((char*)&NowRoom->m_iMaxJoinCount, 4);
					SendBuff->PutData((char*)NowRoom->m_cEnterToken, 32);
					SendBuff->PutData((char*)&ReqSequence, 4);

					// 패킷 보내기
					SendPacket(SessionID, SendBuff);
				}

				++itor_Now;
			}
		}

		ReleaseSRWLockShared(&m_BattleServer_this->m_Room_Umap_srwl);	// ----- 룸 Shared 언락
		*/


		// 6. 로그인 상태로 변경
		m_bLoginCheck = true;
	}

	// 신규 대기방 생성 응답
	//
	// Parameter : SessionID, CProtocolBuff_Lan*
	// return : 없음
	void CBattle_Master_LanClient::Packet_NewRoomCreate_Res(ULONGLONG SessionID, CProtocolBuff_Lan* Payload)
	{
		// 현재 하는거 없음..
	}

	// 토큰 재발행 응답
	//
	// Parameter : SessionID, CProtocolBuff_Lan*
	// return : 없음
	void CBattle_Master_LanClient::Packet_TokenChange_Res(ULONGLONG SessionID, CProtocolBuff_Lan* Payload)
	{
		// 현재 하는거 없음..
	}

	// 방 닫힘 응답
	//
	// Parameter : SessionID, CProtocolBuff_Lan*
	// return : 없음
	void CBattle_Master_LanClient::Packet_RoomClose_Res(ULONGLONG SessionID, CProtocolBuff_Lan* Payload)
	{
		// 현재 하는거 없음..
	}

	// 방 퇴장 응답
	//
	// Parameter : RoomNo, AccountNo
	// return : 없음
	void CBattle_Master_LanClient::Packet_RoomLeave_Res(ULONGLONG SessionID, CProtocolBuff_Lan* Payload)
	{
		// 현재 하는거 없음..
	}




	// -----------------------
	// 채팅 Lan 서버가 호출하는 함수
	// -----------------------

	// 마스터에게, 신규 대기방 생성 패킷 보내기
	//
	// Parameter : RoomNo
	// return : 없음
	void CBattle_Master_LanClient::Packet_NewRoomCreate_Req(int RoomNo)
	{
		// 마스터와 연결이 끊긴 상태라면 패킷 안보낸다.
		if (m_bLoginCheck == false)
			return;

		if (m_ullSessionID == 0xffffffffffffffff)
			g_BattleServer_Room_Dump->Crash();


		// 1. 룸 검색
		AcquireSRWLockShared(&m_BattleServer_this->m_Room_Umap_srwl);	// ----- 룸 Shared 락

		auto FindRoom = m_BattleServer_this->m_Room_Umap.find(RoomNo);

		if (FindRoom == m_BattleServer_this->m_Room_Umap.end())
			g_BattleServer_Room_Dump->Crash();

		CBattleServer_Room::stRoom* NewRoom = FindRoom->second;


		// 2. 패킷 만들기
		WORD Type = en_PACKET_BAT_MAS_REQ_CREATED_ROOM;
		UINT ReqSequence = InterlockedIncrement(&uiReqSequence);

		CProtocolBuff_Lan* SendBuff = CProtocolBuff_Lan::Alloc();

		SendBuff->PutData((char*)&Type, 2);
		SendBuff->PutData((char*)&NewRoom->m_iBattleServerNo, 4);
		SendBuff->PutData((char*)&RoomNo, 4);
		SendBuff->PutData((char*)&NewRoom->m_iMaxJoinCount, 4);
		SendBuff->PutData(NewRoom->m_cEnterToken, 32);

		SendBuff->PutData((char*)&ReqSequence, 4);

		ReleaseSRWLockShared(&m_BattleServer_this->m_Room_Umap_srwl);	// ----- 룸 Shared 언락


		// 3. 마스터에게 Send하기
		SendPacket(m_ullSessionID, SendBuff);
	}

	// 토큰 재발급 함수
	//
	// Parameter : 없음
	// return : 없음
	void CBattle_Master_LanClient::Packet_TokenChange_Req()
	{
		// 마스터와 연결이 끊긴 상태라면 패킷 안보낸다.
		if (m_bLoginCheck == false)
			return;

		if (m_ullSessionID == 0xffffffffffffffff)
			g_BattleServer_Room_Dump->Crash();


		// 1. 토큰 재발급 패킷 만들기
		WORD Type = en_PACKET_BAT_MAS_REQ_CONNECT_TOKEN;
		UINT ReqSequence = InterlockedIncrement(&uiReqSequence);

		CProtocolBuff_Lan* SendBuff = CProtocolBuff_Lan::Alloc();

		SendBuff->PutData((char*)&Type, 2);
		SendBuff->PutData(m_BattleServer_this->m_cConnectToken_Now, 32);
		SendBuff->PutData((char*)&ReqSequence, 4);


		// 2. 마스터에게 패킷 보내기
		SendPacket(m_ullSessionID, SendBuff);
	}

	



	// -----------------------
	// Battle Net 서버가 호출하는 함수
	// -----------------------		
	
	// 마스터에게, 방 닫힘 패킷 보내기
	//
	// Parameter : RoomNo
	// return : 없음
	void CBattle_Master_LanClient::Packet_RoomClose_Req(int RoomNo)
	{
		// 마스터와 연결이 끊긴 상태라면 패킷 안보낸다.
		if (m_bLoginCheck == false)
			return;

		if (m_ullSessionID == 0xffffffffffffffff)
			g_BattleServer_Room_Dump->Crash();

		// 1. 패킷 만들기
		WORD Type = en_PACKET_BAT_MAS_REQ_CLOSED_ROOM;
		UINT ReqSequence = InterlockedIncrement(&uiReqSequence);

		CProtocolBuff_Lan* SendBuff = CProtocolBuff_Lan::Alloc();

		SendBuff->PutData((char*)&Type, 2);
		SendBuff->PutData((char*)&RoomNo, 4);
		SendBuff->PutData((char*)&ReqSequence, 4);

		SendPacket(m_ullSessionID, SendBuff);
	}

	// 마스터에게, 방 퇴장 패킷 보내기
	//
	// Parameter : RoomNo, AccountNo
	// return : 없음
	void CBattle_Master_LanClient::Packet_RoomLeave_Req(int RoomNo, INT64 AccountNo)
	{
		// 마스터와 연결이 끊긴 상태라면 패킷 안보낸다.
		if (m_bLoginCheck == false)
			return;

		if (m_ullSessionID == 0xffffffffffffffff)
			g_BattleServer_Room_Dump->Crash();

		// 1. 패킷 만들기
		WORD Type = en_PACKET_BAT_MAS_REQ_LEFT_USER;
		UINT ReqSequence = InterlockedIncrement(&uiReqSequence);

		CProtocolBuff_Lan* SendBuff = CProtocolBuff_Lan::Alloc();

		SendBuff->PutData((char*)&Type, 2);
		SendBuff->PutData((char*)&RoomNo, 4);
		SendBuff->PutData((char*)&AccountNo, 8);
		SendBuff->PutData((char*)&ReqSequence, 4);

		SendPacket(m_ullSessionID, SendBuff);
	}





	// -----------------------
	// 외부에서 사용 가능한 함수
	// -----------------------

	// 시작 함수
	// 내부적으로, 상속받은 CLanClient의 Start호출.
	//
	// Parameter : 연결할 서버의 IP, 포트, 워커스레드 수, 활성화시킬 워커스레드 수, TCP_NODELAY 사용 여부(true면 사용)
	// return : 성공 시 true , 실패 시 falsel 
	bool CBattle_Master_LanClient::ClientStart(TCHAR* ConnectIP, int Port, int CreateWorker, int ActiveWorker, int Nodelay)
	{
		// 마스터 서버에 연결
		if (Start(ConnectIP, Port, CreateWorker, ActiveWorker, Nodelay) == false)
			return false;

		return true;
	}

	// 종료 함수
	// 내부적으로, 상속받은 CLanClient의 Stop호출.
	// 추가로, 리소스 해제 등
	//
	// Parameter : 없음
	// return : 없음
	void CBattle_Master_LanClient::ClientStop()
	{
		// 마스터 서버와 연결 종료
		Stop();
	}

	// 배틀서버의 this를 입력받는 함수
	// 
	// Parameter : 배틀 서버의 this
	// return : 없음
	void CBattle_Master_LanClient::ParentSet(CBattleServer_Room* ChatThis)
	{
		m_BattleServer_this = ChatThis;
	}




	// -----------------------
	// 가상함수
	// -----------------------

	// 목표 서버에 연결 성공 후, 호출되는 함수 (ConnectFunc에서 연결 성공 후 호출)
	//
	// parameter : 세션키
	// return : 없음
	void CBattle_Master_LanClient::OnConnect(ULONGLONG SessionID)
	{
		if(m_bLoginCheck == true)
			g_BattleServer_Room_Dump->Crash();

		if (m_ullSessionID != 0xffffffffffffffff)
			g_BattleServer_Room_Dump->Crash();

		m_ullSessionID = SessionID;
		
		while (1)
		{
			// 채팅서버랑 연결되었나 체크.
			// 연결 된 상태에서, 마스터 서버로 로그인 패킷 보냄.
			if (m_BattleServer_this->m_Chat_LanServer->m_bLoginCheck == true)
			{
				// 마스터 서버(Lan)로 로그인 패킷 보냄
				CProtocolBuff_Lan* SendBuff = CProtocolBuff_Lan::Alloc();

				WORD Type = en_PACKET_BAT_MAS_REQ_SERVER_ON;

				SendBuff->PutData((char*)&Type, 2);

				SendBuff->PutData((char*)m_tcBattleNetServerIP, 32);
				SendBuff->PutData((char*)&m_iBattleNetServerPort, 2);
				SendBuff->PutData(m_BattleServer_this->m_cConnectToken_Now, 32);
				SendBuff->PutData(m_cMasterToken, 32);

				SendBuff->PutData((char*)m_tcChatNetServerIP, 32);
				SendBuff->PutData((char*)&m_iChatNetServerPort, 2);


				SendPacket(SessionID, SendBuff);

				break;
			}

			Sleep(1);
		}
		
	}

	// 목표 서버에 연결 종료 후 호출되는 함수 (InDIsconnect 안에서 호출)
	//
	// parameter : 세션키
	// return : 없음
	void CBattle_Master_LanClient::OnDisconnect(ULONGLONG SessionID)
	{
		// SessionID를 초기값으로 변경
		m_ullSessionID = 0xffffffffffffffff;

		// 비 로그인 상태로 변경
		m_bLoginCheck = false;

		// 배틀서버에서, Wait모드인 방에 있는 유저들을 모두 쫓아냄.
		m_BattleServer_this->RoomClear();
	}

	// 패킷 수신 완료 후 호출되는 함수.
	//
	// parameter : 유저 세션키, CProtocolBuff_Lan*
	// return : 없음
	void CBattle_Master_LanClient::OnRecv(ULONGLONG SessionID, CProtocolBuff_Lan* Payload)
	{
		// Type 분리
		WORD Type;
		Payload->GetData((char*)&Type, 2);


		// 타입에 따라 분기 처리
		try
		{
			switch (Type)
			{
				// 로그인 요청에 대한 응답
			case en_PACKET_BAT_MAS_RES_SERVER_ON:
				Packet_Login_Res(SessionID, Payload);
				break;
				
				// 신규 대기방 생성 응답
			case en_PACKET_BAT_MAS_RES_CREATED_ROOM:
				Packet_NewRoomCreate_Res(SessionID, Payload);
				break;

				// 토큰 재발행 응답
			case en_PACKET_BAT_MAS_RES_CONNECT_TOKEN:
				Packet_TokenChange_Res(SessionID, Payload);
				break;

				// 방 닫힘 응답
			case en_PACKET_BAT_MAS_RES_CLOSED_ROOM:
				Packet_RoomClose_Res(SessionID, Payload);
				break;

				// 방 퇴장 응답
			case en_PACKET_BAT_MAS_RES_LEFT_USER:
				Packet_RoomLeave_Res(SessionID, Payload);
				break;

				// 없는 타입이면 크래시
			default:
				TCHAR str[100];
				StringCchPrintf(str, 100, _T("CBattle_Master_LanClient. OnRecv(). TypeError. SessionID : %lld, Type : %d"), SessionID, Type);

				throw CException(str);
				break;
			}

		}
		catch (CException& exc)
		{
			// 로그 찍기 (로그 레벨 : 에러)
			g_BattleServer_RoomLog->LogSave(L"CBattleServer_Room", CSystemLog::en_LogLevel::LEVEL_ERROR, L"%s",
				(TCHAR*)exc.GetExceptionText());

			g_BattleServer_Room_Dump->Crash();

			// 접속 끊기 요청
			//Disconnect(SessionID);
		}
	}

	// 패킷 송신 완료 후 호출되는 함수
	//
	// parameter : 유저 세션키, Send 한 사이즈
	// return : 없음
	void CBattle_Master_LanClient::OnSend(ULONGLONG SessionID, DWORD SendSize)
	{}

	// 워커 스레드가 깨어날 시 호출되는 함수.
	// GQCS 바로 하단에서 호출
	// 
	// parameter : 없음
	// return : 없음
	void CBattle_Master_LanClient::OnWorkerThreadBegin()
	{}

	// 워커 스레드가 잠들기 전 호출되는 함수
	// GQCS 바로 위에서 호출
	// 
	// parameter : 없음
	// return : 없음
	void CBattle_Master_LanClient::OnWorkerThreadEnd()
	{}

	// 에러 발생 시 호출되는 함수.
	//
	// parameter : 에러 코드(실제 윈도우 에러코드는 WinGetLastError() 함수로 얻기 가능. 없을 경우 0이 리턴됨)
	//			 : 에러 코드에 대한 스트링
	// return : 없음
	void CBattle_Master_LanClient::OnError(int error, const TCHAR* errorStr)
	{}






	// -----------------------
	// 생성자와 소멸자
	// -----------------------

	// 생성자
	CBattle_Master_LanClient::CBattle_Master_LanClient()
	{
		// SessionID 초기화
		m_ullSessionID = 0xffffffffffffffff;

		// Req시퀀스 초기화
		uiReqSequence = 0;

		// 로그인 상태 false
		m_bLoginCheck = false;
	}

	// 소멸자
	CBattle_Master_LanClient::~CBattle_Master_LanClient()
	{
		// 아직 연결이 되어있으면, 연결 해제
		if (GetClinetState() == true)
			ClientStop();
	}
}

// ---------------
// CGame_MinitorClient
// CLanClienet를 상속받는 모니터링 클라
// ---------------
namespace Library_Jingyu
{
	// -----------------------
	// 생성자와 소멸자
	// -----------------------
	CGame_MinitorClient::CGame_MinitorClient()
		:CLanClient()
	{
		// 모니터링 서버 정보전송 스레드를 종료시킬 이벤트 생성
		//
		// 자동 리셋 Event 
		// 최초 생성 시 non-signalled 상태
		// 이름 없는 Event	
		m_hMonitorThreadExitEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

		m_ullSessionID = 0xffffffffffffffff;
	}

	CGame_MinitorClient::~CGame_MinitorClient()
	{
		// 아직 연결이 되어있으면, 연결 해제
		if (GetClinetState() == true)
			ClientStop();

		// 이벤트 삭제
		CloseHandle(m_hMonitorThreadExitEvent);
	}



	// -----------------------
	// 외부에서 사용 가능한 함수
	// -----------------------

	// 시작 함수
	// 내부적으로, 상속받은 CLanClient의 Start호출.
	//
	// Parameter : 연결할 서버의 IP, 포트, 워커스레드 수, 활성화시킬 워커스레드 수, TCP_NODELAY 사용 여부(true면 사용)
	// return : 성공 시 true , 실패 시 falsel 
	bool CGame_MinitorClient::ClientStart(TCHAR* ConnectIP, int Port, int CreateWorker, int ActiveWorker, int Nodelay)
	{
		// 모니터링 서버에 연결
		if (Start(ConnectIP, Port, CreateWorker, ActiveWorker, Nodelay) == false)
			return false;

		// 모니터링 서버로 정보 전송할 스레드 생성
		m_hMonitorThread = (HANDLE)_beginthreadex(NULL, 0, MonitorThread, this, 0, NULL);

		return true;
	}

	// 종료 함수
	// 내부적으로, 상속받은 CLanClient의 Stop호출.
	// 추가로, 리소스 해제 등
	//
	// Parameter : 없음
	// return : 없음
	void CGame_MinitorClient::ClientStop()
	{
		// 1. 모니터링 서버 정보전송 스레드 종료
		SetEvent(m_hMonitorThreadExitEvent);

		// 종료 대기
		if (WaitForSingleObject(m_hMonitorThread, INFINITE) == WAIT_FAILED)
		{
			DWORD Error = GetLastError();
			printf("MonitorThread Exit Error!!! (%d) \n", Error);
		}

		// 2. 스레드 핸들 반환
		CloseHandle(m_hMonitorThread);

		// 3. 모니터링 서버와 연결 종료
		Stop();
	}

	// 배틀서버의 this를 입력받는 함수
	// 
	// Parameter : 배틀 서버의 this
	// return : 없음
	void CGame_MinitorClient::ParentSet(CBattleServer_Room* ChatThis)
	{
		m_BattleServer_this = ChatThis;
	}




	// -----------------------
	// 내부에서만 사용하는 기능 함수
	// -----------------------

	// 일정 시간마다 모니터링 서버로 정보를 전송하는 스레드
	UINT	WINAPI CGame_MinitorClient::MonitorThread(LPVOID lParam)
	{
		// this 받아두기
		CGame_MinitorClient* g_This = (CGame_MinitorClient*)lParam;

		// 종료 신호 이벤트 받아두기
		HANDLE* hEvent = &g_This->m_hMonitorThreadExitEvent;

		// CPU 사용율 체크 클래스 (채팅서버 소프트웨어)
		CCpuUsage_Process CProcessCPU;

		// CPU 사용율 체크 클래스 (하드웨어)
		CCpuUsage_Processor CProcessorCPU;

		// PDH용 클래스
		CPDH	CPdh;

		while (1)
		{
			// 1초에 한번 깨어나서 정보를 보낸다.
			DWORD Check = WaitForSingleObject(*hEvent, 1000);

			// 이상한 신호라면
			if (Check == WAIT_FAILED)
			{
				DWORD Error = GetLastError();
				printf("MoniterThread Exit Error!!! (%d) \n", Error);
				break;
			}

			// 만약, 종료 신호가 왔다면 스레드 종료.
			else if (Check == WAIT_OBJECT_0)
				break;


			// 그게 아니라면, 일을 한다.
			// 모니터링 서버와 연결중일 때만!
			if (g_This->GetClinetState() == false)
				continue;


			// 프로세서, 프로세스 CPU 사용율, PDH 정보 갱신
			CProcessorCPU.UpdateCpuTime();
			CProcessCPU.UpdateCpuTime();
			CPdh.SetInfo();

			// ----------------------------------
			// 하드웨어 정보 보내기 (프로세서)
			// ----------------------------------
			int TimeStamp = (int)(time(NULL));

			// 1. 하드웨어 CPU 사용률 전체
			g_This->InfoSend(dfMONITOR_DATA_TYPE_SERVER_CPU_TOTAL, (int)CProcessorCPU.ProcessorTotal(), TimeStamp);

			// 2. 하드웨어 사용가능 메모리 (MByte)
			g_This->InfoSend(dfMONITOR_DATA_TYPE_SERVER_AVAILABLE_MEMORY, (int)CPdh.Get_AVA_Mem(), TimeStamp);

			// 3. 하드웨어 이더넷 수신 바이트 (KByte)
			int iData = (int)(CPdh.Get_Net_Recv() / 1024);
			g_This->InfoSend(dfMONITOR_DATA_TYPE_SERVER_NETWORK_RECV, iData, TimeStamp);

			// 4. 하드웨어 이더넷 송신 바이트 (KByte)
			iData = (int)(CPdh.Get_Net_Send() / 1024);
			g_This->InfoSend(dfMONITOR_DATA_TYPE_SERVER_NETWORK_SEND, iData, TimeStamp);

			// 5. 하드웨어 논페이지 메모리 사용량 (MByte)
			iData = (int)(CPdh.Get_NonPaged_Mem() / 1024 / 1024);
			g_This->InfoSend(dfMONITOR_DATA_TYPE_SERVER_NONPAGED_MEMORY, iData, TimeStamp);



			// ----------------------------------
			// 배틀서버 정보 보내기
			// ----------------------------------

			// 배틀서버가 On일 경우, 게임서버 정보 보낸다.
			if (g_This->m_BattleServer_this->GetServerState() == true)
			{
				// 1. 배틀서버 ON		
				g_This->InfoSend(dfMONITOR_DATA_TYPE_BATTLE_SERVER_ON, TRUE, TimeStamp);

				// 2. 배틀서버 CPU 사용률 (커널 + 유저)
				g_This->InfoSend(dfMONITOR_DATA_TYPE_BATTLE_CPU, (int)CProcessCPU.ProcessTotal(), TimeStamp);

				// 3. 배틀서버 메모리 유저 커밋 사용량 (Private) MByte
				int Data = (int)(CPdh.Get_UserCommit() / 1024 / 1024);
				g_This->InfoSend(dfMONITOR_DATA_TYPE_BATTLE_MEMORY_COMMIT, Data, TimeStamp);

				// 4. 배틀서버 패킷풀 사용량
				g_This->InfoSend(dfMONITOR_DATA_TYPE_BATTLE_PACKET_POOL, CProtocolBuff_Net::GetNodeCount() + CProtocolBuff_Lan::GetNodeCount(), TimeStamp);

				// 5. Auth 스레드 초당 루프 수
				g_This->InfoSend(dfMONITOR_DATA_TYPE_BATTLE_AUTH_FPS, g_This->m_BattleServer_this->GetAuthFPS(), TimeStamp);

				// 6. Game 스레드 초당 루프 수
				g_This->InfoSend(dfMONITOR_DATA_TYPE_BATTLE_GAME_FPS, g_This->m_BattleServer_this->GetGameFPS(), TimeStamp);

				// 7. 배틀서버 접속 세션전체
				g_This->InfoSend(dfMONITOR_DATA_TYPE_BATTLE_SESSION_ALL, (int)g_This->m_BattleServer_this->GetClientCount(), TimeStamp);

				// 8. Auth 스레드 모드 인원
				g_This->InfoSend(dfMONITOR_DATA_TYPE_BATTLE_SESSION_AUTH, g_This->m_BattleServer_this->GetAuthModeUserCount(), TimeStamp);

				// 9. Game 스레드 모드 인원
				g_This->InfoSend(dfMONITOR_DATA_TYPE_BATTLE_SESSION_GAME, g_This->m_BattleServer_this->GetGameModeUserCount(), TimeStamp);

				// 10. 대기방 수

				// 11. 플레이 방 수
			}

		}

		return 0;
	}

	// 모니터링 서버로 데이터 전송
	//
	// Parameter : DataType(BYTE), DataValue(int), TimeStamp(int)
	// return : 없음
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
	// 가상함수
	// -----------------------

	// 목표 서버에 연결 성공 후, 호출되는 함수 (ConnectFunc에서 연결 성공 후 호출)
	//
	// parameter : 세션키
	// return : 없음
	void CGame_MinitorClient::OnConnect(ULONGLONG SessionID)
	{
		if (m_ullSessionID != 0xffffffffffffffff)
			g_BattleServer_Room_Dump->Crash();

		m_ullSessionID = SessionID;

		// 모니터링 서버(Lan)로 로그인 패킷 보냄
		CProtocolBuff_Lan* SendBuff = CProtocolBuff_Lan::Alloc();

		WORD Type = en_PACKET_SS_MONITOR_LOGIN;
		int ServerNo = dfSERVER_NO;

		SendBuff->PutData((char*)&Type, 2);
		SendBuff->PutData((char*)&ServerNo, 4);

		SendPacket(SessionID, SendBuff);
	}

	// 목표 서버에 연결 종료 후 호출되는 함수 (InDIsconnect 안에서 호출)
	//
	// parameter : 세션키
	// return : 없음
	void CGame_MinitorClient::OnDisconnect(ULONGLONG SessionID)
	{
		m_ullSessionID = 0xffffffffffffffff;
	}

	// 패킷 수신 완료 후 호출되는 함수.
	//
	// parameter : 유저 세션키, CProtocolBuff_Lan*
	// return : 없음
	void CGame_MinitorClient::OnRecv(ULONGLONG SessionID, CProtocolBuff_Lan* Payload)
	{}

	// 패킷 송신 완료 후 호출되는 함수
	//
	// parameter : 유저 세션키, Send 한 사이즈
	// return : 없음
	void CGame_MinitorClient::OnSend(ULONGLONG SessionID, DWORD SendSize)
	{}

	// 워커 스레드가 깨어날 시 호출되는 함수.
	// GQCS 바로 하단에서 호출
	// 
	// parameter : 없음
	// return : 없음
	void CGame_MinitorClient::OnWorkerThreadBegin()
	{}

	// 워커 스레드가 잠들기 전 호출되는 함수
	// GQCS 바로 위에서 호출
	// 
	// parameter : 없음
	// return : 없음
	void CGame_MinitorClient::OnWorkerThreadEnd()
	{}

	// 에러 발생 시 호출되는 함수.
	//
	// parameter : 에러 코드(실제 윈도우 에러코드는 WinGetLastError() 함수로 얻기 가능. 없을 경우 0이 리턴됨)
	//			 : 에러 코드에 대한 스트링
	// return : 없음
	void CGame_MinitorClient::OnError(int error, const TCHAR* errorStr)
	{}
}

// ---------------------------
//
// 채팅서버의 랜클라와 연결되는 Lan 서버
//
// ---------------------------
namespace Library_Jingyu
{


	// -----------------------
	// Battle Net 서버가 호출하는 함수
	// -----------------------

	// 채팅 랜클라에게, 신규 대기방 생성 패킷 보내기
	//
	// Parameter : stRoom*
	// return : 없음
	void CBattle_Chat_LanServer::Packet_NewRoomCreate_Req(CBattleServer_Room::stRoom* NewRoom)
	{
		// 접속한 채팅 랜클라가 없다면 패킷 안보낸다.
		if (m_bLoginCheck == false)
			return;

		// 접속한 채팅 클라가 없다면, 안보낸다.
		if (m_ullSessionID == 0xffffffffffffffff)
			g_BattleServer_Room_Dump->Crash();


		// 1. 패킷 만들기
		WORD Type = en_PACKET_CHAT_BAT_REQ_CREATED_ROOM;
		UINT ReqSequence = InterlockedIncrement(&m_uiReqSequence);

		CProtocolBuff_Lan* SendBuff = CProtocolBuff_Lan::Alloc();

		SendBuff->PutData((char*)&Type, 2);
		SendBuff->PutData((char*)&NewRoom->m_iBattleServerNo, 4);
		SendBuff->PutData((char*)&NewRoom->m_iRoomNo, 4);
		SendBuff->PutData((char*)&NewRoom->m_iMaxJoinCount, 4);
		SendBuff->PutData(NewRoom->m_cEnterToken, 32);

		SendBuff->PutData((char*)&ReqSequence, 4);

		// 2. 채팅 클라에게 Send하기
		SendPacket(m_ullSessionID, SendBuff);
	}

	// 채팅 랜클라에게, 토큰 발급
	//
	// Parameter : 없음
	// return : 없음
	void CBattle_Chat_LanServer::Packet_TokenChange_Req()
	{
		// 접속한 채팅 랜클라가 없다면 패킷 안보낸다.
		if (m_bLoginCheck == false)
			return;

		// 접속한 채팅 클라가 없다면, 안보낸다.
		if (m_ullSessionID == 0xffffffffffffffff)
			g_BattleServer_Room_Dump->Crash();


		// 토큰 재발급 시점이 되었는지 확인하기.
		DWORD NowTime = timeGetTime();

		if ((m_dwTokenSendTime + m_BattleServer->m_stConst.m_iTokenChangeSlice) <= NowTime)
		{
			// 1. 토큰 발급 시간 갱신
			m_dwTokenSendTime = NowTime;

			// 2. "현재" 토큰을 "이전" 토큰에 복사
			memcpy_s(m_BattleServer->m_cConnectToken_Now, 32, m_BattleServer->m_cConnectToken_Before, 32);

			// 3. "현재" 토큰 다시 만들기
			int i = 0;
			while (i < 32)
			{
				m_BattleServer->m_cConnectToken_Now[i] = (rand() % 128) + 1;

				++i;
			}

			// 4. 토큰 재발급 패킷 만들기
			WORD Type = en_PACKET_CHAT_BAT_REQ_CONNECT_TOKEN;
			UINT ReqSequence = InterlockedIncrement(&m_uiReqSequence);

			CProtocolBuff_Lan* SendBuff = CProtocolBuff_Lan::Alloc();

			SendBuff->PutData((char*)&Type, 2);
			SendBuff->PutData(m_BattleServer->m_cConnectToken_Now, 32);
			SendBuff->PutData((char*)&ReqSequence, 4);


			// 5. 마스터에게 패킷 보내기
			SendPacket(m_ullSessionID, SendBuff);
		}
	}




	// ------------------------
	// 외부에서 호출 가능한 함수
	// ------------------------

	// 서버 시작
	//
	// Parameter : IP, Port, 생성 워커, 활성화 워커, 생성 엑셉트 스레드, 노딜레이, 최대 접속자 수
	// return : 실패 시 false
	bool CBattle_Chat_LanServer::ServerStart(TCHAR* IP, int Port, int CreateWorker, int ActiveWorker, int CreateAccept, int Nodelay, int MaxUser)
	{
		if (Start(IP, Port, CreateWorker, ActiveWorker, CreateAccept, Nodelay, MaxUser) == false)
			return false;

		return true;
	}

	// 서버 종료
	//
	// Parameter : 없음
	// return : 없음
	void CBattle_Chat_LanServer::ServerStop()
	{
		Stop();
	}




	// -----------------------
	// 패킷 처리 함수
	// -----------------------
	
	// 신규 대기방 생성 패킷 응답.
	// 이 안에서 마스터에게도 보내준다.
	//
	// Parameter : CProtocolBuff_Lan*
	// return : 없음
	void CBattle_Chat_LanServer::Packet_NewRoomCreate_Res(ULONGLONG SessionID, CProtocolBuff_Lan* Packet)
	{
		if (SessionID != m_ullSessionID)
			g_BattleServer_Room_Dump->Crash();

		if (m_ullSessionID == 0xffffffffffffffff)
			g_BattleServer_Room_Dump->Crash();

		if (m_bLoginCheck == false)
			g_BattleServer_Room_Dump->Crash();


		// 1. 마샬링
		int RoomNo;
		Packet->GetData((char*)&RoomNo, 4);

		// 2. 마스터에게 신규 방 생성 패킷 보내기
		m_pMasterClient->Packet_NewRoomCreate_Req(RoomNo);
	}
	
	// 방 삭제에 대한 응답
	//
	// Parameter : SessinID, CProtocolBuff_Lan*
	// return : 없음
	void CBattle_Chat_LanServer::Packet_RoomClose_Res(ULONGLONG SessionID, CProtocolBuff_Lan* Packet)
	{

	}

	// 토큰 재발행에 대한 응답
	// 이 안에서 마스터에게도 보내준다.
	//
	// Parameter : SessionID, CProtocolBuff_Lan*
	// return : 없음
	void CBattle_Chat_LanServer::Packet_TokenChange_Res(ULONGLONG SessionID, CProtocolBuff_Lan* Packet)
	{
		if (SessionID != m_ullSessionID)
			g_BattleServer_Room_Dump->Crash();

		if (m_ullSessionID == 0xffffffffffffffff)
			g_BattleServer_Room_Dump->Crash();

		if (m_bLoginCheck == false)
			g_BattleServer_Room_Dump->Crash();


		// 1. 마스터 서버에게 토큰 보내기
		m_pMasterClient->Packet_TokenChange_Req();
	}

	// 로그인 요청
	//
	// Parameter : SessinID, CProtocolBuff_Lan*
	// return : 없음
	void CBattle_Chat_LanServer::Packet_Login(ULONGLONG SessionID, CProtocolBuff_Lan* Packet)
	{
		if (SessionID != m_ullSessionID)
			g_BattleServer_Room_Dump->Crash();

		if (m_ullSessionID == 0xffffffffffffffff)
			g_BattleServer_Room_Dump->Crash();

		if (m_bLoginCheck == true)
			g_BattleServer_Room_Dump->Crash();

		// 채팅서버의 IP와 Port 가져오기
		Packet->GetData((char*)m_pMasterClient->m_tcChatNetServerIP, 32);
		Packet->GetData((char*)&m_pMasterClient->m_iChatNetServerPort, 2);		


		// 응답 패킷 보내기
		CProtocolBuff_Lan* SendBuff = CProtocolBuff_Lan::Alloc();

		WORD Type = en_PACKET_CHAT_BAT_RES_SERVER_ON;

		SendBuff->PutData((char*)&Type, 2);

		SendPacket(SessionID, SendBuff);



		// 채팅서버로 토큰 변경 패킷 보내기
		UINT ReqSequence = InterlockedIncrement(&m_uiReqSequence);
		CProtocolBuff_Lan* TokenBuff = CProtocolBuff_Lan::Alloc();

		Type = en_PACKET_CHAT_BAT_REQ_CONNECT_TOKEN;

		TokenBuff->PutData((char*)&Type, 2);
		TokenBuff->PutData((char*)m_BattleServer->m_cConnectToken_Now, 32);
		TokenBuff->PutData((char*)&ReqSequence, 4);

		SendPacket(SessionID, TokenBuff);


		// 토큰 발급시간 갱신
		m_dwTokenSendTime = timeGetTime();

		// 로그인 상태로 변경
		m_bLoginCheck = true;
	}






	// --------------------------
	// 내부에서만 사용하는 함수
	// --------------------------

	// 마스터 랜 클라, 배틀 Net 서버 셋팅
	//
	// Parameter : CBattle_Master_LanClient*
	// return : 없음
	void CBattle_Chat_LanServer::SetMasterClient(CBattle_Master_LanClient* SetPoint, CBattleServer_Room* SetPoint2)
	{
		m_pMasterClient = SetPoint;
		m_BattleServer = SetPoint2;
	}



	   	 



	// -----------------------
	// 가상함수
	// -----------------------

	// Accept 직후, 호출된다.
	//
	// parameter : 접속한 유저의 IP, Port
	// return false : 클라이언트 접속 거부
	// return true : 접속 허용
	bool CBattle_Chat_LanServer::OnConnectionRequest(TCHAR* IP, USHORT port)
	{
		// 이미 접속한 유저가 있을 시 return false;
		// 채팅 랜 서버에는 한 클라만 접속 가능
		if (m_ullSessionID != 0xffffffffffffffff)
			return false;

		return true;
	}

	// 연결 후 호출되는 함수 (AcceptThread에서 Accept 후 호출)
	//
	// parameter : 접속한 유저에게 할당된 세션키
	// return : 없음
	void CBattle_Chat_LanServer::OnClientJoin(ULONGLONG SessionID)
	{
		if (m_ullSessionID != 0xffffffffffffffff)
			g_BattleServer_Room_Dump->Crash();

		if(m_bLoginCheck == true)
			g_BattleServer_Room_Dump->Crash();

		m_ullSessionID = SessionID;
	}

	// 연결 종료 후 호출되는 함수 (InDIsconnect 안에서 호출)
	//
	// parameter : 유저 세션키
	// return : 없음
	void CBattle_Chat_LanServer::OnClientLeave(ULONGLONG SessionID)
	{
		m_ullSessionID = 0xffffffffffffffff;
		m_bLoginCheck = false;
	}

	// 패킷 수신 완료 후 호출되는 함수.
	//
	// parameter : 유저 세션키, 받은 패킷
	// return : 없음
	void CBattle_Chat_LanServer::OnRecv(ULONGLONG SessionID, CProtocolBuff_Lan* Payload)
	{
		// 타입 확인
		WORD Type;

		Payload->GetData((char*)&Type, 2);

		try
		{
			// 타입에 따라 분기 처리
			switch (Type)
			{
				// 신규 대기방 생성에 대한 응답
			case en_PACKET_CHAT_BAT_RES_CREATED_ROOM:
				Packet_NewRoomCreate_Res(SessionID, Payload);
				break;			

				// 방 삭제에 대한 응답
			case en_PACKET_CHAT_BAT_RES_DESTROY_ROOM:
				break;

				// 연결 토큰 재발행에 대한 응답
			case en_PACKET_CHAT_BAT_RES_CONNECT_TOKEN:
				Packet_TokenChange_Res(SessionID, Payload);
				break;

				// 로그인 
			case en_PACKET_CHAT_BAT_REQ_SERVER_ON:
				Packet_Login(SessionID, Payload);
				break;

				// 없는 타입이면 크래시
			default:
				TCHAR str[100];
				StringCchPrintf(str, 100, _T("CBattle_Chat_LanServer. OnRecv(). TypeError. SessionID : %lld, Type : %d"), SessionID, Type);

				throw CException(str);
				break;
			}
		}

		catch (CException& exc)
		{
			// 로그 찍기 (로그 레벨 : 에러)
			g_BattleServer_RoomLog->LogSave(L"CBattleServer_Room", CSystemLog::en_LogLevel::LEVEL_ERROR, L"%s",
				(TCHAR*)exc.GetExceptionText());

			g_BattleServer_Room_Dump->Crash();

			// 접속 끊기 요청
			//Disconnect(SessionID);
		}
	}
		

	// 패킷 송신 완료 후 호출되는 함수
	//
	// parameter : 유저 세션키, Send 한 사이즈
	// return : 없음
	void CBattle_Chat_LanServer::OnSend(ULONGLONG SessionID, DWORD SendSize)
	{

	}

	// 워커 스레드가 깨어날 시 호출되는 함수.
	// GQCS 바로 하단에서 호출
	// 
	// parameter : 없음
	// return : 없음
	void CBattle_Chat_LanServer::OnWorkerThreadBegin()
	{

	}

	// 워커 스레드가 잠들기 전 호출되는 함수
	// GQCS 바로 위에서 호출
	// 
	// parameter : 없음
	// return : 없음
	void CBattle_Chat_LanServer::OnWorkerThreadEnd()
	{

	}

	// 에러 발생 시 호출되는 함수.
	//
	// parameter : 에러 코드(실제 윈도우 에러코드는 WinGetLastError() 함수로 얻기 가능. 없을 경우 0이 리턴됨)
	//			 : 에러 코드에 대한 스트링
	// return : 없음
	void CBattle_Chat_LanServer::OnError(int error, const TCHAR* errorStr)
	{

	}




	// ---------------
	// 생성자와 소멸자
	// ----------------

	// 생성자
	CBattle_Chat_LanServer::CBattle_Chat_LanServer()
	{
		m_ullSessionID = 0xffffffffffffffff;
		m_bLoginCheck = false;
	}

	// 소멸자
	CBattle_Chat_LanServer::~CBattle_Chat_LanServer()
	{

	}

}