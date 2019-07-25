#pragma once
#include "EntityBase.h"
#include "Mesh.h"
#include "Material.h"

struct Renderable
{
	GComponent(Renderable)
	MeshView Mesh;
	Material Material;
};