#include "stdafx.h"
#include "Map.h"
#include <cmath>



//*******************************************
// �̱��� ���� �Լ�
// ���� ��ü Ÿ�� ����(����), �� ��ü Ÿ�� ����(����), ������ ������(����), ������ ������(����), Ÿ�� 1���� ����� �Է¹���.
//*******************************************

CMap* CMap::Getinstance(int TotalTileWidth, int TotalTileHeight, int WinWidth, int WinHeight, int TileSize)
{
	static CMap SingletonMap(TotalTileWidth, TotalTileHeight, WinWidth, WinHeight, TileSize);
	return &SingletonMap;
}

// ������
// ���� ��ü Ÿ�� ����(����), �� ��ü Ÿ�� ����(����), ������ ������(����), ������ ������(����), Ÿ�� 1���� ������
CMap::CMap(int TotalTileWidth, int TotalTileHeight, int WinWidth, int WinHeight, int TileSize)
{
	// 1. �̱������� CSpritedib�� ��ü ������.
	g_cSpriteDib = CSpritedib::Getinstance(70, 0x00ffffff, 32);

	// 2. �� ��ü�� Ÿ�� ���� ����.
	m_Height = TotalTileHeight;
	m_Width = TotalTileWidth;
	m_TileSize = TileSize;			// Ÿ�� 1���� ����� ����.

	// 3. ������ ������ ����
	m_WindowHeight = WinHeight;
	m_WindowWidth = WinWidth;

	// 4. ������ ������ ����, �� ȭ�鿡 ǥ�õǴ� Ÿ�� ���� ����
	m_HeightTileCount = round( (float)WinHeight / (float)TileSize ) + 1;
	m_WidthTileCount = round((float)WinWidth / (float)TileSize) + 1;

	// 5. Ÿ�� ������ŭ 2���� �迭�� �����Ҵ�
	Tile = new MapTile*[TotalTileHeight];
	for (int i = 0; i < TotalTileHeight; ++i)
		Tile[i] = new MapTile[TotalTileWidth];

	// 6. Ÿ�� ������ŭ x,y,������, �ش� Ÿ���� �ε��� ����(�ȼ� ��ǥ)
	int TempRadius = TileSize / 2;

	int SaveX = 0;
	int SaveY = 0;
	for (int i = 0; i < TotalTileHeight; ++i)
	{		
		for (int h = 0; h < TotalTileWidth; ++h)
		{
			Tile[i][h].m_x = SaveX + TempRadius;
			Tile[i][h].m_y = SaveY + TempRadius;
			Tile[i][h].m_redius = TempRadius;
			Tile[i][h].m_MapIndex = 0;

			SaveX += TempRadius * 2;
		}

		SaveX = 0;
		SaveY += TempRadius * 2;
	}
}

// �Ҹ���
CMap::~CMap()
{
	// Ÿ�� ��������
	for (int i = 0; i < m_Height; ++i)
		delete Tile[i];

	delete Tile;	
}


// ī�޶� X��ǥ ���
int CMap::GetCameraPosX()
{
	return m_iCameraPosX;
}

// ī�޶� Y��ǥ ���
int CMap::GetCameraPosY()
{
	return m_iCameraPosY;
}

// ī�޶� ��ǥ�� ��������, ȭ�� ��ǥ �˾Ƴ��� (X��ǥ)
int CMap::GetShowPosX(int NowX)
{		
	int temp = NowX - m_iCameraPosX;

	return temp;
}

// ī�޶� ��ǥ�� ��������, ȭ�� ��ǥ �˾Ƴ� (Y��ǥ)
int CMap::GetShowPosY(int NowY)
{
	int temp = NowY - m_iCameraPosY;

	return temp;
}

