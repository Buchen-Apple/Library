#ifndef __NETWORK_FUNC_H__
#define __NETWORK_FUNC_H__

#pragma comment(lib,"ws2_32")
#include <Ws2tcpip.h>

#include "Protocol.h"
#include "ProtocolBuff\ProtocolBuff.h"
#include "RingBuff\RingBuff.h"

using namespace Library_Jingyu;

// ����ü ���漱��
struct stDummyClient;

// ��Ʈ��ũ ���� �Լ� (�����ʱ�ȭ, Ŀ��Ʈ ��..)
bool Network_Init(int* TryCount, int* FailCount, int* SuccessCount);

// ��Ʈ��ũ ���μ���
bool Network_Process();

// ���ڷ� ���� Socket ���� �������� [���� ���]���� [���̸� ��󳽴�].(�˻�)
// ���� ��, �ش� ������ ���� ����ü�� �ּҸ� ����
// ���� �� nullptr ����
stDummyClient* DummySearch(SOCKET sock);

// Disconnect ó��
void Disconnect(SOCKET sock);



///////////////////////////////
// ȭ�� ��¿� �� ��ȯ �Լ�
///////////////////////////////
// �����Ͻ� ��� ��ȯ �Լ�
DWORD GetAvgLaytency();


// TPS ��ȯ �Լ�
int GetTPS();


///////////////////////////////
// ������ �� ��.
///////////////////////////////
// ���� ���ڿ� ���� �� SendBuff�� �ִ´�.
void DummyWork_CreateString();



/////////////////////////
// Recv ó�� �Լ���
/////////////////////////
// Recv() ó��
bool RecvProc(stDummyClient* NowDummy);

// "���� ��Ŷ" ó��
bool Network_Res_Acho(WORD Type, stDummyClient* NowDummy, CProtocolBuff* payload);



/////////////////////////
// Send ó��
/////////////////////////
// Send���ۿ� ������ �ֱ�
bool SendPacket(stDummyClient* NowDummy, CProtocolBuff* headerBuff, CProtocolBuff* payloadBuff);

// Send������ �����͸� Send�ϱ�
bool SendProc(CRingBuff* SendBuff, SOCKET sock);


#endif // !__NETWORK_FUNC_H__
