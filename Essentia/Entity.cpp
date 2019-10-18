#include "pch.h"
#include "Entity.h"

EntityManager::EntityManager()
{
	freeIndices.reserve(CMaxInitialEntityCount);
	generations.reserve(CMaxInitialEntityCount);
}

EntityHandle EntityManager::CreateEntity(const Transform& transform)
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

	handle.Version = generations[handle.Index];
	transformManager.CreateTransform({ handle });
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

TransformRef EntityManager::GetTransform(EntityHandle handle)
{
	auto pos = componentManager.GetComponent<PositionComponent>(handle);
	auto rot = componentManager.GetComponent<RotationComponent>(handle);
	auto scale = componentManager.GetComponent<ScaleComponent>(handle);

	TransformRef ref = {};
	ref.Position = (DirectX::XMFLOAT3*)pos;
	ref.Rotation = (DirectX::XMFLOAT3*)rot;
	ref.Scale = (DirectX::XMFLOAT3*)scale;
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

void EntityManager::GetWorldMatrices(EntityHandle* entities, uint32 count, std::vector<DirectX::XMFLOAT4X4>& matrices)
{
	for (uint32 i = 0; i < count; ++i)
	{
		matrices.push_back(transformManager.GetWorldMatrix(entities[i]));
	}
}
