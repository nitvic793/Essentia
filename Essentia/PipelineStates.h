#pragma once

// General Use Case PSOs
#include "ResourceManager.h"

struct PipelineStates
{
public:
	void Initialize();
	PipelineStateID DepthOnlyPSO;
	PipelineStateID	ShadowDirPSO; // Directional Light Shadow Map PSO
	PipelineStateID ScreenSpaceAOPSO;

	//Post Process PSOs
	PipelineStateID QuadPSO;
	PipelineStateID HDRQuadPSO;
	PipelineStateID BlurPSO;
private:
	void CreateShadowPSO();
	void CreateScreenSpaceAOPSO();
};

extern PipelineStates GPipelineStates;