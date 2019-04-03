#include "stdafx.h"
#include "Window.h"
#include "MusicSelectScene.h"
#include "ResultScene.h"
#include "PlayScene.h"

#define AB_TIMER_SCENE L"scene"
#define AB_TIMER_READY L"ready"
#define AB_TIMER_FADE_OUT L"fade_out"
#define AB_TIMER_PLAY L"play"
#define AB_TIMER_CLOSE L"close"

PlayScene::PlayScene(Window* window, Skin& skin, std::shared_ptr<SongManager::Node> node) : SceneBase(window, skin.resolutionWidth(), skin.resolutionHeight()), _skin(skin)
{
	_node = node;
	auto& song = node->song();

	// ランダム初期化
	_seed = std::random_device()();
	_random = std::mt19937(_seed);

	// BMSファイル読み込み
	std::wstring str, hash;
	if (!util::decode((fs::path(song.folder) / song.filename).generic_wstring(), str, &hash) || hash != song.hash)
		return;
	auto next = [&](unsigned int max)
	{
		return std::uniform_int_distribution<unsigned int>(0, max)(_random);
	};
	if (!BmsData::parse(str, _data, next))
		return;

	// 変数初期化
	_max = _data.totalnotes() * 2;
	_aaa_score = (long long)std::ceil(_max * (8 / 9.));
	_aa_score = (long long)std::ceil(_max * (7 / 9.));
	_a_score = (long long)std::ceil(_max * (6 / 9.));
	_b_score = (long long)std::ceil(_max * (5 / 9.));
	_c_score = (long long)std::ceil(_max * (4 / 9.));
	_d_score = (long long)std::ceil(_max * (3 / 9.));
	_e_score = (long long)std::ceil(_max * (2 / 9.));
	_current_bpm = _data.initialBpm();

	// ノーツ情報をラップ
	auto style = _window->playOption().style;
	if (style == PlayOption::Style::SRandom)
	{
		std::array<int, 7> base =
		{
			36 * 1 + 1,
			36 * 1 + 2,
			36 * 1 + 3,
			36 * 1 + 4,
			36 * 1 + 5,
			36 * 1 + 8,
			36 * 1 + 9,
		};
		auto time = std::numeric_limits<double>::lowest();
		std::unordered_map<int, bool> reserved;
		auto reset = [&]
		{
			for (auto a : base)
				reserved[a] = false;
		};
		reset();
		for (auto&& a : _data.events())
		{
			if (a.time != time)
			{
				time = a.time;
				reset();
			}
			Event e = { a, false, Judge::None, false, false };
			if (util::get(reserved, e.e.channel))
			{
				while (true)
				{
					auto dst = base[std::uniform_int_distribution<int>(0, 6)(_random)];
					if (!reserved[dst])
					{
						e.e.channel = dst;
						reserved[dst] = true;
						break;
					}
				};
			}
			_events.push_back(e);
		}
	}
	else
	{
		std::array<int, 7> base =
		{
			36 * 1 + 1,
			36 * 1 + 2,
			36 * 1 + 3,
			36 * 1 + 4,
			36 * 1 + 5,
			36 * 1 + 8,
			36 * 1 + 9,
		};
		std::array<int, 7> dst = base;
		if (style == PlayOption::Style::Mirror)
			std::reverse(dst.begin(), dst.end());
		else if (style == PlayOption::Style::Random)
		{
			auto mirror = base;
			std::reverse(mirror.begin(), mirror.end());
			do std::shuffle(dst.begin(), dst.end(), _random);
			while (dst == base || dst == mirror);
		}
		else if (style == PlayOption::Style::RRandom)
		{
			if (std::bernoulli_distribution()(_random))
				std::reverse(dst.begin(), dst.end());
			std::rotate(dst.begin(), dst.begin() + std::uniform_int_distribution<int>(1, 6)(_random), dst.end());
		}
		std::unordered_map<int, int> keymap;
		for (size_t i = 0; i < base.size(); i++)
			keymap[base[i]] = dst[i];

		for (auto&& a : _data.events())
		{
			Event e = { a, false, Judge::None, false, false };
			auto okv = util::get(keymap, e.e.channel);
			if (okv) e.e.channel = *okv;
			_events.push_back(e);
		}
	}

	// BACKBMP読み込み
	_engine.images()[L"backbmp"] = Image::load(node->file_path(_data.backbmp()));

	// スキンエンジン初期化
	_engine.interpret(skin, [this](const Skin::Element& e) -> std::shared_ptr<SkinEngine::Element>
	{
		if (e.type == L"bga")
		{
			auto ptr = std::shared_ptr<BgaElement>(new BgaElement(*this, e));
			if (!ptr->fail())
				return ptr;
		}
		else if (e.type == L"bar_line")
		{
			auto ptr = std::shared_ptr<BarLineElement>(new BarLineElement(*this, e));
			if (!ptr->fail())
				return ptr;
		}
		else if (e.type == L"note")
		{
			if (e.dst_params.size() == 0)
			{
				// src
				auto type = util::get_any<std::wstring, std::wstring>(e.src_params, L"type", L"");
				auto index = util::round(util::get_any<std::wstring, double>(e.src_params, L"index", 0));
				if (0 <= index && index < AB_LANE_COUNT)
				{
					auto img = SkinEngine::PreparedImage::make(_engine, e);
					if (!img->fail())
					{
						if (type == L"note")
							_note_images[index] = img;
						else if (type == L"mine")
							_mine_images[index] = img;
						else if (type == L"long_begin")
							_long_note_begin_images[index] = img;
						else if (type == L"long_end")
							_long_note_end_images[index] = img;
						else if (type == L"long_body_on")
							_long_note_body_on_images[index] = img;
						else if (type == L"long_body_off")
							_long_note_body_off_images[index] = img;
					}
				}
			}
			else
			{
				// dst
				auto ptr = std::shared_ptr<NoteElement>(new NoteElement(*this, e));
				if (!ptr->fail())
					return ptr;
			}
		}
		else if (e.type == L"groovegauge")
		{
			if (e.dst_params.size() == 0)
			{
				// src
				auto type = util::get_any<std::wstring, std::wstring>(e.src_params, L"type", L"");
				auto img = SkinEngine::PreparedImage::make(_engine, e);
				if (!img->fail())
					_groovegauge_images[type] = img;
			}
			else
			{
				// dst
				auto ptr = std::shared_ptr<GrooveGaugeElement>(new GrooveGaugeElement(*this, e));
				if (!ptr->fail())
					return ptr;
			}
		}
		else if (e.type == L"judge")
		{
			auto ptr = std::shared_ptr<JudgeElement>(new JudgeElement(*this, e));
			if (!ptr->fail())
				return ptr;
		}
		else if (e.type == L"lane_height")
		{
			auto ov = util::get_any<std::wstring, double>(e.src_params, L"value");
			if (ov)
			{
				auto value = util::round(*ov);
				if (value > 0)
					_lane_height = value;
			}
		}

		return _engine.default_element(e);
	});

	// スキンパラメータ設定
	auto& flags = _engine.flags();
	auto& texts = _engine.texts();
	auto& numbers = _engine.numbers();
	flags[L"undefined"] = _data.difficulty() == 0;
	flags[L"beginner"] = _data.difficulty() == 1;
	flags[L"normal"] = _data.difficulty() == 2;
	flags[L"hyper"] = _data.difficulty() == 3;
	flags[L"another"] = _data.difficulty() == 4;
	flags[L"insane"] = _data.difficulty() == 5;
	flags[L"softlanding"] = _data.minBpm() != _data.maxBpm();
	flags[L"backbmp"] = !_engine.images()[L"backbmp"]->fail();
	flags[L"auto"] = _window->playOption().auto_play;
	texts[L"title"] = _data.title();
	texts[L"subtitle"] = _data.subtitle();
	texts[L"artist"] = _data.artist();
	texts[L"subartist"] = _data.subartist();
	texts[L"genre"] = _data.genre();
	numbers[L"level"] = _data.playLevel();
	numbers[L"minbpm"] = util::round(_data.minBpm());
	numbers[L"maxbpm"] = util::round(_data.maxBpm());
	numbers[L"max"] = _max;

	// ハイスピード係数設定
	_hi_speed_exp = 10 * _window->config().hiSpeedBase() / _lane_height;

	// グルーブゲージ設定
	_groove_gauge = _window->playOption().groove_gauge_initial();
	_groove_great = _window->playOption().groove_gauge_great(_data.total(), _data.totalnotes());
	_groove_good = _window->playOption().groove_gauge_good(_data.total(), _data.totalnotes());
	_groove_bad = _window->playOption().groove_gauge_bad(_data.total(), _data.totalnotes());
	_groove_poor = _window->playOption().groove_gauge_poor(_data.total(), _data.totalnotes());
	_groove_poor_blank = _window->playOption().groove_gauge_poor_blank(_data.total(), _data.totalnotes());

	// 判定幅設定
	auto rank = util::clip(_data.judgeRank(), 0, 3);
	if (rank == 0) _pgreat_range = 0.008;
	else if (rank == 1) _pgreat_range = 0.015;
	else if (rank == 2) _pgreat_range = 0.018;
	else if (rank == 3) _pgreat_range = 0.021;

	// BGAロード(スレッドセーフではないのでメインスレッドでロード)
	auto& bmps = _data.bmps();
	int w = 256;
	int h = 256;
	for (auto& a : bmps)
	{
		auto img = Image::load(_node->file_path(a.second));
		_images[a.first] = img;
		if (!img->fail())
		{
			w = max(w, img->width());
			h = max(h, img->height());
		}
	}
	_bga_composite = Image::make(dx::MakeScreen(w, h));

	// 音声ロード開始(別スレッド)
	std::thread([this]
	{
		auto& wavs = _data.wavs();
		int den = wavs.size();
		int num = 0;

		for (auto& a : wavs)
		{
			auto sound = _window->soundManager().load(_node->file_path(a.second));
			sound->volume(0.9);
			_sounds[a.first] = sound;
			num++;
			_load_progress = (double)num / den;
		}

		_load_progress = 1;

		// 曲の長さを計算
		for (auto& a : _data.events())
		{
			if (a.channel == AB_BMS_CHANNEL_BGM || AB_BMS_CHANNEL_NOTE(a.channel))
			{
				auto sound = _sounds[a.data];
				double duration = 0;
				if (sound && !sound->fail())
					duration = sound->duration();
				auto end = a.time + duration;
				_song_duration = max(_song_duration, end);
			}
		}

		_loading = false;
	}).detach();

	// タイマースタート
	_engine.timers()[AB_TIMER_SCENE].start();

	// マウス入力クリア
	dx::GetMouseWheelRotVol();

	_fail = false;
}

