#include "pch.h"
#include "PostProcess.h"
#include "RenderTargetManager.h"
#include "Renderer.h"
#include "PipelineStates.h"

using namespace DirectX;

PostProcess GPostProcess;

PostProcessRenderTarget CreatePostProcessRenderTarget(EngineContext* context, uint32 width, uint32 height, DXGI_FORMAT format)
{
	PostProcessRenderTarget target;
	auto ec = context;
	target.Texture = ec->ShaderResourceManager->CreateTexture2D({ width, height, format }, &target.Resource);
	auto resource = ec->ShaderResourceManager->GetResource(target.Texture);
	target.RenderTarget = ec->RenderTargetManager->CreateRenderTargetView(resource, format);
	return target;
}

void PostProcess::Intitialize()
{
	auto ec = EngineContext::Context;
	auto renderer = ec->RendererInstance;
	auto screenSize = renderer->GetScreenSize();
	auto downscaleFactor = 2;
	auto texFormat = renderer->GetHDRRenderTargetFormat();

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

	renderer->TransitionBarrier(commandList, postTransitions, _countof(postTransitions));
}

PostSceneTextures PostProcess::GetPostSceneTextures()
{
	return PostTextures;
}

void PostProcess::RenderBlurTexture(
	TextureID input,
	ScreenSize screenSize,
	uint32 backBufferIndex, 
	PostProcessRenderTarget target,
	PostProcessRenderTarget intermediateTarget,
	ConstantBufferView vertCBV,
	ConstantBufferView horCBV
)
{
	auto ec = EngineContext::Context;
	auto renderer = ec->RendererInstance;
	auto commandList = renderer->GetDefaultCommandList();

	TransitionDesc transitions[] = {
		{  intermediateTarget.Resource, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET }
	};

	renderer->TransitionBarrier(commandList, transitions, 1);

	BlurParams BlurParams = { XMFLOAT2(1.f,0.f), (float)screenSize.Width, (float)screenSize.Height };
	ec->ShaderResourceManager->CopyToCB(backBufferIndex, { &BlurParams, sizeof(BlurParams) }, horCBV.Offset);
	BlurParams.Direction = XMFLOAT2(0.f, 1.f);
	ec->ShaderResourceManager->CopyToCB(backBufferIndex, { &BlurParams, sizeof(BlurParams) }, vertCBV.Offset);

	renderer->SetTargetSize(commandList, screenSize);
	commandList->SetGraphicsRootSignature(renderer->GetDefaultRootSignature());

	auto blurRtv = ec->RenderTargetManager->GetRTVHandle(intermediateTarget.RenderTarget);
	commandList->ClearRenderTargetView(blurRtv, ColorValues::ClearColor, 0, nullptr);
	renderer->SetRenderTargets(&intermediateTarget.RenderTarget, 1, nullptr);

	commandList->SetPipelineState(ec->ResourceManager->GetPSO(GPipelineStates.BlurPSO));

	renderer->SetShaderResourceViews(commandList, RootSigSRVPixel1, &input, 1);
	renderer->SetConstantBufferView(commandList, RootSigCBPixel0, horCBV);
	renderer->DrawScreenQuad(commandList);

	TransitionDesc blurTransitions[] = {
		{ intermediateTarget.Resource, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE },
		{ target.Resource, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET }
	};

	renderer->TransitionBarrier(commandList, blurTransitions, 2);
	blurRtv = ec->RenderTargetManager->GetRTVHandle(target.RenderTarget);
	renderer->SetRenderTargets(&target.RenderTarget, 1, nullptr);
	commandList->ClearRenderTargetView(blurRtv, ColorValues::ClearColor, 0, nullptr);

	renderer->SetShaderResourceView(commandList, RootSigSRVPixel1, intermediateTarget.Texture);
	renderer->SetConstantBufferView(commandList, RootSigCBPixel0, vertCBV);
	renderer->DrawScreenQuad(commandList);

	renderer->TransitionBarrier(commandList, target.Resource, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
}

void PostProcess::RegisterPostProcess(std::string_view postProcessString, IPostProcessStage* stage)
{
	postProcessStages[postProcessString] = stage;
}

void PostProcess::SetEnabled(std::string_view postProcess, bool enabled)
{
	postProcessStages[postProcess]->SetEnabled(enabled);
}

IPostProcessStage* PostProcess::GetPostProcessStage(std::string_view name)
{
	return postProcessStages[name];
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

	commandList->SetPipelineState(ec->ResourceManager->GetPSO(GPipelineStates.HDRQuadPSO));

	renderer->SetShaderResourceView(commandList, RootSigSRVPixel1, renderer->GetCurrentHDRRenderTargetTexture());
	renderer->DrawScreenQuad(commandList);
}

void IPostProcessStage::SetEnabled(bool enabled)
{
	Enabled = enabled;
}

void IPostProcessStage::RenderToSceneTarget(TextureID inputTexture)
{
	auto ec = EngineContext::Context;
	auto renderer = ec->RendererInstance;
	auto commandList = renderer->GetDefaultCommandList();
	auto rtTexture = renderer->GetCurrentHDRRenderTargetTexture();
	auto rtResource = ec->ShaderResourceManager->GetResource(rtTexture);

	D3D12_VIEWPORT viewport = {};
	D3D12_RECT scissorRect = {};
	renderer->TransitionBarrier(commandList, rtResource, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET);
	auto screenSize = renderer->GetScreenSize();

	auto rt = renderer->GetDefaultHDRRenderTarget();
	renderer->SetRenderTargets(&rt, 1, nullptr);

	viewport.Width = (FLOAT)screenSize.Width;
	viewport.Height = (FLOAT)screenSize.Height;
	scissorRect.right = screenSize.Width;
	scissorRect.bottom = screenSize.Height;

	commandList->RSSetViewports(1, &viewport);
	commandList->RSSetScissorRects(1, &scissorRect);
	commandList->SetPipelineState(ec->ResourceManager->GetPSO(GPipelineStates.HDRQuadPSO));
	renderer->SetShaderResourceView(commandList, RootSigSRVPixel1, inputTexture);
	renderer->DrawScreenQuad(commandList);

	renderer->TransitionBarrier(commandList, rtResource, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
}
