#include "PostProcessDepthOfFieldStage.h"
#include "ShaderManager.h"
#include "InputLayout.h"
#include "Renderer.h"
#include "PostProcess.h"
#include "PipelineStates.h"
#include "Entity.h"
#include "pix3.h"

//Do AO?
using namespace DirectX;

void PostProcessDepthOfFieldStage::Initialize()
{
	auto ec = EngineContext::Context;
	auto renderer = ec->RendererInstance;

	DXGI_SAMPLE_DESC sampleDesc = {};
	sampleDesc.Count = 1;
	auto texFormat = renderer->GetHDRRenderTargetFormat();

	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
	psoDesc.InputLayout.pInputElementDescs = nullptr;
	psoDesc.InputLayout.NumElements = 0;
	psoDesc.pRootSignature = renderer->GetDefaultRootSignature();
	psoDesc.VS = ShaderManager::LoadShader(L"QuadVS.cso");
	psoDesc.PS = ShaderManager::LoadShader(L"CocDownscale.cso");
	psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	psoDesc.DepthStencilState.DepthEnable = false;
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	psoDesc.RTVFormats[0] = texFormat;
	psoDesc.SampleDesc = sampleDesc;
	psoDesc.SampleMask = 0xffffffff;
	psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	psoDesc.RasterizerState.DepthClipEnable = false;
	psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	psoDesc.NumRenderTargets = 1;

	cocDownscalePso = ec->ResourceManager->CreatePSO(psoDesc);

	psoDesc.PS = ShaderManager::LoadShader(L"DofComposite.cso");
	dofCompositePso = ec->ResourceManager->CreatePSO(psoDesc);

	psoDesc.PS = ShaderManager::LoadShader(L"DepthOfFieldPS.cso");
	dofPso = ec->ResourceManager->CreatePSO(psoDesc);

	dofCBV = ec->ShaderResourceManager->CreateCBV(sizeof(DepthOfFieldParams));

	internalResolution = GPostProcess.GetPostSceneTextures().HalfResSize;
	auto fullRes = renderer->GetScreenSize();
	cocDownscaleTarget = CreateSceneRenderTarget(ec, internalResolution.Width, internalResolution.Height, texFormat);
	dofTarget = CreateSceneRenderTarget(ec, internalResolution.Width, internalResolution.Height, texFormat);
	dofCompositeTarget = CreateSceneRenderTarget(ec, fullRes.Width, fullRes.Height, texFormat);
	DofParams = { 0, 0, 10.f, 0.01f, (float)internalResolution.Width, (float)internalResolution.Height };
	GPostProcess.RegisterPostProcess("DepthOfField", this);
}

TextureID PostProcessDepthOfFieldStage::RenderPostProcess(uint32 backbufferIndex, TextureID inputTexture, const FrameContext& frameContext)
{
	auto renderer = GContext->RendererInstance;
	auto em = GContext->EntityManager;
	auto commandList = renderer->GetDefaultCommandList();

	PIXBeginEvent(commandList, PIX_COLOR_DEFAULT, L"DepthOfField");

	uint32 count = 0;
	auto camera = &em->GetComponents<CameraComponent>(count)[0].CameraInstance;
	DofParams.zNear = camera->NearZ;
	DofParams.zFar = camera->FarZ;
	GContext->ShaderResourceManager->CopyToCB(backbufferIndex, { &DofParams, sizeof(DofParams) }, dofCBV.Offset);

	RenderCocDownscale(backbufferIndex, inputTexture);
	RenderDepthOfField(backbufferIndex, cocDownscaleTarget.Texture);
	RenderCompositePass(backbufferIndex, dofTarget.Texture, inputTexture);

	PIXEndEvent(commandList);

	return dofCompositeTarget.Texture;
}

void PostProcessDepthOfFieldStage::RenderCocDownscale(uint32 backbufferIndex, TextureID inputTexture)
{
	auto resolution = internalResolution;
	auto renderer = GContext->RendererInstance;
	auto commandList = renderer->GetDefaultCommandList();
	renderer->TransitionBarrier(commandList, cocDownscaleTarget.Resource, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET);

	auto rtv = GContext->RenderTargetManager->GetRTVHandle(cocDownscaleTarget.RenderTarget);
	commandList->ClearRenderTargetView(rtv, ColorValues::ClearColor, 0, nullptr);
	renderer->SetTargetSize(commandList, resolution);

	TextureID textures[] = { inputTexture, renderer->GetCurrentDepthStencilTexture() };
	renderer->SetShaderResourceViews(commandList, RootSigSRVPixel1, textures, _countof(textures));
	renderer->SetConstantBufferView(commandList, RootSigCBPixel0, dofCBV);
	renderer->SetRenderTargets(&cocDownscaleTarget.RenderTarget, 1, nullptr);
	renderer->SetPipelineState(commandList, cocDownscalePso);
	renderer->DrawScreenQuad(commandList);

	renderer->TransitionBarrier(commandList, cocDownscaleTarget.Resource, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
}

void PostProcessDepthOfFieldStage::RenderDepthOfField(uint32 backbufferIndex, TextureID input)
{
	auto resolution = internalResolution;
	auto renderer = GContext->RendererInstance;
	auto commandList = renderer->GetDefaultCommandList();
	renderer->TransitionBarrier(commandList, dofTarget.Resource, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET);

	auto rtv = GContext->RenderTargetManager->GetRTVHandle(dofTarget.RenderTarget);
	commandList->ClearRenderTargetView(rtv, ColorValues::ClearColor, 0, nullptr);
	renderer->SetTargetSize(commandList, resolution);

	TextureID textures[] = { input, renderer->GetCurrentDepthStencilTexture() };
	renderer->SetShaderResourceViews(commandList, RootSigSRVPixel1, textures, _countof(textures));
	renderer->SetConstantBufferView(commandList, RootSigCBPixel0, dofCBV);
	renderer->SetRenderTargets(&dofTarget.RenderTarget, 1, nullptr);
	renderer->SetPipelineState(commandList, dofPso);
	renderer->DrawScreenQuad(commandList);

	renderer->TransitionBarrier(commandList, dofTarget.Resource, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
}

void PostProcessDepthOfFieldStage::RenderCompositePass(uint32 backbufferIndex, TextureID dofTexture, TextureID sceneTexture)
{
	auto renderer = GContext->RendererInstance;
	auto fullRes = renderer->GetScreenSize();
	auto commandList = renderer->GetDefaultCommandList();
	renderer->TransitionBarrier(commandList, dofCompositeTarget.Resource, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET);

	auto rtv = GContext->RenderTargetManager->GetRTVHandle(dofCompositeTarget.RenderTarget);
	commandList->ClearRenderTargetView(rtv, ColorValues::ClearColor, 0, nullptr);
	renderer->SetTargetSize(commandList, fullRes);

	TextureID textures[] = { dofTexture, sceneTexture };
	renderer->SetShaderResourceViews(commandList, RootSigSRVPixel1, textures, _countof(textures));
	renderer->SetConstantBufferView(commandList, RootSigCBPixel0, dofCBV);
	renderer->SetRenderTargets(&dofCompositeTarget.RenderTarget, 1, nullptr);
	renderer->SetPipelineState(commandList, dofCompositePso);
	renderer->DrawScreenQuad(commandList);

	renderer->TransitionBarrier(commandList, dofCompositeTarget.Resource, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
}