bool PlayScene::update()
{
	static bool switch_lane_cover = false;

	auto& timers = _engine.timers();
	auto& flags = _engine.flags();
	auto& texts = _engine.texts();
	auto& numbers = _engine.numbers();
	auto& bargraph_values = _engine.bargraph_values();
	auto& slider_values = _engine.slider_values();
	auto& ks = _window->keyStates();
	auto& cs = _window->controllerStates();
	auto& op = _window->playOption();

	if (_fail)
	{
		auto& skin = _window->skinManager().selectedSkin(Skin::MusicSelect);
		_window->switchScene(std::shared_ptr<SceneBase>(new MusicSelectScene(_window, skin)));
		return true;
	}

	// キータイマー
	if (!op.auto_play)
	{
		if (cs[AB_KEY_7KEYS_1P_SL] == -1 || cs[AB_KEY_7KEYS_1P_SR] == -1)
		{
			timers[L"scr_on"].stop();
			timers[L"scr_off"].restart();
		}
		if (cs[AB_KEY_7KEYS_1P_1] == -1)
		{
			timers[L"key1_on"].stop();
			timers[L"key1_off"].restart();
		}
		if (cs[AB_KEY_7KEYS_1P_2] == -1)
		{
			timers[L"key2_on"].stop();
			timers[L"key2_off"].restart();
		}
		if (cs[AB_KEY_7KEYS_1P_3] == -1)
		{
			timers[L"key3_on"].stop();
			timers[L"key3_off"].restart();
		}
		if (cs[AB_KEY_7KEYS_1P_4] == -1)
		{
			timers[L"key4_on"].stop();
			timers[L"key4_off"].restart();
		}
		if (cs[AB_KEY_7KEYS_1P_5] == -1)
		{
			timers[L"key5_on"].stop();
			timers[L"key5_off"].restart();
		}
		if (cs[AB_KEY_7KEYS_1P_6] == -1)
		{
			timers[L"key6_on"].stop();
			timers[L"key6_off"].restart();
		}
		if (cs[AB_KEY_7KEYS_1P_7] == -1)
		{
			timers[L"key7_on"].stop();
			timers[L"key7_off"].restart();
		}

		if (cs[AB_KEY_7KEYS_1P_SL] == 1 || cs[AB_KEY_7KEYS_1P_SR] == 1)
		{
			timers[L"scr_on"].restart();
			timers[L"scr_off"].stop();
		}
		if (cs[AB_KEY_7KEYS_1P_1] == 1)
		{
			timers[L"key1_on"].restart();
			timers[L"key1_off"].stop();
		}
		if (cs[AB_KEY_7KEYS_1P_2] == 1)
		{
			timers[L"key2_on"].restart();
			timers[L"key2_off"].stop();
		}
		if (cs[AB_KEY_7KEYS_1P_3] == 1)
		{
			timers[L"key3_on"].restart();
			timers[L"key3_off"].stop();
		}
		if (cs[AB_KEY_7KEYS_1P_4] == 1)
		{
			timers[L"key4_on"].restart();
			timers[L"key4_off"].stop();
		}
		if (cs[AB_KEY_7KEYS_1P_5] == 1)
		{
			timers[L"key5_on"].restart();
			timers[L"key5_off"].stop();
		}
		if (cs[AB_KEY_7KEYS_1P_6] == 1)
		{
			timers[L"key6_on"].restart();
			timers[L"key6_off"].stop();
		}
		if (cs[AB_KEY_7KEYS_1P_7] == 1)
		{
			timers[L"key7_on"].restart();
			timers[L"key7_off"].stop();
		}
	}

	// スタートボタンタイマー
	if (cs[AB_KEY_7KEYS_1P_START] == 1)
	{
		// レーンカバー切り替え
		auto off = timers[L"start_off"]();
		if (timers[L"start_off"].isEnabled() && off <= 0.3 && !switch_lane_cover)
		{
			op.lane_cover_enabled ^= true;
			switch_lane_cover = true;
		}
		else
			switch_lane_cover = false;
		timers[L"start_on"].restart();
		timers[L"start_off"].stop();
	}
	if (cs[AB_KEY_7KEYS_1P_START] == -1)
	{
		timers[L"start_on"].stop();
		timers[L"start_off"].restart();
	}

	// ハイスピード変更
	if (ks[KEY_INPUT_UP] == 1)
		set_hi_speed(op.hi_speed + _window->config().hiSpeedMargin());
	if (cs[AB_KEY_7KEYS_1P_START] > 0 && cs[AB_KEY_7KEYS_1P_2] == 1)
		set_hi_speed(op.hi_speed + _window->config().hiSpeedMargin());
	if (cs[AB_KEY_7KEYS_1P_START] > 0 && cs[AB_KEY_7KEYS_1P_4] == 1)
		set_hi_speed(op.hi_speed + _window->config().hiSpeedMargin());
	if (ks[KEY_INPUT_DOWN] == 1)
		set_hi_speed(op.hi_speed - _window->config().hiSpeedMargin());
	if (cs[AB_KEY_7KEYS_1P_START] > 0 && cs[AB_KEY_7KEYS_1P_1] == 1)
		set_hi_speed(op.hi_speed - _window->config().hiSpeedMargin());
	if (cs[AB_KEY_7KEYS_1P_START] > 0 && cs[AB_KEY_7KEYS_1P_3] == 1)
		set_hi_speed(op.hi_speed - _window->config().hiSpeedMargin());
	if (cs[AB_KEY_7KEYS_1P_START] > 0 && cs[AB_KEY_7KEYS_1P_5] == 1)
		set_hi_speed(op.hi_speed - _window->config().hiSpeedMargin());

	// レーンカバー変更
	if (cs[AB_KEY_7KEYS_1P_START] > 0 && cs[AB_KEY_7KEYS_1P_6] == 1)
		op.lane_cover = util::clip(op.lane_cover - _window->config().laneCoverMargin(), 0, 100);
	if (cs[AB_KEY_7KEYS_1P_START] > 0 && cs[AB_KEY_7KEYS_1P_7] == 1)
		op.lane_cover = util::clip(op.lane_cover + _window->config().laneCoverMargin(), 0, 100);
	op.lane_cover = util::clip(op.lane_cover - dx::GetMouseWheelRotVol(), 0, 100);

	// 演奏アップデート
	if (timers[AB_TIMER_PLAY].isEnabled())
		play_update();

	// 音を鳴らす
	if (cs[AB_KEY_7KEYS_1P_SL] == 1 || cs[AB_KEY_7KEYS_1P_SR] == 1)
	{
		if (_key_sounds[0] && !_key_sounds[0]->fail())
			_key_sounds[0]->play();
	}
	if (cs[AB_KEY_7KEYS_1P_1] == 1)
	{
		if (_key_sounds[1] && !_key_sounds[1]->fail())
			_key_sounds[1]->play();
	}
	if (cs[AB_KEY_7KEYS_1P_2] == 1)
	{
		if (_key_sounds[2] && !_key_sounds[2]->fail())
			_key_sounds[2]->play();
	}
	if (cs[AB_KEY_7KEYS_1P_3] == 1)
	{
		if (_key_sounds[3] && !_key_sounds[3]->fail())
			_key_sounds[3]->play();
	}
	if (cs[AB_KEY_7KEYS_1P_4] == 1)
	{
		if (_key_sounds[4] && !_key_sounds[4]->fail())
			_key_sounds[4]->play();
	}
	if (cs[AB_KEY_7KEYS_1P_5] == 1)
	{
		if (_key_sounds[5] && !_key_sounds[5]->fail())
			_key_sounds[5]->play();
	}
	if (cs[AB_KEY_7KEYS_1P_6] == 1)
	{
		if (_key_sounds[6] && !_key_sounds[6]->fail())
			_key_sounds[6]->play();
	}
	if (cs[AB_KEY_7KEYS_1P_7] == 1)
	{
		if (_key_sounds[7] && !_key_sounds[7]->fail())
			_key_sounds[7]->play();
	}

	// パラメータ設定
	refresh_score();
	auto exscore = _pgreat * 2 + _great;
	flags[L"lane_cover"] = op.lane_cover_enabled;

	numbers[L"pgreat"] = _pgreat;
	numbers[L"great"] = _great;
	numbers[L"good"] = _good;
	numbers[L"bad"] = _bad;
	numbers[L"poor"] = _poor;
	numbers[L"cbrk"] = _cbrk;
	numbers[L"max_combo"] = _max_combo;
	numbers[L"current_combo"] = _current_combo;
	numbers[L"score"] = _va_score();
	numbers[L"exscore"] = exscore;
	numbers[L"groovegauge"] = (long long)(std::floor(_groove_gauge / 2) * 2);
	numbers[L"bpm"] = util::round(_current_bpm);
	numbers[L"hispeed"] = op.hi_speed;
	numbers[L"lane_cover"] = op.lane_cover;
	numbers[L"visible_time"] = util::round(130000 / (_current_bpm * (op.hi_speed / 100.) * (100. / (100 - (op.lane_cover_enabled ? op.lane_cover : 0))))); // 係数が怪しい
	
	bargraph_values[L"score"] = exscore / (double)_max;
	bargraph_values[L"mybest"] = 0;
	bargraph_values[L"target"] = 0;
	bargraph_values[L"load"] = _load_progress;

	slider_values[L"lane_cover"] = op.lane_cover / 100.;

	if (_aaa_score <= exscore)
		timers[L"achieve_aaa"].start();
	if (_aa_score <= exscore)
		timers[L"achieve_aa"].start();
	if (_a_score <= exscore)
		timers[L"achieve_a"].start();
	if (_b_score <= exscore)
		timers[L"achieve_b"].start();
	if (_c_score <= exscore)
		timers[L"achieve_c"].start();
	if (_d_score <= exscore)
		timers[L"achieve_d"].start();
	if (_e_score <= exscore)
		timers[L"achieve_e"].start();

	if (_data.totalnotes() == _max_combo)
		timers[L"fullcombo"].start();

	// シーン制御
	if (_loading || timers[AB_TIMER_SCENE]() < _skin.loadEnd()) return true;

	if (timers[AB_TIMER_FADE_OUT].isEnabled())
	{
		_sounds.clear();
		_key_sounds = decltype(_key_sounds)();
		if (timers[AB_TIMER_FADE_OUT]() > _skin.fadeOut())
		{
			switch_to_result();
			return true;
		}
		return true;
	}

	timers[AB_TIMER_READY].start();
	if (timers[AB_TIMER_READY]() >= _skin.ready() && cs[AB_KEY_7KEYS_1P_START] < 0)
		timers[AB_TIMER_PLAY].start();

	if (timers[AB_TIMER_CLOSE].isEnabled())
	{
		_sounds.clear();
		_key_sounds = decltype(_key_sounds)();
		if (timers[AB_TIMER_CLOSE]() >= _skin.close())
		{
			switch_to_result();
			return true;
		}
	}

	if (!timers[AB_TIMER_CLOSE].isEnabled() && !timers[AB_TIMER_FADE_OUT].isEnabled() && (_window->keyStates()[KEY_INPUT_ESCAPE] == 1 || (_window->controllerStates()[AB_KEY_7KEYS_1P_START] > 0 && _window->controllerStates()[AB_KEY_7KEYS_1P_SELECT] > 0)))
	{
		if (_pgreat + _great + _good + _cbrk == _data.totalnotes())
		{
			timers[AB_TIMER_FADE_OUT].start();
		}
		else
		{
			_groove_gauge = 0;
			_window->systemSoundManager().sound(SystemSoundManager::SoundType::PlayStop)->play();
			timers[AB_TIMER_CLOSE].start();
		}
	}

	return true;
}

