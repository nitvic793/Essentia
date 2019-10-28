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

using Callback = std::function<void()>;

class Game
{
public:
	Game() {};
	void			Setup();
	void			Run();
	void			ReloadSystems();
	void			SetSystemReloadCallback(Callback callback);
	SystemManager*	GetGameSystemsManager();
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
	SystemManager					coreSystemsManager;
	SystemManager					gameSystemsManager;
	Callback						systemLoadCallback;
private:
	EngineContext					engineContext;
};

