#ifndef __NETWORK_FUNC_H__
#define __NETWORK_FUNC_H__

#pragma comment(lib,"ws2_32")
#include <Ws2tcpip.h>
#include <map>

#include "protocol.h"
#include "RingBuff\RingBuff.h"
#include "ProtocolBuff\ProtocolBuff.h"

using namespace Library_Jingyu;



// ------------
// ���ú� ���� �Լ���
// ------------
// RecvPost�Լ�. ���� Recv ȣ��
int RecvPost();

// RecvProc �Լ�. ���ú� ť�� ������ �м�
bool RecvProc();

// ��Ŷ ó��. RecvProc()���� ���� ��Ŷ ó��.
bool PacketProc(BYTE PacketType, CProtocolBuff* Payload);




// ------------
// ��Ŷ �����
// ------------
// ��� ���� �Լ�
void CreateHeader(CProtocolBuff* header, BYTE PayloadSize, BYTE PacketType);

// ���� ��Ŷ �����
bool Network_Send_Echo(CProtocolBuff *header, CProtocolBuff* payload);

// ------------
// ��Ŷ ó�� �Լ���
// ------------
// ���� ��Ŷ ó�� �Լ�
bool Recv_Packet_Echo(CProtocolBuff* Payload);



// ------------
// ���� ���� �Լ���
// ------------
// ���� ���ۿ� ������ �ֱ�
bool SendPacket(CProtocolBuff* header, CProtocolBuff* payload);

// ���� ������ ������ Send() �ϱ�
int SendPost();




#endif // !__NETWORK_FUNC_H__

