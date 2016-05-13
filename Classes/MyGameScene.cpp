#include "MyGameScene.h"
#include "MissionSprite.h"
#include "Snake.h"
#include "SmartSnake.h"
#include "GameMenuScene.h"
#include "SimpleAudioEngine.h"

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
	log("my game init");
	if (!Layer::init()) {
		return false;
	}
	auto mission = Mission::create(mission_id);
	if (!mission) {
		return false;
	}
	score = 0;
	heart = 3;
	pause_n = 1;
	max_hunger = 50;
	this->setTag(mission_id);
	game_map = mission->get_game_map();
	game_map->setTag(mission_id);
	sprite_ban = Sprite::create("ban.png");
	sprite_ban->setVisible(false);
	this->addChild(sprite_ban, 100);
	this->addChild(game_map);
	auto apple_v = game_map->getProperty("apple");
	auto bug_v = game_map->getProperty("bug");
	auto flower_v = game_map->getProperty("flower");
	auto kill_v = game_map->getProperty("kill");
	if (apple_v.isNull()) {
		apple_v = 0;
	}
	if (bug_v.isNull()) {
		bug_v = 0;
	}
	if (flower_v.isNull()) {
		flower_v = 0;
	}
	if (kill_v.isNull()) {
		kill_v = 0;
	}
	apple = apple_v.asInt();
	bug = bug_v.asInt();
	flower = flower_v.asInt();
	kill = kill_v.asInt();
	auto snake_obj_group = game_map->getObjectGroup("snake_objs");
	CCASSERT(snake_obj_group, "snake_objs has not defined!");
	auto snake_objs = snake_obj_group->getObjects();
	snakes.clear();
	snakes.push_back(Snake::create("player", game_map));
	//CCASSERT(snakes[0], "snakes[0] has not defined!");
	has_player = true;
	if (!snakes[0]) {
		snakes.clear();
		has_player = false;
		log("player has not defined!");
	}
	for (auto snake_v : snake_objs) {
		auto snake_vm = snake_v.asValueMap();
		auto snake_name = snake_vm["name"].asString();
		if (snake_name != "player") {
			auto snake_type = snake_vm["type"].asString();
			auto snake = SmartSnake::create(snake_name, game_map);
			if (snake_type == "friend") {
				snake->type = Snake::SnakeType::t_friend;
			}
			else {
				snake->type = Snake::SnakeType::t_enemy;
			}
			if (has_player) {
				snake->setUserObject(snakes[0]);
			}
			snakes.push_back(snake);
		}
	}
	if (has_player) {
		set_UI();
		set_Ctrl();
	}

	scheduleUpdate();

	return true;
}

