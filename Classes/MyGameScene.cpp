#include "MyGameScene.h"
#include "MissionSprite.h"
#include "Snake.h"
#include "SmartSnake.h"
#include "GameMenuScene.h"

USING_NS_CC;
using namespace std;

MyGame * MyGame::create(int mission_id) {
	MyGame *pRet = new(std::nothrow) MyGame();
	if (pRet && pRet->init(mission_id)) {
		pRet->autorelease();
		return pRet;
	}
	else {
		delete pRet;
		pRet = nullptr;
		return nullptr;
	}
	return nullptr;
}

Scene* MyGame::createScene(int mission_id) {
	auto scene = Scene::create();
	auto layer = MyGame::create(mission_id);
	if (!layer) {
		return nullptr;
	}
	scene->addChild(layer);
	return scene;
}

bool MyGame::init(int mission_id) {
	if (!Layer::init()) {
		return false;
	}
	log("my game init");
	auto mission = Mission::create(mission_id);
	if (!mission) {
		return false;
	}
	score = 0;
	heart = 3;
	pause_n = 3;
	max_hunger = 50;
	this->setTag(mission_id);
	game_map = mission->get_game_map();
	game_map->setTag(mission_id);
	sprite_ban = Sprite::create("ban.png");
	sprite_ban->setVisible(false);
	this->addChild(sprite_ban, 100);
	log("%d", game_map);
	this->addChild(game_map);
	log("test");
	auto bug_v = game_map->getProperty("bug");
	auto flower_v = game_map->getProperty("flower");
	auto kill_v = game_map->getProperty("kill");
	if (bug_v.isNull()) {
		bug_v = 0;
	}
	if (flower_v.isNull()) {
		flower_v = 0;
	}
	if (kill_v.isNull()) {
		kill_v = 0;
	}
	bug = bug_v.asInt();
	flower = flower_v.asInt();
	kill = kill_v.asInt();
	auto snake_obj_group = game_map->getObjectGroup("snake_objs");
	CCASSERT(snake_obj_group, "snake_objs has not defined!");
	food_layer = game_map->getLayer("food");
	CCASSERT(food_layer, "food_layer has not defined!");
	auto snake_objs = snake_obj_group->getObjects();
	snakes.clear();
	snakes.push_back(Snake::create("player", game_map));
	CCASSERT(snakes[0], "snakes[0] has not defined!");
	for (auto snake_v : snake_objs) {
		auto snake_vm = snake_v.asValueMap();
		auto snake_name = snake_vm["name"].asString();
		if (snake_name != "player") {
			auto snake_type = snake_vm["type"].asString();
			if (snake_type == "player") {
				snakes.push_back(Snake::create(snake_name, game_map));
			}
			else {
				auto snake = SmartSnake::create(snake_name, game_map);
				if (snake_type == "follow") {
					snake->type = Snake::SnakeType::t_follow;
				}
				else {
					snake->type = Snake::SnakeType::t_enemy;
				}
				snakes.push_back(snake);
			}
		}
	}
	float x = origin.x + 20, y = origin.y + visible_size.height;
	auto label_goal = Label::createWithSystemFont(get_UTF8_string("goal"), "abc", MID_LABEL_FONT_SIZE);
	label_goal->setAnchorPoint(Vec2(0, 1));
	label_goal->setPosition(0, y);
	this->addChild(label_goal);
	y -= label_goal->getContentSize().height + 20;
	if (kill > 0) {
		auto sprite = Sprite::create("snake.png");
		sprite->setAnchorPoint(Vec2(0, 1));
		sprite->setPosition(x, y);
		auto label = Label::createWithSystemFont(" x" + Value(kill).asString(), "abc", SMALL_LABEL_FONT_SIZE);
		label->setAnchorPoint(Vec2(0, 1));
		label->setPosition(x + sprite->getContentSize().width, y);
		label->setName("label_kill");
		y -= max(sprite->getContentSize().height, label->getContentSize().height) + 5;
		this->addChild(sprite);
		this->addChild(label);
	}
	if (bug > 0) {
		auto sprite = Sprite::create("bug.png");
		sprite->setAnchorPoint(Vec2(0, 1));
		sprite->setPosition(x, y);
		auto label = Label::createWithSystemFont(" x" + Value(bug).asString(), "abc", SMALL_LABEL_FONT_SIZE);
		label->setAnchorPoint(Vec2(0, 1));
		label->setPosition(x + sprite->getContentSize().width, y);
		label->setName("label_bug");
		y -= max(sprite->getContentSize().height, label->getContentSize().height) + 5;
		this->addChild(sprite);
		this->addChild(label);
	}
	if (flower > 0) {
		auto sprite = Sprite::create("flower.png");
		sprite->setAnchorPoint(Vec2(0, 1));
		sprite->setPosition(x, y);
		auto label = Label::createWithSystemFont(" x" + Value(flower).asString(), "abc", SMALL_LABEL_FONT_SIZE);
		label->setAnchorPoint(Vec2(0, 1));
		label->setPosition(x + sprite->getContentSize().width, y);
		label->setName("label_flower");
		y -= max(sprite->getContentSize().height, label->getContentSize().height) + 5;
		this->addChild(sprite);
		this->addChild(label);
	}
	y -= 40;

	auto label_state = Label::createWithSystemFont(get_UTF8_string("state"), "abc", MID_LABEL_FONT_SIZE);
	label_state->setAnchorPoint(Vec2(0, 1));
	label_state->setPosition(0, y);
	this->addChild(label_state);
	y -= label_state->getContentSize().height + 20;

	auto sprite = Sprite::create("heart.png");
	sprite->setAnchorPoint(Vec2(0, 1));
	sprite->setPosition(x, y);
	sprite->setName("sprite_heart");
	this->addChild(sprite);
	auto label_heart = Label::createWithSystemFont(" x" + Value(heart).asString(), "abc", SMALL_LABEL_FONT_SIZE);
	label_heart->setAnchorPoint(Vec2(0, 1));
	label_heart->setPosition(x + sprite->getContentSize().width + 5, y);
	label_heart->setName("label_heart");
	this->addChild(label_heart);
	y -= sprite->getContentSize().height + 5;

	auto label_hunger = Label::createWithSystemFont(get_UTF8_string("hunger") + "0/" + Value(max_hunger).asString(), "abc", SMALL_LABEL_FONT_SIZE);
	label_hunger->setAnchorPoint(Vec2(0, 1));
	label_hunger->setPosition(x, y);
	label_hunger->setName("label_hunger");
	this->addChild(label_hunger);
	y -= label_hunger->getContentSize().height + 5;

	auto label_score = Label::createWithSystemFont(get_UTF8_string("score") + "0", "abc", SMALL_LABEL_FONT_SIZE);
	label_score->setAnchorPoint(Vec2(0, 1));
	label_score->setPosition(x, y);
	label_score->setName("label_score");
	this->addChild(label_score);
	y -= label_score->getContentSize().height + 5;

	x = origin.x + visible_size.width - 8 * UNIT + 10; 
	y = origin.y + visible_size.height - 10;
	for (int i = 0; i < foods_num; i++) {
		int gid = i + 1;
		auto v = game_map->getPropertiesForGID(gid).asValueMap();
		if (v.count("max_num") == 0) {
			v["max_num"] = 0;
		}
		remain_num[i] = v["max_num"].asInt();
		if (v.count("cooldown") == 0) {
			v["cooldown"] = 0;
		}
		cooldown[i] = max(v["cooldown"].asFloat(), 0.1f);
		current_cooldown[i] = 0;
		auto sp = Sprite::create("progress.png");
		progress_length = sp->getContentSize().width;
		if (remain_num[i] >= 0) {
			auto sprite = Sprite::create("foods.png", Rect(i % 4 * UNIT, i / 4 * UNIT, UNIT, UNIT));
			sprite->setAnchorPoint(Vec2(0, 1));
			sprite->setPosition(x, y);
			y -= UNIT + 5;
			this->addChild(sprite);
			sprite = Sprite::create("progress.png");
			sprite->setAnchorPoint(Vec2(0, 1));
			sprite->setPosition(x, y);
			//y -= sprite->getContentSize().height + 5;
			this->addChild(sprite);
			sprite = Sprite::create("progress_f.png");
			sprite->setAnchorPoint(Vec2(0, 1));
			sprite->setPosition(x, y);
			y -= sprite->getContentSize().height + 10;
			sprite->setVisible(false);
			progress[i] = sprite;
			this->addChild(sprite);
		}
	}
	y = origin.y;
	menu_back = MenuItemFont::create(get_UTF8_string("abandon"), [this](Ref *sender) {
		string id_string = Value(this->getTag()).asString();
		user_info["mission_challenge" + id_string] = user_info["mission_challenge" + id_string].asInt() + 1;
		FileUtils::getInstance()->writeValueMapToFile(user_info, "res/user_info.xml");
		auto next_scene = GameMenu::createScene();
		auto Transition_scene = TransitionCrossFade::create(SCENE_TURN_TRANSITION_TIME, next_scene);
		Director::getInstance()->replaceScene(Transition_scene);
	});
	menu_back->setAnchorPoint(Vec2(1, 0));
	menu_back->setPosition(origin.x + visible_size.width, y);
	y += menu_back->getContentSize().height + 10;
	menu_again = MenuItemFont::create(get_UTF8_string("again"), [this](Ref *sender) {
		string id_string = Value(this->getTag()).asString();
		user_info["mission_challenge" + id_string] = user_info["mission_challenge" + id_string].asInt() + 1;
		FileUtils::getInstance()->writeValueMapToFile(user_info, "res/user_info.xml");
		auto next_scene = MyGame::createScene(this->getTag());
		auto Transition_scene = TransitionCrossFade::create(SCENE_TURN_TRANSITION_TIME, next_scene);
		Director::getInstance()->replaceScene(Transition_scene);
	});
	menu_again->setAnchorPoint(Vec2(0, 0));
	menu_again->setPosition(x, origin.y);
	auto font_size = MenuItemFont::getFontSize();
	MenuItemFont::setFontSize(BIG_LABEL_FONT_SIZE + 10);
	menu_pause = MenuItemToggle::createWithCallback([this](Ref *ref) {
		if (isUpdate) {
			if (pause_n <= 0) {
				game_map->setVisible(false);
			}
			else {
				add_pause_n(-1);
				auto label = (Label*)this->getChildByName("label_pause");
				label->setString(" x" + Value(this->get_pause_n()).asString());
			}
		}
		else {
			sprite_ban->stopAllActions();
			sprite_ban->setVisible(false);
		}
		set_pause(isUpdate);
	},
		MenuItemFont::create(get_UTF8_string("pause")),
		MenuItemFont::create(get_UTF8_string("go on")),
		NULL
		);
	MenuItemFont::setFontSize(font_size);
	auto menu_start = MenuItemFont::create(get_UTF8_string("start"), [this](Ref *sender) {
		((Node*)sender)->setVisible(false);
		menu_pause->setVisible(true);
		menu_again->setVisible(false);
		menu_back->setVisible(false);
		auto label = (Label*)this->getChildByName("label_pause");
		label->setVisible(true);
		scheduleUpdate();
	});
	menu_start->setAnchorPoint(Vec2(0, 0));
	menu_start->setPosition(x, y);
	menu_pause->setAnchorPoint(Vec2(0, 0));
	menu_pause->setPosition(x, y);
	menu_pause->setVisible(false);
	auto label_pause = Label::createWithSystemFont(" x" + Value(pause_n).asString(), "abc", SMALL_LABEL_FONT_SIZE);
	label_pause->setName("label_pause");
	label_pause->setAnchorPoint(Vec2(0, 0));
	label_pause->setPosition(menu_pause->getPosition() + Vec2(menu_pause->getContentSize().width, 0));
	label_pause->setVisible(false);
	this->addChild(label_pause);
	turn_1 = Sprite::create("arrow.png");
	turn_2 = Sprite::create("arrow.png");
	turn_1->setPosition(menu_pause->getPosition() + Vec2(0, menu_pause->getContentSize().height + 10) + turn_1->getContentSize() / 2);
	turn_2->setPosition(turn_1->getPosition() + Vec2(turn_1->getContentSize().width + 10, 0));
	this->addChild(turn_1);
	this->addChild(turn_2);
	menu_clear_dir = MenuItemFont::create(" X ", [this](Ref *sender) {
		snakes[0]->turn_1 = -1;
		snakes[0]->turn_2 = -1;
		update_dir();
	});
	//menu_clear_dir->setAnchorPoint(Vec2(0, 0));
	menu_clear_dir->setPosition(turn_2->getPosition() + Vec2(turn_2->getContentSize().width + 10, 0));
	update_dir();
	menu = Menu::create(menu_back, menu_pause, menu_again, menu_clear_dir, menu_start, NULL);
	menu->setAnchorPoint(Vec2::ZERO);
	menu->setPosition(Vec2::ZERO);
	this->addChild(menu);

	isUpdate = true;

	auto layer = LayerColor::create(Color4B::GRAY, game_map->getContentSize().width, game_map->getContentSize().height);
	if (layer->isIgnoreAnchorPointForPosition()) {
		layer->ignoreAnchorPointForPosition(false);
	}
	layer->setAnchorPoint(game_map->getAnchorPoint());
	layer->setPosition(game_map->getPosition());
	this->addChild(layer, -1);

	//add listener
	listener_touch = EventListenerTouchOneByOne::create();
	Vec2 *touch_begin = new Vec2();
	listener_touch->onTouchBegan = [touch_begin](Touch *t, Event *e) {
		auto position = t->getLocation();
		*touch_begin = position;
		return true;
	};
	auto set_dir = [](Vec2 v) {
		if (abs(v.x) > abs(v.y)) {
			if (v.x > 0) {
				return DIRECTION::RIGHT;
			}
			return DIRECTION::LEFT;
		}
		if (v.y > 0) {
			return DIRECTION::UP;
		}
		return DIRECTION::DOWN;
	};
	listener_touch->onTouchEnded = [this, set_dir](Touch *t, Event *e) {
		if (Rect(Vec2::ZERO, game_map->getContentSize()).containsPoint(game_map->convertToNodeSpace(t->getStartLocation()))) {
			auto control_mode = user_info["control_mode"].asInt();
			if (control_mode == 0 || control_mode == 2) {
				auto v = t->getLocation() - t->getStartLocation();
				if (v.isZero()) {
					v = t->getLocation() - (origin + visible_size / 2);
					DIRECTION dir = set_dir(v);
					for (auto snake : snakes) {
						auto snake_type = snake->get_type();
						if (snake_type == Snake::SnakeType::t_player || snake_type == Snake::SnakeType::t_follow) {
							snake->turn(dir);
						}
					}
					update_dir();
				}
			}
		}
		return true;
	};
	listener_touch->onTouchMoved = [this, set_dir, touch_begin](Touch *t, Event *e) {
		auto control_mode = user_info["control_mode"].asInt();
		if (control_mode == 0 || control_mode == 1) {
			auto pos = t->getLocation();
			auto start_pre = t->getPreviousLocation() - *touch_begin;
			auto pre_now = pos - t->getPreviousLocation();
			if (!start_pre.isZero() && abs(Vec2::angle(start_pre, pre_now)) > acos(-1.0f) / 4) {
				*touch_begin = pos;
			}
			if (fabs(pos.x - touch_begin->x) > touch_move_len || fabs(pos.y - touch_begin->y) > touch_move_len) {
				DIRECTION dir = set_dir(pos - *touch_begin);
				for (auto snake : snakes) {
					auto snake_type = snake->get_type();
					if (snake_type == Snake::SnakeType::t_player || snake_type == Snake::SnakeType::t_follow) {
						snake->turn(dir);
					}
				}
				update_dir();
				*touch_begin = pos;
			}
		}
	};
	listener_key = EventListenerKeyboard::create();
	listener_key->onKeyPressed = [this](EventKeyboard::KeyCode key, Event *e) {
		DIRECTION dir;
		switch (key) {
		case EventKeyboard::KeyCode::KEY_UP_ARROW:
		case EventKeyboard::KeyCode::KEY_W:
		case EventKeyboard::KeyCode::KEY_CAPITAL_W:
			dir = DIRECTION::UP;
			break;
		case EventKeyboard::KeyCode::KEY_DOWN_ARROW:
		case EventKeyboard::KeyCode::KEY_S:
		case EventKeyboard::KeyCode::KEY_CAPITAL_S:
			dir = DIRECTION::DOWN;
			break;
		case EventKeyboard::KeyCode::KEY_LEFT_ARROW:
		case EventKeyboard::KeyCode::KEY_A:
		case EventKeyboard::KeyCode::KEY_CAPITAL_A:
			dir = DIRECTION::LEFT;
			break;
		case EventKeyboard::KeyCode::KEY_RIGHT_ARROW:
		case EventKeyboard::KeyCode::KEY_D:
		case EventKeyboard::KeyCode::KEY_CAPITAL_D:
			dir = DIRECTION::RIGHT;
			break;
		default:
			return;
		}
		for (auto snake : snakes) {
			auto snake_type = snake->get_type();
			if (snake_type == Snake::SnakeType::t_player || snake_type == Snake::SnakeType::t_follow) {
				snake->turn(dir);
			}
		}
		update_dir();
	};
	Director::getInstance()->getEventDispatcher()->addEventListenerWithSceneGraphPriority(listener_touch, this);
	Director::getInstance()->getEventDispatcher()->addEventListenerWithSceneGraphPriority(listener_key, this);
	return true;
}

