#ifndef __MAP_H__
#define __MAP_H__
#include "SpriteDib.h"

class BaseOBject;

class CMap
{
	friend class BaseOBject;

	//*******************************************
	// �����̺� �Լ�
	//*******************************************
private:
	// �̱����� ���� �����ڴ� Private���� ����
	CMap(int TotalTileWidth, int TotalTileHeight, int WinWidth, int WinHeight, int TileSize);

	// Ÿ�� ����ü
	struct MapTile
	{
		// �ȼ� ���� x,y,������
		int m_x;		
		int m_y;
		int m_MapIndex;		// ǥ���� Ÿ���� �ε���. ������ �� 0�̴�.
		int m_redius;
	};

	//*******************************************
	// ��� ����Ǵ� �����̺� ����
	//*******************************************
private:
	// ī�޶� ��ǥ. ������ǥ(�ȼ�) �� X. �»��
	int m_iCameraPosX;

	// ī�޶� ��ǥ. ������ǥ(�ȼ�) �� Y. �»��
	int m_iCameraPosY;


	//*******************************************
	// ���� ���� �� ������� �ʴ� ����
	//*******************************************
private:
	// Ÿ�� 1���� ������
	int m_TileSize;

	// ������ ������ ����
	int m_WindowHeight;

	// ������ ������ ����
	int m_WindowWidth;

	// �����ϰ��ִ� �ε����� ���� (����)
	int m_Height;

	// �����ϰ��ִ� �ε����� ���� (����)
	int m_Width;

	// �̱������� CSpritedib�� ��ü
	CSpritedib* g_cSpriteDib;

	// MapTile ����ü
	MapTile** Tile;

	// ������ ũ�� ����, �� ȭ�鿡 ǥ�õǴ� Ÿ���� ����
	// 640x480����, ���̴� (480/64 = 7.5(�ݿø��ؼ� 8) + 1 = 9��) / ���̴� (640/64 =  10 + 1 = 11��) �̴�.
	int m_HeightTileCount;
	int m_WidthTileCount;

public:

	// �̱��� ���� �Լ�
	static CMap* Getinstance(int TotalTileWidth, int TotalTileHeight, int WinWidth, int WinHeight, int TileSize);

	// �Ҹ���
	~CMap();

	// ī�޶� X��ǥ ���
	int GetCameraPosX();

	// ī�޶� Y��ǥ ���
	int GetCameraPosY();
	
	// ���� ��ο��ϴ� �Լ�
	bool MapDraw(BYTE* bypDest, int DestWidth, int DestHeight, int DestPitch);

	// ī�޶� ��ǥ ����
	void SetCameraPos(int PlayerX, int PlayerY, int NowSpriteIndex);

	// ī�޶� ��ǥ�� ��������, ���� ��µǾ�� �ϴ� ������ǥ �˾Ƴ��� (X��ǥ)
	int GetShowPosX(int NowX);

	// ī�޶� ��ǥ�� ��������, ���� ��µǾ�� �ϴ� ������ǥ �˾Ƴ��� (Y��ǥ)
	int GetShowPosY(int NowY);

};

#endif // !__MAP_H__
