#pragma once
#include "Timer.h"
#include "Entity.h"
#include "Keyboard.h"
#include "Mouse.h"

class EntityManager;

class ISystem
{
public:
	virtual void Initialize() {};
	virtual void Update(float deltaTime, float totalTime) {};
	virtual void Destroy() {};
	virtual ~ISystem() {};
protected:
	template<typename T>
	T*							GetComponents(uint32& outCount);

	template<typename T>
	EntityHandle*				GetEntities(uint32& count);

	TransformRef				GetTransform(EntityHandle entity);
	EntityManager*				entityManager;
	DirectX::Keyboard::State	keyboard;
	DirectX::Mouse::State		mouse;
private:
	
	friend class SystemManager;
};

template<typename T>
inline T* ISystem::GetComponents(uint32& outCount)
{
	return entityManager->GetComponents<T>(outCount);
}

template<typename T>
inline EntityHandle* ISystem::GetEntities(uint32& count)
{
	return entityManager->GetEntities<T>(count);
}

class SystemManager
{
public:
	void Setup(EntityManager* entity);

	template<typename SystemType>
	void RegisterSystem();

	void Initialize();
	void Update();
	void Destroy();
private:
	std::vector<std::unique_ptr<ISystem>> systems;
	EntityManager* entityManager = nullptr;
};

template<typename SystemType>
inline void SystemManager::RegisterSystem()
{
	systems.push_back(std::unique_ptr<ISystem>((ISystem*)new SystemType()));
}


