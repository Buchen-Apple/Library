#ifndef  __PROTOCOL_STRUCT_H__
#define __PROTOCOL_STRUCT_H__

#include <windows.h>

////////////////////////////////////////////////////////
//
//	NetServer & ChatServer
//
////////////////////////////////////////////////////////

// ���� ���� (OnClientJoin)
struct st_Protocol_NetChat_OnClientJoin
{
	ULONGLONG	SessionID;
	WORD		Type;
};

// ���� ���� (OnClientLeave)
struct st_Protocol_NetChat_OnClientLeave
{
	ULONGLONG	SessionID;
	WORD		Type;
};


////////////////////////////////////////////////////////
//
//	Client & Server Protocol
//
////////////////////////////////////////////////////////

// ä�ü��� ���� �̵� ��û
#pragma pack(push, 1)
struct st_Protocol_CS_CHAT_REQ_SECTOR_MOVE
{
	ULONGLONG	SessionID;
	WORD		Type;

	INT64		AccountNo;
	WORD		SectorX;
	WORD		SectorY;
};
#pragma pack(pop)


// ä�ü��� ���� �̵� ���
#pragma pack(push, 1)
struct st_Protocol_CS_CHAT_RES_SECTOR_MOVE
{
	ULONGLONG	SessionID;
	WORD		Type;

	INT64		AccountNo;
	WORD		SectorX;
	WORD		SectorY;
};
#pragma pack(pop)


// ä�ü��� ä�� ������ ��û
#define CHAT_MAX_SIZE	512
#pragma pack(push, 1)
struct st_Protocol_CS_CHAT_REQ_MESSAGE
{
	ULONGLONG	SessionID;
	WORD		Type;

	INT64		AccountNo;
	WORD		MessageLen;
	WCHAR		Message[CHAT_MAX_SIZE+2];	// null ������
};
#pragma pack(pop)


// ä�ü��� ä�� ������ ����
// ���� �޸�Ǯ���� �ٷ�� ����ü.
#pragma pack(push, 1)
struct st_Protocol_CS_CHAT_RES_MESSAGE
{
	ULONGLONG	SessionID;
	WORD		Type;

	INT64		AccountNo;

	WCHAR		ID[20];						// null ����
	WCHAR		Nickname[20];				// null ����
	WORD		MessageLen;
	WCHAR		Message[CHAT_MAX_SIZE + 2];	// null ������
};
#pragma pack(pop)


// ��Ʈ��Ʈ
#pragma pack(push, 1)
struct st_Protocol_CS_CHAT_REQ_HEARTBEAT
{
	ULONGLONG	SessionID;
	WORD		Type;
};
#pragma pack(pop)


// �޽��� �� ���� ū �޽��� ����ü. define �Ǿ�����
#define BINGNODE	st_Protocol_CS_CHAT_RES_MESSAGE


#endif // ! __PROTOCOL_STRUCT_H__
