#pragma once

// General Use Case PSOs
#include "ResourceManager.h"

struct PipelineStates
{
public:
	void Initialize();
	PipelineStateID	DefaultPSO;
	PipelineStateID	DefaultAnimatedPSO;
	PipelineStateID DefaultNoAOPSO;
	PipelineStateID DepthOnlyPSO;
	PipelineStateID	ShadowDirPSO; // Directional Light Shadow Map PSO
	PipelineStateID ScreenSpaceAOPSO;
	PipelineStateID SSAOBlurPSO;
	PipelineStateID LightAccumPSO;
	PipelineStateID BilateralBlurPSO;
	PipelineStateID VoxelizePSO;
	PipelineStateID	WireframePSO;

	//Post Process PSOs
	PipelineStateID QuadPSO;
	PipelineStateID HDRQuadPSO;
	PipelineStateID BlurPSO;
	PipelineStateID ApplyFogPSO;

	//Compute PSOs
	PipelineStateID MipGen3DCSPSO;
	PipelineStateID VoxelCopyPSO;
private:
	void CreateDefaultPSOs();
	void CreateLightAccumPSO();
	void CreateShadowPSO();
	void CreateScreenSpaceAOPSO();
	void CreateVoxelizePSO();
	void CreateMipGen3DComputePSO();
	void CreateVoxelCopyComputePSO();
	void CreateWireframePSO();
};

extern PipelineStates GPipelineStates;