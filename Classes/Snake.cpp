#include "Snake.h"
#include "SystemHeader.h"
#include "MyGameScene.h"
#include "SimpleAudioEngine.h"

#define HEAD Rect(0, 0, UNIT, UNIT)
#define BODY Rect(0, UNIT, UNIT, UNIT)
#define TURN Rect(0, 2 * UNIT, UNIT, UNIT)
#define TAIL Rect(UNIT, 3 * UNIT, UNIT, UNIT)

//snake is player if type = 0
Snake* Snake::create(string name, GameMap* game_map) {
	Snake *pRet = new(std::nothrow) Snake();
	if (pRet && pRet->init(name, game_map)) {
		pRet->autorelease();
		return pRet;
	}
	else {
		delete pRet;
		pRet = nullptr;
		return nullptr;
	}
}

bool Snake::init(string name, GameMap* game_map) {
	log("snake init");
	if (!SpriteBatchNode::init()) {
		return false;
	}
	auto snake = game_map->getObjectGroup("snake_objs")->getObject(name);
	if (snake.size() == 0) {
		return false;
	}
	position = game_map->to_tile_map_pos(Vec2(snake["x"].asFloat(), snake["y"].asFloat()));
	if (snake.count("direction") == 0) {
		log("direction of snake has not define");
		snake["direction"] = Value(DIRECTION::UP);
	}
	if (snake.count("length") == 0) {
		log("length of snake has not define");
		snake["length"] = 3;
	}
	if (snake.count("speed") == 0) {
		log("speed of snake has not define");
		snake["speed"] = 40;
	}
	current_dir = (DIRECTION)snake["direction"].asInt();
	length = snake["length"].asInt();
	speed = snake["speed"].asInt();
	auto snake_type = snake["type"].asString();
	if (snake_type == "player") {
		image = "snake_player.png";
	}
	else if (snake_type == "friend") {
		image = "snake_friend.png";
	}
	else if (snake_type == "enemy") {
		image = "snake_enemy.png";
	}
	else {
		image = "snake_enemy.png";
	}
	if (!SpriteBatchNode::initWithFile(image)) {
		return false;
	}
	this->game_map = game_map;
	this->setName(name);
	is_died = false;
	snake_nodes = new queue<Sprite*>();
	turn_1 = turn_2 = -1;
	hunger = 0;
	step = 0;
	time_stamp = 0;
	new_head();
	add_time_stamp(1);
	this->setAnchorPoint(Vec2::ZERO);
	this->setPosition(Vec2::ZERO + Vec2(UNIT, UNIT) / 2);
	game_map->addChild(this);
	return true;
}

Snake::~Snake() {
	delete snake_nodes;
	log("deleted a snake");
}

void Snake::go_step() {
	//const int step_length = 500;
	if (speed < 1) {
		speed = 1;
	}
	else if (speed > step_length / 2) {
		speed = step_length / 2;
	}
	if (step >= step_length) {
		this->go_ahead();
		add_time_stamp(1);
		step -= step_length;
	}
	step += speed;
}

bool Snake::go_ahead() {
	if (is_died) {
		return false;
	}
	int s = length - (get_time_stamp() - get_snake_nodes()->front()->getTag());
	if (get_snake_nodes()->front()->getName() == "food") {
		s--;
	}
	if (s == 0) {
		new_tail();
	}
	else if (s > 0) {
		//snake_nodes->back()->setColor(Color3B::GRAY);
		log("too short");
	}
	else if (s < 0) {
		log("too long");
		new_tail();
		new_tail();
	}
	new_head();
	is_checked = false;
	hunger += 1;
	return true;
}

bool Snake::new_tail() {
	if (snake_nodes->empty()) {
		return false;
	}
	auto tail = snake_nodes->front();
	if (!is_died && tail->getName() == "food") {
		tail->setName("tail");
		tail->setColor(Color3B::WHITE);
		game_map->add_empty_n(-1);
		return false;
	}
	auto pos = game_map->to_tile_map_pos(tail->getPosition());
	game_map->get_snake_map()[pos.first][pos.second] = NULL;
	this->removeChild(tail, true);
	snake_nodes->pop();
	if (!is_died && !snake_nodes->empty()) {
		snake_nodes->front()->setSpriteFrame(SpriteFrame::create(this->image, TAIL));
	}
	return true;
}

