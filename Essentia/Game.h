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
	Game() : allocator(CMaxStackHeapSize) {};
	void Setup();
	void Run();
	~Game();
protected:
	virtual void	Initialize() {};
	virtual void	Update() {};
	void			Render();

	Timer								timer;
	std::unique_ptr<Renderer>           renderer;
	std::unique_ptr<DirectX::Keyboard>	keyboard;
	std::unique_ptr<DirectX::Mouse>		mouse;
	std::unique_ptr<Camera>				camera;
	StackAllocator						allocator;
	EntityManager						entityManager;
	SystemManager						systemManager;
private:
	EngineContext						engineContext;
};

