#include "GameMenuScene.h"
#include "MainMenuScene.h"
#include "MyGameScene.h"
#include "SimpleAudioEngine.h"

USING_NS_CC;
using namespace std;

Scene* GameMenu::createScene() {
	auto scene = Scene::create();
	auto layer = GameMenu::create();
	scene->addChild(layer);
	return scene;
}

bool GameMenu::init() {
	if (!Layer::init()) {
		return false;
	}
	//add menu back_to_main_menu
	auto menu_back = MenuItemFont::create(get_UTF8_string("back"), [](Ref *sender) {
		if (user_info["soundEffects"].asInt() == 0) {
			CocosDenshion::SimpleAudioEngine::getInstance()->playEffect("button.wav");
		}
		auto next_scene = MainMenu::createScene();
		auto Transition_scene = TransitionCrossFade::create(SCENE_TURN_TRANSITION_TIME, next_scene);
		Director::getInstance()->replaceScene(Transition_scene);
	});
	menu_back->setAnchorPoint(Vec2(1, 0));
	menu_back->setPosition(origin.x + visible_size.width, origin.y);
	//create menu item
	auto menu = Menu::create(menu_back, NULL);
	menu->setAnchorPoint(Vec2::ZERO);
	menu->setPosition(Vec2::ZERO);
	this->addChild(menu);


	auto listener = EventListenerTouchOneByOne::create();
	select_mission = nullptr;
	listener->onTouchBegan = [this](Touch *t, Event *e) {
		auto pos = t->getLocation();
		pos.y = visible_size.height - pos.y;
		pos.x /= 256;
		pos.y /= 160;
		int tag = floor(pos.y) * 5 + floor(pos.x);
		auto current_mission = user_info["current_mission"].asInt();
		if (tag < current_mission) {
			select_mission = (Mission*)this->getChildByTag(tag + 1);
		}
		else {
			select_mission = NULL;
		}
		if (select_mission) {
			select_mission->setScale(1.2f);
		}
		return true;
	};
	listener->onTouchMoved = [this](Touch *t, Event *e) {
		if (t->getLocation().distance(t->getStartLocation()) < 50) {
			return;
		}
		if (select_mission) {
			select_mission->setScale(1);
			select_mission = nullptr;
		}
	};
	listener->onTouchEnded = [this](Touch *t, Event *e) {
		if(select_mission) {
			if (user_info["soundEffects"].asInt() == 0) {
				CocosDenshion::SimpleAudioEngine::getInstance()->playEffect("button.wav");
			}
			CocosDenshion::SimpleAudioEngine::getInstance()->stopBackgroundMusic();
			auto next_scene = MyGame::createScene(select_mission->getTag());
			auto Transition_scene = TransitionCrossFade::create(SCENE_TURN_TRANSITION_TIME, next_scene);
			Director::getInstance()->replaceScene(Transition_scene);
			select_mission = nullptr;
		}
	};
	Director::getInstance()->getEventDispatcher()->addEventListenerWithSceneGraphPriority(listener, this);

	max_mission = 0;
	for (int i = 1; i <= 20; i++) {
		auto mission = Mission::create(i);
		if (!mission) {
			break;
		}
		log("create mission %d", i);
		if (mission->get_score() > 0) {
			max_mission = i;
		}
		mission->setTag(i);
		float x = ((i - 1) % 5 + 0.5) * mission->getContentSize().width;
		float y = visible_size.height - ((i - 1) / 5 + 0.5) * (mission->getContentSize().height);
		//mission->setAnchorPoint(Vec2(0, 1));
		mission->setPosition(x, y);
		this->addChild(mission);
	}

	if (!CocosDenshion::SimpleAudioEngine::getInstance()->isBackgroundMusicPlaying() && user_info["music"].asInt() == 0) {
		CocosDenshion::SimpleAudioEngine::getInstance()->playBackgroundMusic("start.wav", true);
	}
	return true;
}
