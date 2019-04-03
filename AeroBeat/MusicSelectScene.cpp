#include "stdafx.h"
#include "Window.h"
#include "DecideScene.h"
#include "KeyConfigScene.h"
#include "MusicSelectScene.h"

#define AB_TIMER_SCENE L"scene"
#define AB_TIMER_START_INPUT L"start_input"
#define AB_TIMER_FADE_OUT L"fade_out"
#define AB_TIMER_LOADING L"loading"
#define AB_TIMER_CURSOR_MOVE L"cursor_move"
#define AB_TIMER_INTERNAL_SONG_UP L"\"song_up"
#define AB_TIMER_INTERNAL_SONG_DOWN L"\"song_down"

MusicSelectScene::MusicSelectScene(Window* window, Skin& skin) : SceneBase(window, skin.resolutionWidth(), skin.resolutionHeight()), _skin(skin)
{
	_current_node.push({ window->songManager().root(window->config()), 0 });

	_engine.interpret(skin, [this](const Skin::Element& e) -> std::shared_ptr<SkinEngine::Element>
	{
		if (e.type == L"songbar")
		{
			if (e.src_params.size() > 0 && e.dst_params.size() == 0)
			{
				// src
				auto type = util::get_any<std::wstring, std::wstring>(e.src_params, L"type", L"");
				auto img = SkinEngine::PreparedImage::make(_engine, e);
				if (!img->fail())
					_songbar_images[type] = img;
			}
			else if (e.src_params.size() == 0)
			{
				// dst
				auto ptr = std::shared_ptr<SongBarElement>(new SongBarElement(*this, e));
				if (!ptr->fail())
				{
					_songbar_elements.push_back(ptr);
					return ptr;
				}
			}
		}
		if (e.type == L"songbar_center")
		{
			auto oi = util::get_any<std::wstring, double>(e.src_params, L"index");
			if (oi)
			{
				auto index = util::round(*oi);
				if (index >= 0)
					_songbar_center = index;
			}
		}
		if (e.type == L"songbar_text")
		{
			std::vector<std::shared_ptr<SkinEngine::Element>> v;
			for (auto& a : _songbar_elements)
			{
				auto ptr = std::shared_ptr<SongBarTextElement>(new SongBarTextElement(*this, e, a));
				if (!ptr->fail())
					v.push_back(ptr);
			}
			return std::shared_ptr<SkinEngine::AggregateElement>(new SkinEngine::AggregateElement(_engine, v));
		}
		if (e.type == L"songbar_lamp")
		{
			if (e.src_params.size() > 0 && e.dst_params.size() == 0)
			{
				// src
				auto type = util::get_any<std::wstring, std::wstring>(e.src_params, L"type", L"");
				auto img = SkinEngine::PreparedImage::make(_engine, e);
				if (!img->fail())
					_songbar_lamp_images[type] = img;
			}
			else if (e.src_params.size() == 0)
			{
				// dst
				auto ptr = std::shared_ptr<SongBarLampElement>(new SongBarLampElement(*this, e));
				if (!ptr->fail())
					return ptr;
			}
		}
		if (e.type == L"songbar_level")
		{
			if (e.src_params.size() > 0 && e.dst_params.size() == 0)
			{
				// src
				auto difficulty = util::get_any<std::wstring, std::wstring>(e.src_params, L"difficulty", L"");
				auto digit = util::round(util::get_any<std::wstring, double>(e.src_params, L"digit", 0.0));
				auto type = util::round(util::get_any<std::wstring, double>(e.src_params, L"type", 0.0));
				auto center = util::get_any<std::wstring, std::wstring>(e.src_params, L"center", L"false") == L"true";
				auto img = SkinEngine::PreparedImage::make(_engine, e);

				if (digit > 0 && type > 0 && (type == 10 || type == 11 || type == 24) && img->image_count() % type == 0)
				{
					if (!img->fail())
						_songbar_level_sources[difficulty] = { img, digit, type, center };
				}
			}
			else if (e.src_params.size() == 0)
			{
				// dst
				auto ptr = std::shared_ptr<SongBarLevelElement>(new SongBarLevelElement(*this, e));
				if (!ptr->fail())
					return ptr;
			}
		}
		return _engine.default_element(e);
	});

	// set timers
	_engine.timers()[AB_TIMER_SCENE].start();

	// clear mouse input
	util::clear_mouse_input();
	dx::GetMouseWheelRotVol();

	// play bgm
	_window->systemSoundManager().sound(SystemSoundManager::SoundType::Clear)->stop();
	_window->systemSoundManager().sound(SystemSoundManager::SoundType::Fail)->stop();
	_window->systemSoundManager().sound(SystemSoundManager::SoundType::SelectBgm)->play();
}

