#include "stdafx.h"
#include "NetworkFunc.h"


SRWLOCK g_Session_map_srwl;
map<SOCKET, stSession*> map_Session;

extern long IOCountMinusCount;
extern long JoinUser;
extern long SessionDeleteCount;
long TempCount = 0;


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

	InterlockedIncrement(&SessionDeleteCount);	

	InterlockedDecrement(&JoinUser);	
}



// ------------
// ���ú� ���� �Լ���
// ------------
// RecvProc �Լ�. ť�� ���� üũ �� PacketProc���� �ѱ��.
bool RecvProc(stSession* NowSession)
{
	// -----------------
	// Recv ť ���� ó��
	// -----------------

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

			return false;
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

			return false;
		}
		PayloadBuff.MoveWritePos(DequeueSize);

		// 9. ����� ����ִ� Ÿ�Կ� ���� �б�ó��. false�� ���ϵǸ� ���� ����.
		if (PacketProc(NowSession, &PayloadBuff) == false)
		{
			fputs("PacketProc. return false\n", stdout);			
			return false;
		}
	}

	return true;
}

// Accept�� RecvProc�Լ�
bool RecvPost_Accept(stSession* NowSession)
{
	// ------------------
	// �񵿱� ����� ����
	// ------------------
	// 1. WSABUF ���� �� Recv������ ���ɱ�
	WSABUF wsabuf[2];

	// 2. recv ������ ������ ������.
	char* recvBuff = NowSession->m_RecvBuff.GetBufferPtr();

	// 3. WSABUF ����.
	int wsabufCount = 0;

	int FreeSize = NowSession->m_RecvBuff.GetFreeSize();
	int Size = NowSession->m_RecvBuff.GetNotBrokenPutSize();

	if (Size < FreeSize)
	{
		wsabuf[0].buf = &recvBuff[NowSession->m_RecvBuff.GetWriteBufferPtr_Next()];
		wsabuf[0].len = Size;

		wsabuf[1].buf = &recvBuff[0];
		wsabuf[1].len = FreeSize - Size;
		wsabufCount = 2;

	}

	// 4. �װ� �ƴ϶��, WSABUF�� 1���� �����Ѵ�.
	else
	{
		wsabuf[0].buf = &recvBuff[NowSession->m_RecvBuff.GetWriteBufferPtr_Next()];
		wsabuf[0].len = Size;

		wsabufCount = 1;
	}

	// 5. Overlapped ����ü �ʱ�ȭ
	ZeroMemory(&NowSession->m_RecvOverlapped, sizeof(NowSession->m_RecvOverlapped));

	// 6. WSARecv()
	DWORD recvBytes = 0, flags = 0;
	InterlockedIncrement(&NowSession->m_IOCount);
	int retval = WSARecv(NowSession->m_Client_sock, wsabuf, wsabufCount, &recvBytes, &flags, &NowSession->m_RecvOverlapped, NULL);


	// 7. ���� ó��
	if (retval == SOCKET_ERROR)
	{
		int Error = WSAGetLastError();

		// �񵿱� ������� ���۵Ȱ� �ƴ϶��
		if (Error != WSA_IO_PENDING)
		{
			// I/Oī��Ʈ 1����.
			long Nowval = InterlockedDecrement(&NowSession->m_IOCount);

			// I/O ī��Ʈ�� 0�̶�� ���� ����.
			if(Nowval == 0)
				Disconnect(NowSession);

			// I/Oī��Ʈ�� ���̳ʽ��� �ȴٸ� ���̳ʽ� ī��Ʈ ����.
			if (Nowval < 0)
				InterlockedIncrement(&IOCountMinusCount);

			if (Error != WSAECONNRESET && Error != WSAESHUTDOWN)
				printf("Recv return false1111 %d\n", Error);

			// �ٵ�, ���� �����̶��
			else if (Error == WSAENOBUFS)
			{
				// ȭ�鿡 ���ۺ��� ��� 
				_tprintf(L"[TCP ����] ���� ������ ���� : IP �ּ�=%s, ��Ʈ=%d\n", NowSession->m_IP, NowSession->m_prot);
			}

			return false;
		}
	}

	return true;


}

