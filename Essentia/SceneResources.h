#pragma once

#include "Declarations.h"

struct SceneResources
{
	void				Initalize();
	SceneRenderTarget	VelocityBuffer;
	SceneRenderTarget	PreviousFrame;
	DepthTarget			DepthPrePass;
	DepthTarget			ShadowDepthTarget;
	ConstantBufferView	ShadowCBV;
};

extern SceneResources GSceneResources;