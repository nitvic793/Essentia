#include "pch.h"
#include "Terrain.h"
#include "ImageLoader.h"

MeshHandle TerrainManager::CreateTerrainMesh(const char* heightMapFile)
{
	uint32 width;
	uint32 height;
	std::vector<unsigned char> imageData = es::image::LoadPngImage(heightMapFile, width, height);

	return MeshHandle();
}
