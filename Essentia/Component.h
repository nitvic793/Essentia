#pragma once

#include "Declarations.h"
#include "StringHash.h"
#include <vector>
#include <unordered_map>
#include "EntityBase.h"


struct TestComponent : public IComponent
{
	float test;
	GComponent(TestComponent)
};

struct SpeedComponent : public IComponent
{
	GComponent(SpeedComponent)
		float Speed;
};

class ComponentPoolBase
{
public:
	virtual ComponentTypeID GetType() = 0;
	virtual void			AddComponent(EntityHandle entity) = 0;
	virtual void			RemoveComponent(EntityHandle entity) = 0;
	virtual IComponent*		GetComponent(EntityHandle entity) = 0;
	virtual IComponent*		GetAllComponents(uint32& count) = 0;
	virtual EntityHandle*	GetEntities(uint32& count) = 0;
	virtual EntityHandle	GetEntity(uint32 index) = 0;
	virtual ~ComponentPoolBase() {}
};

template<typename T>
class ComponentPool : public ComponentPoolBase
{
public:
	ComponentPool()
	{
		components.reserve(CMaxInitialComponentCount);
		entities.reserve(CMaxInitialComponentCount);
	}

	virtual void AddComponent(EntityHandle entity) override
	{
		if (componentMap.find(entity.ID) == componentMap.end())
		{
			T val = {};
			entities.push_back(entity.ID);
			components.push_back(val);
			componentMap[entity.ID] = (uint32)components.size() - 1;
		}
	}

	virtual void RemoveComponent(EntityHandle entity) override
	{
		auto index = componentMap[entity.ID];
		componentMap.erase(entity.ID);
		auto lastIndex = components.size() - 1;
		components[index] = components[lastIndex];
		entities[index] = entities[lastIndex];
		components.pop_back();
		entities.pop_back();

		for (auto e : componentMap)
		{
			if (e.second == lastIndex)
			{
				componentMap[e.first] = index;
				break;
			}
		}
	}

	virtual IComponent* GetComponent(EntityHandle entity) override
	{
		auto index = componentMap[entity.ID];
		return (IComponent*)& components[index];
	}

	virtual IComponent* GetAllComponents(uint32& count) override
	{
		count = (uint32)components.size();
		return (IComponent*)components.data();
	}

	virtual EntityHandle* GetEntities(uint32& count) override
	{
		count = (uint32)entities.size();
		return (EntityHandle*)entities.data();
	}

	virtual EntityHandle GetEntity(uint32 index) override
	{
		EntityHandle h;
		h.ID = entities[index];
		return h;
	}

	virtual ComponentTypeID GetType() override
	{
		return T::Type;
	}

	~ComponentPool() {}
private:
	std::vector<T>						components;
	std::vector<Handle>					entities;
	std::unordered_map<Handle, uint32>	componentMap;
};

class ComponentManager
{
public:
	template<typename T>
	ComponentPool<T>* GetOrCreatePool();

	template<typename T>
	void AddComponent(EntityHandle entity, const T& value = T());

	template<typename T>
	void RemoveComponent(EntityHandle entity);

	template<typename T>
	T* GetComponent(EntityHandle entity);

	template<typename T>
	T* GetAllComponents(uint32& count);

	template<typename T>
	EntityHandle* GetEntities(uint32& count);

	template<typename T>
	EntityHandle GetEntity(uint32 index);
private:
	std::unordered_map<ComponentTypeID, std::unique_ptr<ComponentPoolBase>> pools;
};

template<typename T>
inline ComponentPool<T>* ComponentManager::GetOrCreatePool()
{
	if (pools.find(T::Type) == pools.end())
	{
		pools.insert(std::pair<ComponentTypeID, std::unique_ptr<ComponentPoolBase>>(T::Type, std::unique_ptr<ComponentPoolBase>((ComponentPoolBase*)new ComponentPool<T>())));
	}
	return (ComponentPool<T>*)pools[T::Type].get();
}

template<typename T>
inline void ComponentManager::AddComponent(EntityHandle entity, const T& value)
{
	auto pool = GetOrCreatePool<T>();
	pool->AddComponent(entity);
	T* component = (T*)pool->GetComponent(entity);
	memcpy(component, &value, sizeof(T));
}

template<typename T>
inline void ComponentManager::RemoveComponent(EntityHandle entity)
{
	pools[T::Type]->RemoveComponent(entity);
}

template<typename T>
inline T* ComponentManager::GetComponent(EntityHandle entity)
{
	return (T*)pools[T::Type]->GetComponent(entity);
}

template<typename T>
inline T* ComponentManager::GetAllComponents(uint32& count)
{
	return (T*)pools[T::Type]->GetAllComponents(count);
}

template<typename T>
inline EntityHandle* ComponentManager::GetEntities(uint32& count)
{
	return pools[T::Type]->GetEntities(count);
}

template<typename T>
inline EntityHandle ComponentManager::GetEntity(uint32 index)
{
	return pools[T::Type]->GetEntity(index);
}
