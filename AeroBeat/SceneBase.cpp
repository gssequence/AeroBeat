#include "stdafx.h"
#include "Window.h"
#include "SceneBase.h"

SceneBase::SceneBase(Window* window, int width, int height)
{
	_window = window;
	_screenHandle = dx::MakeScreen(width, height);
}

SceneBase::~SceneBase()
{
	dx::DeleteGraph(_screenHandle);
}
