// ConsoleApplication1.cpp: 콘솔 응용 프로그램의 진입점을 정의합니다.
//

#include "stdafx.h"
#include <process.h>
#include <clocale>
#include <conio.h>
#include <time.h>

#include "Mysql\include\mysql.h"
#include "Mysql\include\errmsg.h"
#include "RingBuff\RingBuff.h"
#include "ProtocolBuff\ProtocolBuff.h"
#include "TestProtocol.h" 

#pragma comment(lib, "Mysql/lib/vs14/mysqlclient")

#include <mmsystem.h>
#pragma comment(lib,"winmm.lib")

#define IP			"127.0.0.1"
#define	USER		"root"
#define PASSWORD	"034689"
#define DBNAME		"test_04"
#define DBPORT		3306

#define QUEUE_SIZE	10000

using namespace Library_Jingyu;



// ----------------------
// 스레드 프로토타입
// ----------------------
// 업데이트 스레드 x 2
UINT	WINAPI	UpdateThread(LPVOID lParam);

// DB 쓰기 스레드 x 1
UINT	WINAPI	DBWriteThread(LPVOID lParam);




// ----------------------
// 함수 프로토타입
// ----------------------
// 전달받은 쿼리문을 DB에 날리는 함수
bool DBQuery(const char* query, MYSQL *conn, MYSQL *connection);

// DB Thread에서 일을하는 함수
bool DBWorking(MYSQL *conn, MYSQL *connection);

// 회원가입 직렬화버퍼 만들기. 헤더까지 다 만들어줌.
bool Create_Payload_Account(CProtocolBuff* Message, WORD MessageType);

// 스테이지 클리어 직렬화버퍼 만들기. 헤더까지 다 만들어줌.
bool Create_Payload_StageClear(CProtocolBuff* Message, WORD MessageType);

// Player 직렬화버퍼 만들기. 헤더까지 다 만들어줌.
bool Create_Payload_Player(CProtocolBuff* Message, WORD MessageType);




// ----------------------
// 전역변수 선언
// ----------------------
// DBWriteThread;
CRingBuff g_DBWrite(QUEUE_SIZE);

// 종료용 이벤트
HANDLE g_ExitEvent;

// DB 스레드용 이벤트
HANDLE g_DBWriteThreadEvent;

// ID, Password용 랜덤 문자열
char RandomStr[] = "aslkjkdflksdnflksjaflkeswjflkasdjflkdsajfsdaf123456sd4f6asfds4f6s5d4fd6sfsd65f1s6ad5f1s65dfsd65f4s6d5465sfsafjslkjflkasjf5645646ssdjflksdjfksjlfkjlsd";

// TPS 출력용
LONG64	g_TPS;

// accountTBL에서 사용
LONG64	g_accountTbl_account;

// StageClearTBL에서 사용
LONG64	g_StageClearTbl_account;
int g_StageID = 1;

// PlayerTBL에서 사용.
LONG64	g_PlayerTbl_account;
int g_Level = 10;
LONG64 g_Exp = 10000;

// 업데이트 스레드에서 슬립 얼마나할지.
int g_iSleep;



