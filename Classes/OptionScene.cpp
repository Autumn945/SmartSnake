#include "OptionScene.h"
#include "SystemHeader.h"
#include "MainMenuScene.h"
#include "SimpleAudioEngine.h"

USING_NS_CC;
using namespace std; 

Option* Option::create(string name) {
	Option *pRet = new(std::nothrow) Option();
	if (pRet && (pRet->setName(name), pRet->init())) {
		pRet->autorelease();
		return pRet;
	}
	else {
		delete pRet;
		pRet = nullptr;
		return nullptr;
	}
}

Scene* Option::createScene(string name) {
	auto scene = Scene::create();
	auto layer = Option::create(name);
	scene->addChild(layer);
	return scene;
}

bool Option::init() {
	//////////////////////////////
	// 1. super init first
	if (!Layer::init()) {
		return false;
	}
	//add menu back_to_main_menu
	auto menu_back = MenuItemFont::create(get_UTF8_string("back"), [](Ref *sender) {
		if (user_info["soundEffects"].asInt() == 0) {
			CocosDenshion::SimpleAudioEngine::getInstance()->playEffect("button.wav");
		}
		FileUtils::getInstance()->writeValueMapToFile(user_info, "res/user_info.xml");
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
	//create top label
	auto top_label = Label::createWithSystemFont(get_UTF8_string(this->getName()), "abc", BIG_LABEL_FONT_SIZE);
	top_label->setAnchorPoint(Vec2(0.5, 1));
	top_label->setPosition(origin.x + visible_size.width / 2
		, origin.y + visible_size.height); 
	this->addChild(top_label);
	// set context
	if (this->getName() == "setting") {
		Vec2 pos_left = origin + Vec2(visible_size.width * 0.5, visible_size.height * 0.6);
		float right = origin.x + visible_size.width * 0.6;
		//music
		auto music = Label::createWithSystemFont(get_UTF8_string("music"), "Arial", MID_LABEL_FONT_SIZE);
		music->setAnchorPoint(Vec2(1, 0));
		music->setPosition(pos_left);
		this->addChild(music);
		auto menu_music = MenuItemToggle::createWithCallback([](Ref *sender) {
			if (user_info["soundEffects"].asInt() == 0) {
				CocosDenshion::SimpleAudioEngine::getInstance()->playEffect("button.wav");
			}
			auto item = (MenuItemToggle*)sender;
			int index = item->getSelectedIndex();
			user_info["music"] = index;
			if (index == 1) {
				CocosDenshion::SimpleAudioEngine::getInstance()->stopBackgroundMusic();
			}
			else {
				CocosDenshion::SimpleAudioEngine::getInstance()->playBackgroundMusic("start.wav");
			}
			log("%d", index);
		}
			, MenuItemFont::create("On")
			, MenuItemFont::create("Off")
			, NULL);
		menu_music->setSelectedIndex(user_info["music"].asInt());
		menu_music->setAnchorPoint(Vec2(0, 0));
		menu_music->setPosition(right, pos_left.y);
		menu->addChild(menu_music);
		pos_left.y -= music->getContentSize().height;
		//soundEffects
		auto soundEffects = Label::createWithSystemFont(get_UTF8_string("soundEffects"), "Arial", MID_LABEL_FONT_SIZE);
		soundEffects->setAnchorPoint(Vec2(1, 0));
		soundEffects->setPosition(pos_left);
		this->addChild(soundEffects);
		auto menu_soundEffects = MenuItemToggle::createWithCallback([](Ref *sender) {
			if (user_info["soundEffects"].asInt() == 0) {
				CocosDenshion::SimpleAudioEngine::getInstance()->playEffect("button.wav");
			}
			auto item = (MenuItemToggle*)sender;
			int index = item->getSelectedIndex();
			user_info["soundEffects"] = index;
			log("%d", index);
		}
			, MenuItemFont::create("On")
			, MenuItemFont::create("Off")
			, NULL);
		menu_soundEffects->setSelectedIndex(user_info["soundEffects"].asInt());
		menu_soundEffects->setAnchorPoint(Vec2(0, 0));
		menu_soundEffects->setPosition(right, pos_left.y);
		menu->addChild(menu_soundEffects);
		pos_left.y -= soundEffects->getContentSize().height;
		//control_mode
		auto control_mode = Label::createWithSystemFont(get_UTF8_string("controlModes"), "Arial", MID_LABEL_FONT_SIZE);
		control_mode->setAnchorPoint(Vec2(1, 0));
		control_mode->setPosition(pos_left);
		this->addChild(control_mode);
		auto menu_control_mode = MenuItemToggle::createWithCallback([](Ref *sender) {
			if (user_info["soundEffects"].asInt() == 0) {
				CocosDenshion::SimpleAudioEngine::getInstance()->playEffect("button.wav");
			}
			auto item = (MenuItemToggle*)sender;
			int index = item->getSelectedIndex();
			user_info["control_mode"] = index;
			log("%d", index);
		}
			, MenuItemFont::create(get_UTF8_string("touch") + "+" + get_UTF8_string("slide"))
			, MenuItemFont::create(get_UTF8_string("slide"))
			, MenuItemFont::create(get_UTF8_string("touch"))
			, NULL);
		menu_control_mode->setSelectedIndex(user_info["control_mode"].asInt());
		menu_control_mode->setAnchorPoint(Vec2(0, 0));
		menu_control_mode->setPosition(right, pos_left.y);
		menu->addChild(menu_control_mode);
		pos_left.y -= control_mode->getContentSize().height;
		auto menu_clear = MenuItemFont::create(get_UTF8_string("clear_user_info"), [](Ref *sender) {
			if (user_info["soundEffects"].asInt() == 0) {
				CocosDenshion::SimpleAudioEngine::getInstance()->playEffect("button.wav");
			}
			user_info["clear"] = user_info["clear"].asInt() + 1;
			if (user_info["clear"].asInt() >= 5) {
				user_info.clear();
				FileUtils::getInstance()->writeValueMapToFile(user_info, "res/user_info.xml");
				auto next_scene = MainMenu::createScene();
				auto Transition_scene = TransitionCrossFade::create(SCENE_TURN_TRANSITION_TIME, next_scene);
				Director::getInstance()->replaceScene(Transition_scene);
			}
		});
		user_info["clear"] = 0;
		menu_clear->setPosition(origin.x + visible_size.width * 0.5, pos_left.y);
		menu->addChild(menu_clear);
		return true;
	}
	if (this->getName() == "help") {
		float x = 10, y = origin.x + visible_size.height - top_label->getContentSize().height * 1.5f;
		auto label_help = Label::createWithSystemFont(get_UTF8_string("text_help_0"), "abc", SMALL_LABEL_FONT_SIZE);
		label_help->setAnchorPoint(Vec2(0, 1));
		label_help->setPosition(x, y);
		this->addChild(label_help);
		for (int i = 0; i < 100; i++) {
			string str = "menu_help_" + Value(i).asString();
			auto UTF8str = get_UTF8_string(str);
			if (str == UTF8str) {
				break;
			}
			auto menu_help = MenuItemFont::create(UTF8str, [label_help](Ref *ref) {
				if (user_info["soundEffects"].asInt() == 0) {
					CocosDenshion::SimpleAudioEngine::getInstance()->playEffect("button.wav");
				}
				auto sender = (Node*)ref;
				label_help->setString(get_UTF8_string("text_help_" + Value(sender->getTag()).asString()));
			});
			menu_help->setAnchorPoint(Vec2(0, 1));
			menu_help->setPosition(x, y);
			menu_help->setTag(i);
			menu->addChild(menu_help);
			y -= menu_help->getContentSize().height + 10;
			label_help->setPositionX(max(label_help->getPositionX(), x + menu_help->getContentSize().width * 1.0f));
		}
		label_help->setDimensions(visible_size.width - 1.5f * label_help->getPositionX(), visible_size.height);
		//label_help->setMaxLineWidth(visible_size.width - 2 * label_help->getPositionX());
		//label_help->setLineBreakWithoutSpace(false);
		return true;
	}
	auto label_option = Label::createWithSystemFont(get_UTF8_string("text_" + this->getName()), "abc", SMALL_LABEL_FONT_SIZE);
	label_option->setPosition(origin + visible_size / 2);
	label_option->setDimensions(visible_size.width / 2, visible_size.height / 2);
	this->addChild(label_option);
	if (this->getName() == "about") {
		return true;
	}
	if (this->getName() == "feedback") {
		return true;
	}
	//debug
	return true;
	return false;
}