#include "GameMap.h"
#include "stdlib.h"
#include "Snake.h"

USING_NS_CC;

GameMap::~GameMap() {
	log("deleted a GameMap");
}

pii GameMap::to_tile_map_pos(Vec2 pos) {
	pos = pos / UNIT;
	return pii(float_to_int(pos.x), this->getMapSize().height - float_to_int(pos.y) - 1);
}
Vec2 GameMap::to_cocos_pos(pii pos) {
	pos.second = this->getMapSize().height - pos.second - 1;
	return Vec2(pos.first, pos.second) * UNIT;
}

int GameMap::wall_id(pii pos) {
	if (!wall) {
		return false;
	}
	return wall->getTileGIDAt(Vec2(pos.first, pos.second));
}

int GameMap::food_id(pii pos) {
	if (!food) {
		return 0;
	}
	return food->getTileGIDAt(Vec2(pos.first, pos.second));
}

void GameMap::set_wall(pii pos, int id) {
	if (!wall) {
		return;
	}
	wall->setTileGID(id, Vec2(pos.first, pos.second));
}

void GameMap::set_food(pii pos, int id) {
	if (!food) {
		return;
	}
	food->setTileGID(id, Vec2(pos.first, pos.second));
}

GameMap * GameMap::createWithTMXFile(string file_name) {
	GameMap *pRet = new(std::nothrow) GameMap();
	if (pRet && pRet->initWithTMXFile(file_name)) {
		pRet->autorelease();
		return pRet;
	}
	else {
		delete pRet;
		pRet = nullptr;
		return nullptr;
	}
}

bool GameMap::initWithTMXFile(string file_name) {
	log("GameMap init");
	if (!TMXTiledMap::initWithTMXFile(file_name)) {
		log("initWithTMXFile %s failed!", file_name.c_str());
		return false;
	}

	this->setAnchorPoint(Vec2(0.5, 1));
	this->setPosition(Vec2(origin.x + visible_size.width / 2, origin.y + visible_size.height));
	wall = this->getLayer("wall");
	food = this->getLayer("food");
	if (!wall) {
		log("layer wall of tile_map has not found!");
	}
	if (!food) {
		log("layer food of tile_map has not found!");
	}
	if (!this->getLayer("grass")) {
		log("layer grass of tile_map has not found!");
	}
	else {
		this->getLayer("grass")->setLocalZOrder(1);
	}
	empty_n = this->getMapSize().width * this->getMapSize().height;
	for (int i = 0; i < this->getMapSize().width; i++) {
		for (int j = 0; j < this->getMapSize().height; j++) {
			auto wid = wall_id(pii(i, j));
			if (wid >= random_wall) {
				auto gid_v = this->getPropertiesForGID(wid);
				if (gid_v.isNull()) {
					gid_v = ValueMap();
				}
				auto probability = gid_v.asValueMap()["probability"].asInt();
				if (random(1, 100) > probability) {
					set_wall(pii(i, j), 0);
				}
				else {
					set_wall(pii(i, j), true_wall);
				}
			}
			if (wall_id(pii(i, j))) {
				empty_n--;
			}
		}
	}
	return true;
}

vector<pii> GameMap::get_all_empty_points() {
	auto mp = this->get_snake_map();
	vector<pii> ret;
	auto width = float_to_int(this->getMapSize().width);
	auto height = float_to_int(this->getMapSize().height);
	for (int i = 0; i < width; i++) {
		for (int j = 0; j < height; j++) {
			if (!wall_id(pii(i, j))
				&& mp[i][j] == NULL
				&& food_id(pii(i, j)) == 0) {
				ret.push_back(pii(i, j));
			}
		}
	}
	return ret;
}

pii GameMap::get_random_empty_point() {
	auto mp = this->get_snake_map();
	pii ret;
	auto width = float_to_int(this->getMapSize().width);
	auto height = float_to_int(this->getMapSize().height);
	int r = width * height / 10;
	do {
		r--;
		if (r < 0) {
			break;
		}
		ret = pii(random(0, width - 1),  random(0, height - 1));
	} while (wall_id(ret)
		|| mp[ret.first][ret.second]
		|| food_id(ret) > 0);
	if (r < 0) {
		auto empty_points = get_all_empty_points();
		if (empty_points.empty()) {
			return pii(-1, -1);
		}
		ret = empty_points[random(0, (int)empty_points.size() - 1)];
	}
	return ret;
}

pii GameMap::get_next_position(pii now, int dir) {
	auto width = float_to_int(this->getMapSize().width);
	auto height = float_to_int(this->getMapSize().height);
	auto x = (now.first + dir_vector[dir].first + width) % width;
	auto y = (now.second + dir_vector[dir].second + height) % height; 
	return pii(x, y);
}

bool GameMap::is_empty(pii pos, int delay, Snake *ignore) {
	if (pos.first < 0 || pos.first >= max_game_width || pos.second < 0 || pos.second >= max_game_height) {
		log("********************************************************************************************");
		return false;
	}
	if (wall_id(pos)) {
		return false;
	}
	auto sp = snake_map[pos.first][pos.second];
	if (sp) {
		auto snake = (Snake*)sp->getParent();
		if (snake->get_is_died()) {
			return false;
		}
		if (snake == ignore) {
			return true;
		}
		int length = sp->getTag() - snake->get_tail_time_stamp() + 1;
		int steps = length * Snake::step_length - snake->get_step();
		auto time_snake = (steps + snake->get_speed() - 1) / snake->get_speed();
		if (time_snake > delay) {
			return false;
		}
	}
	return true;
}