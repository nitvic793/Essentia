#include "pch.h"
#include "PipelineStates.h"
#include "Engine.h"
#include "ShaderManager.h"
#include "Renderer.h"
#include "InputLayout.h"
#include "ResourceManager.h"

PipelineStates GPipelineStates;

void PipelineStates::Initialize()
{
	auto ec = EngineContext::Context;
	auto renderer = ec->RendererInstance;
	auto resourceManager = ec->ResourceManager;

	CreateDefaultPSOs();
	CreateShadowPSO();
	CreateScreenSpaceAOPSO();
	CreateLightAccumPSO();
	CreateVoxelizePSO();
	CreateMipGen3DComputePSO();
	CreateVoxelCopyComputePSO();

	DXGI_SAMPLE_DESC sampleDesc = {};
	sampleDesc.Count = 1;
	auto texFormat = renderer->GetRenderTargetFormat();

	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};

	psoDesc.InputLayout.pInputElementDescs = InputLayout::DefaultLayout;
	psoDesc.InputLayout.NumElements = _countof(InputLayout::DefaultLayout);
	psoDesc.pRootSignature = renderer->GetDefaultRootSignature();
	psoDesc.VS = ShaderManager::LoadShader(L"DepthOnlyVS.cso");
	psoDesc.PS = ShaderManager::LoadShader(L"WorldPosPS.cso");
	psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	//psoDesc.DepthStencilState.DepthEnable = true;
	psoDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	psoDesc.RTVFormats[0] = renderer->GetHDRRenderTargetFormat();
	psoDesc.SampleDesc = sampleDesc;
	psoDesc.SampleMask = 0xffffffff;
	psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	psoDesc.NumRenderTargets = 1;
	psoDesc.DSVFormat = renderer->GetDepthStencilFormat();
	
	DepthOnlyPSO = resourceManager->CreatePSO(psoDesc);
	
	psoDesc.VS = ShaderManager::LoadShader(L"DefaultVS.cso");
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

	psoDesc.PS = ShaderManager::LoadShader(L"BilateralBlurPS.cso");
	BilateralBlurPSO = ec->ResourceManager->CreatePSO(psoDesc);

	psoDesc.PS = ShaderManager::LoadShader(L"BlurPS.cso");
	BlurPSO = ec->ResourceManager->CreatePSO(psoDesc);

}

void PipelineStates::CreateDefaultPSOs()
{
	auto resourceManager = GContext->ResourceManager;
	auto renderer = GContext->RendererInstance;
	auto vertexShaderBytecode = ShaderManager::LoadShader(L"DefaultVS.cso");
	auto pixelShaderBytecode = ShaderManager::LoadShader(L"DefaultPS.cso");

	DXGI_SAMPLE_DESC sampleDesc = {};
	sampleDesc.Count = 1;

	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
	psoDesc.InputLayout.pInputElementDescs = InputLayout::DefaultLayout;
	psoDesc.InputLayout.NumElements = _countof(InputLayout::DefaultLayout);
	psoDesc.pRootSignature = renderer->GetDefaultRootSignature();
	psoDesc.VS = vertexShaderBytecode;
	psoDesc.PS = pixelShaderBytecode;
	psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	//psoDesc.DepthStencilState.DepthEnable = false;
	//psoDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
	//psoDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_NEVER;
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	psoDesc.RTVFormats[0] = renderer->GetHDRRenderTargetFormat();
	psoDesc.SampleDesc = sampleDesc;
	psoDesc.SampleMask = 0xffffffff;
	psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	psoDesc.RasterizerState.AntialiasedLineEnable = true;
	//psoDesc.RasterizerState.DepthClipEnable = false;
	psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	psoDesc.NumRenderTargets = 1;
	psoDesc.DSVFormat = renderer->GetDepthStencilFormat();

	DefaultPSO = resourceManager->CreatePSO(psoDesc);
}

void PipelineStates::CreateLightAccumPSO()
{
	auto resourceManager = GContext->ResourceManager;
	auto renderer = GContext->RendererInstance;
	auto vertexShaderBytecode = ShaderManager::LoadShader(L"QuadVS.cso");
	auto pixelShaderBytecode = ShaderManager::LoadShader(L"AccumulateFogPS.cso");

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
	//psoDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
	//psoDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_NEVER;
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	psoDesc.RTVFormats[0] = renderer->GetHDRRenderTargetFormat();
	psoDesc.SampleDesc = sampleDesc;
	psoDesc.SampleMask = 0xffffffff;
	psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	//psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_FRONT;
	psoDesc.RasterizerState.DepthClipEnable = false;
	psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	psoDesc.NumRenderTargets = 1;
	psoDesc.DSVFormat = renderer->GetDepthStencilFormat();

	LightAccumPSO = resourceManager->CreatePSO(psoDesc);

	psoDesc.PS = ShaderManager::LoadShader(L"ApplyVolumetricFogPS.cso");
	ApplyFogPSO = resourceManager->CreatePSO(psoDesc);
}

