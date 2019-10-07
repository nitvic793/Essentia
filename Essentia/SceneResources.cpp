#include "pch.h"
#include "SceneResources.h"
#include "EngineContext.h"
#include "RenderTargetManager.h"
#include "Renderer.h"

SceneTextures GSceneTextures;

void SceneTextures::Initalize()
{
	auto ec = EngineContext::Context;
	auto screenSize = ec->RendererInstance->GetScreenSize();
	VelocityBuffer = CreateSceneRenderTarget(ec, screenSize.Width, screenSize.Height, ec->RendererInstance->GetHDRRenderTargetFormat());
	PreviousFrame = CreateSceneRenderTarget(ec, screenSize.Width, screenSize.Height, ec->RendererInstance->GetHDRRenderTargetFormat());
}
