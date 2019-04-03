#include "stdafx.h"
#include "util.h"
#include "MapConfig.h"
#include "PlayOption.h"

#define AB_PLAYOPTION_PATH L"ABFiles/Settings/PlayOption.json"

#define AB_PLAYOPTION_GAUGE_TYPE L"gauge_type"
#define AB_PLAYOPTION_STYLE L"style"
#define AB_PLAYOPTION_HI_SPEED L"hi_speed"
#define AB_PLAYOPTION_LANE_COVER L"lane_cover"
#define AB_PLAYOPTION_LANE_COVER_ENABLED L"lane_cover_enabled"
#define AB_PLAYOPTION_DISPLAY_TIMING_ADJUSTMENT L"display_timing_adjustment"

void PlayOption::load()
{
	MapConfig config;
	MapConfig::load(AB_PLAYOPTION_PATH, config);
	util::try_stod_round(config.elements()[AB_PLAYOPTION_GAUGE_TYPE], (int&)gauge_type);
	util::try_stod_round(config.elements()[AB_PLAYOPTION_STYLE], (int&)style);
	util::try_stod_round(config.elements()[AB_PLAYOPTION_HI_SPEED], hi_speed);
	util::try_stod_round(config.elements()[AB_PLAYOPTION_LANE_COVER], lane_cover);
	lane_cover_enabled = config.elements()[AB_PLAYOPTION_LANE_COVER_ENABLED] == L"true";
	util::try_stod_round(config.elements()[AB_PLAYOPTION_DISPLAY_TIMING_ADJUSTMENT], display_timing_adjustment);
}

void PlayOption::save()
{
	MapConfig config;
	config.elements()[AB_PLAYOPTION_GAUGE_TYPE] = std::to_wstring(gauge_type);
	config.elements()[AB_PLAYOPTION_STYLE] = std::to_wstring(style);
	config.elements()[AB_PLAYOPTION_HI_SPEED] = std::to_wstring(hi_speed);
	config.elements()[AB_PLAYOPTION_LANE_COVER] = std::to_wstring(lane_cover);
	config.elements()[AB_PLAYOPTION_LANE_COVER_ENABLED] = lane_cover_enabled ? L"true" : L"false";
	config.elements()[AB_PLAYOPTION_DISPLAY_TIMING_ADJUSTMENT] = std::to_wstring(display_timing_adjustment);
	config.save(AB_PLAYOPTION_PATH);
}

int PlayOption::clear_lamp()
{
	if (gauge_type == AssistedEasy)
		return 2;
	else if (gauge_type == Easy)
		return 3;
	else if (gauge_type == Normal)
		return 4;
	else if (gauge_type == Hard)
		return 5;
	else if (gauge_type == ExHard)
		return 6;
	else if (gauge_type == Hazard)
		return 7;
	return 2;
}

int PlayOption::fullcombo_lamp()
{
	return 7;
}

double PlayOption::groove_gauge_initial()
{
	if (gauge_type == AssistedEasy)
		return 20;
	else if (gauge_type == Easy)
		return 20;
	else if (gauge_type == Normal)
		return 20;
	else if (gauge_type == Hard)
		return 100;
	else if (gauge_type == ExHard)
		return 100;
	else if (gauge_type == Hazard)
		return 100;
	return 100;
}

double PlayOption::groove_gauge_great(double total, int total_notes)
{
	if (gauge_type == AssistedEasy || gauge_type == Easy)
		return (total / total_notes) * 1.2;
	else if (gauge_type == Normal)
		return total / total_notes;
	else if (gauge_type == Hard)
		return 0.1;
	else if (gauge_type == ExHard)
		return 0.1;
	else if (gauge_type == Hazard)
		return 0.1;
	return 0;
}

double PlayOption::groove_gauge_good(double total, int total_notes)
{
	return groove_gauge_great(total, total_notes) / 2;
}

double PlayOption::groove_gauge_bad(double total, int total_notes)
{
	if (gauge_type == AssistedEasy || gauge_type == Easy)
		return -3.2;
	else if (gauge_type == Normal)
		return -4;
	else if (gauge_type == Hard)
		return -6;
	else if (gauge_type == ExHard)
		return -12;
	else if (gauge_type == Hazard)
		return -100;
	return -100;
}

double PlayOption::groove_gauge_poor(double total, int total_notes)
{
	if (gauge_type == AssistedEasy || gauge_type == Easy)
		return -4.8;
	else if (gauge_type == Normal)
		return -6;
	else if (gauge_type == Hard)
		return -10;
	else if (gauge_type == ExHard)
		return -20;
	else if (gauge_type == Hazard)
		return -100;
	return -100;
}

double PlayOption::groove_gauge_poor_blank(double total, int total_notes)
{
	if (gauge_type == AssistedEasy || gauge_type == Easy)
		return -1.6;
	else if (gauge_type == Normal)
		return -2;
	else if (gauge_type == Hard)
		return -2;
	else if (gauge_type == ExHard)
		return -4;
	else if (gauge_type == Hazard)
		return -4;
	return -4;
}
