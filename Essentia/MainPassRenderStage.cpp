#include "pch.h"
#include "MainPassRenderStage.h"
#include "Renderer.h"

void MainPassRenderStage::Initialize()
{
	auto ec = EngineContext::Context;
	GRenderStageManager.RegisterStage("MainPassRenderStage", this);
}

void MainPassRenderStage::Render(const uint32 frameIndex, const FrameContext& frameContext)
{

}

void MainPassRenderStage::Clear()
{
}
