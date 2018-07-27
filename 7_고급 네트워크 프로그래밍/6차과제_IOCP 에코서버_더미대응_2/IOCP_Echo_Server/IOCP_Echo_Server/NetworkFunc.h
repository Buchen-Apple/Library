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

struct stSession
{
	SOCKET m_Client_sock;

	TCHAR m_IP[30];
	USHORT m_prot;

	OVERLAPPED m_RecvOverlapped;
	OVERLAPPED m_SendOverlapped;

	CRingBuff m_RecvBuff;
	CRingBuff m_SendBuff;

	LONG m_IOCount = 0;

	// Send���� �������� üũ. 1�̸� Send��, 0�̸� Send�� �ƴ�
	DWORD	m_SendFlag = 0;	
};

extern SRWLOCK g_Session_map_srwl;

// ���� ���� map.
// Key : SOCKET, Value : ���Ǳ���ü
extern map<SOCKET, stSession*> map_Session;

#define	LockSession()	AcquireSRWLockExclusive(&g_Session_map_srwl)
#define UnlockSession()	ReleaseSRWLockExclusive(&g_Session_map_srwl)



// ���� ����
void Disconnect(stSession* DeleteSession);



// ------------
// ���ú� ���� �Լ���
// ------------
// RecvProc �Լ�
void RecvProc(stSession* NowSession);

// RecvPost �Լ�
bool RecvPost(stSession* NowSession);

// ���� ��Ŷ ó�� �б⹮ �Լ�
bool PacketProc(stSession* NowSession, CProtocolBuff* Payload);



// ------------
// ��Ŷ ó�� �Լ���
// ------------
bool Recv_Packet_Echo(stSession* NowSession, CProtocolBuff* Payload);



// ------------
// ������ ��Ŷ ����� �Լ���
// ------------
// ��� �����
void Send_Packet_Header(WORD PayloadSize, CProtocolBuff* Header);

// ������Ŷ �����
void Send_Packet_Echo(CProtocolBuff* header, CProtocolBuff* payload, char* RetrunText, int RetrunTextSize);




// ------------
// ���� ���� �Լ���
// ------------
// ���� ���ۿ� ������ �ֱ�
bool SendPacket(stSession* NowSession, CProtocolBuff* header, CProtocolBuff* payload);

// ���� ������ ������ WSASend() �ϱ�
bool SendPost(stSession* NowSession);





#endif // !__NETWORK_FUNC_H__

