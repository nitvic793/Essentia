#pragma once

#include "Mesh.h"

struct TerrainData
{
	using byte = unsigned char;
	MeshData			TerrainMeshData;
	MeshHandle			TerrainMeshHandle;
	std::vector<byte>	HeightMapData;
	uint32				Width;
	uint32				Height;
};

class TerrainManager
{
public:
	static constexpr float STARTX = 0.5f;
	static constexpr float STARTZ = -0.5f;
	MeshHandle	CreateTerrainMesh(const char* heightMapFile, float minY = -0.1f, float maxY = 0.1f);
	void		UpdateTerrainMesh(const char* terrainName, float scaleMinY, float scaleMaxY);
private:
	std::unordered_map<std::string, TerrainData> terrains;
};