void PipelineStates::CreateShadowPSO()
{
	auto renderer = GContext->RendererInstance;
	D3D12_GRAPHICS_PIPELINE_STATE_DESC descPipelineState;
	ZeroMemory(&descPipelineState, sizeof(descPipelineState));
	auto shadowMapFormat = DXGI_FORMAT_D32_FLOAT;
	descPipelineState.VS = ShaderManager::LoadShader(L"ShadowVS.cso");
	descPipelineState.InputLayout.pInputElementDescs = InputLayout::DefaultLayout;
	descPipelineState.InputLayout.NumElements = _countof(InputLayout::DefaultLayout);
	descPipelineState.pRootSignature = renderer->GetDefaultRootSignature();
	descPipelineState.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	descPipelineState.DepthStencilState.DepthEnable = true;
	descPipelineState.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	descPipelineState.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	descPipelineState.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
	descPipelineState.RasterizerState.CullMode = D3D12_CULL_MODE_FRONT;
	descPipelineState.RasterizerState.AntialiasedLineEnable = TRUE;
	descPipelineState.RasterizerState.DepthClipEnable = true;
	//descPipelineState.RasterizerState.DepthBias = 1000;
	//descPipelineState.RasterizerState.DepthBiasClamp = 0.f;
	//descPipelineState.RasterizerState.SlopeScaledDepthBias = 5.f;
	descPipelineState.SampleMask = UINT_MAX;
	descPipelineState.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	descPipelineState.NumRenderTargets = 0;
	//descPipelineState.RTVFormats[0] = mDsvFormat; 
	descPipelineState.DSVFormat = shadowMapFormat;
	descPipelineState.SampleDesc.Count = 1;

	ShadowDirPSO = GContext->ResourceManager->CreatePSO(descPipelineState);
}

void PipelineStates::CreateScreenSpaceAOPSO()
{
	auto renderer = GContext->RendererInstance;
	auto texFormat = DXGI_FORMAT_R32_FLOAT; //For debugging, should be R32_FLOAT

	DXGI_SAMPLE_DESC sampleDesc = {};
	sampleDesc.Count = 1;

	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
	psoDesc.InputLayout.pInputElementDescs = nullptr;
	psoDesc.InputLayout.NumElements = 0;
	psoDesc.pRootSignature = renderer->GetDefaultRootSignature();
	psoDesc.VS = ShaderManager::LoadShader(L"AOQuadVS.cso");
	psoDesc.PS = ShaderManager::LoadShader(L"ScreenSpaceAOPS.cso");
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

	ScreenSpaceAOPSO = GContext->ResourceManager->CreatePSO(psoDesc);

	psoDesc.PS = ShaderManager::LoadShader(L"SSAOBlurPS.cso");

	SSAOBlurPSO = GContext->ResourceManager->CreatePSO(psoDesc);
}

void PipelineStates::CreateVoxelizePSO()
{
	auto resourceManager = GContext->ResourceManager;
	auto renderer = GContext->RendererInstance;

	DXGI_SAMPLE_DESC sampleDesc = {};
	sampleDesc.Count = 1;

	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
	psoDesc.InputLayout.pInputElementDescs = InputLayout::DefaultLayout;
	psoDesc.InputLayout.NumElements = _countof(InputLayout::DefaultLayout);
	psoDesc.pRootSignature = renderer->GetDefaultRootSignature();
	psoDesc.VS = ShaderManager::LoadShader(L"VoxelizeVS.cso");
	psoDesc.PS = ShaderManager::LoadShader(L"VoxelizePS.cso");
	psoDesc.GS = ShaderManager::LoadShader(L"VoxelizeGS.cso");

	psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	psoDesc.DepthStencilState.DepthEnable = false;
	psoDesc.DepthStencilState.StencilEnable = false;
	psoDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_GREATER_EQUAL;

	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	//psoDesc.RTVFormats[0] = renderer->GetRenderTargetFormat();
	psoDesc.SampleDesc = sampleDesc;
	psoDesc.SampleMask = 0xffffffff;

	psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	psoDesc.RasterizerState.FrontCounterClockwise = true;
	psoDesc.RasterizerState.ForcedSampleCount = 0;
	psoDesc.RasterizerState.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_ON;
	psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
	psoDesc.RasterizerState.DepthClipEnable = true;
	psoDesc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;

	psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	psoDesc.NumRenderTargets = 0;
	//psoDesc.DSVFormat = renderer->GetDepthStencilFormat();

	VoxelizePSO = resourceManager->CreatePSO(psoDesc);
}

void PipelineStates::CreateMipGen3DComputePSO()
{
	auto resourceManager = GContext->ResourceManager;
	auto renderer = GContext->RendererInstance;

	D3D12_COMPUTE_PIPELINE_STATE_DESC psoDesc = {};
	psoDesc.CS = ShaderManager::LoadShader(L"GenerateMips3DCS.cso");
	psoDesc.pRootSignature = renderer->GetDefaultComputeRootSignature();

	MipGen3DCSPSO = resourceManager->CreateComputePSO(psoDesc);
}

void PipelineStates::CreateVoxelCopyComputePSO()
{
	auto resourceManager = GContext->ResourceManager;
	auto renderer = GContext->RendererInstance;

	D3D12_COMPUTE_PIPELINE_STATE_DESC psoDesc = {};
	psoDesc.CS = ShaderManager::LoadShader(L"VoxelCopyCS.cso");
	psoDesc.pRootSignature = renderer->GetDefaultComputeRootSignature();

	VoxelCopyPSO = resourceManager->CreateComputePSO(psoDesc);
}
