#pragma once
#include "cocos2d.h"
#include "GameMap.h"
#include "SystemHeader.h"

class GameMap;
class Snake;

USING_NS_CC;
using namespace std;
class MyGame : public Layer {
private:
	Sprite* progress[foods_num];
	float progress_length;
public:
	enum FOOD {
		food_green_apple = 1
		, food_red_apple
		, food_bird
		, food_cola
		, food_bug
		, food_flower
		, food_heart
		, food_shit
	};
	enum gameOverState {
		hungry
		, eat_shit
		, impact_wall
		, impact_snake
		, no_way
		, friend_die
		, win
	};
	bool has_player;
	int remain_num[foods_num];
	float cooldown[foods_num], current_cooldown[foods_num];
	static const int max_heart = 5, max_pause_n = 5;
	DEFINE_VAR_GET_ADD(int, score);
	DEFINE_VAR_GET_ADD(int, heart);
	DEFINE_VAR_GET_ADD(int, max_hunger);
	DEFINE_VAR_GET_ADD(int, pause_n);

	static Scene* createScene(int mission_id);
	static MyGame* create(int mission_id);
	virtual bool init(int mission_id);
	virtual void update(float dt);
	void update_dir();
	void game_over(gameOverState state);
	void print_log(string str);
	void set_pause(bool pause);
	void set_UI();
	void set_Ctrl();
	bool isUpdate;
	int apple, bug, flower, kill;
	Sprite *turn_1, *turn_2;
	MenuItemFont *menu_clear_dir, *menu_back, *menu_again, *menu_pause;
	Menu* menu;
	EventListenerKeyboard *listener_key;
	EventListenerTouchOneByOne *listener_touch;
	Sprite *sprite_ban;
	GameMap* game_map;
	vector<Snake*> snakes;
};