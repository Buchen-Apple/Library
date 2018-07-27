#pragma once
#ifndef __NETWORK_FUNC_H__
#define __NETWORK_FUNC_H__

#include "Network_Protocol.h"
#include "List_Template.h"
#pragma comment(lib, "ws2_32")
#include <WS2tcpip.h>
#pragma warning(disable:4996)

// �ؽ�Ʈ �ƿ����� ���� ��� �Լ�
void ErrorTextOut(const TCHAR*);

// ���� �ʱ�ȭ, ���� ����, Ŀ��Ʈ �� �Լ�
BOOL NetworkInit(HWND hWNd, TCHAR tIP[30]);

// ���� ����, ���� ���� �� �Լ�
void NetworkClose();

// ��Ʈ��ũ ó�� �Լ�
BOOL NetworkProc(LPARAM);

// ��Ʈ��ũ ó�� ��, Recv ó�� �Լ�
BOOL RecvProc();

// ���� Send �Լ�
BOOL SendProc();

// ��Ŷ Ÿ�Կ� ���� ó�� �Լ�
BOOL PacketProc(BYTE PacketType, char* Packet);



/////////////////////////
// Send ��Ŷ ����� �Լ�
/////////////////////////
// �̵� ���� ��Ŷ �����
void SendProc_MoveStart(st_NETWORK_PACKET_HEADER* header, stPACKET_CS_MOVE_START* packet, int dir, int x, int y);

// ���� ��Ŷ �����
void SendProc_MoveStop(st_NETWORK_PACKET_HEADER* header, stPACKET_CS_MOVE_STOP* packet, int dir, int x, int y);

// ���� ��Ŷ ����� (1�� ����)
void SendProc_Atk_01(st_NETWORK_PACKET_HEADER* header, stPACKET_CS_ATTACK* packet, int dir, int x, int y);

// ���� ��Ŷ ����� (2�� ����)
void SendProc_Atk_02(st_NETWORK_PACKET_HEADER* header, stPACKET_CS_ATTACK* packet, int dir, int x, int y);

// ���� ��Ŷ ����� (3�� ����)
void SendProc_Atk_03(st_NETWORK_PACKET_HEADER* header, stPACKET_CS_ATTACK* packet, int dir, int x, int y);





/////////////////////////
// Send ť�� ������ �ֱ� �Լ�
/////////////////////////
BOOL SendPacket(st_NETWORK_PACKET_HEADER* header, char* packet);



/////////////////////////
// Recv ������ ó�� �Լ�
/////////////////////////
// �� ĳ���� ���� ��Ŷ ó�� �Լ�
BOOL PacketProc_CharacterCreate_My(char* Packet);

// �ٸ���� ĳ���� ���� ��Ŷ ó�� �Լ�
BOOL PacketProc_CharacterCreate_Other(char* Packet);

// ĳ���� ���� ��Ŷ ó�� �Լ�
BOOL PacketProc_CharacterDelete(char* Packet);

// �ٸ� ĳ���� �̵� ���� ��Ŷ
BOOL PacketProc_MoveStart(char* Packet);

// �ٸ� ĳ���� �̵� ���� ��Ŷ
BOOL PacketProc_MoveStop(char* Packet);

// �ٸ� ĳ���� ���� 1�� ��Ŷ
BOOL PacketProc_Atk_01(char* Packet);

// �ٸ� ĳ���� ���� 2�� ��Ŷ
BOOL PacketProc_Atk_02(char* Packet);

// �ٸ� ĳ���� ���� 3�� ��Ŷ
BOOL PacketProc_Atk_03(char* Packet);

// ������ ��Ŷ
BOOL PacketProc_Damage(char* Packet);

#endif // !__NETWORK_FUNC_H__

