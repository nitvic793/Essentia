#pragma once
#include "Declarations.h"

struct Material
{
	TextureID		StartIndex;
	PipelineStateID PipelineID;
	uint32			TextureCount;
};

struct MaterialHandle
{
	uint32 Index;
};

enum MaterialTextureType
{
	DiffuseID = 0,
	NormalsID = 1,
	RoughnessID = 2,
	MetalnessID = 3
};