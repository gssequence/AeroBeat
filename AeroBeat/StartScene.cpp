#include "stdafx.h"
#include "Window.h"
#include "MusicSelectScene.h"
#include "StartScene.h"

void StartScene::_write(const std::wstring& str)
{
	_mutex.lock();

	unsigned limit = _windowHeight / _fontSize;
	if (_lines.size() >= limit)
		_lines.erase(_lines.begin());
	_lines.push_back(str);

	_mutex.unlock();
}

bool StartScene::update()
{
	if (!_started)
	{
		_started = true;
		_logger = [this](auto& a) { _write(a); };
		_completion = [this]
		{
			_completed = true;
			_logger(L"Ready to start AeroBeat");
			_logger(L"Press any key");
		};
		_window->startInitialize(_logger, _completion);
	}

	auto& ks = _window->keyStates();
	auto& cs = _window->controllerStates();
	bool flag = false;
	if (ks[KEY_INPUT_ESCAPE] == 1) return false;
	if (!_completed) return true;
	for (auto& a : ks)
	{
		if (a == 1)
		{
			flag = true;
			break;
		}
	}
	for (auto& a : cs)
	{
		if (a == 1 || flag)
		{
			flag = true;
			break;
		}
	}
	if (flag)
	{
		auto& skin = _window->skinManager().selectedSkin(Skin::MusicSelect);
		_window->switchScene(std::shared_ptr<SceneBase>(new MusicSelectScene(_window, skin)));
		dx::SetMouseDispFlag(false);
		return true;
	}

	return true;
}

void StartScene::draw()
{
	_mutex.lock();

	int i = 0;
	for (auto& a : _lines)
		dx::DrawString(0, _fontSize * i++, a.c_str(), _fontColor);

	_mutex.unlock();
}
