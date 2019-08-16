#pragma once

#include "Renderer.h"
#include "Keyboard.h"
#include "Mouse.h"
#include "Camera.h"
#include "Timer.h"
#include "Memory.h"
#include "Entity.h"
#include "System.h"
#include "Engine.h"

class Game
{
public:
	Game() {};
	void Setup();
	void Run();
	~Game();
protected:
	virtual void	Initialize() {};
	virtual void	Update() {};
	void			Render();

	Timer							timer;
	ScopedPtr<Renderer>				renderer;
	ScopedPtr<DirectX::Keyboard>	keyboard;
	ScopedPtr<DirectX::Mouse>		mouse;
	ScopedPtr<Camera>				camera;
	EntityManager					entityManager;
	SystemManager					systemManager;
private:
	EngineContext					engineContext;
};

