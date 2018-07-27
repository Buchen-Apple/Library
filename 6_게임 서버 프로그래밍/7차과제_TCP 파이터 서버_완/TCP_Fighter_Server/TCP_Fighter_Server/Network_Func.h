#pragma once

#pragma comment(lib,"ws2_32")
#include <Ws2tcpip.h>
#include "Protocol.h"
#include "ProtocolBuff\ProtocolBuff.h"
#include "RingBuff\RingBuff.h"
//#include "profiling\Profiling_Class.h"

// Profiling_Class.h�� ����Ǿ� ���� �ʴٸ�, �Ʒ� ��ũ�ε��� �������� ����. 
#ifndef __PROFILING_CLASS_H__
#define BEGIN(STR) 
#define END(STR)
#define FREQUENCY_SET()
#define PROFILING_SHOW()
#define PROFILING_FILE_SAVE()
#define RESET()
#else
#define BEGIN(STR)				BEGIN(STR)
#define END(STR)				END(STR)
#define FREQUENCY_SET()			FREQUENCY_SET()
#define PROFILING_SHOW()		PROFILING_SHOW()
#define PROFILING_FILE_SAVE()	PROFILING_FILE_SAVE()
#define RESET()					RESET()

#endif // !__PROFILING_CLASS_H__

using namespace Library_Jingyu;

// ����ü ���漱��
struct stAccount;
struct stPlayer;
struct stSectorCheck;

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


// ---------------------------------
// ��Ÿ �Լ�
// ---------------------------------
// ��Ʈ��ũ ���μ���
// ���⼭ false�� ��ȯ�Ǹ�, ���α׷��� ����ȴ�.
bool Network_Process(SOCKET* listen_sock);

// Accept ó��.
bool Accept(SOCKET* client_sock, SOCKADDR_IN clinetaddr);

// Accept ��, �� ���� 9���� ������ ������ ������ NewAccount�� SendBuff�� �ֱ�
bool Accept_Surport(stPlayer* Player, stSectorCheck* Sector);

// Disconnect ó��
void Disconnect(stAccount* Account);

// ���巹Ŀ�� �Լ�
// ���ڷ� (���� �׼�, �׼� ���� �ð�, �׼� ���� ��ġ, (OUT)��� �� ��ǥ)�� �޴´�.
void DeadReckoning(BYTE NowAction, ULONGLONG ActionTick, int ActionStartX, int ActionStartY, int* ResultX, int* ResultY);

// �α� ��� �Լ�
void Log(TCHAR *szString, int LogLevel);





// ---------------------------------
// ����ó���� �Լ�
// ---------------------------------
// ���� ü���� üũ
void SectorChange(stAccount* Account);

// ���ڷ� ���� Sector�� �������� SendBuff�� ���ڷ� ���� ��Ŷ �ֱ�
bool SendPacket_Sector(stPlayer* Player, CProtocolBuff* header, CProtocolBuff* payload, stSectorCheck* Sector);

// ���ڷ� ���� ���� 1���� �������� SendBuff�� ���ڷ� ���� ��Ŷ �ֱ�.
bool SendPacket_SpecialSector(stPlayer* Player, CProtocolBuff* header, CProtocolBuff* payload, int SectorX, int SectorY);

// ��� ������ Index, �ο� ���� ��� �Լ�. ������
void Print_Sector(stPlayer* NowPlayer);

// ���ڷ� ���� X,Y�� ��������, ���ڷ� ���� ����ü�� 9������ �־��ش�.
void GetSector(int SectorX, int SectorY, stSectorCheck* Sector);

// ���ڷ� ���� �õ� ������ǥ, �� ������ǥ�� �������� ĳ���͸� �����ؾ��ϴ� ����, ĳ���͸� �߰��ؾ��ϴ� ���͸� ���Ѵ�.
void GetUpdateSectorArount(int OldSectorX, int OldSectorY, int NewSectorX, int NewSectorY, stSectorCheck* RemoveSector, stSectorCheck* AddSector);

// ���Ͱ� �̵��Ǿ��� �� ȣ��Ǵ� �Լ�. ���� �������� ĳ���� ����/�߰��� üũ�Ѵ�.
void CharacterSectorUpdate(stPlayer* NowPlayer, int NewSectorX, int NowSectorY);







// ---------------------------------
// Update()���� ���� �׼� ó�� �Լ�
// ---------------------------------
// �� �÷��̾� �׼� üũ
void ActionProc();

// ���� �̵� �׼� ó��
void Action_Move(stPlayer* NowPlayer);







