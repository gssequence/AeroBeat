#pragma once

#include "MapConfig.h"
#include "SoundManager.h"

class SystemSoundManager
{
public:
	enum SoundType
	{
		SelectBgm,
		DecideBgm,
		OptionChange,
		OptionOpen,
		OptionClose,
		FolderOpen,
		FolderClose,
		Scratch,
		PlayStop,
		Clear,
		Fail
	};

private:
	std::shared_ptr<SoundManager> _soundManager;
	MapConfig _config;
	std::unordered_map<SoundType, std::shared_ptr<SoundManager::Sound>> _sounds;

public:
	SystemSoundManager() { }
	virtual ~SystemSoundManager() { }

	void initialize(std::shared_ptr<SoundManager> manager, std::function<void(std::wstring)> logger);
	void load();

	void set_bgm(std::wstring value) { _config.elements()[L"bgm"] = value; }
	void set_sound(std::wstring value) { _config.elements()[L"sound"] = value; }
	const auto& sound(SoundType type) { return _sounds[type]; }
};
