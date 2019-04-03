#pragma once
#include "Config.h"
#include "KeyConfig.h"
#include "SkinManager.h"
#include "SongManager.h"
#include "SoundManager.h"
#include "SystemSoundManager.h"
#include "PlayOption.h"

class SceneBase;

class Window
{
private:
	bool _fail = false;
	bool _initialized = false;
	bool _minimized = false;

	Config _config;
	KeyConfig _keyConfig;
	std::vector<std::wstring> _systemFiles;
	SkinManager _skinManager;
	SongManager _songManager;
	SystemSoundManager _systemSoundManager;
	std::shared_ptr<SoundManager> _soundManager;

	std::shared_ptr<SceneBase> _currentScene;
	std::array<long long, 256> _keyStates;
	std::array<std::array<long long, 32>, 4> _joypadStates;
	std::array<long long, AB_KEY_COUNT> _controllerStates;
	int _cursor_x, _cursor_y;

	PlayOption _playOption;

	bool update();
	void draw();

public:
	Window(std::wstring commandLine = L"");
	virtual ~Window();

	void startInitialize(std::function<void(std::wstring)>& logger, std::function<void()>& completion);
	void run();
	void switchScene(std::shared_ptr<SceneBase> scene);
	bool isMinimized();

	bool fail() { return _fail; }
	Config& config() { return _config; }
	auto& keyConfig() { return _keyConfig; }
	const std::vector<std::wstring>& systemFiles() { return _systemFiles; };
	SkinManager& skinManager() { return _skinManager; }
	auto& songManager() { return _songManager; }
	auto& soundManager() { return *_soundManager; }
	auto& systemSoundManager() { return _systemSoundManager; }
	const std::array<long long, 256>& keyStates() { return _keyStates; }
	const auto& joypadStates() { return _joypadStates; }
	const auto& controllerStates() { return _controllerStates; }
	auto& playOption() { return _playOption; }
	void cursorLocalPosition(int w, int h, int& x, int& y)
	{
		x = _cursor_x * w / _config.resolutionWidth();
		y = _cursor_y * h / _config.resolutionHeight();
	}
};
