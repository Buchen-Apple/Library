#include "stdafx.h"
#include "NetworkFunc.h"

SRWLOCK g_Session_map_srwl;
map<SOCKET, stSession*> map_Session;
CRITICAL_SECTION cs;

extern int IOCountMinusCount;
extern int JoinUser;



// ���� ����
void Disconnect(stSession* DeleteSession)
{
	//_tprintf(L"[TCP ����] ���� ���� : IP �ּ�=%s, ��Ʈ=%d\n", DeleteSession->m_IP, DeleteSession->m_prot);

	// map���� ���ܽ�Ű��
	LockSession();
	map_Session.erase(DeleteSession->m_Client_sock);
	UnlockSession();

	// Ŭ���� ����, ���� ��������	
	closesocket(DeleteSession->m_Client_sock);
	delete DeleteSession;	

	JoinUser--;
}



// ------------
// ���ú� ���� �Լ���
// ------------
// RecvProc �Լ�. ť�� ���� üũ �� PacketProc���� �ѱ��.
void RecvProc(stSession* NowSession)
{
	// -----------------
	// Recv ť ���� ó��
	// -----------------
	// 1. ť ��
	//NowSession->m_RecvBuff.EnterLOCK();		// �� ----------------------

	while (1)
	{
		// 1. RecvBuff�� �ּ����� ����� �ִ��� üũ. (���� = ��� ������� ���ų� �ʰ�. ��, �ϴ� �����ŭ�� ũ�Ⱑ �ִ��� üũ)	
		WORD Header_PaylaodSize = 0;

		// RecvBuff�� ��� ���� ���� ũ�Ⱑ ��� ũ�⺸�� �۴ٸ�, �Ϸ� ��Ŷ�� ���ٴ� ���̴� while�� ����.
		//int UseSize = NowSession->m_RecvBuff.GetUseSize();
		if (NowSession->m_RecvBuff.GetUseSize() < dfNETWORK_PACKET_HEADER_SIZE)
		{
			break;
		}

		// 2. ����� Peek���� Ȯ���Ѵ�.  Peek �ȿ�����, ��� �ؼ����� len��ŭ �д´�. 
		// ���۰� ��������� ���� ����.
		int PeekSize = NowSession->m_RecvBuff.Peek((char*)&Header_PaylaodSize, dfNETWORK_PACKET_HEADER_SIZE);
		if (PeekSize == -1)
		{
			fputs("WorkerThread. Recv->Header Peek : Buff Empty\n", stdout);
			shutdown(NowSession->m_Client_sock, SD_BOTH);

			break;
		}

		// 3. �ϼ��� ��Ŷ�� �ִ��� Ȯ��. (�ϼ� ��Ŷ ������ = ��� ������ + ���̷ε� Size)
		// ��� ���, �ϼ� ��Ŷ ����� �ȵǸ� while�� ����.
		if (NowSession->m_RecvBuff.GetUseSize() < (dfNETWORK_PACKET_HEADER_SIZE + Header_PaylaodSize))
		{
			break;
		}

		// 4. RecvBuff���� Peek�ߴ� ����� ����� (�̹� Peek������, �׳� Remove�Ѵ�)
		NowSession->m_RecvBuff.RemoveData(dfNETWORK_PACKET_HEADER_SIZE);

		// 5. RecvBuff���� ���̷ε� Size ��ŭ ���̷ε� ����ȭ ���۷� �̴´�. (��ť�̴�. Peek �ƴ�)
		CProtocolBuff PayloadBuff;
		DWORD PayloadSize = Header_PaylaodSize;

		int DequeueSize = NowSession->m_RecvBuff.Dequeue(PayloadBuff.GetBufferPtr(), PayloadSize);

		// ���۰� ��������� ���� ����
		if (DequeueSize == -1)
		{
			fputs("WorkerThread. Recv->Payload Dequeue : Buff Empty\n", stdout);
			shutdown(NowSession->m_Client_sock, SD_BOTH);

			break;
		}
		PayloadBuff.MoveWritePos(DequeueSize);

		// 9. ����� ����ִ� Ÿ�Կ� ���� �б�ó��. false�� ���ϵǸ� ���� ����.
		if (PacketProc(NowSession, &PayloadBuff) == false)
		{
			fputs("PacketProc. return false\n", stdout);
			shutdown(NowSession->m_Client_sock, SD_BOTH);
			break;
		}

	}

	// 3. �� ����
	//NowSession->m_RecvBuff.LeaveLOCK();		// �� ���� ----------------------

}