void PlayScene::play_update()
{
	auto& timers = _engine.timers();
	auto& flags = _engine.flags();
	auto& texts = _engine.texts();
	auto& numbers = _engine.numbers();
	auto& bargraph_values = _engine.bargraph_values();
	auto& slider_values = _engine.slider_values();
	auto& cs = _window->controllerStates();
	auto& op = _window->playOption();

	auto before = timers[AB_TIMER_PLAY].before();
	auto now = timers[AB_TIMER_PLAY]();
	auto hi_speed = _va_hi_speed();

	slider_values[L"play_position"] = now / _song_duration;

	double beat_back = std::numeric_limits<double>::lowest();
	double beat_forward = (std::numeric_limits<double>::max)();
	_beat_timer->start();

	_bar_line_positions.clear();
	for (auto& a : _note_positions)
		a.clear();
	for (auto& a : _mine_positions)
		a.clear();
	for (auto& a : _long_note_positions)
		a.clear();
	for (auto& a : _judged_lanes)
		a = false;

	double originPosition = 0;
	double originTime = now + (op.auto_play ? 0 : (op.display_timing_adjustment / 1000.));
	double calcBpm = _current_bpm;
	std::unordered_map<double, double> ln_ends;
	for (auto& a : _events)
	{
		bool ln_on = false;

		if (op.auto_play)
		{
			if (before <= a.e.time && a.e.time < now)
			{
				if (a.e.channel == AB_BMS_CHANNEL_BGM)
				{
					auto sound = _sounds[a.e.data];
					if (sound && !sound->fail())
						sound->play();
				}
				else if (AB_BMS_CHANNEL_NOTE(a.e.channel))
				{
					auto sound = _sounds[a.e.data];
					if (sound && !sound->fail())
						sound->play();
					if (a.e.extra == 0)
					{
						judge(Judge::Pgreat);

						if (a.e.channel == (36 * 1 + 6)) // 皿
						{
							timers[L"scr_off"].restart();
							timers[L"scr_bomb"].restart();
						}
						else if (a.e.channel == (36 * 1 + 1)) // SW1
						{
							timers[L"key1_off"].restart();
							timers[L"key1_bomb"].restart();
						}
						else if (a.e.channel == (36 * 1 + 2)) // SW2
						{
							timers[L"key2_off"].restart();
							timers[L"key2_bomb"].restart();
						}
						else if (a.e.channel == (36 * 1 + 3)) // SW3
						{
							timers[L"key3_off"].restart();
							timers[L"key3_bomb"].restart();
						}
						else if (a.e.channel == (36 * 1 + 4)) // SW4
						{
							timers[L"key4_off"].restart();
							timers[L"key4_bomb"].restart();
						}
						else if (a.e.channel == (36 * 1 + 5)) // SW5
						{
							timers[L"key5_off"].restart();
							timers[L"key5_bomb"].restart();
						}
						else if (a.e.channel == (36 * 1 + 8)) // SW6
						{
							timers[L"key6_off"].restart();
							timers[L"key6_bomb"].restart();
						}
						else if (a.e.channel == (36 * 1 + 9)) // SW7
						{
							timers[L"key7_off"].restart();
							timers[L"key7_bomb"].restart();
						}
					}
					else
					{
						if (a.e.channel == (36 * 1 + 6)) // 皿
						{
							timers[L"scr_on"].restart();
							timers[L"scr_long_bomb"].restart();
						}
						else if (a.e.channel == (36 * 1 + 1)) // SW1
						{
							timers[L"key1_on"].restart();
							timers[L"key1_long_bomb"].restart();
						}
						else if (a.e.channel == (36 * 1 + 2)) // SW2
						{
							timers[L"key2_on"].restart();
							timers[L"key2_long_bomb"].restart();
						}
						else if (a.e.channel == (36 * 1 + 3)) // SW3
						{
							timers[L"key3_on"].restart();
							timers[L"key3_long_bomb"].restart();
						}
						else if (a.e.channel == (36 * 1 + 4)) // SW4
						{
							timers[L"key4_on"].restart();
							timers[L"key4_long_bomb"].restart();
						}
						else if (a.e.channel == (36 * 1 + 5)) // SW5
						{
							timers[L"key5_on"].restart();
							timers[L"key5_long_bomb"].restart();
						}
						else if (a.e.channel == (36 * 1 + 8)) // SW6
						{
							timers[L"key6_on"].restart();
							timers[L"key6_long_bomb"].restart();
						}
						else if (a.e.channel == (36 * 1 + 9)) // SW7
						{
							timers[L"key7_on"].restart();
							timers[L"key7_long_bomb"].restart();
						}
					}
					a.processed = true;
				}
				else if (a.e.channel == AB_BMS_CHANNEL_BPM_CHANGE)
				{
					_current_bpm = a.e.extra;
				}
				else if (a.e.channel == AB_BMS_CHANNEL_BGA_BASE)
				{
					auto img = _images[a.e.data];
					if (img)
					{
						_bga_base = img;
						_bga_base->start();
					}
					else
						_bga_base = nullptr;
				}
				else if (a.e.channel == AB_BMS_CHANNEL_BGA_LAYER)
				{
					auto img = _images[a.e.data];
					if (img)
					{
						_bga_layer = img;
						_bga_layer->start();
					}
					else
						_bga_layer = nullptr;
				}
			}
			if (AB_BMS_CHANNEL_NOTE(a.e.channel) && a.e.extra != 0)
			{
				if (before <= a.e.extra && a.e.extra < now)
				{
					judge(Judge::Pgreat);

					if (a.e.channel == (36 * 1 + 6)) // 皿
					{
						timers[L"scr_on"].stop();
						timers[L"scr_long_bomb"].stop();
						timers[L"scr_off"].restart();
						timers[L"scr_bomb"].restart();
					}
					else if (a.e.channel == (36 * 1 + 1)) // SW1
					{
						timers[L"key1_on"].stop();
						timers[L"key1_long_bomb"].stop();
						timers[L"key1_off"].restart();
						timers[L"key1_bomb"].restart();
					}
					else if (a.e.channel == (36 * 1 + 2)) // SW2
					{
						timers[L"key2_on"].stop();
						timers[L"key2_long_bomb"].stop();
						timers[L"key2_off"].restart();
						timers[L"key2_bomb"].restart();
					}
					else if (a.e.channel == (36 * 1 + 3)) // SW3
					{
						timers[L"key3_on"].stop();
						timers[L"key3_long_bomb"].stop();
						timers[L"key3_off"].restart();
						timers[L"key3_bomb"].restart();
					}
					else if (a.e.channel == (36 * 1 + 4)) // SW4
					{
						timers[L"key4_on"].stop();
						timers[L"key4_long_bomb"].stop();
						timers[L"key4_off"].restart();
						timers[L"key4_bomb"].restart();
					}
					else if (a.e.channel == (36 * 1 + 5)) // SW5
					{
						timers[L"key5_on"].stop();
						timers[L"key5_long_bomb"].stop();
						timers[L"key5_off"].restart();
						timers[L"key5_bomb"].restart();
					}
					else if (a.e.channel == (36 * 1 + 8)) // SW6
					{
						timers[L"key6_on"].stop();
						timers[L"key6_long_bomb"].stop();
						timers[L"key6_off"].restart();
						timers[L"key6_bomb"].restart();
					}
					else if (a.e.channel == (36 * 1 + 9)) // SW7
					{
						timers[L"key7_on"].stop();
						timers[L"key7_long_bomb"].stop();
						timers[L"key7_off"].restart();
						timers[L"key7_bomb"].restart();
					}
				}
				if (AB_BMS_CHANNEL_NOTE(a.e.channel) && a.e.extra != 0 && a.e.time <= now && now < a.e.extra)
					ln_on = true;
			}
		}
		else
		{
			if (before <= a.e.time && a.e.time < now)
			{
				if (a.e.channel == AB_BMS_CHANNEL_BGM)
				{
					auto sound = _sounds[a.e.data];
					if (sound && !sound->fail())
						sound->play();
				}
				else if (a.e.channel == AB_BMS_CHANNEL_BPM_CHANGE)
				{
					_current_bpm = a.e.extra;
				}
				else if (a.e.channel == AB_BMS_CHANNEL_BGA_BASE)
				{
					auto img = _images[a.e.data];
					if (img)
					{
						_bga_base = img;
						_bga_base->start();
					}
					else
						_bga_base = nullptr;
				}
				else if (a.e.channel == AB_BMS_CHANNEL_BGA_LAYER)
				{
					auto img = _images[a.e.data];
					if (img)
					{
						_bga_layer = img;
						_bga_layer->start();
					}
					else
						_bga_layer = nullptr;
				}
			}
			if (AB_BMS_CHANNEL_NOTE(a.e.channel))
			{
				auto n = -1;
				bool keydown = false;
				bool keypress = false;
				bool keyup = false;
				switch (a.e.channel)
				{
				case (36 * 1 + 6):
					n = 0;
					keydown = _window->controllerStates()[AB_KEY_7KEYS_1P_SL] == 1 || _window->controllerStates()[AB_KEY_7KEYS_1P_SR] == 1;
					keypress = _window->controllerStates()[AB_KEY_7KEYS_1P_SL] >= 1 || _window->controllerStates()[AB_KEY_7KEYS_1P_SR] >= 1;
					keyup = _window->controllerStates()[AB_KEY_7KEYS_1P_SL] == -1 || _window->controllerStates()[AB_KEY_7KEYS_1P_SR] == -1;
					break;
				case (36 * 1 + 1):
					n = 1;
					keydown = _window->controllerStates()[AB_KEY_7KEYS_1P_1] == 1;
					keypress = _window->controllerStates()[AB_KEY_7KEYS_1P_1] >= 1;
					keyup = _window->controllerStates()[AB_KEY_7KEYS_1P_1] == -1;
					break;
				case (36 * 1 + 2):
					n = 2;
					keydown = _window->controllerStates()[AB_KEY_7KEYS_1P_2] == 1;
					keypress = _window->controllerStates()[AB_KEY_7KEYS_1P_2] >= 1;
					keyup = _window->controllerStates()[AB_KEY_7KEYS_1P_2] == -1;
					break;
				case (36 * 1 + 3):
					n = 3;
					keydown = _window->controllerStates()[AB_KEY_7KEYS_1P_3] == 1;
					keypress = _window->controllerStates()[AB_KEY_7KEYS_1P_3] >= 1;
					keyup = _window->controllerStates()[AB_KEY_7KEYS_1P_3] == -1;
					break;
				case (36 * 1 + 4):
					n = 4;
					keydown = _window->controllerStates()[AB_KEY_7KEYS_1P_4] == 1;
					keypress = _window->controllerStates()[AB_KEY_7KEYS_1P_4] >= 1;
					keyup = _window->controllerStates()[AB_KEY_7KEYS_1P_4] == -1;
					break;
				case (36 * 1 + 5):
					n = 5;
					keydown = _window->controllerStates()[AB_KEY_7KEYS_1P_5] == 1;
					keypress = _window->controllerStates()[AB_KEY_7KEYS_1P_5] >= 1;
					keyup = _window->controllerStates()[AB_KEY_7KEYS_1P_5] == -1;
					break;
				case (36 * 1 + 8):
					n = 6;
					keydown = _window->controllerStates()[AB_KEY_7KEYS_1P_6] == 1;
					keypress = _window->controllerStates()[AB_KEY_7KEYS_1P_6] >= 1;
					keyup = _window->controllerStates()[AB_KEY_7KEYS_1P_6] == -1;
					break;
				case (36 * 1 + 9):
					n = 7;
					keydown = _window->controllerStates()[AB_KEY_7KEYS_1P_7] == 1;
					keypress = _window->controllerStates()[AB_KEY_7KEYS_1P_7] >= 1;
					keyup = _window->controllerStates()[AB_KEY_7KEYS_1P_7] == -1;
					break;
				}
				if (n != -1)
				{
					if (!_judged_lanes[n])
					{
						// 音をセット
						if (!a.processed && std::abs(a.e.time - now) <= _poor_blank_range)
						{
							auto sound = _sounds[a.e.data];
							if (sound && !sound->fail())
								_key_sounds[n] = sound;
						}
						
						// 判定
						if (!a.processed)
						{
							if (keydown)
							{
								if (a.e.extra == 0)
								{
									if (std::abs(a.e.time - now) <= _pgreat_range)
									{
										a.processed = true;
										judge(Judge::Pgreat, now - a.e.time);
										_judged_lanes[n] = true;
										timers[L"key" + std::to_wstring(n) + L"_bomb"].restart();
									}
									else if (std::abs(a.e.time - now) <= _great_range)
									{
										a.processed = true;
										judge(Judge::Great, now - a.e.time);
										_judged_lanes[n] = true;
										timers[L"key" + std::to_wstring(n) + L"_bomb"].restart();
									}
									else if (std::abs(a.e.time - now) <= _good_range)
									{
										a.processed = true;
										judge(Judge::Good, now - a.e.time);
										_judged_lanes[n] = true;
									}
									else if (std::abs(a.e.time - now) <= _bad_range)
									{
										a.processed = true;
										judge(Judge::Bad, now - a.e.time);
										_judged_lanes[n] = true;
									}
									else if (std::abs(a.e.time - now) <= _poor_blank_range)
									{
										judge(Judge::PoorBlank);
										_judged_lanes[n] = true;
									}
								}
								else
								{
									if (std::abs(a.e.time - now) <= _pgreat_range)
									{
										a.processed = true;
										a.ln_judge = Judge::Pgreat;
										a.ln_delay = now - a.e.time;
										_judged_lanes[n] = true;
										ln_on = true;
										timers[L"key" + std::to_wstring(n) + L"_long_bomb"].restart();
									}
									else if (std::abs(a.e.time - now) <= _great_range)
									{
										a.processed = true;
										a.ln_judge = Judge::Great;
										a.ln_delay = now - a.e.time;
										_judged_lanes[n] = true;
										ln_on = true;
										timers[L"key" + std::to_wstring(n) + L"_long_bomb"].restart();
									}
									else if (std::abs(a.e.time - now) <= _good_range)
									{
										a.processed = true;
										a.ln_judge = Judge::Good;
										a.ln_delay = now - a.e.time;
										_judged_lanes[n] = true;
										ln_on = true;
										timers[L"key" + std::to_wstring(n) + L"_long_bomb"].restart();
									}
									else if (std::abs(a.e.time - now) <= _bad_range)
									{
										a.processed = true;
										a.ln_complete = true;
										judge(Judge::Bad, now - a.e.time);
										_judged_lanes[n] = true;
									}
								}
							}
							else if (now - a.e.time > _bad_range)
							{
								a.processed = true;
								a.ln_complete = true;
								judge(Judge::Poor);
								_judged_lanes[n] = true;
							}
						}

						// LN判定
						if (a.e.extra != 0 && a.processed && !a.ln_complete)
						{
							if (keypress)
							{
								if (a.e.extra <= now)
								{
									a.processed = true;
									a.ln_complete = true;
									judge(a.ln_judge, a.ln_delay);
									_judged_lanes[n] = true;
									timers[L"key" + std::to_wstring(n) + L"_long_bomb"].stop();
									if (a.ln_judge == Judge::Pgreat || a.ln_judge == Judge::Great)
										timers[L"key" + std::to_wstring(n) + L"_bomb"].restart();
								}
								else
									ln_on = true;
							}
							else if (keyup)
							{
								if (a.e.extra - _good_range >= now)
								{
									a.processed = true;
									a.ln_complete = true;
									judge(Judge::Bad, true);
									_judged_lanes[n] = true;
									_key_sounds[n]->stop();
								}
								else
								{
									a.processed = true;
									a.ln_complete = true;
									judge(a.ln_judge, a.ln_delay);
									_judged_lanes[n] = true;
									if (a.ln_judge == Judge::Pgreat || a.ln_judge == Judge::Great)
										timers[L"key" + std::to_wstring(n) + L"_bomb"].restart();
								}
								timers[L"key" + std::to_wstring(n) + L"_long_bomb"].stop();
							}
						}
					}
				}
			}
		}

		// 拍オブジェクト
		if (a.e.channel == AB_BMS_CHANNEL_EX_BEAT)
		{
			if (a.e.time < now && beat_back < a.e.time)
				beat_back = a.e.time;
			else if (a.e.time >= now && beat_forward > a.e.time)
				beat_forward = a.e.time;
		}

		// オブジェクト位置計算(判定ラインを0としY座標が0になるほど値は増加)
		if (a.e.channel == AB_BMS_CHANNEL_BPM_CHANGE && a.e.time >= before)
		{
			// BPMの変更があったとき
			// 以前のBPMを使用してBPMの変更位置をoriginとする
			originPosition += (calcBpm * (originTime - a.e.time));

			// BPMの変更時間をoriginとする
			originTime = a.e.time;

			// 計算用BPMの更新
			calcBpm = a.e.extra;
		}
		else if (a.e.channel == AB_BMS_CHANNEL_STOP && a.e.time + a.e.extra > now)
		{
			// 位置originを切り詰め
			if (a.e.time < now)
				originPosition += (calcBpm * (a.e.extra - (now - a.e.time)));
			else
				originPosition += (calcBpm * (a.e.extra));
		}

		// originからのY座標を計算
		double position = (-originPosition + calcBpm * (a.e.time - originTime)) * hi_speed * _hi_speed_exp;

		// 位置情報を保存
		if (a.e.channel == AB_BMS_CHANNEL_EX_BAR_LINE)
			_bar_line_positions.push_back(position);
		else if (a.e.channel == AB_BMS_CHANNEL_EX_LN_END)
			ln_ends[a.e.time] = position;
		else if (a.e.channel == (36 * 1 + 6)) // 皿
		{
			if (a.e.extra != 0)
				_long_note_positions[0].push_back({ position, a.e.extra, ln_on });
			else if (op.auto_play)
				_note_positions[0].push_back(position);
			else if (!a.processed)
				_note_positions[0].push_back(max(0,  position));
		}
		else if (a.e.channel == (36 * 1 + 1)) // SW1
		{
			if (a.e.extra != 0)
				_long_note_positions[1].push_back({ position, a.e.extra, ln_on });
			else if (op.auto_play)
				_note_positions[1].push_back(position);
			else if (!a.processed)
				_note_positions[1].push_back(max(0, position));
		}
		else if (a.e.channel == (36 * 1 + 2)) // SW2
		{
			if (a.e.extra != 0)
				_long_note_positions[2].push_back({ position, a.e.extra, ln_on });
			else if (op.auto_play)
				_note_positions[2].push_back(position);
			else if (!a.processed)
				_note_positions[2].push_back(max(0, position));
		}
		else if (a.e.channel == (36 * 1 + 3)) // SW3
		{
			if (a.e.extra != 0)
				_long_note_positions[3].push_back({ position, a.e.extra, ln_on });
			else if (op.auto_play)
				_note_positions[3].push_back(position);
			else if (!a.processed)
				_note_positions[3].push_back(max(0, position));
		}
		else if (a.e.channel == (36 * 1 + 4)) // SW4
		{
			if (a.e.extra != 0)
				_long_note_positions[4].push_back({ position, a.e.extra, ln_on });
			else if (op.auto_play)
				_note_positions[4].push_back(position);
			else if (!a.processed)
				_note_positions[4].push_back(max(0, position));
		}
		else if (a.e.channel == (36 * 1 + 5)) // SW5
		{
			if (a.e.extra != 0)
				_long_note_positions[5].push_back({ position, a.e.extra, ln_on });
			else if (op.auto_play)
				_note_positions[5].push_back(position);
			else if (!a.processed)
				_note_positions[5].push_back(max(0, position));
		}
		else if (a.e.channel == (36 * 1 + 8)) // SW6
		{
			if (a.e.extra != 0)
				_long_note_positions[6].push_back({ position, a.e.extra, ln_on });
			else if (op.auto_play)
				_note_positions[6].push_back(position);
			else if (!a.processed)
				_note_positions[6].push_back(max(0, position));
		}
		else if (a.e.channel == (36 * 1 + 9)) // SW7
		{
			if (a.e.extra != 0)
				_long_note_positions[7].push_back({ position, a.e.extra, ln_on });
			else if (op.auto_play)
				_note_positions[7].push_back(position);
			else if (!a.processed)
				_note_positions[7].push_back(max(0, position));
		}
		else if (a.e.channel == (36 * 13 + 6)) // 地雷皿
			_mine_positions[0].push_back(position);
		else if (a.e.channel == (36 * 13 + 1)) // 地雷SW1
			_mine_positions[1].push_back(position);
		else if (a.e.channel == (36 * 13 + 2)) // 地雷SW2
			_mine_positions[2].push_back(position);
		else if (a.e.channel == (36 * 13 + 3)) // 地雷SW3
			_mine_positions[3].push_back(position);
		else if (a.e.channel == (36 * 13 + 4)) // 地雷SW4
			_mine_positions[4].push_back(position);
		else if (a.e.channel == (36 * 13 + 5)) // 地雷SW5
			_mine_positions[5].push_back(position);
		else if (a.e.channel == (36 * 13 + 8)) // 地雷SW6
			_mine_positions[6].push_back(position);
		else if (a.e.channel == (36 * 13 + 9)) // 地雷SW7
			_mine_positions[7].push_back(position);
	}

	for (auto& a : _long_note_positions)
	{
		for (auto& b : a)
		{
			b.end = ln_ends[b.end];
		}
	}

	_beat_timer->set_custom_time(1 - (beat_forward - now) / (beat_forward - beat_back));

	if (now >= _song_duration)
		timers[AB_TIMER_FADE_OUT].start();
}

