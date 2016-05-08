#include "MissionSprite.h"

int Mission::get_flower() {
	for (int i = 3; i > 0; i--) {
		if (get_score() >= flower[i - 1]) {
			return i;
		}
	}
	return 0;
}

Mission * Mission::create(int id) {
	Mission *pRet = new(std::nothrow) Mission();
	if (pRet && pRet->init(id)) {
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

bool Mission::init(int id) {
	if (!Sprite::initWithFile("mission.png")) {
		return false;
	}
	game_map = GameMap::createWithTMXFile(Value(id).asString() + ".tmx");
	if (!game_map) {
		log("not found %d.tmx", id);
		return false;
	}
	//shuxing....
	auto scores_v = game_map->getProperty("score");
	if (scores_v.isNull()) {
		scores_v = "1000,2000,3000";
	}
	sscanf(scores_v.asString().c_str(), "%d,%d,%d", &flower[0], &flower[1], &flower[2]);
	log("%d,%d,%d", flower[0], flower[1], flower[2]);
	//
	string id_string = Value(id).asString();
	if (user_info.count("mission_score" + id_string) == 0) {
		user_info["mission_score" + id_string] = 0;
		FileUtils::getInstance()->writeValueMapToFile(user_info, "res/user_info.xml");
	}
	if (user_info.count("mission_success" + id_string) == 0) {
		user_info["mission_success" + id_string] = 0;
		FileUtils::getInstance()->writeValueMapToFile(user_info, "res/user_info.xml");
	}
	if (user_info.count("mission_challenge" + id_string) == 0) {
		user_info["mission_challenge" + id_string] = 0;
		FileUtils::getInstance()->writeValueMapToFile(user_info, "res/user_info.xml");
	}
	score = user_info["mission_score" + id_string].asInt();
	success = user_info["mission_success" + id_string].asInt();
	challenge = user_info["mission_challenge" + id_string].asInt();
	auto center = (Vec2)this->getContentSize() / 2;
	float y = this->getContentSize().height - 10;
	auto name = Label::createWithTTF(String::createWithFormat(get_UTF8_string("mission_id").c_str(), id)->getCString()
		, "font.ttf", SMALL_LABEL_FONT_SIZE);
	name->setPosition(center.x, y);
	name->setAnchorPoint(Vec2(0.5, 1));
	this->addChild(name);
	y -= name->getContentSize().height + 10;
	int flo = get_flower();
	auto sp_f = Sprite::create("flower.png");
	auto current_mission = user_info["current_mission"].asInt();
	if (current_mission < id) {
		flo = 1;
		sp_f = Sprite::create("block.png");
	}
	else if (current_mission == id) {
		flo = 1;
		sp_f = Sprite::create("flag.png");
	}
	float x = center.x - sp_f->getContentSize().width * flo * 0.5;
	for (int i = 0; i < flo; i++) {
		auto sp = Sprite::create();
		sp->setSpriteFrame(sp_f->getSpriteFrame());
		sp->setAnchorPoint(Vec2(0, 1));
		sp->setPosition(x, y);
		x += sp->getContentSize().width;
		this->addChild(sp);
	}
	y -= sp_f->getContentSize().height;
	auto max_score = Label::createWithTTF(get_UTF8_string("max_score") + Value(score).asString()
		, "font.ttf", SMALL_LABEL_FONT_SIZE);
	max_score->setPosition(center.x, y);
	max_score->setAnchorPoint(Vec2(0.5, 1));
	this->addChild(max_score);
	y -= max_score->getContentSize().height;
	auto rate = Label::createWithTTF(get_UTF8_string("success/challenge") + String::createWithFormat("%d/%d", success, challenge)->getCString()
		, "font.ttf", SMALL_LABEL_FONT_SIZE - 10);
	rate->setPosition(center.x, y);
	rate->setAnchorPoint(Vec2(0.5, 1));
	this->addChild(rate);
	return true;
}