void MyGame::update(float dt) {
	if (bug <= 0 && flower <= 0 && kill <= 0) {
		game_over(win);
		return;
	}
	bool player_go = false;
	if (snakes[0]->get_step() >= Snake::step_length) {
		player_go = true;
		int dir = snakes[0]->get_current_dir();
		if (snakes[0]->turn_1 >= 0) {
			dir = snakes[0]->turn_1;
		}
		bool is_died = true;
		bool can_go = false;
		for (int i = 0; i < 4; i++) {
			if (abs(i - snakes[0]->get_current_dir()) == 2) {
				continue;
			}
			if (game_map->is_empty(game_map->get_next_position(snakes[0]->get_position(), i))) {
				is_died = false;
				if (i == dir) {
					can_go = true;
				}
			}
		}
		if (is_died) {
			game_over(no_way);
		}
		if (!can_go && get_heart() > 0) {
			add_heart(-1);
			auto label_heart = (Label*)this->getChildByName("label_heart");
			label_heart->setString(" x" + Value(this->get_heart()).asString());
			Device::vibrate(0.5);
			sprite_ban->setPosition(snakes[0]->convertToWorldSpace(
				snakes[0]->get_snake_nodes()->back()->getPosition() + UNIT * Vec2(dir_vector[dir].first, -dir_vector[dir].second)));
			auto action = Blink::create(2, 5);
			sprite_ban->setVisible(true);
			sprite_ban->runAction(action);
			//snakes[0]->turn_1 = snakes[0]->turn_2 = -1;
			set_pause(true);
			//update_dir();
			return;
		}
	}
	auto fps = Director::getInstance()->getAnimationInterval();
	for (int i = 0; i < foods_num; i++) {
		if (remain_num[i] > 0) {
			current_cooldown[i] += fps;
			if (current_cooldown[i] > cooldown[i]) {
				current_cooldown[i] -= cooldown[i];
				remain_num[i]--;
				auto pos = game_map->get_random_empty_point();
				if (pos.first >= 0) {
					food_layer->setTileGID(i + 1, Vec2(pos.first, pos.second));
				}
			}
			float width = current_cooldown[i] / cooldown[i] * progress_length;
			if (width < 0.5 || remain_num[i] == 0) {
				progress[i]->setVisible(false);
			}
			else {
				progress[i]->setVisible(true);
				progress[i]->setScaleX(width / progress[i]->getContentSize().width);
			}
		}
	}
	auto time_b = clock();
	for (auto snake : snakes) {
		snake->go_step();
	}
	for (auto snake : snakes) {
		if (!snake->get_is_checked()) {
			snake->check();
		}
	}
	if (!snakes[0]->get_is_died() && player_go) {
		auto label_hunger = (Label*)this->getChildByName("label_hunger");
		label_hunger->setString(get_UTF8_string("hunger") + Value(snakes[0]->get_hunger()).asString() + "/" + Value(max_hunger).asString());
		bool hungry = false;
		if (snakes[0]->get_hunger() >= max_hunger) {
			snakes[0]->add_hunger(-20);
			this->add_heart(-1);
			hungry = true;
		}
		auto label_heart = (Label*)this->getChildByName("label_heart");
		label_heart->setString(" x" + Value(this->get_heart()).asString());
		if (this->get_heart() < 0) {
			if (hungry) {
				game_over(gameOverState::hungry);
			}
			else {
				game_over(gameOverState::eat_shit);
			}
		}
	}
	time_b = clock() - time_b;
	if(time_b > 10) log("delay = %d", time_b);
}

