#include "stdafx.h"
#include "NetworkFunc.h"


SRWLOCK g_Session_map_srwl;
map<SOCKET, stSession*> map_Session;

extern long IOCountMinusCount;
extern long JoinUser;
extern long SessionDeleteCount;
extern int		g_LogLevel;			// Main���� �Է��� �α� ��� ����. �ܺκ���
extern TCHAR	g_szLogBuff[1024];		// Main���� �Է��� �α� ��¿� �ӽ� ����. �ܺκ���



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

// �α� ��� �Լ�
void Log(TCHAR *szString, int LogLevel)
{
	_tprintf(L"%s", szString);

	// �α� ������ Warning, Error�� ���Ϸ� �����Ѵ�.
	if (LogLevel >= dfLOG_LEVEL_WARNING)
	{
		// ���� �⵵��, �� �˾ƿ���.
		SYSTEMTIME lst;
		GetLocalTime(&lst);

		// ���� �̸� �����
		TCHAR tFileName[30];
		swprintf_s(tFileName, _countof(tFileName), L"%d%02d Log.txt", lst.wYear, lst.wMonth);

		// ���� ����
		FILE *fp;
		_tfopen_s(&fp, tFileName, L"at");

		fwprintf(fp, L"%s", szString);

		fclose(fp);

	}
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
		int UseSize = NowSession->m_RecvBuff.GetUseSize();
		if (UseSize < dfNETWORK_PACKET_HEADER_SIZE)
		{
			break;
		}

		// 2. ����� Peek���� Ȯ���Ѵ�.  Peek �ȿ�����, ��� �ؼ����� len��ŭ �д´�. 
		// ���۰� ��������� ���� ����.
		int PeekSize = NowSession->m_RecvBuff.Peek((char*)&Header_PaylaodSize, dfNETWORK_PACKET_HEADER_SIZE);
		if (PeekSize == -1)
		{
			SYSTEMTIME lst;
			GetLocalTime(&lst);
			_LOG(dfLOG_LEVEL_ERROR, L"[%02d/%02d/%02d %02d:%02d:%02d] RecvProc(). ���->Peek : Buff Empty\n",
				lst.wYear - 2000, lst.wMonth, lst.wDay, lst.wHour, lst.wMinute, lst.wSecond);

			shutdown(NowSession->m_Client_sock, SD_BOTH);

			return false;
		}

		// 3. �ϼ��� ��Ŷ�� �ִ��� Ȯ��. (�ϼ� ��Ŷ ������ = ��� ������ + ���̷ε� Size)
		// ��� ���, �ϼ� ��Ŷ ����� �ȵǸ� while�� ����.
		if (UseSize < (dfNETWORK_PACKET_HEADER_SIZE + Header_PaylaodSize))
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
			SYSTEMTIME lst;
			GetLocalTime(&lst);
			_LOG(dfLOG_LEVEL_ERROR, L"[%02d/%02d/%02d %02d:%02d:%02d] RecvProc(). ���̷ε�->Dequeue : Buff Empty\n",
				lst.wYear - 2000, lst.wMonth, lst.wDay, lst.wHour, lst.wMinute, lst.wSecond);
			
			shutdown(NowSession->m_Client_sock, SD_BOTH);

			return false;
		}
		PayloadBuff.MoveWritePos(DequeueSize);

		// 9. ����� ����ִ� Ÿ�Կ� ���� �б�ó��. false�� ���ϵǸ� ���� ����.
		if (PacketProc(NowSession, &PayloadBuff) == false)		
			return false;

	}

	return true;
}