void PlayScene::judge(Judge j, double delay)
{
	double gauge_d = 0;
	bool closing = _engine.timers()[AB_TIMER_CLOSE].isEnabled();
	bool fast = delay < 0;

	if (!closing)
	{
		if (j == Judge::Pgreat)
		{
			_pgreat++;
			_current_combo++;
			gauge_d = _groove_great;
		}
		else if (j == Judge::Great)
		{
			_great++;
			_current_combo++;
			gauge_d = _groove_great;
		}
		else if (j == Judge::Good)
		{
			_good++;
			_current_combo++;
			gauge_d = _groove_good;
		}
		else if (j == Judge::Bad)
		{
			_bad++;
			_cbrk++;
			_current_combo = 0;
			gauge_d = _groove_bad;
			_combo_points.push_back(0);
		}
		else if (j == Judge::Poor)
		{
			_poor++;
			_cbrk++;
			_current_combo = 0;
			gauge_d = _groove_poor;
			_combo_points.push_back(0);
		}
		else if (j == Judge::PoorBlank)
		{
			_poor++;
			gauge_d = _groove_poor_blank;
		}
		else
			return;
	}

	_engine.flags()[L"judge_pgreat"] = j == Judge::Pgreat;
	_engine.flags()[L"judge_great"] = j == Judge::Great;
	_engine.flags()[L"judge_good"] = j == Judge::Good;
	_engine.flags()[L"judge_bad"] = j == Judge::Bad;
	_engine.flags()[L"judge_poor"] = j == Judge::Poor;
	_engine.flags()[L"judge_poor_blank"] = j == Judge::PoorBlank;

	_engine.numbers()[L"judge_delay"] = (j == Judge::Poor || j == Judge::PoorBlank) ? 0 : util::round(delay * 1000);

	_engine.timers()[L"judge"].restart();
	_engine.timers()[L"fast"].stop();
	_engine.timers()[L"slow"].stop();
	if (j != Judge::Pgreat)
	{
		if (fast)
		{
			_fast++;
			_engine.timers()[L"fast"].start();
		}
		else
		{
			_slow++;
			_engine.timers()[L"slow"].start();
		}
	}

	_max_combo = max(_max_combo, _current_combo);

	if (!closing)
	{
		auto gauge_type = _window->playOption().gauge_type;
		if (gauge_type == PlayOption::GaugeType::AssistedEasy || gauge_type == PlayOption::GaugeType::Easy || gauge_type == PlayOption::GaugeType::Normal)
			_groove_gauge = util::clip(_groove_gauge + gauge_d, 2., 100.);
		else
			_groove_gauge = util::clip(_groove_gauge + gauge_d, 0., 100.);
	}

	if (!closing && _groove_gauge < 2)
	{
		_window->systemSoundManager().sound(SystemSoundManager::SoundType::PlayStop)->play();
		_engine.timers()[AB_TIMER_CLOSE].start();
	}

	_combo_points.back() = _current_combo;
}

