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

std::vector<IComponent*> ComponentManager::GetComponents(EntityHandle handle)
{
	std::vector<IComponent*> components;
	for (auto& pool : pools)
	{
		const auto& poolBase = pool.second;
		if (poolBase->HasEntity(handle))
		{
			components.push_back(poolBase->GetComponent(handle));
		}
	}
	return components;
}
