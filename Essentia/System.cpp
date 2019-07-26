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

void SystemManager::Update()
{
	auto timer = Timer::GetInstance();
	float deltaTime = timer->DeltaTime;
	float totalTime = timer->TotalTime;
	for (auto& system : systems)
	{
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