void PlayScene::refresh_score()
{
	static long long pgreat = 0;
	static long long great = 0;
	static long long good = 0;
	if (pgreat != _pgreat || great != _great || good != _good)
	{
		pgreat = _pgreat;
		great = _great;
		good = _good;
		auto n = _data.totalnotes();

		auto score = (150000 * pgreat / n) + ((100000 * great + 20000 * good) / n);
		for (auto a : _combo_points)
			score += (2500 * (a * a - (a - 10) * std::abs(a - 11) + 19 * a - 110)) / (2 * n - 11);
		_va_score.set((int)score);
	}
}

void PlayScene::set_hi_speed(int value)
{
	auto a = util::clip(value, _window->config().hiSpeedMin(), _window->config().hiSpeedMax());
	_va_hi_speed.set(a);
	_window->playOption().hi_speed = a;
}

void PlayScene::switch_to_result()
{
	ResultScene::ResultParameters params;

	params.difficulty = _data.difficulty();
	params.level = _data.playLevel();
	params.judge_rank = _data.judgeRank();
	params.title = util::trim(_data.title() + L" " + _data.subtitle());
	params.total_notes = _data.totalnotes();

	if (_engine.timers()[AB_TIMER_CLOSE].isEnabled()
		|| _window->playOption().gauge_type == PlayOption::GaugeType::Normal && _groove_gauge < 80
		|| _window->playOption().gauge_type == PlayOption::GaugeType::Easy && _groove_gauge < 80
		|| _window->playOption().gauge_type == PlayOption::GaugeType::AssistedEasy && _groove_gauge < 60)
		params.cleared = false;
	else
		params.cleared = true;
	auto ex_score = 2 * _pgreat + _great;
	if (ex_score >= _aaa_score)
		params.dj_level = 1;
	else if (ex_score >= _aa_score)
		params.dj_level = 2;
	else if (ex_score >= _a_score)
		params.dj_level = 3;
	else if (ex_score >= _b_score)
		params.dj_level = 4;
	else if (ex_score >= _c_score)
		params.dj_level = 5;
	else if (ex_score >= _d_score)
		params.dj_level = 6;
	else if (ex_score >= _e_score)
		params.dj_level = 7;
	else
		params.dj_level = 8;
	params.pgreat = _pgreat;
	params.great = _great;
	params.good = _good;
	params.bad = _bad;
	params.poor = _poor;
	params.cbrk = _cbrk;
	params.fast = _fast;
	params.slow = _slow;
	params.max_combo = _max_combo;
	params.achievement = (2 * _pgreat + _great) / (double)_max;

	auto& skin = _window->skinManager().selectedSkin(Skin::Result);
	_window->switchScene(std::shared_ptr<SceneBase>(new ResultScene(_window, skin, params)));
}

