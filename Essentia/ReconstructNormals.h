#pragma once

#include "RenderStage.h"

class ReconstructNormals :
	public IRenderStage
{
public:
	virtual void Initialize() override;
	virtual void Render(const uint32 frameIndex, const FrameContext& frameContext) override;
	virtual void CleanUp() override;
private:
	SceneRenderTarget normalTarget;
};