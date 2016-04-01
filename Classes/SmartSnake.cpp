#include "SmartSnake.h"
#include "MyGameScene.h"

//snake is player if type = 0
SmartSnake* SmartSnake::create(string name, GameMap* game_map) {
	SmartSnake *pRet = new(std::nothrow) SmartSnake();
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

bool SmartSnake::init(string name, GameMap* game_map) {
	log("snake init");
	if (!Snake::init(name, game_map)) {
		return false;
	}
	target = pii(-1, -1);
	return true;
}

void SmartSnake::eat_reward(int gid) {
	target = pii(-1, -1);
}

bool SmartSnake::go_die() {
	is_died = true;
	auto game = (MyGame*)game_map->getParent();
	game->kill--;
	auto label = (Label*)game->getChildByName("label_kill");
	if (label) {
		if (game->kill > 0) {
			label->setString(" x" + Value(game->kill).asString());
		}
		else {
			label->setString(" ok!");
		}
	}
	/*auto game = (MyGame*)getUserObject();
	if (snake_nodes->size() > 1) {
		schedule([this](float dt) {
			new_tail();
		}, 1 / 30.0f, snake_nodes->size() - 1, 0, "go die");
	}*/
	while (!snake_nodes->empty()) {
	new_tail();
	}
	return true;
}

bool SmartSnake::go_ahead() {
	if (!is_died && type == Snake::SnakeType::t_enemy) {
		act();
	}
	if (!Snake::go_ahead()) {
		return false;
	}
	return false;
}

Snake::SnakeType SmartSnake::get_type() {
	return type;
}

bool SmartSnake::act() {
	auto begin_time = clock();
	if (!turn_list->empty()) {
		return false;
	}
	bool safe = true;
	if (hunger > 100) {
		safe = false;
	}
	if (target.first >= 0) {
		auto food = game_map->getLayer("food");
		if (food->getTileGIDAt(Vec2(target.first, target.second)) == 0) {
			target = pii(-1, -1);
		}
	}
	if (target.first >= 0) {
		auto dir = game_map->get_target_shortest_path_dir(position, current_dir, target, safe);
		if (dir >= 0) {
			turn_list->push((DIRECTION)dir);
			//log("act 1, dir = %d, delay = %d", dir, clock() - begin_time);
			return true;
		}
	}
	else {
		auto targets = game_map->get_foods();
		for (auto t : targets) {
			auto dir = game_map->get_target_shortest_path_dir(position, current_dir, t, safe);
			if (dir >= 0) {
				target = t;
				turn_list->push((DIRECTION)dir);
				//log("act 2, dir = %d, delay = %d", dir, clock() - begin_time);
				return true;
			}
		}
	}
	int lenght_step_min;
	auto t = game_map->get_accessible_last_snake_node(position, current_dir, lenght_step_min);
	auto dir = game_map->get_target_longest_path_dir(position, current_dir, t);
	if (dir >= 0) {
		//log("go to (%d, %d)", t.first, t.second);
		turn_list->push((DIRECTION)dir);
		//log("act 3, dir = %d, delay = %d", dir, clock() - begin_time);
		return true;
	}
	log("!!!!!!!!!!!!!!!failed!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
	//go_die();
	dir = random(0, 3);
	this->turn((DIRECTION)dir);
	return false;
}