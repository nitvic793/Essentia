#include "pch.h"
#include "Component.h"

void ComponentManager::Initialize(IAllocator* allocator)
{
	this->allocator = allocator;
}

ComponentPoolBase* ComponentManager::GetPool(const char* componentName)
{
	return poolStringMap[componentName];
}

Vector<IComponent*> ComponentManager::GetComponents(EntityHandle handle)
{
	Vector<IComponent*> components((uint32)pools.size());
	for (auto& pool : pools)
	{
		const auto& poolBase = pool.second;
		if (poolBase->HasEntity(handle))
		{
			components.Push(poolBase->GetComponent(handle));
		}
	}
	return components;
}
