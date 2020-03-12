#pragma once

// General Use Case PSOs
#include "ResourceManager.h"

struct PipelineStates
{
public:
	void Initialize();
	PipelineStateID	DefaultPSO;
	PipelineStateID DefaultNoAOPSO;
	PipelineStateID DepthOnlyPSO;
	PipelineStateID	ShadowDirPSO; // Directional Light Shadow Map PSO
	PipelineStateID ScreenSpaceAOPSO;
	PipelineStateID SSAOBlurPSO;
	PipelineStateID LightAccumPSO;

	//Post Process PSOs
	PipelineStateID QuadPSO;
	PipelineStateID HDRQuadPSO;
	PipelineStateID BlurPSO;
	PipelineStateID ApplyFogPSO;
private:
	void CreateDefaultPSOs();
	void CreateLightAccumPSO();
	void CreateShadowPSO();
	void CreateScreenSpaceAOPSO();
};

extern PipelineStates GPipelineStates;