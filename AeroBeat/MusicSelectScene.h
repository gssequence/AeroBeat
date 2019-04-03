#pragma once

#include "SceneBase.h"
#include "SkinEngine.h"

class MusicSelectScene : public SceneBase
{
public:
	struct NodeContext
	{
		std::shared_ptr<SongManager::Node> node;
		int index;
		auto selected() { return node->children()[index]; }
	};

	class SongBarElement : public SkinEngine::ImageElement
	{
	private:
		MusicSelectScene& _scene;
		std::shared_ptr<SongManager::Node> _node;
		int _index = 0;

	public:
		SongBarElement(MusicSelectScene& scene, const Skin::Element& e);
		virtual ~SongBarElement() { }

		virtual void draw() override;

		auto node() { return _node; }
		void set_node(std::shared_ptr<SongManager::Node> value) { _node = value; }
		auto px() { return x; }
		auto py() { return y; }
	};

	class SongBarTextElement : public SkinEngine::TextElement
	{
	private:
		MusicSelectScene& _scene;
		std::shared_ptr<SongBarElement> _parent;

	protected:
		virtual boost::optional<std::wstring> target_text() override;

	public:
		SongBarTextElement(MusicSelectScene& scene, const Skin::Element& e, std::shared_ptr<SongBarElement> parent);
		virtual void draw() override;
	};

	class SongBarLampElement : public SkinEngine::ImageElement
	{
	private:
		MusicSelectScene& _scene;

	public:
		SongBarLampElement(MusicSelectScene& scene, const Skin::Element& e) : ImageElement(scene._engine, e, false), _scene(scene) { }
		virtual void draw() override;
	};

	class SongBarLevelElement : public SkinEngine::NumberElement
	{
	public:
		struct SourceParameters
		{
			std::shared_ptr<SkinEngine::PreparedImage> images;
			int digit;
			int type;
			bool center;
		};

	private:
		MusicSelectScene& _scene;

	public:
		SongBarLevelElement(MusicSelectScene& scene, const Skin::Element& e) : NumberElement(scene._engine, e, false), _scene(scene) { }
		virtual void draw() override;
	};

private:
	Skin& _skin;
	SkinEngine _engine;
	std::mutex _mutex;
	std::unordered_map<std::wstring, std::shared_ptr<SkinEngine::PreparedImage>> _songbar_images, _songbar_lamp_images;
	std::unordered_map<std::wstring, SongBarLevelElement::SourceParameters> _songbar_level_sources;
	int _songbar_center = 0;
	std::vector<std::shared_ptr<SongBarElement>> _songbar_elements;
	std::function<std::shared_ptr<SceneBase>()> _next_scene_func;
	std::stack<NodeContext> _current_node;
	std::shared_ptr<SoundManager::Sound> _preview_sound;
	int _active_panel = -1;
	int _option_style = (int)_window->playOption().style;
	int _option_gauge = (int)_window->playOption().gauge_type;

	void _exit_scene(decltype(_next_scene_func) next);
	void _play_preview(std::wstring folder, std::shared_ptr<SoundManager::Sound> sound);
	void _stop_preview();
	void _move_cursor(int delta);
	void _refresh_state();
	void _set_active_panel(int value);

public:
	MusicSelectScene(Window* window, Skin& skin);
	virtual ~MusicSelectScene() { }

	virtual bool update() override;
	virtual void draw() override;
};
