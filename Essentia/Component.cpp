#include "pch.h"
#include "Component.h"
#include "Serialization.h"
#include "ComponentReflector.h"

void ComponentManager::Initialize(IAllocator* allocator, EngineContext* context)
{
	this->context = context;
	this->allocator = allocator;
}

ComponentPoolBase* ComponentManager::GetPool(const char* componentName)
{
	if (poolStringMap.find(componentName) == poolStringMap.end())
	{
		GComponentReflector.CreatePool(componentName, this);
	}

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


bool operator<(EntityHandle lhs, EntityHandle rhs)
{
	return lhs.ID < rhs.ID;
}

bool operator<=(EntityHandle lhs, EntityHandle rhs)
{
	return lhs.ID <= rhs.ID;
}