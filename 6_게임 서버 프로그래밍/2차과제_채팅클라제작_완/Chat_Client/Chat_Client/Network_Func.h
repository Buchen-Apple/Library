#ifndef __NETWORK_FUNC_H__
#define __NETWORK_FUNC_H__

#include "Protocol.h"
#include "ProtocolBuff\ProtocolBuff.h"

using namespace Library_Jingyu;

#define WM_SOCK			WM_USER+1
#define ROOMNAME_SIZE	30
#define CHAT_SIZE		30

// ���� ���� ���޹ޱ�. (�κ� ���̾�α� �ڵ�(����), �ν��Ͻ� ��..)
void InfoGet(HWND g_LobbyhWnd, HWND* g_RoomhWnd, HINSTANCE hInst, int cmdShow);

// ���� �ʱ�ȭ, ���� ����, Ŀ��Ʈ �� �Լ�
BOOL NetworkInit(TCHAR tIP[30], TCHAR tNickName[dfNICK_MAX_LEN]);

// ���� ����, ���� ���� �� �Լ�
void NetworkClose();

// ��Ʈ��ũ ó�� �Լ�
BOOL NetworkProc(LPARAM);

// ��� �����ϱ�
void CreateHeader(CProtocolBuff* header, CProtocolBuff* payload, WORD Type);

// �� ä���� ���� �� ������. �׸��� �� �����Ϳ� ����ϱ���� �ϴ� �Լ�
void ChatLogic();



/////////////////////////
// ��Ŷ ���� �Լ�
/////////////////////////
// "�α��� ��û" ��Ŷ ����
void CreatePacket_Req_Login(char* header, char* Packet);

// "��ȭ�� ��� ��û" ��Ŷ ����
void CreatePacket_Req_RoomList(char* header);

// "�� ���� ��û" ��Ŷ ����
void CreatePacket_Req_RoomCreate(char* header, char* Packet, TCHAR RoomName[ROOMNAME_SIZE]);

// "�� ���� ��û" ��Ŷ ����
void CreatePacket_Req_RoomJoin(char* header, char* Packet, DWORD RoomNo);

// "ä�� �۽�" ��Ŷ ����
void CreatePacket_Req_ChatSend(char* header, char* Packet, DWORD MessageSize, TCHAR Chat[CHAT_SIZE]);

// "�� ���� ��û" ��Ŷ ����
void CreatePacket_Req_RoomLeave(char* header);



/////////////////////////
// Send ó��
/////////////////////////
// �� SendBuff�� ������ �ֱ�
BOOL SendPacket(CProtocolBuff* headerBuff, CProtocolBuff* payloadBuff);

// SendBuff�� �����͸� Send�ϱ�
BOOL SendProc();


/////////////////////////
// Recv ó�� �Լ���
/////////////////////////
//  Recv() ó�� �Լ�
BOOL RecvProc();

// ��Ŷ Ÿ�Կ� ���� ��Ŷó�� �Լ�
BOOL PacketProc(WORD PacketType, char* Packet);

// "�α��� ��û ���" ��Ŷ ó��
BOOL Network_Res_Login(char* Packet);

// "��ȭ�� ��� ��û ���" ��Ŷ ó��
BOOL Network_Res_RoomList(char* Packet);

// "�� ���� ��û ���" ��Ŷ ó�� (���� �Ⱥ����͵� ��)
BOOL Network_Res_RoomCreate(char* Packet);

// "�� ���� ��û ���" ��Ŷ ó��
BOOL Network_Res_RoomJoin(char* Packet);

// "ä�� ���" ��Ŷ ó�� (���� ������ �ȿ�. ���� ���� �濡 �ִ� �ٸ������ ä�� ó���ϱ�)
BOOL Network_Res_ChatRecv(char* Packet);

// "Ÿ ����� ���� ���" ��Ŷ ó�� (���� �Ⱥ����͵� ��)
BOOL Network_Res_UserEnter(char* Packet);

// "�� ���� ��û ���" ��Ŷ ó�� (���� �Ⱥ����͵� ��)
BOOL Network_Res_RoomLeave(char* Packet);

// "�� ���� ���" ��Ŷ ó��
BOOL Network_Res_RoomDelete(char* Packet);






#endif // !__NETWORK_FUNC_H__