int _tmain()
{
	_tsetlocale(LC_ALL, L"korean");

	// 스레드 카운트 입력받기
	_tprintf(L"업데이트 스레드 슬립(MilliSecond) : ");
	scanf_s("%d", &g_iSleep);

	timeBeginPeriod(1);

	// 1. 업데이트 스레드는 2개, DB 쓰기 스레드는 1개다.
	int UpdateThreadCount = 2;
	int DBWriteThreadCount = 1;
	int TotalCount = UpdateThreadCount + DBWriteThreadCount;

	HANDLE* hThread;
	hThread = new HANDLE[TotalCount];



	// 2. DB 이벤트, 종료 이벤트 생성하기
	g_DBWriteThreadEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	g_ExitEvent = CreateEvent(NULL, TRUE, FALSE, NULL);



	// 3. 스레드 생성하기
	// 업데이트 스레드생성
	int i = 0;
	for (; i < UpdateThreadCount; ++i)
	{
		hThread[i] = (HANDLE)_beginthreadex(NULL, 0, UpdateThread, 0, 0, NULL);
		if (hThread[i] == NULL)
		{
			fputs("업데이트 스레드 생성 실패...\n", stdout);
			return 1;
		}
	}	

	// DB 스레드 생성
	for (int Count = 0; Count < DBWriteThreadCount; ++Count)
	{
		hThread[i] = (HANDLE)_beginthreadex(NULL, 0, DBWriteThread, 0, 0, NULL);
		if (hThread[i] == NULL)
		{
			fputs("DB 스레드 생성 실패...\n", stdout);
			return 1;
		}
	}



	// 4. 1초마다 출력할거 하면서, q눌렀나 확인.
	while (1)
	{
		// DB 저장 TPS, DB 큐 사이즈 출력	
		LONG64 TPS = InterlockedExchange64(&g_TPS, 0);
		_tprintf(L"DB 저장 TPS : %lld, Queue 사용중 사이즈 : %d(%d)\n", TPS, g_DBWrite.GetUseSize(), QUEUE_SIZE);

		if (_kbhit())
		{
			int Key = _getch();

			if (Key == 'q' || Key == 'Q')
			{
				fputs("Q키를 눌렀음. 프로그램 종료 시작.\n", stdout);

				// 모든 스레드들한테 종료 신호 줌.
				SetEvent(g_ExitEvent);

				break;
			}

		}	

		Sleep(1000);
	}


	// 5. 스레드 종료 대기
	WaitForMultipleObjects(TotalCount, hThread, true, INFINITE);

	for (int i = 0; i < TotalCount; ++i)
		CloseHandle(hThread[i]);

	CloseHandle(g_DBWriteThreadEvent);
	CloseHandle(g_ExitEvent);

	fputs("모든 스레드 종료!!\n", stdout);

	timeEndPeriod(1);

	return 0;
}




// 업데이트 스레드 x 2
UINT	WINAPI	UpdateThread(LPVOID lParam)
{
	DWORD dwError = 0;
	srand((unsigned)(time(NULL) + (ULONGLONG)&dwError));
	
	while (1)
	{
		// 큐 사이즈가 300이상일때만 Sleep 대기간다.
		if (g_DBWrite.GetUseSize() > 300)
		{
			// 종료 대기 이벤트를 N밀리세컨드동안 기다려본다. 겸사겸사 쉬기도 한다.
			dwError = WaitForSingleObject(g_ExitEvent, g_iSleep);
			if (dwError == WAIT_FAILED)
			{
				wprintf(L"Update Thread Event Error\n");
				wprintf(L"Update Thread Event Error\n");
				wprintf(L"Update Thread Event Error\n");
				wprintf(L"Update Thread Event Error\n");
				break;
			}

			// 종료 신호 받으면, 즉시 종료
			if (dwError == WAIT_OBJECT_0)
				break;
		}

		/*
		// 종료 대기 이벤트를 N밀리세컨드동안 기다려본다. 겸사겸사 쉬기도 한다.
		dwError = WaitForSingleObject(g_ExitEvent, g_iSleep);
		if (dwError == WAIT_FAILED)
		{
			wprintf(L"Accept Thread Event Error\n");
			wprintf(L"Accept Thread Event Error\n");
			wprintf(L"Accept Thread Event Error\n");
			wprintf(L"Accept Thread Event Error\n");
			break;
		}

		// 종료 신호 받으면, 즉시 종료
		if (dwError == WAIT_OBJECT_0)
			break;		
		*/


		// ------------------
		// 로직처리.
		// ------------------
		int value = rand() % 3;

		// value에 따라 로직처리
		switch (value)
		{
			// 회원 가입
		case df_DBQUERY_MSG_NEW_ACCOUNT:
		{
			CProtocolBuff Message;

			// 메시지 만들어서 직렬화 버퍼에 넣기.
			Create_Payload_Account(&Message, df_DBQUERY_MSG_NEW_ACCOUNT);

			// 직렬화 버퍼를 링버퍼에 인큐하기
			// 업데이트 스레드가 2개이기 때문에 락걸고 한다.
			g_DBWrite.EnterLOCK();
			g_DBWrite.Enqueue(Message.GetBufferPtr(), Message.GetUseSize());
			g_DBWrite.LeaveLOCK();

			// DBEvent를 시그널 상태로 변경
			SetEvent(g_DBWriteThreadEvent);

		}
			break;


			// 스테이지 클리어
		case df_DBQUERY_MSG_STAGE_CLEAR:
		{
			CProtocolBuff Message;

			// 메시지 만들어서 직렬화 버퍼에 넣기.
			Create_Payload_StageClear(&Message, df_DBQUERY_MSG_STAGE_CLEAR);

			// 직렬화 버퍼를 링버퍼에 인큐하기
			// 업데이트 스레드가 2개이기 때문에 락걸고 한다.
			g_DBWrite.EnterLOCK();
			g_DBWrite.Enqueue(Message.GetBufferPtr(), Message.GetUseSize());
			g_DBWrite.LeaveLOCK();

			// DBEvent를 시그널 상태로 변경
			SetEvent(g_DBWriteThreadEvent);

		}
			break;


			// Player
		case df_DBQUERY_MSG_PLAYER:
		{
			CProtocolBuff Message;

			// 메시지 만들어서 직렬화 버퍼에 넣기.
			Create_Payload_Player(&Message, df_DBQUERY_MSG_PLAYER);

			// 직렬화 버퍼를 링버퍼에 인큐하기
			// 업데이트 스레드가 2개이기 때문에 락걸고 한다.
			g_DBWrite.EnterLOCK();
			g_DBWrite.Enqueue(Message.GetBufferPtr(), Message.GetUseSize());
			g_DBWrite.LeaveLOCK();

			// DBEvent를 시그널 상태로 변경
			SetEvent(g_DBWriteThreadEvent);
		}

			break;
		default:
			_tprintf(L"없는 타입\n");
			fputs("업데이트 스레드 종료!!\n", stdout);
			return 0;
		}		
	}
	


	fputs("업데이트 스레드 종료!!\n", stdout);
	return 0;
}


