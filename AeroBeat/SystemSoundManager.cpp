#include "stdafx.h"
#include "SystemSoundManager.h"

void SystemSoundManager::initialize(std::shared_ptr<SoundManager> manager, std::function<void(std::wstring)> logger)
{
	_soundManager = manager;
	MapConfig::load(L"ABFiles/Settings/SystemSounds.json", _config);
	logger(L"Loading system sounds...");
	load();
}

void SystemSoundManager::load()
{
	auto bgms = fs::path(L"ABFiles/BGMs") / _config.elements()[L"bgm"];
	auto sounds = fs::path(L"ABFiles/Sounds") / _config.elements()[L"sound"];

	_sounds[SoundType::SelectBgm] = _soundManager->load(bgms / L"select", true);
	_sounds[SoundType::SelectBgm]->master_volume(0.8);
	_sounds[SoundType::DecideBgm] = _soundManager->load(bgms / L"decide");
	_sounds[SoundType::DecideBgm]->master_volume(0.8);

	_sounds[SoundType::OptionChange] = _soundManager->load(sounds / L"o-change");
	_sounds[SoundType::OptionOpen] = _soundManager->load(sounds / L"o-open");
	_sounds[SoundType::OptionClose] = _soundManager->load(sounds / L"o-close");
	_sounds[SoundType::FolderOpen] = _soundManager->load(sounds / L"f-open");
	_sounds[SoundType::FolderClose] = _soundManager->load(sounds / L"f-close");
	_sounds[SoundType::Scratch] = _soundManager->load(sounds / L"scratch");

	_sounds[SoundType::PlayStop] = _soundManager->load(sounds / L"playstop");

	_sounds[SoundType::Clear] = _soundManager->load(sounds / L"clear");
	_sounds[SoundType::Fail] = _soundManager->load(sounds / L"fail");
}