bool MusicSelectScene::update()
{
	int x, y;
	_window->cursorLocalPosition(_skin.resolutionWidth(), _skin.resolutionHeight(), x, y);
	_engine.set_cursor_x(x);
	_engine.set_cursor_y(y);

	// songbars
	{
		std::lock_guard<std::mutex> lock(_mutex);

		auto& current_context = _current_node.top();
		int i = 0, count = current_context.node->children().size();
		for (auto& a : _songbar_elements)
		{
			int index = ((i + current_context.index - _songbar_center) % count + count) % count;
			a->set_node(current_context.node->children()[index]);
			i++;
		}
	}

	auto& timers = _engine.timers();

	if (timers[AB_TIMER_SCENE]() < _skin.startInput())
	{
		return true;
	}
	else if (!timers[AB_TIMER_START_INPUT].isEnabled())
	{
		timers[AB_TIMER_START_INPUT].start();
		timers[AB_TIMER_CURSOR_MOVE].start();
		_refresh_state();
		return true;
	}
	
	if (timers[AB_TIMER_LOADING].isEnabled())
		return true;

	if (timers[AB_TIMER_FADE_OUT].isEnabled())
	{
		if (timers[AB_TIMER_FADE_OUT]() > _skin.fadeOut())
		{
			_window->switchScene(_next_scene_func());
			return true;
		}
		return true;
	}

	auto mouse_input = util::get_mouse_input();
	if (mouse_input & MOUSE_INPUT_LEFT)
	{
		for (auto& a : _engine.hit())
		{
			if (a == L"key_config")
			{
				// キーコンフィグへ遷移
				_exit_scene([this]()
				{
					auto& skin = _window->skinManager().selectedSkin(Skin::KeyConfig);
					return std::shared_ptr<SceneBase>(new KeyConfigScene(_window, skin));
				});
				_window->systemSoundManager().sound(SystemSoundManager::SoundType::SelectBgm)->stop();
				_stop_preview();
				_window->systemSoundManager().sound(SystemSoundManager::SoundType::OptionChange)->play();
			}
			else if (a == L"option_style_prev")
			{
				_option_style = (5 + _option_style - 1) % 5;
				_window->systemSoundManager().sound(SystemSoundManager::SoundType::OptionChange)->play();
				_refresh_state();
			}
			else if (a == L"option_style_next")
			{
				_option_style = (5 + _option_style + 1) % 5;
				_window->systemSoundManager().sound(SystemSoundManager::SoundType::OptionChange)->play();
				_refresh_state();
			}
			else if (a == L"option_style_regular")
			{
				_option_style = 0;
				_window->systemSoundManager().sound(SystemSoundManager::SoundType::OptionChange)->play();
				_refresh_state();
			}
			else if (a == L"option_style_mirror")
			{
				_option_style = 1;
				_window->systemSoundManager().sound(SystemSoundManager::SoundType::OptionChange)->play();
				_refresh_state();
			}
			else if (a == L"option_style_random")
			{
				_option_style = 2;
				_window->systemSoundManager().sound(SystemSoundManager::SoundType::OptionChange)->play();
				_refresh_state();
			}
			else if (a == L"option_style_srandom")
			{
				_option_style = 3;
				_window->systemSoundManager().sound(SystemSoundManager::SoundType::OptionChange)->play();
				_refresh_state();
			}
			else if (a == L"option_style_rrandom")
			{
				_option_style = 4;
				_window->systemSoundManager().sound(SystemSoundManager::SoundType::OptionChange)->play();
				_refresh_state();
			}
			else if (a == L"option_gauge_prev")
			{
				_option_gauge = (6 + _option_gauge - 1) % 6;
				_window->systemSoundManager().sound(SystemSoundManager::SoundType::OptionChange)->play();
				_refresh_state();
			}
			else if (a == L"option_gauge_next")
			{
				_option_gauge = (6 + _option_gauge + 1) % 6;
				_window->systemSoundManager().sound(SystemSoundManager::SoundType::OptionChange)->play();
				_refresh_state();
			}
			else if (a == L"option_gauge_normal")
			{
				_option_gauge = 0;
				_window->systemSoundManager().sound(SystemSoundManager::SoundType::OptionChange)->play();
				_refresh_state();
			}
			else if (a == L"option_gauge_easy")
			{
				_option_gauge = 1;
				_window->systemSoundManager().sound(SystemSoundManager::SoundType::OptionChange)->play();
				_refresh_state();
			}
			else if (a == L"option_gauge_assisted_easy")
			{
				_option_gauge = 2;
				_window->systemSoundManager().sound(SystemSoundManager::SoundType::OptionChange)->play();
				_refresh_state();
			}
			else if (a == L"option_gauge_hard")
			{
				_option_gauge = 3;
				_window->systemSoundManager().sound(SystemSoundManager::SoundType::OptionChange)->play();
				_refresh_state();
			}
			else if (a == L"option_gauge_ex_hard")
			{
				_option_gauge = 4;
				_window->systemSoundManager().sound(SystemSoundManager::SoundType::OptionChange)->play();
				_refresh_state();
			}
			else if (a == L"option_gauge_hazard")
			{
				_option_gauge = 5;
				_window->systemSoundManager().sound(SystemSoundManager::SoundType::OptionChange)->play();
				_refresh_state();
			}
			else if (a == L"system_display_timing_adjustment_prev")
			{
				auto& a = _window->playOption().display_timing_adjustment;
				a--;
				if (a <= -100)
					a = 99;
				_window->systemSoundManager().sound(SystemSoundManager::SoundType::OptionChange)->play();
				_refresh_state();
			}
			else if (a == L"system_display_timing_adjustment_next")
			{
				auto& a = _window->playOption().display_timing_adjustment;
				a++;
				if (a >= 100)
					a = -99;
				_window->systemSoundManager().sound(SystemSoundManager::SoundType::OptionChange)->play();
				_refresh_state();
			}
			else if (util::starts_with(a, L"panel"))
			{
				int i;
				if (util::try_stod_round(a.substr(5), i) && i >= 0)
				{
					if (_active_panel == i)
						_set_active_panel(-1);
					else
						_set_active_panel(i);
				}
			}
		}
	}

	// キー操作
	auto& current_context = _current_node.top();
	auto& ks = _window->keyStates();
	auto& cs = _window->controllerStates();
	auto wheel = dx::GetMouseWheelRotVol();

	// パネル制御
	if (cs[AB_KEY_7KEYS_1P_START] == 1 || cs[AB_KEY_7KEYS_2P_START] == 1)
	{
		if (_active_panel != 0)
		{
			_set_active_panel(0);
			timers[AB_TIMER_INTERNAL_SONG_UP].stop();
			timers[AB_TIMER_INTERNAL_SONG_DOWN].stop();
		}
		else
			_set_active_panel(-1);
	}

	if (_active_panel == 0)
	{
		// プレイオプションを開いているとき
		if (cs[AB_KEY_7KEYS_1P_2] == 1 || cs[AB_KEY_7KEYS_2P_2] == 1)
		{
			_option_style = (5 + _option_style + 1) % 5;
			_window->systemSoundManager().sound(SystemSoundManager::SoundType::OptionChange)->play();
			_refresh_state();
		}
		if (cs[AB_KEY_7KEYS_1P_4] == 1 || cs[AB_KEY_7KEYS_2P_4] == 1)
		{
			_option_gauge = (6 + _option_gauge + 1) % 6;
			_window->systemSoundManager().sound(SystemSoundManager::SoundType::OptionChange)->play();
			_refresh_state();
		}
	}
	else
	{
		// プレイオプションを閉じているとき
		if (wheel != 0)
		{
			_move_cursor(-wheel);
		}
		else if ((ks[KEY_INPUT_UP] > 0 || cs[AB_KEY_7KEYS_1P_SL] > 0) && !timers[AB_TIMER_INTERNAL_SONG_UP].isEnabled())
		{
			_move_cursor(-1);
			timers[AB_TIMER_INTERNAL_SONG_UP].start();
		}
		else if ((ks[KEY_INPUT_UP] < 0 && cs[AB_KEY_7KEYS_1P_SL] < 0) && timers[AB_TIMER_INTERNAL_SONG_UP].isEnabled())
		{
			timers[AB_TIMER_INTERNAL_SONG_UP].stop();
		}
		else if ((ks[KEY_INPUT_DOWN] > 0 || cs[AB_KEY_7KEYS_1P_SR] > 0) && !timers[AB_TIMER_INTERNAL_SONG_DOWN].isEnabled())
		{
			_move_cursor(1);
			timers[AB_TIMER_INTERNAL_SONG_DOWN].start();
		}
		else if ((ks[KEY_INPUT_DOWN] < 0 && cs[AB_KEY_7KEYS_1P_SR] < 0) && timers[AB_TIMER_INTERNAL_SONG_DOWN].isEnabled())
		{
			timers[AB_TIMER_INTERNAL_SONG_DOWN].stop();
		}
		else if (ks[KEY_INPUT_RETURN] == 1
			|| cs[AB_KEY_7KEYS_1P_1] == 1
			|| cs[AB_KEY_7KEYS_1P_3] == 1
			|| cs[AB_KEY_7KEYS_1P_5] == 1
			|| cs[AB_KEY_7KEYS_1P_7] == 1
			|| cs[AB_KEY_7KEYS_2P_1] == 1
			|| cs[AB_KEY_7KEYS_2P_3] == 1
			|| cs[AB_KEY_7KEYS_2P_5] == 1
			|| cs[AB_KEY_7KEYS_2P_7] == 1)
		{
			auto selected = current_context.selected();
			if (selected->type() == SongManager::Node::NodeType::Song)
			{
				// 決定シーンへ
				_window->playOption().auto_play = false;
				_window->systemSoundManager().sound(SystemSoundManager::SoundType::SelectBgm)->stop();
				if (_preview_sound && _preview_sound->is_playing())
					_preview_sound->stop();
				_preview_sound = nullptr;
				auto& skin = _window->skinManager().selectedSkin(Skin::Decide);
				_window->switchScene(std::shared_ptr<SceneBase>(new DecideScene(_window, skin, selected)));
				return true;
			}
			else
			{
				// フォルダに入る(サブノードの取得を別スレッドで行う)
				std::thread([&, this, selected]
				{
					_engine.timers()[AB_TIMER_LOADING].start();
					if (selected->children().size() > 0)
					{
						std::lock_guard<std::mutex> lock(_mutex);
						_current_node.push({ selected, 0 });
					}
					_window->systemSoundManager().sound(SystemSoundManager::SoundType::FolderOpen)->play();
					_engine.timers()[AB_TIMER_LOADING].stop();
					_engine.timers()[AB_TIMER_CURSOR_MOVE].restart();
					_refresh_state();
				}).detach();
			}
		}
		else if (cs[AB_KEY_7KEYS_1P_6] == 1 || cs[AB_KEY_7KEYS_2P_6] == 1)
		{
			auto selected = current_context.selected();
			if (selected->type() == SongManager::Node::NodeType::Song)
			{
				// 決定シーンへ(オートプレイ)
				_window->playOption().auto_play = true;
				_window->systemSoundManager().sound(SystemSoundManager::SoundType::SelectBgm)->stop();
				if (_preview_sound && _preview_sound->is_playing())
					_preview_sound->stop();
				_preview_sound = nullptr;
				auto& skin = _window->skinManager().selectedSkin(Skin::Decide);
				_window->switchScene(std::shared_ptr<SceneBase>(new DecideScene(_window, skin, selected)));
				return true;
			}
		}
		else if (ks[KEY_INPUT_ESCAPE] == 1 || cs[AB_KEY_7KEYS_1P_2] == 1 || cs[AB_KEY_7KEYS_2P_2] == 1)
		{
			if (_current_node.size() == 1)
				return ks[KEY_INPUT_ESCAPE] != 1;
			_current_node.pop();
			_window->systemSoundManager().sound(SystemSoundManager::SoundType::FolderClose)->play();
			_engine.timers()[AB_TIMER_CURSOR_MOVE].restart();
			_refresh_state();
		}
	}

	// キーリピート
	{
		auto& up = timers[AB_TIMER_INTERNAL_SONG_UP];
		auto up_before = up.before(), up_current = up();
		auto& down = timers[AB_TIMER_INTERNAL_SONG_DOWN];
		auto down_before = down.before(), down_current = down();
		constexpr auto threshold = 0.3;
		constexpr auto speed = 25;
		if (up.isEnabled() && up_current >= threshold && ((long long)(up_before * speed) % 10) != ((long long)(up_current * speed) % 10))
			_move_cursor(-1);
		else if (down.isEnabled() && down_current >= threshold && ((long long)(down_before * speed) % 10) != ((long long)(down_current * speed) % 10))
			_move_cursor(1);
	}

	// 楽曲プレビュー
	{
		constexpr auto threshold = 0.5;
		auto before = timers[AB_TIMER_CURSOR_MOVE].before(), now = timers[AB_TIMER_CURSOR_MOVE]();
		if (before == 0)
		{
			// プレビュー停止
			_stop_preview();
		}
		else if (before < threshold && now >= threshold)
		{
			auto selected = _current_node.top().selected();
			if (selected->type() == SongManager::Node::NodeType::Song)
			{
				auto folder = selected->song().folder;
				std::thread([&, this, folder]
				{
					auto preview_path = folder / fs::path(L"preview");
					auto ptr = _window->soundManager().load(preview_path);
					if (ptr->fail()) return;
					_play_preview(folder, ptr);
				}).detach();
				return true;
			}
		}
	}

	return true;
}

