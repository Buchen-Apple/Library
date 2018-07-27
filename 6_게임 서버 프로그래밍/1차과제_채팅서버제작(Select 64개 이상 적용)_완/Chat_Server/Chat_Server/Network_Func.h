#ifndef __NETWORK_FUNC_H__
#define __NETWORK_FUNC_H__

// �׽�Ʈ������ ���� ����� 3���� ��.
#define FD_SETSIZE      3

#pragma comment(lib,"ws2_32")
#include <Ws2tcpip.h>
#include "Protocol.h"
#include "ProtocolBuff\ProtocolBuff.h"

using namespace Library_Jingyu;

// ����ü ���漱��
struct stClinet;
struct stRoom;

// ���ڷ� ���� UserID�� �������� Ŭ���̾�Ʈ ��Ͽ��� ������ ��󳽴�.(�˻�)
// ���� ��, �ش� ������ ���� ����ü�� �ּҸ� ���� 
// ���� �� nullptr ����
stClinet* ClientSearch(DWORD UserID);

// ���ڷ� ���� RoomID�� �������� �� ��Ͽ��� ���� ��󳽴�.(�˻�)
// ���� ��, �ش� ���� ���� ����ü�� �ּҸ� ���� 
// ���� �� nullptr ����
stRoom* RoomSearch(DWORD RoomID);

// ��Ʈ��ũ ���μ���
bool Network_Process(SOCKET* listen_sock);

// Accept ó��.
void Accept(SOCKET* client_sock, SOCKADDR_IN clinetaddr);

// Disconnect ó��
void Disconnect(DWORD UserID);

// ������ ������ ��� ������ SendBuff�� ��Ŷ �ֱ�
void BroadCast_All(CProtocolBuff* header, CProtocolBuff* Packet);
//void BroadCast_All(char* header, char* Packet);

// ���� �濡 �ִ� ��� ������, SendBuff�� ��Ŷ �ֱ�
// UserID�� -1�� ������ �� ���� ��� ���� ���.
// UserID�� �ٸ� ���� ������, �ش� ID�� ������ ����.
void BroadCast_Room(CProtocolBuff* header, CProtocolBuff* Packet, DWORD RoomID, int UserID);
//void BroadCast_Room(char* header, char* Packet, DWORD RoomID, int UserID);


/////////////////////////
// Recv ó�� �Լ���
/////////////////////////
// Recv() ó��
bool RecvProc(DWORD UserID);

// ��Ŷ ó��
bool PacketProc(WORD PacketType, DWORD UserID, char* Packet);

// "�α��� ��û" ��Ŷ ó��
bool Network_Req_Login(DWORD UserID, char* Packet);

// "���� ��û" ��Ŷ ó��
bool Network_Req_RoomList(DWORD UserID, char* Packet);

// "��ȭ�� ���� ��û" ��Ŷ ó��
bool Network_Req_RoomCreate(DWORD UserID, char* Packet);

// "������ ��û" ��Ŷ ó��
bool Network_Req_RoomJoin(DWORD UserID, char* Packet);

// "ä�� ��û" ��Ŷ ó��
bool Network_Req_Chat(DWORD UserID, char* Packet);

// "�� ���� ��û" ��Ŷ ó��
bool Network_Req_RoomLeave(DWORD UserID, char* Packet);

// "�� ���� ���"�� ����� ������ 
// �濡 ����� �ִٰ�, 0���� �Ǵ� ���� �߻��Ѵ�.
bool Network_Req_RoomDelete(DWORD RoomID);

// "Ÿ ����� ����" ��Ŷ�� ����� ������.
// � ������ �濡 ���� ��, �ش� �濡 �ִ� �ٸ� �������� �ٸ� ����ڰ� �����ߴٰ� �˷��ִ� ��Ŷ (��->Ŭ �� ����)
bool Network_Req_UserRoomJoin(DWORD JoinUserID);




/////////////////////////
// ��Ŷ ���� �Լ�
/////////////////////////
// "�α��� ��û ���" ��Ŷ ����
void CreatePacket_Res_Login(char* header, char* Packet, int UserID, char result);

// "�� ��� ��û ���" ��Ŷ ����
void CreatePacket_Res_RoomList(char* header, char* Packet);

// "��ȭ�� ���� ��û ���" ��Ŷ ����
void CreatePacket_Res_RoomCreate(char* header, char* Packet, char result, WORD RoomSize, char* cRoomName);

// "������ ��û ���" ��Ŷ ����
void CreatePacket_Res_RoomJoin(char* header, char* Packet, char result, DWORD RoomID);

// "ä�� ��û ���" ��Ŷ ����
void CreatePacket_Res_RoomJoin(char* header, char* Packet, WORD MessageSize, char* cMessage, DWORD UserID);

// "�� ���� ��û ���" ��Ŷ ����
void CreatePacket_Res_RoomLeave(char* header, char* Packet, DWORD UserID, DWORD RoomID);

// "�� ���� ���" ��Ŷ ����
void CreatePacket_Res_RoomDelete(char* header, char* Packet, DWORD RoomID);

// "Ÿ ����� ����" ��Ŷ ����
void CreatePacket_Res_UserRoomJoin(char* header, char* Packet, char* cNickName, DWORD UserID);






/////////////////////////
// Send ó��
/////////////////////////
// Send���ۿ� ������ �ֱ�
bool SendPacket(DWORD UserID, CProtocolBuff* headerBuff, CProtocolBuff* payloadBuff);

// Send������ �����͸� Send�ϱ�
bool SendProc(DWORD UserID);


#endif // !__NETWORK_FUNC_H__