// ī�޶� ��ǥ ����
void  CMap::SetCameraPos(int PlayerX, int PlayerY, int NowSpriteIndex)
{	
	// �Է¹��� x,y�� �÷��̾��� ���� ���� ������ǥ�̴�. (��Ʈ�� �»�� �ƴ�!)

	// 1. ī�޶� ��ǥ ���� ��, �뷫������ ȭ���� �߽�? ������ ��ǥ�� ����Ѵ�.
	// X�� �������, Y�����Ѵ�. ������ �ణ �Ʒ��� �־ ���� ��ǥ �״�� ī�޶� ��ġ�� ���ϸ� ���߾��� �ƴҼ��� �ִ�.
	// �� ��ǥ�� �������� ī�޶� ��ġ�� ���Ѵ�.
	int TempCenterX = PlayerX;
	int TempCenterY = PlayerY;

	// 2. ī�޶� ��ġ ����.
	m_iCameraPosX = TempCenterX - (m_WindowWidth / 2);
	m_iCameraPosY = TempCenterY - (m_WindowHeight / 2);

	// 3. ����, ī�޶� ��ġ�� - ��� 0���� �ٲ��ش�.
	if (m_iCameraPosX < 0)
		m_iCameraPosX = 0;

	if (m_iCameraPosY < 0)
		m_iCameraPosY = 0;

	// 3. ����, ī�޶� ��ġ�� >> ���̶�� x ��ǥ�� �����ȴ�.
	if (m_iCameraPosX + m_WindowWidth > m_TileSize * m_Width)
		m_iCameraPosX = (m_TileSize * m_Width) - m_WindowWidth;

	// 3. ���� ī�޶� ��ġ�� �Ʒ� ���̶��, y��ǥ�� �����ȴ�.
	if (m_iCameraPosY + m_WindowHeight > m_TileSize * m_Height)
		m_iCameraPosY = (m_TileSize * m_Height) - m_WindowHeight;


}

