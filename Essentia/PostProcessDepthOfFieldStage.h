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
	void RenderCocDownscale(uint32 backbufferIndex, TextureID input);
	void RenderDepthOfField(uint32 backbufferIndex, TextureID input);
	void RenderCompositePass(uint32 backbufferIndex, TextureID dofTexture, TextureID sceneTexture);

	PipelineStateID dofPso;
	PipelineStateID cocDownscalePso;
	PipelineStateID dofCompositePso;

	SceneRenderTarget cocDownscaleTarget;
	SceneRenderTarget dofTarget;
	SceneRenderTarget dofCompositeTarget;

	ConstantBufferView		blurHorizontalCBV;
	ConstantBufferView		blurVerticalCBV;
	ConstantBufferView		dofCBV;

	ScreenSize				internalResolution;
};