// DB 쓰기 스레드 x 1
UINT	WINAPI	DBWriteThread(LPVOID lParam)
{
	// -----------------
	// 최초 실행되면 DB와 연결
	MYSQL	conn;
	MYSQL	*connection = nullptr;

	// 초기화
	mysql_init(&conn);


	// DB 연결
	connection = mysql_real_connect(&conn, IP, USER, PASSWORD, DBNAME, DBPORT, NULL, 0);
	if (connection == NULL)
	{
		printf("Mysql connection error : %s\n", mysql_error(&conn));
		return 1;
	}

	//한글사용을위해추가.
	mysql_set_character_set(connection, "utf8");

	// -----------------

	HANDLE hEvent[2] = { g_ExitEvent, g_DBWriteThreadEvent };
	DWORD dwError = 0;

	while (1)
	{
		// 종료 신호 or 할일 신호를 기다린다. 무한으로
		dwError = WaitForMultipleObjects(2, hEvent, FALSE, INFINITE);		
		if (dwError == WAIT_FAILED)
		{
			wprintf(L"DBWriteThread Event Error\n");
			wprintf(L"DBWriteThread Event Error\n");
			wprintf(L"DBWriteThread Event Error\n");
			wprintf(L"DBWriteThread Event Error\n");
			break;
		}
		
		// 일단 깨어나면, 오류가 아니면 일하러 간다.
		// 이 안에서는, 큐에 뭐가 없을때까지 한다.
		if (DBWorking(&conn, connection) == false)
			break;


		// 깨어난 이유가, 종료신호라면
		if (dwError == WAIT_OBJECT_0)
		{	
			// 종료 신호 받으면, 큐 사이즈가 0일때만 break;
			if(g_DBWrite.GetUseSize() == 0)
				break;

			// 큐 사이즈가 0이 아니라면, 로직처리 후 break.
			else
			{
				DBWorking(&conn, connection);
				break;
			}
		}		

		// 여기까지 왔으면, 이벤트를 non-signaled 상태로 변경 후 다시 쉬러간다.
		ResetEvent(g_DBWriteThreadEvent);	
	}	
	

	// DB 연결 닫기
	mysql_close(connection);

	fputs("DB 쓰기 스레드 종료!!\n", stdout);
	return 0;
}







