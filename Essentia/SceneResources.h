#pragma once

#include "ConstantBuffer.h"
#include "Declarations.h"

struct SceneResources
{
	void					Initalize();
	PipelineStateID			CurrentPSO;
	SceneRenderTarget		VelocityBuffer;
	SceneRenderTarget		PreviousFrame;
	SceneRenderTarget		AmbientOcclusion;
	SceneRenderTarget		LightAccumTarget;
	DepthTarget				DepthPrePass;
	DepthTarget				ShadowDepthTarget;
	ConstantBufferView		ShadowCBV;
	ConstantBufferView		LightBufferCBV;
	TextureID				NoiseTexture;
	SceneRenderTarget		WorldPosTexture;
	TextureID				VoxelGridSRV;
	ResourceID				VoxelGridResource;
	PerFrameConstantBuffer	FrameData;
	ConstantBufferView		FrameDataCBV;
};

extern SceneResources GSceneResources;