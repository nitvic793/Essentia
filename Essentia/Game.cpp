#include "Game.h"
#include "ModelLoader.h"
#include <DirectXMath.h>
#include "imgui.h"
#include "BaseSystems.h"
#include "ComponentReflector.h"
#include "AnimationSystem.h"
#include "EventTypes.h"
#include "TerrainUpdateSystem.h"
#include "PhysicsSystem.h"
#include "Trace.h"
#include <ScriptingSystem.h>
#include <JobSystem.h>

void Game::Setup(Callback gameSystemsInitCallback)
{
	frameAllocator.Initialize(CMaxScratchSize, Mem::GetDefaultAllocator());
	es::jobs::InitJobSystem();

	GContext->FrameAllocator = &frameAllocator;
	GContext->GameInstance = this;

	renderer = MakeScoped<Renderer>();
	keyboard = MakeScoped<DirectX::Keyboard>();
	mouse = MakeScoped<DirectX::Mouse>();
	entityManager.Initialize(Mem::GetDefaultAllocator());
	

	auto ec = EngineContext::Context;
	ec->EntityManager = &entityManager;
	ec->RendererInstance = renderer.Get();
	ec->DefaultAllocator = Mem::GetDefaultAllocator();
	ec->ComponentReflector = &GComponentReflector;
	ec->GameSystemManager = &gameSystemsManager;
	ec->CoreSystemManager = &coreSystemsManager;
	ec->GameStateManager = &gameStateManager;
	ec->EventBus = &eventBus;
	es::GEventBus = &eventBus;

	coreSystemsManager.Setup(&entityManager);
	gameSystemsManager.Setup(&entityManager);
	scriptSystemsManager.Setup(&entityManager);

	RegisterComponents();
	coreSystemsManager.RegisterSystem<PhysicsSystem>();
	coreSystemsManager.RegisterSystem<TransformUpdateSystem>();
	coreSystemsManager.RegisterSystem<BoundingBoxUpdateSystem>();
	coreSystemsManager.RegisterSystem<UpdateBaseDrawablesSystem>();
	coreSystemsManager.RegisterSystem<UpdateCameraSystem>();
	coreSystemsManager.RegisterSystem<EditorSaveSystem>();
	coreSystemsManager.RegisterSystem<FreeCameraSystem>();
	coreSystemsManager.RegisterSystem<AnimationSystem>(); 
	coreSystemsManager.RegisterSystem<TerrainUpdateSystem>();

	scriptSystemsManager.RegisterSystem<ScriptingSystem>();

	// Initialize renderer and core systems. 
	renderer->Initialize();
	renderer->EndInitialization();
	coreSystemsManager.Initialize();

	// gameSystemsManager is initialized through this external invocation.
	if (gameSystemsInitCallback) 
	{
		gameSystemsInitCallback();
	}

	// This function also loads resources(textures, meshes) along with the scene entities.
	gameStateManager.LoadScene("scene.json"); // empty.json for empty
	Initialize();

	auto windowSize = renderer->GetWindow()->GetWindowSize();
	camera = MakeScopedArgs<Camera>((float)windowSize.Width, (float)windowSize.Height);

	scriptSystemsManager.Initialize(); //Initialize scripting sub-system only after other systems are initialized
}

void Game::Run()
{
	float localCounter = 0.f;
	auto window = renderer->GetWindow();
	timer.Start();

	GameStartEvent gameStart;
	gameStart.totalTime = timer.TotalTime;
	es::GEventBus->Publish(&gameStart);

	window->StartMessagePump([&]
		{
			timer.Tick();
			localCounter += timer.DeltaTime;
			auto kbState = keyboard->GetState();
			auto mouseState = mouse->GetState();
			if (gameStateManager.IsPlaying())
			{
				gameSystemsManager.Update(kbState, mouseState, nullptr);
				scriptSystemsManager.Update(kbState, mouseState, nullptr);
			}

			coreSystemsManager.Update(kbState, mouseState, nullptr);

			Update();
			//camera->Update();
			Render();

			if (kbState.IsKeyDown(DirectX::Keyboard::Escape))
			{
				PostQuitMessage(0);
			}

			if (kbState.IsKeyDown(DirectX::Keyboard::F5) && localCounter > 0.5f)
			{
				localCounter = 0;
				ReloadSystems();
			}

			if (kbState.IsKeyDown(DirectX::Keyboard::F6) && localCounter > 0.5f)
			{
				localCounter = 0;
				gameStateManager.SetIsPlaying(false);
				ResetSystems();
				entityManager.Reset();
			}

			// Flush callbacks at end of frame
			{
				std::scoped_lock lock(centralMutex);
				while (!eventCallbacks.empty())
				{
   					auto& eventCb = eventCallbacks.front();
					eventCb();
					eventCallbacks.pop();
				}
			}

			frameAllocator.Reset();
		});
}

void Game::ReloadSystems()
{
	//systemLoadCallback();

	ReloadScriptSystemEvent event;
	event.totalTime = timer.TotalTime;
	es::GEventBus->Publish(&event);
}

void Game::ResetSystems()
{
	coreSystemsManager.Reset();
	gameSystemsManager.Reset();
	scriptSystemsManager.Reset();
}

void Game::SetSystemReloadCallback(Callback callback)
{
	systemLoadCallback = callback;
}

SystemManager* Game::GetGameSystemsManager()
{
	return &gameSystemsManager;
}

Game::~Game()
{
	coreSystemsManager.Destroy();
	gameSystemsManager.Destroy();
	scriptSystemsManager.Destroy();
	renderer->CleanUp();
	GComponentReflector.CleanUp();
	es::jobs::DestroyJobSystem();
}

void Game::Render()
{
	uint32 count;
	FrameContext frameContext = { nullptr, &timer };

	frameContext.EntityManager = &entityManager;

	auto entities = entityManager.GetEntities<DrawableComponent>(count);
	frameContext.WorldMatrices = entityManager.GetTransposedWorldMatrices(entities, count);
	frameContext.Drawables = entityManager.GetComponents<DrawableComponent>(frameContext.DrawableCount);

	entities = entityManager.GetEntities<DrawableModelComponent>(count);
	frameContext.DrawableModels = frameContext.EntityManager->GetComponents<DrawableModelComponent>(frameContext.DrawableModelCount);
	frameContext.ModelWorldMatrices = frameContext.EntityManager->GetTransposedWorldMatrices(entities, count);

	renderer->Clear();
	renderer->Render(frameContext);
	renderer->Present();
}
