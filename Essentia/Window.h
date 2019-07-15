#pragma once
#include <windows.h>
#include <functional>

class Window
{
public:
	static Window*		Instance;
	LRESULT CALLBACK	WindowsMessageCallback(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

	void				Initialize(HINSTANCE hInstance, int width, int height, const char* windowName = "Essentia", const char* windowTitle = "Essentia", bool fullscreen = false, int ShowWnd = 1);
	HWND				GetWindowHandle();
	void				StartMessagePump(std::function<void()> callback);
	static Window*		GetInstance();
	bool				IsFullscreen();
	~Window();
private:
	Window();
	std::string windowName;
	std::string windowTitle;
	int			width;
	int			height;
	bool		fullscreen;
	HWND		windowHandle;
	
	friend class Renderer;
};

