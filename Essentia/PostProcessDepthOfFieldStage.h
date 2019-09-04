#pragma once
#include "RenderStage.h"
#include "RenderTargetManager.h"

class PostProcessDepthOfFieldStage :
	public IPostProcessStage
{
public:
	virtual void Initialize() override;
	virtual TextureID RenderPostProcess(TextureID inputTexture) override;
private:
	PipelineStateID blurPso;
	PipelineStateID quadPso;

	RenderTargetID  lowResRenderTarget;
	TextureID lowResTexture;
};