void MyGame::update(float dt) {
	if (has_player && apple <= 0 && bug <= 0 && flower <= 0 && kill <= 0) {
		game_over(win);
		return;
	}
	bool player_go = false;
	if (has_player && snakes[0]->get_step() >= Snake::step_length) {
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
			return;
		}
		if (!can_go && get_heart() > 0) {
			add_heart(-1);
			print_log(get_UTF8_string("can not go"));
			snakes[0]->turn_1 = snakes[0]->turn_2 = -1;
			if (user_info["soundEffects"].asInt() == 0) {
				CocosDenshion::SimpleAudioEngine::getInstance()->playEffect("pang.wav");
			}
			Device::vibrate(0.5);
			auto label_heart = (Label*)this->getChildByName("label_heart");
			label_heart->setString(" x" + Value(this->get_heart()).asString());
			sprite_ban->setPosition(snakes[0]->convertToWorldSpace(
				snakes[0]->get_snake_nodes()->back()->getPosition() + UNIT * Vec2(dir_vector[dir].first, -dir_vector[dir].second)));
			auto action = Blink::create(2, 5);
			sprite_ban->setVisible(true);
			sprite_ban->runAction(action);
			//snakes[0]->turn_1 = snakes[0]->turn_2 = -1;
			set_pause(true);
			update_dir();
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
					game_map->get_food()->setTileGID(i + 1, Vec2(pos.first, pos.second));
				}
			}
			if (has_player) {
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
	}
	auto time_b = clock();
	for (auto snake : snakes) {
		snake->go_step();
	}
	snakes[0]->check();
	if (snakes[0]->get_is_died()) {
		return;
	}
	for (auto snake : snakes) {
		if (!snake->get_is_checked()) {
			snake->check();
		}
	}
	int alive = 0;
	for (auto snake : snakes) {
		if (!snake->get_is_died()) {
			alive++;
		}
	}
	if (!has_player && alive == 0) {
		this->removeAllChildren();
		this->init(this->getTag());
	}
	if (has_player && !snakes[0]->get_is_died() && player_go) {
		update_dir();
		auto label_hunger = (Label*)this->getChildByName("label_hunger");
		label_hunger->setString(get_UTF8_string("hunger")
			+ Value(snakes[0]->get_hunger()).asString() + "/" + Value(max_hunger).asString());
		if (snakes[0]->get_hunger() >= max_hunger) {
			snakes[0]->add_hunger(-20);
			this->add_heart(-1);
			if (this->get_heart() == -1) {
				this->add_heart(-1);
			}
			print_log(get_UTF8_string("hungry"));
			Device::vibrate(0.2f);
			if (user_info["soundEffects"].asInt() == 0) {
				CocosDenshion::SimpleAudioEngine::getInstance()->playEffect("hunger.wav");
			}
		}
		auto label_heart = (Label*)this->getChildByName("label_heart");
		label_heart->setString(" x" + Value(this->get_heart()).asString());
		if (this->get_heart() < 0) {
			if (this->get_heart() == -3) {
				game_over(gameOverState::friend_die);
			}
			else if (this->get_heart() == -2) {
				game_over(gameOverState::hungry);
			}
			else {
				game_over(gameOverState::eat_shit);
			}
			return;
		}
	}
	time_b = clock() - time_b;
	if (time_b > 20) {
		("delay = %d", time_b);
	}
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
	Director::getInstance()->getEventDispatcher()->removeEventListener(listener_touch);
	Director::getInstance()->getEventDispatcher()->removeEventListener(listener_key);
	CocosDenshion::SimpleAudioEngine::getInstance()->stopBackgroundMusic();
	string id_string = Value(this->getTag()).asString();
	menu_back->setString(get_UTF8_string("back"));
	menu_back->setVisible(true);
	menu_again->setVisible(true);
	menu_pause->setVisible(false);
	turn_1->setVisible(false);
	turn_2->setVisible(false);
	menu_clear_dir->setVisible(false);
	snakes[0]->set_is_died(true);
	Director::getInstance()->getEventDispatcher()->removeEventListener(listener_key);
	Director::getInstance()->getEventDispatcher()->removeEventListener(listener_touch);
	if (state == win) {
		if (user_info["soundEffects"].asInt() == 0) {
			CocosDenshion::SimpleAudioEngine::getInstance()->playEffect("win.wav");
		}
		auto pre_score = user_info["mission_score" + id_string].asInt();
		auto str = get_UTF8_string("complete");
		add_score(get_heart() * 300);
		add_score((get_max_hunger() - snakes[0]->get_hunger()) * 5); 
		auto label_score = (Label*)getChildByName("label_score");
		label_score->setString(get_UTF8_string("score") + Value(get_score()).asString());
		if (get_score() > pre_score) {
			user_info["mission_score" + id_string] = get_score();
			if (pre_score > 0) {
				if (user_info["soundEffects"].asInt() == 0) {
					CocosDenshion::SimpleAudioEngine::getInstance()->playEffect("yaho.wav");
				}
				str = get_UTF8_string("complete") + get_UTF8_string("new_max_score")
					+ Value(pre_score).asString() + "-->" + Value(get_score()).asString();
			}
		}
		print_log(str);
		user_info["mission_success" + id_string] = user_info["mission_success" + id_string].asInt() + 1;
		user_info["current_mission"] = max(this->getTag() + 1, user_info["current_mission"].asInt());
		FileUtils::getInstance()->writeValueMapToFile(user_info, writable_path + "user_info.xml");

		auto label = Label::createWithTTF(get_UTF8_string("win"), "font.ttf", MID_LABEL_FONT_SIZE);
		label->setColor(Color3B::RED);
		label->setAnchorPoint(menu_pause->getAnchorPoint());
		label->setPosition(menu_pause->getPosition());
		this->addChild(label);

		auto next_mission = Mission::create(this->getTag() + 1);
		if (next_mission) {
			auto menu_next = MenuItemFont::create(get_UTF8_string("next_mission"), [this](Ref* sender) {
				if (user_info["soundEffects"].asInt() == 0) {
					CocosDenshion::SimpleAudioEngine::getInstance()->playEffect("button.wav");
				}
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
		if (user_info["soundEffects"].asInt() == 0) {
			CocosDenshion::SimpleAudioEngine::getInstance()->playEffect("lose.wav");
		}
		auto label_failed = Label::createWithTTF(get_UTF8_string("failed"), "font.ttf", BIG_LABEL_FONT_SIZE);
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
		case MyGame::friend_die:
			str = "friend die";
			break;
		case MyGame::win:
			break;
		default:
			break;
		}
		auto label_heart = (Label*)this->getChildByName("label_heart");
		label_heart->setString(get_UTF8_string("broken"));
		print_log(get_UTF8_string(str));
	}
}

void MyGame::print_log(string str) {
	auto label_score = (Label*)this->getChildByName("label_score");
	if (!label_score) {
		return;
	}
	auto label_log = (Label*)this->getChildByName("label_log");
	if (!label_log) {
		label_log = Label::createWithTTF("", "font.ttf", SMALL_LABEL_FONT_SIZE);
		label_log->setName("label_log");
		this->addChild(label_log);
	}
	label_log->setString(str);
	//log("%s", get_UTF8_string(str).c_str());
	label_log->setAnchorPoint(Vec2(0, 1));
	label_log->setPosition(label_score->getPosition() - Vec2(0, label_score->getContentSize().height + 5));
	label_log->setColor(Color3B::RED);
}

void MyGame::set_pause(bool pause) {
	if (!pause) {
		sprite_ban->stopAllActions();
		sprite_ban->setVisible(false);
	}
	if (!(isUpdate ^ pause)) {
		if (pause) {
			this->unscheduleUpdate();
		}
		else {
			game_map->setVisible(true);
			this->scheduleUpdate();
		}
		menu_back->setVisible(isUpdate);
		menu_again->setVisible(isUpdate);
		menu_pause->setVisible(!isUpdate);
		((Node*)menu_pause->getUserObject())->setVisible(isUpdate);
		isUpdate = !isUpdate;
	}
}

void MyGame::set_UI() {
	float x = origin.x + 20, y = origin.y + visible_size.height;
	auto label_goal = Label::createWithTTF(get_UTF8_string("goal"), "font.ttf", MID_LABEL_FONT_SIZE);
	label_goal->setAnchorPoint(Vec2(0, 1));
	label_goal->setPosition(0, y);
	this->addChild(label_goal);
	y -= label_goal->getContentSize().height + 20;
	if (apple > 0) {
		auto sprite = Sprite::create("apple.png");
		sprite->setAnchorPoint(Vec2(0, 1));
		sprite->setPosition(x, y);
		auto label = Label::createWithTTF(" x" + Value(apple).asString(), "font.ttf", SMALL_LABEL_FONT_SIZE);
		label->setAnchorPoint(Vec2(0, 1));
		label->setPosition(x + sprite->getContentSize().width, y);
		label->setName("label_apple");
		y -= max(sprite->getContentSize().height, label->getContentSize().height) + 5;
		this->addChild(sprite);
		this->addChild(label);
	}
	if (kill > 0) {
		auto sprite = Sprite::create("snake.png");
		sprite->setAnchorPoint(Vec2(0, 1));
		sprite->setPosition(x, y);
		auto label = Label::createWithTTF(" x" + Value(kill).asString(), "font.ttf", SMALL_LABEL_FONT_SIZE);
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
		auto label = Label::createWithTTF(" x" + Value(bug).asString(), "font.ttf", SMALL_LABEL_FONT_SIZE);
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
		auto label = Label::createWithTTF(" x" + Value(flower).asString(), "font.ttf", SMALL_LABEL_FONT_SIZE);
		label->setAnchorPoint(Vec2(0, 1));
		label->setPosition(x + sprite->getContentSize().width, y);
		label->setName("label_flower");
		y -= max(sprite->getContentSize().height, label->getContentSize().height) + 5;
		this->addChild(sprite);
		this->addChild(label);
	}
	y -= 40;

	auto label_state = Label::createWithTTF(get_UTF8_string("state"), "font.ttf", MID_LABEL_FONT_SIZE);
	label_state->setAnchorPoint(Vec2(0, 1));
	label_state->setPosition(0, y);
	this->addChild(label_state);
	y -= label_state->getContentSize().height + 20;

	auto sprite = Sprite::create("heart.png");
	sprite->setAnchorPoint(Vec2(0, 1));
	sprite->setPosition(x, y);
	sprite->setName("sprite_heart");
	this->addChild(sprite);
	auto label_heart = Label::createWithTTF(" x" + Value(heart).asString(), "font.ttf", SMALL_LABEL_FONT_SIZE);
	label_heart->setAnchorPoint(Vec2(0, 1));
	label_heart->setPosition(x + sprite->getContentSize().width + 5, y);
	label_heart->setName("label_heart");
	this->addChild(label_heart);
	y -= sprite->getContentSize().height + 5;

	auto label_hunger = Label::createWithTTF(get_UTF8_string("hunger") + "0/" + Value(max_hunger).asString(), "font.ttf", SMALL_LABEL_FONT_SIZE);
	label_hunger->setAnchorPoint(Vec2(0, 1));
	label_hunger->setPosition(x, y);
	label_hunger->setName("label_hunger");
	this->addChild(label_hunger);
	y -= label_hunger->getContentSize().height + 5;

	auto label_score = Label::createWithTTF(get_UTF8_string("score") + "0", "font.ttf", SMALL_LABEL_FONT_SIZE);
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

	if (user_info["music"].asInt() == 0) {
		log("play");
		CocosDenshion::SimpleAudioEngine::getInstance()->playBackgroundMusic("play.wav", true);
	}
}

void MyGame::set_Ctrl() {
	string id_string = Value(this->getTag()).asString();
	user_info["mission_challenge" + id_string] = user_info["mission_challenge" + id_string].asInt() + 1;
	FileUtils::getInstance()->writeValueMapToFile(user_info, writable_path + "user_info.xml");
	float x = origin.x + visible_size.width - 8 * UNIT + 10;
	float y = origin.y;
	menu_back = MenuItemFont::create(get_UTF8_string("abandon"), [this](Ref *sender) {
		CocosDenshion::SimpleAudioEngine::getInstance()->stopBackgroundMusic();
		if (user_info["soundEffects"].asInt() == 0) {
			CocosDenshion::SimpleAudioEngine::getInstance()->playEffect("button.wav");
		}
		auto next_scene = GameMenu::createScene();
		auto Transition_scene = TransitionCrossFade::create(SCENE_TURN_TRANSITION_TIME, next_scene);
		Director::getInstance()->replaceScene(Transition_scene);
	});
	menu_back->setAnchorPoint(Vec2(1, 0));
	menu_back->setPosition(origin.x + visible_size.width, y);
	y += menu_back->getContentSize().height + 10;
	menu_again = MenuItemFont::create(get_UTF8_string("again"), [this](Ref *sender) {
		if (user_info["soundEffects"].asInt() == 0) {
			CocosDenshion::SimpleAudioEngine::getInstance()->playEffect("button.wav");
		}
		auto next_scene = MyGame::createScene(this->getTag());
		auto Transition_scene = TransitionCrossFade::create(SCENE_TURN_TRANSITION_TIME, next_scene);
		Director::getInstance()->replaceScene(Transition_scene);
	});
	menu_again->setAnchorPoint(Vec2(0, 0));
	menu_again->setPosition(x, origin.y);
	auto font_size = MenuItemFont::getFontSize();
	MenuItemFont::setFontSize(BIG_LABEL_FONT_SIZE + 10);

	menu_pause = MenuItemFont::create(get_UTF8_string("pause"), [this](Ref *ref) {
		auto sender = (MenuItemFont*)ref;
		if (user_info["soundEffects"].asInt() == 0) {
			CocosDenshion::SimpleAudioEngine::getInstance()->playEffect("button.wav");
		}
		if (pause_n <= 0) {
			game_map->setVisible(false);
		}
		else {
			add_pause_n(-1);
			auto label = (Label*)sender->getChildByName("label_pause");
			label->setString(" x" + Value(this->get_pause_n()).asString());
		}
		set_pause(isUpdate);
	});
	MenuItemFont::setFontSize(font_size);
	menu_pause->setVisible(true);
	menu_again->setVisible(false);
	menu_back->setVisible(false);
	menu_pause->setAnchorPoint(Vec2(0, 0));
	menu_pause->setPosition(x, y);
	auto label_go_on = Label::createWithTTF(get_UTF8_string("go on text"), "font.ttf", SMALL_LABEL_FONT_SIZE);
	label_go_on->setAnchorPoint(menu_pause->getAnchorPoint());
	label_go_on->setPosition(menu_pause->getPosition());
	menu_pause->setUserObject(label_go_on);
	label_go_on->setVisible(false);
	this->addChild(label_go_on);
	auto label_pause = Label::createWithTTF(" x" + Value(pause_n).asString(), "font.ttf", SMALL_LABEL_FONT_SIZE);
	label_pause->setName("label_pause");
	label_pause->setAnchorPoint(Vec2(0, 0));
	label_pause->setPosition(Vec2(menu_pause->getContentSize().width, 0));
	menu_pause->addChild(label_pause);
	turn_1 = Sprite::create("arrow.png");
	turn_2 = Sprite::create("arrow.png");
	turn_1->setPosition(menu_pause->getPosition() + Vec2(0, menu_pause->getContentSize().height + 10) + turn_1->getContentSize() / 2);
	turn_2->setPosition(turn_1->getPosition() + Vec2(turn_1->getContentSize().width + 10, 0));
	this->addChild(turn_1);
	this->addChild(turn_2);
	menu_clear_dir = MenuItemFont::create(" X ", [this](Ref *sender) {
		if (user_info["soundEffects"].asInt() == 0) {
			CocosDenshion::SimpleAudioEngine::getInstance()->playEffect("button.wav");
		}
		snakes[0]->turn_1 = -1;
		snakes[0]->turn_2 = -1;
		update_dir();
	});
	//menu_clear_dir->setAnchorPoint(Vec2(0, 0));
	menu_clear_dir->setPosition(turn_2->getPosition() + Vec2(turn_2->getContentSize().width + 10, 0));
	update_dir();
	menu = Menu::create(menu_back, menu_pause, menu_again, menu_clear_dir, NULL);
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
					if (has_player) {
						int steps = Snake::step_length - snakes[0]->get_step();
						int time_s = (steps + snakes[0]->get_speed() - 1) / snakes[0]->get_speed();
						if (!isUpdate && game_map->is_empty(game_map->get_next_position(snakes[0]->get_position(), dir), time_s)) {
							snakes[0]->turn(dir);
							set_pause(false);
						}
						if (isUpdate) {
							snakes[0]->turn(dir);
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
				if (has_player) {
					int steps = Snake::step_length - snakes[0]->get_step();
					int time_s = (steps + snakes[0]->get_speed() - 1) / snakes[0]->get_speed();
					if (!isUpdate && game_map->is_empty(game_map->get_next_position(snakes[0]->get_position(), dir), time_s)) {
						snakes[0]->turn(dir);
						set_pause(false);
					}
					if (isUpdate) {
						snakes[0]->turn(dir);
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
		//this->print_log("onKeyPressed" + Value((int)key).asString());
		if (has_player) {
			int steps = Snake::step_length - snakes[0]->get_step();
			int time_s = (steps + snakes[0]->get_speed() - 1) / snakes[0]->get_speed();
			if (!isUpdate && game_map->is_empty(game_map->get_next_position(snakes[0]->get_position(), dir), time_s)) {
				snakes[0]->turn(dir);
				set_pause(false);
			}
			if (isUpdate) {
				snakes[0]->turn(dir);
			}
		}
		update_dir();
	};
	listener_key->onKeyReleased = [this](EventKeyboard::KeyCode key, Event *e) {
		//this->print_log("onKeyReleased" + Value((int)key).asString());
		switch (key) {
		case EventKeyboard::KeyCode::KEY_ESCAPE:
		case EventKeyboard::KeyCode::KEY_MENU:
			if (user_info["soundEffects"].asInt() == 0) {
				CocosDenshion::SimpleAudioEngine::getInstance()->playEffect("button.wav");
			}
			if (!isUpdate) {
				return;
			}
			if (pause_n <= 0) {
				game_map->setVisible(false);
			}
			else {
				add_pause_n(-1);
				auto label = (Label*)menu_pause->getChildByName("label_pause");
				label->setString(" x" + Value(this->get_pause_n()).asString());
			}
			this->set_pause(true);
			break;
		default:
			return;
		}
	};
	Director::getInstance()->getEventDispatcher()->addEventListenerWithSceneGraphPriority(listener_key, this);
	Director::getInstance()->getEventDispatcher()->addEventListenerWithSceneGraphPriority(listener_touch, this);
}
