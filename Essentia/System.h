#pragma once
#include "Timer.h"
#include "Entity.h"
#include "Keyboard.h"
#include "Mouse.h"
#include "Camera.h"
#include "StringHash.h"

class EntityManager;

class ISystem
{
public:
	ISystem() {};
	ISystem(const char* sysName) :
		systemName(sysName) {};
	virtual void Initialize() {};
	virtual void Update(float deltaTime, float totalTime) {};
	virtual void Destroy() {};
	virtual void Reset() {};
	virtual ~ISystem() {};

	const char* GetName() const;
protected:
	template<typename T>
	T*							GetComponents(uint32& outCount);

	template<typename T>
	EntityHandle*				GetEntities(uint32& count);

	TransformRef				GetTransform(EntityHandle entity);

	Vector<IComponent*>			GetEntityComponents(EntityHandle handle);

	EntityManager*				entityManager;
	DirectX::Keyboard::State	keyboard;
	DirectX::Mouse::State		mouse;
	std::string					systemName;
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

	template<typename SystemType>
	void RegisterSystem(IAllocator* allocator);

	void Initialize();
	void Update(const DirectX::Keyboard::State& kbState, const DirectX::Mouse::State& mouseState, Camera* camera);
	void Destroy();
	void Reset();
	const std::vector<ScopedPtr<ISystem>> &GetSystems();
private:
	std::vector<ScopedPtr<ISystem>> systems;
	EntityManager* entityManager = nullptr;
};

template<typename SystemType>
inline void SystemManager::RegisterSystem()
{
	std::string_view typeName = TypeName<SystemType>();
	ISystem* system = (ISystem*)Mem::Alloc<SystemType>();
	system->systemName = TypeName<SystemType>();
	system->systemName.erase(0, sizeof("class"));
	systems.push_back(ScopedPtr<ISystem>(system));
}

template<typename SystemType>
inline void SystemManager::RegisterSystem(IAllocator* allocator)
{
	auto buffer = allocator->Alloc(sizeof(SystemType));
	ISystem* system = new(buffer) SystemType();
	system->systemName = TypeName<SystemType>();
	system->systemName.erase(0, sizeof("class"));
	systems.push_back(ScopedPtr<ISystem>(system));
}


