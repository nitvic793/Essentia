#pragma once

#include "FrameContext.h"

//TODO: Need to pass info from one stage to another -> Render Target and other info?
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