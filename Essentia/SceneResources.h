#pragma once

#include "Declarations.h"

struct SceneResources
{
	void				Initalize();
	PipelineStateID		CurrentPSO;
	SceneRenderTarget	VelocityBuffer;
	SceneRenderTarget	PreviousFrame;
	SceneRenderTarget	AmbientOcclusion;
	SceneRenderTarget	LightAccumTarget;
	DepthTarget			DepthPrePass;
	DepthTarget			ShadowDepthTarget;
	ConstantBufferView	ShadowCBV;
	ConstantBufferView	LightBufferCBV;
	TextureID			NoiseTexture;
};

extern SceneResources GSceneResources;