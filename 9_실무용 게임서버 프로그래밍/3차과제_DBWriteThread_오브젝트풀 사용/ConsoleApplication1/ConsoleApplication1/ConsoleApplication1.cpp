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
#include "ObjectPool\Object_Pool.h"
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
#define OBJECT_POOL_SIZE	1000

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
bool Create_Payload_Account(st_DBQUERY_MSG_NEW_ACCOUNT* Message, WORD MessageType);

// 스테이지 클리어 직렬화버퍼 만들기. 헤더까지 다 만들어줌.
bool Create_Payload_StageClear(st_DBQUERY_MSG_STAGE_CLEAR* Message, WORD MessageType);

// Player 직렬화버퍼 만들기. 헤더까지 다 만들어줌.
bool Create_Payload_Player(st_DBQUERY_MSG_PLAYER* Message, WORD MessageType);

// 오브젝트 풀의 Free()함수를 호출하는 함수
bool ObjectPool_Free_Func(st_DBQUERY_MSG_NEW_ACCOUNT* addr);


// ----------------------
// 전역변수 선언
// ----------------------
// DBWriteThread;
CRingBuff g_DBWrite(QUEUE_SIZE);

// 업데이트 스레드 종료용 이벤트
HANDLE g_ExitEvent;

// DB 스레드 종료용 이벤트
HANDLE g_DBExitEvent;

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

