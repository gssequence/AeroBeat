#include "stdafx.h"
#include "util.h"
#include "SkinEngine.h"

SkinEngine::SkinEngine()
{
	_flags[L"true"] = true;
	_flags[L"false"] = false;
}

void SkinEngine::interpret(Skin& skin)
{
	interpret(skin, [this](auto a) { return default_element(a); });
}

void SkinEngine::interpret(Skin& skin, CustomFunction custom)
{
	for (auto& e : skin.elements())
	{
		if (e.type == L"image_source" && !util::get(_images, e.source))
		{
			auto op = util::get_any<std::wstring, std::wstring>(e.src_params, L"path");
			if (op)
			{
				auto& path = *op;
				auto image = Image::load(path);
				if (!image->fail())
				{
					_images[e.source] = image;
				}
			}
		}
		else if (e.type == L"font_source" && !util::get(_fonts, e.source))
		{
			auto op = util::get_any<std::wstring, std::wstring>(e.src_params, L"path");
			if (op)
			{
				auto& path = *op;
				auto font = Font::load(path);
				if (!font->fail())
				{
					_fonts[e.source] = font;
				}
			}
			else
			{
				auto of = util::get_any<std::wstring, std::wstring>(e.src_params, L"family");
				auto weight = util::round(util::get_any<std::wstring, double>(e.src_params, L"weight", -1));
				auto os = util::get_any<std::wstring, double>(e.src_params, L"size");
				auto typestr = util::get_any<std::wstring, std::wstring>(e.src_params, L"type", L"default");
				if (of && os)
				{
					auto& family = *of;
					auto size = util::round(*os);
					int type = DX_FONTTYPE_NORMAL;
					if (typestr == L"edge") type = DX_FONTTYPE_EDGE;
					else if (typestr == L"aa") type = DX_FONTTYPE_ANTIALIASING;
					else if (typestr == L"edgeaa") type = DX_FONTTYPE_ANTIALIASING_EDGE;
					auto font = Font::create(family, weight, size, type);
					if (!font->fail())
					{
						_fonts[e.source] = font;
					}
				}
			}
		}
	}
	for (auto& a : skin.elements())
	{
		auto ptr = custom(a);
		if (ptr)
		{
			_elements.push_back(ptr);
		}
	}
}

std::shared_ptr<SkinEngine::Element> SkinEngine::default_element(const Skin::Element& e)
{
	if (e.type == L"image")
	{
		auto ptr = std::shared_ptr<ImageElement>(new ImageElement(*this, e));
		if (!ptr->fail())
			return ptr;
	}
	else if (e.type == L"cursor")
	{
		auto ptr = std::shared_ptr<CursorElement>(new CursorElement(*this, e));
		if (!ptr->fail())
			return ptr;
	}
	else if (e.type == L"mouseover")
	{
		auto ptr = std::shared_ptr<MouseoverElement>(new MouseoverElement(*this, e));
		if (!ptr->fail())
			return ptr;
	}
	else if (e.type == L"number")
	{
		auto ptr = std::shared_ptr<NumberElement>(new NumberElement(*this, e));
		if (!ptr->fail())
			return ptr;
	}
	else if (e.type == L"text")
	{
		auto ptr = std::shared_ptr<TextElement>(new TextElement(*this, e));
		if (!ptr->fail())
			return ptr;
	}
	else if (e.type == L"button")
	{
		this->_buttons.push_back(ButtonElement(e));
	}
	else if (e.type == L"bargraph")
	{
		auto ptr = std::shared_ptr<BarGraphElement>(new BarGraphElement(*this, e));
		if (!ptr->fail())
			return ptr;
	}
	else if (e.type == L"slider")
	{
		auto ptr = std::shared_ptr<SliderElement>(new SliderElement(*this, e));
		if (!ptr->fail())
			return ptr;
	}
	return std::shared_ptr<Element>();
}

void SkinEngine::draw()
{
	for (auto& a : _elements)
		a->draw();
}

std::vector<std::wstring> SkinEngine::hit()
{
	std::vector<std::wstring> ret;
	for (auto& a : _buttons)
	{
		if (a.hit(*this))
			ret.push_back(a.id());
	}
	return std::move(ret);
}

