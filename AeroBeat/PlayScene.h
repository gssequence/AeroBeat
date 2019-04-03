#pragma once

#include "BmsData.h"
#include "SceneBase.h"
#include "SkinEngine.h"
#include "ValueAnimator.h"

#define AB_LANE_COUNT 16

class PlayScene : public SceneBase
{
public:
	enum Judge
	{
		None,
		Pgreat,
		Great,
		Good,
		Bad,
		Poor,
		PoorBlank
	};

	struct Event
	{
		BmsData::Event e;
		bool processed;
		Judge ln_judge;
		double ln_delay;
		bool ln_complete;
	};

	struct LongNotePosition
	{
		double begin;
		double end;
		bool on;
	};

	class BgaElement : public SkinEngine::ImageElement
	{
	private:
		PlayScene& _scene;

	public:
		BgaElement(PlayScene& scene, const Skin::Element& e) : ImageElement(scene._engine, e, false), _scene(scene) { }
		virtual ~BgaElement() { }

		virtual void draw() override;
	};

	class BarLineElement : public SkinEngine::ImageElement
	{
	private:
		PlayScene& _scene;

	public:
		BarLineElement(PlayScene& scene, const Skin::Element& e) : ImageElement(scene._engine, e), _scene(scene) { }
		virtual ~BarLineElement() { }

		virtual void draw() override;
	};

	class NoteElement : public SkinEngine::ImageElement
	{
	private:
		PlayScene& _scene;
		int _note_index = -1;

	public:
		NoteElement(PlayScene& scene, const Skin::Element& e);
		virtual ~NoteElement() { }

		virtual void draw() override;
	};

	class GrooveGaugeElement : public SkinEngine::ImageElement
	{
	private:
		PlayScene& _scene;
		bool _animate = true;
		int _dx = 0, _dy = 0;
		std::mt19937 _rnd;

	public:
		GrooveGaugeElement(PlayScene& scene, const Skin::Element& e);
		virtual ~GrooveGaugeElement() { }

		virtual void draw() override;
	};

	class JudgeElement : public SkinEngine::ImageElement
	{
	private:
		std::wstring _number_id = L"";
		double _dx = 0, _dy = 0;

	public:
		JudgeElement(PlayScene& scene, const Skin::Element& e);
		virtual ~JudgeElement() { }

		virtual void draw() override;
	};

private:
	bool _fail = true;
	bool _loading = true;
	double _load_progress = 0;
	Skin& _skin;
	SkinEngine _engine;
	std::shared_ptr<SongManager::Node> _node;
	unsigned int _seed;
	std::mt19937 _random;
	BmsData _data;
	std::unordered_map<int, std::shared_ptr<SoundManager::Sound>> _sounds;
	std::unordered_map<int, std::shared_ptr<Image>> _images;
	std::vector<Event> _events;
	double _song_duration = 0;
	double _pgreat_range, _great_range = 0.060, _good_range = 0.120, _bad_range = 0.200, _poor_blank_range = 1.000; // quoted from h1dia/BMSPlayer on GitHub
	std::array<std::shared_ptr<SkinEngine::PreparedImage>, AB_LANE_COUNT> _note_images, _mine_images;
	std::array<std::shared_ptr<SkinEngine::PreparedImage>, AB_LANE_COUNT> _long_note_begin_images, _long_note_end_images, _long_note_body_on_images, _long_note_body_off_images;
	std::unordered_map<std::wstring, std::shared_ptr<SkinEngine::PreparedImage>> _groovegauge_images;
	std::array<std::shared_ptr<SoundManager::Sound>, AB_LANE_COUNT> _key_sounds;
	Timer* _beat_timer = &(_engine.timers()[L"beat"] = Timer(true));
	std::vector<long long> _combo_points = { 0 };
	ValueAnimator<int> _va_score = ValueAnimator<int>(0, 0.5);
	ValueAnimator<double> _va_hi_speed = ValueAnimator<double>(_window->playOption().hi_speed, 0.4, ease::easeOutCubic);
	int _lane_height = 100;

	long long _max, _aaa_score, _aa_score, _a_score, _b_score, _c_score, _d_score, _e_score;
	double _current_bpm;
	long long _pgreat = 0, _great = 0, _good = 0, _bad = 0, _poor = 0, _cbrk = 0, _fast = 0, _slow = 0;
	long long _max_combo = 0, _current_combo = 0;
	double _groove_gauge, _groove_great, _groove_good, _groove_bad, _groove_poor, _groove_poor_blank;
	double _hi_speed_exp;
	std::shared_ptr<Image> _bga_base, _bga_layer, _bga_composite;
	std::vector<double> _bar_line_positions;
	std::array<std::vector<double>, AB_LANE_COUNT> _note_positions, _mine_positions;
	std::array<std::vector<LongNotePosition>, AB_LANE_COUNT> _long_note_positions;
	std::array<bool, AB_LANE_COUNT> _judged_lanes;

	void play_update();
	void judge(Judge j, double delay = 0);
	void refresh_score();
	void set_hi_speed(int value);
	void switch_to_result();

public:
	PlayScene(Window* window, Skin& skin, std::shared_ptr<SongManager::Node> node);
	virtual ~PlayScene() { }

	virtual bool update() override;
	virtual void draw() override;
};
