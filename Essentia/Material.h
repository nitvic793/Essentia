#pragma once
#include "Declarations.h"
#include "cereal/cereal.hpp"

struct Material
{
	TextureID		StartIndex;
	PipelineStateID PipelineID;
	uint32			TextureCount;
};

struct MaterialHandle
{
	uint32 Index;

	template<class Archive> 
	void serialize(Archive& a)
	{
		a(CEREAL_NVP(Index));
	};
};

enum MaterialTextureType
{
	DiffuseID = 0,
	NormalsID = 1,
	RoughnessID = 2,
	MetalnessID = 3,
	MaterialTextureCount = 4
};

struct MaterialData
{
	TextureID*	Textures;
	uint32		TextureCount;
};