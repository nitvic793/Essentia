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

struct MeshMaterial
{
	std::string Diffuse;
	std::string Normal;
	std::string Roughness;
	std::string Metalness;
};

struct ModelData
{
	std::vector<MeshData>		Meshes;
	std::vector<MeshMaterial>	Materials;
};

class ModelLoader
{
public:
	static MeshData Load(const std::string& filename);
	static ModelData LoadModel(const std::string& filename);
};