void PlayScene::draw()
{
	// BGA合成
	auto handle_before = dx::GetDrawScreen();
	auto w = _bga_composite->width(), h = _bga_composite->height();

	dx::SetDrawScreen(_bga_composite->handle());
	dx::FillGraph(_bga_composite->handle(), 0, 0, 0);
	dx::SetDrawMode(DX_DRAWMODE_BILINEAR);
	dx::SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);
	dx::SetDrawBright(255, 255, 255);

	std::array<std::shared_ptr<Image>, 2> images = { _bga_base, _bga_layer };
	for (auto a : images)
	{
		if (a && !a->fail())
		{
			int x1, y1, x2, y2;
			if (w <= a->width())
				x1 = 0, x2 = a->width();
			else
				x1 = (w - a->width()) / 2, x2 = (w - a->width()) / 2 + a->width();
			y1 = 0, y2 = min(h, a->height());
			dx::DrawExtendGraph(x1, y1, x2, y2, a->handle(), true);
		}
	}
	dx::SetDrawScreen(handle_before);

	// スキン描画
	_engine.draw();
}

#pragma region BgaElement

void PlayScene::BgaElement::draw()
{
	_preparedImage = SkinEngine::PreparedImage::make(_scene._bga_composite);
	if (!_preparedImage || _preparedImage->fail()) return;

	if (!is_valid(engine, _conditions)) return;
	set_parameters();

	auto& img = *_preparedImage->images()[i];
	dx::DrawRotaGraph3(x, y, ax, ay, w / img.width(), h / img.height(), angle * DX_PI / 180, img.handle(), true);
}

