#pragma once

#include "Mesh.h"

class TerrainManager
{
public:
	static constexpr float STARTX = 0.5f;
	static constexpr float STARTZ = -0.5f;
	MeshHandle CreateTerrainMesh(const char* heightMapFile, float minY = -0.1f, float maxY = 0.1f);
private:

};