// Accept�� RecvProc�Լ�
bool RecvPost_Accept(stSession* NowSession)
{
	// ------------------
	// �񵿱� ����� ����
	// ------------------
	// 1. WSABUF ����.
	WSABUF wsabuf[2];
	int wsabufCount = 0;

	int FreeSize = NowSession->m_RecvBuff.GetFreeSize();
	int Size = NowSession->m_RecvBuff.GetNotBrokenPutSize();

	if (Size < FreeSize)
	{
		wsabuf[0].buf = NowSession->m_RecvBuff.GetRearBufferPtr();
		wsabuf[0].len = Size;

		wsabuf[1].buf = NowSession->m_RecvBuff.GetBufferPtr();
		wsabuf[1].len = FreeSize - Size;
		wsabufCount = 2;

	}
	else
	{
		wsabuf[0].buf = NowSession->m_RecvBuff.GetRearBufferPtr();
		wsabuf[0].len = Size;

		wsabufCount = 1;
	}

	// 2. Overlapped ����ü �ʱ�ȭ
	ZeroMemory(&NowSession->m_RecvOverlapped, sizeof(NowSession->m_RecvOverlapped));

	// 3. WSARecv()
	DWORD recvBytes = 0, flags = 0;
	InterlockedIncrement(&NowSession->m_IOCount);
	int retval = WSARecv(NowSession->m_Client_sock, wsabuf, wsabufCount, &recvBytes, &flags, &NowSession->m_RecvOverlapped, NULL);


	// 4. ���� ó��
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
			{
				SYSTEMTIME lst;
				GetLocalTime(&lst);
				_LOG(dfLOG_LEVEL_ERROR, L"[%02d/%02d/%02d %02d:%02d:%02d] RecvPost_Accept(). ���� �߻�. (%d)\n",
					lst.wYear - 2000, lst.wMonth, lst.wDay, lst.wHour, lst.wMinute, lst.wSecond, Error);
			}

			// �ٵ�, ���� �����̶��
			else if (Error == WSAENOBUFS)
			{
				// ȭ�鿡 ���ۺ��� ��� 
				SYSTEMTIME lst;
				GetLocalTime(&lst);
				_LOG(dfLOG_LEVEL_ERROR, L"[%02d/%02d/%02d %02d:%02d:%02d] RecvPost_Accept(). ���� ������ ���� : IP �ּ�=%s, ��Ʈ=%d\n",
					lst.wYear - 2000, lst.wMonth, lst.wDay, lst.wHour, lst.wMinute, lst.wSecond, NowSession->m_IP, NowSession->m_prot);

				shutdown(NowSession->m_Client_sock, SD_BOTH);
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
	// 1. WSABUF ����.
	WSABUF wsabuf[2];
	int wsabufCount = 0;

	int FreeSize = NowSession->m_RecvBuff.GetFreeSize();
	int Size = NowSession->m_RecvBuff.GetNotBrokenPutSize();

	if (Size < FreeSize)
	{
		wsabuf[0].buf = NowSession->m_RecvBuff.GetRearBufferPtr();
		wsabuf[0].len = Size;

		wsabuf[1].buf = NowSession->m_RecvBuff.GetBufferPtr();
		wsabuf[1].len = FreeSize - Size;
		wsabufCount = 2;

	}
	else
	{
		wsabuf[0].buf = NowSession->m_RecvBuff.GetRearBufferPtr();
		wsabuf[0].len = Size;

		wsabufCount = 1;
	}

	// 2. Overlapped ����ü �ʱ�ȭ
	ZeroMemory(&NowSession->m_RecvOverlapped, sizeof(NowSession->m_RecvOverlapped));

	// 3. WSARecv()
	DWORD recvBytes = 0 , flags = 0;
	InterlockedIncrement(&NowSession->m_IOCount);
	int retval = WSARecv(NowSession->m_Client_sock, wsabuf, wsabufCount, &recvBytes, &flags, &NowSession->m_RecvOverlapped, NULL);
	
	// 4. ���� ó��
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
			{
				SYSTEMTIME lst;
				GetLocalTime(&lst);
				_LOG(dfLOG_LEVEL_ERROR, L"[%02d/%02d/%02d %02d:%02d:%02d] RecvPost(). ���� �߻�. (%d)\n",
					lst.wYear - 2000, lst.wMonth, lst.wDay, lst.wHour, lst.wMinute, lst.wSecond, Error);
			}

			// �ٵ�, ���� �����̶��
			else if (Error == WSAENOBUFS)
			{
				// ȭ�鿡 ���ۺ��� ��� 
				SYSTEMTIME lst;
				GetLocalTime(&lst);
				_LOG(dfLOG_LEVEL_ERROR, L"[%02d/%02d/%02d %02d:%02d:%02d] RecvPost(). ���� ������ ���� : IP �ּ�=%s, ��Ʈ=%d\n",
					lst.wYear - 2000, lst.wMonth, lst.wDay, lst.wHour, lst.wMinute, lst.wSecond, NowSession->m_IP, NowSession->m_prot);

				shutdown(NowSession->m_Client_sock, SD_BOTH);
			}

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
	// 1. ���̷ε� ������ ���
	int Size = Payload->GetUseSize();

	// 2. �� �����ŭ ������ memcpy
	char* Text = new char[Size];
	memcpy_s(Text, Size, Payload->GetBufferPtr(), Size);

	// 3. ���� ��Ŷ �����. Buff�ȿ� ������� �� ����.	
	CProtocolBuff Buff;
	Send_Packet_Echo(&Buff, Size, Text, Size);

	delete[] Text;

	// 4. ���� ��Ŷ�� SendBuff�� �ֱ�
	if (SendPacket(NowSession, &Buff) == false)		
		return false;

	// 5. ������ �����ϱ�.
	if (SendPost(NowSession) == false)
		return false;

	return true;
}


// ------------
// ��Ŷ ����� �Լ���
// ------------
// ������Ŷ �����
void Send_Packet_Echo(CProtocolBuff* Buff, WORD PayloadSize, char* RetrunText, int RetrunTextSize)
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
bool SendPacket(stSession* NowSession, CProtocolBuff* Buff)
{
	// 1. ���� ������ ���ϱ�.
	int Size = Buff->GetUseSize();

	// 2. �ֱ�
	int EnqueueCheck = NowSession->m_SendBuff.Enqueue(Buff->GetBufferPtr(), Size);
	if (EnqueueCheck == -1)
	{		
		SYSTEMTIME lst;
		GetLocalTime(&lst);
		_LOG(dfLOG_LEVEL_ERROR, L"[%02d/%02d/%02d %02d:%02d:%02d] SendPacket(). Enqueue : Size Full\n",
			lst.wYear - 2000, lst.wMonth, lst.wDay, lst.wHour, lst.wMinute, lst.wSecond);		

		return false;
	}	

	return true;
}

// ���� ������ ������ WSASend() �ϱ�
bool SendPost(stSession* NowSession)
{
	while (1)
	{
		// ------------------
		// send ���� �������� üũ
		// ------------------
		// 1. SendFlag(1������)�� 0(3������)�� ���ٸ�, SendFlag(1������)�� 1(2������)���� ����
		// ���⼭ TRUE�� ���ϵǴ� ����, �̹� NowSession->m_SendFlag�� 1(���� ��)�̾��ٴ� ��.
		if (InterlockedCompareExchange(&NowSession->m_SendFlag, TRUE, FALSE) == TRUE)
			return true;


		// 2. SendBuff�� �����Ͱ� �ִ��� Ȯ��
		// ���⼭ ���� UseSize�� ���� ������ �̴�. �Ʒ����� ���� �����Ҷ��� ����Ѵ�.
		int UseSize = NowSession->m_SendBuff.GetUseSize();
		if (UseSize == 0)
		{
			// WSASend �Ȱɾ��� ������, ���� ���� ���·� �ٽ� ����.
			NowSession->m_SendFlag = 0;

			// 3. ��¥�� ����� ������ �ٽ��ѹ� üũ. �� Ǯ�� �Դµ�, ���ؽ�Ʈ ����Ī �Ͼ�� �ٸ� �����尡 �ǵ���� ���ɼ�
			// ������ ������ ���� �ö󰡼� �ѹ� �� �õ�
			if (NowSession->m_SendBuff.GetUseSize() > 0)
				continue;

			break;
		}

		// ------------------
		// Send �غ��ϱ�
		// ------------------
		// 1. WSABUF ����.
		WSABUF wsabuf[2];
		int wsabufCount = 0;

		// 2. BrokenSize ���ϱ�
		// !!!!���� ����!!! 
		// ���� ������ ���� BrokenSize��, UseSize���� ���� ���ؿ��� �帧�� ������!!! 
		// 
		// ------------
		// �ó�����...
		// [Front = 0, rear = 10]�� ��Ȳ�� BrokenSize�� ���ϸ� 10�� ���´�. (BrokenSize = 10)
		// �� ���̿� ���ؽ�Ʈ ����Ī�� �Ͼ�� �ٸ� �����尡 �����͸� �ִ´�. (Fornt = 0, rear = 20)
		// �ٽ� ���ؽ�Ʈ ����Ī���� �ش� ������� ���ƿ� �� UseSize�� ���ϸ� 20�� ���´� (Front = 0, rear = 20, BrokenSize = 10, UseSize = 20)
		// ------------
		// 
		// �� ��Ȳ����, BrokenSize�� UseSize�� ���ϸ�, ���� ��Ȳ�� �ƴѵ� ���۸� 2�� ����ϰԵȴ�.
		// �� ���, [0]�� ���ۿ� &Buff[0]�� ���� ����� 10, [1]�� ���ۿ��� &Buff[0]�� ���� ����� 10�� ���� ��Ȳ�� �߻��Ѵ�.
		// ������ �����Ͱ� ���� �ȴ�!! 
		// �� ���, ����ʿ����� ������ ���� ������?�� ������ �������� �ְ�.. Ȥ�� �׳� ���� �̻��ϰ� ó���Ǽ� ������ ���带 ���Ҽ��� �ְ� �׷���.
		// ��������!!!!!

		int BrokenSize = NowSession->m_SendBuff.GetNotBrokenGetSize();

		// 3. UseSize�� �� ũ�ٸ�, ���۰� ����ٴ� �Ҹ�. 2���� ��� ������.
		if (BrokenSize <  UseSize)
		{
			// fornt ��ġ�� ���۸� �����ͷ� ����(���� 1ĭ �տ� ���� ������, �� �ȿ��� 1ĭ �ձ��� �������)
			wsabuf[0].buf = NowSession->m_SendBuff.GetFrontBufferPtr();
			wsabuf[0].len = BrokenSize;

			wsabuf[1].buf = NowSession->m_SendBuff.GetBufferPtr();;
			wsabuf[1].len = UseSize - BrokenSize;
			wsabufCount = 2;
		}

		// 3-2. �װ� �ƴ϶��, WSABUF�� 1���� �����Ѵ�.
		else
		{
			// fornt ��ġ�� ���۸� �����ͷ� ����(���� 1ĭ �տ� ���� ������, �� �ȿ��� 1ĭ �ձ��� �������)
			wsabuf[0].buf = NowSession->m_SendBuff.GetFrontBufferPtr();
			wsabuf[0].len = BrokenSize;

			wsabufCount = 1;
		}

		// 4. Overlapped ����ü �ʱ�ȭ
		ZeroMemory(&NowSession->m_SendOverlapped, sizeof(NowSession->m_SendOverlapped));

		// 5. WSASend()
		DWORD SendBytes = 0, flags = 0;
		InterlockedIncrement(&NowSession->m_IOCount);
		int retval = WSASend(NowSession->m_Client_sock, wsabuf, wsabufCount, &SendBytes, flags, &NowSession->m_SendOverlapped, NULL);

		// 6. ���� ó��
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
				{
					SYSTEMTIME lst;
					GetLocalTime(&lst);
					_LOG(dfLOG_LEVEL_ERROR, L"[%02d/%02d/%02d %02d:%02d:%02d] SendPost(). ���� �߻� (%d)\n",
						lst.wYear - 2000, lst.wMonth, lst.wDay, lst.wHour, lst.wMinute, lst.wSecond, Error);
				}

				// ���� �����̶��
				else if (Error == WSAENOBUFS)
				{
					// ȭ�鿡 ���ۺ��� ��� 
					SYSTEMTIME lst;
					GetLocalTime(&lst);
					_LOG(dfLOG_LEVEL_ERROR, L"[%02d/%02d/%02d %02d:%02d:%02d] SendPost(). ���� ������ ���� : IP �ּ�=%s, ��Ʈ=%d\n",
						lst.wYear - 2000, lst.wMonth, lst.wDay, lst.wHour, lst.wMinute, lst.wSecond, NowSession->m_IP, NowSession->m_prot);
				}	

				return false;
			}
		}
		break;
	}

	return true;
}