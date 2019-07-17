#pragma once
#include "Vertex.h"

struct MeshData
{
	std::vector<Vertex> Vertices;
	std::vector<uint32> Indices;
};

struct MeshBuffer
{
	//One large buffer or multiple small ones
	//Do multiple small ones for now
};

union Mesh
{
	uint32 id;
};

class MeshManager
{

};