#pragma once

class PlayOption
{
public:
	enum GaugeType
	{
		Normal,
		Easy,
		AssistedEasy,
		Hard,
		ExHard,
		Hazard
	};

	enum Style
	{
		Regular,
		Mirror,
		Random,
		SRandom,
		RRandom
	};

public:
	bool auto_play = false;
	GaugeType gauge_type = Normal;
	Style style = Regular;
	int hi_speed = 250;
	int lane_cover = 0;
	bool lane_cover_enabled = true;
	int display_timing_adjustment = 0;

	PlayOption() { }
	virtual ~PlayOption() { }

	void load();
	void save();

	int clear_lamp();
	int fullcombo_lamp();
	double groove_gauge_initial();
	double groove_gauge_great(double total, int total_notes);
	double groove_gauge_good(double total, int total_notes);
	double groove_gauge_bad(double total, int total_notes);
	double groove_gauge_poor(double total, int total_notes);
	double groove_gauge_poor_blank(double total, int total_notes);
};
