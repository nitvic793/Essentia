#include "pch.h"
#include "VoxelizationStage.h"
#include "Renderer.h"
#include "EngineContext.h"
#include "Entity.h"
#include "PipelineStates.h"
#include "SceneResources.h"

constexpr uint32 VOXELSIZE = 128;

void VoxelizationStage::Initialize()
{
	auto shaderResourceManager = GContext->ShaderResourceManager;
	uint32 voxelGridSize = VOXELSIZE;
	Texture3DCreateProperties props = {};
	auto renderer = GContext->RendererInstance;
	props.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	props.Width = voxelGridSize;
	props.Depth = voxelGridSize;
	props.Height = voxelGridSize;
	props.MipLevels = 1;

	voxelGrid3dTextureSRV = shaderResourceManager->CreateTexture3D(props, &voxelGridResource, nullptr, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
	voxelGrid3dTextureUAV = shaderResourceManager->CreateTexture3DUAV(voxelGridResource, voxelGridSize);
	voxelRT = CreateSceneRenderTarget(GContext, renderer->GetScreenSize().Width, renderer->GetScreenSize().Width, DXGI_FORMAT_R8G8B8A8_UNORM);

	renderer->TransitionBarrier(renderer->GetDefaultCommandList(), voxelRT.Resource, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET);
	GRenderStageManager.RegisterStage("VoxelizationStage", this);
}

void VoxelizationStage::Render(const uint32 frameIndex, const FrameContext& frameContext)
{
	auto ec = EngineContext::Context;
	auto renderer = ec->RendererInstance;
	auto commandList = renderer->GetDefaultCommandList();
	auto modelManager = ec->ModelManager;
	auto sz = renderer->GetScreenSize();
	auto rtManager = GContext->RenderTargetManager;
	auto srManager = GContext->ShaderResourceManager;
	auto resourceManager = GContext->ResourceManager;
	D3D12_VIEWPORT viewport = {};
	viewport.Width = (FLOAT)sz.Width;
	viewport.Height = (FLOAT)sz.Height;
	viewport.MaxDepth = 1.f;
	viewport.MinDepth = 0.f;

	D3D12_RECT scissorRect = {};
	scissorRect.left = 0;
	scissorRect.top = 0;
	scissorRect.right = sz.Width;
	scissorRect.bottom = sz.Height;

	TransitionDesc transitions[] = {
		{ voxelGridResource, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS }
	};

	renderer->TransitionBarrier(commandList, transitions, _countof(transitions));

	commandList->RSSetViewports(1, &viewport);
	commandList->RSSetScissorRects(1, &scissorRect);
	renderer->SetRenderTargets(nullptr, 0, nullptr);
	renderer->SetPipelineState(commandList, GPipelineStates.VoxelizePSO);
	uint32 clearVal[] = { 0, 0, 0, 0 };
	commandList->ClearUnorderedAccessViewUint(
		renderer->GetTextureGPUHandle(voxelGrid3dTextureUAV),
		srManager->GetTextureCPUHandle(voxelGrid3dTextureUAV), 
		resourceManager->GetResource(voxelGridResource), clearVal, 0, nullptr);

	renderer->SetConstantBufferView(commandList, RootSigCBAll1, GSceneResources.ShadowCBV);
	renderer->SetShaderResourceView(commandList, RootSigUAV0, voxelGrid3dTextureUAV);
	uint32 count = 0;
	auto drawables = frameContext.EntityManager->GetComponents<DrawableComponent>(count);

	for (uint32 i = 0; i < count; ++i)
	{
		auto mesh = drawables[i].Mesh;
		renderer->SetConstantBufferView(commandList, RootSigCBVertex0, drawables[i].CBView);
		renderer->DrawMesh(commandList, mesh);
	}

	auto drawableModels = frameContext.EntityManager->GetComponents<DrawableModelComponent>(count);
	for (uint32 i = 0; i < count; ++i)
	{
		auto& model = modelManager->GetModel(drawableModels[i].Model);
		renderer->SetConstantBufferView(commandList, RootSigCBVertex0, drawableModels[i].CBView);
		auto meshHandle = model.Mesh;
		renderer->DrawMesh(commandList, meshHandle);
	}

	TransitionDesc endTransitions[] = {
		{ voxelGridResource, D3D12_RESOURCE_STATE_UNORDERED_ACCESS,  D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE }
	};

	renderer->TransitionBarrier(commandList, endTransitions, _countof(endTransitions));
}