// RecvPost �Լ�. �񵿱� ����� ����
bool RecvPost(stSession* NowSession)
{
	// ------------------
	// �񵿱� ����� ����
	// ------------------
	// 1. WSABUF ���� �� Recv������ ���ɱ�
	WSABUF wsabuf[2];
	//NowSession->m_RecvBuff.EnterLOCK();  // �� ---------------------------

	// 2. recv ������ ������ ������.
	char* recvBuff = NowSession->m_RecvBuff.GetBufferPtr();		

	// 3. WSABUF ����.
	int wsabufCount = 0;	

	int* rear = (int*)NowSession->m_RecvBuff.GetWriteBufferPtr();
	int TempRear = NowSession->m_RecvBuff.NextIndex(*rear, 1);

	int FreeSize = NowSession->m_RecvBuff.GetFreeSize();
	int Size = NowSession->m_RecvBuff.GetNotBrokenPutSize();

	if (Size < FreeSize)
	{
		wsabuf[0].buf = &recvBuff[TempRear];
		wsabuf[0].len = Size;

		wsabuf[1].buf = &recvBuff[0];
		wsabuf[1].len = FreeSize - Size;
		wsabufCount = 2;	

	}

	// 4. �װ� �ƴ϶��, WSABUF�� 1���� �����Ѵ�.
	else
	{
		wsabuf[0].buf = &recvBuff[TempRear];
		wsabuf[0].len = Size;

		wsabufCount = 1;		
	}

	// 5. Overlapped ����ü �ʱ�ȭ
	ZeroMemory(&NowSession->m_RecvOverlapped, sizeof(NowSession->m_RecvOverlapped));

	// 6. WSARecv()
	DWORD recvBytes, flags = 0;
	InterlockedIncrement64((volatile LONG64*)&NowSession->m_IOCount);
	int retval = WSARecv(NowSession->m_Client_sock, wsabuf, wsabufCount, &recvBytes, &flags, &NowSession->m_RecvOverlapped, NULL);

	//NowSession->m_RecvBuff.LeaveLOCK();   // �� ���� ---------------------------

	 // 7. ���ϰ��� 0�̰�, ����Ʈ�� 0�̸� ��������.
	if (&NowSession->m_RecvOverlapped == nullptr && recvBytes == 0)
	{
		// I/Oī��Ʈ�� 0�̶��, ���� ����
		int Nowval = InterlockedDecrement64((volatile LONG64*)&NowSession->m_IOCount);
		if (Nowval < 0)
			IOCountMinusCount++;

		if (Nowval == 0)
			Disconnect(NowSession);
	}

	// 8. ���� ó��
	if (retval == SOCKET_ERROR)
	{
		int Error = WSAGetLastError();

		// �񵿱� ������� ���۵Ȱ� �ƴ϶��
		if (Error != WSA_IO_PENDING)
		{
			// I/Oī��Ʈ�� 0�̶��, ���� ����
			int Nowval = InterlockedDecrement64((volatile LONG64*)&NowSession->m_IOCount);
			if (Nowval < 0)
				IOCountMinusCount++;

			if (Nowval == 0)
				Disconnect(NowSession);

			if (Error != WSAECONNRESET && Error != WSAESHUTDOWN)
			{
				printf("Recv return false1111 %d\n", Error);
				return false;
			}

			// �ٵ�, ���� �����̶��
			else if (Error == WSAENOBUFS)
			{
				// ȭ�鿡 ���ۺ��� ��� 
				_tprintf(L"[TCP ����] ���� ������ ���� : IP �ּ�=%s, ��Ʈ=%d\n", NowSession->m_IP, NowSession->m_prot);
			}

			else if (Error == WSAECONNRESET)
				return true;

			printf("Recv return false2222, %d\n", Error);

			return false;
		}
	}	

	return true;
}

// ��Ŷ ó��. RecvProc()���� ���� ��Ŷ ó��.
bool PacketProc(stSession* NowSession, CProtocolBuff* Payload)
{
	if (Recv_Packet_Echo(NowSession, Payload) == false)
		return false;	

	return true;	
}




