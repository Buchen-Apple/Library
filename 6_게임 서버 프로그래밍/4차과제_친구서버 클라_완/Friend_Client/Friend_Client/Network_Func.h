#ifndef __NETWORK_FUNC_H__
#define __NETWORK_FUNC_H__

#pragma comment(lib,"ws2_32")
#include <Ws2tcpip.h>
#include "Protocol.h"
#include "ProtocolBuff\ProtocolBuff.h"
#include "RingBuff\RingBuff.h"

using namespace Library_Jingyu;

// ���� �ʱ�ȭ, ���� ����, Ŀ��Ʈ �� �Լ�
bool Network_Init();

// �޴��� ���� �׼� ó��
bool PacketProc(int SelectNum);

// ��Ŷ ��� ����
void CreateHeader(CProtocolBuff* headerBuff, WORD MsgType, WORD PayloadSize);

// �� �α������� ����ϴ� �Լ�
void LoginShow();




//////////////////////////
// ��Ŷ ���� �Լ���
/////////////////////////
// "ȸ�� �߰�" ��Ŷ ����
bool Network_Res_AccountAdd();

// "�α���" ��Ŷ ����
bool Network_Res_Login();

// "ȸ����� ��û" ��Ŷ ����
bool Network_Res_AccountList();

// "ģ����� ��û" ��Ŷ ����
bool Network_Res_FriendList();

// "���� ģ����û ����" ��Ŷ ����
bool Network_Res_ReplyList();

// "���� ģ����û ����" ��Ŷ ����
bool Network_Res_RequestList();

// "ģ����û ������" ��Ŷ ����
bool Network_Res_FriendRequest();

// "ģ����û ���" ��Ŷ ����
bool Network_Res_FriendCancel();

// "������û ����" ��Ŷ ����
bool Network_Res_FriendAgree();

// "������û ����" ��Ŷ ����
bool Network_Res_FriendDeny();

// "ģ�� ����" ��Ŷ ����
bool Network_Res_FriendRemove();







//////////////////////////
// ���� ��Ŷ ó�� �Լ���
/////////////////////////
// "ȸ�� �߰� ���" ��Ŷ ó��
void Network_Req_AccountAdd(CProtocolBuff* payload);

// "�α��� ���" ��Ŷ ó��
void Network_Req_Login(CProtocolBuff* payload);

// "ȸ����� ��û ���" ��Ŷ ó��
void Network_Req_AccountList(CProtocolBuff* payload);

// "ģ����� ��û ���" ��Ŷ ó��
void Network_Req_FriendList(CProtocolBuff* payload);

// "���� ģ����û ���� ���" ��Ŷ ó��
void Network_Req_ReplyList(CProtocolBuff* payload);

// "���� ģ����û ���� ���" ��Ŷ ó��
void Network_Req_RequestList(CProtocolBuff* payload);

// "ģ����û ������ ���" ��Ŷ ó��
void Network_Req_FriendRequest(CProtocolBuff* payload);

// "ģ����û ��� ���" ��Ŷ ó��
void Network_Req_FriendCancel(CProtocolBuff* payload);

// "������û ���� ���" ��Ŷ ó��
void Network_Req_FriendAgree(CProtocolBuff* payload);

// "������û ���� ���" ��Ŷ ó��
void Network_Req_FriendDeny(CProtocolBuff* payload);

// "ģ������ ���" ��Ŷ ó��
void Network_Req_FriendRemove(CProtocolBuff* payload);









/////////////////////////
// recv ó��
/////////////////////////
// ���ú� ó��
bool RecvProc(CProtocolBuff* RecvBuff, WORD MsgType);





/////////////////////////
// Send ó��
/////////////////////////
// Send���ۿ� ������ �ֱ�
bool SendPacket(CProtocolBuff* headerBuff, CProtocolBuff* payloadBuff);

// Send������ �����͸� Send�ϱ�
bool SendProc();



#endif // !__NETWORK_FUNC_H__
