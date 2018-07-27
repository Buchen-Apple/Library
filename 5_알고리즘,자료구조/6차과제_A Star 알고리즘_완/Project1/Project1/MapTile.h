#ifndef __MAP_TILE_H__
#define __MAP_TILE_H__

#define MAP_WIDTH			40
#define MAP_HEIGHT			20
#define MAP_TILE_RADIUS		20

enum Tile_Type
{
	NONE = 0, START, FIN, OBSTACLE, OPEN_LIST, CLOSE_LIST
};

struct Map
{
	// ���⿡ ����Ǵ� X,Y�� ���� �ȼ� ��ǥ
	int m_iMapX;
	int m_iMapY;
	Tile_Type m_eTileType;
};

#endif // !__MAP_TILE_H__
