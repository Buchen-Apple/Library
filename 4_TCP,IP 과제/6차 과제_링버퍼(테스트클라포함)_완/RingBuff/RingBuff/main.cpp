#include <stdio.h>
#include <Windows.h>
#include <time.h>
#include "RingBuff.h"
#include "Profiling_Class.h"

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


#define HOPESIZE 121

int main()
{
	FREQUENCY_SET(); // ���ļ� ���ϱ�	
	CRingBuff test(1000);
	char Buff[] = { "0123456789 abcdefghijklmnopqrstuvwxyz songjingyu procademy 9876543210 ABCDEGHIJKNMNOPQRSTUVWXYZ hahahahahahahaHAHAHfinal" };

	int iEnqueueSize;
	int iDequeueSize;
	srand(time(NULL));

	int i = 0;

	while (i < 10000)
	{
		int hopeSize = HOPESIZE;
		char DeBuff[HOPESIZE];
		char PeekDeBuff[HOPESIZE];
		int BuffPlus = 0;
		int iTempHopeSize;
		int iTotal = 0;

		// ������ �ֱ�
		while (iTotal != HOPESIZE)
		{
			iTempHopeSize = rand() % HOPESIZE;
			if (iTotal + iTempHopeSize >= HOPESIZE)
				iTempHopeSize = HOPESIZE - iTotal;

			// ������ ��ť
			BEGIN("ENQUEUE");
			iEnqueueSize = test.Enqueue(&Buff[BuffPlus], iTempHopeSize);
			END("ENQUEUE");

			iTotal += iEnqueueSize;
			BuffPlus += iEnqueueSize;
		}

		BuffPlus = 0;
		iTotal = 0;

		// ������ ����
		while (iTotal != HOPESIZE)
		{
			iTempHopeSize = rand() % 10;
			if (iTotal + iTempHopeSize >= HOPESIZE)
				iTempHopeSize = HOPESIZE - iTotal;

			// �׽�Ʈ�� ���� Peek�� �Ѵ�.
			BEGIN("PEEK");
			test.Peek(&PeekDeBuff[BuffPlus], iTempHopeSize);
			END("PEEK");

			// ������ ��ť
			BEGIN("DEQUEUE");
			iDequeueSize = test.Dequeue(&DeBuff[BuffPlus], iTempHopeSize);
			END("DEQUEUE");

			iTotal += iDequeueSize;
			BuffPlus += iDequeueSize;
		}

		printf("%s", PeekDeBuff);
		printf("%s", DeBuff);
		Sleep(10);

		i++;		
	}


	PROFILING_FILE_SAVE();
	
	return 0;
}