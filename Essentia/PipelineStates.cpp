#include "pch.h"
#include "PipelineStates.h"
#include "Engine.h"
#include "ShaderManager.h"
#include "Renderer.h"

PipelineStates GPipelineStates;

void PipelineStates::Initialize()
{
	auto ec = EngineContext::Context;
	auto renderer = ec->RendererInstance;

	DXGI_SAMPLE_DESC sampleDesc = {};
	sampleDesc.Count = 1;
	auto texFormat = renderer->GetRenderTargetFormat();

	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
	psoDesc.InputLayout.pInputElementDescs = nullptr;
	psoDesc.InputLayout.NumElements = 0;
	psoDesc.pRootSignature = renderer->GetDefaultRootSignature();
	psoDesc.VS = ShaderManager::LoadShader(L"QuadVS.cso");
	psoDesc.PS = ShaderManager::LoadShader(L"QuadPS.cso");
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

	QuadPSO = ec->ResourceManager->CreatePSO(psoDesc);

	psoDesc.RTVFormats[0] = renderer->GetHDRRenderTargetFormat();
	HDRQuadPSO = ec->ResourceManager->CreatePSO(psoDesc);
	psoDesc.PS = ShaderManager::LoadShader(L"BlurPS.cso");
	BlurPSO = ec->ResourceManager->CreatePSO(psoDesc);
}
