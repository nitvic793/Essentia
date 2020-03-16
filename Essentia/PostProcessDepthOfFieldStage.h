#pragma once
#include "RenderStage.h"
#include "RenderTargetManager.h"
#include "PostProcess.h"

struct DepthOfFieldParams
{
	float zNear;
	float zFar;
	float FocusPlaneZ;
	float Scale;
};

class PostProcessDepthOfFieldStage :
	public IPostProcessStage
{
public:
	virtual void Initialize() override;
	virtual TextureID RenderPostProcess(uint32 backbufferIndex, TextureID inputTexture, const FrameContext& frameContext) override;

	DepthOfFieldParams	DofParams;
	BlurParams			BlurParams;
private:
	void RenderBlurTexture(uint32 backbufferIndex, TextureID input);

	PipelineStateID dofPso;

	SceneRenderTarget blurIntermidateTarget;
	SceneRenderTarget blurFinalTarget;
	SceneRenderTarget dofTarget;

	ConstantBufferView		blurHorizontalCBV;
	ConstantBufferView		blurVerticalCBV;
	ConstantBufferView		dofCBV;
};

