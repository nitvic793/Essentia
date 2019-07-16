#pragma once

#include "Renderer.h"
#include "Keyboard.h"
#include "Mouse.h"
#include "Camera.h"

class Game
{
public:
	void Setup();
	void Run();
	~Game();
protected:
	virtual void	Initialize() {};
	virtual void	Update() {};
	void			Render();

	std::unique_ptr<Renderer>			renderer;
	std::unique_ptr<DirectX::Keyboard>	keyboard;
	std::unique_ptr<DirectX::Mouse>		mouse;
	std::unique_ptr<Camera>				camera;
private:
};

