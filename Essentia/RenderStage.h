#pragma once

#include "FrameContext.h"

class IRenderStage
{
public:
	virtual void Initialize() {};
	virtual void Clear() {};
	virtual void Render(const FrameContext& frameContext) = 0;
private:
};