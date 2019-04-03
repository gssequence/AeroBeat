#pragma once

#define AB_CONFIG_PATH L"ABFiles/Settings/Config.json"

class Config
{
private:
	int _resolutionWidth = 1280;
	int _resolutionHeight = 720;
	bool _fullscreen = false;
	bool _vsync = false;
	bool _sceneFilter = false;
	bool _scanBmsAtLaunch = true;
	bool _disablePortAudio = false;
	bool _useDefaultAudioDevice = true;
	bool _minimizeMute = true;
	int _audioDeviceIndex = 0;
	std::vector<std::wstring> _bmsDirectories;
	double _hiSpeedBase = 1;
	int _hiSpeedMin = 25;
	int _hiSpeedMax = 900;
	int _hiSpeedMargin = 25;
	int _laneCoverMargin = 10;

public:
	Config();
	virtual ~Config();

	static boost::optional<Config> load(std::wstring path = AB_CONFIG_PATH);

	int resolutionWidth() { return _resolutionWidth; }
	int resolutionHeight() { return _resolutionHeight; }
	bool fullscreen() { return _fullscreen; }
	bool vsync() { return _vsync; }
	bool sceneFilter() { return _sceneFilter; }
	bool scanBmsAtLaunch() { return _scanBmsAtLaunch; }
	bool disablePortAudio() { return _disablePortAudio; }
	bool useDefaultAudioDevice() { return _useDefaultAudioDevice; }
	bool minimizeMute() { return _minimizeMute; }
	int audioDeviceIndex() { return _audioDeviceIndex; }
	const auto& bmsDirectories() { return _bmsDirectories; }
	auto hiSpeedBase() { return _hiSpeedBase; }
	auto hiSpeedMin() { return _hiSpeedMin; }
	auto hiSpeedMax() { return _hiSpeedMax; }
	auto hiSpeedMargin() { return _hiSpeedMargin; }
	auto laneCoverMargin() { return _laneCoverMargin; }
};
