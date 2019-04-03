#pragma once

#include "SceneBase.h"
#include "SkinEngine.h"

class DecideScene : public SceneBase
{
private:
	std::shared_ptr<SongManager::Node> _node;
	Skin& _skin;
	SkinEngine _engine;

	void _exit_scene();

public:
	DecideScene(Window* window, Skin& skin, std::shared_ptr<SongManager::Node> node);
	virtual ~DecideScene() { }

	virtual bool update() override;
	virtual void draw() override;

};
