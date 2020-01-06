#pragma once
#include "Declarations.h"
#include <vector>
#include "Component.h"
#include "EntityBase.h"
#include "Transform.h"
#include "BaseComponents.h"

constexpr EntityHandle Unknown = { {INT32_MAX} };

class EntityManager
{
public:
	EntityManager();
	void				Initialize(IAllocator* allocator);
	EntityHandle		CreateEntity(const Transform& transform = DefaultTransform, EntityHandle parent = Unknown);
	bool				IsAlive(EntityHandle handle);
	void				Destroy(EntityHandle handle);
	ComponentManager* GetComponentManager();

	template<typename T>
	void			AddComponent(EntityHandle handle, const T& value = T());

	template<typename T>
	T* GetComponent(EntityHandle handle);

	template<typename T>
	T* GetComponents(uint32& count);

	template<typename T>
	EntityHandle* GetEntities(uint32& count);

	Vector<IComponent*>	GetEntityComponents(EntityHandle handle);
	TransformRef				GetTransform(EntityHandle handle);
	void						UpdateTransform(EntityHandle entity, const Transform& transform);
	void						GetTransposedWorldMatrices(EntityHandle* entities, uint32 count, std::vector<DirectX::XMFLOAT4X4>& matrices);
	Vector<DirectX::XMFLOAT4X4>	GetTransposedWorldMatrices(EntityHandle* entities, uint32 count);
	void						GetWorldMatrices(EntityHandle* entities, uint32 count, std::vector<DirectX::XMFLOAT4X4>& matrices);
private:
	std::vector<uint32> generations;
	std::vector<uint32> freeIndices;
	ComponentManager	componentManager;
	TransformManager	transformManager;
	IAllocator* allocator;
};

template<typename T>
inline void EntityManager::AddComponent(EntityHandle handle, const T& value)
{
	componentManager.AddComponent<T>(handle, value);
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

template<typename T>
inline EntityHandle* EntityManager::GetEntities(uint32& count)
{
	return componentManager.GetEntities<T>(count);
}
