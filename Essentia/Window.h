#pragma once
#include <windows.h>
#include <functional>
#include <string>
#include "Declarations.h"

class Window
{
public:
	static Window* Instance;
	LRESULT CALLBACK	WindowsMessageCallback(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

	void				Initialize(HINSTANCE hInstance, int width, int height, const char* windowName = "Essentia", const char* windowTitle = "Essentia", bool fullscreen = false, int ShowWnd = 1);
	HWND				GetWindowHandle();
	void				StartMessagePump(std::function<void()> callback);
	static Window*		GetInstance();
	bool				IsFullscreen();
	ScreenSize			GetWindowSize();
	void				RegisterOnResizeCallback(std::function<void()> callback);
	~Window();
private:
	Window();
	std::vector<std::function<void()>> onResizeCallbacks = {};
	std::string windowName;
	std::string windowTitle;
	int			width;
	int			height;
	bool		fullscreen;
	HWND		windowHandle;

	void		OnResize();
	
	friend class Renderer;
};