#pragma endregion

#pragma region BarLineElement

void PlayScene::BarLineElement::draw()
{
	if (!is_valid(engine, _conditions)) return;
	set_parameters();

	auto& img = *_preparedImage->images()[i];
	for (auto a : _scene._bar_line_positions)
	{
		if (a < 0) continue;
		if (y + img.height() - a < 0) break;
		dx::DrawRotaGraph3(x, util::round(y - a), ax, ay, w / img.width(), h / img.height(), angle * DX_PI / 180, img.handle(), true);
	}
}

#pragma endregion

#pragma region NoteElement

PlayScene::NoteElement::NoteElement(PlayScene& scene, const Skin::Element& e) : ImageElement(scene._engine, e, false), _scene(scene)
{
	if (fail()) return;

	_note_index = util::round(util::get_any<std::wstring, double>(e.src_params, L"index", 0));
	if (_note_index < 0 || AB_LANE_COUNT <= _note_index)
		_fail = true;
}

void PlayScene::NoteElement::draw()
{
	if (!is_valid(engine, _conditions)) return;

	int d = 0;

	// ノート
	_preparedImage = _scene._note_images[_note_index];
	if (_preparedImage && !_preparedImage->fail())
	{
		set_parameters();
		auto& img = *_preparedImage->images()[i];
		d = img.height() / 2;
		for (auto a : _scene._note_positions[_note_index])
		{
			if (a < 0) continue;
			if (y + img.height() - a < 0) break;
			dx::DrawRotaGraph3(x, util::round(y - a), ax, ay, w / img.width(), h / img.height(), angle * DX_PI / 180, img.handle(), true);
		}
	}

	// 地雷
	_preparedImage = _scene._mine_images[_note_index];
	if (_preparedImage && !_preparedImage->fail())
	{
		set_parameters();
		auto& img = *_preparedImage->images()[i];
		for (auto a : _scene._mine_positions[_note_index])
		{
			if (a < 0) continue;
			if (y + img.height() - a < 0) break;
			dx::DrawRotaGraph3(x, util::round(y - a), ax, ay, w / img.width(), h / img.height(), angle * DX_PI / 180, img.handle(), true);
		}
	}

	// LN本体(OFF)
	_preparedImage = _scene._long_note_body_off_images[_note_index];
	if (_preparedImage && !_preparedImage->fail())
	{
		set_parameters();
		auto& img = *_preparedImage->images()[i];
		for (auto a : _scene._long_note_positions[_note_index])
		{
			if (a.end < 0) continue;
			if (a.on) continue;
			dx::DrawRotaGraph3(x, util::round(y - a.end + d), ax, ay, w / img.width(), (a.end - max(0, a.begin)) / img.height(), angle * DX_PI / 180, img.handle(), true);
		}
	}

	// LN本体(ON)
	_preparedImage = _scene._long_note_body_on_images[_note_index];
	if (_preparedImage && !_preparedImage->fail())
	{
		set_parameters();
		auto& img = *_preparedImage->images()[i];
		for (auto a : _scene._long_note_positions[_note_index])
		{
			if (a.end < 0) continue;
			if (!a.on) continue;
			dx::DrawRotaGraph3(x, util::round(y - a.end + d), ax, ay, w / img.width(), (a.end - max(0, a.begin)) / img.height(), angle * DX_PI / 180, img.handle(), true);
		}
	}

	// LN開始点
	_preparedImage = _scene._long_note_begin_images[_note_index];
	if (_preparedImage && !_preparedImage->fail())
	{
		set_parameters();
		auto& img = *_preparedImage->images()[i];
		for (auto a : _scene._long_note_positions[_note_index])
		{
			if (a.begin < 0 && a.end < 0) continue;
			auto begin = max(0, a.begin);
			dx::DrawRotaGraph3(x, util::round(y - begin), ax, ay, w / img.width(), h / img.height(), angle * DX_PI / 180, img.handle(), true);
		}
	}

	// LN終了点
	_preparedImage = _scene._long_note_end_images[_note_index];
	if (_preparedImage && !_preparedImage->fail())
	{
		set_parameters();
		auto& img = *_preparedImage->images()[i];
		for (auto a : _scene._long_note_positions[_note_index])
		{
			if (a.end < 0) continue;
			if (y + img.height() - a.end < 0) break;
			dx::DrawRotaGraph3(x, util::round(y - a.end), ax, ay, w / img.width(), h / img.height(), angle * DX_PI / 180, img.handle(), true);
		}
	}

}

