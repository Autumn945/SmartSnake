#pragma once
#include "cocos2d.h"
#include "Snake.h"

using namespace std;
USING_NS_CC;

class SmartSnake: public Snake {
public:
	Snake::SnakeType type;
public:
	int pre_step_on[max_game_width][max_game_height];
	int step_on[max_game_width][max_game_height];
	int dir_f[4];

	virtual bool init(string name, GameMap* game_map);
	static SmartSnake* create(string name, GameMap* game_map);
	virtual void eat_reward(int gid);
	virtual bool go_die();
	virtual bool go_ahead();
	virtual SnakeType get_type();
	bool act();

	pii dfs_get_accessible_last_snake_node_dir(pii now, int step, int dir, int first_dir);
	int get_accessible_last_snake_node_dir(pii position, int dir, int & start_step);

	int get_target_shortest_path_dir(pii position, int current_dir, int & max_length);

};