// RecvPost �Լ�. �񵿱� ����� ����
bool RecvPost(stSession* NowSession)
{
	// ------------------
	// �񵿱� ����� ����
	// ------------------
	// 1. WSABUF ���� �� Recv������ ���ɱ�
	WSABUF wsabuf[2];

	// 2. recv ������ ������ ������.
	char* recvBuff = NowSession->m_RecvBuff.GetBufferPtr();		

	// 3. WSABUF ����.
	int wsabufCount = 0;	

	int FreeSize = NowSession->m_RecvBuff.GetFreeSize();
	int Size = NowSession->m_RecvBuff.GetNotBrokenPutSize();

	if (Size < FreeSize)
	{
		wsabuf[0].buf = &recvBuff[NowSession->m_RecvBuff.GetWriteBufferPtr_Next()];
		wsabuf[0].len = Size;

		wsabuf[1].buf = &recvBuff[0];
		wsabuf[1].len = FreeSize - Size;
		wsabufCount = 2;

	}

	// 4. �װ� �ƴ϶��, WSABUF�� 1���� �����Ѵ�.
	else
	{
		wsabuf[0].buf = &recvBuff[NowSession->m_RecvBuff.GetWriteBufferPtr_Next()];
		wsabuf[0].len = Size;

		wsabufCount = 1;
	}

	// 5. Overlapped ����ü �ʱ�ȭ
	ZeroMemory(&NowSession->m_RecvOverlapped, sizeof(NowSession->m_RecvOverlapped));

	// 6. WSARecv()
	DWORD recvBytes = 0 , flags = 0;
	InterlockedIncrement(&NowSession->m_IOCount);
	int retval = WSARecv(NowSession->m_Client_sock, wsabuf, wsabufCount, &recvBytes, &flags, &NowSession->m_RecvOverlapped, NULL);	++NowSession->m_IOR;
	
	// 7. ���� ó��
	if (retval == SOCKET_ERROR)
	{
		int Error = WSAGetLastError();

		// �񵿱� ������� ���۵Ȱ� �ƴ϶��
		if (Error != WSA_IO_PENDING)
		{
			// I/Oī��Ʈ 1����.
			int Nowval = InterlockedDecrement(&NowSession->m_IOCount);

			// I/Oī��Ʈ�� ���̳ʽ��� �ȴٸ� ���̳ʽ� ī��Ʈ ����.
			if (Nowval < 0)
				InterlockedIncrement(&IOCountMinusCount);

			if (Error != WSAECONNRESET && Error != WSAESHUTDOWN)
				printf("Recv return false1111 %d\n", Error);		

			// �ٵ�, ���� �����̶��
			else if (Error == WSAENOBUFS)
			{
				// ȭ�鿡 ���ۺ��� ��� 
				_tprintf(L"[TCP ����] ���� ������ ���� : IP �ּ�=%s, ��Ʈ=%d\n", NowSession->m_IP, NowSession->m_prot);
			}

			//printf("Recv return false2222, %d\n", Error);

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

	// 4. ���� ��Ŷ �����
	/*CProtocolBuff Header;
	CProtocolBuff Payload_2;
	Send_Packet_Echo(&Header, &Payload_2, Text, Size);*/
	CProtocolBuff Buff;
	Send_Packet_Echo2(&Buff, Size, Text, Size);

	delete[] Text;

	// 5. ���� ��Ŷ�� SendBuff�� �ֱ�
	//if (SendPacket(NowSession, &Header, &Payload_2) == false)
	//	return false;	

	if (SendPacket2(NowSession, &Buff) == false)
		return false;

	// 6. ������ �����ϱ�.
	if (SendPost(NowSession, true) == false)
		return false;

	return true;
}



// ------------
// ��Ŷ ����� �Լ���
// ------------
// ��� �����

//void Send_Packet_Header(WORD PayloadSize, CProtocolBuff* Header)
//{
//	*Header << PayloadSize;
//}
//
//// ������Ŷ �����
//void Send_Packet_Echo(CProtocolBuff* header, CProtocolBuff* payload, char* RetrunText, int RetrunTextSize)
//{
//	// 1. ���̷ε� ����
//	// ������Ŷ��, ���ڷ� ���� retrunText�� payload�� �ִ� ��Ȱ.
//	memcpy_s(payload->GetBufferPtr(), payload->GetFreeSize(), RetrunText, RetrunTextSize);
//	payload->MoveWritePos(RetrunTextSize);
//
//	// 2. ��� ����
//	Send_Packet_Header(payload->GetUseSize(), header);
//}

// ������Ŷ �����2
void Send_Packet_Echo2(CProtocolBuff* Buff, WORD PayloadSize, char* RetrunText, int RetrunTextSize)
{
	// 1. ��� ����
	// ���� �����Ұ� ����. ���ڷι��� PayloadSize�� �� �����

	// 2. ��� �ֱ�
	*Buff << PayloadSize;

	// 3. ���̷ε� ���� �� �ֱ�
	memcpy_s(Buff->GetBufferPtr() + Buff->GetUseSize(), Buff->GetFreeSize(), RetrunText, RetrunTextSize);
	Buff->MoveWritePos(RetrunTextSize);
}





// ------------
// ���� ���� �Լ���
// ------------
// ���� ���ۿ� ������ �ֱ�
//
//bool SendPacket(stSession* NowSession, CProtocolBuff* header, CProtocolBuff* payload)
//{
//	// 1. ť ��
//	//NowSession->m_SendBuff.EnterLOCK();		// �� ---------------------------	
//
//	// 2. ���� q�� ��� �ֱ�
//	int Size = dfNETWORK_PACKET_HEADER_SIZE;
//	int EnqueueCheck = NowSession->m_SendBuff.Enqueue(header->GetBufferPtr(), Size);	
//	if (EnqueueCheck == -1)
//	{		
//		fputs("SendPacket(). Header->Enqueue. Size Full\n", stdout);
//
//		//NowSession->m_SendBuff.LeaveLOCK();	 // ������ ---------------------------
//		return false;
//	}
//
//	// 3. ���� q�� ���̷ε� �ֱ�
//	int PayloadLen = payload->GetUseSize();
//	EnqueueCheck = NowSession->m_SendBuff.Enqueue(payload->GetBufferPtr(), PayloadLen);	
//	if (EnqueueCheck == -1)
//	{		
//		fputs("SendPacket(). Payload->Enqueue. Size Full\n", stdout);
//
//		//NowSession->m_SendBuff.LeaveLOCK();	 // ������ ---------------------------
//		return false;
//	}
//	
//	//NowSession->m_SendBuff.LeaveLOCK();	 // ������ ---------------------------
//
//	return true;
//}

// ���� ���ۿ� ������ �ֱ�2
bool SendPacket2(stSession* NowSession, CProtocolBuff* Buff)
{
	// 1. ť ��
	

	// ������ �ֱ�
	int Size = Buff->GetUseSize();

	//NowSession->m_SendBuff.EnterLOCK();		// �� ---------------------------	
	int EnqueueCheck = NowSession->m_SendBuff.Enqueue(Buff->GetBufferPtr(), Size);
	if (EnqueueCheck == -1)
	{
		fputs("SendPacket(). Enqueue. Size Full\n", stdout);

		//NowSession->m_SendBuff.LeaveLOCK();	 // ������ ---------------------------
		return false;
	}	

	//NowSession->m_SendBuff.LeaveLOCK();	 // ������ ---------------------------

	return true;
}

// ���� ������ ������ WSASend() �ϱ�
bool SendPost(stSession* NowSession, bool _bR)
{
	while (1)
	{
		// ------------------
		// send ���� �������� üũ
		// ------------------
		// SendFlag(1������)�� 0(3������)�� ���ٸ�, SendFlag(1������)�� 1(2������)���� ����
		// ���⼭ TRUE�� ���ϵǴ� ����, �̹� NowSession->m_SendFlag�� 1(���� ��)�̾��ٴ� ��.
		if (InterlockedCompareExchange(&NowSession->m_SendFlag, TRUE, FALSE) == TRUE)
		{
			//NowSession->m_SendBuff.LeaveLOCK();
			return true;
		}

		// ------------------
		// ���� ���¶�� send���� ź��.
		// ------------------
		//NowSession->m_SendBuff.EnterLOCK();     // �� ----------------------------------		

		// 1. SendBuff�� �����Ͱ� �ִ��� Ȯ��
		if (NowSession->m_SendBuff.GetUseSize() == 0)
		{
			// WSASend �Ȱɾ��� ������, ���� ���� ���·� �ٽ� ����.
			// SendFlag(1������)�� 1(3������)�� ���ٸ�, SendFlag(1������)�� 0(2������)���� ����
			//NowSession->m_SendFlag = 0;
			//InterlockedExchange(&NowSession->m_SendFlag, FALSE);
			InterlockedCompareExchange(&NowSession->m_SendFlag, FALSE, TRUE);


			// ���⼭ ������ �ѹ� �� üũ�Ѵ�. �׷��� ������ ������ goto������ ���� �ö󰡼� �ѹ� �� �Ѵ�.
			if (NowSession->m_SendBuff.GetUseSize() > 0)
			{
				//NowSession->m_SendBuff.LeaveLOCK();
				continue;
			}

			//NowSession->m_SendBuff.LeaveLOCK();		// �� ���� ----------------------------------
			break;
		}

		// 2. ���� �������� �������� ���� ������
		char* sendbuff = NowSession->m_SendBuff.GetBufferPtr();
		WSABUF wsabuf[2];
		int wsabufCount = 0;

		// 3. WSABUF�� 2���� �����Ѵ�.	

		//NowSession->m_SendBuff.EnterLOCK();//-----------------

		int UseSize = NowSession->m_SendBuff.GetUseSize();
		int BrokenSize = NowSession->m_SendBuff.GetNotBrokenGetSize();

		//NowSession->m_SendBuff.LeaveLOCK();//-----------------

		int iValue;
		int iIdx = 0;
		if (BrokenSize <  UseSize)
		{
			iIdx = NowSession->m_SendBuff.GetReadBufferPtr_Next();
			wsabuf[0].buf = &sendbuff[iIdx];
			wsabuf[0].len = BrokenSize;

			iValue = UseSize - BrokenSize;
			wsabuf[1].buf = &sendbuff[0];
			wsabuf[1].len = iValue;// UseSize - Size;
			wsabufCount = 2;
		}

		// 4-2. �װ� �ƴ϶��, WSABUF�� 1���� �����Ѵ�.
		else
		{
			iIdx = NowSession->m_SendBuff.GetReadBufferPtr_Next();
			wsabuf[0].buf = &sendbuff[iIdx];
			wsabuf[0].len = BrokenSize;

			wsabufCount = 1;
		}

		// 5. Overlapped ����ü �ʱ�ȭ
		ZeroMemory(&NowSession->m_SendOverlapped, sizeof(NowSession->m_SendOverlapped));

		// 7. WSASend()
		DWORD SendBytes = 0, flags = 0;
		InterlockedIncrement(&NowSession->m_IOCount);
		//wsabuf[0].len = -10;
		int retval = WSASend(NowSession->m_Client_sock, wsabuf, wsabufCount, &SendBytes, flags, &NowSession->m_SendOverlapped, NULL);

		//NowSession->m_SendBuff.LeaveLOCK();		// �� ���� ---------------------------	

		// 8. ���� ó��
		if (retval == SOCKET_ERROR)
		{
			int Error = WSAGetLastError();

			// �񵿱� ������� ���۵Ȱ� �ƴ϶��
			if (Error != WSA_IO_PENDING)
			{
				// I/Oī��Ʈ 1 ����.

				int Nowval = InterlockedDecrement(&NowSession->m_IOCount);

				// I/Oī��Ʈ�� ���̳ʽ���� ���̳ʽ� ī��Ʈ ����
				if (Nowval < 0)
					InterlockedIncrement(&IOCountMinusCount);

				if (Error != WSAESHUTDOWN && Error != WSAECONNRESET)
					printf("Send %d\n", Error);

				// ���� �����̶��
				else if (Error == WSAENOBUFS)
				{
					// ȭ�鿡 ���ۺ��� ��� 
					_tprintf(L"[TCP ����] WSASend() �� ���� ������ ���� : IP �ּ�=%s, ��Ʈ=%d\n", NowSession->m_IP, NowSession->m_prot);
				}

				//printf("Send return false %d\n", Error);	

				return false;
			}
		}
		break;
	}

	return true;
}