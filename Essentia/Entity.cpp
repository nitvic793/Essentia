#include "pch.h"
#include "Entity.h"
#include "Memory.h"
#include "Utility.h"

using namespace DirectX;

EntityManager::EntityManager()
{
	freeIndices.reserve(CMaxInitialEntityCount);
	generations.reserve(CMaxInitialEntityCount);
	entityNames.resize(CMaxInitialEntityCount);
}

void EntityManager::Initialize(IAllocator* allocator)
{
	this->allocator = allocator;
	componentManager.Initialize(allocator, EngineContext::Context);
}

//EntityHandle EntityManager::CreateEntity(const Transform& transform, EntityHandle parent)
//{
//	return CreateEntity(transform, parent.Handle.Index);
//}

EntityHandle EntityManager::CreateEntity(const Transform& transform, uint32 parentIndex, std::string_view entityName)
{
	HandleType handle;
	if (freeIndices.size() > 0)
	{
		handle.Index = freeIndices.back();
		freeIndices.pop_back();
	}
	else
	{
		generations.push_back(0);
		handle.Index = (uint32)(generations.size() - 1);
	}

	std::string name;
	if (entityName == "")
	{
		std::stringstream ss;
		ss << "Entity ";
		ss << std::to_string(handle.Index);
		name = ss.str();
	}
	else
	{
		name = entityName;
	}

	entityNames[handle.Index] = name;
	
	handle.Version = generations[handle.Index];
	TransformHandle parentTransform = { -1 };

	if (parentIndex != CRootParentEntityIndex)
	{
		parentTransform.Index = (int)parentIndex;
	}

	transformManager.CreateTransform({ handle }, parentTransform);
	EntityHandle entity = { handle };
	PositionComponent position = {};
	RotationComponent rotation = {};
	ScaleComponent scale = {};
	position = transform.Position;
	rotation = transform.Rotation;
	scale = transform.Scale;

	componentManager.AddComponent<PositionComponent>(entity, position);
	componentManager.AddComponent<RotationComponent>(entity, rotation);
	componentManager.AddComponent<ScaleComponent>(entity, scale);
	entityNameMap[name] = entity;

	return entity;
}

bool EntityManager::IsAlive(EntityHandle handle)
{
	return (handle.Handle.Version == generations[handle.Handle.Index]);
}

void EntityManager::Destroy(EntityHandle handle)
{
	auto index = handle.Handle.Index;
	generations[index]++;
	freeIndices.push_back(index);
}

bool EntityManager::HasValidParent(EntityHandle handle)
{
	return transformManager.HasValidParent(handle);
}

void EntityManager::Reset()
{
	generations.clear();
	entityNameMap.clear();
	entityNames.clear();
	freeIndices.clear();
	componentManager.Reset();
	transformManager.Reset();
}

ComponentManager* EntityManager::GetComponentManager()
{
	return &componentManager;
}

EntityHandle EntityManager::GetParent(EntityHandle entity)
{
	return transformManager.GetParent(entity);
}

Vector<EntityHandle> EntityManager::GetChildren(EntityHandle entity)
{
	return transformManager.GetChildren(entity);
}

std::string_view EntityManager::GetEntityName(EntityHandle entity)
{
	return entityNames[entity.Handle.Index];
}

EntityHandle EntityManager::GetEntityByName(std::string_view name)
{
	return entityNameMap[name.data()];
}

Vector<IComponent*> EntityManager::GetEntityComponents(EntityHandle handle)
{
	return componentManager.GetComponents(handle);
}

Vector<ComponentData> EntityManager::GetComponents(EntityHandle handle)
{
	return componentManager.GetEntityComponents(handle);
}

TransformRef EntityManager::GetTransform(EntityHandle handle)
{
	auto pos = componentManager.GetComponent<PositionComponent>(handle);
	auto rot = componentManager.GetComponent<RotationComponent>(handle);
	auto scale = componentManager.GetComponent<ScaleComponent>(handle);

	TransformRef ref = {};
	ref.Position = (DirectX::XMFLOAT3*) & pos->X;
	ref.Rotation = (DirectX::XMFLOAT4*) & rot->X;
	ref.Scale = (DirectX::XMFLOAT3*) & scale->X;
	return ref;
}

void EntityManager::UpdateTransform(EntityHandle entity, const Transform& transform)
{
	transformManager.SetLocal(entity, transform);
}

void EntityManager::GetTransposedWorldMatrices(EntityHandle* entities, uint32 count, std::vector<DirectX::XMFLOAT4X4>& matrices)
{
	for (uint32 i = 0; i < count; ++i)
	{
		matrices.push_back(transformManager.GetTransposedWorldMatrix(entities[i]));
	}
}

Vector<DirectX::XMFLOAT4X4> EntityManager::GetTransposedWorldMatrices(EntityHandle* entities, uint32 count)
{
	Vector<XMFLOAT4X4> matrices(count, Mem::GetFrameAllocator());
	for (uint32 i = 0; i < count; ++i)
	{
		matrices.Push(transformManager.GetTransposedWorldMatrix(entities[i]));
	}
	return matrices;
}

void EntityManager::GetWorldMatrices(EntityHandle* entities, uint32 count, std::vector<DirectX::XMFLOAT4X4>& matrices)
{
	for (uint32 i = 0; i < count; ++i)
	{
		matrices.push_back(transformManager.GetWorldMatrix(entities[i]));
	}
}

void EntityManager::SetWorldMatrix(EntityHandle entity, const DirectX::XMFLOAT4X4& world)
{
	transformManager.SetWorldMatrix(entity, world);
}

const DirectX::XMFLOAT4X4 EntityManager::GetWorldMatrix(EntityHandle entity)
{
	return transformManager.GetWorldMatrix(entity);
}

const DirectX::XMFLOAT4X4 EntityManager::GetLocalMatrix(EntityHandle entity)
{
	return transformManager.GetLocalMatrix(entity);
}
