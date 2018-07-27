#ifndef __NETWORK_FUNC_H__

#pragma comment(lib,"ws2_32")
#include <Ws2tcpip.h>
#include "Protocol.h"
#include "ProtocolBuff\ProtocolBuff.h"
#include "RingBuff\RingBuff.h"

using namespace Library_Jingyu;

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




// ���漱���
struct stSession;
struct stDummyClient;


// ---------------------------------
// ��Ÿ �Լ�
// ---------------------------------
// ��Ʈ��ũ ���μ���
// ���⼭ false�� ��ȯ�Ǹ�, ���α׷��� ����ȴ�.
bool Network_Process();

// ��Ʈ��ũ ���� �Լ� (�����ʱ�ȭ, Ŀ��Ʈ ��..)
bool Network_Init(int* TryCount, int* FailCount, int* SuccessCount);

// Disconnect ó��
void Disconnect(stSession* Account);

// �α� ��� �Լ�
void Log(TCHAR *szString, int LogLevel);

// ���巹Ŀ�� �Լ�
// ���ڷ� (���� �׼�, �׼� ���� �ð�, �׼� ���� ��ġ, (OUT)��� �� ��ǥ)�� �޴´�.
void DeadReckoning(BYTE NowAction, ULONGLONG ActionTick, int ActionStartX, int ActionStartY, int* ResultX, int* ResultY);

// �ʴ� RTT ���.
ULONGLONG RTTAvr();





// ---------------------------------
// �˻��� �Լ�
// --------------------------------
// ���ڷ� ���� Socket ���� �������� [ȸ�� ���]���� [������ ��󳽴�].(�˻�)
// ���� ��, �ش� ������ ���� ����ü�� �ּҸ� ����
// ���� �� nullptr ����
stSession* ClientSearch_AcceptList(SOCKET sock);

// ���ڷ� ���� ���� ID�� [�÷��̾� ���]���� [�÷��̾ ��󳽴�.] (�˻�)
// ���� ��, �ش� �÷��̾��� ����ü �ּҸ� ����
// ���� �� nullptr ����
stDummyClient* ClientSearch_ClientList(DWORD ID);





// ---------------------------------
// AI �Լ�
// ---------------------------------
void DummyAI();




// ---------------------------------
// Update()���� ���� �׼� ó�� �Լ�
// ---------------------------------
// ���� �̵� �׼� ó��
void Action_Move(stDummyClient* NowPlayer);









// ---------------------------------
// Recv ó�� �Լ���
// ---------------------------------
// Recv() ó��
bool RecvProc(stSession* Account);

// ��Ŷ ó��
bool PacketProc(WORD PacketType, stDummyClient* Player, CProtocolBuff* Payload);

// �� ĳ���� ����
bool Network_Recv_CreateMyCharacter(stDummyClient* Player, CProtocolBuff* Payload);

// �ٸ� ��� ĳ���� ����
bool Network_Recv_CreateOtherCharacter(stDummyClient* Player, CProtocolBuff* Payload);

// ĳ���� ����
bool Network_Recv_DeleteCharacter(stDummyClient* Player, CProtocolBuff* Payload);

// �ٸ� ���� �̵� ����
bool Network_Recv_OtherMoveStart(stDummyClient* Player, CProtocolBuff* Payload);

// �ٸ� ���� �̵� ����
bool Network_Recv_OtherMoveStop(stDummyClient* Player, CProtocolBuff* Payload);

// �ٸ� ���� ���� 1�� ����
bool Network_Recv_OtherAttack_01(stDummyClient* Player, CProtocolBuff* Payload);

// �ٸ� ���� ���� 2�� ����
bool Network_Recv_OtherAttack_02(stDummyClient* Player, CProtocolBuff* Payload);

// �ٸ� ���� ���� 3�� ����
bool Network_Recv_OtherAttack_03(stDummyClient* Player, CProtocolBuff* Payload);

// ������ ��Ŷ
bool Network_Recv_Damage(stDummyClient* Player, CProtocolBuff* Payload);

// ��ũ ��Ŷ
bool Network_Recv_Sync(stDummyClient* Player, CProtocolBuff* Payload);

// ���� ��Ŷ
bool Network_Recv_Echo(stDummyClient* Player, CProtocolBuff* Payload);




// ---------------------------------
// ��Ŷ ���� �Լ�
// ---------------------------------
// ��� ���� �Լ�
void CreateHeader(CProtocolBuff* header, BYTE PayloadSize, BYTE PacketType);

// �̵� ���� ��Ŷ �����
bool Network_Send_MoveStart(BYTE Dir, WORD X, WORD Y, CProtocolBuff *header, CProtocolBuff* payload);

// �̵� ���� ��Ŷ �����
bool Network_Send_MoveStop(BYTE Dir, WORD X, WORD Y, CProtocolBuff *header, CProtocolBuff* payload);

// ���� 1�� ���� ��Ŷ �����
bool Network_Send_Attack_01(BYTE Dir, WORD X, WORD Y, CProtocolBuff *header, CProtocolBuff* payload);

// ���� 2�� ���� ��Ŷ �����
bool Network_Send_Attack_02(BYTE Dir, WORD X, WORD Y, CProtocolBuff *header, CProtocolBuff* payload);

// ���� 3�� ���� ��Ŷ �����
bool Network_Send_Attack_03(BYTE Dir, WORD X, WORD Y, CProtocolBuff *header, CProtocolBuff* payload);

// ���� ��Ŷ �����
bool Network_Send_Echo(CProtocolBuff *header, CProtocolBuff* payload);



// ---------------------------------
// Send
// ---------------------------------
// SendBuff�� ������ �ֱ�
bool SendPacket(stSession* Account, CProtocolBuff* header, CProtocolBuff* payload);

// SendBuff�� �����͸� Send�ϱ�
bool SendProc(stSession* Account);





#endif // !__NETWORK_FUNC_H__


