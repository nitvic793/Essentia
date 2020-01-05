#pragma once

// General Use Case PSOs
#include "ResourceManager.h"

struct PipelineStates
{
public:
	void Initialize();
	PipelineStateID DepthOnlyPSO;
	PipelineStateID	ShadowDirPSO; // Directional Light Shadow Map PSO

	//Post Process PSOs
	PipelineStateID QuadPSO;
	PipelineStateID HDRQuadPSO;
	PipelineStateID BlurPSO;
private:
	void CreateShadowPSO();
};

extern PipelineStates GPipelineStates;