void MusicSelectScene::draw()
{
	std::lock_guard<std::mutex> lock(_mutex);

	_engine.draw();
}

void MusicSelectScene::_exit_scene(decltype(_next_scene_func) next)
{
	_next_scene_func = next;
	_engine.timers()[AB_TIMER_FADE_OUT].start();
}

void MusicSelectScene::_play_preview(std::wstring folder, std::shared_ptr<SoundManager::Sound> sound)
{
	if (_preview_sound && _preview_sound->is_playing())
	{
		_preview_sound->stop();
	}
	_preview_sound = nullptr;

	std::wstring node_folder;
	{
		std::lock_guard<std::mutex> lock(_mutex);
		auto& node = _current_node.top().selected();
		if (node->type() != SongManager::Node::NodeType::Song) return;
		node_folder = node->song().folder;
	}
	if (folder == node_folder)
	{
		_window->systemSoundManager().sound(SystemSoundManager::SoundType::SelectBgm)->volume(0.0);
		_preview_sound = sound;
		sound->play();
		std::thread([sound, window = _window]
		{
			while (sound->is_playing())
			{
				std::this_thread::sleep_for(std::chrono::milliseconds(1));
			}
			sound->stop();
			window->systemSoundManager().sound(SystemSoundManager::SoundType::SelectBgm)->volume(1.0);
		}).detach();
	}
}

