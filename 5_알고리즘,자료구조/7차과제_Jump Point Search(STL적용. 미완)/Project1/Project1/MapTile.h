#ifndef __MAP_TILE_H__
#define __MAP_TILE_H__

#define MAP_WIDTH			60
#define MAP_HEIGHT			30
#define MAP_TILE_RADIUS		15

enum Tile_Type
{
	NONE = 0, START, FIN, OBSTACLE, OPEN_LIST, CLOSE_LIST, JUMP
};

struct Map
{
	// ���⿡ ����Ǵ� X,Y�� ���� �ȼ� ��ǥ
	int m_iMapX;
	int m_iMapY;
	Tile_Type m_eTileType;
};

#endif // !__MAP_TILE_H__
