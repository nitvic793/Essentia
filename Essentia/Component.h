#pragma once

#include "Declarations.h"
#include "StringHash.h"
#include <vector>
#include <unordered_map>
#include "EntityBase.h"
#include "Memory.h"
#include <cereal/archives/json.hpp>
#include "Utility.h"
#include "EngineContext.h"
#include "EventTypes.h"

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
	virtual bool			HasEntity(EntityHandle entity) = 0;
	virtual void			Serialize(cereal::JSONOutputArchive& archive, EntityHandle entity) = 0;
	virtual void			Deserialize(cereal::JSONInputArchive& archive, EntityHandle entity) = 0;
	virtual const size_t	GetTypeSize() = 0;
	virtual const char*		GetTypeName() = 0 ;
	virtual void			EmitAddComponentEvent(EntityHandle entity) {};
	virtual void			EmitRemoveComponentEvent(EntityHandle entity) {};
	virtual void			Reset() {};
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

	void AddComponent(EntityHandle entity, T& val)
	{
		if (componentMap.find(entity.ID) == componentMap.end())
		{
			entities.push_back(entity.ID);
			components.push_back(val);
			componentMap[entity.ID] = (uint32)components.size() - 1;
		}
		else
		{
			auto index = componentMap[entity.ID];
			components[index] = val;
		}
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
		if (componentMap.find(entity.ID) == componentMap.end())
		{
			return;
		}

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
		if (componentMap.find(entity.ID) == componentMap.end())
		{
			return nullptr;
		}

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

	virtual bool HasEntity(EntityHandle handle) override
	{
		if (componentMap.find(handle.ID) == componentMap.end())
		{
			return false;
		}

		return true;
	}

	virtual void Serialize(cereal::JSONOutputArchive& archive, EntityHandle entity) override
	{
		auto index = componentMap[entity.ID];
		T component = components[index];
		archive(cereal::make_nvp(T::ComponentName, component));
	}

	virtual void Deserialize(cereal::JSONInputArchive& archive, EntityHandle entity) override
	{
		T component;
		archive(cereal::make_nvp(T::ComponentName, component));
		AddComponent(entity, component);
	}

	virtual const size_t GetTypeSize() override
	{
		return sizeof(T);
	}

	virtual const char* GetTypeName() override
	{
		return T::ComponentName;
	}

	virtual void EmitAddComponentEvent(EntityHandle entity)
	{
		T* component = (T*)this->GetComponent(entity);
		ComponentAddEvent<T> event = {};
		event.component = component;
		event.entity = entity;
		es::GEventBus->Publish(&event);
	}

	virtual void Reset() override
	{
		components.clear();
		entities.clear();
		componentMap.clear();
	}

	~ComponentPool() {}
private:
	std::vector<T>						components;
	std::vector<Handle>					entities;
	std::unordered_map<Handle, uint32>	componentMap;
};

class ComponentPoolGeneric : public ComponentPoolBase
{
public:
	using BufferPtr = void*;

	ComponentPoolGeneric(std::string_view name, size_t size);

	// Inherited via ComponentPoolBase
	virtual ComponentTypeID GetType() override;
	virtual void AddComponent(EntityHandle entity) override;
	virtual void RemoveComponent(EntityHandle entity) override;
	virtual IComponent* GetComponent(EntityHandle entity) override;
	virtual IComponent* GetAllComponents(uint32& count) override;
	virtual EntityHandle* GetEntities(uint32& count) override;
	virtual EntityHandle GetEntity(uint32 index) override;
	virtual bool HasEntity(EntityHandle entity) override;
	virtual void Serialize(cereal::JSONOutputArchive& archive, EntityHandle entity) override;
	virtual void Deserialize(cereal::JSONInputArchive& archive, EntityHandle entity) override;
	virtual const size_t GetTypeSize() override;
	virtual const char* GetTypeName() override;
protected:
	const size_t CComponentSize;
	BufferPtr buffer;
	std::string_view componentName;
	uint64 currentOffset;
};

class ComponentManager
{
	template<typename T>
	void GetEntitiesInternal(std::function<void(EntityHandle*, uint32)> callback);
public:
	void Initialize(IAllocator* allocator, EngineContext* context);

