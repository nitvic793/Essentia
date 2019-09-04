#include "pch.h"
#include "OutlineRenderStage.h"
#include "ShaderManager.h"
#include "InputLayout.h"
#include "Renderer.h"
#include "Entity.h"

void OutlineRenderStage::Initialize()
{
	auto ec = EngineContext::Context;
	auto vertexShaderBytecode = ShaderManager::LoadShader(L"OutlineVS.cso");
	auto pixelShaderBytecode = ShaderManager::LoadShader(L"OutlinePS.cso");

	DXGI_SAMPLE_DESC sampleDesc = {};
	sampleDesc.Count = 1;

	auto rasterizer = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	rasterizer.CullMode = D3D12_CULL_MODE_FRONT;
	rasterizer.DepthClipEnable = false;

	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
	psoDesc.InputLayout.pInputElementDescs = InputLayout::DefaultLayout;
	psoDesc.InputLayout.NumElements = _countof(InputLayout::DefaultLayout);
	psoDesc.pRootSignature = ec->RendererInstance->GetDefaultRootSignature();
	psoDesc.VS = vertexShaderBytecode;
	psoDesc.PS = pixelShaderBytecode;
	psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	psoDesc.SampleDesc = sampleDesc;
	psoDesc.SampleMask = 0xffffffff;
	psoDesc.RasterizerState = rasterizer;
	psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);

	psoDesc.NumRenderTargets = 1;
	psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	psoDesc.DSVFormat = ec->RendererInstance->GetDepthStencilFormat();

	outlinePSO = ec->ResourceManager->CreatePSO(psoDesc);
}

void OutlineRenderStage::Render(const uint32 frameIndex, const FrameContext& frameContext)
{
	auto ec = EngineContext::Context;
	auto renderer = ec->RendererInstance;
	uint32 count;
	auto entities = frameContext.EntityManager->GetEntities<SelectedComponent>(count);
	auto commandList = renderer->GetDefaultCommandList();

	commandList->SetPipelineState(ec->ResourceManager->GetPSO(outlinePSO));
	for (size_t i = 0; i < count; ++i)
	{
		auto drawable = frameContext.EntityManager->GetComponent<DrawableComponent>(entities[i]);
		renderer->SetConstantBufferView(commandList, RootSigCBVertex0, drawable->CBView);
		renderer->DrawMesh(drawable->Mesh);
	}
}
