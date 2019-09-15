#pragma once
#include "RenderStage.h"
#include "RenderTargetManager.h"
#include "PostProcess.h"

class PostProcessDepthOfFieldStage :
	public IPostProcessStage
{
public:
	virtual void Initialize() override;
	virtual TextureID RenderPostProcess(uint32 backbufferIndex, TextureID inputTexture) override;
private:
	PipelineStateID blurPso;

	PostProcessRenderTarget blurIntermidateTarget;
	PostProcessRenderTarget blurFinalTarget;

	ConstantBufferView		blurHorizontalCBV;
	ConstantBufferView		blurVerticalCBV;
};