// ---------------------------------
// �˻��� �Լ�
// ---------------------------------
// ���ڷ� ���� Socket ���� �������� [ȸ�� ���]���� [������ ��󳽴�].(�˻�)
// ���� ��, �ش� ������ ���� ����ü�� �ּҸ� ����
// ���� �� nullptr ����
stAccount* ClientSearch_AcceptList(SOCKET sock);








// ---------------------------------
// Recv ó�� �Լ���
// ---------------------------------
// Recv() ó��
bool RecvProc(stAccount* Account);

// ��Ŷ ó��
bool PacketProc(WORD PacketType, stAccount* Account, CProtocolBuff* Payload);

// "ĳ���� �̵� ����" ��Ŷ ó��
bool Network_Req_MoveStart(stAccount* Account, CProtocolBuff* payload);

// "ĳ���� �̵� ����" ��Ŷ ó��
bool Network_Req_MoveStop(stAccount* Account, CProtocolBuff* payload);

// "���� 1��" ��Ŷ ó��
bool Network_Req_Attack_1(stAccount* Account, CProtocolBuff* payload);

// "���� 2��" ��Ŷ ó��
bool Network_Req_Attack_2(stAccount* Account, CProtocolBuff* payload);

// "���� 3��" ��Ŷ ó��
bool Network_Req_Attack_3(stAccount* Account, CProtocolBuff* payload);

// "���� 1��" ��Ŷ�� ������ ó��. Network_Req_Attack_1()���� ���.
bool Network_Requ_Attck_1_Damage(DWORD AttackID, int AttackX, int AttackY, BYTE AttackDir);

// "���� 2��" ��Ŷ�� ������ ó��. Network_Req_Attack_2()���� ���.
bool Network_Requ_Attck_2_Damage(DWORD AttackID, int AttackX, int AttackY, BYTE AttackDir);

// "���� 3��" ��Ŷ�� ������ ó��. Network_Req_Attack_3()���� ���.
bool Network_Requ_Attck_3_Damage(DWORD AttackID, int AttackX, int AttackY, BYTE AttackDir);

// ���� ��Ŷ ó��
bool Network_Req_StressTest(stAccount* Account, CProtocolBuff* Packet);




// ---------------------------------
// ��Ŷ ���� �Լ�
// ---------------------------------
// ��� ���� �Լ�
void CreateHeader(CProtocolBuff* header, BYTE PayloadSize, BYTE PacketType);

// "�� ĳ���� ����" ��Ŷ ����
void Network_Send_CreateMyCharacter(CProtocolBuff* header, CProtocolBuff* payload, DWORD ID, BYTE Dir, WORD X, WORD Y, BYTE HP);

// "�ٸ� ��� ĳ���� ����" ��Ŷ ����
void Network_Send_CreateOtherCharacter(CProtocolBuff* header, CProtocolBuff* payload, DWORD ID, BYTE Dir, WORD X, WORD Y, BYTE HP);

// "�ٸ� ��� ����" ��Ŷ ����
void Network_Send_RemoveOtherCharacteor(CProtocolBuff* header, CProtocolBuff* payload, DWORD ID);

// "�ٸ� ��� �̵� ����" ��Ŷ ����
void Network_Send_MoveStart(CProtocolBuff* header, CProtocolBuff* payload, DWORD ID, BYTE Dir, WORD X, WORD Y);

// "�ٸ� ��� ����" ��Ŷ ����
void Network_Send_MoveStop(CProtocolBuff* header, CProtocolBuff* payload, DWORD ID, BYTE Dir, WORD X, WORD Y);

// "��ũ ���߱�" ��Ŷ ����
void Network_Send_Sync(CProtocolBuff* header, CProtocolBuff* payload, DWORD ID, WORD X, WORD Y);

// "1�� ���� ����" ��Ŷ �����
void Network_Send_Attack_1(CProtocolBuff* header, CProtocolBuff* payload, DWORD ID, BYTE Dir, WORD X, WORD Y);

// "2�� ���� ����" ��Ŷ �����
void Network_Send_Attack_2(CProtocolBuff* header, CProtocolBuff* payload, DWORD ID, BYTE Dir, WORD X, WORD Y);

// "3�� ���� ����" ��Ŷ �����
void Network_Send_Attack_3(CProtocolBuff* header, CProtocolBuff* payload, DWORD ID, BYTE Dir, WORD X, WORD Y);

// "������" ��Ŷ �����
void Network_Send_Damage(CProtocolBuff *header, CProtocolBuff* payload, DWORD AttackID, DWORD DamageID, BYTE HP);




// ---------------------------------
// Send
// ---------------------------------
// SendBuff�� ������ �ֱ�
BOOL SendPacket(stAccount* Account, CProtocolBuff* header, CProtocolBuff* payload);

// SendBuff�� �����͸� Send�ϱ�
bool SendProc(stAccount* Account);



