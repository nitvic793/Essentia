#include "pch.h"
#include "System.h"
#include "Entity.h"

void SystemManager::Setup(EntityManager* entity)
{
	entityManager = entity;
}

void SystemManager::Initialize()
{
	for (auto& system : systems)
	{
		system->entityManager = entityManager;
		system->Initialize();
	}
}

void SystemManager::Update(const DirectX::Keyboard::State& kbState, const DirectX::Mouse::State& mouseState, Camera* camera)
{
	auto timer = Timer::GetInstance();
	float deltaTime = timer->DeltaTime;
	float totalTime = timer->TotalTime;
	for (auto& system : systems)
	{
		system->keyboard = kbState;
		system->mouse = mouseState;
		system->camera = camera;
		system->Update(deltaTime, totalTime);
	}
}

void SystemManager::Destroy()
{
	for (auto& system : systems)
	{
		system->Destroy();
	}
}

TransformRef ISystem::GetTransform(EntityHandle entity)
{
	return entityManager->GetTransform(entity);
}