void MusicSelectScene::_stop_preview()
{
	if (_preview_sound && _preview_sound->is_playing())
	{
		_preview_sound->stop();
	}
	_preview_sound = nullptr;
	_window->systemSoundManager().sound(SystemSoundManager::SoundType::SelectBgm)->volume(1.0);
}

void MusicSelectScene::_move_cursor(int delta)
{
	if (delta == 0) return;
	auto& current_context = _current_node.top();
	int count = current_context.node->children().size();
	current_context.index = (current_context.index + delta + count) % count;
	_window->systemSoundManager().sound(SystemSoundManager::SoundType::Scratch)->play();
	_engine.timers()[AB_TIMER_CURSOR_MOVE].restart();
	_refresh_state();
}

void MusicSelectScene::_refresh_state()
{
	using NodeType = SongManager::Node::NodeType;

	std::lock_guard<std::mutex> lock(_mutex);
	auto& node = _current_node.top().selected();
	auto type = node->type();

	auto& flags = _engine.flags();
	auto& texts = _engine.texts();
	auto& numbers = _engine.numbers();

	flags[L"song"] = type == NodeType::Song;
	flags[L"song_folder"] = type == NodeType::SongFolder;
	flags[L"custom_folder"] = type == NodeType::CustomFolder;
	flags[L"folder"] = type == NodeType::Folder;

	flags[L"option_style_regular"] = _option_style == 0;
	flags[L"option_style_mirror"] = _option_style == 1;
	flags[L"option_style_random"] = _option_style == 2;
	flags[L"option_style_srandom"] = _option_style == 3;
	flags[L"option_style_rrandom"] = _option_style == 4;
	
	flags[L"option_gauge_normal"] = _option_gauge == 0;
	flags[L"option_gauge_easy"] = _option_gauge == 1;
	flags[L"option_gauge_assisted_easy"] = _option_gauge == 2;
	flags[L"option_gauge_hard"] = _option_gauge == 3;
	flags[L"option_gauge_ex_hard"] = _option_gauge == 4;
	flags[L"option_gauge_hazard"] = _option_gauge == 5;

	numbers[L"system_display_timing_adjustment"] = _window->playOption().display_timing_adjustment;

	_window->playOption().style = (PlayOption::Style)_option_style;
	_window->playOption().gauge_type = (PlayOption::GaugeType)_option_gauge;

	if (type == NodeType::Song)
	{
		auto& song = node->song();

		flags[L"softlanding"] = song.minBpm != song.maxBpm;

		texts[L"title"] = song.title;
		texts[L"subtitle"] = song.subtitle;
		texts[L"artist"] = song.artist;
		texts[L"subartist"] = song.subartist;
		texts[L"genre"] = song.genre;

		numbers[L"minbpm"] = (long long)std::round(song.minBpm);
		numbers[L"maxbpm"] = (long long)std::round(song.maxBpm);
	}
	else
	{
		flags[L"softlanding"] = false;

		texts[L"title"] = node->title();
		texts[L"subtitle"] = L"";
		texts[L"artist"] = L"";
		texts[L"subartist"] = L"";
		texts[L"genre"] = L"";

		numbers[L"minbpm"] = 0;
		numbers[L"maxbpm"] = 0;
	}
}

