#pragma once
#include "RenderStage.h"

class VoxelRadiancePostProcess :
	public IRenderStage
{
public:
	virtual void Initialize() override;
	virtual void Render(const uint32 frameIndex, const FrameContext& frameContext) override;
private:

};

