#include "PostProcessDepthOfFieldStage.h"
#include "ShaderManager.h"
#include "InputLayout.h"
#include "Renderer.h"

void PostProcessDepthOfFieldStage::Initialize()
{
	auto ec = EngineContext::Context;
	auto renderer = ec->RendererInstance;
	auto vertexShaderBytecode = ShaderManager::LoadShader(L"QuadVS.cso");
	auto pixelShaderBytecode = ShaderManager::LoadShader(L"QuadPS.cso");

	DXGI_SAMPLE_DESC sampleDesc = {};
	sampleDesc.Count = 1;

	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
	psoDesc.InputLayout.pInputElementDescs = InputLayout::DefaultLayout;
	psoDesc.InputLayout.NumElements = _countof(InputLayout::DefaultLayout);
	psoDesc.pRootSignature = renderer->GetDefaultRootSignature();
	psoDesc.VS = vertexShaderBytecode;
	psoDesc.PS = pixelShaderBytecode;
	psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	psoDesc.DepthStencilState.DepthEnable = false;
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	psoDesc.SampleDesc = sampleDesc;
	psoDesc.SampleMask = 0xffffffff;
	psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	psoDesc.RasterizerState.DepthClipEnable = false;
	psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	psoDesc.NumRenderTargets = 1;
	psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	psoDesc.DSVFormat = renderer->GetDepthStencilFormat();

	quadPso = ec->ResourceManager->CreatePSO(psoDesc);
}

TextureID PostProcessDepthOfFieldStage::RenderPostProcess(TextureID inputTexture)
{

	return TextureID();
}