bool Snake::new_head() {
	if (!snake_nodes->empty()) {
		int next_dir = current_dir;
		if (turn_1 >= 0) {
			next_dir = turn_1;
			turn_1 = turn_2;
			turn_2 = -1;
		}
		Rect rect = TAIL;
		bool flipped_x = false;
		if (snake_nodes->size() >= 2) {
			switch (next_dir - current_dir) {
			case -1:
			case 3:
				flipped_x = true;
			case 1:
			case -3:
				rect = TURN;
				break;
			case 0:
				rect = BODY;
				break;
			}
		}
		current_dir = next_dir;
		snake_nodes->back()->setSpriteFrame(SpriteFrame::create(this->image, rect));
		snake_nodes->back()->setFlippedX(flipped_x);
		snake_nodes->back()->setRotation(90 * current_dir);
		position = game_map->get_next_position(position, (DIRECTION)current_dir);
	}
	auto head = Sprite::create(this->image, HEAD);
	head->setTag(get_time_stamp());
	head->setPosition(game_map->to_cocos_pos(position));
	head->setRotation(90 * current_dir);
	snake_nodes->push(head);
	if (game_map->get_snake_map()[position.first][position.second] == NULL) {
		game_map->get_snake_map()[position.first][position.second] = head;
	}
	/*for (int i = 0; i < game_map->getmapsize().height; i++) {
		string prt = "";
		for (int j = 0; j < game_map->getmapsize().width; j++) {
			if (game_map->get_snake_map()[j][i]) {
				prt += '*';
			}
			else {
				prt += '.';
			}
		}
		log("%s", prt.c_str());
	}
	log("\n");*/
	this->addChild(head);
	return true;
}

bool Snake::check() {
	if (is_died) {
		return false;
	}
	is_checked = true;
	auto wall = game_map->getLayer("wall");
	auto game = (MyGame*)game_map->getParent();
	if (wall != NULL && wall->getTileGIDAt(Vec2(position.first, position.second)) > 0) {
		log("%s p wall", this->getName().c_str());
		this->go_die();
		if (this->get_type() == SnakeType::t_player) {
			game->game_over(MyGame::gameOverState::impact_wall);
		}
		return true;
	}
	auto sp = game_map->get_snake_map()[position.first][position.second];
	if (sp == NULL) {
		game_map->get_snake_map()[position.first][position.second] = this->snake_nodes->back();
	}
	else if (sp != this->snake_nodes->back()) {
		auto snk = (Snake*)sp->getParent();
		if (snk == NULL) {
			log("%s p NULL", this->getName().c_str());
		}
		else {
			log("%s p %s", this->getName().c_str(), snk->getName().c_str());
		}
		this->go_die();
		if (this->get_type() == SnakeType::t_player) {
			game->game_over(MyGame::gameOverState::impact_snake);
		}
		return true;
	}
	auto food = game_map->getLayer("food");
	if (food != NULL && food->getTileGIDAt(Vec2(position.first, position.second)) > 0) {
		auto gid = food->getTileGIDAt(Vec2(position.first, position.second));
		this->eat(gid);
		food->setTileGID(0, Vec2(position.first, position.second));
	}
	return true;
}

