#ifndef __PROTOCOL_H__
#define __PROTOCOL_H__

#define SERVER_PORT	9000

// -------------------------
// ��Ŷ ��� ����
//
// �� 6����Ʈ
// WORD - Type
// DWORD - PayloadSize
// -------------------------

// ��� ������
#define dfNETWORK_PACKET_HEADER_SIZE	4

#pragma pack(push, 1)
struct stNETWORK_PACKET_HEADE
{
	BYTE	byCode;
	BYTE	bySize;
	BYTE	byType;
	BYTE	byTemp;
};
#pragma pack(pop)


//---------------------------------------------------------------
// ��Ŷ�� ���� �տ� �� ��Ŷ�ڵ�.
//---------------------------------------------------------------
#define dfNETWORK_PACKET_CODE	((BYTE)0x89)


//---------------------------------------------------------------
// ��Ŷ�� ���� �ڿ� �� ��Ŷ�ڵ�.
// ��Ŷ�� �� �κп��� 1Byte �� EndCode �� ���Եȴ�.  0x79
//---------------------------------------------------------------
#define dfNETWORK_PACKET_END	((BYTE)0x79)



#define	dfPACKET_CS_ECHO						252
//---------------------------------------------------------------
// Echo �� ��Ŷ					Client -> Server
//
//	4	-	Time
//
//---------------------------------------------------------------

#define	dfPACKET_SC_ECHO						253
//---------------------------------------------------------------
// Echo ���� ��Ŷ				Server -> Client
//
//	4	-	Time
//
//---------------------------------------------------------------




#endif // !__PROTOCOL_H__


