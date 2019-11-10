#pragma once

// General Use Case PSOs
#include "ResourceManager.h"

struct PipelineStates
{
	void Initialize();
	PipelineStateID DepthOnlyPSO;

	//Post Process PSOs
	PipelineStateID QuadPSO;
	PipelineStateID HDRQuadPSO;
	PipelineStateID BlurPSO;
};

extern PipelineStates GPipelineStates;