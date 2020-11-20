#pragma once

#include "EntityBase.h"
#include "ConstantBuffer.h"
#include "Mesh.h"
#include "Engine.h"
#include "CMath.h"
#include "Terrain.h"

struct TerrainComponent : public IComponent
{
	float		PrevMaxY;
	float		PrevMinY;
	float		ScaleMinY;
	float		ScaleMaxY;
	std::string TerrainName;
	MeshHandle	TerrainMesh;

	static TerrainComponent Create(const char* heightMap, float scaleMinY = -10.f, float scaleMaxY = 10.f)
	{
		TerrainManager* terrainManager = GContext->TerrainManager;
		TerrainComponent component = {};
		component.TerrainName = heightMap;
		component.TerrainMesh = terrainManager->CreateTerrainMesh(heightMap, scaleMinY, scaleMaxY);
		component.ScaleMaxY = scaleMaxY;
		component.ScaleMinY = scaleMinY;
		return component;
	}

	template<class Archive>
	void save(Archive& archive) const
	{
		archive(
			CEREAL_NVP(ScaleMinY),
			CEREAL_NVP(ScaleMaxY),
			CEREAL_NVP(TerrainName)
		);
	};

	template<class Archive>
	void load(Archive& archive)
	{
		archive(
			CEREAL_NVP(ScaleMinY),
			CEREAL_NVP(ScaleMaxY),
			CEREAL_NVP(TerrainName)
		);

		*this = Create(TerrainName.c_str(), ScaleMinY, ScaleMaxY);
	};

	GComponent(TerrainComponent);
};