void SkinEngine::set_switch_timers(std::wstring id, bool value)
{
	auto& on = timers()[id + L"_on"];
	auto& off = timers()[id + L"_off"];
	if (value)
	{
		on.start();
		off.stop();
	}
	else
	{
		on.stop();
		off.start();
	}
}

#pragma region Timeline

SkinEngine::Timeline::Timeline()
{
	_keyframes.push_back({ 0, 0, ease::stop });
}

SkinEngine::Timeline::Timeline(boost::any any, std::unordered_map<std::wstring, double> specials = { }, double def = 0)
{
	auto od = util::get_any<double>(any);
	if (od)
	{
		auto d = *od;
		_keyframes.push_back({ 0, d, ease::stop });
		return;
	}

	auto ot = util::get_any<Skin::Timeline>(any);
	if (ot)
	{
		auto t = *ot;
		for (auto& a : t.keyframes)
		{
			_keyframes.push_back({ a.time, a.value, ease::from_string(a.easing) });
			_time_max = max(_time_max, a.time);
		}
		std::sort(_keyframes.begin(), _keyframes.end(), [](auto& l, auto& r) { return l.time < r.time; });
		_timer = t.timer;
		_loop = t.loop;
		return;
	}

	auto os = util::get_any<std::wstring>(any);
	if (os)
	{
		auto s = *os;
		if (specials.find(s) != specials.end())
		{
			_keyframes.push_back({ 0, specials[s], ease::stop });
			return;
		}
	}

	_keyframes.push_back({ 0, def, ease::stop });
}

double SkinEngine::Timeline::operator()(std::unordered_map<std::wstring, Timer>& timers)
{
	double time = 0;
	auto timeop = util::get(timers, _timer);
	if (timeop)
		time = (*timeop)();
	if (_loop && _time_max > *_loop)
	{
		while (_time_max < time)
			time -= (_time_max - *_loop);
	}

	auto count = _keyframes.size();
	if (count == 0)
		return 0;
	else if (count == 1)
		return _keyframes[0].value;
	else if (_keyframes.begin()->time >= time)
		return _keyframes[0].value;
	else if (_keyframes.back().time <= time)
		return _keyframes.back().value;
	else
	{
		Keyframe* l = nullptr;
		Keyframe* r = nullptr;
		for (auto& a : _keyframes)
		{
			if (a.time <= time)
				l = &a;
			else if (a.time > time)
			{
				r = &a;
				break;
			}
		}
		if (l == nullptr || r == nullptr) return 0;
		return l->value + l->func((time - l->time) / (r->time - l->time)) * (r->value - l->value);
	}
}

#pragma endregion

#pragma region PreparedImage

SkinEngine::PreparedImage::PreparedImage(SkinEngine& engine, const Skin::Element& e)
{
	auto op = util::get<std::wstring, std::shared_ptr<Image>>(engine.images(), e.source);
	if (!op) return;
	auto ptr = *op;
	_images = ptr->trim_multi(
		util::round(util::get_any<std::wstring, double>(e.src_params, L"x", 0.0)),
		util::round(util::get_any<std::wstring, double>(e.src_params, L"y", 0.0)),
		util::round(util::get_any<std::wstring, double>(e.src_params, L"w", ptr->width())),
		util::round(util::get_any<std::wstring, double>(e.src_params, L"h", ptr->height())),
		util::round(util::get_any<std::wstring, double>(e.src_params, L"nx", 1.0)),
		util::round(util::get_any<std::wstring, double>(e.src_params, L"ny", 1.0))
	);
	_image_count = _images.size();
	if (_image_count == 0) return;
	_cycle = util::get_any<std::wstring, double>(e.src_params, L"cycle", 0.0);
	util::unwrap(util::get_any<std::wstring, std::wstring>(e.src_params, L"timer"), _timer);

	_fail = false;
}

SkinEngine::PreparedImage::PreparedImage(std::shared_ptr<Image> image)
{
	if (!image || image->fail()) return;
	_images = { image };
	_image_count = 1;
	_cycle = 0;
	_fail = false;
}

std::shared_ptr<SkinEngine::PreparedImage> SkinEngine::PreparedImage::make(SkinEngine& engine, const Skin::Element& e)
{
	return std::shared_ptr<PreparedImage>(new PreparedImage(engine, e));
}

