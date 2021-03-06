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
#include "GameStateManager.h"
#include "EventSystem.h"
#include <queue>
#include <mutex>

using Callback = std::function<void()>;

class Game
{
public:
	Game() {};
	void			Setup(Callback gameSystemInitCallback);
	void			Run();
	void			ReloadSystems();
	void			ResetSystems();
	void			SetSystemReloadCallback(Callback callback);
	void			AddEventCallback(Callback&& callback) { std::scoped_lock lock(centralMutex);  eventCallbacks.push(callback); }
	SystemManager*	GetGameSystemsManager();
	SystemManager*	GetCoreSystemsManager() { return &coreSystemsManager; }
	std::mutex		centralMutex;
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
	GameStateManager				gameStateManager;
	EntityManager					entityManager;
	SystemManager					coreSystemsManager;
	SystemManager					gameSystemsManager;
	SystemManager					scriptSystemsManager;
	Callback						systemLoadCallback;
	StackAllocator					frameAllocator;
	std::queue<Callback>			eventCallbacks;
	es::EventBus					eventBus;
private:
	EngineContext					engineContext;
};

