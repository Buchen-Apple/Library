#include "stdafx.h"
#include "NetworkFunc.h"

CRingBuff RecvBUff(100);
CRingBuff SendBuff(100);


#define STRING_BUFF_SIZE	512

SOCKET sock;

// ture�� ���� ��, false�� ������ �ƴ�
bool SendFlag;

// �̹��� �����ؾ��ϴ��� üũ.
// true�� ���� �ؾ���. false�� ���� ���ؾ���
//bool SleepFlag;

// ------------
// ���ú� ���� �Լ���
// ------------
// RecvPost�Լ�. ���� Recv ȣ��
int RecvPost()
{
	int a = 10;

	// 1. recv ������ ������ ������.
	char* recvbuff = RecvBUff.GetBufferPtr();

	// 2. �� ���� �� �� �ִ� �������� ���� ���� ���
	int Size = RecvBUff.GetNotBrokenPutSize();

	// 3. �� ���� �� �� �ִ� ������ 0�̶��
	if (Size == 0)
	{
		// ���������� ���� ���, Size�� 1�� ����
		if (RecvBUff.GetFreeSize() > 0)
			Size = 1;
	}

	// 4. 1ĭ �̵��� rear ��ġ �˱�(���� rear ��ġ�� �̵������� ����)	
	int* rear = (int*)RecvBUff.GetWriteBufferPtr();
	int TempRear = RecvBUff.NextIndex(*rear, 1);

	// 5. recv()
	int retval = recv(sock, &recvbuff[TempRear], Size, 0);

	// 6. ���ú� ����üũ
	if (retval == SOCKET_ERROR)
	{
		// WSAEWOULDBLOCK������ �߻��ϸ�, ���� ���۰� ����ִٴ� ��. recv�Ұ� ���ٴ� ���̴� �Լ� ����.
		if (WSAGetLastError() == WSAEWOULDBLOCK)
			return true;

		// 10053, 10054 �Ѵ� ��� ������ ������ ����
		// WSAECONNABORTED(10053) :  �������ݻ��� ������ Ÿ�Ӿƿ��� ���� ����(����ȸ��. virtual circle)�� ������ ���
		// WSAECONNRESET(10054) : ���� ȣ��Ʈ�� ������ ����.
		// �� ���� �׳� return false�� ���� ����� ������ ���´�.
		if (WSAGetLastError() == WSAECONNRESET || WSAGetLastError() == WSAECONNABORTED)
			return false;

		// WSAEWOULDBLOCK������ �ƴϸ� ���� �̻��� ���̴� ���� ����
		else
		{
			_tprintf(L"RecvProc(). retval �� ������ �ƴ� ������ ��(%d). ���� ����\n", WSAGetLastError());		
			return false;	// FALSE�� ���ϵǸ�, ������ �����.
		}
	}

	// Size�� 0�� ������ �ƴѵ�, 0�� ���ϵǴ� ���� ��������.
	else if (retval == 0 && Size != 0)
		return false;

	// 7. ������� ������ Rear�� �̵���Ų��.
	RecvBUff.MoveWritePos(retval);

	if (RecvProc() == false)
		return false;

	return retval;
}

