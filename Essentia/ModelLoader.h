#pragma once

#include <assimp/Importer.hpp>      // C++ importer interface
#include <assimp/scene.h>           // Output data structure
#include <assimp/postprocess.h>     // Post processing flags
#include "Mesh.h"

struct MeshEntry
{
	int NumIndices;
	int BaseVertex;
	int BaseIndex;
};

class ModelLoader
{
public:
	static MeshData Load(const std::string& filename);
};

