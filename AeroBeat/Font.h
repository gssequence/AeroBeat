#pragma once

#include "Image.h"
#include "util.h"

class Font
{
public:
	struct Glyph
	{
		std::wstring image;
		int x, y, w;
	};

private:
	bool _fail = true;
	std::wstring _path;
	int _height;
	std::unordered_map<std::wstring, std::wstring> _images;
	std::unordered_map<wchar_t, Glyph> _glyphs;
	std::unordered_map<std::wstring, std::shared_ptr<Image>> _image_contents;
	std::unordered_map<wchar_t, std::shared_ptr<Image>> _glyph_contents;
	int _handle = -1;

	const int _whiteColor = dx::GetColor(255, 255, 255);
	const int _blackColor = dx::GetColor(0, 0, 0);

	Font(std::wstring path);
	Font(std::wstring family, int weight, int size, int type);

	std::shared_ptr<Image> get_image(std::wstring str);
	std::shared_ptr<Image> get_glyph(wchar_t c);

public:
	virtual ~Font();

	static std::shared_ptr<Font> load(std::wstring path);
	static std::shared_ptr<Font> create(std::wstring family, int weight, int size, int type);

	std::shared_ptr<Image> render(std::wstring str, int margin = 0);

	bool fail() { return _fail; }
	int height() { return _height; }
};

FUSION_ADAPT_STRUCT_AUTO(Font::Glyph, (image)(x)(y)(w))
