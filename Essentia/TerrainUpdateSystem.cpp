#include "TerrainUpdateSystem.h"
#include "TerrainComponent.h"
#include "Entity.h"
#include "EngineContext.h"

void TerrainUpdateSystem::Initialize()
{
}

void TerrainUpdateSystem::Update(float dt, float totalTime)
{
	EntityManager* entityManager = GContext->EntityManager;
	TerrainManager* terrainManager = GContext->TerrainManager;

	uint32 count = 0;
	auto terrains = entityManager->GetComponents<TerrainComponent>(count);

	for (uint32 i = 0; i < count; ++i)
	{
		if (terrains[i].PrevMaxY != terrains[i].ScaleMaxY || terrains[i].PrevMinY != terrains[i].ScaleMinY)
		{
			terrainManager->UpdateTerrainMesh(terrains[i].TerrainName.c_str(), terrains[i].ScaleMinY, terrains[i].ScaleMaxY);
		}

		terrains[i].PrevMaxY = terrains[i].ScaleMaxY;
		terrains[i].PrevMinY = terrains[i].ScaleMinY;
	}
}
