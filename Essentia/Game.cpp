#include "Game.h"
#include "ModelLoader.h"
#include <DirectXMath.h>
#include "imgui.h"
#include "BaseSystems.h"

void Game::Setup()
{
	renderer = MakeScoped<Renderer>();
	keyboard = MakeScoped<DirectX::Keyboard>();
	mouse = MakeScoped<DirectX::Mouse>();

	auto ec = EngineContext::Context;
	ec->EntityManager = &entityManager;
	ec->RendererInstance = renderer.Get();
	
	systemManager.Setup(&entityManager);
	systemManager.RegisterSystem<TransformUpdateSystem>();
	systemManager.RegisterSystem<RotationSystem>();
	systemManager.RegisterSystem<FreeCameraSystem>();

	renderer->Initialize();
	Initialize();
	renderer->EndInitialization();
	systemManager.Initialize();

	auto windowSize = renderer->GetWindow()->GetWindowSize();
	camera = MakeScopedArgs<Camera>((float)windowSize.Width, (float)windowSize.Height);
	auto id = entityManager.CreateEntity();
}

void Game::Run()
{
	auto window = renderer->GetWindow();
	timer.Start();
	window->StartMessagePump([&] 
		{
			timer.Tick();
			auto kbState = keyboard->GetState();
			auto mouseState = mouse->GetState();
			systemManager.Update(kbState, mouseState, camera.Get());
			Update();
			camera->Update();
			Render();

			if (kbState.IsKeyDown(DirectX::Keyboard::Escape))
			{
				PostQuitMessage(0);
			}
		});
}

Game::~Game()
{
	systemManager.Destroy();
	renderer->CleanUp();
}

void Game::Render()
{
	uint32 count;
	auto entities = entityManager.GetEntities<DrawableComponent>(count);
	FrameContext frameContext = { camera.Get(), &timer };

	frameContext.WorldMatrices.reserve(count);
	entityManager.GetTransposedWorldMatrices(entities, count, frameContext.WorldMatrices);
	frameContext.Drawables = entityManager.GetComponents<DrawableComponent>(frameContext.DrawableCount);
	frameContext.EntityManager = &entityManager;

	renderer->Clear();
	renderer->Render(frameContext);
	renderer->Present();
}
