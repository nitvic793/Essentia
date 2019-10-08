#pragma once
#include "PostProcess.h"

struct TemporalAAParams
{
	DirectX::XMFLOAT2 ScreenSize;
};

class PostProcessTemporalAA :
	public IPostProcessStage
{
public:
	virtual void Initialize() override;
	virtual TextureID RenderPostProcess(uint32 backbufferIndex, TextureID inputTexture, const FrameContext& frameContext) override;
private:
	PipelineStateID		temporalAAPso;
	SceneRenderTarget	temporalAATarget;
	ConstantBufferView	temporalAACBV;
};

