#ifndef __NETWORK_FUNC_H__
#define __NETWORK_FUNC_H__

#pragma comment(lib,"ws2_32")
#include <Ws2tcpip.h>
#include <map>

#include "protocol.h"
#include "RingBuff\RingBuff.h"
#include "ProtocolBuff\ProtocolBuff.h"

using namespace Library_Jingyu;
using namespace std;


// �α� ����
#define dfLOG_LEVEL_DEBUG		0
#define dfLOG_LEVEL_WARNING		1
#define dfLOG_LEVEL_ERROR		2

// �α� ��ũ�� ����
#define _LOG(LogLevel, fmt, ...)						\
do{														\
	if(g_LogLevel <= LogLevel)							\
	{													\
		swprintf_s(g_szLogBuff, fmt, ## __VA_ARGS__);	\
		Log(g_szLogBuff, LogLevel);						\
	}													\
} while (0)												\


// ���� ����ü
struct stSession
{
	SOCKET m_Client_sock;	

	TCHAR m_IP[30];
	USHORT m_prot;

	OVERLAPPED m_RecvOverlapped;
	OVERLAPPED m_SendOverlapped;

	CRingBuff m_RecvBuff;
	CRingBuff m_SendBuff;

	LONG	m_IOCount = 0;

	// Send���� �������� üũ. 1�̸� Send��, 0�̸� Send�� �ƴ�
	LONG	m_SendFlag = 0;	
};


extern SRWLOCK g_Session_map_srwl;

// ���� ���� map.
// Key : SOCKET, Value : ���Ǳ���ü
extern map<SOCKET, stSession*> map_Session;

#define	LockSession()	AcquireSRWLockExclusive(&g_Session_map_srwl)
#define UnlockSession()	ReleaseSRWLockExclusive(&g_Session_map_srwl)


// ���� ����
void Disconnect(stSession* DeleteSession);

// �α� ��� �Լ�
void Log(TCHAR *szString, int LogLevel);




// ------------
// ���ú� ���� �Լ���
// ------------
// RecvProc �Լ�
bool RecvProc(stSession* NowSession);

// RecvPost �Լ�
bool RecvPost(stSession* NowSession);

// Accept�� RecvPost�Լ�
bool RecvPost_Accept(stSession* NowSession);

// ���� ��Ŷ ó�� �б⹮ �Լ�
bool PacketProc(stSession* NowSession, CProtocolBuff* Payload);



// ------------
// ��Ŷ ó�� �Լ���
// ------------
bool Recv_Packet_Echo(stSession* NowSession, CProtocolBuff* Payload);



// ------------
// ������ ��Ŷ ����� �Լ���
// ------------
// ������Ŷ �����
void Send_Packet_Echo(CProtocolBuff* Buff, WORD PayloadSize, char* RetrunText, int RetrunTextSize);




// ------------
// ���� ���� �Լ���
// ------------
// ���� ���ۿ� ������ �ֱ�
bool SendPacket(stSession* NowSession, CProtocolBuff* Buff);

// ���� ������ ������ WSASend() �ϱ�
bool SendPost(stSession* NowSession);





#endif // !__NETWORK_FUNC_H__

