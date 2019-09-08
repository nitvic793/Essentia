#pragma once

#include "FrameContext.h"

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

//TODO : Pass the processed screen texture to the next stage
class IPostProcessStage
{
public:
	virtual void		Initialize() {};
	virtual TextureID	RenderPostProcess(uint32 backbufferIndex, TextureID inputTexture) = 0; //Assume input texture size is always full size of the window
};

