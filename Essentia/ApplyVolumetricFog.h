#pragma once
#include "PostProcess.h"

class ApplyVolumetricFog :
	public IPostProcessStage
{
public:
	virtual void Initialize() override;
	virtual TextureID RenderPostProcess(uint32 backbufferIndex, TextureID inputTexture, const FrameContext& frameContext) override;
private:
	SceneRenderTarget volFogTarget;
};

