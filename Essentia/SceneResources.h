#pragma once

#include "Declarations.h"

struct SceneResources
{
	void				Initalize();
	PipelineStateID		CurrentPSO;
	SceneRenderTarget	VelocityBuffer;
	SceneRenderTarget	PreviousFrame;
	SceneRenderTarget	AmbientOcclusion;
	DepthTarget			DepthPrePass;
	DepthTarget			ShadowDepthTarget;
	ConstantBufferView	ShadowCBV;
};

extern SceneResources GSceneResources;