std::shared_ptr<SkinEngine::PreparedImage> SkinEngine::PreparedImage::make(std::shared_ptr<Image> image)
{
	return std::shared_ptr<PreparedImage>(new PreparedImage(image));
}

#pragma endregion

#pragma region ImageElement

SkinEngine::ImageElement::ImageElement(SkinEngine& engine, const Skin::Element& e, bool prepareImage) : Element(engine)
{
	// src
	if (prepareImage)
	{
		_preparedImage = PreparedImage::make(engine, e);
		if (_preparedImage->fail()) return;
	}

	// dst & condition
	_initialize_params(e);

	_fail = false;
}

void SkinEngine::ImageElement::_initialize_params(const Skin::Element& e)
{
	// dst
	int img_width = 0, img_height = 0;
	if (_preparedImage && !_preparedImage->fail() && _preparedImage->image_count() >= 1)
	{
		auto& img = _preparedImage->images()[0];
		if (!img->fail())
		{
			img_width = img->width();
			img_height = img->height();
		}
	}
	_x = Timeline(util::get<std::wstring, boost::any>(e.dst_params, L"x", 0.0));
	_y = Timeline(util::get<std::wstring, boost::any>(e.dst_params, L"y", 0.0));
	_w = Timeline(util::get<std::wstring, boost::any>(e.dst_params, L"w", (double)img_width));
	_h = Timeline(util::get<std::wstring, boost::any>(e.dst_params, L"h", (double)img_height));
	_ax = Timeline(util::get<std::wstring, boost::any>(e.dst_params, L"ax", 0.0), { { L"center", img_width / 2.0 },{ L"right", img_width } });
	_ay = Timeline(util::get<std::wstring, boost::any>(e.dst_params, L"ay", 0.0), { { L"center", img_height / 2.0 },{ L"bottom", img_height } });
	_angle = Timeline(util::get<std::wstring, boost::any>(e.dst_params, L"angle", 0.0));
	_r = Timeline(util::get<std::wstring, boost::any>(e.dst_params, L"r", 255.0));
	_g = Timeline(util::get<std::wstring, boost::any>(e.dst_params, L"g", 255.0));
	_b = Timeline(util::get<std::wstring, boost::any>(e.dst_params, L"b", 255.0));
	_a = Timeline(util::get<std::wstring, boost::any>(e.dst_params, L"a", 255.0));
	_filter = util::get_any<std::wstring, std::wstring>(e.dst_params, L"filter", L"false") == L"true";
	_blend = blend_from_any(util::get<std::wstring, boost::any>(e.dst_params, L"blend", L""));

	// condition
	_conditions = e.conditions;
}

int SkinEngine::ImageElement::blend_from_any(boost::any any)
{
	auto op = util::get_any<std::wstring>(any);
	if (op)
	{
		auto str = *op;
		if (str == L"none") return DX_BLENDMODE_NOBLEND;
		if (str == L"alpha") return DX_BLENDMODE_ALPHA;
		if (str == L"add") return DX_BLENDMODE_ADD;
		if (str == L"sub") return DX_BLENDMODE_SUB;
		if (str == L"mul") return DX_BLENDMODE_MUL;
		if (str == L"invert") return DX_BLENDMODE_INVSRC;
	}
	return DX_BLENDMODE_ALPHA;
}

bool SkinEngine::ImageElement::is_valid(SkinEngine& engine, std::vector<Skin::Condition>& conditions)
{
	for (auto& a : conditions)
	{
		bool ans;
		auto timer = util::get(engine.timers(), a.identifier);
		auto flag = util::get(engine.flags(), a.identifier);
		if (timer)
			ans = a.negate ^ timer->isEnabled();
		else if (flag)
			ans = a.negate ^ *flag;
		else
			ans = a.negate;
		if (!ans)
			return false;
	}
	return true;
}

