#ifndef __MONITOR_PROTOCOL_H__
#define __MONITOR_PROTOCOL_H__


/*
����͸� ������ �������� ��������

���� CommonProtocol.h �� �������� ���������
CommonProtocol.h �� ä��,�α��� �� ��Ŷ���������� ���Ƽ�
ȥ���� �� �� �����Ƿ� ����͸����� ��Ŷ�� ������ ���� �帳�ϴ�.

�̸� ���� h �� �ٿ��� ��� ���ֽð�
���Ŀ� ä�ü��� ������� �ȳ� �� ���� ComonProtocol �� �帳�ϴ�.


# ���� ������Ʈ ä�ú������ - MO ���� �������� ��(ä��) ������ ä�ü����� ����˴ϴ�.
*/

enum en_PACKET_TYPE
{
	

};



// en_PACKET_SS_MONITOR_LOGIN
enum en_PACKET_CS_MONITOR_TOOL_SERVER_CONTROL
{
	dfMONITOR_SERVER_TYPE_LOGIN = 1,
	dfMONITOR_SERVER_TYPE_GAME = 2,
	dfMONITOR_SERVER_TYPE_CHAT = 3,
	dfMONITOR_SERVER_TYPE_AGENT = 4,

	dfMONITOR_SERVER_CONTROL_SHUTDOWN = 1,		// ���� �������� (���Ӽ��� ����)
	dfMONITOR_SERVER_CONTROL_TERMINATE = 2,		// ���� ���μ��� ��������
	dfMONITOR_SERVER_CONTROL_RUN = 3,		// ���� ���μ��� ���� & ����
};


enum en_PACKET_SS_MONITOR_DATA_UPDATE
{
	// ������ ������Ʈ(���� ��Ʈ�ѿ�) ���α׷��� �����Ƿ� �ϵ���� ��ü�� ����͸� ������
	// ��ġ����ŷ ������ ���� �ϵ��� �մϴ�. �ٸ������� �ص� �˴ϴ�. ������ �ϳ��� �ϸ� �˴ϴ�.

	// 2U ���� 1�뿡 ��ġ����ŷ / ��Ʋ / ä��  3���� ������ ���� �ǰڽ��ϴ�.

	// ���� ���� ���μ��� ON ���δ� �ܺο��� �Ǵ��Ͽ� ������ ������
	// ������Ʈ ���� ���� �ǹǷ� �ڱ� �ڽ��� �����°����� �մϴ�.
	// �ٸ� ������ �ش� ������ ��ü�� �Ȱ��� ������ �����ð� �Ŀ� ����͸� ���� OFF �� �ν��ϰ� �˴ϴ�.
	// OFF �Ǵܱ��� 5��~10�� ���� �ð��� �ҿ������ �׷��� �մϴ�.


	dfMONITOR_DATA_TYPE_SERVER_CPU_TOTAL = 1,                    // �ϵ���� CPU ���� ��ü
	dfMONITOR_DATA_TYPE_SERVER_AVAILABLE_MEMORY,                 // �ϵ���� ��밡�� �޸�
	dfMONITOR_DATA_TYPE_SERVER_NETWORK_RECV,                     // �ϵ���� �̴��� ���� ����Ʈ
	dfMONITOR_DATA_TYPE_SERVER_NETWORK_SEND,                     // �ϵ���� �̴��� �۽� ����Ʈ
	dfMONITOR_DATA_TYPE_SERVER_NONPAGED_MEMORY,                  // �ϵ���� �������� �޸� ��뷮

	dfMONITOR_DATA_TYPE_MATCH_SERVER_ON,                        // ��ġ����ŷ ���� ON
	dfMONITOR_DATA_TYPE_MATCH_CPU,                              // ��ġ����ŷ CPU ���� (Ŀ�� + ����)
	dfMONITOR_DATA_TYPE_MATCH_MEMORY_COMMIT,                    // ��ġ����ŷ �޸� ���� Ŀ�� ��뷮 (Private) MByte
	dfMONITOR_DATA_TYPE_MATCH_PACKET_POOL,                      // ��ġ����ŷ ��ŶǮ ��뷮
	dfMONITOR_DATA_TYPE_MATCH_SESSION,                          // ��ġ����ŷ ���� ����
	dfMONITOR_DATA_TYPE_MATCH_PLAYER,                           // ��ġ����ŷ ���� ���� (�α��� ���� ��)
	dfMONITOR_DATA_TYPE_MATCH_MATCHSUCCESS,                     // ��ġ����ŷ �� ���� ���� �� (�ʴ�)


