#include "stdafx.h"
#include "Window.h"
#include "MusicSelectScene.h"
#include "ResultScene.h"

#define AB_TIMER_SCENE L"scene"
#define AB_TIMER_FADE_OUT L"fade_out"

ResultScene::ResultScene(Window* window, Skin& skin, ResultScene::ResultParameters params) : SceneBase(window, skin.resolutionWidth(), skin.resolutionHeight()), _skin(skin)
{
	_engine.interpret(skin);

	// set parameters
	auto& flags = _engine.flags();
	auto& texts = _engine.texts();
	auto& numbers = _engine.numbers();

	flags[L"undefined"] = params.difficulty == 0;
	flags[L"beginner"] = params.difficulty == 1;
	flags[L"normal"] = params.difficulty == 2;
	flags[L"hyper"] = params.difficulty == 3;
	flags[L"another"] = params.difficulty == 4;
	flags[L"insane"] = params.difficulty == 5;
	numbers[L"level"] = params.level;
	flags[L"judge_rank_very_hard"] = params.judge_rank == 0;
	flags[L"judge_rank_hard"] = params.judge_rank == 1;
	flags[L"judge_rank_normal"] = params.judge_rank == 2;
	flags[L"judge_rank_easy"] = params.judge_rank == 3;
	texts[L"title"] = params.title;
	numbers[L"total_notes"] = params.total_notes;

	flags[L"cleared"] = params.cleared;
	flags[L"dj_level_aaa"] = params.dj_level == 1;
	flags[L"dj_level_aa"] = params.dj_level == 2;
	flags[L"dj_level_a"] = params.dj_level == 3;
	flags[L"dj_level_b"] = params.dj_level == 4;
	flags[L"dj_level_c"] = params.dj_level == 5;
	flags[L"dj_level_d"] = params.dj_level == 6;
	flags[L"dj_level_e"] = params.dj_level == 7;
	flags[L"dj_level_f"] = params.dj_level == 8;
	numbers[L"pgreat"] = params.pgreat;
	numbers[L"great"] = params.great;
	numbers[L"good"] = params.good;
	numbers[L"bad"] = params.bad;
	numbers[L"poor"] = params.poor;
	numbers[L"cbrk"] = params.cbrk;
	numbers[L"fast"] = params.fast;
	numbers[L"slow"] = params.slow;
	numbers[L"max_combo"] = params.max_combo;
	numbers[L"ex_score"] = 2 * params.pgreat + params.great;
	numbers[L"miss_count"] = params.bad + params.poor;
	double ai;
	numbers[L"achievement_fractional"] = (long long)(std::floor(std::modf(params.achievement * 100, &ai) * 100));
	numbers[L"achievement_integral"] = (long long)std::round(ai);

	// set timers
	_engine.timers()[AB_TIMER_SCENE].start();

	// play bgm
	if (params.cleared)
		_window->systemSoundManager().sound(SystemSoundManager::SoundType::Clear)->play();
	else
		_window->systemSoundManager().sound(SystemSoundManager::SoundType::Fail)->play();
}

bool ResultScene::update()
{
	auto& timers = _engine.timers();

	if (timers[AB_TIMER_SCENE]() < _skin.startInput())
		return true;
	if (timers[AB_TIMER_FADE_OUT].isEnabled())
	{
		if (timers[AB_TIMER_FADE_OUT]() > _skin.fadeOut())
		{
			auto& skin = _window->skinManager().selectedSkin(Skin::MusicSelect);
			_window->switchScene(std::shared_ptr<SceneBase>(new MusicSelectScene(_window, skin)));
			return true;
		}
		return true;
	}

	// ƒL[‘€ì
	auto& ks = _window->keyStates();
	auto& cs = _window->controllerStates();
	auto flag = false;
	for (auto& a : ks)
	{
		if (a == 1)
		{
			flag = true;
			break;
		}
	}
	for (auto& a : cs)
	{
		if (a == 1 || flag)
		{
			flag = true;
			break;
		}
	}
	if (flag)
	{
		timers[AB_TIMER_FADE_OUT].start();
		return true;
	}

	return true;
}

void ResultScene::draw()
{
	_engine.draw();
}
