#pragma once
#include "RenderStage.h"
#include "RenderTargetManager.h"
#include "PostProcess.h"

struct BlurParams
{
	DirectX::XMFLOAT2	Direction; // 0-Horizontal, 1-Vertical
	float				Width;
	float				Height;
};

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
	void RenderBlurTexture(uint32 backbufferIndex);

	PipelineStateID blurPso;
	PipelineStateID dofPso;

	PostProcessRenderTarget blurIntermidateTarget;
	PostProcessRenderTarget blurFinalTarget;
	PostProcessRenderTarget dofTarget;

	ConstantBufferView		blurHorizontalCBV;
	ConstantBufferView		blurVerticalCBV;
	ConstantBufferView		dofCBV;
};

