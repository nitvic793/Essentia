#pragma once

#include "RenderStage.h"

class DepthOnlyStage : public IRenderStage
{
public:
	virtual void Initialize() override;
	virtual void Render(const uint32 frameIndex, const FrameContext& frameContext) override;
private:
	ID3D12PipelineState*	depthOnlyPso;
	DepthTarget				depthTarget;
	SceneRenderTarget		worldPosTarget;
};