void SkinEngine::ImageElement::set_parameters()
{
	x = util::round(_x(engine.timers()));
	y = util::round(_y(engine.timers()));
	w = _w(engine.timers());
	h = _h(engine.timers());
	ax = util::round(_ax(engine.timers()));
	ay = util::round(_ay(engine.timers()));
	angle = _angle(engine.timers());
	r = util::clip(util::round(_r(engine.timers())), 0, 255);
	g = util::clip(util::round(_g(engine.timers())), 0, 255);
	b = util::clip(util::round(_b(engine.timers())), 0, 255);
	a = util::clip(util::round(_a(engine.timers())), 0, 255);

	if (_preparedImage->cycle() <= 0.0)
		i = 0;
	else
		i = ((int)(engine.timers()[_preparedImage->timer()]() * _preparedImage->image_count() / _preparedImage->cycle())) % _preparedImage->image_count();

	dx::SetDrawMode(_filter ? DX_DRAWMODE_BILINEAR : DX_DRAWMODE_NEAREST);
	dx::SetDrawBlendMode(_blend, a);
	dx::SetDrawBright(r, g, b);
}

void SkinEngine::ImageElement::draw()
{
	if (!is_valid(engine, _conditions)) return;
	set_parameters();

	auto& img = *_preparedImage->images()[i];
	dx::DrawRotaGraph3(x, y, ax, ay, w / img.width() , h / img.height(), angle * DX_PI / 180, img.handle(), true);
}

#pragma endregion

#pragma region CursorElement

void SkinEngine::CursorElement::draw()
{
	if (!is_valid(engine, _conditions)) return;
	set_parameters();
	
	x += engine.cursor_x();
	y += engine.cursor_y();

	auto& img = *_preparedImage->images()[i];
	dx::DrawRotaGraph3(x, y, ax, ay, w / img.width(), h / img.height(), angle * DX_PI / 180, img.handle(), true);
}

#pragma endregion

#pragma region MouseoverElement

SkinEngine::MouseoverElement::MouseoverElement(SkinEngine& engine, const Skin::Element& e) : ImageElement(engine, e)
{
	if (fail()) return;

	mx = util::round(util::get_any<std::wstring, double>(e.src_params, L"mx", 0.0));
	my = util::round(util::get_any<std::wstring, double>(e.src_params, L"my", 0.0));
	mw = util::round(util::get_any<std::wstring, double>(e.src_params, L"mw", 0.0));
	mh = util::round(util::get_any<std::wstring, double>(e.src_params, L"mh", 0.0));
}

void SkinEngine::MouseoverElement::draw()
{
	if (!is_valid(engine, _conditions)) return;
	int cx = engine.cursor_x(), cy = engine.cursor_y();
	if (cx < mx || mx + mw <= cx || cy < my || my + mh <= cy) return;

	set_parameters();

	auto& img = *_preparedImage->images()[i];
	dx::DrawRotaGraph3(x, y, ax, ay, w / img.width(), h / img.height(), angle * DX_PI / 180, img.handle(), true);
}

#pragma endregion

#pragma region NumberElement

SkinEngine::NumberElement::NumberElement(SkinEngine& engine, const Skin::Element& e, bool prepare_image) : ImageElement(engine, e, prepare_image)
{
	if (fail()) return;

	if (!prepare_image) return;

	id = util::get_any<std::wstring, std::wstring>(e.src_params, L"id", L"");
	digit = util::round(util::get_any<std::wstring, double>(e.src_params, L"digit", 0.0));
	type = util::round(util::get_any<std::wstring, double>(e.src_params, L"type", 0.0));
	center = util::get_any<std::wstring, std::wstring>(e.src_params, L"center", L"false") == L"true";

	if (digit > 0 && type > 0 && (type == 10 || type == 11 || type == 24) && _preparedImage->image_count() % type == 0) return;

	_fail = true;
}

void SkinEngine::NumberElement::set_number_parameters(long long number)
{
	set_parameters();
	_number = number;
	i /= type;
	negative = _number < 0;
	digits = util::digits(_number);
}

void SkinEngine::NumberElement::draw()
{
	if (!is_valid(engine, _conditions)) return;

	auto on = util::get<std::wstring, long long>(engine.numbers(), id);
	if (!on) return;
	set_number_parameters(*on);
	draw_number();
}

