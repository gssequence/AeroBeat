#include "stdafx.h"
#include "StartScene.h"
#include "SceneBase.h"
#include "util.h"
#include "Window.h"

Window::Window(std::wstring commandLine)
{
	// 設定の読み込み
	util::unwrap(Config::load(), _config);

	// キーコンフィグの読み込み
	KeyConfig::load(_keyConfig);

	// プレイオプションの読み込み
	_playOption.load();

	// サウンドシステム初期化
	_soundManager = std::shared_ptr<SoundManager>(new SoundManager(_config));

	// DXライブラリの設定
	dx::SetAlwaysRunFlag(true);
	dx::SetGraphMode(config().resolutionWidth(), config().resolutionHeight(), 32);
	dx::SetWaitVSyncFlag(config().vsync());
	dx::ChangeWindowMode(!config().fullscreen());

	// システムファイルの列挙
	fs::recursive_directory_iterator end;
	for (fs::recursive_directory_iterator it(L"ABFiles"); it != end; ++it)
	{
		if (!fs::is_directory(*it))
			_systemFiles.push_back(it->path().generic_wstring());
	}

	// DXライブラリ初期化
	if (dx::DxLib_Init() == -1)
	{
		_fail = true;
		return;
	}

	// 最初のシーンを設定
	_currentScene = std::shared_ptr<SceneBase>(new StartScene(this, config().resolutionWidth(), config().resolutionHeight()));
}

Window::~Window()
{
	_playOption.save();
	dx::DxLib_End();
}

void Window::startInitialize(std::function<void(std::wstring)>& logger, std::function<void()>& completion)
{
	if (_initialized) return;
	_initialized = true;

	std::thread thread([&]
	{
		_skinManager.initialize(_systemFiles, logger);
		_songManager.initialize(config(), logger);
		_systemSoundManager.initialize(_soundManager, logger);
		completion();
	});
	thread.detach();
}

void Window::run()
{
	if (fail()) return;
	while (update())
	{
		draw();
	}
}

void Window::switchScene(std::shared_ptr<SceneBase> scene)
{
	_currentScene = scene;
	scene->update();
}

bool Window::isMinimized()
{
	WINDOWINFO info;
	GetWindowInfo(dx::GetMainWindowHandle(), &info);
	return (info.dwStyle & WS_MINIMIZE) != 0;
}

bool Window::update()
{
	// Keyboard input
	char buf[256];
	dx::GetHitKeyStateAll(buf);
	for (int i = 0; i < 256; i++)
	{
		if (buf[i])
			_keyStates[i] = max(1, _keyStates[i] + 1);
		else
			_keyStates[i] = min(-1, _keyStates[i] - 1);
	}

	// Joypad input
	for (int j = 0; j < 4; j++)
	{
		int state = dx::GetJoypadInputState(DX_INPUT_PAD1 + j);
		for (int i = 0; i < 32; i++)
		{
			int b = state & (1 << i);
			if (b)
				_joypadStates[j][i] = max(1, _joypadStates[j][i] + 1);
			else
				_joypadStates[j][i] = min(-1, _joypadStates[j][i] - 1);
		}
	}

	// Keyboard & Joypad -> Controller
	auto& kc = keyConfig().keys();
	for (int i = 0; i < AB_KEY_COUNT; i++)
	{
		auto& a = _controllerStates[i];
		long long v = _LLONG_MAX;
		for (auto& b : kc[i])
		{
			long long n = _LLONG_MAX;
			if (b.type == KeyConfig::KeyType::Keyboard)
				n = keyStates()[b.id];
			else if (KeyConfig::KeyType::JoyPad1 <= b.type && b.type <= KeyConfig::KeyType::JoyPad4)
				n = joypadStates()[b.type - KeyConfig::KeyType::JoyPad1][b.id];
			else
				continue;
			if (std::abs(n) < std::abs(v))
				v = n;
		}
		if (v != _LLONG_MAX)
			a = v;
	}

	// Cursor position
	dx::GetMousePoint(&_cursor_x, &_cursor_y);

	// Mute
	if (_config.minimizeMute())
		_soundManager->mute(isMinimized());

	return !dx::ScreenFlip() && !dx::ProcessMessage() && !dx::ClearDrawScreen() && _currentScene->update();
}

void Window::draw()
{
	auto handle = _currentScene->screenHandle();
	dx::SetDrawScreen(handle);
	dx::FillGraph(handle, 0, 0, 0);
	_currentScene->draw();
	dx::SetDrawScreen(DX_SCREEN_BACK);
	dx::SetDrawMode(config().sceneFilter() ? DX_DRAWMODE_BILINEAR : DX_DRAWMODE_NEAREST);
	dx::SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);
	dx::SetDrawBright(255, 255, 255);
	dx::DrawExtendGraph(0, 0, config().resolutionWidth(), config().resolutionHeight(), handle, false);
}