// ------------
// ��Ŷ ó�� �Լ���
// ------------
// ���� ��Ŷ ó�� �Լ�
bool Recv_Packet_Echo(stSession* NowSession, CProtocolBuff* Payload)
{
	// 1. ���̷ε� ������� �뷮 ���
	int Size = Payload->GetUseSize();

	// 2. �� �����ŭ ������ memcpy
	char* Text = new char[Size];
	memcpy_s(Text, Size, Payload->GetBufferPtr(), Size);

	// 3. ī�� �� ��ŭ payload�� front�̵�
	Payload->MoveReadPos(Size);

	// 4. ���� ������ �ٽ� �����ϱ�.
	// ���� ��Ŷ �����
	CProtocolBuff Header;
	CProtocolBuff Payload_2;
	Send_Packet_Echo(&Header, &Payload_2, Text, Size);

	// 5. ���� ��Ŷ�� SendBuff�� �ֱ�
	if (SendPacket(NowSession, &Header, &Payload_2) == false)
		return false;

	// 6. ������ �����ϱ�.
	//EnterCriticalSection(&cs);
	SendPost(NowSession);	
	//LeaveCriticalSection(&cs);

	delete Text;

	return true;
}



// ------------
// ��Ŷ ����� �Լ���
// ------------
// ��� �����
void Send_Packet_Header(WORD PayloadSize, CProtocolBuff* Header)
{
	*Header << PayloadSize;
}

// ������Ŷ �����
void Send_Packet_Echo(CProtocolBuff* header, CProtocolBuff* payload, char* RetrunText, int RetrunTextSize)
{
	// 1. ���̷ε� ����
	// ������Ŷ��, ���ڷ� ���� retrunText�� payload�� �ִ� ��Ȱ.
	memcpy_s(payload->GetBufferPtr(), payload->GetFreeSize(), RetrunText, RetrunTextSize);
	payload->MoveWritePos(RetrunTextSize);

	// 2. ��� ����
	Send_Packet_Header(payload->GetUseSize(), header);
}





// ------------
// ���� ���� �Լ���
// ------------
// ���� ���ۿ� ������ �ֱ�
bool SendPacket(stSession* NowSession, CProtocolBuff* header, CProtocolBuff* payload)
{
	//EnterCriticalSection(&cs);

	// 1. ť ��
	NowSession->m_SendBuff.EnterLOCK();		// �� ---------------------------

	// 2. ���� q�� ��� �ֱ�
	int Size = dfNETWORK_PACKET_HEADER_SIZE;
	int EnqueueCheck = NowSession->m_SendBuff.Enqueue(header->GetBufferPtr(), Size);	
	if (EnqueueCheck == -1)
	{		
		fputs("SendPacket(). Header->Enqueue. Size Full\n", stdout);

		//LeaveCriticalSection(&cs);
		NowSession->m_SendBuff.LeaveLOCK();	 // ������ ---------------------------
		return false;
	}

	// 3. ���� q�� ���̷ε� �ֱ�
	int PayloadLen = payload->GetUseSize();
	EnqueueCheck = NowSession->m_SendBuff.Enqueue(payload->GetBufferPtr(), PayloadLen);
	if (EnqueueCheck == -1)
	{		
		fputs("SendPacket(). Payload->Enqueue. Size Full\n", stdout);

		//LeaveCriticalSection(&cs);
		NowSession->m_SendBuff.LeaveLOCK();	 // ������ ---------------------------
		return false;
	}
	
	NowSession->m_SendBuff.LeaveLOCK();	 // ������ ---------------------------

	//LeaveCriticalSection(&cs);

	return true;

}