// 구조체 메모리풀. 
// 사용할 구조체 중에 가장 큰놈으로 잡는다. 그리고 필요할 때마다 해당 타입으로 캐스팅해서 사용
// 오브젝트 할당받을 때 마다 생성자/소멸자 호출할 필요가 없으니, 플레이스먼트 뉴 사용 안함.
CMemoryPool<st_DBQUERY_MSG_NEW_ACCOUNT> MPool(OBJECT_POOL_SIZE, false);



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

	HANDLE* hUpdateThread = new HANDLE[UpdateThreadCount];
	HANDLE hDBWriteThread;


	// 2. DB 이벤트, 종료 이벤트 생성하기
	g_DBWriteThreadEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	g_ExitEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	g_DBExitEvent = CreateEvent(NULL, TRUE, FALSE, NULL);



	// 3. 스레드 생성하기
	// 업데이트 스레드생성
	int i = 0;
	for (; i < UpdateThreadCount; ++i)
	{
		hUpdateThread[i] = (HANDLE)_beginthreadex(NULL, 0, UpdateThread, 0, 0, NULL);
		if (hUpdateThread[i] == NULL)
		{
			fputs("업데이트 스레드 생성 실패...\n", stdout);
			return 1;
		}
	}	

	// DB 스레드 생성
	hDBWriteThread = (HANDLE)_beginthreadex(NULL, 0, DBWriteThread, 0, 0, NULL);
	if (hDBWriteThread == NULL)
	{
		fputs("DB 스레드 생성 실패...\n", stdout);
		return 1;
	}


	// 4. 1초마다 출력할거 하면서, q눌렀나 확인.
	while (1)
	{
		// DB 저장 TPS, DB 큐 사이즈 출력	
		LONG64 TPS = InterlockedExchange64(&g_TPS, 0);
		_tprintf(L"TPS : %lld, Queue 사이즈 : %d(%d), ObjectPool 카운트 : %d(%d)\n", TPS, g_DBWrite.GetUseSize(), QUEUE_SIZE, MPool.GetUseCount(), OBJECT_POOL_SIZE);

		if (_kbhit())
		{
			int Key = _getch();

			if (Key == 'q' || Key == 'Q')
			{
				fputs("Q키를 눌렀음. 프로그램 종료 시작.\n", stdout);

				// 모든 '업데이트' 스레드들한테 종료 신호 줌.
				SetEvent(g_ExitEvent);

				break;
			}

		}	

		Sleep(1000);
	}


	// 5. 업데이트 스레드 종료 대기 후 종료
	WaitForMultipleObjects(UpdateThreadCount, hUpdateThread, true, INFINITE);

	for (int i=0; i < UpdateThreadCount; ++i)
		CloseHandle(hUpdateThread[i]);

	delete[] hUpdateThread;

	// 6. 다 종료됐으면, DB 스레드 종료해야하는지 확인
	while (1)
	{
		// 큐 사이즈가 0이면, DB 스레드에게 종료 신호 준다.
		if (g_DBWrite.GetUseSize() == 0)
		{
			SetEvent(g_DBExitEvent);
			break;
		}
		Sleep(1);
	}
	
	// 7. DB 스레드 종료 대기
	WaitForSingleObject(hDBWriteThread, INFINITE);

	// 8. 종료됐으면 DB 스레드 핸들 제거
	CloseHandle(hDBWriteThread);

	// 9. 이벤트 제거
	CloseHandle(g_DBWriteThreadEvent);
	CloseHandle(g_ExitEvent);
	CloseHandle(g_DBExitEvent);
		

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
		// 큐 사이즈가 1000이상일때만 Sleep 대기간다.
		if (g_DBWrite.GetUseSize() > 1000)
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
			// 오브젝트풀 하나 얻어오기.
			// st_DBQUERY_MSG_NEW_ACCOUNT타입으로 캐스팅한다.
			MPool.EnterLOCK();
			st_DBQUERY_MSG_NEW_ACCOUNT* Message = (st_DBQUERY_MSG_NEW_ACCOUNT*)MPool.Alloc();
			MPool.LeaveLOCK();
			
			// 메시지 만들기
			Create_Payload_Account(Message, df_DBQUERY_MSG_NEW_ACCOUNT);

			// 할당받은 오브젝트 풀의 '주소(8바이트)'를 큐에다 인큐하기.
			void* addr = Message;
			g_DBWrite.EnterLOCK();
			g_DBWrite.Enqueue((char*)&addr, sizeof(void*));
			g_DBWrite.LeaveLOCK();	

			// DBEvent를 시그널 상태로 변경
			SetEvent(g_DBWriteThreadEvent);

		}
			break;


			// 스테이지 클리어
		case df_DBQUERY_MSG_STAGE_CLEAR:
		{
			// 오브젝트풀 하나 얻어오기.
			// st_DBQUERY_MSG_STAGE_CLEAR타입으로 캐스팅한다.
			MPool.EnterLOCK();
			st_DBQUERY_MSG_STAGE_CLEAR* Message = (st_DBQUERY_MSG_STAGE_CLEAR*)MPool.Alloc();
			MPool.LeaveLOCK();

			// 메시지 만들기
			Create_Payload_StageClear(Message, df_DBQUERY_MSG_STAGE_CLEAR);

			// 할당받은 오브젝트 풀의 '주소(8바이트)'를 큐에다 인큐하기.
			void* addr = Message;
			g_DBWrite.EnterLOCK();
			g_DBWrite.Enqueue((char*)&addr, sizeof(void*));
			g_DBWrite.LeaveLOCK();

			// DBEvent를 시그널 상태로 변경
			SetEvent(g_DBWriteThreadEvent);

		}
			break;


			// Player
		case df_DBQUERY_MSG_PLAYER:
		{

			// 오브젝트풀 하나 얻어오기.
			// st_DBQUERY_MSG_PLAYER타입으로 캐스팅한다.
			MPool.EnterLOCK();
			st_DBQUERY_MSG_PLAYER* Message = (st_DBQUERY_MSG_PLAYER*)MPool.Alloc();
			MPool.LeaveLOCK();
			
			// 메시지 만들기
			Create_Payload_Player(Message, df_DBQUERY_MSG_PLAYER);

			// 할당받은 오브젝트 풀의 '주소(8바이트)'를 큐에다 인큐하기.
			void* addr = Message;
			g_DBWrite.EnterLOCK();
			g_DBWrite.Enqueue((char*)&addr, sizeof(void*));
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

	HANDLE hEvent[2] = { g_DBExitEvent, g_DBWriteThreadEvent };
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
		// 이 안에서는, 큐에 뭐가 없을때까지 한다. 종료신호로 깨어났어도 일단 일을 하러 가본다.
		if (DBWorking(&conn, connection) == false)
			break;

		// 깨어난 이유가, 종료신호라면 바로 종료.
		// 이미 Main스레드에서 큐 사이즈 확인하면서 종료신호 줌.
		if (dwError == WAIT_OBJECT_0)
			break;
		

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


		// 일단 8바이트만큼 Dequeue 한다.
		void* addr;
		int Size = sizeof(void*);
		if (g_DBWrite.Dequeue((char*)&addr, Size) != Size)
		{
			_tprintf(L"디큐 실패\n");
			return false;
		}

		st_DBQUERY_HEADER* Header = (st_DBQUERY_HEADER*)addr;

		// 2. Type에 따라서 쿼리 만들어서 날린다.
		switch (Header->Type)
		{
			// 회원 가입
		case df_DBQUERY_MSG_NEW_ACCOUNT:
		{
			st_DBQUERY_MSG_NEW_ACCOUNT* Payload = (st_DBQUERY_MSG_NEW_ACCOUNT*)addr;
			
			// 2. 타입에 맞게 쿼리문 만들기
			char query[200];
			sprintf_s(query, 200, "INSERT INTO `accountTBL`(`accountNo`, `id`,`password`) VALUES(%lld, '%s', '%s')", Payload->iAccountNo, Payload->szID, Payload->szPassword);

			// 3. 쿼리문 날리기
			if (DBQuery(query, conn, connection) == false)
			{
				// 오브젝트풀 free
				ObjectPool_Free_Func((st_DBQUERY_MSG_NEW_ACCOUNT*)addr);
				return false;
			}
		}
		break;

		// 스테이지 클리어
		case df_DBQUERY_MSG_STAGE_CLEAR:
		{
			st_DBQUERY_MSG_STAGE_CLEAR* Payload = (st_DBQUERY_MSG_STAGE_CLEAR*)addr;

			// 2. 타입에 맞게 쿼리문 만들기
			char query[200];
			sprintf_s(query, 200, "INSERT INTO `stageclearTBL`(`accountNo`,`stageID`) VALUES(%lld, %d)", Payload->iAccountNo, Payload->iStageID);

			// 3. 쿼리문 날리기
			if (DBQuery(query, conn, connection) == false)
			{
				// 오브젝트풀 free
				ObjectPool_Free_Func((st_DBQUERY_MSG_NEW_ACCOUNT*)addr);
				return false;
			}
		}
		break;

		// Player
		case df_DBQUERY_MSG_PLAYER:
		{
			st_DBQUERY_MSG_PLAYER* Payload = (st_DBQUERY_MSG_PLAYER*)addr;

			// 2. 타입에 맞게 쿼리문 만들기
			char query[200];
			sprintf_s(query, 200, "INSERT INTO `playerTBL`(`accountNo`,`Level`, `Exp`) VALUES(%lld, %d, %lld)", Payload->iAccountNo, Payload->iLevel, Payload->iExp);

			// 3. 쿼리문 날리기
			if (DBQuery(query, conn, connection) == false)
			{
				// 오브젝트풀 free
				ObjectPool_Free_Func((st_DBQUERY_MSG_NEW_ACCOUNT*)addr);
				return false;
			}
		}
		break;

		default:			
		{
			_tprintf(L"DBWorking() 존재하지 않는 타입(%d)\n", Header->Type);
			// 오브젝트풀 free
			ObjectPool_Free_Func((st_DBQUERY_MSG_NEW_ACCOUNT*)addr);
			return false;
		}
		}

		// 쿼리 보내기 성공했어도, 오브젝트풀 free
		ObjectPool_Free_Func((st_DBQUERY_MSG_NEW_ACCOUNT*)addr);

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
bool Create_Payload_Account(st_DBQUERY_MSG_NEW_ACCOUNT* Message, WORD MessageType)
{
	// 1. ID 생성
	// 문자열 랜덤으로 생성 후 구조체에 넣기
	int iStr = (rand() % 120);

	memcpy_s(Message->szID, 20, &RandomStr[iStr], 20);
	Message->szID[19] = '\0';


	// 2. 비밀번호 생성
	// 비밀번호 랜덤으로 생성 후 구조체에 넣기
	int iPass = (rand() % 20);

	memcpy_s(Message->szPassword, 20, &RandomStr[iPass], 20);
	Message->szPassword[19] = '\0';

	// 3. Account 생성
	// 생성 후 구조체에 넣기
	Message->iAccountNo = InterlockedIncrement64(&g_accountTbl_account);	
	
	// 4. 헤더에 타입 넣기 생성
	Message->Type = MessageType;

	return true;
}

// 스테이지 클리어 직렬화버퍼 만들기. 헤더까지 다 만들어줌.
bool Create_Payload_StageClear(st_DBQUERY_MSG_STAGE_CLEAR* Message, WORD MessageType)
{
	// 1. 페이로드 넣기
	Message->iAccountNo = InterlockedIncrement64(&g_StageClearTbl_account);
	Message->iStageID = g_StageID;

	// 2. 타입 넣기
	Message->Type = MessageType;

	return true;
}

// Player 직렬화버퍼 만들기. 헤더까지 다 만들어줌.
bool Create_Payload_Player(st_DBQUERY_MSG_PLAYER* Message, WORD MessageType)
{
	// 1. 페이로드 넣기
	Message->iAccountNo = InterlockedIncrement64(&g_PlayerTbl_account);
	Message->iLevel = g_Level;
	Message->iExp = g_Exp;


	// 2. 타입 넣기
	Message->Type = MessageType;

	return true;
}




// 오브젝트 풀의 Free()함수를 호출하는 함수
bool ObjectPool_Free_Func(st_DBQUERY_MSG_NEW_ACCOUNT* addr)
{
	MPool.EnterLOCK();
	bool Check = MPool.Free(addr);
	MPool.LeaveLOCK();

	if (Check == false)
		return false;

	return true;
}