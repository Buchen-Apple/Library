// Test.cpp : 이 파일에는 'main' 함수가 포함됩니다. 거기서 프로그램 실행이 시작되고 종료됩니다.
//

#include "pch.h"
#include "ProtocolBuff/ProtocolBuff(Net)_ObjectPool.h"

namespace Library_Jingyu
{
	LONG g_lNET_BUFF_SIZE = 200;
}


using namespace Library_Jingyu;

#pragma pack(push, 1)
struct Head
{
	BYTE Code;
	WORD Len;
	BYTE Random;
	BYTE Checksum;
};
#pragma pack(pop)

int _tmain()
{
	char aa[] = "aaaaaaaaaabbbbbbbbbbcccccccccc1234567890abcdefghijklmn";		

	CProtocolBuff_Net* Data = CProtocolBuff_Net::Alloc();
	Data->PutData(aa, 55);
	
	BYTE XORCode = 0xa9;
	Data->Encode2(119, XORCode);


	// -------------------------------


	CProtocolBuff_Net* Deq = CProtocolBuff_Net::Alloc();
	Deq->Clear();

	Head head;
	Data->GetData((char*)&head, 5);
	Data->GetData(Deq->GetBufferPtr(), 55);	
	Deq->MoveWritePos(55);		

	Deq->Decode2(head.Len, head.Random, head.Checksum, XORCode);	

	return 0;
}
