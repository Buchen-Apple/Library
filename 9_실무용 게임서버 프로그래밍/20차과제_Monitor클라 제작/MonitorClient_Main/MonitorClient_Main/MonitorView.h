#ifndef __MONITOR_VIEW_H__
#define __MONITOR_VIEW_H__

//// �ڽ� ������ Ŭ���� ��� ////////////////////////////
#include "Protocol_Set\CommonProtocol.h"

// --------------- 
// ť ����
// --------------- 
namespace Library_Jingyu
{

#define QUEUE_LEN 100

	// ť ����
	typedef struct
	{
		int front;
		int rear;
		int Peek;
		int queArr[QUEUE_LEN] = { 0, };
	}Queue;

	// �ʱ�ȭ
	void Init(Queue* pq);
	// ���� ��ġ Ȯ��
	int NextPos(int pos);
	// ��ť
	int Dequeue(Queue* pq);
	// ��ť
	void Enqueue(Queue* pq, int y);
	// ù ť Peek
	bool FirstPeek(Queue* pq, int* Data);
	// ���� ť Peek
	bool NextPeek(Queue* pq, int* Data);
}

// --------------- 
// ����͸� ���
// --------------- 
namespace Library_Jingyu
{

#define dfMAXCHILD		100
#define UM_PARENTBACKCHANGE WM_USER+1	// �ڽ��� �θ𿡰� ������ �޽���
#define UM_CHILDFONTCHANGE	WM_USER+2   //�θ� �ڽĿ��� ������ �޽���


	class CMonitorGraphUnit
	{

	private:
		struct ColumnInfo
		{
			int ServerID;
			int DataType;
			int iLastVal = 0;			// ���� ť�� ���� ���� ������ ��
			TCHAR DataName[20];
			Queue queue;
		};

		ColumnInfo *CoInfo;

	public:

		enum TYPE
		{
			BAR_SINGLE_VERT,
			BAR_SINGLE_HORZ,
			BAR_COLUMN_VERT,
			BAR_COLUMN_HORZ,
			LINE_SINGLE,
			LINE_MULTI,
			PIE,
			ONOFF
		};

	public:

		CMonitorGraphUnit(HINSTANCE hInstance, HWND hWndParent, COLORREF BackColor, TYPE enType, int iPosX, int iPosY, int iWidth, int iHeight, LPCTSTR str);
		~CMonitorGraphUnit();

		/////////////////////////////////////////////////////////
		// ������ ���ν���
		/////////////////////////////////////////////////////////
		static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

		/////////////////////////////////////////////////////////
		// ������ �ֱ�, ó�� ����Ʈ �� �߰�����
		/////////////////////////////////////////////////////////
		void InsertData(int iData, int ServerID, int DataType);
		void AddData(int iMax, int AleCount, LPCTSTR Unit);

		/////////////////////////////////////////////////////////
		// �÷� ���� ���� ����
		/////////////////////////////////////////////////////////
		void SetColumnInfo(int iColumnCount, int ServerID[], int DataType[], TCHAR DataName[][20]);

	private:
		//------------------------------------------------------
		// Paint�Լ�
		//------------------------------------------------------
		void Paint_LINE_SINGLE();
		void Paint_LINE_MULTI();
		void Paint_BAR_SINGLE_VERT();
		void Paint_BAR_COLUMN_VERT();
		void Paint_PIE();
		void paint_ONOFF();

		//------------------------------------------------------
		// �˶� ��, �θ� ��� ���������� ĥ�ϱ� üũ �Լ�
		//------------------------------------------------------
		void ParentBackCheck(int Data);

		//------------------------------------------------------
		// �⺻ UI ����
		//------------------------------------------------------
		void UISet();

		//------------------------------------------------------
		// �θ� ������ �ڵ�, �� ������ �ڵ�, �ν��Ͻ� �ڵ�
		//------------------------------------------------------
		HWND hWndParent;
		HWND hWnd;
		HINSTANCE hInstance;

		//------------------------------------------------------
		// ������ ��ġ,ũ��,����, �׷��� Ÿ�� ��.. �ڷ�
		//------------------------------------------------------
		TYPE _enGraphType;
		RECT rt;
		RECT TitleRt;
		RECT LeftRt;
		RECT RightRt;	// LINE_MULTIŸ�� �׷��������� ���ȴ�. ȭ�� ������ �׷��� ���� ǥ���ϴ� ����.
		RECT BottomRt;	// BAR_COLUMN_VERTŸ�� �׷��������� ���ȴ�. ȭ�� �ϴܿ� �׷��� ���� ǥ���ϴ� ����.
		RECT ClientRt;
		COLORREF BackColor;

		//------------------------------------------------------
		// ������
		//------------------------------------------------------	
		Queue queue;	// ť	
		double iAddY;
		int Greedint[4];
		bool AleOnOff;			//�ش� �����찡 �˶��� �︱�� ����. false�� �︮�� ����
		int iColumnCount;		// �ش� �������� �÷� �� ī��Ʈ

		//------------------------------------------------------
		// Ÿ��Ʋ�� ǥ�õ� �ؽ�Ʈ
		//------------------------------------------------------	
		TCHAR tWinName[20];		// ������ �̸�
		TCHAR tUnit[10];		// ǥ�� ����	

		//------------------------------------------------------
		// �׷��� Max�� , ��� �︮�� ��
		//------------------------------------------------------
		bool MaxVariable;
		int iMax;
		double AleCount;
		bool bObjectCheck;

		//------------------------------------------------------
		// MemDC�� ���ҽ�
		//------------------------------------------------------
		HDC MemDC;
		HBRUSH BackBrush, TitleBrush;
		HBRUSH PieBrush[5];						// ���� ä��� �� �귯��
		HBITMAP MyBitmap, OldBitmap;
		HFONT MyFont;
		COLORREF NowFontColor, NormalFontColor;	// ����� ��Ʈ�� �÷� , �ش� �������� �⺻ ��Ʈ �÷�
												// ���� �� ������, �ڵ忡���� NowFontColor�� �����Ű�� ��Ȳ�� ���� NowFontColor��Ʈ�� NormalFontColor Ȥ�� ���� ��Ʈ�� �ȴ�.
		HPEN* GraphPen;


		//------------------------------------------------------
		// ���� this�� ������ ���� ������
		//------------------------------------------------------
		static CMonitorGraphUnit *pThis;
	};

}

#endif // !__MONITOR_VIEW_H__