// DB Thread에서 일을하는 함수
// 큐에서 데이터 꺼낸 후 쿼리문 날리는 일 까지 한다.
bool DBWorking(MYSQL *conn, MYSQL *connection)
{
	while (1)
	{
		if (g_DBWrite.GetUseSize() == 0)
			break;

		// 1. 헤더 사이즈만큼 Dequeue
		st_DBQUERY_HEADER header;
		if (g_DBWrite.Dequeue((char*)&header, df_HEADER_SIZE) != df_HEADER_SIZE)
		{
			_tprintf(L"헤더 디큐 실패\n");
			return false;
		}


		// 2. Type에 따라서 쿼리 만들어서 날린다.
		switch (header.Type)
		{
			// 회원 가입
		case df_DBQUERY_MSG_NEW_ACCOUNT:
		{
			st_DBQUERY_MSG_NEW_ACCOUNT Payload;

			// 1. 페이로드 사이즈만큼 dequeue
			if (g_DBWrite.Dequeue((char*)&Payload, header.Size) != header.Size)
			{
				_tprintf(L"회원가입 페이로드 디큐 실패\n");
				return false;
			}

			// 2. 타입에 맞게 쿼리문 만들기
			char query[200];
			sprintf_s(query, 200, "INSERT INTO `accountTBL`(`accountNo`, `id`,`password`) VALUES(%lld, '%s', '%s')", Payload.iAccountNo, Payload.szID, Payload.szPassword);

			// 3. 쿼리문 날리기
			if (DBQuery(query, conn, connection) == false)
				return false;
		}
		break;

		// 스테이지 클리어
		case df_DBQUERY_MSG_STAGE_CLEAR:
		{
			st_DBQUERY_MSG_STAGE_CLEAR Payload;

			// 1. 페이로드 사이즈만큼 dequeue
			if (g_DBWrite.Dequeue((char*)&Payload, header.Size) != header.Size)
			{
				_tprintf(L"스테이지 클리어 페이로드 디큐 실패\n");
				return false;
			}

			// 2. 타입에 맞게 쿼리문 만들기
			char query[200];
			sprintf_s(query, 200, "INSERT INTO `stageclearTBL`(`accountNo`,`stageID`) VALUES(%lld, %d)", Payload.iAccountNo, Payload.iStageID);

			// 3. 쿼리문 날리기
			if (DBQuery(query, conn, connection) == false)
				return false;
		}
		break;

		// Player
		case df_DBQUERY_MSG_PLAYER:
		{
			st_DBQUERY_MSG_PLAYER Payload;

			// 1. 페이로드 사이즈만큼 dequeue
			if (g_DBWrite.Dequeue((char*)&Payload, header.Size) != header.Size)
			{
				_tprintf(L"Player 페이로드 디큐 실패\n");
				return false;
			}

			// 2. 타입에 맞게 쿼리문 만들기
			char query[200];
			sprintf_s(query, 200, "INSERT INTO `playerTBL`(`accountNo`,`Level`, `Exp`) VALUES(%lld, %d, %lld)", Payload.iAccountNo, Payload.iLevel, Payload.iExp);

			// 3. 쿼리문 날리기
			if (DBQuery(query, conn, connection) == false)
				return false;
		}
		break;

		default:
			_tprintf(L"DBWorking() 존재하지 않는 타입(%d)\n", header.Type);
			return false;
		}

	}
	
	
	return true;
}

