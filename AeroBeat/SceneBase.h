#pragma once

class Window;

class SceneBase
{
private:
	int _screenHandle;

protected:
	Window* _window;

public:
	SceneBase(Window* window, int width, int height);
	virtual ~SceneBase();

	virtual bool update() { return true; }
	virtual void draw() = 0;

	int screenHandle() { return _screenHandle; }
};
