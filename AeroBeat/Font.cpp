#include "stdafx.h"
#include "Font.h"

namespace qi = boost::spirit::qi;

#pragma region Parser

struct FontParseResult
{
	int height;
	std::unordered_map<std::wstring, std::wstring> images;
	std::unordered_map<wchar_t, Font::Glyph> glyphs;
};

FUSION_ADAPT_STRUCT_AUTO(FontParseResult, (height)(images)(glyphs))

template<typename Iterator>
struct Skipper : qi::grammar<Iterator>
{
	Skipper() : base_type(definition)
	{
		definition = qi::standard_wide::space | L"//" >> *(qi::standard_wide::char_ - qi::eol) >> qi::eol;
	}

	qi::rule<Iterator> definition;
};

template<typename Iterator>
struct FontParser : qi::grammar<Iterator, FontParseResult(), Skipper<Iterator>>
{
	FontParser() : base_type(definition)
	{
		definition %= height >> images >> glyphs;
		height %= qi::lit(L"height") >> L':' >> qi::int_;
		images %= *image;
		image %= L'[' >> identifier >> L']' >> L':' >> stringLiteral;
		glyphs %= *glyph;
		glyph %= qi::hex >> L':' >> glyph_content;
		glyph_content %= L'[' >> identifier >> L']' >> L'(' >> qi::int_ >> L',' >> qi::int_ >> L',' >> qi::int_ >> L')';
		identifier %= qi::lexeme[(qi::standard_wide::alpha | qi::standard_wide::char_(L'_')) >> *(qi::standard_wide::alnum | qi::standard_wide::char_(L'_'))];
		stringLiteral %= qi::lexeme[L'"' >> +(qi::standard_wide::char_ - L'"') >> L'"'];
	}

	qi::rule<Iterator, FontParseResult(), Skipper<Iterator>> definition;
	qi::rule<Iterator, int(), Skipper<Iterator>> height;
	qi::rule<Iterator, std::unordered_map<std::wstring, std::wstring>(), Skipper<Iterator>> images;
	qi::rule<Iterator, std::pair<std::wstring, std::wstring>(), Skipper<Iterator>> image;
	qi::rule<Iterator, std::unordered_map<wchar_t, Font::Glyph>(), Skipper<Iterator>> glyphs;
	qi::rule<Iterator, std::pair<wchar_t, Font::Glyph>(), Skipper<Iterator>> glyph;
	qi::rule<Iterator, Font::Glyph(), Skipper<Iterator>> glyph_content;
	qi::rule<Iterator, std::wstring(), Skipper<Iterator>> identifier;
	qi::rule<Iterator, std::wstring(), Skipper<Iterator>> stringLiteral;
};

#pragma endregion

Font::Font(std::wstring path)
{
	std::wstring content;
	if (!util::decode(path, content)) return;
	auto it = content.begin();
	auto end = content.end();
	FontParser<decltype(it)> parser;
	Skipper<decltype(it)> skipper;
	FontParseResult result;
	if (qi::phrase_parse(it, end, parser, skipper, result) && it == end)
	{
		if (result.height <= 0 || result.images.size() == 0 || result.glyphs.size() == 0) return;
		_path = path;
		_height = result.height;
		_images = std::move(result.images);
		_glyphs = std::move(result.glyphs);
		_fail = false;
		return;
	}
}

Font::Font(std::wstring family, int weight, int size, int type)
{
	_handle = dx::CreateFontToHandle(family.c_str(), size, weight, type);
	_fail = _handle == -1;
	if (!_fail)
	{
		_height = size;
	}
}

Font::~Font()
{
	if (_handle != -1)
		dx::DeleteFontToHandle(_handle);
}

std::shared_ptr<Font> Font::load(std::wstring path)
{
	return std::shared_ptr<Font>(new Font(path));
}

std::shared_ptr<Font> Font::create(std::wstring family, int weight, int size, int type)
{
	return std::shared_ptr<Font>(new Font(family, weight, size, type));
}

std::shared_ptr<Image> Font::get_image(std::wstring str)
{
	auto img = util::get(_image_contents, str);
	if (img)
		return *img;
	else
	{
		auto path = util::get(_images, str);
		if (path)
		{
			auto image = Image::load((fs::path(_path).parent_path() / *path).generic_wstring());
			if (!image->fail())
			{
				_image_contents[str] = image;
				return image;
			}
		}
	}
	return std::shared_ptr<Image>();
}

std::shared_ptr<Image> Font::get_glyph(wchar_t c)
{
	auto g = util::get(_glyph_contents, c);
	if (g)
		return *g;
	else
	{
		auto info = util::get(_glyphs, c);
		if (info)
		{
			auto image = get_image(info->image);
			if (image)
			{
				auto t = image->trim(info->x, info->y, info->w, _height);
				if (!t->fail())
				{
					_glyph_contents[c] = t;
					return t;
				}
			}
		}
	}

	if (c == L' ')
		return std::shared_ptr<Image>();
	else if (c == L'?')
		return get_glyph(L' ');
	else
		return get_glyph(L'?');
}

std::shared_ptr<Image> Font::render(std::wstring str, int margin)
{
	if (str.empty()) return nullptr;

	dx::SetDrawMode(DX_DRAWMODE_NEAREST);
	dx::SetDrawBlendMode(DX_BLENDMODE_ALPHA, 255);
	dx::SetDrawBright(255, 255, 255);

	if (_handle != -1)
	{
		int w = dx::GetDrawStringWidthToHandle(str.c_str(), str.size(), _handle);
		int before = dx::GetDrawScreen();
		auto image = Image::make(dx::MakeScreen(w, _height, true));
		if (image->fail()) return image;
		dx::SetDrawScreen(image->handle());
		dx::DrawStringToHandle(0, 0, str.c_str(), _whiteColor, _handle, _blackColor);
		dx::SetDrawScreen(before);
		return image;
	}
	else
	{
		int w = 0;
		std::vector<std::shared_ptr<Image>> glyphs;
		int before = dx::GetDrawScreen();
		for (auto& a : str)
		{
			auto g = get_glyph(a);
			if (g)
			{
				glyphs.push_back(g);
				w += g->width();
			}
		}
		int n = glyphs.size();
		if (n >= 2)
			w += (n - 1) * margin;
		auto image = Image::make(dx::MakeScreen(w, _height, true));
		if (image->fail()) return image;
		dx::SetDrawScreen(image->handle());
		int x = 0;
		for (auto& a : glyphs)
		{
			dx::DrawGraph(x, 0, a->handle(), true);
			x += a->width() + margin;
		}
		dx::SetDrawScreen(before);
		return image;
	}
}
