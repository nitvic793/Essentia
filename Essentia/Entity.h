#pragma once
#include "Declarations.h"
#include <vector>
#include "Component.h"
#include "EntityBase.h"


class EntityManager
{
public:
	EntityManager();
	EntityHandle	CreateEntity();
	bool			IsAlive(EntityHandle handle);
	void			Destroy(EntityHandle handle);
	
	template<typename T>
	void			AddComponent(EntityHandle handle);

	template<typename T>
	T*				GetComponent(EntityHandle handle);

	template<typename T>
	T*				GetComponents(uint32& count);
private:
	std::vector<uint32> generations;
	std::vector<uint32> freeIndices;
	ComponentManager	componentManager;
};

template<typename T>
inline void EntityManager::AddComponent(EntityHandle handle)
{
	componentManager.AddComponent<T>(handle);
}

template<typename T>
inline T* EntityManager::GetComponent(EntityHandle handle)
{
	return componentManager.GetComponent<T>(handle);
}

template<typename T>
inline T* EntityManager::GetComponents(uint32& count)
{
	return componentManager.GetAllComponents<T>(count);
}