void MyGame::update_dir() {
	if (snakes[0]->turn_1 >= 0) {
		turn_1->setVisible(true);
		turn_1->setRotation(90 * snakes[0]->turn_1);
		if (snakes[0]->turn_2 >= 0) {
			turn_2->setVisible(true);
			turn_2->setRotation(90 * snakes[0]->turn_2);
		}
		else {
			turn_2->setVisible(false);
		}
		menu_clear_dir->setVisible(true);
	}
	else {
		turn_1->setVisible(false);
		turn_2->setVisible(false);
		menu_clear_dir->setVisible(false);
	}
}

void MyGame::game_over(gameOverState state) {
	unscheduleUpdate();
	string id_string = Value(this->getTag()).asString();
	menu_back->setString(get_UTF8_string("back"));
	menu_back->setVisible(true);
	menu_again->setVisible(true);
	menu_pause->setVisible(false);
	turn_1->setVisible(false);
	turn_2->setVisible(false);
	menu_clear_dir->setVisible(false);
	Director::getInstance()->getEventDispatcher()->removeEventListener(listener_key);
	Director::getInstance()->getEventDispatcher()->removeEventListener(listener_touch);
	auto label_pause = (Label*)this->getChildByName("label_pause");
	label_pause->setVisible(false);
	if (state == win) {
		user_info["mission_score" + id_string] = max(get_score(), user_info["mission_score" + id_string].asInt());
		user_info["mission_success" + id_string] = user_info["mission_success" + id_string].asInt() + 1;
		FileUtils::getInstance()->writeValueMapToFile(user_info, "res/user_info.xml");

		auto label = Label::createWithSystemFont(get_UTF8_string("win"), "abc", MID_LABEL_FONT_SIZE);
		label->setColor(Color3B::RED);
		label->setAnchorPoint(menu_pause->getAnchorPoint());
		label->setPosition(menu_pause->getPosition());
		this->addChild(label);

		auto next_mission_scene = MyGame::createScene(this->getTag() + 1);
		if (next_mission_scene) {
			auto menu_next = MenuItemFont::create(get_UTF8_string("next_mission"), [this](Ref* sender) {
				auto next_mission_scene = MyGame::createScene(this->getTag() + 1);
				auto Transition_scene = TransitionCrossFade::create(SCENE_TURN_TRANSITION_TIME, next_mission_scene);
				Director::getInstance()->replaceScene(Transition_scene);
			});
			menu_next->setAnchorPoint(menu_pause->getAnchorPoint());
			menu_next->setPosition(menu_pause->getPosition());
			menu->addChild(menu_next);
			label->setPositionY(menu_next->getPositionY() + menu_next->getContentSize().height + 5);
		}
	}
	else {
		auto label_failed = Label::createWithSystemFont(get_UTF8_string("failed"), "abc", BIG_LABEL_FONT_SIZE);
		label_failed->setColor(Color3B::RED);
		label_failed->setAnchorPoint(menu_pause->getAnchorPoint());
		label_failed->setPosition(menu_pause->getPosition());
		this->addChild(label_failed);
		string str = "i do not know";
		switch (state) {
		case MyGame::hungry:
			str = "starvation";
			break;
		case MyGame::eat_shit:
			str = "eat too much shit";
			break;
		case MyGame::impact_wall:
			str = "impact wall";
			break;
		case MyGame::impact_snake:
			str = "impact snake";
			break;
		case MyGame::no_way:
			str = "no way";
			break;
		case MyGame::win:
			break;
		default:
			break;
		}
		auto label_heart = (Label*)this->getChildByName("label_heart");
		label_heart->setString(get_UTF8_string("broken"));
		auto label_score = (Label*)this->getChildByName("label_score");
		Label* label = Label::createWithSystemFont(get_UTF8_string(str), "abc", SMALL_LABEL_FONT_SIZE);
		log("%s", get_UTF8_string(str).c_str());
		label->setAnchorPoint(Vec2(0, 1));
		label->setPosition(label_score->getPosition() - Vec2(0, label_score->getContentSize().height + 5));
		label->setColor(Color3B::RED);
		this->addChild(label);
	}
}

void MyGame::set_pause(bool pause) {
	if (!(isUpdate ^ pause)) {
		if (pause) {
			auto label = (Label*)this->getChildByName("label_pause");
			label->setVisible(false);
			this->unscheduleUpdate();
		}
		else {
			auto label = (Label*)this->getChildByName("label_pause");
			label->setVisible(true);
			game_map->setVisible(true);
			this->scheduleUpdate();
		}
		menu_back->setVisible(isUpdate);
		menu_again->setVisible(isUpdate);
		menu_pause->setSelectedIndex(isUpdate);
		isUpdate = !isUpdate;
	}
}