void MusicSelectScene::_set_active_panel(int value)
{
	if (_active_panel == value) return;
	if (_active_panel != -1)
		_engine.set_switch_timers(L"panel" + std::to_wstring(_active_panel), false);
	if (value != -1)
		_engine.set_switch_timers(L"panel" + std::to_wstring(value), true);
	_active_panel = value;
	if (_active_panel == -1)
		_window->systemSoundManager().sound(SystemSoundManager::SoundType::OptionClose)->play();
	else
		_window->systemSoundManager().sound(SystemSoundManager::SoundType::OptionOpen)->play();
}

#pragma region SongBarElement

MusicSelectScene::SongBarElement::SongBarElement(MusicSelectScene& scene, const Skin::Element& e) : SkinEngine::ImageElement(scene._engine, e, false), _scene(scene)
{

}

void MusicSelectScene::SongBarElement::draw()
{
	auto node = *_node;
	switch (node.type())
	{
	case SongManager::Node::NodeType::Song:
		_preparedImage = _scene._songbar_images[L"song"];
		break;
	case SongManager::Node::NodeType::SongFolder:
		_preparedImage = _scene._songbar_images[L"song_folder"];
		break;
	case SongManager::Node::NodeType::CustomFolder:
		_preparedImage = _scene._songbar_images[L"custom_folder"];
		break;
	case SongManager::Node::NodeType::Folder:
		_preparedImage = _scene._songbar_images[L"folder"];
		break;
	}
	if (!_preparedImage || _preparedImage->fail()) return;

	if (!is_valid(engine, _conditions)) return;
	set_parameters();

	auto& img = *_preparedImage->images()[i];
	dx::DrawRotaGraph3(x, y, ax, ay, w / img.width(), h / img.height(), angle * DX_PI / 180, img.handle(), true);
}