	ComponentPoolBase* GetPool(const char* componentName);

	template<typename T>
	ComponentPool<T>* GetOrCreatePool();

	ComponentPoolGeneric* GetOrCreatePool(std::string_view name, size_t size);

	template<typename T>
	void AddComponent(EntityHandle entity, const T& value = T());

	template<typename T>
	void RemoveComponent(EntityHandle entity);

	void RemoveComponent(std::string_view componentName, EntityHandle handle);

	template<typename T>
	T* GetComponent(EntityHandle entity);

	/**
	 * Try get component associated with entity. Will create component pool if not found
	 * .
	 */
	template<typename T>
	T* TryGetComponent(EntityHandle entity);

	template<typename T>
	T* GetAllComponents(uint32& count);

	template<typename T>
	EntityHandle* GetEntities(uint32& count);

	template<typename ...Args>
	Vector<EntityHandle> GetEntities();

	template<typename T>
	EntityHandle GetEntity(uint32 index);

	Vector<IComponent*>		GetComponents(EntityHandle handle);
	Vector<ComponentData>	GetEntityComponents(EntityHandle handle);
	Vector<const char*>		GetComponentNameList();
	void					AddComponent(const char* name, EntityHandle entity, IComponent* initValue = nullptr);
	void					Reset();
private:
	IAllocator* allocator;
	EngineContext* context;
	std::unordered_map<ComponentTypeID, ScopedPtr<ComponentPoolBase>> pools;
	std::unordered_map<std::string_view, ComponentPoolBase*> poolStringMap;
};

template<typename T>
inline void ComponentManager::GetEntitiesInternal(std::function<void(EntityHandle*, uint32)> callback)
{
	uint32 count;
	auto entities = GetEntities<T>(count);
	callback(entities, count);
}

template<typename T>
inline ComponentPool<T>* ComponentManager::GetOrCreatePool()
{
	if (pools.find(T::Type) == pools.end())
	{
		size_t size = sizeof(ComponentPool<T>);
		auto buffer = allocator->Alloc(size);
		ComponentPool<T>* pool = new(buffer) ComponentPool<T>();
		poolStringMap[T::ComponentName] = pool;
		pools.insert(
			std::pair<ComponentTypeID,
			ScopedPtr<ComponentPoolBase>>(
				T::Type,
				ScopedPtr<ComponentPoolBase>((ComponentPoolBase*)pool)
				)

		);
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
inline T* ComponentManager::TryGetComponent(EntityHandle entity)
{
	ComponentPool<T>* pool = GetOrCreatePool<T>();
	return (T*)pool->GetComponent(entity);
}

template<typename T>
inline T* ComponentManager::GetAllComponents(uint32& count)
{
	auto pool = GetOrCreatePool<T>();
	return (T*)pool->GetAllComponents(count);
}

template<typename T>
inline EntityHandle* ComponentManager::GetEntities(uint32& count)
{
	auto pool = GetOrCreatePool<T>();
	return pool->GetEntities(count);
}


template<typename ...Args>
inline Vector<EntityHandle> ComponentManager::GetEntities()
{
	constexpr size_t argCount = sizeof...(Args);
	Vector<EntityHandle> commonEntities(context->FrameAllocator);
	Vector<EntityHandle> entityList(context->FrameAllocator);

	int a = 0;
	auto cb = [&](EntityHandle* entities, uint32 count)->void
	{
		entityList.Grow(count);
		entityList.CopyFrom(entities, count);
	};

	auto c = { 0, (GetEntitiesInternal<Args>(cb), 0) ... };
	entityList.Sort();

	uint64 prevId = INT_MAX;
	int count = 0;

	//Find common entities between with given components 
	for (auto& e : entityList)
	{
		if (e.ID == prevId)
		{
			count++;
		}
		else
		{
			count = 1;
			prevId = e.ID;
		}

		// Add entity to list if same entity was repeated in list same as sizeof...(Args)
		if (count == argCount) // If count of same entity repeated  == number of component args
		{
			commonEntities.Grow(1);
			commonEntities.Push(e);
		}
	}
	return commonEntities;
}

template<typename T>
inline EntityHandle ComponentManager::GetEntity(uint32 index)
{
	return pools[T::Type]->GetEntity(index);
}