// RecvProc �Լ�. ���ú� ť�� ������ �м�
bool RecvProc()
{
	while (1)
	{
		// 1. RecvBuff�� �ּ����� ����� �ִ��� üũ. (���� = ��� ������� ���ų� �ʰ�. ��, �ϴ� �����ŭ�� ũ�Ⱑ �ִ��� üũ)	
		stNETWORK_PACKET_HEADE HeaderBuff;

		// RecvBuff�� ��� ���� ���� ũ�Ⱑ ��� ũ�⺸�� �۴ٸ�, �Ϸ� ��Ŷ�� ���ٴ� ���̴� while�� ����.
		if (RecvBUff.GetUseSize() < dfNETWORK_PACKET_HEADER_SIZE)
		{
			break;
		}

		// 2. ����� Peek���� Ȯ���Ѵ�.  Peek �ȿ�����, ��� �ؼ����� len��ŭ �д´�. 
		// ���۰� ��������� ���� ����.
		int SIze = RecvBUff.Peek((char*)&HeaderBuff, dfNETWORK_PACKET_HEADER_SIZE);

		if (SIze == -1)
		{
			fputs("WorkerThread. Recv->Header Peek : Buff Empty\n", stdout);
			return false;
		}

		// 3. �ϼ��� ��Ŷ�� �ִ��� Ȯ��. (�ϼ� ��Ŷ ������ = ��� ������ + ���̷ε� Size + �Ϸ��ڵ�)
		// ��� ���, �ϼ� ��Ŷ ����� �ȵǸ� ������ Ȯ��.
		int UseSize = RecvBUff.GetUseSize();
		if (UseSize < (dfNETWORK_PACKET_HEADER_SIZE + HeaderBuff.bySize + 1))
		{
			//SleepFlag = false;
			return true;
		}

		// 4. ��� �ڵ� Ȯ��
		if (HeaderBuff.byCode != dfNETWORK_PACKET_CODE)
		{
			_tprintf(_T("RecvProc(). �Ϸ���Ŷ ��� ó�� �� PacketCodeƲ��. ���� ����\n"));
			return false;
		}

		// 5. RecvBuff���� Peek�ߴ� ����� ����� (�̹� Peek������, �׳� Remove�Ѵ�)
		RecvBUff.RemoveData(dfNETWORK_PACKET_HEADER_SIZE);

		// 6. RecvBuff���� ���̷ε� Size ��ŭ ���̷ε� ����ȭ ���۷� �̴´�. (��ť�̴�. Peek �ƴ�)
		CProtocolBuff PayloadBuff;
		DWORD PayloadSize = HeaderBuff.bySize;

		int DequeueSize = RecvBUff.Dequeue(PayloadBuff.GetBufferPtr(), PayloadSize);

		// ���۰� ��������� ���� ����
		if (DequeueSize == -1)
		{
			fputs("WorkerThread. Recv->Payload Dequeue : Buff Empty\n", stdout);
			return false;
		}
		PayloadBuff.MoveWritePos(DequeueSize);

		// 7. RecvBuff���� �����ڵ� 1Byte����.	(��ť�̴�. Peek �ƴ�)
		BYTE EndCode;
		DequeueSize = RecvBUff.Dequeue((char*)&EndCode, 1);
		if (DequeueSize == -1)
		{
			fputs("WorkerThread. Recv->EndCode Dequeue : Buff Empty\n", stdout);
			return false;
		}

		// 8. �����ڵ� Ȯ��
		if (EndCode != dfNETWORK_PACKET_END)
		{
			_tprintf(_T("RecvProc(). �Ϸ���Ŷ �����ڵ� ó�� �� ������Ŷ �ƴ�. ���� ����\n"));			
			return false;
		}

		// 9. ����� ����ִ� Ÿ�Կ� ���� �б�ó��. false�� ���ϵǸ� ���� ����.
		if (PacketProc(HeaderBuff.byType, &PayloadBuff) == false)
		{
			return false;
		}

		Sleep(300);

	}

	return true;
}

// ��Ŷ ó��. RecvProc()���� ���� ��Ŷ ó��.
bool PacketProc(BYTE PacketType, CProtocolBuff* Payload)
{

	switch (PacketType)
	{
		// ���ڿ� ���� ��Ŷ
	case dfPACKET_SC_ECHO:
	{
		if (Recv_Packet_Echo(Payload) == false)
			return false;
	}
	break;


	default:
		printf("�� �� ���� ��Ŷ!\n");
		break;
	}

	return true;
}




// ------------
// ��Ŷ �����
// ------------
// ��� ���� �Լ�
void CreateHeader(CProtocolBuff* header, BYTE PayloadSize, BYTE PacketType)
{
	// ��Ŷ �ڵ� (1Byte)
	*header << dfNETWORK_PACKET_CODE;

	// ���̷ε� ������ (1Byte)
	*header << PayloadSize;

	// ��Ŷ Ÿ�� (1Byte)
	*header << PacketType;

	// ������ (1Byte)
	BYTE temp = 12;
	*header << temp;
}

// ���� ��Ŷ �����
bool Network_Send_Echo(CProtocolBuff *header, CProtocolBuff* payload)
{
	char buf[STRING_BUFF_SIZE + 1];

	int test = (rand() % 20) + 3;

	for (int i = 0; i < test - 2; ++i)
		buf[i] = 'a';

	buf[test - 2] = 'b';
	buf[test - 1] = '\0';

	int len = (int)strlen(buf);

	if (len == 0)
	{
		printf("(strlen(buf) : %d, test : %d\n", len, test);
		return false;
	}

	printf("%s\n", buf);
	
	// ���̷ε忡 �ֱ�
	for(int i=0; i<len; ++i)
		*payload << buf[i];

	// ��� �����
	CreateHeader(header, payload->GetUseSize(), dfPACKET_CS_ECHO);

	return true;
}


