#pragma once
#include "RenderStage.h"

class ImguiRenderStage : public IRenderStage
{
public:
	virtual void Initialize() override;
	virtual void Render(const uint32 frameIndex, const FrameContext& frameContext) override;
	virtual void CleanUp() override;
private:
	DescriptorHeap	imguiHeap;
	bool			show = false;
};

