#ifndef __DEFINE__
#define __DEFINE__


struct st_SESSION
{
	int SessionID;
};


struct st_PLAYER
{
	int SessionID = -1;	// -1�̸� �� �����̴�.
	int Content[3];

	bool UseFlag = false;	// ����� ����. true�� ��� ��. false�� ��� �� �ƴ�
};


#define dfTHREAD_NUM	3
#endif