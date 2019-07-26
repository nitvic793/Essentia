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