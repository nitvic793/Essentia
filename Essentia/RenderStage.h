#pragma once

#include "FrameContext.h"

enum RenderStageType
{
	eRenderStageMain = 0,
	eRenderTypeTransparent,
	eRenderStageGUI,
	eRenderTypeCount
};

//TODO: Need to pass info from one stage to another -> Render Target and other info?
//Assumptions: Render Target set by renderer is always the default render target
class IRenderStage
{
public:
	virtual void Initialize() {};
	virtual void Clear() {};
	virtual void Render(const uint32 frameIndex, const FrameContext& frameContext) = 0;
	virtual void CleanUp() {};
	virtual ~IRenderStage() {};
private:
};
