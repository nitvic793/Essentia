#include "pch.h"
#include "Entity.h"

EntityManager::EntityManager()
{
	freeIndices.reserve(CMaxInitialEntityCount);
	generations.reserve(CMaxInitialEntityCount);
}

EntityHandle EntityManager::CreateEntity()
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
	return { handle };
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