// ���� ��ο��ϴ� �Լ�
bool CMap::MapDraw(BYTE* bypDest, int DestWidth, int DestHeight, int DestPitch)
{
	// Dest�� ���� ���� ��ġ �����صα� ---------
	BYTE* FirstDestSave = bypDest;

	// ī�޶� ��ǥ ����, �̹� �����ӿ� �����, ȭ�� �»�� Ÿ�� index ���ϱ�
	int NowTileY = m_iCameraPosY / m_TileSize;
	int NowTileX = m_iCameraPosX / m_TileSize;	

	// ����, �ε����� - ��� 0���� �ٲ��ش�.
	if (NowTileX < 0)
		NowTileX = 0;

	if (NowTileY < 0)
		NowTileY = 0;

	// ������ ������ ���� ���������� �� �̻� ��ũ�ѵǸ� �ȵȴ�.
	// Ÿ���� X�ε����� 89(100����)���� ����.
	if (NowTileX > m_Width - m_WidthTileCount)
		NowTileX = m_Width - m_WidthTileCount;

	// ������ �Ʒ� ���� ����������, �� �̻� ��ũ�ѵǸ� �ȵȴ�.
	if (NowTileY > m_Height - m_HeightTileCount)
		NowTileY = m_Height - m_HeightTileCount;

	int i;
	int h;
	int AddTileX = 0;
	int AddTileY = 0;
	// m_HeightTileCount
	for (i = 0; i < m_HeightTileCount; ++i)
	{
		BYTE* SecondDestSave = bypDest;
		int TempHeight = 0;

		for (h = 0; h < m_WidthTileCount; ++h)
		{
			BYTE* save = g_cSpriteDib->m_stpSprite[Tile[NowTileY][NowTileX].m_MapIndex].bypImge;	// �پ� �� ������ Ÿ���� ���� �ּ�(0,0)

			// �ش� �Լ������� �����, �������� �����Ѵ�. Ŭ������ �ϱ� ������ ���� ������ �ʿ��ϴ�.
			int iSpriteWidth = g_cSpriteDib->m_stpSprite[Tile[NowTileY][NowTileX].m_MapIndex].iWidth;
			int iSpriteHeight = g_cSpriteDib->m_stpSprite[Tile[NowTileY][NowTileX].m_MapIndex].iHeight;
			int iSpritePitch = g_cSpriteDib->m_stpSprite[Tile[NowTileY][NowTileX].m_MapIndex].iPitch;	

			// �ش� Ÿ���� �»�� �˾ƿ��� ����.(ȭ�� ��ǥ)
			int iSpriteShowX = GetShowPosX(Tile[NowTileY][NowTileX].m_x - Tile[NowTileY][NowTileX].m_redius);
			int iSpriteShowY = GetShowPosY(Tile[NowTileY][NowTileX].m_y - Tile[NowTileY][NowTileX].m_redius);

			// ��������Ʈ ���� ��ġ ����.
			int iSpriteStartX = 0;
			int iSpriteStartY = 0;	
						
			// Ŭ���� ó�� -------------
			// ��
			if (iSpriteShowY < 0)
			{
				int iAddY = iSpriteShowY * -1;				// ���� �󸶳� �ö󰬴��� ����.
				iSpriteStartY += iAddY;						// ��������Ʈ�� ������ y ��ġ�� �Ʒ��� iAddY��ŭ ������.
				iSpriteHeight -= iAddY;						// ��������Ʈ�� ���̸� iAddY��ŭ ���δ�. 

				// ���̰� -�� ���Դٸ� ���̸� 0���� �����.
				if (iSpriteHeight < 0)
					iSpriteHeight = 0;
			}

			 // ��
			if (iSpriteShowY + m_TileSize > m_WindowHeight)
			{
				int iAddY = (iSpriteShowY + m_TileSize) - m_WindowHeight;		// �Ʒ��� �󸶳� ���������� ����	
				iSpriteHeight -= iAddY;											// ��������Ʈ�� ���̸� iAddY��ŭ ���δ�. 	

				// ���̰� -�� ���Դٸ� ���̸� 0���� �����.
				if (iSpriteHeight < 0)
					iSpriteHeight = 0;						 		
			}

			// ��
			int iAddX = 0;
			if (iSpriteShowX < 0)
			{
				iAddX = iSpriteShowX *-1;				// �·� �󸶳� ������ ����.
				iSpriteStartX += iAddX;					// ��������Ʈ�� ������ x ��ġ�� �������� iAddX��ŭ �����δ�.
				iSpriteWidth -= iAddX;					// ���̵� �׸�ŭ ���δ�.

				// ���̰� -�� �������� ���̸� 0���� �����.
				if (iSpriteWidth < 0)
					iSpriteWidth = 0;
			}

			// �� (>>ȭ�� ���� �����ϸ� ��ũ�� ���� ����)
			if ((iSpriteShowX + m_TileSize > m_WindowWidth))
			{
				iAddX = iSpriteShowX + m_TileSize - m_WindowWidth;	// ��� �󸶳� ������ ����. ��� �̷��� ��!
				iSpriteWidth -= iAddX;					// ���̵� �׸�ŭ ���δ�.
			}
			

			// ��������Ʈ�� ���� ������ �̵�. ----------
			// ���⼭ ���� ����Ʈ ������ ����Ѵ�.
			g_cSpriteDib->m_stpSprite[Tile[NowTileY][NowTileX].m_MapIndex].bypImge += (iSpriteStartX * g_cSpriteDib->m_iColorByte) + (iSpriteStartY * iSpritePitch);

			// Dest�� ���� ���� ��ġ �����صα� ---------
			BYTE* DestSave = bypDest;

			// ��������Ʈ ���� -------------
			// ���������� 1�پ� �Ʒ��� �̵�
			int cpySize = iSpriteWidth *  g_cSpriteDib->m_iColorByte;
			for (int aa = 0; aa < iSpriteHeight; ++aa)
			{				
				memcpy(bypDest, g_cSpriteDib->m_stpSprite[Tile[NowTileY][NowTileX].m_MapIndex].bypImge, cpySize);
				bypDest += DestPitch;
				g_cSpriteDib->m_stpSprite[Tile[NowTileY][NowTileX].m_MapIndex].bypImge += iSpritePitch;
			}

			// Dest�� ���� ���� ��ġ ���� ---------
			bypDest = DestSave;			

			// �� ��������Ʈ ������ �ּҸ� ������Ű�鼭 ��������� ���� ��ġ�� �̵���Ų��.
			g_cSpriteDib->m_stpSprite[Tile[NowTileY][NowTileX].m_MapIndex].bypImge = save;

			TempHeight = iSpriteHeight;

			// Ÿ�� Index�� X��ġ�� >>�� 1ĭ �̵�
			if (NowTileX + 1 == m_Width)
				break;

			NowTileX++;
			AddTileX++;

			// ������� ��ġ�� 1ĭ >>�� �̵�(Ÿ�� ���� 1ĭ)
			bypDest += cpySize;			
		}

		// ������� ��ġ�� ���� ��, Ÿ�� 1����ŭ �Ʒ��� ������.
		bypDest = SecondDestSave;
		bypDest += (DestPitch * TempHeight);

		// Ÿ�� Index�� x��ġ�� ó�� ��ġ�� �̵�
		NowTileX -= AddTileX;
		AddTileX = 0;

		// Ÿ�� Index�� y��ġ�� 1�� �Ʒ��� �̵�
		if(NowTileY + 1 != m_Height)
			NowTileY++;

	}

	bypDest = FirstDestSave;

	return TRUE;	
}