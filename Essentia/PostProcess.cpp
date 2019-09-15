#include "pch.h"
#include "PostProcess.h"
#include "RenderTargetManager.h"
#include "Renderer.h"
#include "PipelineStates.h"

PostProcess GPostProcess;

PostProcessRenderTarget CreatePostProcessRenderTarget(EngineContext* context, uint32 width, uint32 height, DXGI_FORMAT format)
{
	PostProcessRenderTarget target;
	auto ec = context;
	target.Resource = ec->ResourceManager->CreateResource(
		CD3DX12_RESOURCE_DESC::Tex2D(format, width, height, 1, 0, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET),
		nullptr, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	auto texResource = ec->ResourceManager->GetResource(target.Resource);
	target.Texture = ec->ShaderResourceManager->CreateTexture(texResource);
	target.RenderTarget = ec->RenderTargetManager->CreateRenderTargetView(texResource, format);
	return target;
}

void PostProcess::Intitialize()
{
	auto ec = EngineContext::Context;
	auto renderer = ec->RendererInstance;
	auto screenSize = renderer->GetScreenSize();
	auto downscaleFactor = 2;
	auto texFormat = renderer->GetRenderTargetFormat();

	screenSize.Height /= downscaleFactor;
	screenSize.Width /= downscaleFactor;
	PostTextures.HalfResSize = screenSize;
	PostTextures.HalfResTexture = CreatePostProcessRenderTarget(ec, screenSize.Width, screenSize.Height, texFormat);

	screenSize = renderer->GetScreenSize();
	downscaleFactor *= 2;
	screenSize.Height /= downscaleFactor;
	screenSize.Width /= downscaleFactor;
	PostTextures.QuarterResSize = screenSize;
	PostTextures.QuarterResTexture = CreatePostProcessRenderTarget(ec, screenSize.Width, screenSize.Height, texFormat);

	screenSize = renderer->GetScreenSize();
	downscaleFactor *= 2;
	screenSize.Height /= downscaleFactor;
	screenSize.Width /= downscaleFactor;
	PostTextures.HalfQuarterSize = screenSize;
	PostTextures.HalfQuarterTexture = CreatePostProcessRenderTarget(ec, screenSize.Width, screenSize.Height, texFormat);
}

void PostProcess::GenerateLowResTextures()
{
	auto ec = EngineContext::Context;
	auto renderer = ec->RendererInstance;
	auto commandList = renderer->GetDefaultCommandList();
	auto screenSize = renderer->GetScreenSize();
	auto downscaleFactor = 2;

	TransitionDesc preTransitions[] = {
		{ PostTextures.HalfResTexture.Resource, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET },
		{ PostTextures.QuarterResTexture.Resource, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET },
		{ PostTextures.HalfQuarterTexture.Resource, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET }
	};

	renderer->TransitionBarrier(commandList, preTransitions, _countof(preTransitions));

	RenderToTexture(commandList, PostTextures.HalfResTexture, PostTextures.HalfResSize, renderer);
	RenderToTexture(commandList, PostTextures.QuarterResTexture, PostTextures.QuarterResSize, renderer);
	RenderToTexture(commandList, PostTextures.HalfQuarterTexture, PostTextures.HalfQuarterSize, renderer);

	TransitionDesc postTransitions[] = {
		{ PostTextures.HalfResTexture.Resource, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE },
		{ PostTextures.QuarterResTexture.Resource, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE },
		{ PostTextures.HalfQuarterTexture.Resource, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE }
	};

	renderer->TransitionBarrier(commandList, postTransitions, _countof(preTransitions));
}

PostSceneTextures PostProcess::GetPostSceneTextures()
{
	return PostTextures;
}

void PostProcess::RegisterPostProcess(std::string_view postProcessString, IPostProcessStage* stage)
{
	postProcessStages[postProcessString] = stage;
}

void PostProcess::SetEnabled(std::string_view postProcess, bool enabled)
{
	postProcessStages[postProcess]->SetEnabled(enabled);
}

void PostProcess::RenderToTexture(ID3D12GraphicsCommandList* commandList, PostProcessRenderTarget target, ScreenSize screenSize, Renderer* renderer)
{
	auto ec = EngineContext::Context;
	auto rtv = ec->RenderTargetManager->GetRTVHandle(target.RenderTarget);
	float color[] = { 0,0,0,1 };

	D3D12_VIEWPORT viewport = {};

	viewport.Width = (FLOAT)screenSize.Width;
	viewport.Height = (FLOAT)screenSize.Height;

	D3D12_RECT scissorRect = {};

	scissorRect.left = 0;
	scissorRect.top = 0;
	scissorRect.right = screenSize.Width;
	scissorRect.bottom = screenSize.Height;

	commandList->SetGraphicsRootSignature(renderer->GetDefaultRootSignature());
	commandList->ClearRenderTargetView(rtv, color, 0, nullptr);
	renderer->SetRenderTargets(&target.RenderTarget, 1, nullptr);
	commandList->RSSetViewports(1, &viewport);
	commandList->RSSetScissorRects(1, &scissorRect);

	commandList->SetPipelineState(ec->ResourceManager->GetPSO(GPipelineStates.QuadPSO));

	renderer->SetShaderResourceView(commandList, RootSigSRVPixel1, renderer->GetCurrentRenderTargetTexture());
	renderer->DrawScreenQuad(commandList);
}

void IPostProcessStage::SetEnabled(bool enabled)
{
	Enabled = enabled;
}
