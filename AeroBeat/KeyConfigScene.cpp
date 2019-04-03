#include "stdafx.h"
#include "Window.h"
#include "MusicSelectScene.h"
#include "KeyConfigScene.h"

#define AB_TIMER_SCENE L"scene"
#define AB_TIMER_START_INPUT L"start_input"
#define AB_TIMER_FADE_OUT L"fade_out"

KeyConfigScene::KeyConfigScene(Window* window, Skin& skin) : SceneBase(window, skin.resolutionWidth(), skin.resolutionHeight()), _skin(skin)
{
	_engine.interpret(skin);

	// set timers
	_engine.timers()[AB_TIMER_SCENE].start();

	// clear mouse input
	util::clear_mouse_input();

	// refresh state
	refresh_state();
}

bool KeyConfigScene::update()
{
	int x, y;
	_window->cursorLocalPosition(_skin.resolutionWidth(), _skin.resolutionHeight(), x, y);
	_engine.set_cursor_x(x);
	_engine.set_cursor_y(y);

	auto& timers = _engine.timers();

	if (timers[AB_TIMER_SCENE]() < _skin.startInput())
	{
		return true;
	}
	else if (!timers[AB_TIMER_START_INPUT].isEnabled())
	{
		timers[AB_TIMER_START_INPUT].start();
		return true;
	}

	if (timers[AB_TIMER_FADE_OUT].isEnabled())
	{
		if (timers[AB_TIMER_FADE_OUT]() > _skin.fadeOut())
		{
			auto& skin = _window->skinManager().selectedSkin(Skin::MusicSelect);
			_window->switchScene(std::shared_ptr<SceneBase>(new MusicSelectScene(_window, skin)));
			return true;
		}
		return true;
	}

	auto mouse_input = util::get_mouse_input();
	if (mouse_input & MOUSE_INPUT_RIGHT)
	{
		_exit_scene();
		return true;
	}
	else if (mouse_input & MOUSE_INPUT_LEFT)
	{
		auto keys_prev = _keys;
		auto key_prev = _key;
		auto slot_prev = _slot;
		for (auto& a : _engine.hit())
		{
			if (a == L"keys_prev")
			{
				_keys = (3 + _keys - 1) % 3;
				_key = 0;
			}
			else if (a == L"keys_next")
			{
				_keys = (3 + _keys + 1) % 3;
				_key = 0;
			}
			else if (a == L"key_prev")
			{
				int i = AB_KEY_7KEYS_COUNT;
				if (_keys == 1) i = AB_KEY_9BUTTONS_COUNT;
				else if (_keys == 2) i = AB_KEY_5KEYS_COUNT;
				_key = (i + _key - 1) % i;
			}
			else if (a == L"key_next")
			{
				int i = AB_KEY_7KEYS_COUNT;
				if (_keys == 1) i = AB_KEY_9BUTTONS_COUNT;
				else if (_keys == 2) i = AB_KEY_5KEYS_COUNT;
				_key = (i + _key + 1) % i;
			}
			else if (util::starts_with(a, L"slot") && a.size() == 5)
			{
				int i = a[4] - L'0';
				if (i >= 0 && i <= 9)
					_slot = i;
			}
			else if (util::starts_with(a, L"key") && a.size() == 5)
			{
				int i = a[3] - L'0';
				int j = a[4] - L'0';
				if (i >= 0 && i <= 9 && j >= 0 && j <= 9)
				{
					int n = 10 * i + j;
					if (_keys == 0 && n < AB_KEY_7KEYS_COUNT || _keys == 1 && n < AB_KEY_9BUTTONS_COUNT || _keys == 2 && n < AB_KEY_5KEYS_COUNT)
						_key = n;
				}
			}
		}
		if (keys_prev != _keys || key_prev != _key || slot_prev != _slot)
		{
			refresh_state();
			_window->systemSoundManager().sound(SystemSoundManager::SoundType::OptionChange)->play();
		}
	}
	else
	{
		auto& keyconfig = _window->keyConfig();
		if (_window->keyStates()[KEY_INPUT_F1] == 1)
		{
			keyconfig.clear();
			refresh_state();
			_window->systemSoundManager().sound(SystemSoundManager::SoundType::OptionChange)->play();
		}
		else if (_window->keyStates()[KEY_INPUT_F2] == 1)
		{
			keyconfig.reset();
			refresh_state();
			_window->systemSoundManager().sound(SystemSoundManager::SoundType::OptionChange)->play();
		}
		else if (_window->keyStates()[KEY_INPUT_ESCAPE] == 1)
			_exit_scene();
		else if (_slot >= 0 && _slot <= 9)
		{
			int base = 1;
			if (_keys == 1) base = AB_KEY_9BUTTONS_1;
			else if (_keys == 2) base = AB_KEY_5KEYS_1P_1;
			auto& keys = keyconfig.keys()[base + _key];

			for (unsigned i = 0; i < _window->keyStates().size(); i++)
			{
				if (_window->keyStates()[i] == 1)
				{
					if (i == KEY_INPUT_DELETE)
						keys[_slot] = KeyConfig::Key::make(KeyConfig::KeyType::None, 0);
					else
						keys[_slot] = KeyConfig::Key::make(KeyConfig::KeyType::Keyboard, i);
					refresh_state();
					_window->systemSoundManager().sound(SystemSoundManager::SoundType::OptionChange)->play();
					return true;
				}
			}
			for (unsigned j = 0; j < _window->joypadStates().size(); j++)
			{
				for (unsigned i = 0; i < _window->joypadStates()[j].size(); i++)
				{
					if (_window->joypadStates()[j][i] == 1)
					{
						keys[_slot] = KeyConfig::Key::make((KeyConfig::KeyType)(KeyConfig::KeyType::JoyPad1 + j), i);
						refresh_state();
						_window->systemSoundManager().sound(SystemSoundManager::SoundType::OptionChange)->play();
						return true;
					}
				}
			}
		}
	}
	return true;
}

