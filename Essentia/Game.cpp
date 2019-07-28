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
		entity2 = entityManager->CreateEntity();
		MaterialHandle mat = { 0 };
		MeshHandle mesh = { 1 };
		MeshHandle cone = Es::CreateMesh("../../Assets/Models/cone.obj");
		entityManager->AddComponent<DrawableComponent>(entity, DrawableComponent::Create(mesh, mat));
		entityManager->AddComponent<DrawableComponent>(entity2, DrawableComponent::Create(cone, mat));
	}

	virtual void Update(float deltaTime, float totalTime) override
	{
		auto transform = GetTransform(entity);
		transform.Rotation->y = totalTime/2;
		transform.Position->x = sin(totalTime/2);

		transform = GetTransform(entity2);
		transform.Rotation->x = totalTime;
		transform.Position->x = cos(totalTime);
	}

private:
	EntityHandle entity;
	EntityHandle entity2;
};

class FreeCameraSystem : public ISystem
{
public:
	virtual void Update(float deltaTime, float totalTime) override
	{
		float horizontal = 0.f;
		float vertical = 0.f;
		if (keyboard.IsKeyDown(DirectX::Keyboard::A))
		{
			horizontal = -1.f;
		}

		if (keyboard.IsKeyDown(DirectX::Keyboard::D))
		{
			horizontal = 1.f;
		}

		if (keyboard.IsKeyDown(DirectX::Keyboard::W))
		{
			vertical = 1.f;
		}

		if (keyboard.IsKeyDown(DirectX::Keyboard::S))
		{
			vertical = -1.f;
		}

		camera->Position.x += horizontal * Speed * deltaTime;
		camera->Position.z += vertical * Speed * deltaTime;
	}

	float Speed = 10.f;
};


void Game::Setup()
{
	renderer = std::make_unique<Renderer>();
	keyboard = std::make_unique<DirectX::Keyboard>();
	mouse = std::make_unique<DirectX::Mouse>();

	auto ec = EngineContext::Context;
	ec->EntityManager = &entityManager;
	ec->RendererInstance = renderer.get();
	
	systemManager.Setup(&entityManager);
	systemManager.RegisterSystem<TransformUpdateSystem>();
	systemManager.RegisterSystem<RotationSystem>();
	systemManager.RegisterSystem<FreeCameraSystem>();

	renderer->Initialize();
	Initialize();
	systemManager.Initialize();
	renderer->EndInitialization();

	auto windowSize = renderer->GetWindow()->GetWindowSize();
	camera = std::make_unique<Camera>((float)windowSize.Width, (float)windowSize.Height);
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
			systemManager.Update(kbState, mouseState, camera.get());
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
	FrameContext frameContext = { camera.get(), &timer };

	frameContext.WorldMatrices.reserve(count);
	entityManager.GetTransposedWorldMatrices(entities, count, frameContext.WorldMatrices);
	frameContext.Drawables = entityManager.GetComponents<DrawableComponent>(frameContext.DrawableCount);

	renderer->Clear();
	renderer->Render(frameContext);
	renderer->Present();
}