void Snake::eat_reward(int gid) {
	hunger -= 20;
	if (hunger < 0) {
		hunger = 0;
	}
	auto game = (MyGame*)game_map->getParent();
	game->add_score(food_score[gid - 1]);
	auto label = (Label*)game->getChildByName("label_score");
	label->setString(get_UTF8_string("score") + Value(game->get_score()).asString()); auto food_gid = (MyGame::FOOD)gid;
	auto str_audio = "eat.wav";
	switch (food_gid) {
	case MyGame::food_green_apple:
		game->print_log(get_UTF8_string("eat green_apple"));
		break;
	case MyGame::food_red_apple:
		game->print_log(get_UTF8_string("eat red_apple"));
		break;
	case MyGame::food_bird:
	{
		hunger -= 30;
		if (hunger < 0) {
			game->add_max_hunger(-hunger);
			hunger = 0;
		}
		game->print_log(get_UTF8_string("eat bird"));
		break;
	}
	case MyGame::food_cola:
	{
		if (game->get_pause_n() < MyGame::max_pause_n) {
			game->add_pause_n(1);
		}
		auto label = (Label*)game->menu_pause->getChildByName("label_pause");
		label->setString(" x" + Value(game->get_pause_n()).asString());
		game->print_log(get_UTF8_string("eat cola"));
		break;
	}
	case MyGame::food_bug:
	{
		game->bug--;
		auto label = (Label*)game->getChildByName("label_bug");
		if (label) {
			if (game->bug > 0) {
				label->setString(" x" + Value(game->bug).asString());
			}
			else {
				label->setString(" ok!");
			}
		}
		game->print_log(get_UTF8_string("eat bug"));
		break;
	}
	case MyGame::food_flower:
	{
		game->flower--;
		auto label = (Label*)game->getChildByName("label_flower");
		if (label) {
			if (game->flower > 0) {
				label->setString(" x" + Value(game->flower).asString());
			}
			else {
				label->setString(" ok!");
			}
		}
		game->print_log(get_UTF8_string("eat flower"));
		break;
	}
	case MyGame::food_heart:
	{
		if (game->get_heart() < MyGame::max_heart) {
			game->add_heart(1);
		}
		game->print_log(get_UTF8_string("eat heart"));
		str_audio = "eat_heart.wav";
		break;
	}
	case MyGame::food_shit:
	{
		game->add_heart(-1);
		game->print_log(get_UTF8_string("eat shit"));
		str_audio = "eat_shit.wav";
		Device::vibrate(0.2f);
		break;
	}
	default:
		break;
	}
	if (user_info["soundEffects"].asInt() == 0) {
		CocosDenshion::SimpleAudioEngine::getInstance()->playEffect(str_audio);
	}
}

bool Snake::eat(int gid) {
	eat_reward(gid);
	//score += MyGame::get_game_node()->get_speed();
	snake_nodes->back()->setName("food");
	snake_nodes->back()->setTag(snake_nodes->back()->getTag() + 1);
	length++;
	add_time_stamp(1);
	snake_nodes->back()->setColor(Color3B::GRAY);
	auto game = (MyGame*)game_map->getParent();
	if (game->remain_num[gid - 1] >= 0) {
		game->remain_num[gid - 1]++;
	}
	auto food_gid = (MyGame::FOOD)gid;
	switch (food_gid) {
	case MyGame::food_green_apple:
		this->add_speed(3);
		break;
	case MyGame::food_red_apple:
		this->add_speed(-2);
		break;
	case MyGame::food_bird:
		break;
	case MyGame::food_cola:
		break;
	case MyGame::food_bug:
		break;
	case MyGame::food_flower:
		break;
	case MyGame::food_heart:
		break;
	case MyGame::food_shit:
		break;
	default:
		break;
	}
	log("%s eat, pos = (%d, %d), head = %d, time = %d", this->getName().c_str(), position.first, position.second, snake_nodes->back(), clock());
	return true;
}

bool Snake::turn(DIRECTION dir) {
	if (turn_2 >= 0) {
		return false;
	}
	int last_turn = current_dir;
	if (turn_1 >= 0) {
		last_turn = turn_1;
	}
	if (abs(dir - last_turn) == 1 || abs(dir - last_turn) == 3) {
		if (turn_1 >= 0) {
			turn_2 = dir;
		}
		else {
			turn_1 = dir;
		}
		return true;
	}
	return false;
}

bool Snake::go_die() {
	is_died = true;
	return false;
}

int Snake::get_tail_time_stamp() {
	return get_time_stamp() - get_length();
}

Snake::SnakeType Snake::get_type() {
	return t_player;
}