void SkinEngine::NumberElement::draw_number()
{
	auto d = (unsigned)digit;
	if (center) d = min(digits.size(), d);
	for (unsigned j = 0; j < d; j++)
	{
		int n;
		if (type == 24 && j == 0)
		{
			n = _number >= 0 ? 11 : 23;
		}
		else
		{
			unsigned k = d - j - 1;
			if (digits.size() - 1 >= k) n = digits[k];
			else if (type == 24 || type == 11) n = 10;
			else if (type == 10) n = 0;
			if (type == 24 && negative) n += 12;
		}

		auto& img = *_preparedImage->images()[type * i + n];
		if (center)
			dx::DrawRotaGraph3((int)(x + w * j - (w * d / 2)), y, ax, ay, w / img.width(), h / img.height(), angle * DX_PI / 180, img.handle(), true);
		else
			dx::DrawRotaGraph3((int)(x + w * j), y, ax, ay, w / img.width(), h / img.height(), angle * DX_PI / 180, img.handle(), true);
	}
}

#pragma endregion

#pragma region TextElement

SkinEngine::TextElement::TextElement(SkinEngine& engine, const Skin::Element& e) : Element(engine)
{
	// src
	auto op = util::get<std::wstring, std::shared_ptr<Font>>(engine.fonts(), e.source);
	if (!op) return;
	_font = *op;
	_id = util::get_any<std::wstring, std::wstring>(e.src_params, L"id", L"");

	// dst
	_x = Timeline(util::get<std::wstring, boost::any>(e.dst_params, L"x", 0.0));
	_y = Timeline(util::get<std::wstring, boost::any>(e.dst_params, L"y", 0.0));
	_w = Timeline(util::get<std::wstring, boost::any>(e.dst_params, L"w", HUGE_VAL));
	_h = Timeline(util::get<std::wstring, boost::any>(e.dst_params, L"h", (double)_font->height()));
	
	auto str = util::get_any<std::wstring, std::wstring>(e.dst_params, L"ax");
	if (str && (*str == L"center" || *str == L"right"))
		_axtype = *str == L"center" ? 1 : 2;
	else
		_ax = Timeline(util::get<std::wstring, boost::any>(e.dst_params, L"ax", 0.0));
	str = util::get_any<std::wstring, std::wstring>(e.dst_params, L"ay");
	if (str && (*str == L"center" || *str == L"bottom"))
		_aytype = *str == L"center" ? 1 : 2;
	else
		_ay = Timeline(util::get<std::wstring, boost::any>(e.dst_params, L"ay", 0.0));
	
	_angle = Timeline(util::get<std::wstring, boost::any>(e.dst_params, L"angle", 0.0));
	_r = Timeline(util::get<std::wstring, boost::any>(e.dst_params, L"r", 255.0));
	_g = Timeline(util::get<std::wstring, boost::any>(e.dst_params, L"g", 255.0));
	_b = Timeline(util::get<std::wstring, boost::any>(e.dst_params, L"b", 255.0));
	_a = Timeline(util::get<std::wstring, boost::any>(e.dst_params, L"a", 255.0));
	_filter = util::get_any<std::wstring, std::wstring>(e.dst_params, L"filter", L"false") == L"true";
	_blend = ImageElement::blend_from_any(util::get<std::wstring, boost::any>(e.dst_params, L"blend", L""));
	_margin = Timeline(util::get<std::wstring, boost::any>(e.dst_params, L"margin", 0));

	// condition
	_conditions = e.conditions;

	_fail = false;
}

boost::optional<std::wstring> SkinEngine::TextElement::target_text()
{
	return util::get<std::wstring, std::wstring>(engine.texts(), _id);
}

bool SkinEngine::TextElement::set_parameters()
{
	auto ot = target_text();
	if (!ot) return false;

	auto text = *ot;
	auto margin = util::round(_margin(engine.timers()));
	if (text != _before_text || margin != _before_margin || !img || img->fail())
	{
		_before_text = text;
		_before_margin = margin;
		img = _font->render(_before_text, _before_margin);
	}
	if (!img || img->fail()) return false;

	x = util::round(_x(engine.timers()));
	y = util::round(_y(engine.timers()));
	w = _w(engine.timers());
	h = _h(engine.timers());

	ew = min(w, img->width() * h / _font->height());
	ax = util::round(_axtype == 0 ? util::round(_ax(engine.timers())) : _axtype == 1 ? img->width() / 2.0 : img->width());
	ay = util::round(_aytype == 0 ? util::round(_ay(engine.timers())) : _aytype == 1 ? img->height() / 2.0 : img->height());

	angle = _angle(engine.timers());
	r = util::clip(util::round(_r(engine.timers())), 0, 255);
	g = util::clip(util::round(_g(engine.timers())), 0, 255);
	b = util::clip(util::round(_b(engine.timers())), 0, 255);
	a = util::clip(util::round(_a(engine.timers())), 0, 255);

	dx::SetDrawMode(_filter ? DX_DRAWMODE_BILINEAR : DX_DRAWMODE_NEAREST);
	dx::SetDrawBlendMode(_blend, a);
	dx::SetDrawBright(r, g, b);

	return true;
}

