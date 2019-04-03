#pragma once

#define AB_BMS_CHANNEL_BGM (36 * 0 + 1)
#define AB_BMS_CHANNEL_BAR_LENGTH (36 * 0 + 2)
#define AB_BMS_CHANNEL_BPM_CHANGE (36 * 0 + 3)
#define AB_BMS_CHANNEL_BGA_BASE (36 * 0 + 4)
#define AB_BMS_CHANNEL_BGA_POOR (36 * 0 + 6)
#define AB_BMS_CHANNEL_BGA_LAYER (36 * 0 + 7)
#define AB_BMS_CHANNEL_EXBPM_CHANGE (36 * 0 + 8)
#define AB_BMS_CHANNEL_STOP (36 * 0 + 9)

#define AB_BMS_CHANNEL_EX_BAR_LINE (36 * 36 + 1)
#define AB_BMS_CHANNEL_EX_LN_END (36 * 36 + 2)
#define AB_BMS_CHANNEL_EX_BEAT (36 * 36 + 3)

#define AB_BMS_CHANNEL_NOTE_BEGIN (36 * 1 + 1)
#define AB_BMS_CHANNEL_NOTE_END (36 * 2 + 35)
#define AB_BMS_CHANNEL_LN_BEGIN (36 * 5 + 1)
#define AB_BMS_CHANNEL_LN_END (36 * 6 + 35)

#define AB_BMS_CHANNEL_NOTE(a) (AB_BMS_CHANNEL_NOTE_BEGIN <= a && a <= AB_BMS_CHANNEL_NOTE_END)
#define AB_BMS_CHANNEL_LN(a) (AB_BMS_CHANNEL_LN_BEGIN <= a && a <= AB_BMS_CHANNEL_LN_END)

class BmsData
{
public:
	struct Event
	{
		double time;
		int channel;
		int data;
		double extra;
	};

private:
	bool _has_ln = false, _has_random = false;
	int _player, _judgeRank = 2, _playLevel = 0, _difficulty = 0, _lnobj = -1, _totalnotes = 0;
	double _total = 160, _initialBpm = 130, _minBpm, _maxBpm;
	std::wstring _stageFile, _banner, _backbmp, _title, _subtitle, _artist, _subartist, _genre;
	std::unordered_map<int, int> _stops;
	std::unordered_map<int, double> _bpms, _barLengths;
	std::unordered_map<int, std::wstring> _wavs, _bmps;
	std::vector<Event> _events;

public:
	BmsData() { }
	virtual ~BmsData() { }

	static bool parse(std::wstring str, BmsData& data, std::function<unsigned int(unsigned int)> next = [] (auto max) { return 0; });

	const auto has_ln() { return _has_ln; }
	const auto has_random() { return _has_random; }
	const auto player() { return _player; }
	const auto judgeRank() { return _judgeRank; }
	const auto playLevel() { return _playLevel; }
	const auto difficulty() { return _difficulty; }
	const auto lnobj() { return _lnobj; }
	const auto totalnotes() { return _totalnotes; }
	const auto total() { return _total; }
	const auto initialBpm() { return _initialBpm; }
	const auto minBpm() { return _minBpm; }
	const auto maxBpm() { return _maxBpm; }
	const auto& stageFile() { return _stageFile; }
	const auto& banner() { return _banner; }
	const auto& backbmp() { return _backbmp; }
	const auto& title() { return _title; }
	const auto& subtitle() { return _subtitle; }
	const auto& artist() { return _artist; }
	const auto& subartist() { return _subartist; }
	const auto& genre() { return _genre; }
	const auto& wavs() { return _wavs; }
	const auto& bmps() { return _bmps; }
	const auto& events() { return _events; }
};
