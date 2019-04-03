#pragma once
#include "SceneBase.h"

class StartScene : public SceneBase
{
private:
	const int _fontSize = 12;
	const int _windowHeight;
	const int _fontColor = dx::GetColor(255, 255, 255);
	bool _started = false;
	bool _completed = false;
	std::function<void(std::wstring)> _logger;
	std::function<void()> _completion;
	std::mutex _mutex;
	std::vector<std::wstring> _lines{ L"AeroBeat version develop" };

	void _write(const std::wstring& str);

public:
	StartScene(Window* window, int width, int height) : SceneBase(window, width, height), _windowHeight(height)
	{
		dx::SetFontSize(_fontSize);
	}

	virtual ~StartScene() { }
	
	virtual bool update() override;
	virtual void draw() override;
};
