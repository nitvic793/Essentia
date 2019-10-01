#include "pch.h"
#include "SkyBoxRenderStage.h"
#include "ShaderManager.h"
#include "InputLayout.h"
#include "Renderer.h"
#include "Entity.h"

void SkyBoxRenderStage::Initialize()
{
	auto context = EngineContext::Context;
	shaderResourceManager = context->ShaderResourceManager;
	auto rm = context->ResourceManager;
	auto meshManager = context->MeshManager;
	renderer = context->RendererInstance;
	entityManager = context->EntityManager;
	cubeMesh = meshManager->GetMeshView("../../Assets/Models/cube.obj");

	D3D12_GRAPHICS_PIPELINE_STATE_DESC descPipelineState;
	ZeroMemory(&descPipelineState, sizeof(descPipelineState));
	auto depthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	depthStencilState.DepthEnable = true;
	depthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	depthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;

	auto rasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	rasterizerState.DepthClipEnable = true;
	rasterizerState.CullMode = D3D12_CULL_MODE_FRONT;
	rasterizerState.FillMode = D3D12_FILL_MODE_SOLID;

	descPipelineState.VS = ShaderManager::LoadShader(L"SkyboxVS.cso");
	descPipelineState.PS = ShaderManager::LoadShader(L"SkyboxPS.cso");
	descPipelineState.InputLayout.pInputElementDescs = InputLayout::DefaultLayout;
	descPipelineState.InputLayout.NumElements = _countof(InputLayout::DefaultLayout);
	descPipelineState.pRootSignature = renderer->GetDefaultRootSignature();
	descPipelineState.DepthStencilState = depthStencilState;
	descPipelineState.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	descPipelineState.RasterizerState = rasterizerState;
	descPipelineState.SampleMask = UINT_MAX;
	descPipelineState.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	descPipelineState.NumRenderTargets = 1;
	descPipelineState.RTVFormats[0] = renderer->GetHDRRenderTargetFormat();
	descPipelineState.SampleDesc.Count = 1;
	descPipelineState.DSVFormat = renderer->GetDepthStencilFormat();

	auto pso = rm->CreatePSO(descPipelineState);
	skyPSO = rm->GetPSO(pso);
}

void SkyBoxRenderStage::Render(const uint32 frameIndex, const FrameContext& frameContext)
{
	uint32 count = 0;
	auto skybox = entityManager->GetComponents<SkyboxComponent>(count);
	Assert(count <= 1, "Cannot have more than one skybox");

	if (count == 0) return;

	auto cubeMap = skybox[0].CubeMap;
	auto offsets = renderer->GetHeapOffsets();
	auto frame = renderer->GetFrameManager();
	auto commandList = renderer->GetDefaultCommandList();

	PerObjectConstantBuffer buffer;
	buffer.View = frameContext.Camera->GetViewTransposed();
	buffer.Projection = frameContext.Camera->GetProjectionTransposed();
	shaderResourceManager->CopyToCB(frameIndex, { &buffer, sizeof(buffer) }, skybox[0].CBView.Offset);

	commandList->SetPipelineState(skyPSO);
	commandList->SetGraphicsRootDescriptorTable(RootSigCBVertex0, frame->GetHandle(frameIndex, offsets.ConstantBufferOffset + skybox[0].CBView.Index));
	commandList->SetGraphicsRootDescriptorTable(RootSigSRVPixel1, frame->GetHandle(frameIndex, offsets.TexturesOffset + cubeMap));
	renderer->DrawMesh(cubeMesh);
}

void SkyBoxRenderStage::CleanUp()
{
}
