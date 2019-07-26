#include "Game.h"
#include "ModelLoader.h"

class TransformUpdateSystem : public ISystem
{
public:
	virtual void Update(float dt, float totalTime) override
	{
		uint32 count;
		auto pos = GetComponents<PositionComponent>(count);
		auto rot = GetComponents<RotationComponent>(count);
		auto scale = GetComponents<ScaleComponent>(count);
		auto entities = GetEntities<PositionComponent>(count);
		for (uint32 i = 0; i < count; ++i)
		{
			auto Pos = pos[i].Position;
			auto Rot = rot[i].Rotation;
			auto Scale = scale[i].Scale;
			Transform transform = { Pos, Rot, Scale };
			entityManager->UpdateTransform(entities[i], transform);
		}
	}
};

class RotationSystem : public ISystem
{
public:
	virtual void Initialize()
	{
		entity = entityManager->CreateEntity();
	}
	virtual void Update(float deltaTime, float totalTime) override
	{
		auto transform = GetTransform(entity);
		transform.Rotation->y = sin(totalTime * 2);
		transform.Position->x = sin(totalTime);
	}
private:
	EntityHandle entity;
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

	systemManager.Setup(&entityManager);
	systemManager.RegisterSystem<TransformUpdateSystem>();
	systemManager.RegisterSystem<RotationSystem>();
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
	uint32 count;
	auto entities = entityManager.GetEntities<PositionComponent>(count);
	FrameContext frameContext = { camera.get(), &timer };
	frameContext.WorldMatrices.reserve(count);
	entityManager.GetTransposedWorldMatrices(entities, count, frameContext.WorldMatrices);
	renderer->Clear();
	renderer->Render(frameContext);
	renderer->Present();
}
