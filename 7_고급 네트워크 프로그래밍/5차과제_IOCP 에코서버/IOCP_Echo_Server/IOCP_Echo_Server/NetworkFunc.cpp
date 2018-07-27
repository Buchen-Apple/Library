#include "stdafx.h"
#include "NetworkFunc.h"

SRWLOCK g_Session_map_srwl;
map<SOCKET, stSession*> map_Session;



// ���� ����
void Disconnect(stSession* DeleteSession)
{
	// �� �Լ��� ȣ��Ǿ��� ��, ������ I/O Count�� 0�̶�� �����Ѵ�.
	if (DeleteSession->m_IOCount > 0)
		return;

	//_tprintf(L"[TCP ����] ���� ���� : IP �ּ�=%s, ��Ʈ=%d\n", DeleteSession->m_IP, DeleteSession->m_prot);

	SOCKET TempSock = DeleteSession->m_Client_sock;

	// Ŭ���� ����, ���� ��������	
	closesocket(DeleteSession->m_Client_sock);
	delete DeleteSession;

	// map���� ���ܽ�Ű��
	LockSession();
	map_Session.erase(TempSock);
	UnlockSession();
}



// ------------
// ���ú� ���� �Լ���
// ------------
// RecvProc �Լ�. ť�� ���� üũ �� PacketProc���� �ѱ��.
void RecvProc(stSession* NowSession, DWORD cbTransferred)
{
	if (NowSession == nullptr)
		return;

	// -----------------
	// Recv ť ���� ó��
	// -----------------
	// 1. ť ��
	//NowSession->m_RecvBuff.EnterLOCK();		// �� ----------------------

	// 2. ���� ����Ʈ��ŭ rear �̵�
	NowSession->m_RecvBuff.MoveWritePos(cbTransferred);

	while (1)
	{
		// 1. RecvBuff�� �ּ����� ����� �ִ��� üũ. (���� = ��� ������� ���ų� �ʰ�. ��, �ϴ� �����ŭ�� ũ�Ⱑ �ִ��� üũ)	
		stNETWORK_PACKET_HEADE HeaderBuff;

		// RecvBuff�� ��� ���� ���� ũ�Ⱑ ��� ũ�⺸�� �۴ٸ�, �Ϸ� ��Ŷ�� ���ٴ� ���̴� while�� ����.
		if (NowSession->m_RecvBuff.GetUseSize() < dfNETWORK_PACKET_HEADER_SIZE)
		{
			break;
		}

		// 2. ����� Peek���� Ȯ���Ѵ�.  Peek �ȿ�����, ��� �ؼ����� len��ŭ �д´�. 
		// ���۰� ��������� ���� ����.
		if (NowSession->m_RecvBuff.Peek((char*)&HeaderBuff, dfNETWORK_PACKET_HEADER_SIZE) == -1)
		{
			fputs("WorkerThread. Recv->Header Peek : Buff Empty\n", stdout);
			shutdown(NowSession->m_Client_sock, SD_BOTH);

			break;
		}

		// 3. �ϼ��� ��Ŷ�� �ִ��� Ȯ��. (�ϼ� ��Ŷ ������ = ��� ������ + ���̷ε� Size + �Ϸ��ڵ�)
		// ��� ���, �ϼ� ��Ŷ ����� �ȵǸ� while�� ����.
		if (NowSession->m_RecvBuff.GetUseSize() < (dfNETWORK_PACKET_HEADER_SIZE + HeaderBuff.bySize + 1))
		{
			break;
		}

		// 4. ��� �ڵ� Ȯ��
		if (HeaderBuff.byCode != dfNETWORK_PACKET_CODE)
		{
			_tprintf(_T("RecvProc(). �Ϸ���Ŷ ��� ó�� �� PacketCodeƲ��. ���� ����\n"));
			shutdown(NowSession->m_Client_sock, SD_BOTH);

			break;
		}

		// 5. RecvBuff���� Peek�ߴ� ����� ����� (�̹� Peek������, �׳� Remove�Ѵ�)
		NowSession->m_RecvBuff.RemoveData(dfNETWORK_PACKET_HEADER_SIZE);

		// 6. RecvBuff���� ���̷ε� Size ��ŭ ���̷ε� ����ȭ ���۷� �̴´�. (��ť�̴�. Peek �ƴ�)
		CProtocolBuff PayloadBuff;
		DWORD PayloadSize = HeaderBuff.bySize;

		int DequeueSize = NowSession->m_RecvBuff.Dequeue(PayloadBuff.GetBufferPtr(), PayloadSize);

		// ���۰� ��������� ���� ����
		if (DequeueSize == -1)
		{
			fputs("WorkerThread. Recv->Payload Dequeue : Buff Empty\n", stdout);
			shutdown(NowSession->m_Client_sock, SD_BOTH);

			break;
		}
		PayloadBuff.MoveWritePos(DequeueSize);

		// 7. RecvBuff���� �����ڵ� 1Byte����.	(��ť�̴�. Peek �ƴ�)
		BYTE EndCode;
		DequeueSize = NowSession->m_RecvBuff.Dequeue((char*)&EndCode, 1);
		if (DequeueSize == -1)
		{
			fputs("WorkerThread. Recv->EndCode Dequeue : Buff Empty\n", stdout);
			shutdown(NowSession->m_Client_sock, SD_BOTH);

			break;
		}

		// 8. �����ڵ� Ȯ��
		if (EndCode != dfNETWORK_PACKET_END)
		{
			_tprintf(_T("RecvProc(). �Ϸ���Ŷ �����ڵ� ó�� �� ������Ŷ �ƴ�. ���� ����\n"));
			shutdown(NowSession->m_Client_sock, SD_BOTH);

			break;
		}


		// 9. ����� ����ִ� Ÿ�Կ� ���� �б�ó��. false�� ���ϵǸ� ���� ����.
		if (PacketProc(HeaderBuff.byType, NowSession, &PayloadBuff) == false)
		{
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

	// 3. �� ���� �� �� �ִ� �������� ���� ���� ���
	int Size = NowSession->m_RecvBuff.GetNotBrokenPutSize();

	// 4. 1ĭ �̵��� rear ��ġ �˱�(���� rear ��ġ�� �̵������� ����)	
	int* rear = (int*)NowSession->m_RecvBuff.GetWriteBufferPtr();
	int TempRear = NowSession->m_RecvBuff.NextIndex(*rear, 1);

	// 5-1. ���� Size�� free������� �۴ٸ�, WSABUF�� 2���� �����Ѵ�.
	int wsabufCount = 0;
	int FreeSize = NowSession->m_RecvBuff.GetFreeSize();
	if (Size < FreeSize)
	{
		wsabuf[0].buf = &recvBuff[TempRear];
		wsabuf[0].len = Size;

		wsabuf[1].buf = &recvBuff[0];
		wsabuf[1].len = FreeSize - Size;
		wsabufCount = 2;
	}

	// 5-2. �װ� �ƴ϶��, WSABUF�� 1���� �����Ѵ�.
	else
	{
		wsabuf[0].buf = &recvBuff[TempRear];
		wsabuf[0].len = Size;

		wsabufCount = 1;
	}

	// 6. Overlapped ����ü �ʱ�ȭ
	ZeroMemory(&NowSession->m_RecvOverlapped, sizeof(NowSession->m_RecvOverlapped));

	// 7. WSARecv()
	DWORD recvBytes, flags = 0;
	InterlockedIncrement64((LONG64*)&NowSession->m_IOCount);
	int retval = WSARecv(NowSession->m_Client_sock, wsabuf, wsabufCount, &recvBytes, &flags, &NowSession->m_RecvOverlapped, NULL);

	//NowSession->m_RecvBuff.LeaveLOCK();   // �� ���� ---------------------------

	 // 8. ���ϰ��� 0�̰�, ����Ʈ�� 0�̸� ��������.
	if (retval == 0 && recvBytes == 0)
	{
		// I/Oī��Ʈ�� 0�̶��, ���� ����
		int a = InterlockedDecrement64((LONG64*)&NowSession->m_IOCount);
		if (a == 0)
		{
			Disconnect(NowSession);
		}
	}

	// 9. ���� ó��
	if (retval == SOCKET_ERROR)
	{
		int Error = WSAGetLastError();

		// �񵿱� ������� ���۵Ȱ� �ƴ϶��
		if (Error != WSA_IO_PENDING)
		{
			// I/Oī��Ʈ�� 0�̶��, ���� ����
			int a = InterlockedDecrement64((LONG64*)&NowSession->m_IOCount);
			if (a == 0)
			{
				Disconnect(NowSession);
			}

			if(Error != WSAECONNRESET && Error != WSAESHUTDOWN)
				printf("Recv %d\n", Error);

			// �ٵ�, ���� �����̶��
			if (Error == WSAENOBUFS)
			{
				// ȭ�鿡 ���ۺ��� ��� 
				_tprintf(L"[TCP ����] ���� ������ ���� : IP �ּ�=%s, ��Ʈ=%d\n", NowSession->m_IP, NowSession->m_prot);
			}			

			return false;
		}
	}

	return true;
}

// ��Ŷ ó��. RecvProc()���� ���� ��Ŷ ó��.
bool PacketProc(BYTE PacketType, stSession* NowSession, CProtocolBuff* Payload)
{

	switch (PacketType)
	{
		// ���ڿ� ���� ��Ŷ
	case dfPACKET_CS_ECHO:
	{
		if (Recv_Packet_Echo(NowSession, Payload) == false)
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
// ��Ŷ ó�� �Լ���
// ------------
// ���� ��Ŷ ó�� �Լ�
bool Recv_Packet_Echo(stSession* NowSession, CProtocolBuff* Payload)
{
	// 1. ���̷ε� ������� �뷮 ���
	int Size = Payload->GetUseSize();

	// 2. �� �����ŭ ������ memcpy
	char* Text = new char[Size + 1];
	memcpy_s(Text, Size + 1, Payload->GetBufferPtr(), Size);

	// 3. ī�� �� ��ŭ payload�� front�̵�
	Payload->MoveReadPos(Size);

	// 4. ���ڿ� ����� ���� ������ ��ġ�� NULL �ֱ�
	Text[Size] = '\0';

	// 5. ���ڿ� ���
	LockSession();
	_tprintf(L"[%s:%d] ���� ������(User : %zd) : ", NowSession->m_IP, NowSession->m_prot, map_Session.size());
	printf("%s\n", Text);
	UnlockSession();

	// 6. ���� ������ �ٽ� �����ϱ�.
	// ���� ��Ŷ �����
	CProtocolBuff Header;
	CProtocolBuff Payload_2;
	Send_Packet_Echo(&Header, &Payload_2, Text, Size);

	// ���� ��Ŷ�� SendBuff�� �ֱ�
	if (SendPacket(NowSession, &Header, &Payload_2) == false)
		return false;

	// ������ �����ϱ�. Send���� �ƴ� ��쿡��...
	SendPost(NowSession);

	delete Text;
	return true;
}



// ------------
// ��Ŷ ����� �Լ���
// ------------
// ��� �����
void Send_Packet_Header(BYTE Type, BYTE PayloadSize, CProtocolBuff* Header)
{
	// ��Ŷ �ڵ� (1Byte)
	*Header << dfNETWORK_PACKET_CODE;

	// ���̷ε� ������ (1Byte)
	*Header << PayloadSize;

	// ��Ŷ Ÿ�� (1Byte)
	*Header << Type;

	// ������ (1Byte)
	BYTE temp = 12;
	*Header << temp;

}

// ������Ŷ �����
void Send_Packet_Echo(CProtocolBuff* header, CProtocolBuff* payload, char* RetrunText, int RetrunTextSize)
{
	// 1. ���̷ε� ����
	// ������Ŷ��, ���ڷ� ���� retrunText�� payload�� �ִ� ��Ȱ.
	memcpy_s(payload->GetBufferPtr(), payload->GetFreeSize(), RetrunText, RetrunTextSize);
	payload->MoveWritePos(RetrunTextSize);

	// 2. ��� ����
	Send_Packet_Header(dfPACKET_SC_ECHO, payload->GetUseSize(), header);
}





// ------------
// ���� ���� �Լ���
// ------------
// ���� ���ۿ� ������ �ֱ�
bool SendPacket(stSession* NowSession, CProtocolBuff* header, CProtocolBuff* payload)
{
	// 1. ť ��
	//NowSession->m_SendBuff.EnterLOCK();		// �� ---------------------------

	// 2. ���� q�� ��� �ֱ�
	int Size = dfNETWORK_PACKET_HEADER_SIZE;
	int EnqueueCheck = NowSession->m_SendBuff.Enqueue(header->GetBufferPtr(), Size);
	if (EnqueueCheck == -1)
	{		
		fputs("SendPacket(). Header->Enqueue. Size Full\n", stdout);

		//NowSession->m_SendBuff.LeaveLOCK();	 // ������ ---------------------------
		return false;
	}

	// 3. ���� q�� ���̷ε� �ֱ�
	int PayloadLen = payload->GetUseSize();
	EnqueueCheck = NowSession->m_SendBuff.Enqueue(payload->GetBufferPtr(), PayloadLen);
	if (EnqueueCheck == -1)
	{		
		fputs("SendPacket(). Payload->Enqueue. Size Full\n", stdout);

		//NowSession->m_SendBuff.LeaveLOCK();	 // ������ ---------------------------
		return false;
	}

	// 3. ���� q�� �����ڵ� �ֱ�
	char EndCode = dfNETWORK_PACKET_END;
	EnqueueCheck = NowSession->m_SendBuff.Enqueue(&EndCode, 1);
	if (EnqueueCheck == -1)
	{
		fputs("SendPacket(). EndCode->Enqueue. Size Full\n", stdout);

		//NowSession->m_SendBuff.LeaveLOCK();	 // ������ ---------------------------
		return false;
	}
	
	//NowSession->m_SendBuff.LeaveLOCK();	 // ������ ---------------------------

	return true;

}

// ���� ������ ������ WSASend() �ϱ�
void SendPost(stSession* NowSession)
{
	// ------------------
	// send ���� �������� üũ
	// ------------------
	// SendFlag(1������)�� 0(3������)�� ���ٸ�, SendFlag(1������)�� 1(2������)���� ����
	DWORD Dest = (DWORD)InterlockedCompareExchange64((LONG64*)&NowSession->m_SendFlag, TRUE, FALSE);

	// ���� ������� �ʾҴٸ�, (��, ���⼭�� NowSession->m_SendFlag�� �̹� 1(���� ��)�̾��ٴ� ��) ����
	if (Dest  == NowSession->m_SendFlag)
		return;

	// ------------------
	// ���� ���¶�� send���� ź��.
	// ------------------
	WSABUF wsabuf[2];

	// 1. ���� �������� �������� ���� ������
	char* sendbuff = NowSession->m_SendBuff.GetBufferPtr();

	// 2. SendBuff�� �����Ͱ� �ִ��� Ȯ��
	if (NowSession->m_SendBuff.GetUseSize() == 0)
	{
		// WSASend �Ȱɾ��� ������, ���� ���� ���·� �ٽ� ����.
		// SendFlag(1������)�� 1(3������)�� ���ٸ�, SendFlag(1������)�� 0(2������)���� ����
		InterlockedCompareExchange64((LONG64*)&NowSession->m_SendFlag, FALSE, TRUE);
		return;
	}

	// 3 �� ���� ���� �� �ִ� �������� ���� ���� ���
	int Size = NowSession->m_SendBuff.GetNotBrokenGetSize();

	// 4. 1ĭ �̵��� front ��ġ �˱�(���� front ��ġ�� �̵������� ����)	
	int *front = (int*)NowSession->m_SendBuff.GetReadBufferPtr();
	int TempFront = NowSession->m_SendBuff.NextIndex(*front, 1);

	// 5-1. ���� Size�� Use ������� �۴ٸ�, WSABUF�� 2���� �����Ѵ�.
	int wsabufCount = 0;
	int UseSize = NowSession->m_SendBuff.GetUseSize();
	if (Size < UseSize)
	{
		wsabuf[0].buf = &sendbuff[TempFront];
		wsabuf[0].len = Size;

		wsabuf[1].buf = &sendbuff[0];
		wsabuf[1].len = UseSize - Size;
		wsabufCount = 2;
	}

	// 5-2. �װ� �ƴ϶��, WSABUF�� 1���� �����Ѵ�.
	else
	{
		wsabuf[0].buf = &sendbuff[TempFront];
		wsabuf[0].len = Size;

		wsabufCount = 1;
	}

	// 6. Overlapped ����ü �ʱ�ȭ
	ZeroMemory(&NowSession->m_SendOverlapped, sizeof(NowSession->m_SendOverlapped));

	// 7. WSASend()
	DWORD SendBytes, flags = 0;
	InterlockedIncrement64((LONG64*)&NowSession->m_IOCount);
	int retval = WSASend(NowSession->m_Client_sock, wsabuf, wsabufCount, &SendBytes, flags, &NowSession->m_SendOverlapped, NULL);

	// 8. ���� ó��
	if (retval == SOCKET_ERROR)
	{
		int Error = WSAGetLastError();

		// �񵿱� ������� ���۵Ȱ� �ƴ϶��
		if (Error != WSA_IO_PENDING)
		{
			// I/Oī��Ʈ 1 ����.
			// I/Oī��Ʈ�� 0�̶��, ���� ����
			int a = InterlockedDecrement64((LONG64*)&NowSession->m_IOCount);
			if (a == 0)
			{
				Disconnect(NowSession);
			}

			if(Error != WSAESHUTDOWN && Error != WSAECONNRESET)
				printf("Send %d\n", Error);

			// �ٵ�, ���� �����̶��
			if (Error == WSAENOBUFS)
			{
				// ȭ�鿡 ���ۺ��� ��� 
				_tprintf(L"[TCP ����] WSASend() �� ���� ������ ���� : IP �ּ�=%s, ��Ʈ=%d\n", NowSession->m_IP, NowSession->m_prot);
			}

			
		}
	}	
}