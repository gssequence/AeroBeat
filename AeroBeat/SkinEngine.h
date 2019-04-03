#pragma once

#include "ease.h"
#include "Skin.h"
#include "Image.h"
#include "Font.h"
#include "Timer.h"

class SkinEngine
{
public:
	class Timeline
	{
	private:
		struct Keyframe
		{
			double time;
			double value;
			ease::EaseFunc func;
		};

		std::wstring _timer;
		boost::optional<double> _loop;
		double _time_max = 0;
		std::vector<Keyframe> _keyframes;

	public:
		Timeline();
		Timeline(boost::any any, std::unordered_map<std::wstring, double> specials, double def);

		double operator()(std::unordered_map<std::wstring, Timer>& timers);
	};

	class Element
	{
	protected:
		SkinEngine& engine;

	public:
		Element(SkinEngine& engine) : engine(engine) { }
		virtual ~Element() { }
		virtual void draw() = 0;
	};

	class PreparedImage
	{
	private:
		bool _fail = true;
		double _cycle;
		std::vector<std::shared_ptr<Image>> _images;
		int _image_count;
		std::wstring _timer = L"scene";

		PreparedImage(SkinEngine& engine, const Skin::Element& e);
		PreparedImage(std::shared_ptr<Image> image);

	public:
		virtual ~PreparedImage() { }

		static std::shared_ptr<PreparedImage> make(SkinEngine& engine, const Skin::Element& e);
		static std::shared_ptr<PreparedImage> make(std::shared_ptr<Image> image);

		auto fail() { return _fail; }
		auto cycle() { return _cycle; }
		const auto& images() { return _images; }
		auto image_count() { return _image_count; }
		const auto& timer() { return _timer; }
	};

	class ImageElement : public Element
	{
	private:
		Timeline _x, _y, _w, _h, _ax, _ay, _angle, _r, _g, _b, _a;
		bool _filter;
		int _blend;

		void _initialize_params(const Skin::Element& e);

	protected:
		bool _fail = true;
		std::shared_ptr<PreparedImage> _preparedImage;
		std::vector<Skin::Condition> _conditions;
		int x, y, ax, ay, r, g, b, a, i;
		double w, h, angle;

		void set_parameters();

	public:
		ImageElement(SkinEngine& engine, const Skin::Element& e, bool prepare_image = true);
		virtual void draw() override;

		static int blend_from_any(boost::any any);
		static bool is_valid(SkinEngine& engine, std::vector<Skin::Condition>& conditions);

		virtual bool fail() { return _fail; }
	};

	class CursorElement : public ImageElement
	{
	public:
		CursorElement(SkinEngine& engine, const Skin::Element& e) : ImageElement(engine, e) { }
		virtual void draw() override;
	};

	class MouseoverElement : public ImageElement
	{
	private:
		int mx, my, mw, mh;

	public:
		MouseoverElement(SkinEngine& engine, const Skin::Element& e);
		virtual void draw() override;
	};

	class NumberElement : public ImageElement
	{
	private:
		std::wstring id;
		long long _number;

	protected:
		int digit;
		int type;
		bool center;
		bool negative;
		std::vector<int> digits;

		void set_number_parameters(long long number);
		void draw_number();

	public:
		NumberElement(SkinEngine& engine, const Skin::Element& e, bool prepare_image = true);
		virtual void draw() override;
	};

	class TextElement : public Element
	{
	private:
		bool _fail = true;
		std::shared_ptr<Font> _font;
		int _axtype = 0, _aytype = 0;
		Timeline _x, _y, _w, _h, _ax, _ay, _angle, _r, _g, _b, _a, _margin;
		bool _filter;
		int _blend;
		std::wstring _id;
		std::wstring _before_text;
		int _before_margin;

	protected:
		std::vector<Skin::Condition> _conditions;
		std::shared_ptr<Image> img;
		int x, y, ax, ay, r, g, b, a;
		double w, h, ew, angle;

		bool set_parameters();
		virtual boost::optional<std::wstring> target_text();

	public:
		TextElement(SkinEngine& engine, const Skin::Element& e);
		virtual void draw() override;

		bool fail() { return _fail; }
	};

	class ButtonElement
	{
	private:
		std::vector<Skin::Condition> _conditions;
		Timeline _x, _y, _w, _h;
		std::wstring _id;

	public:
		ButtonElement(const Skin::Element& e);

		bool hit(SkinEngine& engine);

		auto id() { return _id; }
	};

	class BarGraphElement : public ImageElement
	{
	private:
		std::wstring id;
		bool _horizontal = false;

	public:
		BarGraphElement(SkinEngine& engine, const Skin::Element& e);
		virtual void draw() override;
	};

	class SliderElement : public ImageElement
	{
	private:
		std::wstring id;
		double range = 0;
		enum Direction
		{
			Up,
			Down,
			Left,
			Right
		} direction;

	public:
		SliderElement(SkinEngine& engine, const Skin::Element& e);
		virtual void draw() override;
	};

	class AggregateElement : public Element
	{
	private:
		std::vector<std::shared_ptr<Element>> _elements;

	public:
		AggregateElement(SkinEngine& engine, std::vector<std::shared_ptr<Element>> elements) : Element(engine), _elements(elements) { }
		virtual void draw() override
		{
			for (auto& a : _elements)
				a->draw();
		}
	};

private:
	std::vector<std::shared_ptr<Element>> _elements;
	std::vector<ButtonElement> _buttons;
	std::unordered_map<std::wstring, std::shared_ptr<Image>> _images;
	std::unordered_map<std::wstring, std::shared_ptr<Font>> _fonts;
	std::unordered_map<std::wstring, Timer> _timers;
	std::unordered_map<std::wstring, bool> _flags;
	std::unordered_map<std::wstring, long long> _numbers;
	std::unordered_map<std::wstring, std::wstring> _texts;
	std::unordered_map<std::wstring, double> _bargraph_values;
	std::unordered_map<std::wstring, double> _slider_values;
	int _cursor_x = 10, _cursor_y = 10;

public:
	SkinEngine();
	virtual ~SkinEngine() { }

	typedef std::function<std::shared_ptr<Element>(const Skin::Element&)> CustomFunction;

	void interpret(Skin& skin);
	void interpret(Skin& skin, CustomFunction custom);
	std::shared_ptr<Element> default_element(const Skin::Element& e);
	void draw();
	std::vector<std::wstring> hit();
	void set_switch_timers(std::wstring id, bool value);

	auto& images() { return _images; }
	auto& fonts() { return _fonts; }
	auto& timers() { return _timers; }
	auto& flags() { return _flags; }
	auto& numbers() { return _numbers; }
	auto& texts() { return _texts; }
	auto& bargraph_values() { return _bargraph_values; }
	auto& slider_values() { return _slider_values; }
	auto cursor_x() { return _cursor_x; }
	auto cursor_y() { return _cursor_y; }
	void set_cursor_x(int x) { _cursor_x = x; }
	void set_cursor_y(int y) { _cursor_y = y; }
};
