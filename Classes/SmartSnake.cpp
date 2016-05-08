#include "SmartSnake.h"
#include "MyGameScene.h"


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
	log("smartsnake init");
	if (!Snake::init(name, game_map)) {
		return false;
	}
	return true;
}

void SmartSnake::eat_reward(int gid) {
	if (this->get_type() == Snake::SnakeType::t_friend && random(0, 1)) {
		Snake::eat_reward(gid);
	}
	hunger = 0;
}

bool SmartSnake::go_die() {
	Snake::go_die();
	auto game = (MyGame*)game_map->getParent();
	if (get_type() == Snake::SnakeType::t_enemy) {
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
	}
	else {
		Device::vibrate(0.4f);
		game->add_heart(-max(1, game->get_heart()));
		if (game->get_heart() == -1) {
			game->add_heart(-2);
		}
		game->print_log(get_UTF8_string("a friend is died"));
	}
	return true;
	game_map->add_empty_n(snake_nodes->size());
	while (!snake_nodes->empty()) {
		new_tail();
	}
	return true;
}

bool SmartSnake::go_ahead() {
	if (get_is_died()) {
		return false;
	}
	if (type == Snake::SnakeType::t_enemy) {
		act();
		if (turn_1 < 0 || !game_map->is_empty(game_map->get_next_position(position, turn_1))) {
			vector<int> dir_v;
			for (int i = 0; i < 4; i++) {
				if (abs(i - current_dir) != 2 && game_map->is_empty(game_map->get_next_position(position, i))) {
					dir_v.push_back(i);
				}
			}
			if (dir_v.size() > 0) {
				turn_1 = dir_v[random(0, (int)dir_v.size() - 1)];
			}
		}
		log("ture dir = %d", turn_1);
	}
	else {
		turn_1 = ((Snake*)getUserObject())->get_current_dir();
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
	if (turn_1 >= 0) {
		return false;
	}
	if (hunger > game_map->get_empty_n() * 2 + get_length()) {
		log("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!hungery");
	}
	int max_lenght;
	auto dir = this->get_target_shortest_path_dir(position, current_dir, max_lenght);
	if (hunger > game_map->get_empty_n() * 2 + get_length() || max_lenght >= 0) {
		turn_1 = dir;
		log("act 1, dir = %d, max_lenght = %d, delay = %d", dir, max_lenght, clock() - begin_time);
		return true;
	}
	max_lenght = 0;
	memset(pre_step_on, -1, sizeof(pre_step_on));
	dir = this->get_accessible_last_snake_node_dir(position, current_dir, max_lenght);
	turn_1 = dir;
	log("act 2, dir = %d, max_lenght = %d, delay = %d", dir, max_lenght, clock() - begin_time);
	if (max_lenght < 0) {
		log("failed! pos = (%d, %d)", position.first, position.second);
	}
	return false;
}

pii SmartSnake::dfs_get_accessible_last_snake_node_dir(pii now, int step, int dir, int first_dir) {
	if (pre_step_on[now.first][now.second] >= 0) {
		step_on[now.first][now.second] = pre_step_on[now.first][now.second];
	}
	if (step_on[now.first][now.second] >= 0) {
		int length = step - step_on[now.first][now.second] - get_length();
		return pii(length, first_dir);
	}
	step_on[now.first][now.second] = step;
	pii ret = pii(-0x3fffff, -1);
	auto sp = game_map->get_snake_map()[now.first][now.second];
	if (sp && sp->getParent() == this && first_dir >= 0) {
		int length = step - (sp->getTag() - this->get_tail_time_stamp() + 1);
		if (length > ret.first) {
			ret = pii(length, first_dir);
		}
		if (length < 0) {
			return ret;
		}
	}
	auto steps = (step + 1) * Snake::step_length - this->get_step();
	auto time_snake = (steps - 1) / this->get_speed() + 1;
	if (steps <= 0) {
		time_snake = 0;
	}
	for (int i = 0; i < 4; i++) {
		int c_dir = dir_f[i];
		if (abs(c_dir - dir) == 2) {
			continue;
		}
		auto nxt = game_map->get_next_position(now, c_dir);
		if (game_map->is_empty(nxt, time_snake, this)) {
			auto _first_dir = first_dir;
			if (_first_dir < 0) {
				_first_dir = c_dir;
			}
			auto tmp = dfs_get_accessible_last_snake_node_dir(nxt, step + 1, c_dir, _first_dir);
			if (tmp.first > ret.first) {
				ret = tmp;
			}
		}
	}
	return ret;
}

int SmartSnake::get_accessible_last_snake_node_dir(pii position, int dir, int &start_step) {
	log("in func");
	dir_f[0] = 0;
	dir_f[1] = 1;
	dir_f[2] = 2;
	dir_f[3] = 3;
	pii ret = pii(-0x3fffff, -1);
	do {
		memset(step_on, -1, sizeof(step_on));
		auto tmp = dfs_get_accessible_last_snake_node_dir(position, start_step, dir, -1);
		if (tmp.first > ret.first) {
			ret = tmp;
		}
	} while (next_permutation(dir_f, dir_f + 4));
	start_step = ret.first;
	return ret.second;
}

int SmartSnake::get_target_shortest_path_dir(pii position, int current_dir, int & max_length) {
	log("in shortest_path");
	max_length = -0x3fffff;
	int ret = -1;
	int dir_f[] = { 0, 1, 2, 3 };
	do {
		int vis[max_game_width][max_game_height];
		memset(vis, -1, sizeof(vis));
		pii pre_position[max_game_width][max_game_height];
		queue<pair<pii, int> > q;
		q.push(make_pair(position, 0));
		vis[position.first][position.second] = current_dir;
		while (!q.empty()) {
			auto now = q.front().first;
			auto step = q.front().second + 1;
			q.pop();
			for (int i = 0; i < 4; i++) {
				int c_dir = dir_f[i];
				int pre_dir = vis[now.first][now.second];
				if (abs(c_dir - pre_dir) == 2) {
					continue;
				}
				auto nxt = game_map->get_next_position(now, c_dir);
				if (vis[nxt.first][nxt.second] >= 0) {
					continue;
				}
				vis[nxt.first][nxt.second] = c_dir;
				pre_position[nxt.first][nxt.second] = now;
				auto food_id = game_map->food_id(nxt);
				if (food_id > 0 && (this->getTag() & (1 << (food_id - 1))) == 0) {
					pii target = nxt;
					log("target = (%d, %d)", target.first, target.second);
					vector<pii> vt;
					int ret_tmp = -1;
					while (target != position) {
						vt.push_back(target);
						ret_tmp = vis[target.first][target.second];
						target = pre_position[target.first][target.second];
					}
					vt.push_back(target);
					target = vt[0];
					memset(pre_step_on, -1, sizeof(pre_step_on));
					int vs = vt.size();
					for (int i = 1; i < vs; i++) {
						pre_step_on[vt[i].first][vt[i].second] = vs - i - 1;
					}
					int length = vs - 1;
					auto tmp = get_accessible_last_snake_node_dir(target, vis[target.first][target.second] - 1, length);
					if (length > 0) {
						max_length = length;
						return ret_tmp;
					}
					log("length = %d, ret_tmp = %d", max_length, ret_tmp);
					if (max_length < length) {
						max_length = length;
						ret = ret_tmp;
					}
				}
				int steps = step * Snake::step_length - this->get_step();
				int time_s = (steps - 1) / this->get_speed() + 1;
				if (steps <= 0) {
					time_s = 0;
				}
				if (!game_map->is_empty(nxt, time_s)) {
					continue;
				}
				q.push(make_pair(nxt, step));
			}
		}
	} while (next_permutation(dir_f, dir_f + 4));
	return ret;
}