// 전달받은 쿼리문을 DB에 날리는 함수
// DBWorking()에서 호출된다.
bool DBQuery(const char* query, MYSQL *conn, MYSQL *connection)
{
	int Error = mysql_query(connection, query);

	// 연결이 끊긴거면 5회동안 재연결 시도
	if (Error == CR_SOCKET_CREATE_ERROR ||
		Error == CR_CONNECTION_ERROR ||
		Error == CR_CONN_HOST_ERROR ||		
		Error == CR_SERVER_HANDSHAKE_ERR||
		Error == CR_SERVER_GONE_ERROR ||
		Error == CR_INVALID_CONN_HANDLE ||
		Error == CR_SERVER_LOST)
	{

		// 초기화
		//mysql_init(conn);

		int Count = 0;
		while (Count < 5)
		{
			// DB 연결
			connection = mysql_real_connect(conn, IP, USER, PASSWORD, DBNAME, DBPORT, NULL, 0);
			if (connection != NULL)
				break;

			Count++;
		}

		// 5회 연결 시도했는데도 연결 실패면, 에러 찍고 리턴 false.
		if (connection == NULL)
		{
			printf("DBQuery()1. Mysql connection error : %s(%d)\n", mysql_error(conn), mysql_errno(conn));
			return false;
		}

		// 연결이 성공했으면 쿼리 다시 날려본다.
		else
		{
			if (mysql_query(connection, query) != 0)
			{
				// 여기서도 실패했으면 그냥 false 리턴.
				printf("DBQuery()1. Mysql query error : %s(%d)\n", mysql_error(conn), mysql_errno(conn));
				return false;

			}			
		}
	}
	
	// 위 if문에 안걸렸는데 Error가 생겼으면 쿼리 에러로 판단하고 false 리턴
	else if (Error != 0)
	{
		printf("DBQuery()1. Mysql query error : %s(%d)\n", mysql_error(conn), mysql_errno(conn));		
		return false;
	}

	// 여기까지 오면 성공한 것이니 TPS 카운트 하나 증가
	InterlockedIncrement64(&g_TPS);

	return true;
}






// 회원가입 직렬화버퍼 만들기. 헤더까지 다 만들어줌.
bool Create_Payload_Account(CProtocolBuff* Message, WORD MessageType)
{
	// 1. ID 생성
	// 문자열 랜덤으로 생성
	int iStr = (rand() % 120);

	char cStr[20];
	memcpy_s(cStr, 19, &RandomStr[iStr], 19);
	cStr[19] = '\0';
	

	// 2. 비밀번호 생성
	// 비밀번호 랜덤으로 생성
	int iPass = (rand() % 20);

	char cPass[20];
	memcpy_s(cPass, 19, &RandomStr[iPass], 19);
	cPass[19] = '\0';
		

	// 3. 헤더 생성
	WORD payloadSize = sizeof(st_DBQUERY_MSG_NEW_ACCOUNT);

	*Message << MessageType;
	*Message << payloadSize;
	

	// 4. 페이로드들 넣기.
	// ID 와 Password
	LONG64 AccountNo = InterlockedIncrement64(&g_accountTbl_account);

	*Message << AccountNo;

	/*memcpy_s(Message->GetBufferPtr() + Message->GetUseSize(), Message->GetFreeSize(), cStr, 20);
	Message->MoveWritePos(20);

	memcpy_s(Message->GetBufferPtr() + Message->GetUseSize(), Message->GetFreeSize(), cPass, 20);
	Message->MoveWritePos(20);*/

	for (int i = 0; i < 20; ++i)
		*Message << cStr[i];

	for (int i = 0; i < 20; ++i)
		*Message << cPass[i];


	return true;
}

// 스테이지 클리어 직렬화버퍼 만들기. 헤더까지 다 만들어줌.
bool Create_Payload_StageClear(CProtocolBuff* Message, WORD MessageType)
{
	// 1. 헤더 생성
	WORD payloadSize = sizeof(st_DBQUERY_MSG_STAGE_CLEAR);

	*Message << MessageType;
	*Message << payloadSize;

	// 2. 페이로드들 넣기
	LONG64 AccountNo = InterlockedIncrement64(&g_StageClearTbl_account);

	*Message << AccountNo;
	*Message << g_StageID;

	return true;
}

// Player 직렬화버퍼 만들기. 헤더까지 다 만들어줌.
bool Create_Payload_Player(CProtocolBuff* Message, WORD MessageType)
{
	// 1. 헤더 생성
	WORD payloadSize = sizeof(st_DBQUERY_MSG_PLAYER);

	*Message << MessageType;
	*Message << payloadSize;

	// 2. 페이로드들 넣기
	LONG64 AccountNo = InterlockedIncrement64(&g_PlayerTbl_account);

	*Message << AccountNo;
	*Message << g_Level;
	*Message << g_Exp;

	return true;
}