#pragma endregion

#pragma region SongBarTextElement

MusicSelectScene::SongBarTextElement::SongBarTextElement(MusicSelectScene& scene, const Skin::Element& e, std::shared_ptr<SongBarElement> parent) : SkinEngine::TextElement(scene._engine, e), _scene(scene)
{
	_parent = parent;
}

boost::optional<std::wstring> MusicSelectScene::SongBarTextElement::target_text()
{
	return _parent->node()->title();
}

void MusicSelectScene::SongBarTextElement::draw()
{
	if (!SkinEngine::ImageElement::is_valid(engine, _conditions)) return;

	if (!set_parameters()) return;

	x += _parent->px();
	y += _parent->py();

	dx::DrawRotaGraph3(x, y, ax, ay, ew / img->width(), h / img->height(), angle * DX_PI / 180, img->handle(), true);
}

#pragma endregion

#pragma region SongBarLampElement

void MusicSelectScene::SongBarLampElement::draw()
{
	if (!is_valid(engine, _conditions)) return;
	
	for (auto& a : _scene._songbar_elements)
	{
		switch (a->node()->clearLamp())
		{
		case 0:
			_preparedImage = _scene._songbar_lamp_images[L"noplay"];
			break;
		case 1:
			_preparedImage = _scene._songbar_lamp_images[L"failed"];
			break;
		case 2:
			_preparedImage = _scene._songbar_lamp_images[L"assist"];
			break;
		case 3:
			_preparedImage = _scene._songbar_lamp_images[L"easy"];
			break;
		case 4:
			_preparedImage = _scene._songbar_lamp_images[L"clear"];
			break;
		case 5:
			_preparedImage = _scene._songbar_lamp_images[L"hard"];
			break;
		case 6:
			_preparedImage = _scene._songbar_lamp_images[L"exhard"];
			break;
		case 7:
			_preparedImage = _scene._songbar_lamp_images[L"fullcombo"];
			break;
		}
		if (!_preparedImage || _preparedImage->fail()) continue;

		set_parameters();

		x += a->px();
		y += a->py();

		auto& img = *_preparedImage->images()[i];
		dx::DrawRotaGraph3(x, y, ax, ay, w / img.width(), h / img.height(), angle * DX_PI / 180, img.handle(), true);
	}
}

#pragma endregion

#pragma region SongBarLevelElement

void MusicSelectScene::SongBarLevelElement::draw()
{
	if (!is_valid(engine, _conditions)) return;

	for (auto& a : _scene._songbar_elements)
	{
		if (a->node()->type() != SongManager::Node::NodeType::Song) continue;
		SourceParameters* params = nullptr;
		auto& song = a->node()->song();
		switch (song.difficulty)
		{
		case 0:
			params = &_scene._songbar_level_sources[L"undefined"];
			break;
		case 1:
			params = &_scene._songbar_level_sources[L"beginner"];
			break;
		case 2:
			params = &_scene._songbar_level_sources[L"normal"];
			break;
		case 3:
			params = &_scene._songbar_level_sources[L"hyper"];
			break;
		case 4:
			params = &_scene._songbar_level_sources[L"another"];
			break;
		case 5:
			params = &_scene._songbar_level_sources[L"insane"];
			break;
		}
		if (params == nullptr || !params->images || params->images->fail()) continue;

		digit = params->digit;
		type = params->type;
		center = params->center;
		_preparedImage = params->images;
		set_number_parameters(song.playLevel);
		x += a->px();
		y += a->py();
		draw_number();
	}
}

#pragma endregion