void SkinEngine::TextElement::draw()
{
	if (!ImageElement::is_valid(engine, _conditions)) return;
	if (!set_parameters()) return;

	dx::DrawRotaGraph3(x, y, ax, ay, ew / img->width(), h / img->height(), angle * DX_PI / 180, img->handle(), true);
}

#pragma endregion

#pragma region ButtonElement

SkinEngine::ButtonElement::ButtonElement(const Skin::Element& e)
{
	_id = util::get_any<std::wstring, std::wstring>(e.src_params, L"id", L"");
	_x = Timeline(util::get<std::wstring, boost::any>(e.dst_params, L"x", 0.0));
	_y = Timeline(util::get<std::wstring, boost::any>(e.dst_params, L"y", 0.0));
	_w = Timeline(util::get<std::wstring, boost::any>(e.dst_params, L"w", 0.0));
	_h = Timeline(util::get<std::wstring, boost::any>(e.dst_params, L"h", 0.0));
	_conditions = e.conditions;
}

bool SkinEngine::ButtonElement::hit(SkinEngine& engine)
{
	if (!ImageElement::is_valid(engine, _conditions)) return false;
	auto x = util::round(_x(engine.timers()));
	auto y = util::round(_y(engine.timers()));
	auto w = util::round(_w(engine.timers()));
	auto h = util::round(_h(engine.timers()));
	int cx = engine.cursor_x(), cy = engine.cursor_y();
	if (cx < x || x + w <= cx || cy < y || y + h <= cy) return false;
	return true;
}

#pragma endregion

#pragma region BarGraphElement

SkinEngine::BarGraphElement::BarGraphElement(SkinEngine& engine, const Skin::Element& e) : ImageElement(engine, e)
{
	if (fail()) return;

	id = util::get_any<std::wstring, std::wstring>(e.src_params, L"id", L"");
	_horizontal = util::get_any<std::wstring, std::wstring>(e.src_params, L"direction", L"") == L"horizontal";
}

void SkinEngine::BarGraphElement::draw()
{
	if (!is_valid(engine, _conditions)) return;
	set_parameters();

	auto factor = util::clip(util::get<std::wstring, double>(engine.bargraph_values(), id, 0), 0., 1.);
	if (_horizontal)
		w *= factor;
	else
		h *= factor;

	auto& img = *_preparedImage->images()[i];
	dx::DrawRotaGraph3(x, y, ax, ay, w / img.width(), h / img.height(), angle * DX_PI / 180, img.handle(), true);
}

#pragma endregion

#pragma region SliderElement

SkinEngine::SliderElement::SliderElement(SkinEngine& engine, const Skin::Element& e) : ImageElement(engine, e)
{
	if (fail()) return;

	id = util::get_any<std::wstring, std::wstring>(e.src_params, L"id", L"");
	range = util::get_any<std::wstring, double>(e.src_params, L"range", 0);
	auto a = util::get_any<std::wstring, std::wstring>(e.src_params, L"direction", L"up");
	if (a == L"down") direction = Down;
	else if (a == L"left") direction = Left;
	else if (a == L"right") direction = Right;
	else direction = Up;
}

void SkinEngine::SliderElement::draw()
{
	if (!is_valid(engine, _conditions)) return;
	set_parameters();

	auto factor = util::clip(util::get<std::wstring, double>(engine.slider_values(), id, 0), 0., 1.);
	switch (direction)
	{
	case Up:
		y -= util::round(factor * range);
		break;
	case Down:
		y += util::round(factor * range);
		break;
	case Left:
		x -= util::round(factor * range);
		break;
	case Right:
		x += util::round(factor * range);
		break;
	}

	auto& img = *_preparedImage->images()[i];
	dx::DrawRotaGraph3(x, y, ax, ay, w / img.width(), h / img.height(), angle * DX_PI / 180, img.handle(), true);
}

#pragma endregion
