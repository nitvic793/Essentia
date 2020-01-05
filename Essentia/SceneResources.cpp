#include "pch.h"
#include "SceneResources.h"
#include "EngineContext.h"
#include "RenderTargetManager.h"
#include "Renderer.h"

SceneResources GSceneResources;

void SceneResources::Initalize()
{
	auto ec = EngineContext::Context;
	auto screenSize = ec->RendererInstance->GetScreenSize();
	VelocityBuffer = CreateSceneRenderTarget(ec, screenSize.Width, screenSize.Height, ec->RendererInstance->GetHDRRenderTargetFormat());
	PreviousFrame = CreateSceneRenderTarget(ec, screenSize.Width, screenSize.Height, ec->RendererInstance->GetHDRRenderTargetFormat());
}
