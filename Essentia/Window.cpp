#include "Window.h"
#include "Keyboard.h"
#include "Mouse.h"
#include <windowsx.h>

using namespace DirectX;

Window* Window::Instance = nullptr;

LRESULT CALLBACK WindowsProcessMessage(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	return Window::Instance->WindowsMessageCallback(hWnd, msg, wParam, lParam);
}

LRESULT Window::WindowsMessageCallback(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_ACTIVATEAPP:
		Keyboard::ProcessMessage(msg, wParam, lParam);
		Mouse::ProcessMessage(msg, wParam, lParam);
		break;
	case WM_KEYDOWN:
	case WM_SYSKEYDOWN:
	case WM_KEYUP:
	case WM_SYSKEYUP:
		Keyboard::ProcessMessage(msg, wParam, lParam);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	case WM_LBUTTONDOWN:
	case WM_MBUTTONDOWN:
	case WM_RBUTTONDOWN:
		//OnMouseDown(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		Mouse::ProcessMessage(msg, wParam, lParam);
		return 0;

		// Mouse button being released (while the cursor is currently over our window)
	case WM_LBUTTONUP:
	case WM_MBUTTONUP:
	case WM_RBUTTONUP:
		//OnMouseUp(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		Mouse::ProcessMessage(msg, wParam, lParam);
		return 0;

		// Cursor moves over the window (or outside, while we're currently capturing it)
	case WM_MOUSEMOVE:
		//OnMouseMove(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		Mouse::ProcessMessage(msg, wParam, lParam);
		return 0;

		// Mouse wheel is scrolled
	case WM_MOUSEWHEEL:
		//OnMouseWheel(GET_WHEEL_DELTA_WPARAM(wParam) / (float)WHEEL_DELTA, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		Mouse::ProcessMessage(msg, wParam, lParam);
		return 0;
	case WM_INPUT:
	case WM_XBUTTONDOWN:
	case WM_XBUTTONUP:
	case WM_MOUSEHOVER:
		Mouse::ProcessMessage(msg, wParam, lParam);
		break;
	}

	return DefWindowProc(hWnd, msg, wParam, lParam);
}

void Window::Initialize(HINSTANCE hInstance, int width, int height, const char* windowName, const char* windowTitle, bool fullscreen, int ShowWnd)
{
	this->windowName = windowName;
	this->windowTitle = windowTitle;

	if (fullscreen)
	{
		HMONITOR hmon = MonitorFromWindow(windowHandle,
			MONITOR_DEFAULTTONEAREST);
		MONITORINFO mi = { sizeof(mi) };
		GetMonitorInfo(hmon, &mi);

		width = mi.rcMonitor.right - mi.rcMonitor.left;
		height = mi.rcMonitor.bottom - mi.rcMonitor.top;
	}

	this->width = width;
	this->height = height;

	WNDCLASSEX wndClass;
	ZeroMemory(&wndClass, sizeof(WNDCLASSEX));
	wndClass.cbSize = sizeof(WNDCLASSEX);
	wndClass.style = CS_HREDRAW | CS_VREDRAW;
	wndClass.lpfnWndProc = WindowsProcessMessage;
	wndClass.hInstance = hInstance;
	wndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndClass.hbrBackground = (HBRUSH)COLOR_WINDOW;
	wndClass.lpszClassName = windowName;

	RegisterClassEx(&wndClass);
	RECT clientRect;
	SetRect(&clientRect, 0, 0, width, height);
	AdjustWindowRect(
		&clientRect,
		WS_OVERLAPPEDWINDOW,	// Has a title bar, border, min and max buttons, etc.
		false);

	RECT desktopRect;
	GetClientRect(GetDesktopWindow(), &desktopRect);
	int centeredX = (desktopRect.right / 2) - (clientRect.right / 2);
	int centeredY = (desktopRect.bottom / 2) - (clientRect.bottom / 2);

	windowHandle = CreateWindow(
		wndClass.lpszClassName,
		windowTitle,
		WS_OVERLAPPEDWINDOW,
		centeredX,
		centeredY,
		clientRect.right - clientRect.left,	// Calculated width
		clientRect.bottom - clientRect.top,	// Calculated height
		0,			// No parent window
		0,			// No menu
		hInstance,	// The app's handle
		0);	  // used with multiple windows, NULL

	if (fullscreen)
	{
		SetWindowLong(windowHandle, GWL_STYLE, 0);
	}

	ShowWindow(windowHandle, ShowWnd);
	UpdateWindow(windowHandle);
}

HWND Window::GetWindowHandle()
{
	return windowHandle;
}

void Window::StartMessagePump(std::function<void()> callback)
{
	MSG msg = {};
	while (msg.message != WM_QUIT)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			callback();
		}
	}
}

Window* Window::GetInstance()
{
	return Instance;
}

bool Window::IsFullscreen()
{
	return fullscreen;
}

WindowSize Window::GetWindowSize()
{
	return WindowSize{ width, height };
}

Window::Window()
{
	Instance = this;
}

Window::~Window()
{
}