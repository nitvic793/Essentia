#pragma once
#include "Timer.h"
#include "Entity.h"

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
	T* GetComponents(uint32& outCount);
private:
	EntityManager* entityManager;
	friend class SystemManager;
};

template<typename T>
inline T* ISystem::GetComponents(uint32& outCount)
{
	return entityManager->GetComponents<T>(outCount);
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


