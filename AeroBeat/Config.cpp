#include "stdafx.h"
#include "util.h"
#include "Config.h"

#define AB_CONFIG_RESOLUTION_WIDTH L"config.resolution_width"
#define AB_CONFIG_RESOLUTION_HEIGHT L"config.resolution_height"
#define AB_CONFIG_FULLSCREEN L"config.fullscreen"
#define AB_CONFIG_VSYNC L"config.vsync"
#define AB_CONFIG_SCENE_FILTER L"config.scene_filter"
#define AB_CONFIG_SCAN_BMS_AT_LAUNCH L"config.scan_bms_at_launch"
#define AB_CONFIG_DISABLE_PORTAUDIO L"config.disable_portaudio"
#define AB_CONFIG_USE_DEFAULT_AUDIO_DEVICE L"config.use_default_audio_device"
#define AB_CONFIG_AUDIO_DEVICE_INDEX L"config.audio_device_index"
#define AB_CONFIG_MINIMIZE_MUTE L"config.minimize_mute"
#define AB_CONFIG_BMS_DIRECTORIES L"config.bms_directories"
#define AB_CONFIG_HI_SPEED_BASE L"config.hi_speed_base"
#define AB_CONFIG_HI_SPEED_MIN L"config.hi_speed_min"
#define AB_CONFIG_HI_SPEED_MAX L"config.hi_speed_max"
#define AB_CONFIG_HI_SPEED_MARGIN L"config.hi_speed_margin"
#define AB_CONFIG_LANE_COVER_MARGIN L"config.lane_cover_margin"

Config::Config()
{
}

Config::~Config()
{
}

boost::optional<Config> Config::load(std::wstring path)
{
	std::wstring str;
	if (!util::decode(path, str)) return boost::none;
	std::wstringstream ifs(str);

	boost::property_tree::wptree tree;
	boost::property_tree::json_parser::read_json(ifs, tree);

	Config ret;
	util::unwrap(tree.get_optional<int>(AB_CONFIG_RESOLUTION_WIDTH), ret._resolutionWidth);
	util::unwrap(tree.get_optional<int>(AB_CONFIG_RESOLUTION_HEIGHT), ret._resolutionHeight);
	util::unwrap(tree.get_optional<bool>(AB_CONFIG_FULLSCREEN), ret._fullscreen);
	util::unwrap(tree.get_optional<bool>(AB_CONFIG_VSYNC), ret._vsync);
	util::unwrap(tree.get_optional<bool>(AB_CONFIG_SCENE_FILTER), ret._sceneFilter);
	util::unwrap(tree.get_optional<bool>(AB_CONFIG_SCAN_BMS_AT_LAUNCH), ret._scanBmsAtLaunch);
	util::unwrap(tree.get_optional<bool>(AB_CONFIG_DISABLE_PORTAUDIO), ret._disablePortAudio);
	util::unwrap(tree.get_optional<bool>(AB_CONFIG_USE_DEFAULT_AUDIO_DEVICE), ret._useDefaultAudioDevice);
	util::unwrap(tree.get_optional<int>(AB_CONFIG_AUDIO_DEVICE_INDEX), ret._audioDeviceIndex);
	util::unwrap(tree.get_optional<bool>(AB_CONFIG_MINIMIZE_MUTE), ret._minimizeMute);
	auto dirsop = tree.get_child_optional(AB_CONFIG_BMS_DIRECTORIES);
	if (dirsop)
	{
		for (auto& a : *dirsop)
		{
			auto op = a.second.get_value_optional<std::wstring>();
			if (op)
				ret._bmsDirectories.push_back(*op);
		}
	}
	util::unwrap(tree.get_optional<double>(AB_CONFIG_HI_SPEED_BASE), ret._hiSpeedBase);
	util::unwrap(tree.get_optional<int>(AB_CONFIG_HI_SPEED_MIN), ret._hiSpeedMin);
	util::unwrap(tree.get_optional<int>(AB_CONFIG_HI_SPEED_MAX), ret._hiSpeedMax);
	util::unwrap(tree.get_optional<int>(AB_CONFIG_HI_SPEED_MARGIN), ret._hiSpeedMargin);
	util::unwrap(tree.get_optional<int>(AB_CONFIG_LANE_COVER_MARGIN), ret._laneCoverMargin);

	return ret;
}
