#include "Scene_Class.h"

int main()
{	
	cs_Initial();	// ȭ���� Ŀ�� �Ⱥ��̰� ó�� �� �ڵ� ����
	system("mode con cols=83 lines=33");
	while (1)
	{
		hSceneHandle.run();
	}

	return 0;
}

