#include "Game.h"
#include "ModelLoader.h"

class Sys : public ISystem
{
public:
	virtual void Update(float dt, float totalTime) override
	{
		uint32 count;
		auto components = GetComponents<TestComponent>(count);
		for (auto i = 0u; i < count; ++i)
		{
			components[i].test = totalTime;
		}
	}
};

void Game::Setup()
{
	renderer = std::make_unique<Renderer>();
	keyboard = std::make_unique<DirectX::Keyboard>();
	mouse = std::make_unique<DirectX::Mouse>();

	renderer->Initialize();
	auto meshMgr = renderer->GetMeshManager();
	Initialize();
	renderer->EndInitialization();

	auto windowSize = renderer->GetWindow()->GetWindowSize();
	camera = std::make_unique<Camera>((float)windowSize.Width, (float)windowSize.Height);
	auto id = entityManager.CreateEntity();
	auto id2 = entityManager.CreateEntity();

	entityManager.AddComponent<TestComponent>(id);
	entityManager.AddComponent<TestComponent>(id2);

	systemManager.Setup(&entityManager);
	systemManager.RegisterSystem<Sys>();
}

void Game::Run()
{
	auto window = renderer->GetWindow();
	timer.Start();
	systemManager.Initialize();
	window->StartMessagePump([&] 
		{
			timer.Tick();
			auto kb = keyboard->GetState();
			camera->Update();
			systemManager.Update();
			Update();
			Render();

			if (kb.IsKeyDown(DirectX::Keyboard::Escape))
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
	FrameContext frameContext = { camera.get(), &timer };
	renderer->Clear();
	renderer->Render(frameContext);
	renderer->Present();
}
