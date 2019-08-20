#pragma once

#include <assimp/Importer.hpp>      // C++ importer interface
#include <assimp/scene.h>           // Output data structure
#include <assimp/postprocess.h>     // Post processing flags
#include "Mesh.h"

struct MeshMaterial
{
	std::string Diffuse;
	std::string Normal;
	std::string Roughness;
	std::string Metalness;
};

struct ModelData
{
	MeshData					MeshData;
	std::vector<MeshMaterial>	Materials;
};

class ModelLoader
{
public:
	static MeshData Load(const std::string& filename);
	static ModelData LoadModel(const std::string& filename);
};