// ���� ������ ������ WSASend() �ϱ�
bool SendPost(stSession* NowSession)
{
	NowSession->m_SendBuff.EnterLOCK();     // �� ----------------------------------

	if (NowSession->m_SendBuff.GetUseSize() == 0)
	{
		NowSession->m_SendBuff.LeaveLOCK();		// �� ���� ----------------------------------
		return false;
	}

	// ------------------
	// send ���� �������� üũ
	// ------------------
	// SendFlag(1������)�� 0(3������)�� ���ٸ�, SendFlag(1������)�� 1(2������)���� ����
	DWORD Dest = (DWORD)InterlockedCompareExchange64((volatile LONG64*)&NowSession->m_SendFlag, TRUE, FALSE);

	// ���� ������� �ʾҴٸ�, (��, ���⼭�� NowSession->m_SendFlag�� �̹� 1(���� ��)�̾��ٴ� ��) ����
	if (Dest == NowSession->m_SendFlag)
	{
		NowSession->m_SendBuff.LeaveLOCK();		// �� ���� ----------------------------------
		//LeaveCriticalSection(&cs);
		return false;
	}	
	// ------------------
	// ���� ���¶�� send���� ź��.
	// ------------------
	WSABUF wsabuf[2];
	int wsabufCount = 0;

	// 1. ���� �������� �������� ���� ������
	char* sendbuff = NowSession->m_SendBuff.GetBufferPtr();

	// 2. SendBuff�� �����Ͱ� �ִ��� Ȯ��
	if (NowSession->m_SendBuff.GetUseSize() == 0)
	{
		// WSASend �Ȱɾ��� ������, ���� ���� ���·� �ٽ� ����.
		// SendFlag(1������)�� 1(3������)�� ���ٸ�, SendFlag(1������)�� 0(2������)���� ����
		InterlockedCompareExchange64((volatile LONG64*)&NowSession->m_SendFlag, FALSE, TRUE);

		NowSession->m_SendBuff.LeaveLOCK();		// �� ���� ----------------------------------
		//LeaveCriticalSection(&cs);
		return false;
	}		

	// 3. WSABUF�� 2���� �����Ѵ�.		
	int Size = NowSession->m_SendBuff.GetNotBrokenGetSize();
	int UseSize = NowSession->m_SendBuff.GetUseSize();

	// 4. 1ĭ �̵��� front ��ġ �˱�(���� front ��ġ�� �̵������� ����)	
	int *front = (int*)NowSession->m_SendBuff.GetReadBufferPtr();
	int TempFront = NowSession->m_SendBuff.NextIndex(*front, 1);

	if (Size <  UseSize)
	{	
		wsabuf[0].buf = &sendbuff[TempFront];
		wsabuf[0].len = Size;

		wsabuf[1].buf = &sendbuff[0];
		wsabuf[1].len = UseSize - Size;
		wsabufCount = 2;				
	}

	// 4-2. �װ� �ƴ϶��, WSABUF�� 1���� �����Ѵ�.
	else
	{
		wsabuf[0].buf = &sendbuff[TempFront];
		wsabuf[0].len = Size;

		wsabufCount = 1;
	}	

	// 5. Overlapped ����ü �ʱ�ȭ
	ZeroMemory(&NowSession->m_SendOverlapped, sizeof(NowSession->m_SendOverlapped));

	// 7. WSASend()
	DWORD SendBytes, flags = 0;
	InterlockedIncrement64((volatile LONG64*)&NowSession->m_IOCount);
	int retval = WSASend(NowSession->m_Client_sock, wsabuf, wsabufCount, &SendBytes, flags, &NowSession->m_SendOverlapped, NULL);	

	// 8. ���ϰ��� 0�̰�, ����Ʈ�� 0�̸� ��������.
	if (&NowSession->m_RecvOverlapped == nullptr && SendBytes == 0)
	{
		// I/Oī��Ʈ�� 0�̶��, ���� ����
		int Nowval = InterlockedDecrement64((volatile LONG64*)&NowSession->m_IOCount);
		if (Nowval < 0)
			IOCountMinusCount++;

		if (Nowval == 0)
		{
			Disconnect(NowSession);
			NowSession->m_SendBuff.LeaveLOCK();		// �� ���� ---------------------------
			return true;
		}
	}

	// 9. ���� ó��
	if (retval == SOCKET_ERROR)
	{
		int Error = WSAGetLastError();

		// �񵿱� ������� ���۵Ȱ� �ƴ϶��
		if (Error != WSA_IO_PENDING)
		{
			// I/Oī��Ʈ 1 ����.
			// I/Oī��Ʈ�� 0�̶��, ���� ����
			int Nowval = InterlockedDecrement64((volatile LONG64*)&NowSession->m_IOCount);
			if (Nowval < 0)
				IOCountMinusCount++;

			if (Nowval == 0)
			{
				NowSession->m_SendBuff.LeaveLOCK();		// �� ���� ---------------------------
				Disconnect(NowSession);
				return true;
			}

			if(Error != WSAESHUTDOWN && Error != WSAECONNRESET)
				printf("Send %d\n", Error);

			// �ٵ�, ���� �����̶��
			if (Error == WSAENOBUFS)
			{
				// ȭ�鿡 ���ۺ��� ��� 
				_tprintf(L"[TCP ����] WSASend() �� ���� ������ ���� : IP �ּ�=%s, ��Ʈ=%d\n", NowSession->m_IP, NowSession->m_prot);
			}	

			printf("Send return false %d\n", Error);

			NowSession->m_SendBuff.LeaveLOCK();		// �� ���� ---------------------------
			return false;
						
		}
	}	

	NowSession->m_SendBuff.LeaveLOCK();		// �� ���� ---------------------------
	return true;	
}