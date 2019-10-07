#pragma once
#include "PostProcess.h"

struct MotionBlurParams
{
	DirectX::XMFLOAT2	ScreenSize;
	float				VelocityScale;
	float				Padding;
};

class PostProcessMotionBlur :
	public IPostProcessStage
{
public:
	virtual void Initialize() override;
	virtual TextureID RenderPostProcess(uint32 backbufferIndex, TextureID inputTexture, const FrameContext& frameContext) override;
private:
	PipelineStateID		motionBlurPso;
	ConstantBufferView	motionCBV;
	SceneRenderTarget   motionBlurTarget;
};

