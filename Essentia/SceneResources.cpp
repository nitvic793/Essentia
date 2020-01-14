#include "pch.h"
#include "SceneResources.h"
#include "EngineContext.h"
#include "RenderTargetManager.h"
#include "Renderer.h"
#include "PipelineStates.h"
SceneResources GSceneResources;

void SceneResources::Initalize()
{
	auto ec = EngineContext::Context;
	CurrentPSO = GPipelineStates.DefaultPSO;
	auto screenSize = ec->RendererInstance->GetScreenSize();
	VelocityBuffer = CreateSceneRenderTarget(ec, screenSize.Width, screenSize.Height, ec->RendererInstance->GetHDRRenderTargetFormat());
	PreviousFrame = CreateSceneRenderTarget(ec, screenSize.Width, screenSize.Height, ec->RendererInstance->GetHDRRenderTargetFormat());
}
