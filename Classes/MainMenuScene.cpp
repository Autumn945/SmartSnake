#include "MainMenuScene.h"
#include "SystemHeader.h"
#include "GameMenuScene.h"
#include "OptionScene.h"
#include "SimpleAudioEngine.h"

USING_NS_CC;
//using namespace std;

Scene* MainMenu::createScene() {
	auto scene = Scene::create();
	auto layer = MainMenu::create();
	scene->addChild(layer);
	return scene;
}

bool MainMenu::init() {
	if (!Layer::init()) {
		return false;
	}
	log("MainMenu init");

	if (user_info.count("control_mode") == 0) {
		user_info["control_mode"] = 0;
	}
	if (user_info.count("soundEffects") == 0) {
		user_info["soundEffects"] = 0;
	}
	if (user_info.count("music") == 0) {
		user_info["music"] = 0;
	}
	if (user_info.count("current_mission") == 0) {
		user_info["current_mission"] = 1;
	}
	FileUtils::getInstance()->writeValueMapToFile(user_info, "res/user_info.xml");
	//(0.5, 1 - 0.618)
	auto position = Vec2(origin.x + visible_size.width / 2
		, origin.y + visible_size.height * (1 - 0.618));

	// scene turn to option function
	auto create_menu_turn_to_option = [](string name) {
		return MenuItemFont::create(get_UTF8_string(name), [name](Ref *sender) {
			if (user_info["soundEffects"].asInt() == 0) {
				CocosDenshion::SimpleAudioEngine::getInstance()->playEffect("button.wav");
			}
			auto next_scene = Option::createScene(name);
			auto Transition_scene = TransitionCrossFade::create(SCENE_TURN_TRANSITION_TIME, next_scene);
			Director::getInstance()->replaceScene(Transition_scene);
		});
	};

	//main menu
	auto menu_start = MenuItemFont::create(get_UTF8_string("start"), [=](Ref *sender) {
		log("hit start");
		if (user_info["soundEffects"].asInt() == 0) {
			CocosDenshion::SimpleAudioEngine::getInstance()->playEffect("button.wav");
		}
		//CocosDenshion::SimpleAudioEngine::getInstance()->stopBackgroundMusic();
		auto next_scene = GameMenu::createScene();
		auto Transition_scene = TransitionCrossFade::create(SCENE_TURN_TRANSITION_TIME, next_scene); 
		Director::getInstance()->replaceScene(Transition_scene);
	});
	auto menu_option_setting = create_menu_turn_to_option("setting");
	auto menu_option_help = create_menu_turn_to_option("help");
	auto menu_option_about = create_menu_turn_to_option("about");
	auto menu_option_feedback = create_menu_turn_to_option("feedback");
	auto menu_exit = MenuItemFont::create(get_UTF8_string("exit"), [](Ref *sender) {
		if (user_info["soundEffects"].asInt() == 0) {
			CocosDenshion::SimpleAudioEngine::getInstance()->playEffect("button.wav");
		}
		Director::getInstance()->end();
	});

	//add into a vector
	vector<MenuItem *> vector_menu;
	vector_menu.push_back(menu_start);
	vector_menu.push_back(menu_option_setting);
	vector_menu.push_back(menu_option_help);
	vector_menu.push_back(menu_option_about);
	vector_menu.push_back(menu_option_feedback);
	vector_menu.push_back(menu_exit);

	//max content width
	auto menu_item_width = 0.0f;
	for (auto m : vector_menu) {
		menu_item_width = max(menu_item_width, m->getContentSize().width);
	}

	//get action of turn menu
	auto ac_rotate_to_n90 = RotateTo::create(MAIN_MENU_TURN_TIME, Vec3(0, -90, 0));
	auto ac_rotate_to_90 = RotateTo::create(MAIN_MENU_TURN_TIME, Vec3(0, 90, 0));
	auto ac_rotate_to_0 = RotateTo::create(MAIN_MENU_TURN_TIME, Vec3(0, 0, 0));
	auto ac_fade_out = FadeOut::create(MAIN_MENU_TURN_TIME);
	auto ac_fade_in = FadeIn::create(MAIN_MENU_TURN_TIME);
	auto ac_move_to_mid = MoveTo::create(MAIN_MENU_TURN_TIME, position);
	auto ac_move_to_left = MoveTo::create(MAIN_MENU_TURN_TIME, position - Vec2(menu_item_width / 2, 0));
	auto ac_move_to_right = MoveTo::create(MAIN_MENU_TURN_TIME, position + Vec2(menu_item_width / 2, 0));
	auto ac_turn_right = Sequence::create(Spawn::create(ac_rotate_to_90, ac_move_to_right, ac_fade_out, NULL), CallFuncN::create([](Ref *sender) {
		((Node *)sender)->setVisible(false);
		log("right");
	}), NULL);
	auto ac_turn_mid = Sequence::create(CallFuncN::create([](Ref *sender) {
		((Node *)sender)->setVisible(true);
		log("mid");
	}), Spawn::create(ac_rotate_to_0, ac_move_to_mid, ac_fade_in, NULL), NULL);
	auto ac_turn_left = Sequence::create(Spawn::create(ac_rotate_to_n90, ac_move_to_left, ac_fade_out, NULL), CallFuncN::create([](Ref *sender) {
		((Node *)sender)->setVisible(false);
		log("left");
	}), NULL);
	ac_turn_right->retain();
	ac_turn_mid->retain();
	ac_turn_left->retain();
	//if ac_turn_mid has not run, set tag = 1
	ac_turn_mid->setTag(1);

	//add turn menu
	auto menu_turn_left = MenuItemFont::create("  <  ", [=](Ref *sender) {
		//if ac_turn_mid is runing, tag = 0; isDone = flase
		if (ac_turn_mid->isDone() + ac_turn_mid->getTag() == 0) {
			return;
		}
		if (user_info["soundEffects"].asInt() == 0) {
			CocosDenshion::SimpleAudioEngine::getInstance()->playEffect("turn.wav");
		}
		//if ac_turn_mid has been run, set tag = 0
		ac_turn_mid->setTag(0);
		//run action
		vector_menu[menu_id]->runAction(ac_turn_left);
		//next menu id
		menu_id = (menu_id + 1) % vector_menu.size();
		//init menu item
		vector_menu[menu_id]->stopAllActions();
		vector_menu[menu_id]->setPosition(position + Vec2(menu_item_width / 2, 0));
		vector_menu[menu_id]->setRotation3D(Vec3(0, 90, 0));
		vector_menu[menu_id]->setOpacity(100);
		vector_menu[menu_id]->setVisible(false);
		//run action
		vector_menu[menu_id]->runAction(ac_turn_mid);
	});
	auto menu_turn_right = MenuItemFont::create("  >  ", [=](Ref *sender) {
		if (ac_turn_mid->isDone() + ac_turn_mid->getTag() == 0) {
			return;
		}
		if (user_info["soundEffects"].asInt() == 0) {
			CocosDenshion::SimpleAudioEngine::getInstance()->playEffect("turn.wav");
		}
		//if ac_turn_mid has been run, set tag = 0
		ac_turn_mid->setTag(0);
		//run action
		vector_menu[menu_id]->runAction(ac_turn_right);
		//next menu id
		menu_id = (menu_id - 1 + vector_menu.size()) % vector_menu.size();
		//init menu item
		vector_menu[menu_id]->stopAllActions();
		vector_menu[menu_id]->setPosition(position - Vec2(menu_item_width / 2, 0));
		vector_menu[menu_id]->setRotation3D(Vec3(0, -90, 0));
		vector_menu[menu_id]->setOpacity(100);
		vector_menu[menu_id]->setVisible(false);
		//run action
		vector_menu[menu_id]->runAction(ac_turn_mid);
	});

	//create menu
	auto menu = Menu::create(menu_turn_left, menu_turn_right, NULL);
	menu->setPosition(Vec2::ZERO);
	menu->setAnchorPoint(Vec2::ZERO);
	menu_turn_left->setPosition(position - Vec2(menu_item_width / 2 + menu_turn_left->getContentSize().width / 2, 0));
	menu_turn_right->setPosition(position + Vec2(menu_item_width / 2 + menu_turn_right->getContentSize().width / 2, 0));
	this->addChild(menu);
	//init all menu itme
	for (auto m : vector_menu) {
		menu->addChild(m);
		m->setVisible(false);
	}
	menu_id = 0;
	//init menu item
	vector_menu[menu_id]->setPosition(position);
	vector_menu[menu_id]->setRotation3D(Vec3(0, 0, 0));
	vector_menu[menu_id]->setOpacity(255);
	vector_menu[menu_id]->setVisible(true);
	if (!CocosDenshion::SimpleAudioEngine::getInstance()->isBackgroundMusicPlaying() && user_info["music"].asInt() == 0) {
		CocosDenshion::SimpleAudioEngine::getInstance()->playBackgroundMusic("start.wav", true);
	}
	return true;
}