#pragma once

#include "Declarations.h"

struct SceneTextures
{
	void				Initalize();
	SceneRenderTarget	VelocityBuffer;
	SceneRenderTarget	PreviousFrame;
};

extern SceneTextures GSceneTextures;