#pragma endregion

#pragma region GrooveGaugeElement

PlayScene::GrooveGaugeElement::GrooveGaugeElement(PlayScene& scene, const Skin::Element& e) : ImageElement(scene._engine, e, false), _scene(scene)
{
	if (fail()) return;

	_dx = util::round(util::get_any<std::wstring, double>(e.src_params, L"dx", 0));
	_dy = util::round(util::get_any<std::wstring, double>(e.src_params, L"dy", 0));
	if (util::get_any<std::wstring, std::wstring>(e.src_params, L"animate", L"true") == L"false")
		_animate = false;

	_rnd = std::mt19937(std::random_device()());
}

void PlayScene::GrooveGaugeElement::draw()
{
	if (!is_valid(engine, _conditions)) return;

	auto amount = (int)std::floor(_scene._groove_gauge / 2);
	auto gauge = _scene._window->playOption().gauge_type;
	int animate_num = 0;
	if (_animate)
		animate_num = std::uniform_int_distribution<int>(0, 2)(_rnd);
	for (int j = 0; j < 50; j++)
	{
		std::wstring suffix = j < amount ? L"_on" : L"_off";
		if (j == amount - 2 && animate_num < 2)
			suffix = L"_off";
		else if (j == amount - 3 && animate_num < 1)
			suffix = L"_off";

		if (gauge == PlayOption::GaugeType::AssistedEasy)
		{
			if (j < 29)
				_preparedImage = _scene._groovegauge_images[L"normal" + suffix];
			else
				_preparedImage = _scene._groovegauge_images[L"hard" + suffix];
		}
		else if (gauge == PlayOption::GaugeType::Easy || gauge == PlayOption::GaugeType::Normal)
		{
			if (j < 39)
				_preparedImage = _scene._groovegauge_images[L"normal" + suffix];
			else
				_preparedImage = _scene._groovegauge_images[L"hard" + suffix];
		}
		else if (gauge == PlayOption::GaugeType::Hard)
		{
			_preparedImage = _scene._groovegauge_images[L"hard" + suffix];
		}
		else
		{
			_preparedImage = _scene._groovegauge_images[L"exhard" + suffix];
		}

		if (_preparedImage && !_preparedImage->fail())
		{
			set_parameters();
			auto& img = *_preparedImage->images()[i];
			dx::DrawRotaGraph3(x + _dx * j, y, ax, ay, w / img.width(), h / img.height(), angle * DX_PI / 180, img.handle(), true);
		}
	}
}

#pragma endregion

#pragma region JudgeElement

PlayScene::JudgeElement::JudgeElement(PlayScene& scene, const Skin::Element& e) : ImageElement(scene._engine, e)
{
	if (fail()) return;

	_dx = util::get_any<std::wstring, double>(e.src_params, L"dx", 0);
	_dy = util::get_any<std::wstring, double>(e.src_params, L"dy", 0);
	_number_id = util::get_any<std::wstring, std::wstring>(e.src_params, L"id", L"");
}

void PlayScene::JudgeElement::draw()
{
	if (!is_valid(engine, _conditions)) return;
	set_parameters();

	auto n = util::digit_number(engine.numbers()[_number_id]);
	x += util::round(_dx * n);
	y += util::round(_dy * n);

	auto& img = *_preparedImage->images()[i];
	dx::DrawRotaGraph3(x, y, ax, ay, w / img.width(), h / img.height(), angle * DX_PI / 180, img.handle(), true);
}

#pragma endregion