// ------------
// ��Ŷ ó�� �Լ���
// ------------
// ���� ��Ŷ ó�� �Լ�
bool Recv_Packet_Echo(CProtocolBuff* Payload)
{
	// 1. ���̷ε� ������� �뷮 ���
	int Size = Payload->GetUseSize();

	// 2. �� �����ŭ ������ memcpy
	char* Text = new char[Size + 1];
	memcpy_s(Text, Size + 1, Payload->GetBufferPtr(), Size);

	// 3. ī�� �� ��ŭ payload�� front�̵�
	Payload->MoveWritePos(Size);

	// 4. ���ڿ� ����� ���� ������ ��ġ�� NULL �ֱ�
	Text[Size] = '\0';

	// 5. ���ڿ� ���
	printf("[���� ������] : %s\n", Text);	

	_tprintf(L"[TCP Ŭ��] %d ����Ʈ ����.\n\n", Size);

	delete Text;

	// 6. ������ �� �޾�����, �ٽ� ������ ����, ���� ���·� �����.
	SendFlag = false;
	//SleepFlag = true;


	return true;
}




// ------------
// ���� ���� �Լ���
// ------------
// ���� ���ۿ� ������ �ֱ�
bool SendPacket(CProtocolBuff* header, CProtocolBuff* payload)
{	

	// 1. ���� q�� ��� �ֱ�
	int Size = dfNETWORK_PACKET_HEADER_SIZE;
	int EnqueueCheck = SendBuff.Enqueue(header->GetBufferPtr(), Size);
	if (EnqueueCheck == -1)
	{
		fputs("SendPacket(). Header->Enqueue. Size Full\n", stdout);
		return false;
	}

	// 3. ���� q�� ���̷ε� �ֱ�
	int PayloadLen = payload->GetUseSize();
	EnqueueCheck = SendBuff.Enqueue(payload->GetBufferPtr(), PayloadLen);
	if (EnqueueCheck == -1)
	{
		fputs("SendPacket(). Payload->Enqueue. Size Full\n", stdout);
		return false;
	}

	// 3. ���� q�� �����ڵ� �ֱ�
	char EndCode = dfNETWORK_PACKET_END;
	EnqueueCheck = SendBuff.Enqueue(&EndCode, 1);
	if (EnqueueCheck == -1)
	{
		fputs("SendPacket(). EndCode->Enqueue. Size Full\n", stdout);
		return false;
	}	

	return true;

}

// ���� ������ ������ Send() �ϱ�
int SendPost()
{
	// 1. ���� �������� �������� ���� ������
	char* sendbuff = SendBuff.GetBufferPtr();

	int TotalSize = 0;

	// 2. SendBuff�� �� �������� or Send����(����)���� ��� send
	while (1)
	{
		// 3. SendBuff�� �����Ͱ� �ִ��� Ȯ��(���� üũ��)
		if (SendBuff.GetUseSize() == 0)
			break;

		// 4. �� ���� ���� �� �ִ� �������� ���� ���� ���
		int Size = SendBuff.GetNotBrokenGetSize();

		// 5. ���� ������ 0�̶�� 1�� ����. send() �� 1����Ʈ�� �õ��ϵ��� �ϱ� ���ؼ�.
		// �׷��� send()�� ������ �ߴ��� Ȯ�� ����
		if (Size == 0)
			Size = 1;

		// 6. front ������ ����
		int *front = (int*)SendBuff.GetReadBufferPtr();

		// 7. front�� 1ĭ �� index�� �˾ƿ���.
		int BuffArrayCheck = SendBuff.NextIndex(*front, 1);

		// 8. Send()
		int SendSize = send(sock, &sendbuff[BuffArrayCheck], Size, 0);

		// 9. ���� üũ. �����̸� �� �̻� �������� while�� ����. ���� Select�� �ٽ� ����
		if (SendSize == SOCKET_ERROR)
		{
			if (WSAGetLastError() == WSAEWOULDBLOCK ||
				WSAGetLastError() == 10054)
				break;

			else
			{
				_tprintf(_T("SendProc(). Send�� ���� �߻�. ���� ���� (%d)\n"), WSAGetLastError());
				return false;
			}

		}

		// 10. ���� ����� ��������, �� ��ŭ remove
		SendBuff.RemoveData(SendSize);

		TotalSize += SendSize;
	}

	_tprintf(L"[TCP Ŭ��] %d ����Ʈ ����.\n", TotalSize - 5);

	if (TotalSize != 0)
		SendFlag = true;

	return TotalSize;
}