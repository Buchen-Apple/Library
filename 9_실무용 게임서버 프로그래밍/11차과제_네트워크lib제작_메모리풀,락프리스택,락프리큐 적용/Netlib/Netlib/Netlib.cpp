// Netlib.cpp : 이 파일에는 'main' 함수가 포함됩니다. 거기서 프로그램 실행이 시작되고 종료됩니다.
//

#include "pch.h"
#include "NetworkLib\NetworkLib.h"

#include <conio.h>

#include <mmsystem.h>
#pragma comment(lib,"winmm.lib")

#define PORT	6000

using namespace Library_Jingyu;
using namespace std;

extern LONG g_lPacketAllocCount;


#define CREATE_WORKER_COUNT	4
#define ACTIVE_WORKER_COUNT	2
#define CREATE_ACCEPT_COUNT	1




class MyLanServer : public CLanServer
{
private:
	// ------------
	// 패킷 처리 함수들
	// ------------


public:
	// Accept 직후, 호출된다.
	// return false : 클라이언트 접속 거부
	// return true : 접속 허용
	virtual bool OnConnectionRequest(TCHAR* IP, USHORT port);

	// Accept 후 접속처리 완료 후 호출
	virtual void OnClientJoin(ULONGLONG ClinetID);

	// Disconnect 후 호출된다.
	virtual void OnClientLeave(ULONGLONG ClinetID);

	// 패킷 수신 완료 후 호출되는 함수.
	virtual void OnRecv(ULONGLONG ClinetID, CProtocolBuff* Payload);

	// 패킷 송신 완료 후 호출되는 함수
	virtual void OnSend(ULONGLONG ClinetID, DWORD SendSize);

	// 워커 스레드 GQCS 바로 하단에서 호출
	virtual void OnWorkerThreadBegin();

	// 워커스레드 1루푸 종료 후 호출되는 함수.
	virtual void OnWorkerThreadEnd();

	// 에러 발생 시 호출되는 함수.
	virtual void OnError(int error, const TCHAR* errorStr);
};


// ------------
// 패킷 처리 함수들
// ------------
// Accept 직후, 호출된다.
//
// return false : 클라이언트 접속 거부
// return true : 접속 허용
bool MyLanServer::OnConnectionRequest(TCHAR* IP, USHORT port)
{
	//printf("OnConnectionRequest\n");
	return true;
}

// Accept 후 접속처리 완료 후 호출
void MyLanServer::OnClientJoin(ULONGLONG ClinetID)
{
	// 데이터 페이로드에 만들기
	CProtocolBuff* Payload = CProtocolBuff::Alloc();
	InterlockedIncrement(&g_lPacketAllocCount);
	__int64 i64_Data = 0x7fffffffffffffff;

	Payload->PutData((char*)&i64_Data, 8);

	// SendBuff에 넣기
	bool Check = SendPacket(ClinetID, Payload);
}

// Disconnect 후 호출된다.
void MyLanServer::OnClientLeave(ULONGLONG ClinetID)
{
	//printf("OnClientLeave : %lld\n", ClinetID);
}

// 패킷 수신 완료 후 호출되는 함수.
void MyLanServer::OnRecv(ULONGLONG ClinetID, CProtocolBuff* Payload)
{
	// 1. 페이로드 사이즈 얻기
	int Size = Payload->GetUseSize();

	// 2. 그 사이즈만큼 데이터 memcpy
	char* Text = new char[Size];
	Payload->GetData(Text, Size);

	// 3. 에코 패킷 만들기. Buff안에는 페이로드만 있다.
	CProtocolBuff* Buff = CProtocolBuff::Alloc();
	InterlockedIncrement(&g_lPacketAllocCount);
	Buff->PutData(Text, Size);

	delete[] Text;

	// 4. 에코 패킷을 SendBuff에 넣기.
	SendPacket(ClinetID, Buff);
}

// 패킷 송신 완료 후 호출되는 함수
void MyLanServer::OnSend(ULONGLONG ClinetID, DWORD SendSize)
{
	//printf("OnSend  --->   ");
	//printf("id : %lld, Byte : %d\n", ClinetID, SendSize);
}

// 워커 스레드 GQCS 바로 하단에서 호출
void MyLanServer::OnWorkerThreadBegin()
{
	//printf("OnWorkerThreadBegin\n");
}

// 워커스레드 1루프 종료 후 호출되는 함수.
void MyLanServer::OnWorkerThreadEnd()
{
	//printf("OnWorkerThreadEnd\n");
}

// 에러 발생 시 호출되는 함수.
void MyLanServer::OnError(int error, const TCHAR* errorStr)
{
	//LOG(L"OnError", CSystemLog::en_LogLevel::LEVEL_ERROR, L" [%s ---> %d, %d]", errorStr, error, WinGetLastError());
}





int _tmain()
{
	timeBeginPeriod(1);

	system("mode con: cols=120 lines=8");

	MyLanServer LanServer;

	TCHAR IP[30] = L"0,0,0,0";

	if (LanServer.Start(IP, PORT, CREATE_WORKER_COUNT, ACTIVE_WORKER_COUNT, CREATE_ACCEPT_COUNT, false, 200) == false)
	{
		// 윈도우 에러와 내 에러를 다 얻어본다.
		int WinError = LanServer.WinGetLastError();
		int MyError = LanServer.NetLibGetLastError();

		return 0;
	}

	while (1)
	{
		_tprintf_s(L"================================================================\n"
					"WorkerThread : %d, ActiveWorkerThread : %d, AcceptThread : %d\n\n" 
					"JoinCount : %lld, PacketCount : [%d]\n"
					"================================================================\n\n",
					CREATE_WORKER_COUNT, ACTIVE_WORKER_COUNT, CREATE_ACCEPT_COUNT, 
					LanServer.GetClientCount(), g_lPacketAllocCount);
		// 서버 종료 체크
		/*if (_kbhit())
		{
			int Key = _getch();

			if (Key == 'q' || Key == 'Q')
			{

				_tprintf(L"Q Key. Server Exit\n");
				LanServer.Stop();

				break;
			}
		}*/

		Sleep(1000);
	}

	timeEndPeriod(1);


	return 0;
}