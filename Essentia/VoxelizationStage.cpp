#include "pch.h"
#include "VoxelizationStage.h"
#include "Renderer.h"
#include "EngineContext.h"
#include "Entity.h"
#include "PipelineStates.h"
#include "SceneResources.h"

#include <DirectXCollision.h>
using namespace DirectX;

//Reference: https://github.com/KolyaNaichuk/RenderSDK/blob/c176de53d1f0c08f7e5be6790229203da305bc65/Samples/DynamicGI/Source/DXApplication.cpp

void VoxelizationStage::Initialize()
{
	auto shaderResourceManager = GContext->ShaderResourceManager;
	uint32 voxelGridSize = CVoxelSize;
	Texture3DCreateProperties props = {};
	auto renderer = GContext->RendererInstance;
	props.Format = DXGI_FORMAT_R16G16B16A16_FLOAT; //DXGI_FORMAT_R16G16B16A16_UNORM; //
	props.Width = voxelGridSize;
	props.Depth = voxelGridSize;
	props.Height = voxelGridSize;
	props.MipLevels = CVoxelGridMips;

	voxelGrid3dTextureSRV = shaderResourceManager->CreateTexture3D(props, &voxelGridResource, nullptr, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
	voxelGrid3dTextureUAV = shaderResourceManager->CreateTexture3DUAV(voxelGridResource, voxelGridSize);
	GSceneResources.VoxelGridSRV = voxelGrid3dTextureSRV;
	GSceneResources.VoxelGridResource = voxelGridResource;
	GSceneResources.VoxelRadiance.VoxelGridUAV = voxelGrid3dTextureUAV;

	props.MipLevels = 1;
	ResourceID voxelGridRawResource;
	GSceneResources.VoxelRadiance.VoxelGridRawSRV = shaderResourceManager->CreateTexture3D(props, &voxelGridRawResource, nullptr, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
	GSceneResources.VoxelRadiance.VoxelGridRawUAV = shaderResourceManager->CreateTexture3DUAV(voxelGridRawResource, voxelGridSize);
	GSceneResources.VoxelRadiance.VoxelGridRawResource = voxelGridRawResource;

	voxelParamsCBV = shaderResourceManager->CreateCBV(sizeof(VoxelParams));
	GRenderStageManager.RegisterStage("VoxelizationStage", this);

	GSceneResources.VoxelRadiance.MipCount = CVoxelGridMips;
	GSceneResources.VoxelRadiance.VoxelGridUAVMips = (TextureID*)Mem::Alloc(sizeof(TextureID) * CVoxelGridMips);
	uint32 voxelGridMipSize = CVoxelSize;
	for (uint32 i = 0; i < CVoxelGridMips; ++i)
	{
		GSceneResources.VoxelRadiance.VoxelGridUAVMips[i] = shaderResourceManager->CreateTexture3DUAV(voxelGridResource, voxelGridMipSize, i);
		voxelGridMipSize /= 2;
	}
}

static bool done = false;

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
	auto shaderResourceManager = GContext->ShaderResourceManager;
	uint32 count = 0;
	auto cameras = GContext->EntityManager->GetComponents<CameraComponent>(count);

	shaderResourceManager->CopyToCB(frameIndex, { &GSceneResources.FrameData.VoxelData, sizeof(VoxelParams) }, voxelParamsCBV);

	D3D12_VIEWPORT viewport = {};
	viewport.Width = (FLOAT)CVoxelSize;
	viewport.Height = (FLOAT)CVoxelSize;
	viewport.MaxDepth = 1.f;
	viewport.MinDepth = 0.f;

	D3D12_RECT scissorRect = {};
	scissorRect.left = 0;
	scissorRect.top = 0;
	scissorRect.right = sz.Width;
	scissorRect.bottom = sz.Height;

	TransitionDesc transitions[] = {
		//{ GSceneResources.VoxelRadiance.VoxelGridRawResource, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS }
		{ GSceneResources.VoxelGridResource, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS }
	};

	renderer->TransitionBarrier(commandList, transitions, _countof(transitions));

	commandList->RSSetViewports(1, &viewport);
	commandList->RSSetScissorRects(1, &scissorRect);
	renderer->SetRenderTargets(nullptr, 0, nullptr);
	renderer->SetPipelineState(commandList, GPipelineStates.VoxelizePSO);
	uint32 clearVal[] = { 0, 0, 0, 0 };
	const FLOAT clearColor[] = { 0.0f, 0.0f, 0.0f, 0.0f };
	//commandList->ClearUnorderedAccessViewFloat(
	//	renderer->GetTextureGPUHandle(GSceneResources.VoxelRadiance.VoxelGridRawUAV),
	//	srManager->GetTextureCPUHandle(GSceneResources.VoxelRadiance.VoxelGridRawUAV),
	//	resourceManager->GetResource(voxelGridResource), clearColor, 0, nullptr);

	commandList->ClearUnorderedAccessViewFloat(
		renderer->GetTextureGPUHandle(GSceneResources.VoxelRadiance.VoxelGridUAV),
		srManager->GetTextureCPUHandle(GSceneResources.VoxelRadiance.VoxelGridUAV),
		resourceManager->GetResource(voxelGridResource), clearColor, 0, nullptr);

	renderer->SetConstantBufferView(commandList, RootSigCBAll1, GSceneResources.ShadowCBV);
	renderer->SetConstantBufferView(commandList, RootSigCBAll2, voxelParamsCBV);
	renderer->SetShaderResourceView(commandList, RootSigUAV0, GSceneResources.VoxelRadiance.VoxelGridUAV);
	//renderer->SetShaderResourceView(commandList, RootSigUAV0, GSceneResources.VoxelRadiance.VoxelGridRawUAV);
	renderer->SetConstantBufferView(commandList, RootSigCBPixel0, GSceneResources.LightBufferCBV);
	renderer->SetShaderResourceView(commandList, RootSigSRVPixel2, GSceneResources.ShadowDepthTarget.Texture);

	auto drawables = frameContext.EntityManager->GetComponents<DrawableComponent>(count);

	for (uint32 i = 0; i < count; ++i)
	{
		auto mesh = drawables[i].Mesh;
		renderer->SetConstantBufferView(commandList, RootSigCBVertex0, drawables[i].CBView);
		renderer->SetShaderResourceViewMaterial(commandList, RootSigSRVPixel1, drawables[i].Material);
		renderer->DrawMesh(commandList, mesh);
	}

	auto meshManager = GContext->MeshManager;
	auto drawableModels = frameContext.EntityManager->GetComponents<DrawableModelComponent>(count);
	for (uint32 i = 0; i < count; ++i)
	{
		auto& model = modelManager->GetModel(drawableModels[i].Model);
		renderer->SetConstantBufferView(commandList, RootSigCBVertex0, drawableModels[i].CBView);
		auto meshHandle = model.Mesh;
		auto mesh = meshManager->GetMeshView(meshHandle);
		int matIndex = 0;
		commandList->IASetVertexBuffers(0, 1, &mesh.VertexBufferView);
		commandList->IASetIndexBuffer(&mesh.IndexBufferView);
		for (auto& m : mesh.MeshEntries)
		{
			auto materialHandle = model.Materials[matIndex]; //material maps to each mesh entry in model.
			auto material = shaderResourceManager->GetMaterial(materialHandle);
			renderer->SetShaderResourceViewMaterial(commandList, RootSigSRVPixel1, materialHandle);
			commandList->DrawIndexedInstanced(m.NumIndices, 1, m.BaseIndex, m.BaseVertex, 0);
			matIndex++;
		}
	}

	//TransitionDesc endTransitions[] = {
	//	{ GSceneResources.VoxelGridResource, D3D12_RESOURCE_STATE_UNORDERED_ACCESS,  D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE }
	//	//{ GSceneResources.VoxelRadiance.VoxelGridRawResource, D3D12_RESOURCE_STATE_UNORDERED_ACCESS,  D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE },
	//	//{ GSceneResources.VoxelGridResource, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS }
	//};

	//renderer->TransitionBarrier(commandList, endTransitions, _countof(endTransitions));
}


