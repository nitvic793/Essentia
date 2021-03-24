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

ComponentPoolGeneric* ComponentManager::GetOrCreatePool(std::string_view name, size_t size)
{
	auto type = crc32(name);
	if (pools.find(type) == pools.end())
	{
		size_t size = sizeof(ComponentPoolGeneric);
		auto buffer = allocator->Alloc(size);
		ComponentPoolGeneric* pool = new(buffer) ComponentPoolGeneric(name, size);
		poolStringMap[name.data()] = pool;
		pools.insert(
			std::pair<ComponentTypeID,
			ScopedPtr<ComponentPoolBase>>(
				type,
				ScopedPtr<ComponentPoolBase>((ComponentPoolBase*)pool)
				)

		);
	}
	return (ComponentPoolGeneric*)pools[type].get();
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
	pool->EmitAddComponentEvent(entity);
}


bool operator<(EntityHandle lhs, EntityHandle rhs)
{
	return lhs.ID < rhs.ID;
}

bool operator<=(EntityHandle lhs, EntityHandle rhs)
{
	return lhs.ID <= rhs.ID;
}

ComponentPoolGeneric::ComponentPoolGeneric(std::string_view name, size_t size) :
	CComponentSize(size), componentName(name)
{
}

ComponentTypeID ComponentPoolGeneric::GetType()
{
	return crc32(componentName);
}

void ComponentPoolGeneric::AddComponent(EntityHandle entity)
{
}

void ComponentPoolGeneric::RemoveComponent(EntityHandle entity)
{
}

IComponent* ComponentPoolGeneric::GetComponent(EntityHandle entity)
{
	return nullptr;
}

IComponent* ComponentPoolGeneric::GetAllComponents(uint32& count)
{
	return nullptr;
}

EntityHandle* ComponentPoolGeneric::GetEntities(uint32& count)
{
	return nullptr;
}

EntityHandle ComponentPoolGeneric::GetEntity(uint32 index)
{
	return EntityHandle();
}

bool ComponentPoolGeneric::HasEntity(EntityHandle entity)
{
	return false;
}

void ComponentPoolGeneric::Serialize(cereal::JSONOutputArchive& archive, EntityHandle entity)
{
}

void ComponentPoolGeneric::Deserialize(cereal::JSONInputArchive& archive, EntityHandle entity)
{
}

const size_t ComponentPoolGeneric::GetTypeSize()
{
	return CComponentSize;
}

const char* ComponentPoolGeneric::GetTypeName()
{
	return componentName.data();
}