void KeyConfigScene::draw()
{
	_engine.draw();
}

void KeyConfigScene::refresh_state()
{
	auto& flags = _engine.flags();
	flags[L"keys_7_14"] = _keys == 0;
	flags[L"keys_9"] = _keys == 1;
	flags[L"keys_5_10"] = _keys == 2;
	flags[L"slot0"] = _slot == 0;
	flags[L"slot1"] = _slot == 1;
	flags[L"slot2"] = _slot == 2;
	flags[L"slot3"] = _slot == 3;
	flags[L"slot4"] = _slot == 4;
	flags[L"slot5"] = _slot == 5;
	flags[L"slot6"] = _slot == 6;
	flags[L"slot7"] = _slot == 7;
	flags[L"slot8"] = _slot == 8;
	flags[L"slot9"] = _slot == 9;

	flags[L"key00"] = _key == 0;
	flags[L"key01"] = _key == 1;
	flags[L"key02"] = _key == 2;
	flags[L"key03"] = _key == 3;
	flags[L"key04"] = _key == 4;
	flags[L"key05"] = _key == 5;
	flags[L"key06"] = _key == 6;
	flags[L"key07"] = _key == 7;
	flags[L"key08"] = _key == 8;
	flags[L"key09"] = _key == 9;
	flags[L"key10"] = _key == 10;
	flags[L"key11"] = _key == 11;
	flags[L"key12"] = _key == 12;
	flags[L"key13"] = _key == 13;
	flags[L"key14"] = _key == 14;
	flags[L"key15"] = _key == 15;
	flags[L"key16"] = _key == 16;
	flags[L"key17"] = _key == 17;
	flags[L"key18"] = _key == 18;
	flags[L"key19"] = _key == 19;
	flags[L"key20"] = _key == 20;
	flags[L"key21"] = _key == 21;

	int base = 1;
	if (_keys == 1) base = AB_KEY_9BUTTONS_1;
	else if (_keys == 2) base = AB_KEY_5KEYS_1P_1;
	auto& keys = _window->keyConfig().keys()[base + _key];

	auto& texts = _engine.texts();
	texts[L"slot0"] = KeyConfig::name(keys[0]);
	texts[L"slot1"] = KeyConfig::name(keys[1]);
	texts[L"slot2"] = KeyConfig::name(keys[2]);
	texts[L"slot3"] = KeyConfig::name(keys[3]);
	texts[L"slot4"] = KeyConfig::name(keys[4]);
	texts[L"slot5"] = KeyConfig::name(keys[5]);
	texts[L"slot6"] = KeyConfig::name(keys[6]);
	texts[L"slot7"] = KeyConfig::name(keys[7]);
	texts[L"slot8"] = KeyConfig::name(keys[8]);
	texts[L"slot9"] = KeyConfig::name(keys[9]);
}

void KeyConfigScene::_exit_scene()
{
	_window->keyConfig().save();
	_engine.timers()[AB_TIMER_FADE_OUT].start();
}
