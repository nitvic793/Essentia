#include "pch.h"
#include "ApplyVolumetricFog.h"
#include "Renderer.h"
#include "PipelineStates.h"
#include "SceneResources.h"

void ApplyVolumetricFog::Initialize()
{
	auto renderer = GContext->RendererInstance;
	auto texFormat = renderer->GetHDRRenderTargetFormat();
	auto size = renderer->GetScreenSize();

	volFogTarget = CreateSceneRenderTarget(GContext, size.Width, size.Height, texFormat);
	GPostProcess.RegisterPostProcess("ApplyVolumetricFog", this);
}

TextureID ApplyVolumetricFog::RenderPostProcess(uint32 backbufferIndex, TextureID inputTexture, const FrameContext& frameContext)
{
	auto renderer = GContext->RendererInstance;
	auto commandList = renderer->GetDefaultCommandList();

	renderer->TransitionBarrier(commandList, volFogTarget.Resource, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET);

	auto rtv = GContext->RenderTargetManager->GetRTVHandle(volFogTarget.RenderTarget);
	renderer->SetRenderTargets(&volFogTarget.RenderTarget, 1, nullptr);
	renderer->SetTargetSize(commandList, renderer->GetScreenSize());
	commandList->ClearRenderTargetView(rtv, ColorValues::ClearColor, 0, nullptr);

	TextureID textures[] = { inputTexture ,  GSceneResources.LightAccumTarget.Texture };
	renderer->SetShaderResourceViews(commandList, RootSigSRVPixel1, textures, _countof(textures));
	commandList->SetPipelineState(GContext->ResourceManager->GetPSO(GPipelineStates.ApplyFogPSO));
	renderer->DrawScreenQuad(commandList);

	renderer->TransitionBarrier(commandList, volFogTarget.Resource, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	return volFogTarget.Texture;
}
