#pragma once

// General Use Case PSOs
#include "ResourceManager.h"

struct PipelineStates
{
	void Initialize();

	PipelineStateID QuadPSO;
	PipelineStateID HDRQuadPSO;
};

extern PipelineStates GPipelineStates;