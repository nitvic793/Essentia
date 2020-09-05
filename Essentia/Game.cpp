#include "Game.h"
#include "ModelLoader.h"
#include <DirectXMath.h>
#include "imgui.h"
#include "BaseSystems.h"
#include "ComponentReflector.h"
#include "AnimationSystem.h"

void Game::Setup()
{
	renderer = MakeScoped<Renderer>();
	keyboard = MakeScoped<DirectX::Keyboard>();
	mouse = MakeScoped<DirectX::Mouse>();
	entityManager.Initialize(Mem::GetDefaultAllocator());
	frameAllocator.Initialize(CMaxScratchSize, Mem::GetDefaultAllocator());

	auto ec = EngineContext::Context;
	ec->EntityManager = &entityManager;
	ec->RendererInstance = renderer.Get();
	ec->DefaultAllocator = Mem::GetDefaultAllocator();
	ec->FrameAllocator = &frameAllocator;
	ec->ComponentReflector = &GComponentReflector;
	ec->GameSystemManager = &gameSystemsManager;
	ec->CoreSystemManager = &coreSystemsManager;
	ec->GameStateManager = &gameStateManager;

	coreSystemsManager.Setup(&entityManager);
	gameSystemsManager.Setup(&entityManager);

	RegisterComponents();
	coreSystemsManager.RegisterSystem<TransformUpdateSystem>();
	coreSystemsManager.RegisterSystem<UpdateBaseDrawablesSystem>();
	coreSystemsManager.RegisterSystem<UpdateCameraSystem>();
	coreSystemsManager.RegisterSystem<EditorSaveSystem>();
	coreSystemsManager.RegisterSystem<FreeCameraSystem>();
	coreSystemsManager.RegisterSystem<AnimationSystem>();

	// Initialize renderer and core systems. gameSystemsManager is initialized through external invocation.
	renderer->Initialize();
	renderer->EndInitialization();
	coreSystemsManager.Initialize();

	// This function also loads resources(textures, meshes) along with the scene entities.
	gameStateManager.LoadScene("scene.json");
	Initialize();

	auto windowSize = renderer->GetWindow()->GetWindowSize();
	camera = MakeScopedArgs<Camera>((float)windowSize.Width, (float)windowSize.Height);
}

void Game::Run()
{
	float localCounter = 0.f;
	auto window = renderer->GetWindow();
	timer.Start();
	window->StartMessagePump([&]
		{
			timer.Tick();
			localCounter += timer.DeltaTime;
			auto kbState = keyboard->GetState();
			auto mouseState = mouse->GetState();
			coreSystemsManager.Update(kbState, mouseState, nullptr);
			if (gameStateManager.IsPlaying())
			{
				gameSystemsManager.Update(kbState, mouseState, nullptr);
			}

			Update();
			camera->Update();
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
	systemLoadCallback();
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
	renderer->CleanUp();
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
