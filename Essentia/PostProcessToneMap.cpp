#include "pch.h"
#include "PostProcessToneMap.h"
#include "Renderer.h"
#include "ShaderManager.h"

void PostProcessToneMap::Initialize()
{
	auto ec = EngineContext::Context;
	auto renderer = ec->RendererInstance;

	DXGI_SAMPLE_DESC sampleDesc = {};
	sampleDesc.Count = 1;
	auto texFormat = renderer->GetHDRRenderTargetFormat();
	auto size = renderer->GetScreenSize();

	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
	psoDesc.InputLayout.pInputElementDescs = nullptr;
	psoDesc.InputLayout.NumElements = 0;
	psoDesc.pRootSignature = renderer->GetDefaultRootSignature();
	psoDesc.VS = ShaderManager::LoadShader(L"QuadVS.cso");
	psoDesc.PS = ShaderManager::LoadShader(L"ToneMapPS.cso");
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

	toneMapPSO = ec->ResourceManager->CreatePSO(psoDesc);
	toneMapRenderTarget = CreatePostProcessRenderTarget(ec, size.Width, size.Height, texFormat);
	GPostProcess.RegisterPostProcess("ToneMap", this);
}

TextureID PostProcessToneMap::RenderPostProcess(uint32 backbufferIndex, TextureID inputTexture, const FrameContext& frameContext)
{
	auto ec = EngineContext::Context;
	auto renderer = ec->RendererInstance;
	auto commandList = renderer->GetDefaultCommandList();

	renderer->TransitionBarrier(commandList, toneMapRenderTarget.Resource, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET);

	auto rtv = ec->RenderTargetManager->GetRTVHandle(toneMapRenderTarget.RenderTarget);
	renderer->SetRenderTargets(&toneMapRenderTarget.RenderTarget, 1, nullptr);
	renderer->SetTargetSize(commandList, renderer->GetScreenSize());
	commandList->ClearRenderTargetView(rtv, ColorValues::ClearColor, 0, nullptr);
	renderer->SetShaderResourceView(commandList, RootSigSRVPixel1, inputTexture);
	commandList->SetPipelineState(ec->ResourceManager->GetPSO(toneMapPSO));
	renderer->DrawScreenQuad(commandList);

	renderer->TransitionBarrier(commandList, toneMapRenderTarget.Resource, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	return toneMapRenderTarget.Texture;
}
