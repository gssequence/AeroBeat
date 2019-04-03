#pragma once

class Image
{
private:
	bool _fail = true;
	int _handle;
	int _width;
	int _height;

	Image(std::wstring path);
	Image(Image& image, int x, int y, int w, int h);
	Image(int handle);

public:
	virtual ~Image();

	static std::shared_ptr<Image> load(std::wstring path);
	static std::shared_ptr<Image> make(int handle);

	std::shared_ptr<Image> trim(int x, int y, int w, int h);
	std::vector<std::shared_ptr<Image>> trim_multi(int x, int y, int w, int h, int nx, int ny);

	void start();

	bool fail() { return _fail; }
	int handle() { return _handle; }
	int width() { return _width; }
	int height() { return _height; }
};
