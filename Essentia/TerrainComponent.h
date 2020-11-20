#pragma once

#include "EntityBase.h"
#include "ConstantBufferViewPool.h"
#include "Mesh.h"
#include "Engine.h"
#include "CMath.h"
#include "Terrain.h"

struct TerrainComponent : public IComponent
{
	float			PrevMaxY;
	float			PrevMinY;
	float			ScaleMinY;
	float			ScaleMaxY;
	std::string		TerrainName;

	// Since we have separate pipeline to render terrain, need to duplicate this to keep it simple.
	MeshHandle			TerrainMesh;
	MaterialHandle		Material;
	PerObjectConstantBuffer ConstantBuffer;
	ConstantBufferView ConstantBufferView;

	static TerrainComponent Create(const char* heightMap, MaterialHandle material, float scaleMinY = -10.f, float scaleMaxY = 10.f)
	{
		TerrainManager* terrainManager = GContext->TerrainManager;
		TerrainComponent component = {};
		component.TerrainName = heightMap;
		component.TerrainMesh = terrainManager->CreateTerrainMesh(heightMap, scaleMinY, scaleMaxY);
		component.ScaleMaxY = scaleMaxY;
		component.ScaleMinY = scaleMinY;
		component.ConstantBufferView = GContext->ConstantBufferViewPool->RequestConstantBufferView<PerObjectConstantBuffer>();
		return component;
	}

	template<class Archive>
	void save(Archive& archive) const
	{
		auto MaterialName = GContext->ShaderResourceManager->GetMaterialName(Material);
		archive(
			CEREAL_NVP(ScaleMinY),
			CEREAL_NVP(ScaleMaxY),
			CEREAL_NVP(TerrainName),
			CEREAL_NVP(MaterialName)
		);
	};

	template<class Archive>
	void load(Archive& archive)
	{
		std::string MaterialName;
		archive(
			CEREAL_NVP(ScaleMinY),
			CEREAL_NVP(ScaleMaxY),
			CEREAL_NVP(TerrainName),
			CEREAL_NVP(MaterialName)
		);

		auto materialHandle = GContext->ShaderResourceManager->GetMaterialHandle(MaterialName.c_str());
		*this = Create(TerrainName.c_str(), materialHandle, ScaleMinY, ScaleMaxY);
	};

	GComponent(TerrainComponent);
};