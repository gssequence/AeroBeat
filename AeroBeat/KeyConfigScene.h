#pragma once

#include "SceneBase.h"
#include "SkinEngine.h"

class KeyConfigScene : public SceneBase
{
private:
	Skin& _skin;
	SkinEngine _engine;

	int _keys = 0, _slot = -1, _key = 0;

	void refresh_state();
	void _exit_scene();

public:
	KeyConfigScene(Window* window, Skin& skin);
	virtual ~KeyConfigScene() { }

	virtual bool update() override;
	virtual void draw() override;
};
