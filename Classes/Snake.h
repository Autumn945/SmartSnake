#pragma once
#include "cocos2d.h"
#include "GameMap.h"
#include "SystemHeader.h"

using namespace std;
USING_NS_CC;

class Snake : public SpriteBatchNode{
public:
	enum SnakeType {
		t_player
		, t_friend
		, t_enemy
	};
	static const int step_length = 500;
	DEFINE_VAR_GET_SET(GameMap*, game_map);
	DEFINE_VAR_GET_SET(queue<Sprite*>*, snake_nodes);
	int turn_1, turn_2;
	DEFINE_VAR_GET(int, current_dir);
	DEFINE_VAR_GET(pii, position);
	DEFINE_VAR_GET(string, image);
	DEFINE_VAR_GET_SET(bool, is_died);
	DEFINE_VAR_GET(bool, is_checked);
	DEFINE_VAR_GET_SET(int, length);
	DEFINE_VAR_GET_ADD(int, hunger);
	DEFINE_VAR_GET_ADD(int, speed);
	DEFINE_VAR_GET_ADD(int, step);
	DEFINE_VAR_GET_ADD(int, time_stamp);

public:
	~Snake();
	void go_step();
	virtual bool init(string name, GameMap* game_map);
	static Snake* create(string name, GameMap* game_map);
	virtual bool go_ahead();
	virtual bool new_tail();
	virtual bool new_head();
	virtual bool check();
	virtual void eat_reward(int gid);
	virtual bool eat(int gid);
	virtual bool turn(DIRECTION dir);
	virtual bool go_die();
	virtual int get_tail_time_stamp();
	virtual SnakeType get_type();
};

