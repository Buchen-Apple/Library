#ifndef __NETWORK_FUNC_H
#define __NETWORK_FUNC_H

// �׽�Ʈ������ ���� ����� 3���� ��.
//#define FD_SETSIZE      3

#pragma comment(lib,"ws2_32")
#include <Ws2tcpip.h>
#include "Protocol.h"
#include "ProtocolBuff\ProtocolBuff.h"
#include "RingBuff\RingBuff.h"

using namespace Library_Jingyu;

// ����ü ���漱��
struct stAccount;
struct stAcceptUser;

// ���ڷ� ���� Socket ���� �������� [Accept ���]���� [������ ��󳽴�].(�˻�)
// ���� ��, �ش� ������ ���� ����ü�� �ּҸ� ����
// ���� �� nullptr ����
stAcceptUser* ClientSearch_AcceptList(SOCKET sock);

// ���ڷ� ���� ȸ��No ���� �������� [ȸ�� ���]���� [������ ��󳽴�].(�˻�)
// ���� ��, �ش� ������ ȸ�� ���� ����ü�� �ּҸ� ����
// ���� �� nullptr ����
stAccount* ClientSearch_AccountList(UINT64 AccountNo);

// ��Ʈ��ũ ���μ���
bool Network_Process(SOCKET* listen_sock);

// Accept ó��.
void Accept(SOCKET* client_sock, SOCKADDR_IN clinetaddr);

// Disconnect ó��
void Disconnect(SOCKET sock);

// ��Ŷ ��� ����
void CreateHeader(CProtocolBuff* headerBuff, WORD MsgType, WORD PayloadSize);




/////////////////////////////
// ������ �Լ�
////////////////////////////

// ���� Ŀ��Ʈ�� ���� �� �˾ƿ���
UINT64 AcceptCount();

// TPSüũ (���� SendȽ���� üũ��)
UINT64 TPSCount();

// �ʴ� Send����Ʈ �� (Send Per Second)
UINT64 SPSCount();

// �ʴ� Recv����Ʈ �� (Recv Per Second)
UINT64 RPSCount();


/////////////////////////////
// Json �� ���� ����� �Լ�
////////////////////////////
// ���̽��� ������ ������ �����ϴ� �Լ�(UTF-16)
bool Json_Create();

// ���� ���� �� ������ ���� �Լ�
bool FileCreate_UTF16(const TCHAR* FileName, const TCHAR* tpJson, size_t StringSize);

// ���̽����� ������ �о�� �����ϴ� �Լ�(UTF-16)
bool Json_Get();

// ���Ͽ��� ������ �о���� �Լ�
// pJson�� �о�� �����Ͱ� ����ȴ�.
bool LoadFile_UTF16(const TCHAR* FileName, TCHAR** pJson);




/////////////////////////
// Recv ó�� �Լ���
/////////////////////////
// Recv() ó��
bool RecvProc(SOCKET sock);

// ��Ŷ ó��
bool PacketProc(WORD PacketType, SOCKET sock, char* Packet);

// "ȸ������ ��û(ȸ����� �߰�)" ��Ŷ ó��
bool Network_Req_AccountAdd(SOCKET sock, char* Packet);

// "�α��� ��û" ��Ŷ ó��
bool Network_Req_Login(SOCKET sock, char* Packet);

// "ȸ�� ��� ��û" ��Ŷ ó��
bool Network_Req_AccountList(SOCKET sock, char* Packet);

// "ģ�� ��� ��û" ��Ŷ ó��
bool Network_Req_FriendList(SOCKET sock, char* Packet);

// "���� ģ����û ���" ��Ŷ ó��
bool Network_Req_RequestList(SOCKET sock, char* Packet);

// "���� ģ����û ���" ��Ŷ ó��
bool Network_Req_ReplytList(SOCKET sock, char* Packet);

// "ģ������ ��û" ��Ŷ ó��
bool Network_Req_FriendRemove(SOCKET sock, char* Packet);

// "ģ�� ��û" ��Ŷ ó��
bool Network_Req_FriendRequest(SOCKET sock, char* Packet);

// "ģ����û ���" ��Ŷ ó��
bool Network_Req_RequestCancel(SOCKET sock, char* Packet);

// "������û �ź�" ��Ŷ ó��
bool Network_Req_RequestDeny(SOCKET sock, char* Packet);

// "ģ����û ����" ��Ŷ ó��
bool Network_Req_FriendAgree(SOCKET sock, char* Packet);

// ��Ʈ���� �׽�Ʈ�� ��Ŷ ó��
bool Network_Req_StressTest(SOCKET sock, char* Packet);




/////////////////////////
// ��Ŷ ���� �Լ�
/////////////////////////
// "ȸ������ ��û(ȸ����� �߰�)" ��Ŷ ����
void Network_Res_AccountAdd(char* header, char* Packet, UINT64 AccountNo);

// "�α��� ��û ���" ��Ŷ ����
void Network_Res_Login(char* header, char* Packet, UINT64 AccountNo, TCHAR* Nick);

// "ȸ�� ��� ��û ���" ��Ŷ ����
void Network_Res_AccountList(char* header, char* Packet);

// "ģ�� ��� ��û ���" ��Ŷ ����
void Network_Res_FriendList(char* header, char* Packet, UINT64 AccountNo, UINT FriendCount);

// "���� ģ����û ��� ���" ��Ŷ ó��
void Network_Res_RequestList(char* header, char* Packet, UINT64 AccountNo, UINT RequestCount);

// "���� ģ����û ��� ���" ��Ŷ ó��
void Network_Res_ReplytList(char* header, char* Packet, UINT64 AccountNo, UINT ReplyCount);

// "ģ������ ��û ���" ��Ŷ ����
void Network_Res_FriendRemove(char* header, char* Packet, UINT64 AccountNo, BYTE Result);

// "ģ�� ��û ���" ��Ŷ ����
void Network_Res_FriendRequest(char* header, char* Packet, UINT64 AccountNo, BYTE Result);

// "ģ����û ��� ���" ��Ŷ ����
void Network_Res_RequestCancel(char* header, char* Packet, UINT64 AccountNo, BYTE Result);

// "������û ��� ���" ��Ŷ ����
void Network_Res_RequestDeny(char* header, char* Packet, UINT64 AccountNo, BYTE Result);

// "ģ����û ���� ���" ��Ŷ ����
void Network_Res_FriendAgree(char* header, char* Packet, UINT64 AccountNo, BYTE Result);



/////////////////////////
// Send ó��
/////////////////////////
// Send���ۿ� ������ �ֱ�
bool SendPacket(CRingBuff* SendBuff, CProtocolBuff* headerBuff, CProtocolBuff* payloadBuff);

// Send������ �����͸� Send�ϱ�
bool SendProc(SOCKET sock);



#endif // !__NETWORK_FUNC_H

