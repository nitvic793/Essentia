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

void ComponentManager::RemoveComponent(std::string_view componentName, EntityHandle handle)
{
	poolStringMap[componentName]->RemoveComponent(handle);
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

Vector<ComponentData> ComponentManager::GetEntityComponents(EntityHandle handle)
{
	Vector<ComponentData> components((uint32)pools.size(), Mem::GetFrameAllocator());
	for (auto& pool : pools)
	{
		const auto& poolBase = pool.second;
		if (poolBase->HasEntity(handle))
		{
			components.Push({
					poolBase->GetTypeName(),
					poolBase->GetComponent(handle)
				});
		}
	}
	return components;
}

Vector<const char*> ComponentManager::GetComponentNameList()
{
	Vector<const char*> list((uint32)pools.size(), Mem::GetFrameAllocator());
	for (auto& pool : poolStringMap)
	{
		list.Push(pool.first.data());
	}

	return list;
}

void ComponentManager::AddComponent(const char* name, EntityHandle entity, IComponent* initValue)
{
	auto pool = GetPool(name);
	pool->AddComponent(entity);
	if (initValue)
	{
		auto component = pool->GetComponent(entity);
		memcpy(component, initValue, pool->GetTypeSize());
	}

}


bool operator<(EntityHandle lhs, EntityHandle rhs)
{
	return lhs.ID < rhs.ID;
}

bool operator<=(EntityHandle lhs, EntityHandle rhs)
{
	return lhs.ID <= rhs.ID;
}