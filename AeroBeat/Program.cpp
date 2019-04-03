#include "stdafx.h"
#include "Window.h"

int APIENTRY wWinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPWSTR cmdLine, int cmdShow)
{
	Window w(cmdLine);
	if (w.fail())
	{
		MessageBox(NULL, L"Failed to initialize DirectX.", L"Error", MB_OK | MB_ICONERROR);
		return EXIT_FAILURE;
	}
	w.run();
	return 0;
}
