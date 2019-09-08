#pragma once
#include "Declarations.h"
#include "EngineContext.h"

struct PostProcessRenderTarget
{
	TextureID		Texture;
	ResourceID		Resource;
	RenderTargetID	RenderTarget;
};

PostProcessRenderTarget CreatePostProcessRenderTarget(EngineContext* context, uint32 width, uint32 height, DXGI_FORMAT format);