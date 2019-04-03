#include "stdafx.h"
#include "Window.h"
#include "PlayScene.h"
#include "DecideScene.h"

#define AB_TIMER_SCENE L"scene"
#define AB_TIMER_FADE_OUT L"fade_out"

DecideScene::DecideScene(Window* window, Skin& skin, std::shared_ptr<SongManager::Node> node) : SceneBase(window, skin.resolutionWidth(), skin.resolutionHeight()), _skin(skin)
{
	_node = node;

	_engine.interpret(skin);

	// set parameters
	auto& song = node->song();
	auto& flags = _engine.flags();
	auto& texts = _engine.texts();
	auto& numbers = _engine.numbers();
	flags[L"undefined"] = song.difficulty == 0;
	flags[L"beginner"] = song.difficulty == 1;
	flags[L"normal"] = song.difficulty == 2;
	flags[L"hyper"] = song.difficulty == 3;
	flags[L"another"] = song.difficulty == 4;
	flags[L"insane"] = song.difficulty == 5;
	texts[L"title"] = song.title;
	texts[L"subtitle"] = song.subtitle;
	texts[L"artist"] = song.artist;
	texts[L"subartist"] = song.subartist;
	texts[L"genre"] = song.genre;
	numbers[L"level"] = song.playLevel;

	// set timers
	_engine.timers()[AB_TIMER_SCENE].start();

	// play bgm
	_window->systemSoundManager().sound(SystemSoundManager::SoundType::DecideBgm)->play();
}

void DecideScene::_exit_scene()
{
	// TODO: プレイスキンへ変える
	auto& skin = _window->skinManager().selectedSkin(Skin::Play7);
	_window->switchScene(std::shared_ptr<SceneBase>(new PlayScene(_window, skin, _node)));
}

bool DecideScene::update()
{
	auto& timers = _engine.timers();

	if (timers[AB_TIMER_SCENE]() < _skin.startInput())
		return true;
	if (timers[AB_TIMER_FADE_OUT].isEnabled())
	{
		if (timers[AB_TIMER_FADE_OUT]() > _skin.fadeOut())
		{
			_exit_scene();
			return true;
		}
		return true;
	}
	if (timers[AB_TIMER_SCENE]() >= _skin.sceneTime() - _skin.fadeOut())
	{
		timers[AB_TIMER_FADE_OUT].start();
		return true;
	}

	// キー操作
	auto& ks = _window->keyStates();
	auto& cs = _window->controllerStates();
	if (cs[AB_KEY_7KEYS_1P_1] == 1 ||
		cs[AB_KEY_7KEYS_1P_2] == 1 ||
		cs[AB_KEY_7KEYS_1P_3] == 1 ||
		cs[AB_KEY_7KEYS_1P_4] == 1 ||
		cs[AB_KEY_7KEYS_1P_5] == 1 ||
		cs[AB_KEY_7KEYS_1P_6] == 1 ||
		cs[AB_KEY_7KEYS_1P_7] == 1 ||
		cs[AB_KEY_7KEYS_1P_START] == 1 ||
		cs[AB_KEY_7KEYS_1P_SELECT] == 1 ||
		cs[AB_KEY_7KEYS_2P_1] == 1 ||
		cs[AB_KEY_7KEYS_2P_2] == 1 ||
		cs[AB_KEY_7KEYS_2P_3] == 1 ||
		cs[AB_KEY_7KEYS_2P_4] == 1 ||
		cs[AB_KEY_7KEYS_2P_5] == 1 ||
		cs[AB_KEY_7KEYS_2P_6] == 1 ||
		cs[AB_KEY_7KEYS_2P_7] == 1 ||
		cs[AB_KEY_7KEYS_2P_START] == 1 ||
		cs[AB_KEY_7KEYS_2P_SELECT] == 1)
	{
		timers[AB_TIMER_FADE_OUT].start();
		return true;
	}

	return true;
}

void DecideScene::draw()
{
	_engine.draw();
}
