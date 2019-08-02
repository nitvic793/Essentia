#pragma once
#include "RenderStage.h"
#include "Engine.h"

class MainPassRenderStage :
	public IRenderStage
{
public:
	virtual void Initialize() override;
	virtual void Render(const uint32 frameIndex, const FrameContext& frameContext) override;
	virtual void Clear() override;
private:
};

