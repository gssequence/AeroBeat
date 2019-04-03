#include "stdafx.h"
#include "Image.h"

Image::Image(std::wstring path)
{
	if (!fs::exists(path) || fs::is_directory(path)) return;

	int h = dx::LoadGraph(path.c_str());
	if (h == -1) return;
	_handle = h;
	_fail = false;
	dx::GetGraphSize(_handle, &_width, &_height);
}

Image::Image(Image& image, int x, int y, int w, int h)
{
	if (image.fail()) return;
	int dh = dx::DerivationGraph(x, y, w, h, image.handle());
	if (dh == -1) return;
	_handle = dh;
	_fail = false;
	dx::GetGraphSize(_handle, &_width, &_height);
}

Image::Image(int handle)
{
	if (handle == -1) return;
	_handle = handle;
	_fail = false;
	dx::GetGraphSize(_handle, &_width, &_height);
}

Image::~Image()
{
	dx::DeleteGraph(_handle);
}

std::shared_ptr<Image> Image::load(std::wstring path)
{
	return std::shared_ptr<Image>(new Image(path));
}

std::shared_ptr<Image> Image::make(int handle)
{
	return std::shared_ptr<Image>(new Image(handle));
}

std::shared_ptr<Image> Image::trim(int x, int y, int w, int h)
{
	return std::shared_ptr<Image>(new Image(*this, x, y, w, h));
}

std::vector<std::shared_ptr<Image>> Image::trim_multi(int x, int y, int w, int h, int nx, int ny)
{
	std::vector<std::shared_ptr<Image>> ret;
	for (int j = 0; j < ny; j++)
	{
		for (int i = 0; i < nx; i++)
		{
			auto img = trim(x + i * w, y + j * h, w, h);
			if (img->fail()) return decltype(ret)();
			ret.push_back(img);
		}
	}
	return std::move(ret);
}

void Image::start()
{
	dx::SeekMovieToGraph(_handle, 0);
	dx::PlayMovieToGraph(_handle);
}
