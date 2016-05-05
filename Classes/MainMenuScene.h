#pragma once
#include "cocos2d.h"
#include "MyGameScene.h"

USING_NS_CC;
using namespace std;
class MainMenu : public Layer {
public:
	static Scene* createScene();
	virtual bool init();
	CREATE_FUNC(MainMenu);
	MyGame *game;
	virtual void update(float dt);

	int menu_id;
};
