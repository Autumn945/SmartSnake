#pragma once
#include "cocos2d.h"
#include "SystemHeader.h"

class Snake;

USING_NS_CC;
using namespace std;
class GameMap : public TMXTiledMap {
public:
	protected: Sprite* snake_map[max_game_width][max_game_height];
	public: Sprite* (*get_snake_map())[max_game_height]{
		return snake_map;
	}
	DEFINE_VAR_GET_ADD(int, empty_n);
	DEFINE_VAR_GET(TMXLayer*, food);
	DEFINE_VAR_GET(TMXLayer*, wall);
	pii to_tile_map_pos(Vec2 pos);
	Vec2 to_cocos_pos(pii pos);
	static const int random_wall = 29;
	static const int true_wall = 17;
	int wall_id(pii pos);
	int food_id(pii pos);
	void set_wall(pii pos, int id);
	void set_food(pii pos, int id);
	~GameMap();
	static GameMap* createWithTMXFile(string file_name);
	virtual bool initWithTMXFile(string);
	vector<pii> get_all_empty_points();
	pii get_random_empty_point();
	pii get_next_position(pii now, int dir);
	bool is_empty(pii pos, int delay = 0, Snake *ignore = NULL);
};