	dfMONITOR_DATA_TYPE_MASTER_SERVER_ON,                        // ������ ���� ON
	dfMONITOR_DATA_TYPE_MASTER_CPU,                              // ������ CPU ���� (���μ���)
	dfMONITOR_DATA_TYPE_MASTER_CPU_SERVER,                       // ������ CPU ���� (���� ��ǻ�� ��ü)
	dfMONITOR_DATA_TYPE_MASTER_MEMORY_COMMIT,                    // ������ �޸� ���� Ŀ�� ��뷮 (Private) MByte
	dfMONITOR_DATA_TYPE_MASTER_PACKET_POOL,                      // ������ ��ŶǮ ��뷮
	dfMONITOR_DATA_TYPE_MASTER_MATCH_CONNECT,                    // ������ ��ġ����ŷ ���� ���� ��
	dfMONITOR_DATA_TYPE_MASTER_MATCH_LOGIN,                      // ������ ��ġ����ŷ ���� �α��� ��
	dfMONITOR_DATA_TYPE_MASTER_STAY_CLIENT,                      // ������ ����� Ŭ���̾�Ʈ
	dfMONITOR_DATA_TYPE_MASTER_BATTLE_CONNECT,                   // ������ ��Ʋ ���� ���� ��
	dfMONITOR_DATA_TYPE_MASTER_BATTLE_LOGIN,                     // ������ ��Ʋ ���� �α��� ��
	dfMONITOR_DATA_TYPE_MASTER_BATTLE_STANDBY_ROOM,              // ������ ��Ʋ ���� �α��� ��



	dfMONITOR_DATA_TYPE_BATTLE_SERVER_ON,                       // ��Ʋ���� ON
	dfMONITOR_DATA_TYPE_BATTLE_CPU,                             // ��Ʋ���� CPU ���� (Ŀ�� + ����)
	dfMONITOR_DATA_TYPE_BATTLE_MEMORY_COMMIT,                   // ��Ʋ���� �޸� ���� Ŀ�� ��뷮 (Private) MByte
	dfMONITOR_DATA_TYPE_BATTLE_PACKET_POOL,                     // ��Ʋ���� ��ŶǮ ��뷮
	dfMONITOR_DATA_TYPE_BATTLE_AUTH_FPS,                        // ��Ʋ���� Auth ������ �ʴ� ���� ��
	dfMONITOR_DATA_TYPE_BATTLE_GAME_FPS,                        // ��Ʋ���� Game ������ �ʴ� ���� ��
	dfMONITOR_DATA_TYPE_BATTLE_SESSION_ALL,                     // ��Ʋ���� ���� ������ü
	dfMONITOR_DATA_TYPE_BATTLE_SESSION_AUTH,                    // ��Ʋ���� Auth ������ ��� �ο�
	dfMONITOR_DATA_TYPE_BATTLE_SESSION_GAME,                    // ��Ʋ���� Game ������ ��� �ο�
	dfMONITOR_DATA_TYPE_BATTLE_,		                        // ��Ʋ���� �α����� ������ ��ü �ο�		<- ����
	dfMONITOR_DATA_TYPE_BATTLE_ROOM_WAIT,                       // ��Ʋ���� ���� ��
	dfMONITOR_DATA_TYPE_BATTLE_ROOM_PLAY,                       // ��Ʋ���� �÷��̹� ��

	dfMONITOR_DATA_TYPE_CHAT_SERVER_ON,                         // ä�ü��� ON
	dfMONITOR_DATA_TYPE_CHAT_CPU,                               // ä�ü��� CPU ���� (Ŀ�� + ����)
	dfMONITOR_DATA_TYPE_CHAT_MEMORY_COMMIT,                     // ä�ü��� �޸� ���� Ŀ�� ��뷮 (Private) MByte
	dfMONITOR_DATA_TYPE_CHAT_PACKET_POOL,                       // ä�ü��� ��ŶǮ ��뷮
	dfMONITOR_DATA_TYPE_CHAT_SESSION,                           // ä�ü��� ���� ������ü
	dfMONITOR_DATA_TYPE_CHAT_PLAYER,                            // ä�ü��� �α����� ������ ��ü �ο�
	dfMONITOR_DATA_TYPE_CHAT_ROOM                               // ��Ʋ���� �� ��

};





#endif // !__MONITOR_PROTOCOL_H__
